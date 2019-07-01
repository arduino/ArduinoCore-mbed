#include "Arduino.h"

PinDescription g_APinDescription[] = {
  // D0 - D7
  P1_3,  NULL,     // D0/TX
  P1_10, NULL,     // D1/RX
  P1_11, NULL,     // D2
  P1_12, NULL,     // D3
  P1_15, NULL,     // D4
  P1_13, NULL,     // D5
  P1_14, NULL,     // D6
  P0_9,  NULL,     // D7

  // D8 - D13
  P0_10, NULL,     // D8
  P0_27, NULL,     // D9
  P1_2,  NULL,     // D10
  P1_1,  NULL,     // D11/MOSI
  P1_8,  NULL,     // D12/MISO
  P0_13, NULL,     // D13/SCK/LED

  // A0 - A7
  P0_4,  NULL,     // A0
  P0_5,  NULL,     // A1
  P0_30, NULL,     // A2
  P0_29, NULL,     // A3
  P0_31, NULL,     // A4/SDA
  P0_2,  NULL,     // A5/SCL
  P0_28, NULL,     // A6
  P0_3,  NULL,     // A7

  // LEDs
  P0_24, NULL,     // LED R
  P0_16, NULL,     // LED G
  P0_6,  NULL,     // LED B
  P1_9,  NULL,     // LED PWR

  P0_19, NULL,     // INT APDS

  // PDM
  P0_17, NULL,     // PDM PWR
  P0_26, NULL,     // PDM CLK
  P0_25, NULL,     // PDM DIN

  // Internal I2C
  P0_14, NULL,     // SDA2
  P0_15, NULL,     // SCL2

  // Internal I2C
  P1_0,  NULL,     // I2C_PULL
  P0_22, NULL,     // VDD_ENV_ENABLE
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}

void initVariant() {
  // turn power LED on
  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  // Errata Nano33BLE - I2C pullup is on SWO line, need to disable TRACE
  // was being enabled by nrfx_clock_anomaly_132
  CoreDebug->DEMCR = 0;
  NRF_CLOCK->TRACECONFIG = 0;
}

#ifdef SERIAL_CDC
#include "CDC.h"
CDC SerialUSB;
#endif