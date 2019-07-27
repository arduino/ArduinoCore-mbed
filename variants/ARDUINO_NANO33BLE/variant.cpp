#include "Arduino.h"

PinDescription g_APinDescription[] = {
  // D0 - D7
  P1_3,  NULL, NULL,     // D0/TX
  P1_10, NULL, NULL,     // D1/RX
  P1_11, NULL, NULL,     // D2
  P1_12, NULL, NULL,     // D3
  P1_15, NULL, NULL,     // D4
  P1_13, NULL, NULL,     // D5
  P1_14, NULL, NULL,     // D6
  P0_23,  NULL, NULL,     // D7

  // D8 - D13
  P0_21, NULL, NULL,     // D8
  P0_27, NULL, NULL,     // D9
  P1_2,  NULL, NULL,     // D10
  P1_1,  NULL, NULL,     // D11/MOSI
  P1_8,  NULL, NULL,     // D12/MISO
  P0_13, NULL, NULL,     // D13/SCK/LED

  // A0 - A7
  P0_4,  NULL, NULL,     // A0
  P0_5,  NULL, NULL,     // A1
  P0_30, NULL, NULL,     // A2
  P0_29, NULL, NULL,     // A3
  P0_31, NULL, NULL,     // A4/SDA
  P0_2,  NULL, NULL,     // A5/SCL
  P0_28, NULL, NULL,     // A6
  P0_3,  NULL, NULL,     // A7

  // LEDs
  P0_24, NULL, NULL,     // LED R
  P0_16, NULL, NULL,     // LED G
  P0_6,  NULL, NULL,     // LED B
  P1_9,  NULL, NULL,     // LED PWR

  P0_19, NULL, NULL,     // INT APDS

  // PDM
  P0_17, NULL, NULL,     // PDM PWR
  P0_26, NULL, NULL,     // PDM CLK
  P0_25, NULL, NULL,     // PDM DIN

  // Internal I2C
  P0_14, NULL, NULL,     // SDA2
  P0_15, NULL, NULL,     // SCL2

  // Internal I2C
  P1_0,  NULL, NULL,     // I2C_PULL
  P0_22, NULL, NULL,     // VDD_ENV_ENABLE
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}

#include "nrf_rtc.h"

void initVariant() {
  // turn power LED on
  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  // Errata Nano33BLE - I2C pullup is on SWO line, need to disable TRACE
  // was being enabled by nrfx_clock_anomaly_132
  CoreDebug->DEMCR = 0;
  NRF_CLOCK->TRACECONFIG = 0;

  // FIXME: bootloader enables interrupt on COMPARE[0], which we don't handle
  // Disable it here to avoid getting stuck when OVERFLOW irq is triggered
  nrf_rtc_event_disable(NRF_RTC1, NRF_RTC_INT_COMPARE0_MASK);
  nrf_rtc_int_disable(NRF_RTC1, NRF_RTC_INT_COMPARE0_MASK);

  // FIXME: always enable I2C pullup and power @startup
  // Change for maximum powersave
  pinMode(PIN_ENABLE_SENSORS_3V3, OUTPUT);
  pinMode(PIN_ENABLE_I2C_PULLUP, OUTPUT);

  digitalWrite(PIN_ENABLE_SENSORS_3V3, HIGH);
  digitalWrite(PIN_ENABLE_I2C_PULLUP, HIGH);
}

#ifdef SERIAL_CDC

static void utox8(uint32_t val, uint8_t* s) {
  for (int i = 0; i < 16; i=i+2) {
    int d = val & 0XF;
    val = (val >> 4);

    s[15 - i -1] = d > 9 ? 'A' + d - 10 : '0' + d;
    s[15 - i] = '\0';
  }
}

uint8_t getUniqueSerialNumber(uint8_t* name) {
  #define SERIAL_NUMBER_WORD_0  NRF_FICR->DEVICEADDR[1]
  #define SERIAL_NUMBER_WORD_1  NRF_FICR->DEVICEADDR[0]

  utox8(SERIAL_NUMBER_WORD_0, &name[0]);
  utox8(SERIAL_NUMBER_WORD_1, &name[16]);

  return 32;
}

void _ontouch1200bps_() {
  __disable_irq();
  NRF_POWER->GPREGRET = DFU_MAGIC_SERIAL_ONLY_RESET;
  NVIC_SystemReset();
}

#endif