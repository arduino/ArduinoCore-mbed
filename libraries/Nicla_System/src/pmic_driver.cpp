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

uint8_t BQ25120A::getStatus()
{
  uint8_t c = readByte(BQ25120A_ADDRESS, BQ25120A_STATUS); // Read PRODUCT_ID register for BQ25120A
  return c;
}

void BQ25120A::writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  setHighImpedanceModeEnabled(false);
  nicla::i2c_mutex.lock();
  Wire1.beginTransmission(address);
  Wire1.write(subAddress);
  Wire1.write(data);
  Wire1.endTransmission();
  nicla::i2c_mutex.unlock();
  setHighImpedanceModeEnabled(true);
}

uint8_t BQ25120A::readByte(uint8_t address, uint8_t subAddress)
{
  setHighImpedanceModeEnabled(false);
  nicla::i2c_mutex.lock();
  Wire1.beginTransmission(address);
  Wire1.write(subAddress);
  Wire1.endTransmission(false);
  Wire1.requestFrom(address, 1);
  uint32_t timeout = 100;
  uint32_t start_time = millis();
  while(!Wire1.available() && (millis() - start_time) < timeout) {}
  uint8_t ret = Wire1.read();
  nicla::i2c_mutex.unlock();
  setHighImpedanceModeEnabled(true);
  return ret;
}

void BQ25120A::setHighImpedanceModeEnabled(bool enabled) {
  if(enabled){
    cd = 0;
  } else {
    cd = 1;
    delayMicroseconds(64); // Give some time to the BQ25120A to wake up
  }
}