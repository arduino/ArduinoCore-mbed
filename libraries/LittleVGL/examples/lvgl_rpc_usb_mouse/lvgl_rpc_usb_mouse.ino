#include "Portenta_LittleVGL.h"
#include "RPC_internal.h"
#include "USBHost.h"

int16_t touchpad_x = 0;
int16_t touchpad_y = 0;
uint8_t button = 0;
static lv_indev_drv_t indev_drv_mouse;
static lv_indev_drv_t indev_drv_btn;
static lv_obj_t * myCustomLabel;

void btn_event_cb(lv_obj_t * myCustomLabel, lv_event_t event)
{
  if (event == LV_EVENT_CLICKED) {
    lv_label_set_text(myCustomLabel , "ButtonClicked");
  }
}

void on_mouse(uint8_t btn, int8_t x, int8_t y) {
  Serial1.print("Mouse: ");
  Serial1.print(btn);
  Serial1.print(" ");
  Serial1.print(x);
  Serial1.print(" ");
  Serial1.println(y);
  touchpad_x += x;
  touchpad_y += y;
  if (touchpad_x < 0) {
    touchpad_x = 0;
  }
  if (touchpad_y < 0) {
    touchpad_y = 0;
  }
  button = btn;
}

void on_key(char ch) {
  Serial1.print("Keyboard: ");
  Serial1.println(ch);
}

bool my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
  data->point.x = touchpad_x;
  data->point.y = touchpad_y;
  data->state = LV_INDEV_STATE_REL;
  return false; /*No buffering now so no more data read*/
}

bool button_read(lv_indev_drv_t * drv, lv_indev_data_t*data){
    static uint32_t last_btn = 0;   /*Store the last pressed button*/
    int btn_pr = button - 1;        /*Get the ID (0,1,2...) of the pressed button*/
    if(btn_pr >= 0) {               /*Is there a button press? (E.g. -1 indicated no button was pressed)*/
       last_btn = btn_pr;           /*Save the ID of the pressed button*/
       data->state = LV_INDEV_STATE_PR;  /*Set the pressed state*/
    } else {
       data->state = LV_INDEV_STATE_REL; /*Set the released state*/
    }

    data->btn_id = last_btn;            /*Save the last button*/

    return false;                    /*No buffering now so no more data read*/
}
void setup() {
  // put your setup code here, to run once:
  RPC1.begin();
  Serial1.begin(115200);
  RPC1.bind("on_mouse", on_mouse);
  RPC1.bind("on_key", on_key);
  portenta_init_video();

  // Mouse pointer init
  lv_indev_drv_init(&indev_drv_mouse);      /*Basic initialization*/
  indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
  indev_drv_mouse.read_cb = my_input_read;
  lv_indev_t * my_indev_mouse = lv_indev_drv_register(&indev_drv_mouse);

  // Mouse pointer
  lv_obj_t * cursor_obj =  lv_img_create(lv_scr_act(), NULL); //create object
  lv_label_set_text(cursor_obj, "Sys layer");
  lv_indev_set_cursor(my_indev_mouse, cursor_obj); // connect the object to the driver

  // Mouse press
  lv_indev_drv_init(&indev_drv_btn);      /*Basic initialization*/
  indev_drv_btn.type = LV_INDEV_TYPE_BUTTON;
  indev_drv_btn.read_cb = button_read;
  lv_indev_t * my_indev_btn = lv_indev_drv_register(&indev_drv_btn);

  //Set your objects
  myCustomLabel = lv_label_create(lv_scr_act(), NULL);
  lv_obj_align(myCustomLabel, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(myCustomLabel , "Button");

 /*Assign buttons to points on the screen*/
  static const lv_point_t btn_points[1] = {
          {720/2, 480/2},   /*Button 0 -> x:10; y:10*/
  };
  lv_indev_set_button_points(my_indev_btn, btn_points);


  //Create a task
  //lv_task_create(label_refresher_task, 1000, LV_TASK_PRIO_MID, NULL);

  //Assign a callback to the button
  lv_obj_set_event_cb(myCustomLabel, btn_event_cb);
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_task_handler();
  //delay(3);
}
