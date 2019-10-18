#include "Arduino.h"

PinDescription g_APinDescription[] = {
  // D0 - D7
  PA_10, NULL, NULL,     // D0/TX
  PA_11, NULL, NULL,     // D1/RX
  PA_0,  NULL, NULL,     // D2
  PB_2,  NULL, NULL,     // D3
  PB_5,  NULL, NULL,     // D4
  PB_6,  NULL, NULL,     // D5
  PB_7,  NULL, NULL,     // D6
  NC,    NULL, NULL,     // D7

  // D8 - D13
  NC,    NULL, NULL,     // D8
  PB_7,  NULL, NULL,     // D9
  PB_12, NULL, NULL,     // D10
  PB_15, NULL, NULL,     // D11/MOSI
  PB_14, NULL, NULL,     // D12/MISO
  PB_13, NULL, NULL,     // D13/SCK/LED

  // A0 - A7
  PA_4,  NULL, NULL,     // A0
  PA_5,  NULL, NULL,     // A1
  PA_3,  NULL, NULL,     // A2
  PA_2,  NULL, NULL,     // A3
  PB_9,  NULL, NULL,     // A4/SDA
  PB_8,  NULL, NULL,     // A5/SCL
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
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

#define STM32_UUID ((uint32_t *)0x1FF80050)

uint8_t getUniqueSerialNumber(uint8_t* name) {
  utox8(STM32_UUID[0], &name[0]);
  utox8(STM32_UUID[1], &name[16]);
  return 32;
}

void _ontouch1200bps_() {
}

#endif