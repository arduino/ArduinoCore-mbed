/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Audio Python module.
 */
#if defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_GIGA)

#include <stdio.h>
#include "stm32h7xx_hal.h"
#include "audio.h"
#include "stdbool.h"
#include "Arduino.h"

//static DMA_HandleTypeDef hdma_dfsdm_right_rx;
//static DMA_HandleTypeDef hdma_dfsdm_left_rx;

volatile uint16_t *g_pcmbuf = NULL;

#define DMA_XFER_NONE   (0x00U)
#define DMA_XFER_HALF   (0x01U)
#define DMA_XFER_FULL   (0x04U)

static volatile uint32_t xfer_status = 0;

DFSDM_Filter_HandleTypeDef hdfsdm1_filter;
DFSDM_Channel_HandleTypeDef hdfsdm1_channel;
DMA_HandleTypeDef hdma_dfsdm1_flt;

static uint32_t HAL_RCC_DFSDM1_CLK_ENABLED=0;

static uint32_t DFSDM1_Init = 0;

static int attenuation = 5;

ALIGN_32BYTES(int32_t                      RecBuff[PDM_BUFFER_SIZE]);

#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))

void PDMIrqHandler(bool halftranfer);
void PDMsetBufferSize(int size);


void AUDIO_DFSDM1_DMA_IRQHandler(void)
{
    //HAL_DMA_IRQHandler(hdfsdm1_filter.hdmaReg);
    HAL_DMA_IRQHandler(&hdma_dfsdm1_flt);
}

void DFSDM1_FLT0_IRQHandler(void)
{
  /* USER CODE END DFSDM1_FLT0_IRQn 0 */
  HAL_DFSDM_IRQHandler(&hdfsdm1_filter);
}

/**
  * @brief  Half regular conversion complete callback. 
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
    if(hdfsdm_filter == &hdfsdm1_filter)
    {
        xfer_status |= DMA_XFER_HALF;
        // Invalidate Data Cache to get the updated content of the SRAM
        #ifdef CORE_CM7
        SCB_InvalidateDCache_by_Addr((uint32_t*)&RecBuff[0],sizeof(RecBuff)/2);
        #endif
    }
    PDMIrqHandler(true);
}

/**
  * @brief  Regular conversion complete callback. 
  * @note   In interrupt mode, user has to read conversion value in this function
            using HAL_DFSDM_FilterGetRegularValue.
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
    if(hdfsdm_filter == &hdfsdm1_filter)
    {
        xfer_status |= DMA_XFER_FULL;
        // Invalidate Data Cache to get the updated content of the SRAM
        #ifdef CORE_CM7
        SCB_InvalidateDCache_by_Addr((uint32_t*)&RecBuff[PDM_BUFFER_SIZE/2],sizeof(RecBuff)/2);
        #endif
    }
    PDMIrqHandler(false);
}


/**
* @brief DFSDM_Filter MSP Initialization
* This function configures the hardware resources used in this example
* @param hdfsdm_filter: DFSDM_Filter handle pointer
* @retval None
*/
void HAL_DFSDM_FilterMspInit(DFSDM_Filter_HandleTypeDef* hdfsdm_filter)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(DFSDM1_Init == 0)
  {
    /* Peripheral clock enable */
    HAL_RCC_DFSDM1_CLK_ENABLED++;
    if(HAL_RCC_DFSDM1_CLK_ENABLED==1){
      __HAL_RCC_DFSDM1_CLK_ENABLE();
    }

    AUDIO_DFSDM1_CK_CLK_ENABLE();
    GPIO_InitStruct.Pin = AUDIO_DFSDM1_CK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = AUDIO_DFSDM1_CK_AF;
    HAL_GPIO_Init(AUDIO_DFSDM1_CK_PORT, &GPIO_InitStruct);

    AUDIO_DFSDM1_D1_CLK_ENABLE();

    GPIO_InitStruct.Pin = AUDIO_DFSDM1_D1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = AUDIO_DFSDM1_D1_AF;
    HAL_GPIO_Init(AUDIO_DFSDM1_D1_PORT, &GPIO_InitStruct);

    /* DFSDM1 interrupt Init */
    HAL_NVIC_SetPriority(DFSDM1_FLT0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DFSDM1_FLT0_IRQn);
  }
  
    /* DFSDM1 DMA Init */
    /* DFSDM1_FLT0 Init */
  if(hdfsdm_filter->Instance == DFSDM1_Filter0){
    hdma_dfsdm1_flt.Instance = DMA1_Stream0;
    hdma_dfsdm1_flt.Init.Request = DMA_REQUEST_DFSDM1_FLT0;
    hdma_dfsdm1_flt.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dfsdm1_flt.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dfsdm1_flt.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dfsdm1_flt.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_dfsdm1_flt.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_dfsdm1_flt.Init.Mode = DMA_CIRCULAR;
    hdma_dfsdm1_flt.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dfsdm1_flt.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_DeInit(&hdma_dfsdm1_flt);
    if (HAL_DMA_Init(&hdma_dfsdm1_flt) != HAL_OK)
    {
      Error_Handler();
    }

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
    __HAL_LINKDMA(hdfsdm_filter,hdmaInj,hdma_dfsdm1_flt);
    __HAL_LINKDMA(hdfsdm_filter,hdmaReg,hdma_dfsdm1_flt);
  }

}

/**
* @brief DFSDM_Channel MSP Initialization
* This function configures the hardware resources used in this example
* @param hdfsdm_channel: DFSDM_Channel handle pointer
* @retval None
*/
void HAL_DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef* hdfsdm_channel)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(DFSDM1_Init == 0)
  {
    /* Peripheral clock enable */
    HAL_RCC_DFSDM1_CLK_ENABLED++;
    if(HAL_RCC_DFSDM1_CLK_ENABLED==1){
      __HAL_RCC_DFSDM1_CLK_ENABLE();
    }

    AUDIO_DFSDM1_CK_CLK_ENABLE();
    GPIO_InitStruct.Pin = AUDIO_DFSDM1_CK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = AUDIO_DFSDM1_CK_AF;
    HAL_GPIO_Init(AUDIO_DFSDM1_CK_PORT, &GPIO_InitStruct);

    AUDIO_DFSDM1_D1_CLK_ENABLE();

    GPIO_InitStruct.Pin = AUDIO_DFSDM1_D1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = AUDIO_DFSDM1_D1_AF;
    HAL_GPIO_Init(AUDIO_DFSDM1_D1_PORT, &GPIO_InitStruct);
  }

}

/**
* @brief DFSDM_Filter MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdfsdm_filter: DFSDM_Filter handle pointer
* @retval None
*/
void HAL_DFSDM_FilterMspDeInit(DFSDM_Filter_HandleTypeDef* hdfsdm_filter)
{
  DFSDM1_Init-- ;
  if(DFSDM1_Init == 0)
    {
    /* Peripheral clock disable */
    if (HAL_RCC_DFSDM1_CLK_ENABLED > 0) {
      __HAL_RCC_DFSDM1_CLK_DISABLE();
      HAL_RCC_DFSDM1_CLK_ENABLED--;
    }
  
    /**DFSDM1 GPIO Configuration*/
    HAL_GPIO_DeInit(AUDIO_DFSDM1_CK_PORT, AUDIO_DFSDM1_CK_PIN);
    HAL_GPIO_DeInit(AUDIO_DFSDM1_D1_PORT, AUDIO_DFSDM1_D1_PIN);

    /* DFSDM1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(DFSDM1_FLT0_IRQn);

    /* DFSDM1 DMA DeInit */
    HAL_DMA_DeInit(hdfsdm_filter->hdmaInj);
    HAL_DMA_DeInit(hdfsdm_filter->hdmaReg);
  }

}

/**
* @brief DFSDM_Channel MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdfsdm_channel: DFSDM_Channel handle pointer
* @retval None
*/
void HAL_DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef* hdfsdm_channel)
{
  DFSDM1_Init-- ;
  if(DFSDM1_Init == 0)
    {
    /* Peripheral clock disable */
    if (HAL_RCC_DFSDM1_CLK_ENABLED > 0) {
      __HAL_RCC_DFSDM1_CLK_DISABLE();
      HAL_RCC_DFSDM1_CLK_ENABLED--;
    }
  
    /**DFSDM1 GPIO Configuration*/
    HAL_GPIO_DeInit(AUDIO_DFSDM1_CK_PORT, AUDIO_DFSDM1_CK_PIN);
    HAL_GPIO_DeInit(AUDIO_DFSDM1_D1_PORT, AUDIO_DFSDM1_D1_PIN);
  }

}

void dfsdm_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    AUDIO_DFSDM1_CLK_ENABLE();
    AUDIO_DFSDM1_CK_CLK_ENABLE();
    AUDIO_DFSDM1_D1_CLK_ENABLE();

    GPIO_InitStruct.Pin = AUDIO_DFSDM1_CK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = AUDIO_DFSDM1_CK_AF;
    HAL_GPIO_Init(AUDIO_DFSDM1_CK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = AUDIO_DFSDM1_D1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = AUDIO_DFSDM1_D1_AF;
    HAL_GPIO_Init(AUDIO_DFSDM1_D1_PORT, &GPIO_InitStruct);
}

static int DFSDM_Init(uint32_t frequency)
{
    __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&hdfsdm1_channel);
    hdfsdm1_channel.Instance                      = AUDIO_DFSDM;
    hdfsdm1_channel.Init.OutputClock.Activation   = ENABLE;
    hdfsdm1_channel.Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    hdfsdm1_channel.Init.OutputClock.Divider      = AUDIO_DFSDM1_CLK_DIVIDER;  /* 2MHz */
    hdfsdm1_channel.Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hdfsdm1_channel.Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
    hdfsdm1_channel.Init.Input.Pins               = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
    hdfsdm1_channel.Init.SerialInterface.Type     = DFSDM_CHANNEL_SPI_RISING;
    hdfsdm1_channel.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hdfsdm1_channel.Init.Awd.FilterOrder          = DFSDM_CHANNEL_FASTSINC_ORDER;
    hdfsdm1_channel.Init.Awd.Oversampling         = 2000000/frequency; /* 2MHz/125 = 16kHz */
    hdfsdm1_channel.Init.Offset                   = 0;
    hdfsdm1_channel.Init.RightBitShift            = 0;
    if(HAL_OK != HAL_DFSDM_ChannelInit(&hdfsdm1_channel))
    {
        return 0;
    }

    __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&hdfsdm1_filter);
    hdfsdm1_filter.Instance                          = DFSDM1_Filter0;
    hdfsdm1_filter.Init.RegularParam.Trigger         = DFSDM_FILTER_SW_TRIGGER;
    hdfsdm1_filter.Init.RegularParam.FastMode        = ENABLE;
    hdfsdm1_filter.Init.RegularParam.DmaMode         = ENABLE;
    hdfsdm1_filter.Init.InjectedParam.Trigger        = DFSDM_FILTER_SINC3_ORDER;
    hdfsdm1_filter.Init.InjectedParam.ScanMode       = ENABLE;
    hdfsdm1_filter.Init.InjectedParam.DmaMode        = ENABLE;
    hdfsdm1_filter.Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
    hdfsdm1_filter.Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
    hdfsdm1_filter.Init.FilterParam.SincOrder        = DFSDM_FILTER_FASTSINC_ORDER;
    hdfsdm1_filter.Init.FilterParam.Oversampling     = 2000000/frequency; /* 2MHz/125 = 16kHz */
    hdfsdm1_filter.Init.FilterParam.IntOversampling  = 1;
    if(HAL_OK != HAL_DFSDM_FilterInit(&hdfsdm1_filter))
    {
        return 0;
    }

    if(HAL_OK != HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter, AUDIO_DFSDM1_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))
    {
        return 0;
    }

    // Enable the DMA clock
    AUDIO_DFSDM1_DMA_CLK_ENABLE();
    __HAL_RCC_DFSDM1_CLK_ENABLE();

    // Configure the DFSDM DMA
    hdma_dfsdm1_flt.Instance                 = AUDIO_DFSDM1_DMA_STREAM;
    hdma_dfsdm1_flt.Init.Request             = DMA_REQUEST_DFSDM1_FLT0;
    hdma_dfsdm1_flt.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_dfsdm1_flt.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_dfsdm1_flt.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_dfsdm1_flt.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dfsdm1_flt.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma_dfsdm1_flt.Init.Mode                = DMA_CIRCULAR;
    hdma_dfsdm1_flt.Init.Priority            = DMA_PRIORITY_HIGH;

    // Initialize the DMA stream
    HAL_DMA_DeInit(&hdma_dfsdm1_flt);
    if (HAL_DMA_Init(&hdma_dfsdm1_flt) != HAL_OK) {
        return 0;
    }

    __HAL_LINKDMA(&hdfsdm1_filter, hdmaInj, hdma_dfsdm1_flt);
    __HAL_LINKDMA(&hdfsdm1_filter, hdmaReg, hdma_dfsdm1_flt);


    return 1;
}

int py_audio_init(size_t channels, uint32_t frequency)
{
    RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;

    HAL_RCCEx_GetPeriphCLKConfig(&rcc_ex_clk_init_struct);

    rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_DFSDM1;
    rcc_ex_clk_init_struct.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_D2PCLK1;

    HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);

    dfsdm_init();

    if (!DFSDM_Init(frequency)) {
        return 0;
    }

    if (channels != 1) {
        return 0;
    }

    // Configure and enable DFSDM1 DMA IRQ Channel
    HAL_NVIC_SetPriority(AUDIO_DFSDM1_DMA_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
    HAL_NVIC_EnableIRQ(AUDIO_DFSDM1_DMA_IRQ);


    HAL_NVIC_SetPriority(DFSDM1_FLT0_IRQn, AUDIO_IN_IRQ_PREPRIO, 0);
    HAL_NVIC_EnableIRQ(DFSDM1_FLT0_IRQn);

    HAL_NVIC_SetPriority(DFSDM1_FLT1_IRQn, AUDIO_IN_IRQ_PREPRIO, 0);
    HAL_NVIC_EnableIRQ(DFSDM1_FLT1_IRQn);

    uint32_t min_buff_size = PDM_BUFFER_SIZE * channels * sizeof(int16_t);
    uint32_t buff_size = PDMgetBufferSize();
    if(buff_size < min_buff_size) {
      PDMsetBufferSize(min_buff_size);
    }

    return 1;
}

void py_audio_gain_set(int gain_db)
{
  attenuation = 8 - gain_db;
}

void py_audio_deinit()
{
    // Disable IRQs
    HAL_NVIC_DisableIRQ(DFSDM1_FLT0_IRQn);
    HAL_NVIC_DisableIRQ(DFSDM1_FLT1_IRQn);
    HAL_NVIC_DisableIRQ(AUDIO_DFSDM1_DMA_IRQ);

    if (hdfsdm1_channel.Instance != NULL) {
      HAL_DFSDM_ChannelDeInit(&hdfsdm1_channel);
      hdfsdm1_channel.Instance = NULL;
    }

    if (hdfsdm1_filter.Instance != NULL) {
      //HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm1_filter);
      HAL_DFSDM_FilterDeInit(&hdfsdm1_filter);
      hdfsdm1_filter.Instance = NULL;
    }

    if (hdma_dfsdm1_flt.Instance != NULL) {
      HAL_DMA_DeInit(&hdma_dfsdm1_flt);
      hdma_dfsdm1_flt.Instance = NULL;
    }

    //free(g_pcmbuf);
    xfer_status = 0;
    g_pcmbuf = NULL;
}

void audio_pendsv_callback(void)
{
    int i = 0;
    // Check for half transfer complete.
    if ((xfer_status & DMA_XFER_HALF)) {
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_HALF);
        
        for (int i=0; i<PDM_BUFFER_SIZE/2; i++) {
            ((int16_t*)g_pcmbuf)[i]     = SaturaLH((RecBuff[i] >> attenuation), -32768, 32767);
        }
    } else if ((xfer_status & DMA_XFER_FULL)) { // Check for transfer complete.
        // Clear buffer state.
        xfer_status &= ~(DMA_XFER_FULL);
        
        for(int i = 0; i < PDM_BUFFER_SIZE/2; i++)
        {
            ((int16_t*)g_pcmbuf)[i]     = SaturaLH((RecBuff[PDM_BUFFER_SIZE/2 + i] >> attenuation), -32768, 32767);
        }
    }
}

int get_filter_state()
{
    return HAL_DFSDM_FilterGetState(&hdfsdm1_filter);
}

int py_audio_start_streaming()
{   
    // Clear DMA buffer status
    xfer_status &= DMA_XFER_NONE;

    // Start DMA transfer
    if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter, (uint32_t*) RecBuff, PDM_BUFFER_SIZE) != HAL_OK) {
        return 0;
    }

    return 1;
}

void py_audio_stop_streaming()
{
    // Stop DMA.
    HAL_DFSDM_FilterRegularStop_DMA(&hdfsdm1_filter);
}

void Error_Handler(void)
{
  while (1)
  {
    HAL_Delay(1000);
  }
}

#endif
