#ifdef ARDUINO_PORTENTA_H7_M7

#include "Portenta_System.h"
#include "Wire.h"
#include "mbed.h"

#define PMIC_ADDRESS 0x08

bool arduino::Portenta_System::begin()
{
  Wire1.begin();
}

bool arduino::Portenta_System::enterLowPower() {
  /* TO DO */
}

arduino::Portenta_System Portenta;

#endif