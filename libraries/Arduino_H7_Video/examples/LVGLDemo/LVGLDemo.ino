
/*
  LVGLDemo

  created 17 Apr 2023
  by Leonardo Cavagnis
*/

#include "Arduino_H7_Video.h"
#include "Arduino_GigaDisplayTouch.h"

#include "lvgl.h"

Arduino_H7_Video          Display(800, 480, GigaDisplayShield); /* Arduino_H7_Video Display(1024, 768, USBCVideo); */
Arduino_GigaDisplayTouch  TouchDetector;

void gigaTouchHandler(uint8_t contacts, GDTpoint_t* points) {
  if (contacts > 0) {
    Serial.print("Touch detected: ");
    Serial.print(points[0].x);
    Serial.print(",");
    Serial.println(points[0].y);
  }
}

void setup() {
  Serial.begin(115200);

  Display.begin();
  TouchDetector.begin();
  TouchDetector.onDetect(gigaTouchHandler);

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