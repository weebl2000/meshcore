#include <Arduino.h>
#include "ThinkNodeM4Board.h"
#include <Wire.h>

#include <bluefruit.h>

void ThinkNodeM4Board::begin() {
  NRF52Board::begin();
  btn_prev_state = HIGH;

#ifdef HAS_SERIAL_BATTERY_LEVEL
  _serial_batt_mv = 0;
  _serial_batt_valid = false;
  Serial2.begin(SERIAL_BATTERY_BAUD);
#endif

  Wire.begin();

  delay(10);   // give LR1110 some time to power up
}

#ifdef HAS_SERIAL_BATTERY_LEVEL
// Protocol: [0xFE] [percent] [volt_int] [volt_hundredths] [volt_tenthousandths] [0xFD]
// 4800 baud from secondary MCU in dock
void ThinkNodeM4Board::pollSerialBattery() {
  if (Serial2.available() < 6)
    return;

  // flush stale data, keep only latest frame
  while (Serial2.available() > 11)
    Serial2.read();

  // scan for start byte
  for (int tries = 0; tries < 10; tries++) {
    if (Serial2.read() == 0xFE) {
      uint8_t percent   = Serial2.read();
      uint8_t volt_int  = Serial2.read();
      uint8_t volt_dec2 = Serial2.read();
      uint8_t volt_dec4 = Serial2.read();
      uint8_t end_byte  = Serial2.read();

      if (end_byte != 0xFD)
        return;  // bad frame, try again next time

      (void)percent;  // available if needed later

      float voltage = volt_int + (volt_dec2 / 100.0f) + (volt_dec4 / 10000.0f);
      voltage *= 2.0f;  // voltage divider in dock

      _serial_batt_mv = (uint16_t)(voltage * 1000.0f);
      _serial_batt_valid = true;
      return;
    }
  }
}
#endif

uint16_t ThinkNodeM4Board::getBattMilliVolts() {
#ifdef HAS_SERIAL_BATTERY_LEVEL
  pollSerialBattery();
  if (_serial_batt_valid)
    return _serial_batt_mv;
#endif

  // ADC fallback
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(ADC_RESOLUTION);
  delay(10);

  int adcvalue = analogRead(PIN_VBAT_READ);
  return (uint16_t)((float)adcvalue * REAL_VBAT_MV_PER_LSB);
}
