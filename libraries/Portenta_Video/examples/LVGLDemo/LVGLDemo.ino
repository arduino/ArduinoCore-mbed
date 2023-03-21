#include "Portenta_Video.h" 
#include "lvgl.h"

Portenta_Video Display;

void setup() {
  Display.begin();

  #if LVGL_VERSION_MAJOR == 8
    /* Change the active screen's background color */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x03989e), LV_PART_MAIN);

    /* Create a white label, set its text and align it to the center */
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello Arduino - v8.x!");
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  #elif LVGL_VERSION_MAJOR == 7
    /* Change the active screen's background color */
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, LV_STATE_DEFAULT, lv_color_hex(0x03989e));
    lv_obj_add_style(lv_scr_act(), LV_OBJ_PART_MAIN, &style);

    /* Create a white label, set its text and align it to the center */
    lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_recolor(label, true); 
    lv_label_set_text(label, "#ffffff Hello Arduino - v7.x!#");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
  #endif
}

void loop() { 
  /* Feed LVGL engine */
  #if LVGL_VERSION_MAJOR == 8
    lv_timer_handler();
  #elif LVGL_VERSION_MAJOR == 7
    lv_task_handler();
  #endif
}