#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BatteryManager.h"
#include "AnimationPlayer.h"

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
AnimationPlayer player(display, battery);

const unsigned long TIMEOUT = 30000; // 30 seconds
unsigned long lastTouchTime = 0;

// Touch tracking
unsigned long touchStartTime = 0;
int tapCount = 0;
unsigned long lastTapTime = 0;
bool lastTouchState = false;

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
  
  player.begin();
  player.playIntro();
}

void loop() {
  battery.update();
  handleTouch();
  player.update();
  handleSleep();
}

// ── TOUCH HANDLER ────────────────────────────────────────────────────────
void handleTouch() {
  if (!player.isTouchEnabled()) return;
  bool touchState = (digitalRead(TTP223_PIN) == HIGH);
  unsigned long now = millis();

  static bool longPressActive = false;
  static bool specialVideoActive = false;

  // Deteksi Awal Sentuh (Rising Edge)
  if (touchState && !lastTouchState) {
    if (now - lastTapTime < 400) tapCount++;
    else tapCount = 1;
    touchStartTime = now;
    lastTapTime = now;
    lastTouchTime = now;
    player.resetFrames();
    longPressActive = false;
    specialVideoActive = false;
  }

  // Logika Tap (Setelah jari diangkat)
  if (!touchState && lastTouchState) {
    if (tapCount == 2) {
      player.playRandomVideo();
    } else if (tapCount >= 3) {
      player.playVideo(8, true); // Blooms (cute)
    }
    else if (now - touchStartTime < 400) {
      player.playVideo(2, true); // Glance (one tap / blink)
    }
  }

  // Logika Long Press
  if (touchState && (now - touchStartTime > 400)) {
    if (now - touchStartTime > 3000) {
      if (!specialVideoActive) {
        player.playSpecialVideo();
        specialVideoActive = true;
      }
    }
    else {
      if (!longPressActive) {
        player.playVideo(9, true); // Love (uwu)
        longPressActive = true;
        tapCount = 0; // Reset tap count to prevent tap actions on release
      }
    }
  }

  // Reset Tap Counter
  if (!touchState && (now - lastTapTime > 700)) tapCount = 0;

  lastTouchState = touchState;
}

void handleSleep() {
  // Reset sleep timer if an animation is currently playing
  if (player.isAnimActive()) {
    lastTouchTime = millis();
  }

  // Timeout 30 detik
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
