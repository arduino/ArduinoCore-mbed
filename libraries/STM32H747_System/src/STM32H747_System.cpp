#include "STM32H747_System.h"
#include "Wire.h"

#define PMIC_ADDRESS 0x08

extern RTC_HandleTypeDef RTCHandle;

uint8_t STM32H747::readReg(uint8_t subAddress) {
  Wire1.beginTransmission(PMIC_ADDRESS);
  Wire1.write(subAddress);
  Wire1.endTransmission(false);
  Wire1.requestFrom(PMIC_ADDRESS, 1);
  while(!Wire1.available()) {}
  uint8_t ret= Wire1.read();
  Serial.println(ret, HEX);
  return ret;
}

void STM32H747::setRegister(uint8_t reg, uint8_t val) {
  Wire1.beginTransmission(PMIC_ADDRESS);
  Wire1.write(reg);
  Wire1.write(val);
  Wire1.endTransmission();
}

uint32_t STM32H747::readBackupRegister(RTCBackup reg) {
  return HAL_RTCEx_BKUPRead(&RTCHandle, (uint32_t)reg);
}

void STM32H747::writeBackupRegister(RTCBackup reg, uint32_t data) {
  HAL_RTCEx_BKUPWrite(&RTCHandle, (uint32_t)reg, data);
}

reset_reason_t STM32H747::getResetReason() {
  return (reset_reason_t)readBackupRegister(RTCBackup::DR8);
}

/*
 * This function disables the external oscillators and use the HSI instead.
 * If lowspeed = true : f = 100MHz
 * If lowspeed = false: f = 400MHz
*/
bool STM32H747::useInternalOscillator(bool lowspeed) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  // If we are reconfiguring the clock, select CSI as system clock source to allow modification of the PLL configuration 
  if (__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSE) {
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    //RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_CSI;
    RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_HSE;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
      return false;
    }
  }
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  //RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  //RCC_OscInitStruct.HSICalibrationValue  = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  if (lowspeed) {
    RCC_OscInitStruct.PLL.PLLN = 50;
  } else {
    RCC_OscInitStruct.PLL.PLLN = 200;
  }

  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 10;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    return false;
  }
  

  /* Select PLL as system clock source and configure bus clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_D3PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    return false;
  }

  /* -4- Optional: Disable HSE  Oscillator (if the HSE  is no more needed by the application) */
  RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState        = RCC_HSE_OFF;
    RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_NONE;

  if (lowspeed) {
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  } else {
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  }

  pinMode(PH_1, OUTPUT);
  digitalWrite(PH_1, LOW);

  return true;
}
