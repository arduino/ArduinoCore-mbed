#include "Arduino.h"
#include "pinDefinitions.h"

RTC_HandleTypeDef RTCHandle;

AnalogPinDescription g_AAnalogPinDescription[] = {
  { PA_0C,        NULL },    // A0    ADC2_INP0
  { PA_1C,        NULL },    // A1    ADC2_INP1
  { PC_2C,        NULL },    // A2    ADC3_INP0
  { PC_3C,        NULL },    // A3    ADC3_INP1
  { PC_2_ALT0,    NULL },    // A4    ADC1_INP12
  { PC_3_ALT2,    NULL },    // A5    ADC2_INP13
  { PA_4,         NULL },    // A6    ADC1_INP18
  { PA_6,         NULL }     // A7    ADC1_INP7
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
  { PA_6,         NULL, NULL, NULL },    // A7    ADC1_INP7

  // LEDS
  { PK_5,         NULL, NULL, NULL },    // LEDR
  { PK_6,         NULL, NULL, NULL },    // LEDG
  { PK_7,         NULL, NULL, NULL },    // LEDB

  { PA_0,         NULL, NULL, NULL },
  { PA_1,         NULL, NULL, NULL },
  { PA_2,         NULL, NULL, NULL },
  { PA_3,         NULL, NULL, NULL },
  { PA_4,         NULL, NULL, NULL },
  { PA_5,         NULL, NULL, NULL },
  { PA_6,         NULL, NULL, NULL },
  { PA_7,         NULL, NULL, NULL },
  { PA_8,         NULL, NULL, NULL },
  { PA_9,         NULL, NULL, NULL },
  { PA_10,        NULL, NULL, NULL },
  { PA_11,        NULL, NULL, NULL },
  { PA_12,        NULL, NULL, NULL },
  { PA_13,        NULL, NULL, NULL },
  { PA_14,        NULL, NULL, NULL },
  { PA_15,        NULL, NULL, NULL },
  { PB_0,         NULL, NULL, NULL },
  { PB_1,         NULL, NULL, NULL },
  { PB_2,         NULL, NULL, NULL },
  { PB_3,         NULL, NULL, NULL },
  { PB_4,         NULL, NULL, NULL },
  { PB_5,         NULL, NULL, NULL },
  { PB_6,         NULL, NULL, NULL },
  { PB_7,         NULL, NULL, NULL },
  { PB_8,         NULL, NULL, NULL },
  { PB_9,         NULL, NULL, NULL },
  { PB_10,        NULL, NULL, NULL },
  { PB_11,        NULL, NULL, NULL },
  { PB_12,        NULL, NULL, NULL },
  { PB_13,        NULL, NULL, NULL },
  { PB_14,        NULL, NULL, NULL },
  { PB_15,        NULL, NULL, NULL },
  { PC_0,         NULL, NULL, NULL },
  { PC_1,         NULL, NULL, NULL },
  { PC_2,         NULL, NULL, NULL },
  { PC_3,         NULL, NULL, NULL },
  { PC_4,         NULL, NULL, NULL },
  { PC_5,         NULL, NULL, NULL },
  { PC_6,         NULL, NULL, NULL },
  { PC_7,         NULL, NULL, NULL },
  { PC_8,         NULL, NULL, NULL },
  { PC_9,         NULL, NULL, NULL },
  { PC_10,        NULL, NULL, NULL },
  { PC_11,        NULL, NULL, NULL },
  { PC_12,        NULL, NULL, NULL },
  { PC_13,        NULL, NULL, NULL },
  { PC_14,        NULL, NULL, NULL },
  { PC_15,        NULL, NULL, NULL },
  { PD_0,         NULL, NULL, NULL },
  { PD_1,         NULL, NULL, NULL },
  { PD_2,         NULL, NULL, NULL },
  { PD_3,         NULL, NULL, NULL },
  { PD_4,         NULL, NULL, NULL },
  { PD_5,         NULL, NULL, NULL },
  { PD_6,         NULL, NULL, NULL },
  { PD_7,         NULL, NULL, NULL },
  { PD_8,         NULL, NULL, NULL },
  { PD_9,         NULL, NULL, NULL },
  { PD_10,        NULL, NULL, NULL },
  { PD_11,        NULL, NULL, NULL },
  { PD_12,        NULL, NULL, NULL },
  { PD_13,        NULL, NULL, NULL },
  { PD_14,        NULL, NULL, NULL },
  { PD_15,        NULL, NULL, NULL },
  { PE_0,         NULL, NULL, NULL },
  { PE_1,         NULL, NULL, NULL },
  { PE_2,         NULL, NULL, NULL },
  { PE_3,         NULL, NULL, NULL },
  { PE_4,         NULL, NULL, NULL },
  { PE_5,         NULL, NULL, NULL },
  { PE_6,         NULL, NULL, NULL },
  { PE_7,         NULL, NULL, NULL },
  { PE_8,         NULL, NULL, NULL },
  { PE_9,         NULL, NULL, NULL },
  { PE_10,        NULL, NULL, NULL },
  { PE_11,        NULL, NULL, NULL },
  { PE_12,        NULL, NULL, NULL },
  { PE_13,        NULL, NULL, NULL },
  { PE_14,        NULL, NULL, NULL },
  { PE_15,        NULL, NULL, NULL },
  { PF_0,         NULL, NULL, NULL },
  { PF_1,         NULL, NULL, NULL },
  { PF_2,         NULL, NULL, NULL },
  { PF_3,         NULL, NULL, NULL },
  { PF_4,         NULL, NULL, NULL },
  { PF_5,         NULL, NULL, NULL },
  { PF_6,         NULL, NULL, NULL },
  { PF_7,         NULL, NULL, NULL },
  { PF_8,         NULL, NULL, NULL },
  { PF_9,         NULL, NULL, NULL },
  { PF_10,        NULL, NULL, NULL },
  { PF_11,        NULL, NULL, NULL },
  { PF_12,        NULL, NULL, NULL },
  { PF_13,        NULL, NULL, NULL },
  { PF_14,        NULL, NULL, NULL },
  { PF_15,        NULL, NULL, NULL },
  { PG_0,         NULL, NULL, NULL },
  { PG_1,         NULL, NULL, NULL },
  { PG_2,         NULL, NULL, NULL },
  { PG_3,         NULL, NULL, NULL },
  { PG_4,         NULL, NULL, NULL },
  { PG_5,         NULL, NULL, NULL },
  { PG_6,         NULL, NULL, NULL },
  { PG_7,         NULL, NULL, NULL },
  { PG_8,         NULL, NULL, NULL },
  { PG_9,         NULL, NULL, NULL },
  { PG_10,        NULL, NULL, NULL },
  { PG_11,        NULL, NULL, NULL },
  { PG_12,        NULL, NULL, NULL },
  { PG_13,        NULL, NULL, NULL },
  { PG_14,        NULL, NULL, NULL },
  { PG_15,        NULL, NULL, NULL },
  { PH_0,         NULL, NULL, NULL },
  { PH_1,         NULL, NULL, NULL },
  { PH_2,         NULL, NULL, NULL },
  { PH_3,         NULL, NULL, NULL },
  { PH_4,         NULL, NULL, NULL },
  { PH_5,         NULL, NULL, NULL },
  { PH_6,         NULL, NULL, NULL },
  { PH_7,         NULL, NULL, NULL },
  { PH_8,         NULL, NULL, NULL },
  { PH_9,         NULL, NULL, NULL },
  { PH_10,        NULL, NULL, NULL },
  { PH_11,        NULL, NULL, NULL },
  { PH_12,        NULL, NULL, NULL },
  { PH_13,        NULL, NULL, NULL },
  { PH_14,        NULL, NULL, NULL },
  { PH_15,        NULL, NULL, NULL },
  { PI_0,         NULL, NULL, NULL },
  { PI_1,         NULL, NULL, NULL },
  { PI_2,         NULL, NULL, NULL },
  { PI_3,         NULL, NULL, NULL },
  { PI_4,         NULL, NULL, NULL },
  { PI_5,         NULL, NULL, NULL },
  { PI_6,         NULL, NULL, NULL },
  { PI_7,         NULL, NULL, NULL },
  { PI_8,         NULL, NULL, NULL },
  { PI_9,         NULL, NULL, NULL },
  { PI_10,        NULL, NULL, NULL },
  { PI_11,        NULL, NULL, NULL },
  { PI_12,        NULL, NULL, NULL },
  { PI_13,        NULL, NULL, NULL },
  { PI_14,        NULL, NULL, NULL },
  { PI_15,        NULL, NULL, NULL },
  { PJ_0,         NULL, NULL, NULL },
  { PJ_1,         NULL, NULL, NULL },
  { PJ_2,         NULL, NULL, NULL },
  { PJ_3,         NULL, NULL, NULL },
  { PJ_4,         NULL, NULL, NULL },
  { PJ_5,         NULL, NULL, NULL },
  { PJ_6,         NULL, NULL, NULL },
  { PJ_7,         NULL, NULL, NULL },
  { PJ_8,         NULL, NULL, NULL },
  { PJ_9,         NULL, NULL, NULL },
  { PJ_10,        NULL, NULL, NULL },
  { PJ_11,        NULL, NULL, NULL },
  { PJ_12,        NULL, NULL, NULL },
  { PJ_13,        NULL, NULL, NULL },
  { PJ_14,        NULL, NULL, NULL },
  { PJ_15,        NULL, NULL, NULL },
  { PK_0,         NULL, NULL, NULL },
  { PK_1,         NULL, NULL, NULL },
  { PK_2,         NULL, NULL, NULL },
  { PK_3,         NULL, NULL, NULL },
  { PK_4,         NULL, NULL, NULL },
  { PK_5,         NULL, NULL, NULL },
  { PK_6,         NULL, NULL, NULL },
  { PK_7,         NULL, NULL, NULL },
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
