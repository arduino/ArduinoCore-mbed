#ifndef Nicla_System_h_
#define Nicla_System_h_

#include "Arduino.h"
#include "BQ25120A.h"
#include "RGBled.h"

#include <mbed.h>
#include <I2C.h>

// Deprecated. Whether or not to use a write operation by default
// can now be controlled with the default paramter of pingI2C().
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

enum class ChargingSafetyTimerOption {
  ThirtyMinutes = 0b00,
  ThreeHours = 0b01,
  NineHours = 0b10,
  Disabled = 0b11
};


class nicla {

public:

  /**
   * @brief Initializes the Nicla Sense ME board.
   * 
   * @param mountedOnMkr Defines whether the board is mounted as a shield on a MKR board or not.
   * This is used to drive the pin high that connects to the reset pin 
   * of the MKR board to prevent it from resetting.
   * @return true if the board is initialized successfully.    
   */
  static bool begin(bool mountedOnMkr = false);

  /**
   * @brief Enables the 3.3V LDO voltage regulator.
   * 
   * @return true if the LDO is enabled successfully. 
   * This is done by verifying that the register was written correctly.
   */
  static bool enable3V3LDO();

  /**
   * @brief Enables the 1.8V LDO voltage regulator.
   * 
   * @return true if the LDO is enabled successfully. 
   * This is done by verifying that the register was written correctly.
   */
  static bool enable1V8LDO();

  /**
   * @brief Disables the LDO voltage regulator.
   * 
   * @return true if the LDO is disabled successfully. 
   * This is done by verifying that the register was written correctly.
   */
  static bool disableLDO();

  /**
   * @brief Enter Ship Mode. This is used during the shipping time or the shelf time of the product.
   * This will turn off the battery FET and thus isolate the battery from the system (turns off leakage path).
   * So, whatever is leaky in your system won't be leaking out of the battery during this time.
   * 
   * @return true if the ship mode is entered successfully.
   */
  static bool enterShipMode();

  /**
   * @brief Enables fast charging of the battery. By default charging is already enabled when the board is powered.
   * The default charging current without enabling fast charging is 10mA. Charging can be disabled by calling disableCharging().
   * 
   * @param mA The desired milliampere (mA) charging current. Range: 5mA - 35mA and 40mA - 300mA. The default value is 20mA.
   * A safe default charging current value that works for most common LiPo batteries is 0.5C, which means charging at a rate equal to half of the battery's capacity.
   * For example, a 200mAh battery could be safely charged at 100mA (0.1A).
   * This charging rate is generally safe for most LiPo batteries and provides a good balance between charging speed and battery longevity.
   * @note There is a saftey timer that will stop the charging after 3 hours by default. 
   * This can be configured by calling configureChargingSafetyTimer().
   * @return true If the fast charging is enabled successfully. False, otherwise.   
   * @see disableCharging()
   */
  static bool enableCharging(uint16_t mA = 20);

  /**
   * @brief Configures the charging safety timer after which the charging is stopped.
   * This is useful to prevent overcharging the battery. The timer can have one of the following options:
   * 30 minutes, 3 hours, 9 hours or disabled.
   * 
   * @param option One of the following options: ThirtyMinutes, ThreeHours, NineHours or Disabled.
   * @return true if the charging safety timer is configured successfully, false otherwise.
   */
  static bool configureChargingSafetyTimer(ChargingSafetyTimerOption option);

  /**
   * @brief Disables charging of the battery. It can be resumed by calling enableCharging().
   * 
   * @return true If the charging is disabled successfully. False, otherwise.
   */
  static bool disableCharging();

  /**
   * @brief Determines if the board is charged from the battery.
   * 
   * @return true If the board is powered from the battery. False, when powered from USB / VIN.
   */
  static bool runsOnBattery();

  /**
   * @brief Enables or disables the negative temperature coefficient (NTC) thermistor. It is enabled by default.
   * NTCs are used to prevent the batteries from being charged at temperatures that are too high or too low.
   * Set to disabled for standard LiPo batteries without NTC.
   * If your battery has only a plus and minus wire, it does not have an NTC.
   * @note Disabling the NTC will also disable the on-charge-state-change interrupt.
   * @param enabled Whether to enabled the NTC.
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
  static int8_t getBatteryVoltagePercentage(bool useLatchedValue = false);

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
   * @brief Get the Battery Temperature using the negative temperature coefficient (NTC) thermistor. 
   * The following values are possible: "Normal", "Extreme", "Cool", "Warm".
   * When the battery is cool, the charging current is reduced by half.
   * When the battery is warm, the charging current is reduced by 140 mV.
   * When the battery is unter an extreme temperature (hot or cold), the charging is suspended.
   * @note If the battery isn't configured to have a NTC, the temperature is reported as "Normal".
   * The presence of the NTC is not determined automatically and needs to be set using the setBatteryNTCEnabled() function.
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
   * @note If it doesn't report 'Charging' even though you enabled charging with enableCharging(), the battery might be full.
   * @return OperatingStatus One of the following: Ready, Charging, ChargingComplete, Error.
   */
  static OperatingStatus getOperatingStatus();

  /// Provides access to the IS31FL3194 LED driver that drives the RGB LED.
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
   * @param useWriteOperation If true, a write operation to a register is performed to reset the watchdog timer. 
   * If false, a read operation is performed. The default is false.
   */
  static void pingI2C(bool useWriteOperation = false);
  
  [[deprecated("Use pingI2C() instead.")]]
  static void checkChgReg();

  /** 
   * A cached version of the fast charge settings for the PMIC.
   * This is used to avoid unnecessary I2C communication.
   **/
  static uint8_t _fastChargeRegisterData;

  /// Mutex to prevent concurrent access to the I2C interface.
  static rtos::Mutex _i2c_mutex;
};

#endif