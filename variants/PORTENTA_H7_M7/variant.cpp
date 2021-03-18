#include "Arduino.h"

RTC_HandleTypeDef RTCHandle;

AnalogPinDescription g_AAnalogPinDescription[] = {
  { PA_0C,        NULL },    // A0    ADC2_INP0
  { PA_1C,        NULL },    // A1    ADC2_INP1
  { PC_2C,        NULL },    // A2    ADC3_INP0
  { PC_3C,        NULL },    // A3    ADC3_INP1
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

  // A0 - A6
  { PA_0C,        NULL, NULL, NULL },    // A0    ADC2_INP0
  { PA_1C,        NULL, NULL, NULL },    // A1    ADC2_INP1
  { PC_2C,        NULL, NULL, NULL },    // A2    ADC3_INP0
  { PC_3C,        NULL, NULL, NULL },    // A3    ADC3_INP1
  { PC_2_ALT0,    NULL, NULL, NULL },    // A4    ADC1_INP12
  { PC_3_ALT0,    NULL, NULL, NULL },    // A5    ADC1_INP13
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

#include "drivers/I2C.h"

void fixup3V1Rail() {
  mbed::I2C i2c(PB_7, PB_6);
  char data[2];
  data[0]=0x42;
  data[1]=(1);
  i2c.write(8 << 1, data, sizeof(data));
}

void initVariant() {
  RTCHandle.Instance = RTC;
  // Turn off LED from bootloader
  pinMode(PK_6, OUTPUT);
  digitalWrite(PK_6, HIGH);
  fixup3V1Rail();
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

#if 0
  // This address need to be in range 0x10000000-0x3FFF0000 to be usable by the M4 as a trampoline
  uint32_t  __attribute__((aligned(0x10000))) trampoline[2];
  static const uint32_t RAM_BASE_FOR_TRAMPOLINE = (uint32_t)&trampoline[0];

#if 0

  // This snippet MUST be executed BEFORE calling bootM4()
  // The only purpose it to fread() a file into CM4_BINARY_START location

  SDRAM.begin(0);

  // Copy M4 firmware to SDRAM
  FILE* fw = fopen("/fs/fw.bin", "r");
  if (fw == NULL) {
    while (1) {
      Serial.println("Please copy a firmware for M4 core in the PORTENTA mass storage");
      delay(100);
    }
  }
  fread((uint8_t*)CM4_BINARY_START, getFileSize(fw), 1, fw);
  fclose(fw);
#endif

  // We need to call this in case we want to use BACKUP_SRAM as trampoline
  HAL_PWR_EnableBkUpAccess();

  // Copy first 2 words of the firmware in trampoline location
  memcpy((void*)RAM_BASE_FOR_TRAMPOLINE, (void*)CM4_BINARY_START, 8);

  SCB_CleanDCache();

  // Set CM4_BOOT0 address
  // This actually writes a flash register and thus is persistent across reboots
  // RAM_BASE_FOR_TRAMPOLINE must be aligned to 0x10000 barrier
  LL_SYSCFG_SetCM4BootAddress0(RAM_BASE_FOR_TRAMPOLINE >> 16);
#else
  // Classic boot, just set the address and we are ready to go
  LL_SYSCFG_SetCM4BootAddress0(CM4_BINARY_START >> 16);
  LL_RCC_ForceCM4Boot();
#endif
}

#endif
