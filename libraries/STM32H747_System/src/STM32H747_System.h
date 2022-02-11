#ifndef H747_System_h_
#define H747_System_h_

#include "Arduino.h"

class STM32H747 {

public:
  virtual bool begin() = 0;
  virtual bool enterLowPower() = 0;

protected:
  uint8_t readReg(uint8_t subAddress);
  void setRegister(uint8_t reg, uint8_t val);
  bool useInternalOscillator(bool lowspeed = false);
};

#endif