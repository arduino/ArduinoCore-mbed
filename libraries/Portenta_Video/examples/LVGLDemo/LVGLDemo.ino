#include "H7_Video.h" 
#include "lvgl.h"

H7_Video Display(480,800);

//@TODO: Complete demo with 4 main features + touch management

void setup() {
  Display.begin(false);

  /* Change the active screen's background color */
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x03989e), LV_PART_MAIN);

  /* Create a white label, set its text and align it to the center */
  lv_obj_t * label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello Arduino!");
  lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void loop() { 
  /* Feed LVGL engine */
  lv_timer_handler();
}