#ifdef ARDUINO_PORTENTA_H7_M7

#include "Portenta_System.h"
#include "Wire.h"
#include "mbed.h"
#include "SecureQSPI.h"

#define PMIC_ADDRESS 0x08

bool arduino::Portenta_System::begin()
{
  Wire1.begin();
}

bool arduino::Portenta_System::enterLowPower() {
  /* TO DO */
}

// 8Kbit secure OTP area (on MX25L12833F)
bool arduino::Portenta_System::getSecureFlashData(void* buf, size_t size) {
    static SecureQSPIFBlockDevice root;
    root.init();
    auto ret = root.readSecure(buf, 0, size > 512 ? 512 : size);
    return ret == 0;
}

arduino::Portenta_System Portenta;

#endif