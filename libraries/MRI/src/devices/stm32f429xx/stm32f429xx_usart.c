/* Copyright 2015 Adam Green     (http://mbed.org/users/AdamGreen/)
   Copyright 2015 Chang,Jia-Rung (https://github.com/JaredCJR)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Routines used to provide STM32F429xx USART functionality to the mri debugger. */
#include <string.h>
#include <stdlib.h>
#include "core/platforms.h"
#include "../../architectures/armv7-m/debug_cm3.h"
#include "stm32f429xx_init.h"
#include "stm32f429xx_usart.h"

UART_HandleTypeDef huart;

void __mriExceptionHandler(void); /* armv7-m_asm.S */

void __mriStm32f429xxUart_Init(Token *pParameterTokens)
{
    huart.Instance = USART1;

    NVIC_DisableIRQ(USART1_IRQn);

    __HAL_UART_DISABLE_IT(&huart, UART_IT_TXE);
    __HAL_UART_DISABLE_IT(&huart, UART_IT_RXNE);

    NVIC_SetVector(USART1_IRQn,           (uint32_t)&__mriExceptionHandler);
    /* HardFault, MemoryManagement, BusFault and UsageFauls ISRs are all
     * handled defined within the mbed library. Therefore redefining of
     * the HardFault ISR symbol and branch to __mriExceptionHandler will
     * not work, however, we can set the target of a interrupt exception
     * via NVIC_SetVector(...);
     */
    //NVIC_SetVector(HardFault_IRQn,        (uint32_t)&__mriExceptionHandler);
    //NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)&__mriExceptionHandler);
    //NVIC_SetVector(BusFault_IRQn,         (uint32_t)&__mriExceptionHandler);
    //NVIC_SetVector(UsageFault_IRQn,       (uint32_t)&__mriExceptionHandler);

    /* USART1 has the top priority in the system */
    NVIC_SetPriority(USART1_IRQn,           0);
    //NVIC_SetPriority(HardFault_IRQn,        1);
    //NVIC_SetPriority(MemoryManagement_IRQn, 1);
    //NVIC_SetPriority(BusFault_IRQn,         1);
    //NVIC_SetPriority(UsageFault_IRQn,       1);

    __HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);
    NVIC_EnableIRQ(USART1_IRQn);
}

uint32_t Platform_CommHasReceiveData(void)
{
    return (huart.Instance->ISR & USART_ISR_RXNE_RXFNE);
}

int Platform_CommReceiveChar(void)
{
    while(!Platform_CommHasReceiveData()) { /* busy wait */ }
    return (huart.Instance->RDR & 0x1FF);
}

void Platform_CommSendChar(int Character)
{
    while (!(huart.Instance->ISR & USART_ISR_TXE_TXFNF)) { /* busy wait */  }
    huart.Instance->TDR = (Character & 0x1FF);
}

int Platform_CommCausedInterrupt(void)
{
    int interruptSource = (int)getCurrentlyExecutingExceptionNumber() - 16;
    return (interruptSource == USART1_IRQn);
}

void Platform_CommClearInterrupt(void)
{
    /* Clear UART receive interrupt flag, to avoid infinit loop in USARTx_Handler */
    huart.Instance->RQR |= USART_RQR_RXFRQ;
}

int Platform_CommSharingWithApplication(void)
{
    return 0;
}

static int isManualBaudRate(void)
{
    return 1;
}

int Platform_CommShouldWaitForGdbConnect(void)
{
    return !isManualBaudRate() && !Platform_CommSharingWithApplication();
}

int Platform_CommIsWaitingForGdbToConnect(void)
{
    return 0;
}

void Platform_CommPrepareToWaitForGdbConnection(void)
{
    /* stm32f429 does not support auto-baudrate */
    return;
}


void Platform_CommWaitForReceiveDataToStop(void)
{
    /* stm32f429 does not support auto-baudrate */
    return;
}
