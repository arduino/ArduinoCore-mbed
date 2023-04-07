#include <mbed.h>
#include <I2C.h>
#include "BQ25120A.h"
#include "Nicla_System.h"
#include "DigitalOut.h"

static mbed::DigitalOut cd(p25);

uint8_t BQ25120A::getStatus()
{
  uint8_t c = readByte(BQ25120A_ADDRESS, BQ25120A_STATUS); // Read PRODUCT_ID register for BQ25120A
  return c;
}

bool BQ25120A::writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  cd = 1;
  nicla::i2c_mutex.lock();
  Wire1.beginTransmission(address);
  Wire1.write(subAddress);
  Wire1.write(data);
  uint8_t result = Wire1.endTransmission();
  nicla::i2c_mutex.unlock();
  cd = 0;
  return result == 0;
}

uint8_t BQ25120A::readByte(uint8_t address, uint8_t subAddress)
{
  cd = 1;
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
  cd = 0;
  return ret;
}
