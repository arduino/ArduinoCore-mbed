#include "Portenta_lvgl.h"
#include "lv_demo_widgets.h"

void setup() {
  portenta_init_video();
  lv_demo_widgets();
}

void loop() {
  lv_task_handler();
  delay(3);
}
