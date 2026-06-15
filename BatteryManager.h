#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>

#define PIN_BAT_ADC 3
#define PIN_CHRG 2
#define PIN_FULL_CHRG 1

class BatteryManager {
public:
    BatteryManager();
    void begin();
    void update(); // Call this in main loop
    
    int getPercentage();
    bool isCharging();
    bool isFullCharge();
    float getVoltage();

private:
    float _voltage;
    int _percentage;
    bool _charging;
    bool _full_charge;
    
    // For smoothing
    float _filteredVoltage;
    const float _alpha = 0.3; // Filter constant for smoothing (0.0 - 1.0)
    const float ADC_CALIBRATION_FACTOR = 0.898f;
    
    unsigned long _lastUpdate;
    const unsigned long _updateInterval = 1000; // Update every 1 second
    
    float readVoltage();
};

#endif
