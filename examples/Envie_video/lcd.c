/* Includes ------------------------------------------------------------------*/
#include <stm32h7xx_hal.h>
#include <stdint.h>
#include "image_320x240_argb8888.h"
#include "life_augmented_argb8888.h"
#include <string.h>
#include <stdio.h>

/** @addtogroup STM32H7xx_HAL_Examples
    @{
*/

/** @addtogroup LCD_DSI_CmdMode_DoubleBuffering
    @{
*/

/* Private typedef -----------------------------------------------------------*/
LTDC_HandleTypeDef hltdc_eval;
static DMA2D_HandleTypeDef           hdma2d;
DSI_HandleTypeDef hdsi_eval;
DSI_VidCfgTypeDef hdsivideo_handle;
DSI_CmdCfgTypeDef CmdCfg;
DSI_LPCmdTypeDef LPCmd;
DSI_PLLInitTypeDef dsiPllInit;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
/* Private define ------------------------------------------------------------*/

#define VSYNC               1
#define VBP                 1
#define VFP                 1
#define VACT                480
#define HSYNC               1
#define HBP                 1
#define HFP                 1
#define HACT                800

#define LCD_FB_START_ADDRESS       ((uint32_t)0xD0000000)
#define LAYER0_ADDRESS               (LCD_FB_START_ADDRESS)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO int32_t  front_buffer   = 0;
static __IO int32_t  pend_buffer   = -1;
static uint32_t ImageIndex = 0;
static const uint32_t * Images[] =
{
  image_320x240_argb8888,
  life_augmented_argb8888,
};

static const uint32_t Buffers[] =
{
  LAYER0_ADDRESS,
  LAYER0_ADDRESS + (800 * 480 * 4),
};

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void CopyBuffer(uint32_t *pSrc,
                       uint32_t *pDst,
                       uint16_t x,
                       uint16_t y,
                       uint16_t xsize,
                       uint16_t ysize);
static uint8_t LCD_Init(void);
void LTDC_Init(void);
static void LCD_BriefDisplay(void);
static void CPU_CACHE_Enable(void);
static void MPU_Config_Display(void);

void BSP_LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address)
{
  LTDC_LayerCfgTypeDef  Layercfg;

  /* Layer Init */
  Layercfg.WindowX0 = 0;
  Layercfg.WindowX1 = 800;
  Layercfg.WindowY0 = 0;
  Layercfg.WindowY1 = 480;
  Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  Layercfg.FBStartAdress = FB_Address;
  Layercfg.Alpha = 255;
  Layercfg.Alpha0 = 0;
  Layercfg.Backcolor.Blue = 0;
  Layercfg.Backcolor.Green = 0;
  Layercfg.Backcolor.Red = 0;
  Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  Layercfg.ImageWidth = 800;
  Layercfg.ImageHeight = 480;

  HAL_LTDC_ConfigLayer(&hdsi_eval, &Layercfg, LayerIndex);
}

__weak void BSP_LCD_MspInit(void)
{
  /** @brief Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /** @brief Toggle Sw reset of LTDC IP */
  __HAL_RCC_LTDC_FORCE_RESET();
  __HAL_RCC_LTDC_RELEASE_RESET();

  /** @brief Enable the DMA2D clock */
  __HAL_RCC_DMA2D_CLK_ENABLE();

  /** @brief Toggle Sw reset of DMA2D IP */
  __HAL_RCC_DMA2D_FORCE_RESET();
  __HAL_RCC_DMA2D_RELEASE_RESET();

  /** @brief Enable DSI Host and wrapper clocks */
  __HAL_RCC_DSI_CLK_ENABLE();

  /** @brief Soft Reset the DSI Host and wrapper */
  __HAL_RCC_DSI_FORCE_RESET();
  __HAL_RCC_DSI_RELEASE_RESET();

  /** @brief NVIC configuration for LTDC interrupt that is now enabled */
  HAL_NVIC_SetPriority(LTDC_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(LTDC_IRQn);

  /** @brief NVIC configuration for DMA2D interrupt that is now enabled */
  HAL_NVIC_SetPriority(DMA2D_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DMA2D_IRQn);

  /** @brief NVIC configuration for DSI interrupt that is now enabled */
  HAL_NVIC_SetPriority(DSI_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DSI_IRQn);
}


/* Private functions ---------------------------------------------------------*/

/**
    @brief  Main program
    @param  None
    @retval None
*/
int doStuff(void)
{

  /* System Init, System clock, voltage scaling and L1-Cache configuration are done by CPU1 (Cortex-M7)
     in the meantime Domain D2 is put in STOP mode(Cortex-M4 in deep-sleep)
  */

  /* Configure the MPU attributes as Write Through for SDRAM*/
  MPU_Config_Display();

  /* Initialize the LCD   */
  if ( LCD_Init() != 0)
  {
    Error_Handler();
  }

  /* Disable DSI Wrapper in order to access and configure the LTDC */
  __HAL_DSI_WRAPPER_DISABLE(&hdsi_eval);

  /* Initialize LTDC layer 0 iused for Hint */
  BSP_LCD_LayerDefaultInit(0, LAYER0_ADDRESS);

  /* Enable DSI Wrapper so DSI IP will drive the LTDC */
  __HAL_DSI_WRAPPER_ENABLE(&hdsi_eval);

  /* Display example brief   */
  LCD_BriefDisplay();

  /* Copy Buffer 0 into buffer 1, so only image area to be redrawn later */
  CopyBuffer((uint32_t *)Buffers[0], (uint32_t *)Buffers[1], 0, 0, 800, 480);

  /*Draw first image */
  CopyBuffer((uint32_t *)Images[ImageIndex++], (uint32_t *)Buffers[front_buffer], 240, 160, 320, 240);
  pend_buffer = 0;

  /*Refresh the LCD display*/
  HAL_DSI_Refresh(&hdsi_eval);

  /* Infinite loop */
  while (1)
  {
    if (pend_buffer < 0)
    {
      /* Prepare back buffer */
      CopyBuffer((uint32_t *)Images[ImageIndex++], (uint32_t *)Buffers[1 - front_buffer], 240, 160, 320, 240);
      pend_buffer = 1 - front_buffer;

      if (ImageIndex >= 2)
      {
        ImageIndex = 0;
      }

      /* Refresh the display */
      HAL_DSI_Refresh(&hdsi_eval);

      /* Wait some time before switching to next stage */
      HAL_Delay(2000);
    }
  }
}

/**
    @brief  End of Refresh DSI callback.
    @param  hdsi: pointer to a DSI_HandleTypeDef structure that contains
                  the configuration information for the DSI.
    @retval None
*/
void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi)
{
  if (pend_buffer >= 0)
  {
    /* Disable DSI Wrapper */
    __HAL_DSI_WRAPPER_DISABLE(&hdsi_eval);
    /* Update LTDC configuaration */
    LTDC_LAYER(&hltdc_eval, 0)->CFBAR = ((uint32_t)Buffers[pend_buffer]);
    __HAL_LTDC_RELOAD_CONFIG(&hltdc_eval);
    /* Enable DSI Wrapper */
    __HAL_DSI_WRAPPER_ENABLE(&hdsi_eval);

    front_buffer = pend_buffer;
    pend_buffer = -1;
  }
}

/**
    @brief  Initializes the DSI LCD.
    The ititialization is done as below:
        - DSI PLL ititialization
        - DSI ititialization
        - LTDC ititialization
        - OTM8009A LCD Display IC Driver ititialization
    @param  None
    @retval LCD state
*/
static uint8_t LCD_Init(void)
{
  DSI_PHY_TimerTypeDef  PhyTimings;

  /* Toggle Hardware Reset of the DSI LCD using
     its XRES signal (active low) */
  //BSP_LCD_Reset();

  /* Call first MSP Initialize only in case of first initialization
    This will set IP blocks LTDC, DSI and DMA2D
    - out of reset
    - clocked
    - NVIC IRQ related to IP blocks enabled
  */
  BSP_LCD_MspInit();

  /* LCD clock configuration */
  /* LCD clock configuration */
  /* PLL3_VCO Input = HSE_VALUE/PLL3M = 5 Mhz */
  /* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 800 Mhz */
  /* PLLLCDCLK = PLL3_VCO Output/PLL3R = 800/19 = 42 Mhz */
  /* LTDC clock frequency = PLLLCDCLK = 42 Mhz */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLL3.PLL3M = 5;
  PeriphClkInitStruct.PLL3.PLL3N = 160;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 19;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
  hdsi_eval.Instance = DSI;

  HAL_DSI_DeInit(&(hdsi_eval));

  dsiPllInit.PLLNDIV  = 100;
  dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV5;
  dsiPllInit.PLLODF  = DSI_PLL_OUT_DIV1;

  hdsi_eval.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
  hdsi_eval.Init.TXEscapeCkdiv = 0x4;


  HAL_DSI_Init(&(hdsi_eval), &(dsiPllInit));

  /* Configure the DSI for Command mode */
  CmdCfg.VirtualChannelID      = 0;
  CmdCfg.HSPolarity            = DSI_HSYNC_ACTIVE_HIGH;
  CmdCfg.VSPolarity            = DSI_VSYNC_ACTIVE_HIGH;
  CmdCfg.DEPolarity            = DSI_DATA_ENABLE_ACTIVE_HIGH;
  CmdCfg.ColorCoding           = DSI_RGB888;
  CmdCfg.CommandSize           = HACT;
  CmdCfg.TearingEffectSource   = DSI_TE_DSILINK;
  CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
  CmdCfg.VSyncPol              = DSI_VSYNC_FALLING;
  CmdCfg.AutomaticRefresh      = DSI_AR_DISABLE;
  CmdCfg.TEAcknowledgeRequest  = DSI_TE_ACKNOWLEDGE_ENABLE;
  HAL_DSI_ConfigAdaptedCommandMode(&hdsi_eval, &CmdCfg);

  LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_ENABLE;
  LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_ENABLE;
  LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_ENABLE;
  LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_ENABLE;
  LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_ENABLE;
  LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_ENABLE;
  LPCmd.LPGenLongWrite        = DSI_LP_GLW_ENABLE;
  LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_ENABLE;
  LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_ENABLE;
  LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_ENABLE;
  LPCmd.LPDcsLongWrite        = DSI_LP_DLW_ENABLE;
  HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

  /* Initialize LTDC */
  LTDC_Init();

  /* Start DSI */
  HAL_DSI_Start(&(hdsi_eval));

  /* Configure DSI PHY HS2LP and LP2HS timings */
  PhyTimings.ClockLaneHS2LPTime = 35;
  PhyTimings.ClockLaneLP2HSTime = 35;
  PhyTimings.DataLaneHS2LPTime = 35;
  PhyTimings.DataLaneLP2HSTime = 35;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 10;
  HAL_DSI_ConfigPhyTimer(&hdsi_eval, &PhyTimings);

  LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_DISABLE;
  LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_DISABLE;
  LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_DISABLE;
  LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_DISABLE;
  LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_DISABLE;
  LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_DISABLE;
  LPCmd.LPGenLongWrite        = DSI_LP_GLW_DISABLE;
  LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_DISABLE;
  LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_DISABLE;
  LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_DISABLE;
  LPCmd.LPDcsLongWrite        = DSI_LP_DLW_DISABLE;
  HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

  HAL_DSI_ConfigFlowControl(&hdsi_eval, DSI_FLOW_CONTROL_BTA);
  HAL_DSI_ForceRXLowPower(&hdsi_eval, ENABLE);

  return 0;
}

/**
    @brief
    @param  None
    @retval None
*/
void LTDC_Init(void)
{
  /* DeInit */
  HAL_LTDC_DeInit(&hltdc_eval);

  /* LTDC Config */
  /* Timing and polarity */
  hltdc_eval.Init.HorizontalSync = HSYNC;
  hltdc_eval.Init.VerticalSync = VSYNC;
  hltdc_eval.Init.AccumulatedHBP = HSYNC + HBP;
  hltdc_eval.Init.AccumulatedVBP = VSYNC + VBP;
  hltdc_eval.Init.AccumulatedActiveH = VSYNC + VBP + VACT;
  hltdc_eval.Init.AccumulatedActiveW = HSYNC + HBP + HACT;
  hltdc_eval.Init.TotalHeigh = VSYNC + VBP + VACT + VFP;
  hltdc_eval.Init.TotalWidth = HSYNC + HBP + HACT + HFP;

  /* background value */
  hltdc_eval.Init.Backcolor.Blue = 0;
  hltdc_eval.Init.Backcolor.Green = 0;
  hltdc_eval.Init.Backcolor.Red = 0;

  /* Polarity */
  hltdc_eval.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc_eval.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc_eval.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc_eval.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc_eval.Instance = LTDC;

  HAL_LTDC_Init(&hltdc_eval);
}

/**
    @brief  Display Example description.
    @param  None
    @retval None
*/
static void LCD_BriefDisplay(void)
{
#if 0
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(0, 0, 800, 112);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillRect(0, 112, 800, 368);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_DisplayStringAtLine(1, (uint8_t *)"           LCD_DSI_CmdMode_DoubleBuffering");
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_DisplayStringAtLine(4, (uint8_t *)"This example shows how to display images on LCD DSI using two buffers");
  BSP_LCD_DisplayStringAtLine(5, (uint8_t *)"one for display and the other for draw");
#endif
}

/**
    @brief  Converts a line to an ARGB8888 pixel format.
    @param  pSrc: Pointer to source buffer
    @param  pDst: Output color
    @param  xSize: Buffer width
    @param  ColorMode: Input color mode
    @retval None
*/
static void CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{

  uint32_t destination = (uint32_t)pDst + (y * 800 + x) * 4;
  uint32_t source      = (uint32_t)pSrc;

  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
  hdma2d.Init.Mode         = DMA2D_M2M;
  hdma2d.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 800 - xsize;
  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
  hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */

  /*##-2- DMA2D Callbacks Configuration ######################################*/
  hdma2d.XferCpltCallback  = NULL;

  /*##-3- Foreground Configuration ###########################################*/
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

  hdma2d.Instance          = DMA2D;

  /* DMA2D Initialization */
  if (HAL_DMA2D_Init(&hdma2d) == HAL_OK)
  {
    if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&hdma2d, source, destination, xsize, ysize) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&hdma2d, 100);
      }
    }
  }
}

/**
    @brief  System Clock Configuration
            The system Clock is configured as follow :
               System Clock source            = PLL (HSE)
               SYSCLK(Hz)                     = 400000000 (CM7 CPU Clock)
               HCLK(Hz)                       = 200000000 (CM4 CPU, AXI and AHBs Clock)
               AHB Prescaler                  = 2
               D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
               D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
               D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
               D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
               HSE Frequency(Hz)              = 25000000
               PLL_M                          = 5
               PLL_N                          = 160
               PLL_P                          = 2
               PLL_Q                          = 4
               PLL_R                          = 2
               VDD(V)                         = 3.3
               Flash Latency(WS)              = 4
    @param  None
    @retval None
*/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if (ret != HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if (ret != HAL_OK)
  {
    Error_Handler();
  }

  /*
    Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
           (GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)
           The I/O Compensation Cell activation  procedure requires :
         - The activation of the CSI clock
         - The activation of the SYSCFG clock
         - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
  */

  /*activate CSI clock mondatory for I/O Compensation Cell*/
  __HAL_RCC_CSI_ENABLE() ;

  /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
  __HAL_RCC_SYSCFG_CLK_ENABLE() ;

  /* Enables the I/O Compensation Cell */
  HAL_EnableCompensationCell();
}

/**
  @brief  CPU L1-Cache enable.
  @param  None
  @retval None
*/
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
    @brief  Configure the MPU attributes as Write Through for External SDRAM.
    @note   The Base Address is 0xD0000000 .
            The Configured Region Size is 32MB because same as SDRAM size.
    @param  None
    @retval None
*/
static void MPU_Config_Display(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = LCD_FB_START_ADDRESS;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
    @brief Error Handler
    @retval None
*/
static void Error_Handler(void)
{

  while (1) {
    ;  /* Blocking on error */
  }
}

#ifdef  USE_FULL_ASSERT

/**
    @brief  Reports the name of the source file and the source line number
            where the assert_param error has occurred.
    @param  file: pointer to the source file name
    @param  line: assert_param error line source number
    @retval None
*/
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
    @}
*/

/**
    @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
