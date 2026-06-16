#include "AnimationPlayer.h"
#include "VideoFrame.h"
#include "charge.h"

AnimationPlayer::AnimationPlayer(Adafruit_SSD1306& display, BatteryManager& battery)
    : _display(display), _battery(battery), _state(STATE_INTRO),
      _currentVideoIdx(0), _interruptible(false), _specialVideoFrame(0),
      _previousMillis(0) {
    resetFrames();
}

void AnimationPlayer::begin() {
    resetFrames();
}

void AnimationPlayer::resetFrames() {
    for (int i = 0; i < TOTAL_VIDEOS; i++) {
        _playlistFrames[i] = 0;
    }
    _specialVideoFrame = 0;
}

void AnimationPlayer::playIntro() {
    _state = STATE_INTRO;
    _currentVideoIdx = 0;
    _interruptible = false;
    resetFrames();
    _previousMillis = millis();
}

void AnimationPlayer::playIdle() {
    _state = STATE_IDLE;
    _currentVideoIdx = 1;
    _interruptible = true;
    _playlistFrames[1] = 0;
    _previousMillis = millis();
}

void AnimationPlayer::playVideo(int index, bool interruptible) {
    if (index >= 0 && index < TOTAL_VIDEOS) {
        resetFrames();
        _state = STATE_ANIMATION;
        _currentVideoIdx = index;
        _interruptible = interruptible;
        _playlistFrames[index] = 0;
        _previousMillis = millis();
    }
}

void AnimationPlayer::playNextVideo() {
    // Cycles through the interactive playlist animations (indices 2 to 9)
    int nextIdx = _currentVideoIdx + 1;
    if (nextIdx > 9 || nextIdx < 2) {
        nextIdx = 2; // Loop back to the first interactive animation (glance/one tap)
    }
    playVideo(nextIdx, true);
}

void AnimationPlayer::playRandomVideo() {
    // Picks a random animation from the double tap pool (indices 3 to 7)
    int rndIdx = random(3, 8); // random(min, max) in Arduino is exclusive of max
    playVideo(rndIdx, true);
}

void AnimationPlayer::playSpecialVideo() {
    resetFrames();
    _state = STATE_SPECIAL_VIDEO;
    _interruptible = false;
    _specialVideoFrame = 0;
    _previousMillis = millis();
}

void AnimationPlayer::playCharging() {
    _state = STATE_CHARGING;
    _interruptible = true;
    _previousMillis = millis();
}

void AnimationPlayer::playFullCharge() {
    _state = STATE_FULL_CHARGE;
    _interruptible = true;
    _previousMillis = millis();
}

bool AnimationPlayer::isTouchEnabled() const {
    return (_state != STATE_INTRO && _state != STATE_SPECIAL_VIDEO);
}

bool AnimationPlayer::isInterruptible() const {
    return _interruptible;
}

bool AnimationPlayer::isAnimActive() const {
    return (_state == STATE_INTRO || _state == STATE_ANIMATION || _state == STATE_SPECIAL_VIDEO);
}

AnimationPlayer::PlayerState AnimationPlayer::getState() const {
    return _state;
}

void AnimationPlayer::drawBatteryStatus() {
    int percent = _battery.getPercentage();
    bool isCharging = _battery.isCharging();
    bool isFullCharge = _battery.isFullCharge();

    if (isCharging) {
        _display.drawBitmap(0, 0, charging, 128, 64, SSD1306_WHITE);
    } else if (isFullCharge) {
        _display.drawBitmap(0, 0, fullCharge, 128, 64, SSD1306_WHITE);
    }
 
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(104, 0);
    _display.print(percent);
    _display.print("%");
}

void AnimationPlayer::update() {
    unsigned long currentMillis = millis();

    switch (_state) {
        case STATE_INTRO: {
            Animate video = playlist[0];
            if (_playlistFrames[0] >= video.frame_count) {
                _playlistFrames[0] = 0;
                playIdle();
                return;
            }

            if (currentMillis - _previousMillis >= video.delays[_playlistFrames[0]]) {
                _previousMillis = currentMillis;

                _display.clearDisplay();
                _display.drawBitmap(0, 0, video.frames[_playlistFrames[0]], video.width, video.height, SSD1306_WHITE);
                _display.display();

                _playlistFrames[0]++;
            }
            break;
        }

        case STATE_IDLE: {
            Animate video = playlist[1];
            if (_playlistFrames[1] >= video.frame_count) {
                _playlistFrames[1] = 0;
            }

            if (currentMillis - _previousMillis >= video.delays[_playlistFrames[1]]) {
                _previousMillis = currentMillis;

                _display.clearDisplay();
                _display.drawBitmap(0, 0, video.frames[_playlistFrames[1]], video.width, video.height, SSD1306_WHITE);
                drawBatteryStatus();
                _display.display();

                _playlistFrames[1]++;
            }
            break;
        }

        case STATE_ANIMATION: {
            Animate video = playlist[_currentVideoIdx];
            if (_playlistFrames[_currentVideoIdx] >= video.frame_count) {
                _playlistFrames[_currentVideoIdx] = 0;
                playIdle();
                return;
            }

            if (currentMillis - _previousMillis >= video.delays[_playlistFrames[_currentVideoIdx]]) {
                _previousMillis = currentMillis;

                _display.clearDisplay();
                _display.drawBitmap(0, 0, video.frames[_playlistFrames[_currentVideoIdx]], video.width, video.height, SSD1306_WHITE);
                _display.display();

                _playlistFrames[_currentVideoIdx]++;
            }
            break;
        }

        case STATE_SPECIAL_VIDEO: {
            if (_specialVideoFrame >= VIDEO_FRAME_COUNT) {
                _specialVideoFrame = 0;
                playIdle();
                return;
            }

            if (currentMillis - _previousMillis >= VIDEO_DELAY) {
                _previousMillis = currentMillis;

                _display.clearDisplay();
                _display.drawBitmap(0, 0, video_frames[_specialVideoFrame], 128, 64, SSD1306_WHITE);
                _display.display();

                _specialVideoFrame++;
            }
            break;
        }

        case STATE_CHARGING: {
            if (currentMillis - _previousMillis >= 500) {
                _previousMillis = currentMillis;

                _display.clearDisplay();
                _display.drawBitmap(0, 0, charging, 128, 64, SSD1306_WHITE);
                
                int percent = _battery.getPercentage();
                _display.setTextSize(1);
                _display.setTextColor(SSD1306_WHITE);
                _display.setCursor(104, 0);
                _display.print(percent);
                _display.print("%");
                
                _display.display();
            }
            break;
        }

        case STATE_FULL_CHARGE: {
            if (currentMillis - _previousMillis >= 500) {
                _previousMillis = currentMillis;

                _display.clearDisplay();
                _display.drawBitmap(0, 0, fullCharge, 128, 64, SSD1306_WHITE);
                
                int percent = _battery.getPercentage();
                _display.setTextSize(1);
                _display.setTextColor(SSD1306_WHITE);
                _display.setCursor(104, 0);
                _display.print(percent);
                _display.print("%");
                
                _display.display();
            }
            break;
        }
    }
}
