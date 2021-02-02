#include "Arduino.h"
#include "himax.h"
#include "camera.h"
#include "stm32h7xx_hal_dcmi.h"

#define CAMERA_FRAME_BUFFER               0xC0200000

#define ARGB8888_BYTE_PER_PIXEL  4

static const int CamRes[][2] = {
    {160, 120},
    {320, 240},
};
static __IO uint32_t camera_frame_ready = 0;
static md_callback_t user_md_callback = NULL;

/* DCMI DMA Stream definitions */
#define CAMERA_DCMI_DMAx_CLK_ENABLE         __HAL_RCC_DMA2_CLK_ENABLE
#define CAMERA_DCMI_DMAx_STREAM             DMA2_Stream3
#define CAMERA_DCMI_DMAx_IRQ                DMA2_Stream3_IRQn 

extern "C" {

DCMI_HandleTypeDef  hdcmi_discovery;

void BSP_CAMERA_PwrUp(void);

/**
  * @brief  Initializes the DCMI MSP.
  * @param  hdcmi: HDMI handle
  * @param  Params : pointer on additional configuration parameters, can be NULL. 
  * @retval None
  */
void BSP_CAMERA_MspInit(DCMI_HandleTypeDef *hdcmi, void *Params)
{
  static DMA_HandleTypeDef hdma_handler;
  GPIO_InitTypeDef gpio_init_structure;

  /*** Enable peripherals and GPIO clocks ***/
  /* Enable DCMI clock */
  __HAL_RCC_DCMI_CLK_ENABLE();

  /* Enable DMA clock */
  CAMERA_DCMI_DMAx_CLK_ENABLE();

  /* Enable GPIO clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  gpio_init_structure.Pin       = GPIO_PIN_4 | GPIO_PIN_6;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &gpio_init_structure);

  gpio_init_structure.Pin       = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);

  gpio_init_structure.Pin       = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOI, &gpio_init_structure);

  /*** Configure the DMA ***/
  /* Set the parameters to be configured */
  hdma_handler.Init.Request             = DMA_REQUEST_DCMI;
  hdma_handler.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_handler.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_handler.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_handler.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  hdma_handler.Init.Mode                = DMA_NORMAL;
  hdma_handler.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_handler.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_handler.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_handler.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_handler.Init.PeriphBurst         = DMA_PBURST_SINGLE; 

  hdma_handler.Instance = CAMERA_DCMI_DMAx_STREAM;

  /* Associate the initialized DMA handle to the DCMI handle */
  __HAL_LINKDMA(hdcmi, DMA_Handle, hdma_handler);
  
  /*** Configure the NVIC for DCMI and DMA ***/
  /* NVIC configuration for DCMI transfer complete interrupt */
  HAL_NVIC_SetPriority(DCMI_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DCMI_IRQn);

  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(CAMERA_DCMI_DMAx_IRQ, 0x0F, 0);
  HAL_NVIC_EnableIRQ(CAMERA_DCMI_DMAx_IRQ);

  /* Configure the DMA stream */
  HAL_DMA_Init(hdcmi->DMA_Handle);
}

/**
  * @brief  DeInitializes the DCMI MSP.
  * @param  hdcmi: HDMI handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  * @retval None
  */
void BSP_CAMERA_MspDeInit(DCMI_HandleTypeDef *hdcmi, void *Params)
{
  /* Disable NVIC  for DCMI transfer complete interrupt */
  HAL_NVIC_DisableIRQ(DCMI_IRQn);  
  
  /* Disable NVIC for DMA2 transfer complete interrupt */
  HAL_NVIC_DisableIRQ(CAMERA_DCMI_DMAx_IRQ);
  
  /* Configure the DMA stream */
  HAL_DMA_DeInit(hdcmi->DMA_Handle);  

  /* Disable DCMI clock */
  __HAL_RCC_DCMI_CLK_DISABLE();

  /* GPIO pins clock and DMA clock can be shut down in the application
     by surcharging this __weak function */
}

/**
  * @}
  */

/** @defgroup STM32H747I_DISCOVERY_CAMERA_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Set Camera image rotation on LCD Displayed frame buffer.
  * @param  rotation : uint32_t rotation of camera image in preview buffer sent to LCD
  *         need to be of type Camera_ImageRotationTypeDef
  * @retval Camera status
  */
uint8_t BSP_CAMERA_SetRotation(uint32_t rotation)
{
}

/**
  * @brief  Get Camera image rotation on LCD Displayed frame buffer.
  * @retval rotation : uint32_t value of type Camera_ImageRotationTypeDef
  */
uint32_t BSP_CAMERA_GetRotation(void)
{
}

/**
  * @brief  Initializes the camera.
  * @param  Resolution : camera sensor requested resolution (x, y) : standard resolution
  *         naming QQVGA, QVGA, VGA ...
  * @retval Camera status
  */
uint8_t BSP_CAMERA_Init(uint32_t Resolution)
{
  DCMI_HandleTypeDef *phdcmi;
  uint8_t status = -1;

  /* Get the DCMI handle structure */
  phdcmi = &hdcmi_discovery;

  /*** Configures the DCMI to interface with the camera module ***/
  /* DCMI configuration */
  phdcmi->Init.CaptureRate      = DCMI_CR_ALL_FRAME;
  phdcmi->Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
  phdcmi->Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
  phdcmi->Init.VSPolarity       = DCMI_VSPOLARITY_LOW;
  phdcmi->Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  phdcmi->Init.PCKPolarity      = DCMI_PCKPOLARITY_FALLING;
  phdcmi->Init.ByteSelectMode   = DCMI_BSM_ALL;         // Capture all received bytes
  phdcmi->Init.ByteSelectStart  = DCMI_OEBS_ODD;        // Ignored
  phdcmi->Init.LineSelectMode   = DCMI_LSM_ALL;         // Capture all received lines
  phdcmi->Init.LineSelectStart  = DCMI_OELS_ODD;        // Ignored
  phdcmi->Instance              = DCMI;

  /* Power up camera */
  BSP_CAMERA_PwrUp();
  if (HIMAX_Open()!=0) return status;

  /* DCMI Initialization */
  BSP_CAMERA_MspInit(&hdcmi_discovery, NULL);
  HAL_DCMI_Init(phdcmi);

  /*
  * @param  X0    DCMI window X offset
  * @param  Y0    DCMI window Y offset
  * @param  XSize DCMI Pixel per line
  * @param  YSize DCMI Line number
  * @retval HAL status
  */
  HAL_DCMI_EnableCROP(phdcmi);
  HAL_DCMI_ConfigCROP(phdcmi, 0, 0, CamRes[Resolution][0] - 1, CamRes[Resolution][1] - 1);

  __HAL_DCMI_DISABLE_IT(&hdcmi_discovery, DCMI_IT_LINE);

  /* Return CAMERA_OK status */
  status = 0;

  return status;
}

/**
  * @brief  DeInitializes the camera.
  * @retval Camera status
  */
uint8_t BSP_CAMERA_DeInit(void)
{ 
  hdcmi_discovery.Instance              = DCMI;

  HAL_DCMI_DeInit(&hdcmi_discovery);
  BSP_CAMERA_MspDeInit(&hdcmi_discovery, NULL);
  return 1;
}

/**
  * @brief  Starts the camera capture in snapshot mode.
  * @param  buff: pointer to the camera output buffer
  * @retval None
  */
void BSP_CAMERA_SnapshotStart(uint8_t *buff, uint32_t framesize)
{
  __HAL_DCMI_ENABLE_IT(&hdcmi_discovery, DCMI_IT_FRAME);

  /* Start the camera capture */
  HAL_DCMI_Start_DMA(&hdcmi_discovery, DCMI_MODE_SNAPSHOT, (uint32_t)buff, framesize / 4);
}

/**
  * @brief Suspend the CAMERA capture 
  * @retval None
  */
void BSP_CAMERA_Suspend(void) 
{
  /* Suspend the Camera Capture */
  HAL_DCMI_Suspend(&hdcmi_discovery);
}

/**
  * @brief Resume the CAMERA capture 
  * @retval None
  */
void BSP_CAMERA_Resume(void)
{
  /* Start the Camera Capture */
  HAL_DCMI_Resume(&hdcmi_discovery);
}

/**
  * @brief  Stop the CAMERA capture 
  * @retval Camera status
  */
uint8_t BSP_CAMERA_Stop(void)
{
  uint8_t status = 0;

  if(HAL_DCMI_Stop(&hdcmi_discovery) == HAL_OK)
  {
     status = 1;
  }

  /* Set Camera in Power Down */
  // BSP_CAMERA_PwrDown();

  return status;
}

TIM_HandleTypeDef  TIMHandle  = {0};

#define DCMI_TIM                (TIM1)
#define DCMI_TIM_PIN            (GPIO_PIN_1)
#define DCMI_TIM_PORT           (GPIOK)
#define DCMI_TIM_AF             (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL        (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()   __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()  __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()    HAL_RCC_GetPCLK2Freq()
#define OMV_XCLK_FREQUENCY      (6000000)

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
}

static int extclk_config(int frequency)
{
    /* TCLK (PCLK) */
    int tclk = DCMI_TIM_PCLK_FREQ();

    /* Period should be even */
    int period = (tclk / frequency) - 1;

    if (TIMHandle.Init.Period && (TIMHandle.Init.Period != period)) {
        // __HAL_TIM_SET_AUTORELOAD sets TIMHandle.Init.Period...
        __HAL_TIM_SET_AUTORELOAD(&TIMHandle, period);
        __HAL_TIM_SET_COMPARE(&TIMHandle, DCMI_TIM_CHANNEL, period / 2);
        return 0;
    }

    /* Timer base configuration */
    TIMHandle.Instance           = DCMI_TIM;
    TIMHandle.Init.Period        = period;
    TIMHandle.Init.Prescaler     = TIM_ETRPRESCALER_DIV1;
    TIMHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    TIMHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    /* Timer channel configuration */
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse       = period / 2;
    TIMOCHandle.OCMode      = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity  = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCFastMode  = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState = TIM_OCIDLESTATE_RESET;
    TIMOCHandle.OCNIdleState = TIM_OCIDLESTATE_RESET;

    if ((HAL_TIM_PWM_Init(&TIMHandle) != HAL_OK)
    || (HAL_TIM_PWM_ConfigChannel(&TIMHandle, &TIMOCHandle, DCMI_TIM_CHANNEL) != HAL_OK)
    || (HAL_TIM_PWM_Start(&TIMHandle, DCMI_TIM_CHANNEL) != HAL_OK)) {
        return -1;
    }

    return 0;
}

/**
  * @brief  CANERA power up
  * @retval None
  */
void BSP_CAMERA_PwrUp(void)
{
  mbed::DigitalOut* powerup = new mbed::DigitalOut(PC_13);
  *powerup = 0;
  delay(50);
  *powerup = 1;
  delay(50);

  extclk_config(OMV_XCLK_FREQUENCY);

  HAL_Delay(30);     /* POWER_DOWN de-asserted during 3ms */
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
  camera_frame_ready++;
}

void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hdcmi_discovery.DMA_Handle);
}

void DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&hdcmi_discovery);
}

/**
  * @brief  Line Event callback.
  * @retval None
  */
void BSP_CAMERA_LineEventCallback(void)
{
}

/**
  * @brief  Line event callback
  * @param  hdcmi: pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  BSP_CAMERA_LineEventCallback();
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

void CameraClass::HIMAXIrqHandler()
{
  if (user_md_callback) {
    user_md_callback();
  }
}

int CameraClass::begin(uint32_t resolution, uint32_t framerate)
{  
  if (resolution >= CAMERA_RMAX) {
    return -1;
  }

  /*## Camera Initialization and capture start ############################*/
  /* Initialize the Camera in QVGA mode */
  if(BSP_CAMERA_Init(resolution) != 0)
  {
    return -1;
  }

  if (HIMAX_SetResolution(resolution) != 0) {
    return -1;
  }

  if (HIMAX_SetFramerate(framerate) != 0) {
    return -1;
  }

  if (HIMAX_Mode(HIMAX_Streaming) != 0) {
    return -1;
  }

  user_md_callback = NULL;
  this->initialized = true;
  this->resolution = resolution;
  return 0;
}

int CameraClass::framerate(uint32_t framerate)
{
  if (this->initialized == false) {
    return -1;
  }
  return HIMAX_SetFramerate(framerate);
}

int CameraClass::grab(uint8_t *buffer, uint32_t timeout)
{
  if (this->initialized == false) {
    return -1;
  }

  BSP_CAMERA_Resume();

  /* Frame size from resolution. */
  uint32_t framesize = CamRes[this->resolution][0] * CamRes[this->resolution][1];

  camera_frame_ready = 0;

  /* Start the Camera Snapshot Capture */
  BSP_CAMERA_SnapshotStart(buffer, framesize);

  /* Wait until camera frame is ready : DCMI Frame event */
  for (uint32_t start = millis(); camera_frame_ready == 0;) {
    __WFI();
    if ((millis() - start) > timeout) {
      HAL_DMA_Abort(hdcmi_discovery.DMA_Handle);
      return -1;
    }
  }

  /* Stop the camera to avoid having the DMA2D work in parallel of Display */
  /* which cause perturbation of LTDC                                      */
  BSP_CAMERA_Suspend();

  /* Invalidate buffer after DMA transfer */
  SCB_InvalidateDCache_by_Addr((uint32_t*)buffer, framesize);

  return 0;
}

int CameraClass::standby(bool enable)
{
  if (this->initialized == false) {
    return -1;
  } else if (enable) {
    return HIMAX_Mode(HIMAX_Standby);
  } else {
    return HIMAX_Mode(HIMAX_Streaming);
  }
}

int CameraClass::motionDetectionThreshold(uint32_t low, uint32_t high)
{
  if (this->initialized == false) {
    return -1;
  }
  return HIMAX_SetMDThreshold(low, high);
}

int CameraClass::motionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  uint32_t width, height;

  if (this->initialized == false) {
    return -1;
  }

  width = CamRes[this->resolution][0];
  height= CamRes[this->resolution][1];

  if (((x+w) > width) || ((y+h) > height)) {
      return -1;
  }
  return HIMAX_SetLROI(x, y, x+w, y+h);
}

int CameraClass::motionDetection(bool enable, md_callback_t callback)
{
  if (this->initialized == false) {
    return -1;
  }

  this->md_irq.rise(0);

  if (enable == false) {
    user_md_callback = NULL;
  } else {
    user_md_callback = callback;
    this->md_irq.rise(mbed::Callback<void()>(this, &CameraClass::HIMAXIrqHandler));
    this->md_irq.enable_irq();
  }

  return HIMAX_EnableMD(enable);
}

int CameraClass::motionDetected()
{
  int ret = 0;

  if (this->initialized == false) {
    return -1;
  }

  ret = HIMAX_PollMD();
  if (ret == 1) {
    HIMAX_ClearMD();
  }
  return ret;
}

int CameraClass::testPattern(bool walking)
{
  if (this->initialized == false) {
    return -1;
  }
  HIMAX_TestPattern(true, walking);
  return 0;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
