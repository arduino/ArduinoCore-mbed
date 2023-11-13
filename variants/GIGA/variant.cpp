#include "Arduino.h"
#include "pinDefinitions.h"

RTC_HandleTypeDef RTCHandle;

AnalogPinDescription g_AAnalogPinDescription[] = {
  { PC_4,         NULL },    // A0   ADC1_INP4
  { PC_5,         NULL },    // A1   ADC1_INP8
  { PB_0,         NULL },    // A2   ADC1_INP9
  { PB_1,         NULL },    // A3   ADC1_INP5
  { PC_3,         NULL },    // A4   ADC1_INP13
  { PC_2,         NULL },    // A5   ADC1_INP12
  { PC_0,         NULL },    // A6   ADC1_INP10
  { PA_0,         NULL },    // A7   ADC1_INP16
  { PC_2C,        NULL },    // A8   ADC3_INP0
  { PC_3C,        NULL },    // A9   ADC3_INP1
  { PA_1C,        NULL },    // A10  ADC2_INP1
  { PA_0C,        NULL },    // A11  ADC2_INP0
  { PA_4,         NULL },    // A12  DAC1_OUT1
  { PA_5,         NULL },    // A13  DAC1_OUT2
};

AnalogPinDescription g_AAnalogOutPinDescription[] = {
  { PA_4,         NULL },    // A12  DAC1_OUT1
  { PA_5,         NULL },    // A13  DAC1_OUT2
};

PinDescription g_APinDescription[] = {
  // D0 - D21
  { PB_7,         NULL, NULL, NULL },    // D0  RX
  { PA_9,         NULL, NULL, NULL },    // D1  TX
  { PA_3,         NULL, NULL, NULL },    // D2
  { PA_2,         NULL, NULL, NULL },    // D3
  { PJ_8,         NULL, NULL, NULL },    // D4
  { PA_7,         NULL, NULL, NULL },    // D5
  { PD_13,        NULL, NULL, NULL },    // D6
  { PB_4,         NULL, NULL, NULL },    // D7
  { PB_8,         NULL, NULL, NULL },    // D8
  { PB_9,         NULL, NULL, NULL },    // D9
  { PK_1,         NULL, NULL, NULL },    // D10
  { PJ_10,        NULL, NULL, NULL },    // D11
  { PJ_11,        NULL, NULL, NULL },    // D12
  { PH_6,         NULL, NULL, NULL },    // D13
  { PG_14,        NULL, NULL, NULL },    // D14  TX3
  { PC_7,         NULL, NULL, NULL },    // D15  RX3
  { PH_13,        NULL, NULL, NULL },    // D16  TX2
  { PI_9,         NULL, NULL, NULL },    // D17  RX2
  { PD_5,         NULL, NULL, NULL },    // D18  TX1
  { PD_6,         NULL, NULL, NULL },    // D19  RX1
  { PB_11,        NULL, NULL, NULL },    // D20  SDA
  { PH_4,         NULL, NULL, NULL },    // D21  SCL

  // D22 - D53
  { PJ_12,        NULL, NULL, NULL },    // D22
  { PG_13,        NULL, NULL, NULL },    // D23
  { PG_12,        NULL, NULL, NULL },    // D24
  { PJ_0,         NULL, NULL, NULL },    // D25
  { PJ_14,        NULL, NULL, NULL },    // D26
  { PJ_1,         NULL, NULL, NULL },    // D27
  { PJ_15,        NULL, NULL, NULL },    // D28
  { PJ_2,         NULL, NULL, NULL },    // D29
  { PK_3,         NULL, NULL, NULL },    // D30
  { PJ_3,         NULL, NULL, NULL },    // D31
  { PK_4,         NULL, NULL, NULL },    // D32
  { PJ_4,         NULL, NULL, NULL },    // D33
  { PK_5,         NULL, NULL, NULL },    // D34
  { PJ_5,         NULL, NULL, NULL },    // D35
  { PK_6,         NULL, NULL, NULL },    // D36
  { PJ_6,         NULL, NULL, NULL },    // D37
  { PJ_7,         NULL, NULL, NULL },    // D38
  { PI_14,        NULL, NULL, NULL },    // D39
  { PE_6,         NULL, NULL, NULL },    // D40
  { PK_7,         NULL, NULL, NULL },    // D41
  { PI_15,        NULL, NULL, NULL },    // D42
  { PI_10,        NULL, NULL, NULL },    // D43
  { PG_10,        NULL, NULL, NULL },    // D44
  { PI_13,        NULL, NULL, NULL },    // D45
  { PH_15,        NULL, NULL, NULL },    // D46
  { PB_2,         NULL, NULL, NULL },    // D47
  { PK_0,         NULL, NULL, NULL },    // D48
  { PE_4,         NULL, NULL, NULL },    // D49
  { PI_11,        NULL, NULL, NULL },    // D50
  { PE_5,         NULL, NULL, NULL },    // D51
  { PK_2,         NULL, NULL, NULL },    // D52
  { PG_7,         NULL, NULL, NULL },    // D53

  // D54 - D67
  { PI_5,         NULL, NULL, NULL },    // D54
  { PH_8,         NULL, NULL, NULL },    // D55
  { PA_6,         NULL, NULL, NULL },    // D56
  { PJ_9,         NULL, NULL, NULL },    // D57
  { PI_7,         NULL, NULL, NULL },    // D58
  { PI_6,         NULL, NULL, NULL },    // D59
  { PI_4,         NULL, NULL, NULL },    // D60
  { PH_14,        NULL, NULL, NULL },    // D61
  { PG_11,        NULL, NULL, NULL },    // D62
  { PH_11,        NULL, NULL, NULL },    // D63
  { PH_10,        NULL, NULL, NULL },    // D64
  { PH_9,         NULL, NULL, NULL },    // D65
  { PA_1,         NULL, NULL, NULL },    // D66
  { PD_4,         NULL, NULL, NULL },    // D67

  // D68 - D74
  { PC_6,         NULL, NULL, NULL },    // D68
  { PI_0,         NULL, NULL, NULL },    // D69
  { PI_1,         NULL, NULL, NULL },    // D70
  { PI_2,         NULL, NULL, NULL },    // D71
  { PI_3,         NULL, NULL, NULL },    // D72
  { PC_1,         NULL, NULL, NULL },    // D73
  { PB_12,        NULL, NULL, NULL },    // D74
  { PD_3,         NULL, NULL, NULL },    // D75

  // A0 - A9
  { PC_4,         NULL, NULL, NULL },    // A0 (D76)
  { PC_5,         NULL, NULL, NULL },    // A1 (D77)
  { PB_0,         NULL, NULL, NULL },    // A2 (D78)
  { PB_1,         NULL, NULL, NULL },    // A3 (D79)
  { PC_3,         NULL, NULL, NULL },    // A4 (D80)
  { PC_2,         NULL, NULL, NULL },    // A5 (D81)
  { PC_0,         NULL, NULL, NULL },    // A6 (D82)
  { PA_0,         NULL, NULL, NULL },    // A7 (D83)
  { PA_4,         NULL, NULL, NULL },    // A12  DAC0 (D84)
  { PA_5,         NULL, NULL, NULL },    // A13  DAC1 (D85)

  // LEDS
  { PI_12,        NULL, NULL, NULL },    // D86  LEDR
  { PJ_13,        NULL, NULL, NULL },    // D87  LEDG
  { PE_3,         NULL, NULL, NULL },    // D88  LEDB

  // SPI1
  { PG_9,         NULL, NULL, NULL },    // D89  MISO
  { PD_7,         NULL, NULL, NULL },    // D90  MOSI
  { PB_3,         NULL, NULL, NULL },    // D91  SCK

  // USB HOST ENABLE
  { PA_15,        NULL, NULL, NULL },    // D92  USB HOST ENABLE

  // CAN
  { PB_5,         NULL, NULL, NULL },    // D93  CAN RX
  { PB_13,        NULL, NULL, NULL },    // D94  CAN TX

  // WIFI CONTROL
  { PI_8,         NULL, NULL, NULL },    // D95  WIFI HOST WAKE
  { PB_10,        NULL, NULL, NULL },    // D96  WIFI ON

  // BLE CONTROL
  { PH_7,         NULL, NULL, NULL },    // D97  BLE DEVICE WAKE
  { PG_3,         NULL, NULL, NULL },    // D98  BLE HOST WAKE
  { PA_10,        NULL, NULL, NULL },    // D99  BLE ON

  // BOOT
  { PC_13,        NULL, NULL, NULL },    // D100 BOOT0

  // I2C1
  { PB_6_ALT0,    NULL, NULL, NULL },    // D101  SCL1
  { PH_12,        NULL, NULL, NULL },    // D102  SDA1
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}

void initVariant() {
  RTCHandle.Instance = RTC;
  // Turn off LED from bootloader
  pinMode(LEDG, OUTPUT);
  digitalWrite(LEDG, HIGH);
  // Disable the FMC bank1 (enabled after reset)
  // See https://github.com/STMicroelectronics/STM32CubeH7/blob/beced99ac090fece04d1e0eb6648b8075e156c6c/Projects/STM32H747I-DISCO/Applications/OpenAMP/OpenAMP_RTOS_PingPong/Common/Src/system_stm32h7xx.c#L215
  FMC_Bank1_R->BTCR[0] = 0x000030D2;
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
  utox8(HAL_GetUIDw2(), &name[32]);
  return 48;
}

void _ontouch1200bps_() {
  HAL_RTCEx_BKUPWrite(&RTCHandle, RTC_BKP_DR0, 0xDF59);
  NVIC_SystemReset();
}

#include "stm32h7xx_ll_system.h"

void bootM4() {
  // Classic boot, just set the address and we are ready to go
  LL_SYSCFG_SetCM4BootAddress0(CM4_BINARY_START >> 16);
  LL_RCC_ForceCM4Boot();
}

#endif
