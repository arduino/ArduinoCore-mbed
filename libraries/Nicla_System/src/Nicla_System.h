#ifndef Nicla_System_h_
#define Nicla_System_h_

#include "Arduino.h"
#include "BQ25120A.h"
#include "RGBled.h"

#include <mbed.h>
#include <I2C.h>

#define USE_FASTCHG_TO_KICK_WATCHDOG 1

enum class OperatingStatus {
  Ready = 0b00,
  Charging = 0b01,
  ChargingComplete = 0b10,
  Error = 0b11
};

// 3 bits are used to indicate the battery charge level
enum class BatteryChargeLevel {
  Unknown = 0b000,
  Empty = 0b001,
  AlmostEmpty = 0b010,
  HalfFull = 0b011,
  AlmostFull = 0b100,
  Full = 0b101
};

// 2 bits are used to indicate the battery temperature
enum class BatteryTemperature {
  Normal = 0b00,
  Extreme = 0b01,
  Cool = 0b10,
  Warm = 0b11
};

class nicla {

public:
  static bool begin(bool mountedOnMkr = false);
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
  static bool enableCharge(uint8_t mA = 20, bool disableNtc = true);

  /**
   * @brief Enables or disables the negative temperature coefficient (NTC) thermistor.
   * NTCs are used to prevent the batteries from being charged at temperatures that are too high or too low.
   * Set to disabled for standard LiPo batteries without NTC.
   * If your battery has only a plus and minus wire, it does not have an NTC.
   * The default is enabled.
   */
  static void setBatteryNTCEnabled(bool enabled);

  /**
   * @brief Get the Regulated Battery Voltage in Volts.
   * 
   * @return float The regulated battery voltage in Volts. The default regulated voltage is 4.2V.
   */
  static float getRegulatedBatteryVoltage();

  /**
   * @brief Set the Regulated Battery Voltage.
   * 
   * @param voltage The voltage in the range of 3.6V to 4.65V.
   */
  static void setRegulatedBatteryVoltage(float voltage);

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
   * @return BatteryChargeLevel The battery charge level represented by one of the following values:
   * BatteryChargeLevel::Unknown, BatteryChargeLevel::Empty, BatteryChargeLevel::AlmostEmpty, 
   * BatteryChargeLevel::HalfFull, BatteryChargeLevel::AlmostFull, BatteryChargeLevel::Full
   */
  static BatteryChargeLevel getBatteryChargeLevel();

  /**
   * @brief Get the Battery Temperature. The following values are possible:
   * "Normal", "Extreme", "Cool", "Warm".
   * When the battery is cool, the charging current is reduced by half.
   * When the battery is warm, the charging current is reduced by 140 mV.
   * When the battery is unter an extreme temperature (hot or cold), the charging is suspended.
   * @note If the battery doesn't have a negative temperature coefficient (NTC) thermistor, the temperature is always "Normal".
   * This is not determined automatically and needs to be set using the setBatteryNTCEnabled() function.
   * @see setBatteryNTCEnabled()
   * @return BatteryTemperature The battery temperature represented by one of the following constants:
   * BatteryTemperature::Normal, BatteryTemperature::Extreme, BatteryTemperature::Cool, BatteryTemperature::Warm
   */
  static BatteryTemperature getBatteryTemperature();

  /**
   * @brief Returns potential battery faults retrieved from the fault register.
   * 
   * - Bit 3: 1 - VIN overvoltage fault. VIN_OV continues to show fault after an I2C read as long as OV exists
   * - Bit 2: 1 - VIN undervoltage fault. VIN_UV is set when the input falls below VSLP. VIN_UV fault shows only one time. Once read, VIN_UV clears until the the UVLO event occurs.
   * - Bit 1: 1 – BAT_UVLO fault. BAT_UVLO continues to show fault after an I2C read as long as BAT_UVLO conditions exist.
   * - Bit 0: 1 – BAT_OCP fault. BAT_OCP is cleared after I2C read.
   * 
   * @note Some of the registers are not persistent. See chapter 9.6.2 and 9.6.3 of the datasheet.
   * @return uint8_t The battery faults encoded in a 16bit integer.
   */
  static uint8_t getBatteryFaults();


  /**
   * @brief Get the current operating status of the PMIC.
   * 
   * @return OperatingStatus One of the following: Ready, Charging, ChargingComplete, Error.
   */
  static OperatingStatus getOperatingStatus();

  static RGBled leds;
  static BQ25120A _pmic;
  
  /// Flag to check if the begin function has been called. This is used to automatically call the begin function if necessary.
  static bool started;

  friend class RGBled;
  friend class BQ25120A;
  friend class Arduino_BHY2;


private:
  /// Defines if the connected battery has a negative temperature coefficient (NTC) thermistor.
  static bool _ntcEnabled;

  /**
   * @brief Pings the I2C interface by querying the PMIC's fast charge register every 10 seconds.
   * This is invoked by a thread and is meant to kick the watchdog timer to prevent the PMIC from entering a low power state.
   * The I2C interface reset timer for the host is 50 seconds.
   */
  static void pingI2C();
  
  /**
   * @brief Synchronizes the fast charge settings with the PMIC.
   * This ensures that the fast charge settings as specified via enableCharge() are applied again the register got wiped.
   */
  static void synchronizeFastChargeSettings();

  /** 
   * A cached version of the fast charge settings for the PMIC.
   * This is used to reapply the settings if the register got wiped.
   **/
  static uint8_t _fastChargeRegisterData;

  /// Mutex to prevent concurrent access to the I2C interface.
  static rtos::Mutex _i2c_mutex;
};

#endif