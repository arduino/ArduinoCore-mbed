#include "Portenta_Video.h" 
#include "lvgl.h"
#include "video_driver.h"

#define DISP_HOR_RES  480
#define DISP_VER_RES  800

Portenta_Video Display;

void setup() {
  Display.begin();

  /* Initiliaze LVGL library */
  lv_init();

  /* Create a draw buffer */
  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /* Declare a buffer for 1/10 screen size */
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, DISP_HOR_RES * DISP_VER_RES / 10);  /* Initialize the display buffer. */
  
  /* Initialize display features for LVGL library */
  static lv_disp_drv_t disp_drv;        /* Descriptor of a display driver */
  lv_disp_drv_init(&disp_drv);          /* Basic initialization */
  disp_drv.flush_cb = my_disp_flush;    /* Set your driver function */
  disp_drv.draw_buf = &draw_buf;        /* Assign the buffer to the display */
  disp_drv.hor_res = DISP_HOR_RES;      /* Set the horizontal resolution of the display */
  disp_drv.ver_res = DISP_VER_RES;      /* Set the vertical resolution of the display */
  lv_disp_drv_register(&disp_drv);      /* Finally register the driver */

  /* Create a label */
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello Arduino!");
  lv_obj_set_pos(label, 10, 10);
}

void loop() { 
  /* Feed LVGL engine */
  lv_timer_handler();
}