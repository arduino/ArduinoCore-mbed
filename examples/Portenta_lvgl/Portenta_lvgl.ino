#include <lvgl.h>

#include "Portenta_Video.h"
#include "SDRAM.h"
#include "lv_demo_widgets.h"

static uint32_t lcd_x_size = 0;
static uint32_t lcd_y_size = 0;

uint16_t * fb;

/* Display flushing */
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

   uint16_t * fb_start = fb + area->y1 * lcd_x_size + area->x1;
   uint32_t w = lv_area_get_width(area);
   uint32_t h = lv_area_get_height(area);
   
   //GPU: Has artifacts
   //stm32_LCD_DrawImage((void*)color_p, (void*)fb_start, w, h, DMA2D_INPUT_RGB565);

  //NO GPU
  int32_t y;    
  for(y = area->y1; y <= area->y2; y++) {
      memcpy(&fb[y * lcd_x_size + area->x1], color_p, w * sizeof(lv_color_t));
      color_p += w;
  }
  
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}


/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
   stm32_LCD_DrawImage((void*)src, (void*)dest, length, 1, DMA2D_INPUT_RGB565);  
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
   uint16_t * fb_start = fb + fill_area->y1 * dest_width + fill_area->x1;
   uint32_t w = lv_area_get_width(fill_area);
   uint32_t h = lv_area_get_height(fill_area);
//   stm32_LCD_FillArea((void*)color_p, (void*)fb_start, w, h, color.full);
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
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = my_disp_flush;
 // disp_drv.gpu_fill_cb = gpu_fill;
 // disp_drv.gpu_blend_cb = gpu_blend;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Hell world label*/
  //lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
  //lv_label_set_text(label, "Hello Arduino! Dev-7,0");
  //lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

   lv_demo_widgets();

}

void loop() {
  uint32_t t1 = millis();
  lv_task_handler();
  delay(3);
  uint32_t t2 = millis();
  lv_tick_inc(t2-t1);
}
