#include "USBHost.h"

uint32_t USBHost::Init(const tusbh_class_reg_t class_table[]) {
  get_usb_phy()->deinit();

  mq = tusbh_mq_create();
  tusbh_mq_init(mq);

#if defined(USB_CORE_ID_FS)
  _fs = tusb_get_host(USB_CORE_ID_FS);
  HOST_PORT_POWER_ON_FS();
  root_fs.mq = mq;
  root_fs.id = "FS";
  root_fs.support_classes = class_table;
  tusb_host_init(_fs, &root_fs);
  tusb_open_host(_fs);
#else
  (void)root_fs;
  _fs = 0;
#endif

#if defined(USB_CORE_ID_HS)
  _hs = tusb_get_host(USB_CORE_ID_HS);
  HOST_PORT_POWER_ON_HS();
  root_hs.mq = mq;
  root_hs.id = "HS";
  root_hs.support_classes = class_table;
  tusb_host_init(_hs, &root_hs);
  tusb_open_host(_hs);
#else
  (void)root_hs;
  hs = 0;
#endif
}

uint32_t USBHost::Task() {
	tusbh_msg_loop(mq);
}

extern "C" {
  // host need accurate delay
  void tusb_delay_ms(uint32_t ms)
  {
    delayMicroseconds(ms*1000);
  }
}
