#ifndef Portenta_System_h_
#define Portenta_System_h_

#include "STM32H747_System.h"

namespace arduino {

class Portenta_System: public STM32H747
{

public:
  Portenta_System() {};
  virtual bool begin();
  virtual bool enterLowPower();

  String getBoardRevision();
  uint16_t getCarrierSpecs();
};

}

extern arduino::Portenta_System Portenta;

#endif //Portenta_System_h_