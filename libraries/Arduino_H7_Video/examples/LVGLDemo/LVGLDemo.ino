
/*
  LVGLDemo

  created DD MMM YYYY
  by Leonardo Cavagnis
*/

#include "Arduino_H7_Video.h"

#include "lvgl.h"
#include "giga_touch.h"

Arduino_H7_Video Display(800, 480, GIGA_DISPLAY_SHIELD);
//Arduino_H7_Video Display(720, 480);

//@TODO: Complete demo with 4 main features

void my_touchpad_read(lv_indev_drv_t * indev, lv_indev_data_t * data) {
  data->state = (touchpad_pressed) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  if(data->state == LV_INDEV_STATE_PR) {
    data->point.x = giga_get_touch_x();
    data->point.y = giga_get_touch_y();
    touchpad_pressed = false;

    Serial.print("Touch detected: ");
    Serial.print(data->point.x);
    Serial.print(",");
    Serial.println(data->point.y);
  }
  
  return;
}

void setup() {
  Serial.begin(115200);
  giga_touch_setup();

  Display.begin();
  Display.attachLVGLTouchCb((void (*)(void*,void*))(my_touchpad_read));
  
  /* Change the active screen's background color */
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x03989e), LV_PART_MAIN);

  /* Create a white label, set its text and align it to the center */
  lv_obj_t * label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello Arduino!");
  lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void loop() { 
  /* Touch handler */
  giga_touch_handler();
  
  /* Feed LVGL engine */
  lv_timer_handler();
}