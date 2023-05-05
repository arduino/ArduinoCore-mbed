#include <mbed.h>
#include <I2C.h>
#include "BQ25120A.h"
#include "Nicla_System.h"
#include "DigitalOut.h"

// Set the CD pin to low to enter high impedance mode
// Note that this only applies when powered with a battery 
// and the condition VIN < VUVLO is met.
// When VIN > VUVLO this enables charging.
static mbed::DigitalOut cd(p25, 0);

uint8_t BQ25120A::getStatusRegister()
{
  return readByte(BQ25120A_ADDRESS, BQ25120A_STATUS);
}

uint8_t BQ25120A::getFaultsRegister()
{
  return readByte(BQ25120A_ADDRESS, BQ25120A_FAULTS);
}

uint8_t BQ25120A::getFastChargeControlRegister()
{
  return readByte(BQ25120A_ADDRESS, BQ25120A_FAST_CHG);
}

uint8_t BQ25120A::getLDOControlRegister()
{
  return readByte(BQ25120A_ADDRESS, BQ25120A_LDO_CTRL);
}

bool BQ25120A::runsOnBattery(uint8_t address){
  uint8_t faults = readByteUnprotected(address, BQ25120A_FAULTS);
  // Read VIN under voltage fault (VIN_UV on Bit 6) from the faults register.
  bool runsOnBattery = (faults & 0b01000000) != 0;
  return runsOnBattery;
}

void BQ25120A::writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  nicla::_i2c_mutex.lock();
  // Only enter active mode when runnning on battery.
  // When powered from VIN, driving CD HIGH would disable charging.
  if(runsOnBattery(address)){
    setHighImpedanceModeEnabled(false);
  }
  Wire1.beginTransmission(address);
  Wire1.write(subAddress);
  Wire1.write(data);
  Wire1.endTransmission();
  nicla::_i2c_mutex.unlock();
  setHighImpedanceModeEnabled(true);
}

uint8_t BQ25120A::readByteUnprotected(uint8_t address, uint8_t subAddress){
  Wire1.beginTransmission(address);
  Wire1.write(subAddress);
  Wire1.endTransmission(false);
  Wire1.requestFrom(address, 1);
  uint32_t timeout = 100;
  uint32_t start_time = millis();
  while(!Wire1.available() && (millis() - start_time) < timeout) {}
  return Wire1.read();
}

uint8_t BQ25120A::readByte(uint8_t address, uint8_t subAddress)
{
  nicla::_i2c_mutex.lock();
  // Only enter active mode when runnning on battery.
  // When powered from VIN, driving CD HIGH would disable charging.
  if(runsOnBattery(address)){
    setHighImpedanceModeEnabled(false);
  }
  uint8_t ret = readByteUnprotected(address, subAddress);
  nicla::_i2c_mutex.unlock();  
  setHighImpedanceModeEnabled(true);
  return ret;
}

void BQ25120A::setHighImpedanceModeEnabled(bool enabled) {
  if(enabled){
    cd = 0;
  } else {
    cd = 1;
    delayMicroseconds(128); // Give some time to the BQ25120A to wake up
  }
}