/*
 * variant.cpp - ThinkNode M4 (nRF52840 + LR1110)
 */

#include "variant.h"
#include "wiring_constants.h"
#include "wiring_digital.h"

const uint32_t g_ADigitalPinMap[] =
{
  0xff,  // P0.00 - LFXO (do not use)
  0xff,  // P0.01 - LFXO (do not use)
  2,  // P0.02
  3,  // P0.03
  4,  // P0.04
  5,  // P0.05
  6,  // P0.06
  7,  // P0.07
  8,  // P0.08
  9,  // P0.09
  10, // P0.10
  11, // P0.11
  12, // P0.12
  13, // P0.13
  14, // P0.14
  15, // P0.15
  16, // P0.16
  17, // P0.17
  18, // P0.18
  19, // P0.19
  20, // P0.20
  21, // P0.21
  22, // P0.22
  23, // P0.23
  24, // P0.24
  25, // P0.25
  26, // P0.26
  27, // P0.27
  28, // P0.28
  29, // P0.29
  30, // P0.30
  31, // P0.31
  32, // P1.00
  33, // P1.01
  34, // P1.02
  35, // P1.03
  36, // P1.04
  37, // P1.05
  38, // P1.06
  39, // P1.07
  40, // P1.08
  41, // P1.09
  42, // P1.10
  43, // P1.11
  44, // P1.12
  45, // P1.13
  46, // P1.14
  47, // P1.15
};

void initVariant()
{
  // Enable radio power
  pinMode(PIN_POWER_EN, OUTPUT);
  digitalWrite(PIN_POWER_EN, HIGH);

  // LEDs off
  pinMode(PIN_LED_NOTIFICATION, OUTPUT);
  digitalWrite(PIN_LED_NOTIFICATION, LOW);
  pinMode(PIN_LED_PAIRING, OUTPUT);
  digitalWrite(PIN_LED_PAIRING, LOW);
  pinMode(Battery_LED_1, OUTPUT);
  digitalWrite(Battery_LED_1, LOW);
  pinMode(Battery_LED_2, OUTPUT);
  digitalWrite(Battery_LED_2, LOW);
  pinMode(Battery_LED_3, OUTPUT);
  digitalWrite(Battery_LED_3, LOW);
  pinMode(Battery_LED_4, OUTPUT);
  digitalWrite(Battery_LED_4, LOW);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // GPS: powered but in standby
  pinMode(PIN_GPS_EN, OUTPUT);
  digitalWrite(PIN_GPS_EN, !GPS_EN_ACTIVE);       // disabled (HIGH, since active LOW)
  pinMode(PIN_GPS_RESET, OUTPUT);
  digitalWrite(PIN_GPS_RESET, !GPS_RESET_ACTIVE);  // not in reset (LOW, since active HIGH)
  pinMode(PIN_GPS_STANDBY, OUTPUT);
  digitalWrite(PIN_GPS_STANDBY, HIGH);              // standby active
}
