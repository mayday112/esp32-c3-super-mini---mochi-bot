#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "animate_registry.h"
#include "VideoFrame.h"
#include "charge.h"
#include "BatteryManager.h"

#define TTP223_PIN 4
#define SDA_PIN 8
#define SCL_PIN 9

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define WARNA_LAYAR WHITE

#define uS_TO_S_FACTOR 1000000ULL   // Microseconds to seconds
#define TIME_TO_SLEEP 1800  // Deep sleep duration in seconds 1800s (30 minute)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

BatteryManager battery;

const unsigned long TIMEOUT = 30000;//30 seconds
unsigned long lastTouchTime = 0;
unsigned long previousMillis = 0;
RTC_DATA_ATTR bool enableTouch = false;//false dulu saat intro pertama diputar

enum State { INTRO, IDLE, BLINK, RANDOM, PLAY_VIDEO, BLOOMS, LOVE };
State currentState = INTRO;

int currentVideoIdx = 0;//intro
//RTC_DATA_ATTR bool introPlayed = false;
//String state = "idle";// idle,one: blink (sementara ganti jadi tampilkan persentase baterai),two: random animation,triple: playVideo(without display battery), long: Blooms, long >3s: love
int currentFrames[TOTAL_VIDEOS] = {
  0,//curent frame for intro
  0,//curent frame for idle
  0,//curent frame for glance
  0,//curent frame for angry
  0,//curent frame for sad
  0,//curent frame for grumpy
  0,//curent frame for giggle
  0,//curent frame for happy
  0,//curent frame for cute
  0,//curent frame for uwu
};
int currentVideoFrame = 0;

// Touch tracking
unsigned long touchStartTime = 0;
int tapCount = 0;
unsigned long lastTapTime = 0;
bool lastTouchState = false;

unsigned long lastActivityTime = 0;

void setup() {
  Serial.begin(115200);

  pinMode(TTP223_PIN, INPUT); // Set sebagai input digital

  battery.begin();

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }

  // Setup Deep Sleep dengan External Wakeup (Pin 4) & set deep sleep timer
  // TTP223 kasih output HIGH saat sentuh, jadi kita set ke mode HIGH_LEVEL
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_enable_gpio_wakeup(BIT(TTP223_PIN), ESP_GPIO_WAKEUP_GPIO_HIGH);

  // I2C ngebut 800 kHz biar FPS video aman
  Wire.setClock(800000);
  display.clearDisplay();
  display.display();
  delay(300);
  lastTouchTime = millis();
}

void loop() {
  battery.update();
  handleTouch();

  switch (currentState) {
    case INTRO: handleIntro(); break;
    case IDLE: playIdle(); break;
    case BLINK: animate(); break;
    case RANDOM: animate(); break;
    case PLAY_VIDEO: playSpecialVideo(); break;
    case BLOOMS: animate(); break;
    case LOVE: animate(); break;
    default: playIdle();
  }

  handleSleep();
}

// ── TOUCH HANDLER ────────────────────────────────────────────────────────
void handleTouch() {
  if (!enableTouch) return;
  bool touchState = (digitalRead(TTP223_PIN) == HIGH);
  unsigned long now = millis();

  // Deteksi Awal Sentuh (Rising Edge)
  if (touchState && !lastTouchState) {
    if (now - lastTapTime < 400) tapCount++;
    else tapCount = 1;
    touchStartTime = now;
    lastTapTime = now;
    lastTouchTime = now;
    resetAllFrames();
  }

  // Logika Tap (Setelah jari diangkat)
  if (!touchState && lastTouchState) {
    if (tapCount == 2) {
      currentVideoIdx = random(3, 7);
      currentState = RANDOM;
    } else if (tapCount >= 3) {
      currentVideoIdx = 8;
      currentState = BLOOMS;
    }
    else if (now - touchStartTime < 400) {
      currentVideoIdx = 2;
      currentState = BLINK; // Tap tunggal
    }
  }

  // Logika Long Press
  if (touchState && (now - touchStartTime > 400)) {
    if (now - touchStartTime > 3000) {
      currentState = PLAY_VIDEO;
      enableTouch = false;
    }
    else {
      currentVideoIdx = 9;
      currentState = LOVE;
    }
  }

  // Reset Tap Counter
  if (!touchState && (now - lastTapTime > 700)) tapCount = 0;

  lastTouchState = touchState;
}

void drawBattery() {
  int percent = battery.getPercentage();
  bool isCharging = battery.isCharging();
  bool isFullCharge = battery.isFullCharge();

  if(isCharging) display.drawBitmap(0, 0, charging, SCREEN_WIDTH, SCREEN_HEIGHT, WARNA_LAYAR);
  else if (isFullCharge) display.drawBitmap(0, 0, fullCharge, SCREEN_WIDTH, SCREEN_HEIGHT, WARNA_LAYAR);
 
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Hitung posisi pojok kanan atas
  // Layar 128px, teks ukuran 1 (6px lebar per karakter).
  // "100%" ada 4 karakter = 24px.
  // 128 - 24 = 104 (x)
  display.setCursor(104, 0);
  display.print(percent);
  display.print("%");
}

void handleSleep() {
  // Timeout 10 detik
  if (millis() - lastTouchTime > TIMEOUT) {
    display.clearDisplay();
    display.println("Tidur dulu...");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
    esp_deep_sleep_start();
  }
}

void animate() {
  unsigned long currentMillis = millis();
  Animate video = playlist[currentVideoIdx];

  if (currentFrames[currentVideoIdx] >= video.frame_count) {
    currentFrames[currentVideoIdx] = 0;
    currentState = IDLE;
    return;
  }

  if (currentMillis - previousMillis >= video.delays[currentFrames[currentVideoIdx]]) {
    lastTouchTime = currentMillis;
    lastActivityTime = currentMillis;
    previousMillis = currentMillis;

    display.clearDisplay();

    // Variabel WARNA_LAYAR otomatis berubah ngikutin chip yang dipilih
    display.drawBitmap(0, 0, video.frames[currentFrames[currentVideoIdx]], video.width, video.height, WARNA_LAYAR);
    display.display();

    currentFrames[currentVideoIdx]++;
  }
}

void handleIntro() {
  unsigned long currentMillis = millis();

  if (currentFrames[0] >= playlist[0].frame_count) {
    currentFrames[0] = 0;
    enableTouch = true;
    currentVideoIdx = 1;
    currentState = IDLE;
//    introPlayed = true;
    return;
  }

  if (currentMillis - previousMillis >= playlist[0].delays[currentFrames[0]]) {
    lastTouchTime = currentMillis;
    lastActivityTime = currentMillis;
    previousMillis = currentMillis;

    display.clearDisplay();

    // Variabel WARNA_LAYAR otomatis berubah ngikutin chip yang dipilih
    display.drawBitmap(0, 0, playlist[0].frames[currentFrames[0]], playlist[0].width, playlist[0].height, WARNA_LAYAR);
    display.display();

    currentFrames[0]++;
  }
}

void playIdle() {
  unsigned long currentMillis = millis();

  if (currentFrames[1] >= playlist[1].frame_count) {
    currentFrames[1] = 0;
  }

  if (currentMillis - previousMillis >= playlist[1].delays[currentFrames[1]]) {
    lastActivityTime = currentMillis;
    previousMillis = currentMillis;

    display.clearDisplay();

    // Variabel WARNA_LAYAR otomatis berubah ngikutin chip yang dipilih
    display.drawBitmap(0, 0, playlist[1].frames[currentFrames[1]], playlist[1].width, playlist[1].height, WARNA_LAYAR);
    drawBattery();
    display.display();

    currentFrames[1]++;
  }
}

void playSpecialVideo() {
  unsigned long currentMillis = millis();

  if (currentVideoFrame >= VIDEO_FRAME_COUNT) {
    currentVideoFrame = 0;
    currentState = IDLE;
    enableTouch = true;
    return;
  }

  if (currentMillis - previousMillis >= VIDEO_DELAY) {
    lastTouchTime = currentMillis;
    lastActivityTime = currentMillis;
    previousMillis = currentMillis;

    display.clearDisplay();

    // Variabel WARNA_LAYAR otomatis berubah ngikutin chip yang dipilih
    display.drawBitmap(0, 0, video_frames[currentVideoFrame], SCREEN_WIDTH, SCREEN_HEIGHT, WARNA_LAYAR);
    display.display();

    currentVideoFrame++;
  }
}

void resetAllFrames(){
  for(int i = 0; i < TOTAL_VIDEOS; i++){
    currentFrames[i] = 0; 
  }
}
