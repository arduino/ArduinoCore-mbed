#include "Portenta_lvgl.h"
#include "lv_demo_widgets.h"

void LCD_ST7701_Init();

uint16_t touchpad_x;
uint16_t touchpad_y;
bool touchpad_pressed = false;

void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
  if(touchpad_pressed) {
    data->point.x = touchpad_x;
    data->point.y = touchpad_y;
    data->state = LV_INDEV_STATE_PRESSED;
    touchpad_pressed = false;
  } else {
    data->state = LV_INDEV_STATE_RELEASED; 
  }
}

void i2c_touch_setup();
void i2c_touch_loop();

void setup() {
  Serial.begin(115200);

  i2c_touch_setup();
  giga_init_video();
  LCD_ST7701_Init();
  lv_demo_widgets();

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);      /*Basic initialization*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_input_read;
  /*Register the driver in LVGL and save the created input device object*/
  lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);

  Serial.println(lv_disp_get_physical_ver_res(NULL));
  Serial.println(lv_disp_get_ver_res(NULL));
}

void loop() {
  i2c_touch_loop();
#if LVGL_VERSION_MAJOR > 7
  lv_timer_handler();
#else
  lv_task_handler();
#endif
}
