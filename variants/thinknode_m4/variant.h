/*
 * variant.h - ThinkNode M4 (nRF52840 + LR1110)
 */

#pragma once

#include "WVariant.h"

////////////////////////////////////////////////////////////////////////////////
// Low frequency clock source

#define USE_LFXO    // 32.768 kHz crystal oscillator
#define VARIANT_MCK (64000000ul)

////////////////////////////////////////////////////////////////////////////////
// Number of pins

#define PINS_COUNT              (48)
#define NUM_DIGITAL_PINS        (48)
#define NUM_ANALOG_INPUTS       (1)
#define NUM_ANALOG_OUTPUTS      (0)

////////////////////////////////////////////////////////////////////////////////
// Power

#define NRF_APM                                 // detect usb power

#define EXT_CHRG_DETECT         (38)            // P1.06
#define EXT_CHRG_DETECT_VALUE   HIGH

#define PIN_A0                  (2)             // P0.02
#define PIN_VBAT_READ           PIN_A0
#define AREF_VOLTAGE            (3.0f)
#define ADC_MULTIPLIER          (2.0f)
#define ADC_RESOLUTION          (12)
#define ADC_MAX                 (4096)

static const uint8_t A0 = PIN_A0;

#define PIN_POWER_EN            (11)            // P0.11 - LoRa radio power

// Serial battery interface (secondary MCU in dock, 4800 baud)
#define HAS_SERIAL_BATTERY_LEVEL 1
#define SERIAL_BATTERY_RX       (30)            // P0.30
#define SERIAL_BATTERY_TX       (5)             // P0.05
#define PIN_SERIAL2_RX          SERIAL_BATTERY_RX
#define PIN_SERIAL2_TX          SERIAL_BATTERY_TX
#define SERIAL_BATTERY_BAUD     4800

////////////////////////////////////////////////////////////////////////////////
// UART pin definition

#define PIN_SERIAL1_RX          PIN_GPS_TX
#define PIN_SERIAL1_TX          PIN_GPS_RX

////////////////////////////////////////////////////////////////////////////////
// I2C pin definition

#define HAS_WIRE                (1)
#define WIRE_INTERFACES_COUNT   (1)

#define PIN_WIRE_SDA            (23)            // P0.23
#define PIN_WIRE_SCL            (25)            // P0.25
#define I2C_NO_RESCAN

////////////////////////////////////////////////////////////////////////////////
// SPI pin definition

#define SPI_INTERFACES_COUNT    (1)

#define PIN_SPI_MISO            (8)             // P0.08
#define PIN_SPI_MOSI            (7)             // P0.07
#define PIN_SPI_SCK             (6)             // P0.06
#define PIN_SPI_NSS             (27)            // P0.27

////////////////////////////////////////////////////////////////////////////////
// Builtin LEDs

#define PIN_LED_NOTIFICATION    (41)            // P1.09
#define PIN_LED_PAIRING         (13)            // P0.13

#define Battery_LED_1           (15)            // P0.15
#define Battery_LED_2           (17)            // P0.17
#define Battery_LED_3           (34)            // P1.02
#define Battery_LED_4           (36)            // P1.04

#define LED_BLUE                (-1)            // No blue LED
#define LED_BUILTIN             PIN_LED_NOTIFICATION
#define LED_PIN                 LED_BUILTIN
#define LED_STATE_ON            HIGH

////////////////////////////////////////////////////////////////////////////////
// Builtin buttons

#define PIN_BUTTON1             (4)             // P0.04
#define BUTTON_PIN              PIN_BUTTON1

////////////////////////////////////////////////////////////////////////////////
// GPS

#define HAS_GPS                 1
#define PIN_GPS_RX              (44)            // P1.12 - GPS module TX -> MCU RX
#define PIN_GPS_TX              (46)            // P1.14 - GPS module RX <- MCU TX
#define PIN_GPS_EN              (43)            // P1.11 - GPS enable
#define PIN_GPS_RESET           (3)             // P0.03 - GPS reset
#define PIN_GPS_STANDBY         (28)            // P0.28 - GPS standby
#define GPS_RESET_ACTIVE        HIGH
#define GPS_EN_ACTIVE           LOW
#define GPS_BAUDRATE            9600

////////////////////////////////////////////////////////////////////////////////
// Peripherals on I2C bus - VEXT control

#define VEXT_ENABLE             (32)            // P1.00
#define VEXT_ON_VALUE           LOW
