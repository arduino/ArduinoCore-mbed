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

bool nicla::begin()
{
  pinMode(p25, OUTPUT);
  pinMode(P0_10, OUTPUT);
  digitalWrite(P0_10, HIGH);
  Wire1.begin();
  _chg_reg = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG);
#ifndef NO_NEED_FOR_WATCHDOG_THREAD
  static rtos::Thread th(osPriorityHigh, 1024, nullptr, "ping_thread");
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

bool nicla::enableCharge(uint8_t mA)
{
  digitalWrite(p25, LOW);
  if (mA < 35) {
    _chg_reg = ((mA-5) << 2);
  } else {
    _chg_reg = (((mA-40)/10) << 2) | 0x80;
  }
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG, _chg_reg);
  return _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG) == _chg_reg;
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
