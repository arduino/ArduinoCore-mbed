#include <lvgl.h>
#include "mbed.h"

#include "Portenta_Video.h"
#include "SDRAM.h"
#include "lv_demo_widgets.h"

static uint32_t lcd_x_size = 0;
static uint32_t lcd_y_size = 0;

static uint16_t * fb;
static lv_disp_drv_t disp_drv;

/* Display flushing */
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

#if ARDUINO_PORTENTA_H7_M7
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


/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
   It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{

#if ARDUINO_PORTENTA_H7_M7
  SCB_CleanInvalidateDCache();
#endif

  DMA2D_HandleTypeDef * dma2d = stm32_get_DMA2D();

  dma2d->Instance = DMA2D;
  dma2d->Init.Mode = DMA2D_M2M_BLEND;
  dma2d->Init.OutputOffset = 0;

  /* Foreground layer */
  dma2d->LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  dma2d->LayerCfg[1].InputAlpha = opa;
  dma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  dma2d->LayerCfg[1].InputOffset = 0;
  dma2d->LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;

  /* Background layer */
  dma2d->LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  dma2d->LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
  dma2d->LayerCfg[0].InputOffset = 0;

  /* DMA2D Initialization */
  if (HAL_DMA2D_Init(dma2d) == HAL_OK) {
    if (HAL_DMA2D_ConfigLayer(dma2d, 0) == HAL_OK && HAL_DMA2D_ConfigLayer(dma2d, 1) == HAL_OK) {
      HAL_DMA2D_BlendingStart(dma2d, (uint32_t) src, (uint32_t) dest, (uint32_t) dest, length, 1);
      HAL_DMA2D_PollForTransfer(dma2d, 1000);
    }
  }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color */
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                     const lv_area_t * fill_area, lv_color_t color)
{
#if ARDUINO_PORTENTA_H7_M7
  SCB_CleanInvalidateDCache();
#endif

  DMA2D_HandleTypeDef * dma2d = stm32_get_DMA2D();

  lv_color_t * destination = dest_buf + (dest_width * fill_area->y1 + fill_area->x1);

  uint32_t w = fill_area->x2 - fill_area->x1 + 1;
  dma2d->Instance = DMA2D;
  dma2d->Init.Mode = DMA2D_R2M;
  dma2d->Init.ColorMode = DMA2D_OUTPUT_RGB565;
  dma2d->Init.OutputOffset = dest_width - w;
  dma2d->LayerCfg[1].InputAlpha = DMA2D_NO_MODIF_ALPHA;
  dma2d->LayerCfg[1].InputColorMode = DMA2D_OUTPUT_RGB565;

  /* DMA2D Initialization */
  if (HAL_DMA2D_Init(dma2d) == HAL_OK) {
    if (HAL_DMA2D_ConfigLayer(dma2d, 1) == HAL_OK) {
      lv_coord_t h = lv_area_get_height(fill_area);
      if (HAL_DMA2D_BlendingStart(dma2d, lv_color_to32(color), (uint32_t)destination, (uint32_t)destination, w, h) == HAL_OK) {
        HAL_DMA2D_PollForTransfer(dma2d, 1000);
      }
    }
  }
}


struct edid recognized_edid;
mbed::DigitalOut video_on(PK_2);
mbed::DigitalOut video_rst(PJ_3);
void setup() {
  // put your setup code here, to run once:
  delay(1000);
  video_on = 1;
  delay(10);
  video_rst = 1;
  delay(10);
  int ret = -1;
  video_on = 0;
  delay(10);
  video_rst = 0;
  delay(100);
  while (ret < 0) {
    video_on = 0;
    delay(10);
    video_rst = 0;
    delay(100);
    video_on = 1;
    delay(100);
    video_rst = 1;
    ret = anx7625_init(0);
  }
  anx7625_dp_get_edid(0, &recognized_edid);
  anx7625_dp_start(0, &recognized_edid, EDID_MODE_720x480_60Hz);
  SDRAM.begin(getFramebufferEnd());

  lv_init();

  lcd_x_size = stm32_getXSize();
  lcd_y_size = stm32_getYSize();

  fb = (uint16_t *)getNextFrameBuffer();
  getNextFrameBuffer();

  static lv_color_t buf[LV_HOR_RES_MAX * LV_VER_RES_MAX / 6];
  static lv_disp_buf_t disp_buf;
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX / 6);

  /*Initialize the display*/
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.gpu_fill_cb = gpu_fill;
  disp_drv.gpu_blend_cb = gpu_blend;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Hell world label*/
  //lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
  //lv_label_set_text(label, "Hello Arduino! Dev-7,0");
  //lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

  lv_demo_widgets();

}

void loop() {
  lv_task_handler();
  delay(3);
}
