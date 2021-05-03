#include "Arduino.h"
#include "pinDefinitions.h"

AnalogPinDescription g_AAnalogPinDescription[] = {
  { p26,        NULL },    // A0
  { p27,        NULL },    // A1
  { p28,        NULL },    // A2
  { p29,        NULL },    // A3
};

PinDescription g_APinDescription[] = {
  // D0 - D29
  { p0,  NULL, NULL, NULL },
  { p1,  NULL, NULL, NULL },
  { p2,  NULL, NULL, NULL },
  { p3,  NULL, NULL, NULL },
  { p4,  NULL, NULL, NULL },
  { p5,  NULL, NULL, NULL },
  { p6,  NULL, NULL, NULL },
  { p7,  NULL, NULL, NULL },
  { p8,  NULL, NULL, NULL },
  { p9,  NULL, NULL, NULL },
  { p10, NULL, NULL, NULL },
  { p11, NULL, NULL, NULL },
  { p12, NULL, NULL, NULL },
  { p13, NULL, NULL, NULL },
  { p14, NULL, NULL, NULL },
  { p15, NULL, NULL, NULL },
  { p16, NULL, NULL, NULL },
  { p17, NULL, NULL, NULL },
  { p18, NULL, NULL, NULL },
  { p19, NULL, NULL, NULL },
  { p20, NULL, NULL, NULL },
  { p21, NULL, NULL, NULL },
  { p22, NULL, NULL, NULL },
  { p23, NULL, NULL, NULL },
  { p24, NULL, NULL, NULL },
  { p25, NULL, NULL, NULL },
  { p26, NULL, NULL, NULL },
  { p27, NULL, NULL, NULL },
  { p28, NULL, NULL, NULL },
  { p29, NULL, NULL, NULL },
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}

#include "drivers/I2C.h"


void initVariant() {
}

#ifdef SERIAL_CDC

static void utox8(uint32_t val, uint8_t* s) {
  for (int i = 0; i < 16; i=i+2) {
    int d = val & 0XF;
    val = (val >> 4);

    s[15 - i -1] = d > 9 ? 'A' + d - 10 : '0' + d;
    s[15 - i] = '\0';
  }
}

extern "C" {
#include "hardware/flash.h"
#include "pico/bootrom.h"
}

uint8_t getUniqueSerialNumber(uint8_t* name) {
  uint32_t id[2];
  flash_get_unique_id((uint8_t*)&id[0]);
  utox8(id[0], &name[0]);
  utox8(id[1], &name[16]);
  return 32;
}

void _ontouch1200bps_() {
  reset_usb_boot(1 << digitalPinToPinName(LED_BUILTIN), 0);
}

#endif
