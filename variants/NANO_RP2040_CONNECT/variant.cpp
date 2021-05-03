#include "Arduino.h"
#include "pinDefinitions.h"

AnalogPinDescription g_AAnalogPinDescription[] = {
  { p26,        NULL },    // A0
  { p27,        NULL },    // A1
  { p28,        NULL },    // A2
  { p29,        NULL },    // A3
};

PinDescription g_APinDescription[] = {
  // D0 - D7
  { p1,         NULL, NULL, NULL },    // D0
  { p0,         NULL, NULL, NULL },    // D1
  { p25,        NULL, NULL, NULL },    // D2
  { p15,        NULL, NULL, NULL },    // D3
  { p16,        NULL, NULL, NULL },    // D4
  { p17,        NULL, NULL, NULL },    // D5
  { p18,        NULL, NULL, NULL },    // D6
  { p19,        NULL, NULL, NULL },    // D7

  // D8 - D13
  { p20,        NULL, NULL, NULL },    // D8
  { p21,        NULL, NULL, NULL },    // D9
  { p5,         NULL, NULL, NULL },    // D10
  { p7,         NULL, NULL, NULL },    // D11 / SPITX
  { p4,         NULL, NULL, NULL },    // D12 / SPIRX
  { p6,         NULL, NULL, NULL },    // D13 / SPICLK / LEDB 

  // Analog as digital
  // A4 to A7 are controlled by Nina module and exposed via different APIs
  { p26,        NULL, NULL, NULL },    // A0 -> D14
  { p27,        NULL, NULL, NULL },    // A1 -> D15
  { p28,        NULL, NULL, NULL },    // A2 -> D16
  { p29,        NULL, NULL, NULL },    // A3 -> D17

  // I2C
  { p12,        NULL, NULL, NULL },    // A4 / SDA  -> D18
  { p13,        NULL, NULL, NULL },    // A5 / SCL  -> D19

  // Internal pins - D20 - D23
  { p2,         NULL, NULL, NULL },    // GPIO0
  { p24,        NULL, NULL, NULL },    // IMU IRQ
  { p22,        NULL, NULL, NULL },    // PDM DATA IN
  { p23,        NULL, NULL, NULL },    // PDM CLOCK

  // Internal pins Nina - D24 - D29
  { p3,         NULL, NULL, NULL },    // RESET_NINA
  { p8,         NULL, NULL, NULL },    // SPI1_CIPO / UART1_TX
  { p9,         NULL, NULL, NULL },    // SPI1_CS / UART1_RX
  { p10,        NULL, NULL, NULL },    // SPI1_ACK / UART1_CTS
  { p11,        NULL, NULL, NULL },    // SPI1_COPI / UART1_RTS
  { p14,        NULL, NULL, NULL },    // SPI1_SCK
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
