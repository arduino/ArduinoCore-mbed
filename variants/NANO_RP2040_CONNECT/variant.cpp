#include "Arduino.h"

AnalogPinDescription g_AAnalogPinDescription[] = {
  { p26,        NULL },    // A0
  { p27,        NULL },    // A1
  { p28,        NULL },    // A2
  { p29,        NULL },    // A3
};

PinDescription g_APinDescription[] = {
  // D0 - D7
  { p0,         NULL, NULL, NULL },    // D0
  { p1,         NULL, NULL, NULL },    // D1
  { p2,         NULL, NULL, NULL },    // D2
  { p3,         NULL, NULL, NULL },    // D3
  { p4,         NULL, NULL, NULL },    // D4
  { p5,         NULL, NULL, NULL },    // D5
  { p6,         NULL, NULL, NULL },    // D6
  { p7,         NULL, NULL, NULL },    // D7

  // D8 - D14
  { p8,         NULL, NULL, NULL },    // D8
  { p9,         NULL, NULL, NULL },    // D9
  { p10,        NULL, NULL, NULL },    // D10
  { p11,        NULL, NULL, NULL },    // D11
  { p12,        NULL, NULL, NULL },    // D12
  { p13,        NULL, NULL, NULL },    // D13
  { p14,        NULL, NULL, NULL },    // D14

  // D15 - D31
  { p15,        NULL, NULL, NULL },    // D15
  { p16,        NULL, NULL, NULL },    // D16
  { p17,        NULL, NULL, NULL },    // D17
  { p18,        NULL, NULL, NULL },    // D18
  { p19,        NULL, NULL, NULL },    // D19
  { p20,        NULL, NULL, NULL },    // D20
  { p21,        NULL, NULL, NULL },    // D21
  { p22,        NULL, NULL, NULL },    // D22
  { p23,        NULL, NULL, NULL },    // D23
  { p24,        NULL, NULL, NULL },    // D24
  { p25,        NULL, NULL, NULL },    // D25
  { p26,        NULL, NULL, NULL },    // D26
  { p27,        NULL, NULL, NULL },    // D27
  { p28,        NULL, NULL, NULL },    // D28
  { p29,        NULL, NULL, NULL },    // D29
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
  reset_usb_boot(0, 0);
}

#endif
