#ifndef BQ25120A_h
#define BQ25120A_h

#define BQ25120A_ADDRESS           0x6A

// Register Map
// https://www.ti.com/lit/ds/symlink/bq25120a.pdf
#define BQ25120A_STATUS             0x00
#define BQ25120A_FAULTS             0x01
#define BQ25120A_TS_CONTROL         0x02
#define BQ25120A_FAST_CHG           0x03
#define BQ25120A_TERMINATION_CURR   0x04
#define BQ25120A_BATTERY_CTRL       0x05 // Battery Voltage Control Register
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

  /**
   * @brief Gets the data from the status register.
   * @see Section 9.6.1 of the datasheet.
   * 
   * @return uint8_t The data from the status register.
   */
  uint8_t getStatusRegister();
  
  /**
   * @brief Gets the data from the faults register.
   * @see Section 9.6.2 of the datasheet.
   * 
   * @return uint8_t The data from the faults register.
   */
  uint8_t getFaultsRegister();

  /**
   * @brief Gets the data from the SYS VOUT Control Register.
   * @see Section 9.6.7 of the datasheet.
   * 
   * @return uint8_t The data from the SYS VOUT Control Register.
   */
  uint8_t getLDOControlRegister();

  /**
   * @brief Gets the data from the fast charge control register.
   * @see Section 9.6.4 of the datasheet.
   * 
   * @return uint8_t The data from the fast charge control register.
   */
  uint8_t getFastChargeControlRegister();


  /**
   * @brief Determines if the board is charged from the battery.
   * 
   * @return true If the board is powered from the battery. False, when powered from USB / VIN.
   */
  bool runsOnBattery(uint8_t address);

  /**
   * @brief Writes a byte to the BQ25120A over I2C.
   * @param address The I2C address of the BQ25120A.
   * @param subAddress The memory location of the register to write to.
   * @param data The data to write to the register.
   */
  void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);

  /**
   * @brief Reads a byte from the BQ25120A over I2C.
   * 
   * @param address The I2C address of the BQ25120A.
   * @param subAddress The memory location of the register to read from.
   * @return uint8_t The data read from the register.
   */
  uint8_t readByte(uint8_t address, uint8_t subAddress);

  private:
  /**
   * @brief Set the High Impedance Mode Enabled or Disabled.
   * When enabled, drives the CD pin low to enter high impedance mode.
   * Note that this only applies when powered with a battery and the condition VIN < VUVLO is met.
   * When VIN > VUVLO this enables charging instead.
   * 
   * When disabled, drives the CD pin high to exit high impedance mode (Active Battery).
   * When VIN > VUVLO this disables charging.
   * When exiting this mode, charging resumes if VIN is present, CD is low and charging is enabled.
   * 
   * @note The CD pin is internally pulled down.
   * @param enabled Defines if the high impedance mode should be enabled or disabled.
   */
  void setHighImpedanceModeEnabled(bool enabled);

  /**
   * @brief Reads a byte from the BQ25120A over I2C without locking the bus through the mutex.
   * 
   * @param address The I2C address of the BQ25120A.
   * @param subAddress The memory location of the register to read from.
   * @return uint8_t The data read from the register.
   */
  uint8_t readByteUnprotected(uint8_t address, uint8_t subAddress);
};

#endif
