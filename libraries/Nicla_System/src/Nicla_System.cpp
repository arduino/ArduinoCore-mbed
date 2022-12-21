#include "Nicla_System.h"

#if defined __has_include
#  if __has_include (<Arduino_BHY2.h>)
#    define NO_NEED_FOR_WATCHDOG_THREAD
#  else
#    include "rtos.h"
#  endif
#endif


RGBled nicla::leds;
BQ25120A nicla::_pmic;
rtos::Mutex nicla::i2c_mutex;
bool nicla::started = false;
uint8_t nicla::_chg_reg = 0;

void nicla::pingI2CThd() {
  while(1) {
    // already protected by a mutex on Wire operations
    checkChgReg();
    delay(10000);
  }
}

bool nicla::begin(bool mounted_on_mkr)
{
  if (mounted_on_mkr) {
    // GPIO3 is on MKR RESET pin, so we must configure it HIGH or it will, well, reset the board :)
    pinMode(p25, OUTPUT);
    pinMode(P0_10, OUTPUT);
    digitalWrite(P0_10, HIGH);
  }
  Wire1.begin();
  _chg_reg = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG);
#ifndef NO_NEED_FOR_WATCHDOG_THREAD
  static rtos::Thread th(osPriorityHigh, 768, nullptr, "ping_thread");
  th.start(&nicla::pingI2CThd);
#endif
  started = true;

  return true;
}

  /*
      LDO reg:
      |   B7   |  B6   |  B5   |  B4   |  B3   |  B2   | B1  | B0  |
      | EN_LDO | LDO_4 | LDO_3 | LDO_2 | LDO_1 | LDO_0 |  X  |  X  |

      Conversion function:
      LDO = 0.8V + LDO_CODE * 100mV

      - for LDO = 3.3V:
          - set LCO_CODE = 25 (0x19)
          - shift to lef by 2 positions: (0x19 << 2) = 0x64
          - set EN_LDO: 0xE4
      - for LDO = 1.8V:
          - set LCO_CODE = 10 (0x0A)
          - shift to lef by 2 positions: (0x0A << 2) = 0x28
          - set EN_LDO: 0xA8
  */

bool nicla::enable3V3LDO()
{
  uint8_t ldo_reg = 0xE4;
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL, ldo_reg);
  if (_pmic.readByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL) != ldo_reg) {
    return false;
  }
  return true;
}

bool nicla::enable1V8LDO()
{
  uint8_t ldo_reg = 0xA8;
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL, ldo_reg);
  if (_pmic.readByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL) != ldo_reg) {
    return false;
  }
  return true;
}

bool nicla::disableLDO()
{
  uint8_t ldo_reg = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL);
  ldo_reg &= 0x7F;
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL, ldo_reg);
  if (_pmic.readByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL) != ldo_reg) {
    return false;
  }
  return true;
}

bool nicla::enterShipMode()
{
  //    STATUS reg:
  //    | B7 | B6 |      B5     | B4 | B3 | B2 | B1 | B0 |
  //    | RO | RO | EN_SHIPMODE | RO | RO | RO | RO | RO |

  uint8_t status_reg = _pmic.getStatus();
  status_reg |= 0x20;
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_STATUS, status_reg);
}

uint8_t nicla::readLDOreg()
{
  return _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL);
}

bool nicla::ntc_disabled;

bool nicla::enableCharge(uint8_t mA, bool disable_ntc)
{
  if (mA < 5) {
    _chg_reg = 0x3;
  } else if (mA < 35) {
    _chg_reg = ((mA-5) << 2);
  } else {
    _chg_reg = (((mA-40)/10) << 2) | 0x80;
  }
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG, _chg_reg);

  // For very depleted batteries, set ULVO at the very minimum to re-enable charging
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_ILIM_UVLO_CTRL, 0x3F);

  // Disable TS and interrupt on charge
  ntc_disabled = disable_ntc;
  if (ntc_disabled) {
    _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_TS_CONTROL, 1 << 3);
  }

  // also set max battery voltage to 4.2V (VBREG)
  // _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_BATTERY_CTRL, (4.2f - 3.6f)*100);

  return _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG) == _chg_reg;
}

uint16_t nicla::getFault() {
  uint16_t tmp = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_FAULTS) << 8;
  tmp |= (_pmic.readByte(BQ25120A_ADDRESS, BQ25120A_TS_CONTROL) & 0x60);
  return tmp;
}

int nicla::getBatteryStatus() {
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_BATT_MON, 1);
  delay(3);
  uint8_t data = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_BATT_MON);
  float percent = 0.6f + (data >> 5) * 0.1f + ((data >> 2) & 0x7) * 0.02f;

  int res = 0;
  if (percent >= 0.98) {
    res |= BATTERY_FULL;
  } else if (percent >= 0.94){
    res |= BATTERY_ALMOST_FULL;
  } else if (percent >= 0.90){
    res |= BATTERY_HALF;
  } else if (percent >= 0.86){
    res |= BATTERY_ALMOST_EMPTY;
  } else {
    res |= BATTERY_EMPTY;
  }

  if (!ntc_disabled) {
    auto ts = ((_pmic.readByte(BQ25120A_ADDRESS, BQ25120A_TS_CONTROL) >> 5) & 0x3);
    if (ts == 1) {
      res |= BATTERY_COLD;
    } else if (ts == 2) {
      res |= BATTERY_COOL;
    } else if (ts == 3) {
      res |= BATTERY_HOT;
    }
  }

  return res;
}

void nicla::checkChgReg()
{
  if (_chg_reg != _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG)) {
    _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG, _chg_reg);
  }
}

I2CLed  LEDR(red);
I2CLed  LEDG(green);
I2CLed  LEDB(blue);
I2CLed  LED_BUILTIN(white);

void pinMode(I2CLed pin, PinMode mode)
{
  if (!nicla::started) {
    nicla::begin();
    nicla::leds.begin();
    nicla::leds.setColor(off);
  }
}

void digitalWrite(I2CLed pin, PinStatus value)
{
  switch (pin.get()) {
    case red:
      nicla::leds.setColorRed(value == LOW ? 0 : 0xFF);
      break;
    case blue:
      nicla::leds.setColorBlue(value == LOW ? 0 : 0xFF);
      break;
    case green:
      nicla::leds.setColorGreen(value == LOW ? 0 : 0xFF);
      break;
    case white:
      if (value == LOW) {
        nicla::leds.setColor(0, 0, 0);
      } else {
        nicla::leds.setColor(0xFF, 0xFF, 0xFF);
      }
      break;
  }
}
