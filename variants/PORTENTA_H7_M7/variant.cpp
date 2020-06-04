#include "Arduino.h"

RTC_HandleTypeDef RTCHandle;

AnalogPinDescription g_AAnalogPinDescription[] = {
  { PA_0_C_ALT2,  NULL },    // A0    ADC2_INP0
  { PA_1_C_ALT0,  NULL },    // A1    ADC2_INP1
  { PC_2_C_ALT0,  NULL },    // A2    ADC3_INP0
  { PC_3_C,       NULL },    // A3    ADC3_INP1
  { PC_2_ALT0,    NULL },    // A4    ADC1_INP12
  { PC_3_ALT2,    NULL },    // A5    ADC2_INP13
  { PA_4,         NULL }     // A6    ADC1_INP18
};

PinDescription g_APinDescription[] = {
  // D0 - D7
  { PH_15,        NULL, NULL, NULL },    // D0
  { PK_1,         NULL, NULL, NULL },    // D1
  { PJ_11,        NULL, NULL, NULL },    // D2
  { PG_7,         NULL, NULL, NULL },    // D3
  { PC_7,         NULL, NULL, NULL },    // D4
  { PC_6,         NULL, NULL, NULL },    // D5
  { PA_8,         NULL, NULL, NULL },    // D6
  { PI_0,         NULL, NULL, NULL },    // D7

  // D8 - D14
  { PC_3,         NULL, NULL, NULL },    // D8
  { PI_1,         NULL, NULL, NULL },    // D9
  { PC_2,         NULL, NULL, NULL },    // D10
  { PH_8,         NULL, NULL, NULL },    // D11
  { PH_7,         NULL, NULL, NULL },    // D12
  { PA_10,        NULL, NULL, NULL },    // D13
  { PA_9,         NULL, NULL, NULL },    // D14

  // A0 - A7
  { PA_0_C_ALT2,  NULL, NULL, NULL },    // A0    ADC2_INP0
  { PA_1_C_ALT0,  NULL, NULL, NULL },    // A1    ADC2_INP1
  { PC_2_C_ALT0,  NULL, NULL, NULL },    // A2    ADC3_INP0
  { PC_3_C,       NULL, NULL, NULL },    // A3    ADC3_INP1
  { PC_2_ALT0,    NULL, NULL, NULL },    // A4    ADC1_INP12
  { PC_3_ALT2,    NULL, NULL, NULL },    // A5    ADC2_INP13
  { PA_4,         NULL, NULL, NULL },    // A6    ADC1_INP18

  // LEDS
  { PK_5,         NULL, NULL, NULL },    // LEDR
  { PK_6,         NULL, NULL, NULL },    // LEDG
  { PK_7,         NULL, NULL, NULL },    // LEDB
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}

void initVariant() {
  RTCHandle.Instance = RTC;
  // Turn off LED from bootloader
  pinMode(PK_6, OUTPUT);
  digitalWrite(PK_6, HIGH);

  // configure analog mux to split Pxy and Pxy_C
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_OPEN);
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_OPEN);
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_OPEN);
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
