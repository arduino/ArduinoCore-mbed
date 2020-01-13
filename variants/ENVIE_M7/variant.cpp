#include "Arduino.h"

RTC_HandleTypeDef RTCHandle;

PinDescription g_APinDescription[] = {
  // D0 - D7
  PH_15,  NULL, NULL,     // D0
  PK_1, NULL, NULL,     // D1
  PJ_11, NULL, NULL,     // D2
  PG_7, NULL, NULL,     // D3
  PC_7, NULL, NULL,     // D4
  PC_6, NULL, NULL,     // D5
  PA_8, NULL, NULL,     // D6
  PI_0,  NULL, NULL,     // D7

  // D8 - D14
  PC_3, NULL, NULL,     // D8
  PI_2, NULL, NULL,     // D9
  PC_2,  NULL, NULL,     // D10
  PH_8,  NULL, NULL,     // D11
  PH_7,  NULL, NULL,     // D12
  PA_10, NULL, NULL,     // D13
  PA_9, NULL, NULL,     // D14

  // A0 - A7
  PA_0_ALT0,  NULL, NULL,     // A0
  PA_1_ALT0,  NULL, NULL,     // A1
  PC_2_ALT0, NULL, NULL,     // A2
  //PC_3_ALT0, NULL, NULL,     // A3
  PC_3, NULL, NULL,     // A3       // FIXME: this is wrong
  PC_2, NULL, NULL,     // A4
  PC_3,  NULL, NULL,     // A5
  PA_4, NULL, NULL,     // A6
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}

void initVariant() {
	RTCHandle.Instance = RTC;
	// Turn off LED red from bootloader
	digitalWrite(PK_6, HIGH);
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
  utox8(HAL_GetUIDw0(), &name[0]);
  utox8(HAL_GetUIDw1(), &name[16]);
  return 32;
}

void _ontouch1200bps_() {
  HAL_RTCEx_BKUPWrite(&RTCHandle, RTC_BKP_DR0, 0xDF59);
  NVIC_SystemReset();
}

#endif