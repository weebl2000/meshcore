#pragma once

#include <Arduino.h>
#include <MeshCore.h>
#include <helpers/NRF52Board.h>

#define VBAT_MV_PER_LSB   (AREF_VOLTAGE * 1000.0f / ADC_MAX)
#define REAL_VBAT_MV_PER_LSB (ADC_MULTIPLIER * VBAT_MV_PER_LSB)

class ThinkNodeM4Board : public NRF52BoardDCDC {
protected:
#if NRF52_POWER_MANAGEMENT
  void initiateShutdown(uint8_t reason) override;
#endif
  uint8_t btn_prev_state;

#ifdef HAS_SERIAL_BATTERY_LEVEL
  uint16_t _serial_batt_mv;
  bool _serial_batt_valid;
  void pollSerialBattery();
#endif

public:
  ThinkNodeM4Board() : NRF52Board("THINKNODE_M4_OTA") {}
  void begin();
  uint16_t getBattMilliVolts() override;

#if defined(P_LORA_TX_LED)
  void onBeforeTransmit() override {
    digitalWrite(P_LORA_TX_LED, P_LORA_TX_LED_ON);
  }
  void onAfterTransmit() override {
    digitalWrite(P_LORA_TX_LED, !P_LORA_TX_LED_ON);
  }
#endif

  const char* getManufacturerName() const override {
    return "Elecrow ThinkNode M4";
  }

  int buttonStateChanged() {
  #ifdef BUTTON_PIN
    uint8_t v = digitalRead(BUTTON_PIN);
    if (v != btn_prev_state) {
      btn_prev_state = v;
      return (v == LOW) ? 1 : -1;
    }
  #endif
    return 0;
  }

  void powerOff() override {
    #ifdef P_LORA_TX_LED
    digitalWrite(P_LORA_TX_LED, !P_LORA_TX_LED_ON);
    #endif

    sd_power_system_off();
  }
};
