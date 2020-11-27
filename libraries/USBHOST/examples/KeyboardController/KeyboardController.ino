/*
           _______                    _    _  _____ ____
          |__   __|                  | |  | |/ ____|  _ \
             | | ___  ___ _ __  _   _| |  | | (___ | |_) |
             | |/ _ \/ _ \ '_ \| | | | |  | |\___ \|  _ <
             | |  __/  __/ | | | |_| | |__| |____) | |_) |
             |_|\___|\___|_| |_|\__, |\____/|_____/|____/
                                 __/ |
                                |___/

   TeenyUSB - light weight usb stack for STM32 micro controllers

   Copyright (c) 2019 XToolBox  - admin@xtoolbox.org
                           www.tusb.org

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include "USBHost.h"

USBHost usb;

static int process_key(tusbh_ep_info_t* ep, const uint8_t* key);

static const tusbh_boot_key_class_t cls_boot_key = {
  .backend = &tusbh_boot_keyboard_backend,
  //.on_key = process_key
};

static const tusbh_boot_mouse_class_t cls_boot_mouse = {
  .backend = &tusbh_boot_mouse_backend,
  // .on_mouse = process_mouse
};

static const tusbh_hid_class_t cls_hid = {
  .backend = &tusbh_hid_backend,
  //.on_recv_data = process_hid_recv,
  //.on_send_done = process_hid_sent,
};

static const tusbh_hub_class_t cls_hub = {
  .backend = &tusbh_hub_backend,
};

static const tusbh_vendor_class_t cls_vendor = {
  .backend = &tusbh_vendor_backend,
  //.transfer_done = process_vendor_xfer_done
};

int msc_ff_mount(tusbh_interface_t* interface, int max_lun, const tusbh_block_info_t* blocks);
int msc_ff_unmount(tusbh_interface_t* interface);

static const tusbh_msc_class_t cls_msc_bot = {
  .backend = &tusbh_msc_bot_backend,
  //  .mount = msc_ff_mount,
  //  .unmount = msc_ff_unmount,
};

static const tusbh_cdc_acm_class_t cls_cdc_acm = {
  .backend = &tusbh_cdc_acm_backend,
};

static const tusbh_cdc_rndis_class_t cls_cdc_rndis = {
  .backend = &tusbh_cdc_rndis_backend,
};

static const tusbh_class_reg_t class_table[] = {
  (tusbh_class_reg_t)&cls_boot_key,
  (tusbh_class_reg_t)&cls_boot_mouse,
  (tusbh_class_reg_t)&cls_hub,
  (tusbh_class_reg_t)&cls_msc_bot,
  (tusbh_class_reg_t)&cls_cdc_acm,
  (tusbh_class_reg_t)&cls_cdc_rndis,
  (tusbh_class_reg_t)&cls_hid,
  (tusbh_class_reg_t)&cls_vendor,
  0,
};

void setup()
{
  Serial1.begin(115200);
  usb.Init(USB_CORE_ID_HS, class_table);
  //usb.Init(USB_CORE_ID_FS, class_table);
}

void loop() {
  //usb.Task();
}

#define MOD_CTRL      (0x01 | 0x10)
#define MOD_SHIFT     (0x02 | 0x20)
#define MOD_ALT       (0x04 | 0x40)
#define MOD_WIN       (0x08 | 0x80)

#define LED_NUM_LOCK    1
#define LED_CAPS_LOCK   2
#define LED_SCROLL_LOCK 4

#define stdin_recvchar  Serial1.write

static uint8_t key_leds;
static const char knum[] = "1234567890";
static const char ksign[] = "!@#$%^&*()";
static const char tabA[] = "\t -=[]\\#;'`,./";
static const char tabB[] = "\t _+{}|~:\"~<>?";
// route the key event to stdin
static int process_key(tusbh_ep_info_t* ep, const uint8_t* keys)
{
  printf("\n");
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
