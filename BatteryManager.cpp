#include "BatteryManager.h"

BatteryManager::BatteryManager() :
  _voltage(0), _percentage(0), _charging(false),
  _filteredVoltage(0), _lastUpdate(0) {}

void BatteryManager::begin() {
  pinMode(PIN_CHRG, INPUT_PULLUP);
  pinMode(PIN_FULL_CHRG, INPUT_PULLUP);
  analogReadResolution(12); // ESP32-C3 has 12-bit ADC
  analogSetPinAttenuation(PIN_BAT_ADC, ADC_11db);
  
  // Initial reading
  _filteredVoltage = readVoltage();
  _voltage = _filteredVoltage;
}

float BatteryManager::readVoltage() {
  // ADC reads 0-4095 for 0-3.3V (default attenuation)
  // 10k/10k divider means V_bat = 2 * V_adc
   uint32_t sum = 0;

  for (int i = 0; i < 10; i++)
  {
    sum += analogRead(PIN_BAT_ADC);
  }

  int raw = sum / 10;
//  int raw = analogRead(PIN_BAT_ADC);
  float v_adc = (raw / 4095.0) * 3.3;

  // Calibrate factor based on actual resistor tolerance if needed
  // Factor 2.0 because of 10k/10k divider
  return v_adc * 2.0 * ADC_CALIBRATION_FACTOR;
}

void BatteryManager::update() {
  if (millis() - _lastUpdate >= _updateInterval) {
    float rawV = readVoltage();

    // Smoothing filter (EMA)
    _filteredVoltage = (_alpha * rawV) + ((1.0 - _alpha) * _filteredVoltage);
    _voltage = _filteredVoltage;

    // Detection PIN_CHRG (Active LOW for ME4054B)
    _charging = (digitalRead(PIN_CHRG) == LOW);
    _full_charge = (digitalRead(PIN_FULL_CHRG) == LOW);

    // Map 3.0V - 4.2V to 0 - 100%
    // Using a slightly wider range for stability
    float pct = (_voltage - 3.3) / (4.2 - 3.3) * 100.0;
    if (pct > 100) pct = 100;
    if (pct < 0) pct = 0;
    _percentage = (int)pct;

    _lastUpdate = millis();
  }
}

int BatteryManager::getPercentage() {
  return _percentage;
}

bool BatteryManager::isCharging() {
  return _charging;
}

bool BatteryManager::isFullCharge() {
  return _full_charge;
}

float BatteryManager::getVoltage() {
  return _voltage;
}
