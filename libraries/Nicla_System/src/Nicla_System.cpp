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
rtos::Mutex nicla::_i2c_mutex;
bool nicla::started = false;
uint8_t nicla::_fastChargeRegisterData = 0;

/// Enabled is the default value also represented in the TS Control Register (Bit 7 = 1).
bool nicla::_ntcEnabled = true;

void nicla::pingI2C(bool useWriteOperation) {
  // PMIC commands already protected by a mutex on Wire operations.
  if(useWriteOperation){
    // Write the current charging settings to the register to reset the watchdog timer.
    _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG, _fastChargeRegisterData);
  } else {
    _pmic.getStatusRegister();
  }
}

bool nicla::begin(bool mountedOnMkr)
{
  if (mountedOnMkr) {
    // GPIO3 is on MKR RESET pin, so we must configure it HIGH or it will, well, reset the board :)
    pinMode(P0_10, OUTPUT);
    digitalWrite(P0_10, HIGH);
  }
  Wire1.begin();
  _fastChargeRegisterData = _pmic.getFastChargeControlRegister();

#ifndef NO_NEED_FOR_WATCHDOG_THREAD
  // If not using the BHY2 library, we need to start a thread to ping the PMIC every 10 seconds.
  static rtos::Thread th(osPriorityHigh, 768, nullptr, "ping_thread");
  th.start([]() {
    while(1) {
      pingI2C();
      delay(10000);
    }
  });
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
  if (_pmic.getLDOControlRegister() != ldo_reg) {
    return false;
  }
  return true;
}

bool nicla::enable1V8LDO()
{
  uint8_t ldo_reg = 0xA8;
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL, ldo_reg);
  if (_pmic.getLDOControlRegister() != ldo_reg) {
    return false;
  }
  return true;
}

bool nicla::disableLDO()
{
  uint8_t ldo_reg = _pmic.getLDOControlRegister();
  ldo_reg &= 0x7F; // Zeroes the EN_LS_LDO bit to turn it off
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL, ldo_reg);
  return _pmic.getLDOControlRegister() == ldo_reg;
}

bool nicla::enterShipMode()
{
  //    STATUS reg:
  //    | B7 | B6 |      B5     | B4 | B3 | B2 | B1 | B0 |
  //    | RO | RO | EN_SHIPMODE | RO | RO | RO | RO | RO |

  uint8_t status_reg = _pmic.getStatusRegister();
  status_reg |= 0x20;
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_STATUS, status_reg);
  if (_pmic.readByte(BQ25120A_ADDRESS, BQ25120A_STATUS) != status_reg) {
    return false;
  }
  return true;
}

bool nicla::enableCharging(uint16_t mA)
{
  /*
  The ICHRG is calculated using the following equation: 
  - If ICHRG_RANGE (Bit 7) is 0, then ICHRG = 5 mA + ICHRGCODE x 1 mA. 
  - If ICHRG_RANGE (Bit 7) is 1, then ICHRG = 40 mA + ICHRGCODE x 10 mA. 
  - If a value greater than 35 mA (ICHRG_RANGE = 0) or 300 mA (ICHRG_RANGE = 1) is written, 
    the setting goes to 35 mA or 300 mA respectively, except if the ICHRG bits are all 1 (that is, 11111), 
    then the externally programmed value is used. See section 9.6.4 in the datasheet.
  */

  if (mA > 300) {
    mA = 300;
  }

  if (mA < 5) {
    mA = 5;
  }

  if(mA > 35 && mA < 40) {
    mA = 35;
  }

  if (mA <= 35) {
    // Values 5 mA to 35 mA
    _fastChargeRegisterData = ((mA-5) << 2); // e.g. 20mA - 5mA = 15mA << 2 -> 0b00111100
  } else {
    // Values 40 mA to 300 mA
    // e.g. (200mA - 40mA) / 10 = 16mA << 2 -> 0b01000000 | 0x80 -> 0b11000000
    _fastChargeRegisterData = (((mA-40)/10) << 2) | 0x80;
  }

  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG, _fastChargeRegisterData);

  // For very depleted batteries, set BULVO to the very minimum to re-enable charging.
  // 2.2V or 2.0V are the minimum values for BULVO. The latter is not mentioned in the datasheet
  // but it looks like a typo since 2.2V is mentioned twice. See: Table 22 in the datasheet.
  // Also sets the input current limit to 350mA.
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_ILIM_UVLO_CTRL, 0x3F);

  return _pmic.getFastChargeControlRegister() == _fastChargeRegisterData;
}

bool nicla::configureChargingSafetyTimer(ChargingSafetyTimerOption option){
  // See: Table 24 in the datasheet.
  // The two bits need to be shifted to skip the unused LSB.
  uint8_t timerValue = static_cast<uint8_t>(option) << 1;
  uint8_t dpmTimerRegisterData = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_VIN_DPM);

  dpmTimerRegisterData &= 0b11111001; // Clear bits 1 and 2
  dpmTimerRegisterData |= timerValue; // Update bits 1 and 2
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_VIN_DPM, dpmTimerRegisterData);

  return _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_VIN_DPM) == dpmTimerRegisterData;
}

bool nicla::disableCharging()
{
  // Set Bit 1 to 1 to disable charging.
  _fastChargeRegisterData |= 0b10;
  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG, _fastChargeRegisterData);
  return _pmic.getFastChargeControlRegister() == _fastChargeRegisterData;
}

bool nicla::runsOnBattery() {
  return _pmic.runsOnBattery(BQ25120A_ADDRESS);
}

uint8_t nicla::getBatteryFaults() {
  // Skips the mask bits (4 LSBs)
  return (_pmic.getFaultsRegister() >> 4) & 0b1111;
}

void nicla::setBatteryNTCEnabled(bool enabled){
  if (_ntcEnabled != enabled) {
    _ntcEnabled = enabled;

    // Read the current TS_CONTROL register value
    uint8_t tsControlRegister = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_TS_CONTROL);

    if (_ntcEnabled) {
      // Set bit 7 and bit 3 to 1 to enable temperature sense and interrupt on charge status change.
      tsControlRegister |= 0b10001000;
    } else {
      // Set bit 7 and bit 3 to 0 to disable temperature sense and interrupt on charge status change.
      // INT only shows faults and does not show charge status.
      tsControlRegister &= 0b01110111;
    }

    _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_TS_CONTROL, tsControlRegister);
  }
}

float nicla::getRegulatedBatteryVoltage(){
  /*
  According to https://www.ti.com/lit/ds/symlink/bq25120a.pdf Page 40:

  +---------+--------------------+
  |   Bit   | Regulation Voltage |
  +---------+--------------------+
  | 7 (MSB) | 640 mV             |
  | 6       | 320 mV             |
  | 5       | 160 mV             |
  | 4       | 80 mV              |
  | 3       | 40 mV              |
  | 2       | 20 mV              |
  | 1       | 10 mV              |
  | 0 (LSB) | –                  |
  +---------+--------------------+

  // Example: 01111000 results in
  // 3.6 + 0.32 + 0.16 + 0.08 + 0.04 = 4.2V which is the default value after reset
  */

  // Read the Battery Voltage Control Register that holds the regulated battery voltage
  uint8_t data = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_BATTERY_CTRL);
  int milliVolts = 3600; // 3.6V is the minimum voltage
  
  // Shift the data to the right by 1 bit to remove the LSB that is not used.
  uint8_t shiftedData = (data >> 1) & 0b01111111;
  milliVolts += shiftedData * 10;

  return milliVolts / 1000.0f;

}

void nicla::setRegulatedBatteryVoltage(float voltage){
  if (voltage < 3.6f){
    voltage = 3.6f;
  } else if (voltage > 4.2f) {
    voltage = 4.2f;
  }

  // The formula is: VBATREG = 3.6 V + Sum(VBREG[Bit 7:1])

  /*
  +---------+--------------------+
  |   Bit   | Regulation Voltage |
  +---------+--------------------+
  | 7 (MSB) | 640 mV             |
  | 6       | 320 mV             |
  | 5       | 160 mV             |
  | 4       | 80 mV              |
  | 3       | 40 mV              |
  | 2       | 20 mV              |
  | 1       | 10 mV              |
  | 0 (LSB) | –                  |
  +---------+--------------------+
  */

  uint16_t voltageAddition = (voltage - 3.6f) * 100;
  // Shift one bit to the left because the LSB is not used.
  uint8_t value = voltageAddition << 1;
  // e.g. 4.2V - 3.6V = 0.6V * 100 = 60. 60 << 1 = 120 = 01111000

  _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_BATTERY_CTRL, value);
}

float nicla::getCurrentBatteryVoltage(){
  auto percentage = getBatteryVoltagePercentage();
  if (percentage < 0) {
    return 0;
  }
  return getRegulatedBatteryVoltage() / 100 * percentage;
}

int8_t nicla::getBatteryVoltagePercentage(bool useLatchedValue) {
  /*
  * 9.3.4 Voltage Based Battery Monitor (Page 20)
  * The device implements a simple voltage battery monitor which can be used to determine the depth of discharge.
  * Prior to entering High-Z mode, the device will initiate a VBMON reading. The host can read the latched value for
  * the no-load battery voltage, or initiate a reading using VBMON_READ to see the battery voltage under a known
  * load. The register will be updated and can be read 2ms after a read is initiated. The VBMON voltage threshold is
  * readable with 2% increments with ±1.5% accuracy between 60% and 100% of VBATREG using the VBMON_TH
  * registers. Reading the value during charge is possible, but for the most accurate battery voltage indication, it is
  * recommended to disable charge, initiate a read, and then re-enable charge.
  */
  /*
  According to https://www.ti.com/lit/ds/symlink/bq25120a.pdf Page 45:
  MSB = Bit 7, LSB = Bit 0

  +----------+------------------------+
  | Bits 5+6 |        Meaning         |
  +----------+------------------------+
  |       11 | 90% to 100% of VBATREG |
  |       10 | 80% to 90% of VBATREG  |
  |       01 | 70% to 80% of VBATREG  |
  |       00 | 60% to 70% of VBATREG  |
  +----------+------------------------+

  +----------+-------------------------+
  | Bits 2-4 |         Meaning         |
  +----------+-------------------------+
  |      111 | Above 8% of VBMON_RANGE |
  |      110 | Above 6% of VBMON_RANGE |
  |      011 | Above 4% of VBMON_RANGE |
  |      010 | Above 2% of VBMON_RANGE |
  |      001 | Above 0% of VBMON_RANGE |
  +----------+-------------------------+

  Example: 0 11 111 00 -> 90% + 8% = 98 - 100% of VBATREG
  */
  constexpr uint8_t BAT_UVLO_FAULT = 0b00100000; // Battery Under-Voltage Lock-Out fault
  uint8_t faults = _pmic.getFaultsRegister();
  if(faults & BAT_UVLO_FAULT) return -1; // Battery is not connected or voltage is too low

  
  if(!useLatchedValue){
    // Disable charging while reading battery percentage. SEE chapter 9.3.4    
    bool chargingEnabled = (_fastChargeRegisterData & 0b10) == 0; // Bit 1 is 0 if charging is enabled.
    
    if(chargingEnabled) {
      disableCharging();
    }
    // Write 1 to bit 7 VBMON_READ to trigger a new reading
    _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_BATT_MON, 0b10000000);
    
    delay(3); // According to datasheet, 2ms is enough, but we add 1ms for safety
    
    if(chargingEnabled) {
      // Re-enable charging by setting bit 1 to 0
      _fastChargeRegisterData &= 0b11111101;
      _pmic.writeByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG, _fastChargeRegisterData);
    }
  }
  uint8_t data = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_BATT_MON);

  // If bit 2 - 7 are all zeroes, the battery status could not be read
  if((data & 0b11111100) == 0) return -2;

   // Extract bits 5 and 6
   // Masking is optional because the MSB is always 0
  uint8_t bits56 = (data >> 5) & 0b11;

  // Extract bits 2 to 4
  uint8_t bits234 = (data >> 2) & 0b111;
  
  // FIXME: The pattern 000 is not defined in the datasheet but still occurs
  // along with a valid bits56 pattern. We assume that it means 0%.
  // if(bits234 == 0b000) return -1; // Battery status could not be read

  // Lookup tables for mapping bit patterns to percentage values
  // The datasheet says that the threshold values are above what's written in the table.
  // Therefore we use the next higher value in steps of 2%. 
  // That way the final percentage is always rounded up and can reach 100%.
  int thresholdLookup[] = {0, 2, 4, 6, 0, 0, 8, 10};

  // bits56 has a range of 0 to 3, so we multiply it by 10 and add 60 to get a range of 60 to 90
  int percentageTens = 60 + (bits56 * 10);
  // Map bit patterns to percentage values using lookup table
  int percentageOnes = thresholdLookup[bits234];

  // Calculate the final percentage
 return percentageTens + percentageOnes;
}

BatteryChargeLevel nicla::getBatteryChargeLevel() {
  auto percent = getBatteryVoltagePercentage();

  if (percent >= 98) {
    return BatteryChargeLevel::Full;
  } else if (percent >= 94){
    return BatteryChargeLevel::AlmostFull;
  } else if (percent >= 90){
    return BatteryChargeLevel::HalfFull;
  } else if (percent >= 86){
    return BatteryChargeLevel::AlmostEmpty;
  } else if(percent < 86 && percent > 0)  {
    // < 84% is considered empty
    return BatteryChargeLevel::Empty;
  } else {
    // Battery status could not be read
    return BatteryChargeLevel::Unknown;
  }
}

BatteryTemperature nicla::getBatteryTemperature() {
  if(!_ntcEnabled) return BatteryTemperature::Normal;
  
  /*
  +------+-------------------------------------------------------------+
  | Bits |                         Description                         |
  +------+-------------------------------------------------------------+
  |   00 | Normal, No TS fault                                         |
  |   01 | TS temp < TCOLD or TS temp > THOT (Charging suspended)      |
  |   10 | TCOOL > TS temp > TCOLD (Charging current reduced by half)  |
  |   11 | TWARM < TS temp < THOT (Charging voltage reduced by 140 mV) |
  +------+-------------------------------------------------------------+
  */
  // Extract bits 5 and 6 (TS_FAULT0 and TS_FAULT1)
  uint8_t temperatureSenseFault = _pmic.readByte(BQ25120A_ADDRESS, BQ25120A_TS_CONTROL) >> 5 & 0b11;
  return static_cast<BatteryTemperature>(temperatureSenseFault);
}


OperatingStatus nicla::getOperatingStatus() {
  // Extract bits 6 and 7
  uint8_t status = _pmic.getStatusRegister() >> 6 & 0b11;
  return static_cast<OperatingStatus>(status);
}

void nicla::checkChgReg(){
  pingI2C();
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
