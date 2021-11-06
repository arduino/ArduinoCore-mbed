/*
 * TODO: Add license.
 * Copyright (c) 2021
 *
 * This work is licensed under <>, see the file LICENSE for details.
 *
 * Camera driver.
 */
#include "Arduino.h"
#include "camera.h"
#include "Wire.h"
#include "stm32h7xx_hal_dcmi.h"

/* Workaround for the broken UNUSED macro */
#undef UNUSED
#define UNUSED(x) ((void)((uint32_t)(x)))

// Include all image sensor drivers.
#include "himax.h"
#include "gc2145.h"

#define DCMI_TIM                    (TIM1)
#define DCMI_TIM_PIN                (GPIO_PIN_1)
#define DCMI_TIM_PORT               (GPIOK)
#define DCMI_TIM_AF                 (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL            (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()       __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()      __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()        HAL_RCC_GetPCLK2Freq()
#define DCMI_TIM_FREQUENCY          (6000000)

#define DCMI_DMA_CLK_ENABLE()       __HAL_RCC_DMA2_CLK_ENABLE()
#define DCMI_DMA_STREAM             DMA2_Stream3
#define DCMI_DMA_IRQ                DMA2_Stream3_IRQn

static DMA_HandleTypeDef hdma   = {0};
static DCMI_HandleTypeDef hdcmi = {0};
static TIM_HandleTypeDef  htim  = {0};
static volatile uint32_t frame_ready = 0;

static const int CamRes[][2] = {
    {160, 120},
    {320, 240},
    {320, 320},
};


extern "C" {

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == DCMI_TIM) {
        /* Enable DCMI timer clock */
        DCMI_TIM_CLK_ENABLE();

        /* Timer GPIO configuration */
        GPIO_InitTypeDef  GPIO_InitStructure;
        GPIO_InitStructure.Pin       = DCMI_TIM_PIN;
        GPIO_InitStructure.Pull      = GPIO_NOPULL;
        GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
        GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Alternate = DCMI_TIM_AF;
        HAL_GPIO_Init(DCMI_TIM_PORT, &GPIO_InitStructure);
    }
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef *hdcmi)
{
    GPIO_InitTypeDef hgpio;

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    /*** Configure the GPIOs ***/
    hgpio.Mode      = GPIO_MODE_AF_PP;
    hgpio.Pull      = GPIO_PULLUP;
    hgpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    hgpio.Alternate = GPIO_AF13_DCMI;

    hgpio.Pin       = GPIO_PIN_4 | GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &hgpio);

    hgpio.Pin       = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOI, &hgpio);

    hgpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOH, &hgpio);
}

void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef* hdcmi)
{
  /* Disable NVIC  for DCMI transfer complete interrupt */
  HAL_NVIC_DisableIRQ(DCMI_IRQn);
  
  /* Disable NVIC for DMA2 transfer complete interrupt */
  HAL_NVIC_DisableIRQ(DCMI_DMA_IRQ);
  
  /* Configure the DMA stream */
  HAL_DMA_DeInit(hdcmi->DMA_Handle);

  /* Disable DCMI clock */
  __HAL_RCC_DCMI_CLK_DISABLE();

  /* GPIO pins clock and DMA clock can be shut down in the application
     by surcharging this __weak function */
}

/**
  * @brief  Initializes the camera.
  * @retval Camera status
  */
uint8_t BSP_CAMERA_Init()
{
    /*** Configure the DMA ***/
    hdma.Instance                 = DCMI_DMA_STREAM;
    hdma.Init.Request             = DMA_REQUEST_DCMI;
    hdma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma.Init.MemInc              = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_NORMAL;
    hdma.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma.Init.MemBurst            = DMA_MBURST_INC4;
    hdma.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    /* Enable DMA clock */
    DCMI_DMA_CLK_ENABLE();

    /* Configure the DMA stream */
    HAL_DMA_Init(&hdma);

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(DCMI_DMA_IRQ, 0x03, 0);
    HAL_NVIC_EnableIRQ(DCMI_DMA_IRQ);

    /*** Configures the DCMI to interface with the camera module ***/
    hdcmi.Instance              = DCMI;
    hdcmi.Init.CaptureRate      = DCMI_CR_ALL_FRAME;
    hdcmi.Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.VSPolarity       = DCMI_VSPOLARITY_LOW;
    hdcmi.Init.PCKPolarity      = DCMI_PCKPOLARITY_FALLING;
    hdcmi.Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.ByteSelectMode   = DCMI_BSM_ALL;         // Capture all received bytes
    hdcmi.Init.ByteSelectStart  = DCMI_OEBS_ODD;        // Ignored
    hdcmi.Init.LineSelectMode   = DCMI_LSM_ALL;         // Capture all received lines
    hdcmi.Init.LineSelectStart  = DCMI_OELS_ODD;        // Ignored

    /* Associate the initialized DMA handle to the DCMI handle */
    __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma);

    /* Enable DCMI clock */
    __HAL_RCC_DCMI_CLK_ENABLE();

    /* DCMI Initialization */
    HAL_DCMI_Init(&hdcmi);
    __HAL_DCMI_ENABLE_IT(&hdcmi, DCMI_IT_FRAME);
    __HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_LINE);

    /* NVIC configuration for DCMI transfer complete interrupt */
    HAL_NVIC_SetPriority(DCMI_IRQn, 0x02, 0);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    return 0;
}

/**
  * @brief  DeInitializes the camera.
  * @retval Camera status
  */
uint8_t BSP_CAMERA_DeInit(void)
{ 
  hdcmi.Instance = DCMI;

  HAL_DCMI_DeInit(&hdcmi);
  return 1;
}

static int BSP_CAMERA_EXTCLK_Config(int frequency)
{
    /* TCLK (PCLK) */
    uint32_t tclk = DCMI_TIM_PCLK_FREQ() * 2;

    /* Period should be even */
    uint32_t period = (tclk / frequency) - 1;

    if (htim.Init.Period && (htim.Init.Period != period)) {
        // __HAL_TIM_SET_AUTORELOAD sets htim.Init.Period...
        __HAL_TIM_SET_AUTORELOAD(&htim, period);
        __HAL_TIM_SET_COMPARE(&htim, DCMI_TIM_CHANNEL, period / 2);
        return 0;
    }

    /* Timer base configuration */
    htim.Instance               = DCMI_TIM;
    htim.Init.Period            = period;
    htim.Init.Prescaler         = 0;
    htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim.Init.RepetitionCounter = 0;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    /* Timer channel configuration */
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse           = period / 2;
    TIMOCHandle.OCMode          = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity      = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCFastMode      = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState     = TIM_OCIDLESTATE_RESET;
    TIMOCHandle.OCNIdleState    = TIM_OCIDLESTATE_RESET;

    if ((HAL_TIM_PWM_Init(&htim) != HAL_OK)
    || (HAL_TIM_PWM_ConfigChannel(&htim, &TIMOCHandle, DCMI_TIM_CHANNEL) != HAL_OK)
    || (HAL_TIM_PWM_Start(&htim, DCMI_TIM_CHANNEL) != HAL_OK)) {
        return -1;
    }

    return 0;
}

/**
  * @brief  CAMERA power down
  * @retval None
  */
void BSP_CAMERA_PwrDown(void)
{
  digitalWrite(PC_13, LOW);
}

/**
  * @brief  Error callback.
  * @retval None
  */
void BSP_CAMERA_ErrorCallback(void)
{
}

void BSP_CAMERA_FrameEventCallback(void)
{
  frame_ready++;
}

void DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&hdcmi);
}

void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma);
}

/**
  * @brief  VSYNC Event callback.
  * @retval None
  */
void BSP_CAMERA_VsyncEventCallback(void)
{
}

/**
  * @brief  VSYNC event callback
  * @param  hdcmi: pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  BSP_CAMERA_VsyncEventCallback();
}

/**
  * @brief  Frame event callback
  * @param  hdcmi: pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  BSP_CAMERA_FrameEventCallback();
}

/**
  * @brief  Error callback
  * @param  hdcmi: pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{        
  BSP_CAMERA_ErrorCallback();
}

}

int Camera::Reset()
{
    // Reset sensor.
    digitalWrite(PC_13, LOW);
    HAL_Delay(10);

    digitalWrite(PC_13, HIGH);
    HAL_Delay(100);
    return 0;
}

int Camera::ProbeSensor()
{
    uint8_t addr;
    for (addr=1; addr<127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            break;
        }
    }

    return addr;
}

int Camera::begin(uint32_t resolution, uint32_t framerate)
{  
    if (resolution >= CAMERA_RMAX) {
        return -1;
    }

    // Configure the initial sensor clock.
    BSP_CAMERA_EXTCLK_Config(DCMI_TIM_FREQUENCY);
    HAL_Delay(10);

    // Reset the image sensor.
    Reset();

    // Start I2C
    Wire.begin();
    Wire.setClock(400000);

    if (ProbeSensor() != this->sensor->GetID()) {
        return -1;
    }

    if (sensor->GetClockFrequency() != DCMI_TIM_FREQUENCY) {
        // Reconfigure the sensor clock frequency.
        BSP_CAMERA_EXTCLK_Config(sensor->GetClockFrequency());
        HAL_Delay(10);
    }

    if (this->sensor->Init() != 0) {
        return -1;
    }

    if (BSP_CAMERA_Init() != 0) {
        return -1;
    }

    if (SetResolution(resolution) != 0) {
        return -1;
    }

    return 0;
}

int Camera::GetID()
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->GetID();
}

int Camera::SetFrameRate(uint32_t framerate)
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->SetFrameRate(framerate);
}

int Camera::SetResolution(uint32_t resolution)
{
    if (this->sensor == NULL) {
        return -1;
    }

    /*
     * @param  X0    DCMI window X offset
     * @param  Y0    DCMI window Y offset
     * @param  XSize DCMI Pixel per line
     * @param  YSize DCMI Line number
     * @retval HAL status
     */
    HAL_DCMI_EnableCROP(&hdcmi);
    HAL_DCMI_ConfigCROP(&hdcmi, 0, 0, CamRes[resolution][0] - 1, CamRes[resolution][1] - 1);

    if (this->sensor->SetResolution(resolution) == 0) {
        this->resolution = resolution;
        return 0;
    }
    return -1;
}

int Camera::SetPixelFormat(uint32_t pixelformat)
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->SetPixelFormat(pixelformat);
}

int Camera::SetStandby(bool enable)
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->SetStandby(enable);
}

int Camera::SetTestPattern(bool enable, bool walking)
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->SetTestPattern(enable, walking);
}

int Camera::GrabFrame(uint8_t *framebuffer, uint32_t timeout)
{
    if (this->sensor == NULL) {
        return -1;
    }

    if ((uint32_t) framebuffer & 0x1F) {
        // FB must be aligned to 32 bytes cache lines.
        return -1;
    }

    /* Frame size from resolution. */
    uint32_t framesize = CamRes[this->resolution][0] * CamRes[this->resolution][1];

    frame_ready = 0;

    /* Start the Camera Snapshot Capture */
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) framebuffer, framesize / 4);

    /* Wait until camera frame is ready : DCMI Frame event */
    for (uint32_t start = millis(); frame_ready == 0;) {
        __WFI();
        if ((millis() - start) > timeout) {
            HAL_DMA_Abort(&hdma);
            return -1;
        }
    }
    
    HAL_DCMI_Stop(&hdcmi);

    /* Invalidate buffer after DMA transfer */
    SCB_InvalidateDCache_by_Addr((uint32_t*) framebuffer, framesize);
    return 0;
}
