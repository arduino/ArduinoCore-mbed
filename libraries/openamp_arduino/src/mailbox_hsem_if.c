/**
  ******************************************************************************
  * File Name          : mailbox_hsem_if.c
  * Description        : This file provides code for the configuration
  *                      of the mailbox based on hardware semaphore.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "openamp/open_amp.h"
#include "stm32h7xx_hal.h"
#include "openamp_conf.h"
#include "mailbox_hsem_if.h"
//#include "metal/atomic.h"
//#include "metal/device.h"

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* Private define ------------------------------------------------------------*/
#define RX_NO_MSG           0
#define RX_NEW_MSG          1

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t msg_received = RX_NO_MSG;

#include "cmsis_os.h"
extern osThreadId eventHandlerThreadId;

/* Private functions ---------------------------------------------------------*/
void HAL_HSEM_FreeCallback(uint32_t SemMask)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(SemMask);
  msg_received = RX_NEW_MSG;

#ifdef CORE_CM7
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_1));   
#endif
#ifdef CORE_CM4
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));   
#endif

  osSignalSet(eventHandlerThreadId, 0x1);
}

/**
  * @brief  Initialize MAILBOX with IPCC peripheral
  * @param  None
  * @retval : Operation result
  */
int MAILBOX_Init(void)
{
  __HAL_RCC_HSEM_CLK_ENABLE();

#ifdef CORE_CM7
  /* Enable CM7 receive irq */
  HAL_NVIC_SetPriority(HSEM1_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(HSEM1_IRQn);
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_1));    
#endif        
#ifdef CORE_CM4 
  /* Enable CM4 receive irq */
  HAL_NVIC_SetPriority(HSEM2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(HSEM2_IRQn);
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));    
#endif

  return 0;
}

/**
  * @brief  Initialize MAILBOX with IPCC peripheral
  * @param  virtio device
  * @retval : Operation result
  */
int MAILBOX_Poll(struct virtio_device *vdev)
{
  /* If we got an interrupt, ask for the corresponding virtqueue processing */

  if (msg_received == RX_NEW_MSG)
  {
#ifdef CORE_CM7   
    rproc_virtio_notified(vdev, VRING0_ID);
#endif                
#ifdef CORE_CM4   
    rproc_virtio_notified(vdev, VRING1_ID);
#endif                
    msg_received = RX_NO_MSG;
    return 0;
  }

  return -EAGAIN;
}

/**
  * @brief  Callback function called by OpenAMP MW to notify message processing
  * @param  VRING id
  * @retval Operation result
  */
int MAILBOX_Notify(void *priv, uint32_t id)
{
   (void)priv;
   (void)id;

#ifdef CORE_CM7 
  HAL_HSEM_FastTake(HSEM_ID_0); 
  HAL_HSEM_Release(HSEM_ID_0,0);
#endif                
#ifdef CORE_CM4   
  HAL_HSEM_FastTake(HSEM_ID_1); 
  HAL_HSEM_Release(HSEM_ID_1,0);
#endif  

  return 0;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
