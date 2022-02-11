#ifndef Nicla_Vision_System_h_
#define Nicla_Vision_System_h_

#include "STM32H747_System.h"

namespace arduino {

class Nicla_Vision_System: public STM32H747
{

public:
  Nicla_Vision_System() {};
  virtual bool begin();
  virtual bool enterLowPower();

  virtual void enable3V3VDDIO_EXT();
  virtual void enable1V8VDDIO_EXT();
  virtual void disableVDDIO_EXT();
};

}

extern arduino::Nicla_Vision_System Nicla_Vision;

#endif