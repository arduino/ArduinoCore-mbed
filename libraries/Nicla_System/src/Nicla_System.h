#ifndef Nicla_System_h_
#define Nicla_System_h_

#include "Arduino.h"
#include "BQ25120A.h"
#include "RGBled.h"

#include <mbed.h>
#include <I2C.h>

#define USE_FASTCHG_TO_KICK_WATCHDOG 1

// 3 bits are used to indicate the battery charge level
#define BATTERY_CHARGE_MASK     0b00000111
#define BATTERY_FULL            5 // Bit pattern: 101
#define BATTERY_ALMOST_FULL     4 // Bit pattern: 100
#define BATTERY_HALF            3 // Bit pattern: 011
#define BATTERY_ALMOST_EMPTY    2 // Bit pattern: 010
#define BATTERY_EMPTY           1 // Bit pattern: 001
#define BATTERY_UNKNOWN         0 // Bit pattern: 000

// 2 bits are used to indicate the battery temperature
#define BATTERY_TEMPERATURE_MASK    0b00011000
#define BATTERY_TEMPERATURE_NORMAL  (0 << 4)
#define BATTERY_TEMPERATURE_EXTREME (1 << 4)
#define BATTERY_TEMPERTURE_COOL     (2 << 4)
#define BATTERY_TEMPERTURE_WARM     (3 << 4)

#define BATTERY_CHARGING        (1 << 7)

class nicla {

public:
  static bool begin(bool mounted_on_mkr = false);
  static bool enable3V3LDO();
  static bool enable1V8LDO();
  static bool disableLDO();

  /**
   * @brief Enter Ship Mode. This is used during the shipping time or the shelf time of the product.
   * This will turn off the battery FET and thus isolate the battery from the system (turns off leakage path).
   * So, whatever is leaky in your system won't be leaking out of the battery during this time.
   * 
   * @return true if the ship mode is entered successfully.
   */
  static bool enterShipMode();
  static uint8_t readLDOreg();
  static bool enableCharge(uint8_t mA = 20, bool disable_ntc = true);

  /**
   * @brief Get the Regulated Battery Voltage in Volts.
   * 
   * @return float The regulated battery voltage in Volts. The default regulated voltage is 4.2V.
   */
  static float getRegulatedBatteryVoltage();

  /**
   * @brief Get the Current Battery Voltage in Volts. This value is calculated by multiplying
   * the regulated voltage by the battery percentage.
   * 
   * @return float The current battery voltage in Volts.
   */
  static float getCurrentBatteryVoltage();

  /**
   * @brief Get the percentage of the battery's regulated voltage under a known load.
   * 
   * The accuracy of the battery voltage monitor (VBAT monitor) is between -3.5% and +3.5% of the regulated voltage (VBATREG).
   * @note This does not denote the actual battery charge level but the percentage of the fully charged voltage.
   * Many common LiPo batteries have a nominal voltage of 3.7V and a fully charged voltage of 4.2V.
   * For a 4.2V regulated voltage battery < 84% is considered empty. < 60% is considered critical; the battery may be damaged.
   * @param useLatchedValue Before entering a low power state (High Impedance mode), the device will determine the voltage
   * and store it in a latched register. If true, the latched (stored) value is returned.
   * If false, a new reading is taken from the PMIC. The default is false, so a new reading is taken.
   * @return int8_t The percentage of the regulated voltage in the range of 60% to 100%.
   * A value of < 0 indicates that the battery percentage could not be determined.
   */
  static int8_t getBatteryPercentage(bool useLatchedValue = false);

  /**
   * @brief Get the Battery Charge level encoded as a number (0-5). The following values are possible:
   * "Unknown", "Empty", "Almost Empty", "Half Full", "Almost Full", "Full"
   * 
   * @return uint8_t The battery charge level represented by one of the following constants:
   * BATTERY_UNKNOWN, BATTERY_FULL, BATTERY_ALMOST_FULL, BATTERY_HALF, BATTERY_ALMOST_EMPTY, BATTERY_EMPTY
   */
  static uint8_t getBatteryChargeLevel();

  /**
   * @brief Get the Battery Temperature. The following values are possible:
   * "Normal", "Extreme", "Cool", "Warm".
   * When the battery is cool, the charging current is reduced by half.
   * When the battery is warm, the charging current is reduced by 140 mV.
   * When the battery is unter an extreme temperature (hot or cold), the charging is suspended.
   * @note If the battery doesn't have a negative temperature coefficient (NTC) thermistor, the temperature is always "Normal".
   * @return uint8_t The battery temperature represented by one of the following constants:
   * BATTERY_TEMPERATURE_NORMAL, BATTERY_TEMPERATURE_EXTREME, BATTERY_TEMPERTURE_COOL, BATTERY_TEMPERTURE_WARM
   */
  static uint8_t getBatteryTemperature();

  /**
   * @brief Get the Battery Status (charge level and temperature).
   * The first 3 bits indicate the battery charge level. They can be retrieved using the BATTERY_CHARGE_MASK. 
   * The 4th and 5th bit indicate the battery temperature. They can be retrieved using the BATTERY_TEMPERATURE_MASK.
   * @see getBatteryChargeLevel()
   * @see getBatteryTemperature()
   * @return uint8_t The battery status containing the charge level and temperature. 
   */
  static uint8_t getBatteryStatus();


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