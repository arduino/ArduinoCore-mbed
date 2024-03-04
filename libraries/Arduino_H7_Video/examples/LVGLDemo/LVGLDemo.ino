
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

/* Button click event callback */
static void btn_event_cb(lv_event_t * e) {
  static uint32_t cnt = 1;
  lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t * label = lv_obj_get_child(btn, 0);
  lv_label_set_text_fmt(label, "%"LV_PRIu32, cnt);
  cnt++;
}

/* Slider update value handler */
static void set_slider_val(void * bar, int32_t val) {
  lv_bar_set_value((lv_obj_t *)bar, val, LV_ANIM_ON);
}

void setup() {
  Serial.begin(115200);

  Display.begin();
  TouchDetector.begin();

  /* Create a container with grid 2x2 */
  static lv_coord_t col_dsc[] = {370, 370, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {215, 215, LV_GRID_TEMPLATE_LAST};
  lv_obj_t * cont = lv_obj_create(lv_scr_act());
  lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);
  lv_obj_set_size(cont, Display.width(), Display.height());
  lv_obj_set_style_bg_color(cont, lv_color_hex(0x03989e), LV_PART_MAIN);
  lv_obj_center(cont);

  lv_obj_t * label;
  lv_obj_t * obj;

  /* [0;0] - Image */
  obj = lv_obj_create(cont);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                        LV_GRID_ALIGN_STRETCH, 0, 1);

  LV_IMG_DECLARE(img_arduinologo);
  lv_obj_t * img1 = lv_img_create(obj);
  lv_img_set_src(img1, &img_arduinologo);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(img1, 200, 150);

  /* [1;0] - Checkboxes and button */
  obj = lv_obj_create(cont);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                        LV_GRID_ALIGN_STRETCH, 0, 1);
  lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);

  lv_obj_t * cb;
  cb = lv_checkbox_create(obj);
  lv_checkbox_set_text(cb, "Apple");

  cb = lv_checkbox_create(obj);
  lv_checkbox_set_text(cb, "Banana");
  lv_obj_add_state(cb, LV_STATE_CHECKED);

  static lv_style_t style_radio;
  static lv_style_t style_radio_chk;
  lv_style_init(&style_radio);
  lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);
  lv_style_init(&style_radio_chk);
  #if (LVGL_VERSION_MAJOR == 9)
  lv_style_set_bg_image_src(&style_radio_chk, NULL);
  #else
  lv_style_set_bg_img_src(&style_radio_chk, NULL);
  #endif
  
  cb = lv_checkbox_create(obj);
  lv_checkbox_set_text(cb, "Lemon");
  lv_obj_add_flag(cb, LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_obj_add_style(cb, &style_radio, LV_PART_INDICATOR);
  lv_obj_add_style(cb, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
  
  cb = lv_checkbox_create(obj);
  lv_checkbox_set_text(cb, "Melon");
  lv_obj_add_flag(cb, LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_obj_add_style(cb, &style_radio, LV_PART_INDICATOR);
  lv_obj_add_style(cb, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_add_state(cb, LV_STATE_CHECKED);

  lv_obj_t * btn = lv_btn_create(obj);
  lv_obj_set_size(btn, 100, 40);
  lv_obj_center(btn);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

  label = lv_label_create(btn);
  lv_label_set_text(label, "Click me!");
  lv_obj_center(label);

  /* [0;1] - Slider */
  obj = lv_obj_create(cont);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                        LV_GRID_ALIGN_STRETCH, 1, 1);
  
  lv_obj_t * slider = lv_slider_create(obj);
  lv_slider_set_value(slider, 75, LV_ANIM_OFF);
  lv_obj_center(slider);
  label = lv_label_create(obj);
  lv_label_set_text(label, "Drag me!");
  lv_obj_align_to(label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  
  /* [1;1] - Bar */
  obj = lv_obj_create(cont);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                        LV_GRID_ALIGN_STRETCH, 1, 1);

  lv_obj_t * bar = lv_bar_create(obj);
  lv_obj_set_size(bar, 200, 20);
  lv_obj_center(bar);
  lv_bar_set_value(bar, 70, LV_ANIM_OFF);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, set_slider_val);
  lv_anim_set_time(&a, 3000);
  lv_anim_set_playback_time(&a, 3000);
  lv_anim_set_var(&a, bar);
  lv_anim_set_values(&a, 0, 100);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);
}

void loop() { 
  /* Feed LVGL engine */
  lv_timer_handler();
}