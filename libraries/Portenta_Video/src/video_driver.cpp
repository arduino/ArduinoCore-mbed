#include "video_driver.h"
#include "SDRAM.h"
#include "video_modes.h"
#include "anx7625.h"
#include "dsi.h"

static uint16_t * fb;
static uint32_t lcd_x_size = 0;
static uint32_t lcd_y_size = 0;

struct edid recognized_edid;

void portenta_init_video() {
  int ret = -1;

  //Initialization of ANX7625
  ret = anx7625_init(0);
  if(ret < 0) {
    printf("Cannot continue, anx7625 init failed.\n");
    while(1);
  }

  //Checking HDMI plug event
  anx7625_wait_hpd_event(0);

  //Read EDID
  anx7625_dp_get_edid(0, &recognized_edid);

  //DSI Configuration
  anx7625_dp_start(0, &recognized_edid, EDID_MODE_720x480_60Hz);

  //Configure SDRAM 
  SDRAM.begin(getFramebufferEnd());

  lcd_x_size = stm32_getXSize();
  lcd_y_size = stm32_getYSize();

  fb = (uint16_t *)getNextFrameBuffer();
  getNextFrameBuffer();
}

void giga_init_video(bool landscape) {
  // put your setup code here, to run once:
  int ret = -1;

  SDRAM.begin();

  extern struct envie_edid_mode envie_known_modes[];
  struct edid _edid;
  int mode = EDID_MODE_480x800_60Hz;
  struct display_timing dt;
  dt.pixelclock = envie_known_modes[mode].pixel_clock;
  dt.hactive = envie_known_modes[mode].hactive;
  dt.hsync_len = envie_known_modes[mode].hsync_len;
  dt.hback_porch = envie_known_modes[mode].hback_porch;
  dt.hfront_porch = envie_known_modes[mode].hfront_porch;
  dt.vactive = envie_known_modes[mode].vactive;
  dt.vsync_len = envie_known_modes[mode].vsync_len;
  dt.vback_porch = envie_known_modes[mode].vback_porch;
  dt.vfront_porch = envie_known_modes[mode].vfront_porch;
  dt.hpol = envie_known_modes[mode].hpol;
  dt.vpol = envie_known_modes[mode].vpol;

  stm32_dsi_config(0, &_edid, &dt);

  lcd_x_size = stm32_getXSize();
  lcd_y_size = stm32_getYSize();

  fb = (uint16_t *)getNextFrameBuffer();
  getNextFrameBuffer();
}

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {

#if defined(__CORTEX_M7)
  SCB_CleanInvalidateDCache();
  SCB_InvalidateICache();
#endif

  DMA2D_HandleTypeDef * dma2d = stm32_get_DMA2D();

  lv_color_t * pDst = (lv_color_t*)fb;
  pDst += area->y1 * lcd_x_size + area->x1;

  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
  dma2d->Init.Mode         = DMA2D_M2M;
  dma2d->Init.ColorMode    = DMA2D_OUTPUT_RGB565;
  dma2d->Init.OutputOffset = lcd_x_size - w;
  dma2d->Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
  dma2d->Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */

  /*##-2- DMA2D Callbacks Configuration ######################################*/
  dma2d->XferCpltCallback  = NULL;

  /*##-3- Foreground Configuration ###########################################*/
  dma2d->LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  dma2d->LayerCfg[1].InputAlpha = 0xFF;
  dma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  dma2d->LayerCfg[1].InputOffset = 0;
  dma2d->LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
  dma2d->LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

  /* DMA2D Initialization */
  if (HAL_DMA2D_Init(dma2d) == HAL_OK) {
    if (HAL_DMA2D_ConfigLayer(dma2d, 1) == HAL_OK) {
      HAL_DMA2D_Start(dma2d, (uint32_t)color_p, (uint32_t)pDst, w, h);
      HAL_DMA2D_PollForTransfer(dma2d, 1000);
    }
  }

  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}