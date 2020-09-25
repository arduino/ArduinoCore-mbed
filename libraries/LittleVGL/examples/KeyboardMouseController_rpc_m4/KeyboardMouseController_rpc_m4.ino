#include "USBHost.h"
#include "RPC_internal.h"

#ifndef CORE_CM4
#error "This sketch should be compiled for Portenta (M4 core)"
#endif

USBHost usb;

#define MOD_CTRL      (0x01 | 0x10)
#define MOD_SHIFT     (0x02 | 0x20)
#define MOD_ALT       (0x04 | 0x40)
#define MOD_WIN       (0x08 | 0x80)

#define LED_NUM_LOCK    1
#define LED_CAPS_LOCK   2
#define LED_SCROLL_LOCK 4

static uint8_t key_leds;
static const char knum[] = "1234567890";
static const char ksign[] = "!@#$%^&*()";
static const char tabA[] = "\t -=[]\\#;'`,./";
static const char tabB[] = "\t _+{}|~:\"~<>?";
// route the key event to stdin

static void stdin_recvchar(char ch) {
  RPC1.call("on_key", ch);
}

static int process_key(tusbh_ep_info_t* ep, const uint8_t* keys)
{
  uint8_t modify = keys[0];
  uint8_t key = keys[2];
  uint8_t last_leds = key_leds;
  if (key >= KEY_A && key <= KEY_Z) {
    char ch = 'A' + key - KEY_A;
    if ( (!!(modify & MOD_SHIFT)) == (!!(key_leds & LED_CAPS_LOCK)) ) {
      ch += 'a' - 'A';
    }
    stdin_recvchar(ch);
  } else if (key >= KEY_1 && key <= KEY_0) {
    if (modify & MOD_SHIFT) {
      stdin_recvchar(ksign[key - KEY_1]);
    } else {
      stdin_recvchar(knum[key - KEY_1]);
    }
  } else if (key >= KEY_TAB && key <= KEY_SLASH) {
    if (modify & MOD_SHIFT) {
      stdin_recvchar(tabB[key - KEY_TAB]);
    } else {
      stdin_recvchar(tabA[key - KEY_TAB]);
    }
  } else if (key == KEY_ENTER) {
    stdin_recvchar('\r');
  } else if (key == KEY_CAPSLOCK) {
    key_leds ^= LED_CAPS_LOCK;
  } else if (key == KEY_NUMLOCK) {
    key_leds ^= LED_NUM_LOCK;
  } else if (key == KEY_SCROLLLOCK) {
    key_leds ^= LED_SCROLL_LOCK;
  }

  if (key_leds != last_leds) {
    tusbh_set_keyboard_led(ep, key_leds);
  }
  return 0;
}

static int process_mouse(tusbh_ep_info_t* ep, const uint8_t* mouse)
{
  uint8_t btn = mouse[0];
  int8_t x = ((int8_t*)mouse)[1];
  int8_t y = ((int8_t*)mouse)[2];
  RPC1.call("on_mouse", btn, x, y);
}

static const tusbh_boot_key_class_t cls_boot_key = {
  .backend = &tusbh_boot_keyboard_backend,
  .on_key = process_key
};

static const tusbh_boot_mouse_class_t cls_boot_mouse = {
  .backend = &tusbh_boot_mouse_backend,
  .on_mouse = process_mouse
};

static const tusbh_hid_class_t cls_hid = {
  .backend = &tusbh_hid_backend,
  //.on_recv_data = process_hid_recv,
  //.on_send_done = process_hid_sent,
};

static const tusbh_hub_class_t cls_hub = {
  .backend = &tusbh_hub_backend,
};

static const tusbh_class_reg_t class_table[] = {
  (tusbh_class_reg_t)&cls_boot_key,
  (tusbh_class_reg_t)&cls_boot_mouse,
  (tusbh_class_reg_t)&cls_hub,
  (tusbh_class_reg_t)&cls_hid,
  0,
};

void setup()
{
  Serial1.begin(115200);
  RPC1.begin();
  usb.Init(USB_CORE_ID_HS, class_table);
  //usb.Init(USB_CORE_ID_FS, class_table);
}

void loop() {
  usb.Task();
}
