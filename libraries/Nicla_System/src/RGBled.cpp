/* 12/23/2017 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer
 *  
 This basic sketch is to operate the IS31FL3194 3-channel led driver
 http://www.issi.com/WW/pdf/IS31FL3194.pdf
 
 The sketch uses default SDA/SCL pins on the Ladybug development board 
 but should work with almost any Arduino-based board.
 Library may be used freely and without limit only with attribution.
 
  */
#include <mbed.h>
#include <I2C.h>
#include "RGBled.h"
#include "Nicla_System.h"

void RGBled::begin()
{
  reset();
  init();
  powerUp();
}

void RGBled::end()
{
  powerDown();
}

void RGBled::setColor(RGBColors color)
{
  if(color == off) {
  _blue = 0x00;
  _green = 0x00;
  _red = 0x00;
  }

  if(color == green) {
  _blue = 0x00;
  _green = 0xFF;
  _red = 0x00;
  }

  if(color == blue) {
  _blue = 0xFF;
  _green = 0x00;
  _red = 0x00;
  }

  if(color == red) {
  _blue = 0x00;
  _green = 0x00;
  _red = 0xFF;
  }

  if(color == cyan) {
  _blue = 0x20;
  _green = 0x20;
  _red = 0x00;
  }

  if(color == magenta) {
  _blue = 0x20;
  _green = 0x00;
  _red = 0x20;
  }

  if(color == yellow) {
  _blue = 0x00;
  _green = 0x20;
  _red = 0x20;
  }

  if(color == white) {
  _blue = 0xFF;
  _green = 0xFF;
  _red = 0xFF;
  }

  setColor(_red, _green, _blue);

}

void RGBled::setColorBlue(uint8_t blue) {
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT1, blue >> scale_factor);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_COLOR_UPDATE, 0xC5);
}

void RGBled::setColorRed(uint8_t red) {
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT3, red  >> scale_factor);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_COLOR_UPDATE, 0xC5);
}

void RGBled::setColorGreen(uint8_t green) {
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT2, green >> scale_factor);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_COLOR_UPDATE, 0xC5);
}

void RGBled::setColor(uint8_t red, uint8_t green, uint8_t blue)
{
  // set rgb led current
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT1, blue >> scale_factor); //maximum current
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT2, green >> scale_factor);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT3, red >> scale_factor);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_COLOR_UPDATE, 0xC5); // write to color update register for changes to take effect
}

// Read the Chip ID register, this is a good test of communication
uint8_t RGBled::getChipID() 
{
  uint8_t c = readByte(IS31FL3194_ADDRESS, IS31FL3194_PRODUCT_ID); // Read PRODUCT_ID register for IS31FL3194
  return c;
}


void RGBled::reset()
{
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_RESET, 0xC5);
}


void RGBled::powerDown()
{
  uint8_t d = readByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG, d & ~(0x01)); //clear bit 0 to shut down
}


void RGBled::powerUp()
{
  uint8_t d = readByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG);
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG, d | 0x01); //set bit 0 to enable
}


void RGBled::init()// configure rgb led function
{
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OP_CONFIG, 0x01);     // normal operation in current mode
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_OUT_CONFIG, 0x07);    // enable all three ouputs
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_CURRENT_BAND, 0x00);  // 10 mA max current
  writeByte(IS31FL3194_ADDRESS, IS31FL3194_HOLD_FUNCTION, 0x00); // hold function disable
}

void RGBled::ledBlink(RGBColors color, uint32_t duration)
{
  setColor(color);
  delay(duration);
  setColor(off);
}

void RGBled::writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  nicla::i2c_mutex.lock();
  Wire1.beginTransmission(address);
  Wire1.write(subAddress);
  Wire1.write(data);
  Wire1.endTransmission();
  nicla::i2c_mutex.unlock();
}

uint8_t RGBled::readByte(uint8_t address, uint8_t subAddress)
{
  nicla::i2c_mutex.lock();
  //char response = 0xFF;
  Wire1.beginTransmission(address);
  Wire1.write(subAddress);
  Wire1.endTransmission(false);
  Wire1.requestFrom(address, 1);
  uint32_t timeout = 100;
  uint32_t start_time = millis();
  while(!Wire1.available() && (millis() - start_time) < timeout) {}
  uint8_t ret = Wire1.read();
  nicla::i2c_mutex.unlock();
  return ret;
}