/**
  ******************************************************************************
  * @file    dsi.cpp
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>

#include "dsi.h"
#include "SDRAM.h"

/* Private define ------------------------------------------------------------*/
#define LCD_MAX_X_SIZE 		1280
#define LCD_MAX_Y_SIZE 		1024

#define BYTES_PER_PIXEL		2
#define FB_BASE_ADDRESS 	((uint32_t)SDRAM_START_ADDRESS)
#define FB_ADDRESS_0 		(FB_BASE_ADDRESS)
#define FB_ADDRESS_1 		(FB_BASE_ADDRESS + (LCD_MAX_X_SIZE * LCD_MAX_Y_SIZE * BYTES_PER_PIXEL))

/* Private variables ---------------------------------------------------------*/
static DMA2D_HandleTypeDef dma2d;
static LTDC_HandleTypeDef  ltdc;

static uint32_t lcd_x_size = LCD_MAX_X_SIZE;
static uint32_t lcd_y_size = LCD_MAX_Y_SIZE;

static DMA2D_CLUTCfgTypeDef clut;
static uint32_t __ALIGNED(32) L8_CLUT[256];

static uint32_t pend_buffer = 0;

volatile uint32_t reloadLTDC_status = 0;

/* Exported variables --------------------------------------------------------*/
DSI_HandleTypeDef   dsi;

/* Private function prototypes -----------------------------------------------*/
static void dsi_fillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);
static void dsi_layerInit(uint16_t LayerIndex, uint32_t FB_Address);

/* Functions -----------------------------------------------------------------*/
int dsi_init(uint8_t bus, struct edid *edid, struct display_timing *dt) {
#ifdef ARDUINO_GIGA
	static const uint32_t DSI_PLLNDIV = 125;
	static const uint32_t DSI_PLLIDF = DSI_PLL_IN_DIV4;
	static const uint32_t DSI_PLLODF = DSI_PLL_OUT_DIV1;
	static const uint32_t DSI_TXEXCAPECLOCKDIV = 4;
	#undef HSE_VALUE
	#define HSE_VALUE 16000000
#else
	static const uint32_t DSI_PLLNDIV = 40;
	static const uint32_t DSI_PLLIDF = DSI_PLL_IN_DIV2;
	static const uint32_t DSI_PLLODF = DSI_PLL_OUT_DIV1;
	static const uint32_t DSI_TXEXCAPECLOCKDIV = 4;
#endif

	static uint32_t LTDC_FREQ_STEP = 100;
	// set PLL3 to start from a 1MHz reference and increment by 100 or 200 KHz based on the frequency range

	if (dt->pixelclock/LTDC_FREQ_STEP > 512) LTDC_FREQ_STEP = 200;

	static const uint32_t LTDC_PLL3M = HSE_VALUE/1000000;
	static const uint32_t LTDC_PLL3N = dt->pixelclock/LTDC_FREQ_STEP;
	static const uint32_t LTDC_PLL3P = 2;
	static const uint32_t LTDC_PLL3Q = 7;
	static const uint32_t LTDC_PLL3R = 1000 / LTDC_FREQ_STEP; // expected pixel clock
	dt->pixelclock = (LTDC_PLL3N) *LTDC_FREQ_STEP; 	// real pixel clock

	static const uint32_t LANE_BYTE_CLOCK =	62500;

	// TODO: switch USB to use HSI48
	lcd_x_size = dt->hactive;
	lcd_y_size = dt->vactive;

	DSI_PLLInitTypeDef dsiPllInit;
	DSI_PHY_TimerTypeDef dsiPhyInit;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
	DSI_VidCfgTypeDef hdsivideo_handle;

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

	/*************************DSI Initialization***********************************/

	/* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
	dsi.Instance = DSI;

	HAL_DSI_DeInit(&(dsi));

	/* Configure the DSI PLL */
	dsiPllInit.PLLNDIV    = DSI_PLLNDIV;
	dsiPllInit.PLLIDF     = DSI_PLLIDF;
	dsiPllInit.PLLODF     = DSI_PLLODF;

	/* Set number of Lanes */
	dsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
	/* Set the TX escape clock division ratio */
	dsi.Init.TXEscapeCkdiv = DSI_TXEXCAPECLOCKDIV;
	/* Disable the automatic clock lane control (the ANX7265 must be clocked) */
	dsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;

	HAL_DSI_Init(&dsi, &dsiPllInit);

	hdsivideo_handle.VirtualChannelID     = 0;

	/* Timing parameters for Video modes */
	/* Set Timing parameters of DSI depending on its chosen format */
	hdsivideo_handle.ColorCoding          = DSI_RGB565;					// may choose DSI_RGB888
	hdsivideo_handle.LooselyPacked        = DSI_LOOSELY_PACKED_DISABLE;
	hdsivideo_handle.VSPolarity           = dt->vpol ? DSI_VSYNC_ACTIVE_HIGH : DSI_VSYNC_ACTIVE_LOW;
	hdsivideo_handle.HSPolarity           = dt->hpol ? DSI_VSYNC_ACTIVE_HIGH : DSI_HSYNC_ACTIVE_LOW;
	hdsivideo_handle.DEPolarity           = DSI_DATA_ENABLE_ACTIVE_HIGH;
	hdsivideo_handle.Mode                 = DSI_VID_MODE_BURST;
	hdsivideo_handle.NullPacketSize       = 0xFFF;
	hdsivideo_handle.NumberOfChunks       = 0;
	hdsivideo_handle.PacketSize           = lcd_x_size;
	hdsivideo_handle.HorizontalSyncActive = dt->hsync_len*LANE_BYTE_CLOCK/dt->pixelclock;
	hdsivideo_handle.HorizontalBackPorch  = dt->hback_porch*LANE_BYTE_CLOCK/dt->pixelclock;
	hdsivideo_handle.HorizontalLine       = (dt->hactive + dt->hsync_len + dt->hback_porch + dt->hfront_porch)*LANE_BYTE_CLOCK/dt->pixelclock;
	hdsivideo_handle.VerticalSyncActive   = dt->vsync_len;
	hdsivideo_handle.VerticalBackPorch    = dt->vback_porch;
	hdsivideo_handle.VerticalFrontPorch   = dt->vfront_porch;
	hdsivideo_handle.VerticalActive       = dt->vactive;

	/* Enable or disable sending LP command while streaming is active in video mode */
	hdsivideo_handle.LPCommandEnable      = DSI_LP_COMMAND_ENABLE; /* Enable sending commands in mode LP (Low Power) */

	/* Largest packet size possible to transmit in LP mode in VSA, VBP, VFP regions */
	/* Only useful when sending LP packets is allowed while streaming is active in video mode */
	hdsivideo_handle.LPLargestPacketSize          = 16;

	/* Largest packet size possible to transmit in LP mode in HFP region during VACT period */
	/* Only useful when sending LP packets is allowed while streaming is active in video mode */
	hdsivideo_handle.LPVACTLargestPacketSize      = 0;

	/* Specify for each region, if the going in LP mode is allowed */
	/* while streaming is active in video mode                     */
	hdsivideo_handle.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;   /* Allow sending LP commands during HFP period */
	hdsivideo_handle.LPHorizontalBackPorchEnable  = DSI_LP_HBP_ENABLE;   /* Allow sending LP commands during HBP period */
	hdsivideo_handle.LPVerticalActiveEnable = DSI_LP_VACT_ENABLE;  /* Allow sending LP commands during VACT period */
	hdsivideo_handle.LPVerticalFrontPorchEnable = DSI_LP_VFP_ENABLE;   /* Allow sending LP commands during VFP period */
	hdsivideo_handle.LPVerticalBackPorchEnable = DSI_LP_VBP_ENABLE;   /* Allow sending LP commands during VBP period */
	hdsivideo_handle.LPVerticalSyncActiveEnable = DSI_LP_VSYNC_ENABLE; /* Allow sending LP commands during VSync = VSA period */

	/* Configure DSI Video mode timings with settings set above */
	HAL_DSI_ConfigVideoMode(&dsi, &hdsivideo_handle);

	/* Configure DSI PHY HS2LP and LP2HS timings */
	dsiPhyInit.ClockLaneHS2LPTime = 35;
	dsiPhyInit.ClockLaneLP2HSTime = 35;
	dsiPhyInit.DataLaneHS2LPTime = 35;
	dsiPhyInit.DataLaneLP2HSTime = 35;
	dsiPhyInit.DataLaneMaxReadTime = 0;
	dsiPhyInit.StopWaitTime = 10;
	HAL_DSI_ConfigPhyTimer(&dsi, &dsiPhyInit);

	/*************************End DSI Initialization*******************************/

	/************************LTDC Initialization***********************************/

	/* LCD clock configuration */
	/* LCD clock configuration */
	/* PLL3_VCO Input = HSE_VALUE/PLL3M = 1 Mhz */
	/* PLL3_VCO Output = PLL3_VCO Input * PLL3N */
	/* PLLLCDCLK = PLL3_VCO Output/PLL3R */
	/* LTDC clock frequency = PLLLCDCLK */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLL3.PLL3M = LTDC_PLL3M;
	PeriphClkInitStruct.PLL3.PLL3N = LTDC_PLL3N;
	PeriphClkInitStruct.PLL3.PLL3P = LTDC_PLL3P;
	PeriphClkInitStruct.PLL3.PLL3Q = LTDC_PLL3Q;
	PeriphClkInitStruct.PLL3.PLL3R = LTDC_PLL3R;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	/* Base address of LTDC registers to be set before calling De-Init */
	ltdc.Instance = LTDC;

	HAL_LTDC_DeInit(&(ltdc));

	/* Timing Configuration */
	ltdc.Init.HorizontalSync = (dt->hsync_len -1);
	ltdc.Init.AccumulatedHBP = (dt->hsync_len + dt->hback_porch -1);
	ltdc.Init.AccumulatedActiveW = (dt->hactive + dt->hsync_len + dt->hback_porch -1);
	ltdc.Init.TotalWidth = (dt->hactive + dt->hsync_len + dt->hback_porch + dt->hfront_porch -1);
	ltdc.Init.VerticalSync = (dt->vsync_len -1);
	ltdc.Init.AccumulatedVBP = (dt->vsync_len + dt->vback_porch-1);
	ltdc.Init.AccumulatedActiveH = (dt->vactive + dt->vsync_len + dt->vback_porch-1);
	ltdc.Init.TotalHeigh = (dt->vactive + dt->vsync_len + dt->vback_porch + dt->vfront_porch-1);

	/* background value */
	ltdc.Init.Backcolor.Blue = 0x00;
	ltdc.Init.Backcolor.Green = 0x00;
	ltdc.Init.Backcolor.Red = 0x00;

	ltdc.LayerCfg->ImageWidth  = lcd_x_size;
	ltdc.LayerCfg->ImageHeight = lcd_y_size;

	/* Polarity */
	ltdc.Init.HSPolarity = dt->hpol ? LTDC_HSPOLARITY_AH : LTDC_HSPOLARITY_AL;
	ltdc.Init.VSPolarity = dt->vpol ? LTDC_VSPOLARITY_AH : LTDC_VSPOLARITY_AL;
	ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	/* Initialize & Start the LTDC */
	HAL_LTDC_Init(&ltdc);

	/* Enable the DSI host and wrapper : but LTDC is not started yet at this stage */
	HAL_DSI_Start(&dsi);

	HAL_DSI_Refresh(&dsi);

	dsi_layerInit(0, FB_ADDRESS_0);
	dsi_layerInit(1, FB_ADDRESS_0 + (lcd_x_size * lcd_y_size * BYTES_PER_PIXEL));

	HAL_DSI_PatternGeneratorStop(&dsi);
	
	dsi_lcdClear(0);
	dsi_drawCurrentFrameBuffer();
	dsi_lcdClear(0);
	dsi_drawCurrentFrameBuffer();
}

void dsi_lcdClear(uint32_t Color) {
	/* Clear the LCD */
	dsi_fillBuffer(pend_buffer%2, (uint32_t *)(ltdc.LayerCfg[pend_buffer%2].FBStartAdress), lcd_x_size, lcd_y_size, 0, Color);
}

void dsi_lcdFillArea(void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode) {
	dsi_fillBuffer(pend_buffer%2, pDst, xSize, ySize, lcd_x_size - xSize, ColorMode);
}

void dsi_lcdDrawImage(void *pSrc, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode) {
#if defined(__CORTEX_M7) 
	SCB_CleanInvalidateDCache();
	SCB_InvalidateICache();
#endif
	
	/* Configure the DMA2D Mode, Color Mode and output offset */
	dma2d.Init.Mode         = DMA2D_M2M_PFC;
	dma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565;
	dma2d.Init.OutputOffset = lcd_x_size - xSize;

	if (pDst == NULL) {
		pDst = (uint32_t *)(ltdc.LayerCfg[pend_buffer%2].FBStartAdress);
	}

	/* Foreground Configuration */
	dma2d.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
	dma2d.LayerCfg[1].InputAlpha = 0x00;
	dma2d.LayerCfg[1].InputColorMode = ColorMode;
	dma2d.LayerCfg[1].InputOffset = 0;

	dma2d.Instance = DMA2D;

	/* DMA2D Initialization */
	if(HAL_DMA2D_Init(&dma2d) == HAL_OK) {
		if(HAL_DMA2D_ConfigLayer(&dma2d, 1) == HAL_OK) {
			if (HAL_DMA2D_Start(&dma2d, (uint32_t)pSrc, (uint32_t)pDst, xSize, ySize) == HAL_OK) {
				/* Polling For DMA transfer */
				HAL_DMA2D_PollForTransfer(&dma2d, 25);
			}
		}
	}
}

void dsi_configueCLUT(uint32_t *colors) {
	memcpy(L8_CLUT, colors, 256 * 4);
	clut.pCLUT = (uint32_t *)L8_CLUT;
	clut.CLUTColorMode = DMA2D_CCM_ARGB8888;
	clut.Size = 0xFF;

#ifdef CORE_CM7
	SCB_CleanInvalidateDCache();
	SCB_InvalidateICache();
#endif

	HAL_DMA2D_ConfigLayer(&dma2d, 1);
	HAL_DMA2D_CLUTLoad(&dma2d, clut, 1);
	HAL_DMA2D_PollForTransfer(&dma2d, 100);

	HAL_DMA2D_ConfigLayer(&dma2d, 0);
	HAL_DMA2D_CLUTLoad(&dma2d, clut, 0);
	HAL_DMA2D_PollForTransfer(&dma2d, 100);
}

uint32_t dsi_getFramebufferEnd(void) {
	return (FB_BASE_ADDRESS + 2 * (lcd_x_size * lcd_y_size * BYTES_PER_PIXEL));
}

void dsi_drawCurrentFrameBuffer(bool reload) {
	int fb = pend_buffer++ % 2;

	/* Enable current LTDC layer */
	__HAL_LTDC_LAYER_ENABLE(&(ltdc), fb);
	/* Disable active LTDC layer */
	__HAL_LTDC_LAYER_DISABLE(&(ltdc), !fb);

	if (!reload) {
		return;
	}
	/* LTDC reload request within next vertical blanking */
	reloadLTDC_status = 0;
	HAL_LTDC_Reload(&ltdc, LTDC_SRCR_VBR);
	
	while(reloadLTDC_status == 0) {
		/* Wait till reload takes effect */
		delay(1);
	}
}

uint32_t dsi_getCurrentFrameBuffer() {
	return (ltdc.LayerCfg[pend_buffer%2].FBStartAdress);
}

uint32_t dsi_getActiveFrameBuffer() {
	return (ltdc.LayerCfg[(pend_buffer+1)%2].FBStartAdress);
}

uint32_t dsi_getDisplayXSize(){
	return lcd_x_size;
}

uint32_t dsi_getDisplayYSize(){
	return lcd_y_size;
}

void dsi_layerInit(uint16_t LayerIndex, uint32_t FB_Address) {
	LTDC_LayerCfgTypeDef  Layercfg;

	/* Layer Init */
	Layercfg.WindowX0 = 0;
	Layercfg.WindowX1 = lcd_x_size;
	Layercfg.WindowY0 = 0;
	Layercfg.WindowY1 = lcd_y_size;
	Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	Layercfg.FBStartAdress = FB_Address;
	Layercfg.Alpha = 255;
	Layercfg.Alpha0 = 0;
	Layercfg.Backcolor.Blue = 0;
	Layercfg.Backcolor.Green = 0;
	Layercfg.Backcolor.Red = 0;
	Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	Layercfg.ImageWidth = lcd_x_size;
	Layercfg.ImageHeight = lcd_y_size;

	HAL_LTDC_ConfigLayer(&ltdc, &Layercfg, LayerIndex);
}

void dsi_fillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) {
	/* Register to memory mode with ARGB8888 as color Mode */
	dma2d.Init.Mode         = DMA2D_R2M;
	dma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565;	//DMA2D_OUTPUT_ARGB8888
	dma2d.Init.OutputOffset = OffLine;

	dma2d.Instance = DMA2D;

	/* DMA2D Initialization */
	if(HAL_DMA2D_Init(&dma2d) == HAL_OK) {
		if(HAL_DMA2D_ConfigLayer(&dma2d, 1) == HAL_OK) {
			if (HAL_DMA2D_Start(&dma2d, ColorIndex, (uint32_t)pDst, xSize, ySize) == HAL_OK) {
				/* Polling For DMA transfer */
				HAL_DMA2D_PollForTransfer(&dma2d, 25);
			}
		}
	}
}

/* Handler for LTDC global interrupt request */
extern "C" void LTDC_IRQHandler(void) {
	HAL_LTDC_IRQHandler(&ltdc);
}

/* Reload LTDC event callback */
extern "C" void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
  reloadLTDC_status = 1;
}

/**** END OF FILE ****/
