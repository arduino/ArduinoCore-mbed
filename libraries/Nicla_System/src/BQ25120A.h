#ifndef BQ25120A_h
#define BQ25120A_h

#define BQ25120A_ADDRESS           0x6A

// Regsiter Map
// https://www.ti.com/lit/ds/symlink/bq25120a.pdf?ts=1610608851953&ref_url=https%253A%252F%252Fwww.startpage.com%252F
#define BQ25120A_STATUS             0x00
#define BQ25120A_FAULTS             0x01
#define BQ25120A_TS_CONTROL         0x02
#define BQ25120A_FAST_CHG           0x03
#define BQ25120A_TERMINATION_CURR   0x04
#define BQ25120A_BATTERY_CTRL       0x05
#define BQ25120A_SYS_VOUT_CTRL      0x06
#define BQ25120A_LDO_CTRL           0x07
#define BQ25120A_PUSH_BUTT_CTRL     0x08
#define BQ25120A_ILIM_UVLO_CTRL     0x09
#define BQ25120A_BATT_MON           0x0A
#define BQ25120A_VIN_DPM            0x0B

#include "Wire.h"

class BQ25120A
{
  public:
  BQ25120A() {};

  uint8_t getStatus();
  void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);
  uint8_t readByte(uint8_t address, uint8_t subAddress);

};

#endif