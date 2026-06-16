#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "animate_registry.h"
#include "BatteryManager.h"

class AnimationPlayer {
public:
    enum PlayerState {
        STATE_INTRO,
        STATE_IDLE,
        STATE_ANIMATION,
        STATE_SPECIAL_VIDEO,
        STATE_CHARGING,
        STATE_FULL_CHARGE
    };

    AnimationPlayer(Adafruit_SSD1306& display, BatteryManager& battery);
    void begin();
    void update();

    void playIntro();
    void playIdle();
    void playVideo(int index, bool interruptible = true);
    void playNextVideo();
    void playRandomVideo();
    void playSpecialVideo();
    void playCharging();
    void playFullCharge();

    bool isTouchEnabled() const;
    bool isInterruptible() const;
    bool isAnimActive() const;
    PlayerState getState() const;
    void resetFrames();

private:
    Adafruit_SSD1306& _display;
    BatteryManager& _battery;

    PlayerState _state;
    int _currentVideoIdx;
    bool _interruptible;

    int _playlistFrames[TOTAL_VIDEOS];
    int _specialVideoFrame;
    unsigned long _previousMillis;

    void drawBatteryStatus();
};

#endif // ANIMATION_PLAYER_H
