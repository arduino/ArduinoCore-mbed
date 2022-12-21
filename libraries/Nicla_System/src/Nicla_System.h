#ifndef Nicla_System_h_
#define Nicla_System_h_

#include "Arduino.h"
#include "BQ25120A.h"
#include "RGBled.h"

#include <mbed.h>
#include <I2C.h>

#define USE_FASTCHG_TO_KICK_WATCHDOG 1

#define BATTERY_FULL            5
#define BATTERY_ALMOST_FULL     4
#define BATTERY_HALF            3
#define BATTERY_ALMOST_EMPTY    2
#define BATTERY_EMPTY           1
#define BATTERY_COLD            (1 << 4)
#define BATTERY_COOL            (2 << 4)
#define BATTERY_HOT             (3 << 4)
#define BATTERY_CHARGING        (1 << 7)

class nicla {

public:
  static bool begin(bool mounted_on_mkr = false);
  static bool enable3V3LDO();
  static bool enable1V8LDO();
  static bool disableLDO();
  static bool enterShipMode();
  static uint8_t readLDOreg();
  static bool enableCharge(uint8_t mA = 20, bool disable_ntc = true);
  static int getBatteryStatus();
  static uint16_t getFault();
  static bool ntc_disabled;

  static RGBled leds;
  static BQ25120A _pmic;

  friend class RGBled;
  friend class BQ25120A;
  friend class Arduino_BHY2;

  static bool started;

private:
  static void pingI2CThd();
  static void checkChgReg();
  static rtos::Mutex i2c_mutex;
  static uint8_t _chg_reg;
};

#endif