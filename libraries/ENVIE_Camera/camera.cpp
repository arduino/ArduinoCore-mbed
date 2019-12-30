#include "Arduino.h"
#include "himax.h"
#include "camera.h"
#include "stm32h7xx_hal_dcmi.h"

#define LCD_FRAME_BUFFER                  0xC0000000 /* LCD Frame buffer of size 800x480 in ARGB8888 */
#define CAMERA_FRAME_BUFFER               0xC0200000

#define QVGA_RES_X	320
#define QVGA_RES_Y	240

#define CAM_W_RES       324
#define CAM_H_RES       244
#define CAM_PIXEL_IMAGE_SIZE (CAM_H_RES * CAM_W_RES)
#define SKIP_FRAMES 10

#define ARGB8888_BYTE_PER_PIXEL  4

static uint32_t   CameraResX = QVGA_RES_X;
static uint32_t   CameraResY = QVGA_RES_Y;
static uint32_t   LcdResX    = 0;
static uint32_t   LcdResY    = 0;

__IO uint32_t camera_frame_ready = 0;
__IO uint32_t lcd_frame_ready = 0;
 
/* DCMI DMA Stream definitions */
#define CAMERA_DCMI_DMAx_CLK_ENABLE         __HAL_RCC_DMA2_CLK_ENABLE
#define CAMERA_DCMI_DMAx_STREAM             DMA2_Stream3
#define CAMERA_DCMI_DMAx_IRQ                DMA2_Stream3_IRQn 

#define CAMERA_R160x120                 0x00   /* QQVGA Resolution                     */
#define CAMERA_R320x240                 0x01   /* QVGA Resolution                      */
#define CAMERA_R480x272                 0x02   /* 480x272 Resolution                   */
#define CAMERA_R640x480                 0x03   /* VGA Resolution                       */  


DCMI_HandleTypeDef  hdcmi_discovery;

static uint32_t CameraCurrentResolution;

/** @defgroup STM32H747I_DISCOVERY_CAMERA_Private_FunctionPrototypes Private FunctionPrototypes
  * @{
  */
static uint32_t GetSize(uint32_t Resolution);
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
  uint8_t status = 0;

  /* Get the DCMI handle structure */
  phdcmi = &hdcmi_discovery;

  /*** Configures the DCMI to interface with the camera module ***/
  /* DCMI configuration */
  phdcmi->Init.CaptureRate      = DCMI_CR_ALL_FRAME;
  phdcmi->Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
  phdcmi->Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
  phdcmi->Init.VSPolarity       = DCMI_VSPOLARITY_HIGH;
  phdcmi->Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  phdcmi->Init.PCKPolarity      = DCMI_PCKPOLARITY_RISING;
  phdcmi->Instance              = DCMI;

  /* Power up camera */
  // BSP_CAMERA_PwrUp();
  HIMAX_Open();

  /* DCMI Initialization */
  BSP_CAMERA_MspInit(&hdcmi_discovery, NULL);
  HAL_DCMI_Init(phdcmi);

/*
      HAL_DCMI_ConfigCROP(phdcmi,           // Crop in the middle of the VGA picture
                         (CAMERA_VGA_RES_X - CAMERA_480x272_RES_X)/2,
                         (CAMERA_VGA_RES_Y - CAMERA_480x272_RES_Y)/2,
                         (CAMERA_480x272_RES_X * 2) - 1,
                          CAMERA_480x272_RES_Y - 1);
      HAL_DCMI_EnableCROP(phdcmi);
*/

  HAL_DCMI_DisableCROP(phdcmi);

  CameraCurrentResolution = Resolution;

  /* Return CAMERA_OK status */
  status = 1;

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
  * @brief  Starts the camera capture in continuous mode.
  * @param  buff: pointer to the camera output buffer
  * @retval None
  */
void BSP_CAMERA_ContinuousStart(uint8_t *buff)
{
  /* Start the camera capture */
  HAL_DCMI_Start_DMA(&hdcmi_discovery, DCMI_MODE_CONTINUOUS, (uint32_t)buff, GetSize(CameraCurrentResolution));
}

/**
  * @brief  Starts the camera capture in snapshot mode.
  * @param  buff: pointer to the camera output buffer
  * @retval None
  */
void BSP_CAMERA_SnapshotStart(uint8_t *buff)
{
  /* Start the camera capture */
  HAL_DCMI_Start_DMA(&hdcmi_discovery, DCMI_MODE_SNAPSHOT, (uint32_t)buff, GetSize(CameraCurrentResolution));
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

/**
  * @brief  CANERA power up
  * @retval None
  */
void BSP_CAMERA_PwrUp(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable GPIO clock */
  __HAL_RCC_GPIOJ_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  gpio_init_structure.Pin       = GPIO_PIN_14;
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

  /* De-assert the camera POWER_DOWN pin (active high) */
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_14, GPIO_PIN_RESET);

  HAL_Delay(3);     /* POWER_DOWN de-asserted during 3ms */
}

/**
  * @brief  CAMERA power down
  * @retval None
  */
void BSP_CAMERA_PwrDown(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable GPIO clock */
  __HAL_RCC_GPIOJ_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  gpio_init_structure.Pin       = GPIO_PIN_14;
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

  /* Assert the camera POWER_DOWN pin (active high) */
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_14, GPIO_PIN_SET);
}

/**
  * @brief  Get the capture size in pixels unit.
  * @param  Resolution: the current resolution.
  * @retval capture size in pixels unit.
  */
static uint32_t GetSize(uint32_t Resolution)
{
  uint32_t size = 0;

  /* Get capture size */
  switch (Resolution)
  {
  case CAMERA_R160x120:
    {
      size =  0x2580;
    }
    break;
  case CAMERA_R320x240:
    {
      size =  0x9600;
    }
    break;
  case CAMERA_R480x272:
    {
      size =  0xFF00;
    }
    break;
  case CAMERA_R640x480:
    {
      size =  0x25800;
    }
    break;
  default:
    {
      break;
    }
  }

  return size;
}

/**
  * @brief  Initializes the DCMI MSP.
  * @param  hdcmi: HDMI handle
  * @param  Params : pointer on additional configuration parameters, can be NULL. 
  * @retval None
  */
__weak void BSP_CAMERA_MspInit(DCMI_HandleTypeDef *hdcmi, void *Params)
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
  hdma_handler.Init.Mode                = DMA_CIRCULAR;
  hdma_handler.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_handler.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_handler.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_handler.Init.MemBurst            = DMA_MBURST_SINGLE;
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
__weak void BSP_CAMERA_MspDeInit(DCMI_HandleTypeDef *hdcmi, void *Params)
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
  * @brief  Error callback.
  * @retval None
  */
__weak void BSP_CAMERA_ErrorCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_ErrorCallback could be implemented in the user file
   */
}

void BSP_CAMERA_FrameEventCallback(void)
{
  camera_frame_ready = 1;
}


void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hdcmi_discovery.DMA_Handle);
}

void DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&hdcmi_discovery);
}

int CameraClass::begin(void)
{  
  CameraResX = QVGA_RES_X;
  CameraResY = QVGA_RES_Y;

  /*## Camera Initialization and capture start ############################*/
  /* Initialize the Camera in QVGA mode */
  if(BSP_CAMERA_Init(CAMERA_R320x240) != 1)
  {
    printf("BSP_CAMERA_Init failed\n");
    return 0;
  }

}

int CameraClass::snapshot(void)
{
  printf("Start snapshot\n");
  HIMAX_Mode(HIMAX_Streaming);

  /* Start the Camera Snapshot Capture */
  //BSP_CAMERA_SnapshotStart((uint8_t *)CAMERA_FRAME_BUFFER);
  BSP_CAMERA_ContinuousStart((uint8_t *)CAMERA_FRAME_BUFFER);

  /* Wait until camera frame is ready : DCMI Frame event */
  while(camera_frame_ready == 0)
  {
  }

  printf("Snapshot done\n");
  
  /* Stop the camera to avoid having the DMA2D work in parallel of Display */
  /* which cause perturbation of LTDC                                      */
  BSP_CAMERA_Stop();
}

extern "C" {

/**
  * @brief  Line Event callback.
  * @retval None
  */
__weak void BSP_CAMERA_LineEventCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_LineEventCallback could be implemented in the user file
   */
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
__weak void BSP_CAMERA_VsyncEventCallback(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_VsyncEventCallback could be implemented in the user file
   */
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
