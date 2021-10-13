#include "Portenta_lvgl.h"
#include "lv_demo_widgets.h"

void setup() {
  portenta_init_video();
  lv_demo_widgets();
}

void loop() {
#if LVGL_VERSION_MAJOR > 7
  lv_timer_handler();
#else
  lv_task_handler();
#endif
}
