#include "USBHost.h"
#include "USB251xB.h"

static rtos::Thread t(osPriorityHigh);

void USBHost::supplyPowerOnVBUS(bool enable){
  mbed::DigitalOut otg(PJ_6, enable ? 0 : 1);
}

void USBHost::InternalTask() {
  while (1) {
    tusbh_msg_loop(mq);
  }
}

uint32_t USBHost::Init(uint8_t id, const tusbh_class_reg_t class_table[]) {

  mq = tusbh_mq_create();
  tusbh_mq_init(mq);

  if (id == USB_CORE_ID_FS) {
    _fs = tusb_get_host(USB_CORE_ID_FS);
    HOST_PORT_POWER_ON_FS();
    root_fs.mq = mq;
    root_fs.id = "FS";
    root_fs.support_classes = class_table;
    tusb_host_init(_fs, &root_fs);
    tusb_open_host(_fs);
    start_hub();
  }

  if (id == USB_CORE_ID_HS) {

#ifndef CORE_CM4
    get_usb_phy()->deinit();
#endif
    mbed::DigitalOut otg(PJ_6, 1);

    _hs = tusb_get_host(USB_CORE_ID_HS);
    HOST_PORT_POWER_ON_HS();
    root_hs.mq = mq;
    root_hs.id = "HS";
    root_hs.support_classes = class_table;
    tusb_host_init(_hs, &root_hs);
    tusb_open_host(_hs);
  }

  t.start(mbed::callback(this, &USBHost::InternalTask));
}



uint32_t USBHost::Task() {

}

extern "C" {
  // host need accurate delay
  void tusb_delay_ms(uint32_t ms)
  {
    delay(ms);
  }
}
