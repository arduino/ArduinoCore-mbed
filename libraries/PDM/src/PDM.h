#ifndef _PDM_H_INCLUDED
#define _PDM_H_INCLUDED

#include <Arduino.h>

#ifndef ARDUINO_ARCH_NRF52840
#error "This library targets only NRF52840 boards, not every mbed target"
#endif

#include "utility/PDMDoubleBuffer.h"

class PDMClass
{
public:
  PDMClass(int pwrPin, int clkPin, int dinPin);
  virtual ~PDMClass();

  int begin(int channels, long sampleRate);
  void end();

  virtual int available();
  virtual int read(void* buffer, size_t size);

  void onReceive(void(*)(void));

  void setGain(int gain);
  void setBufferSize(int bufferSize);

// private:
  void IrqHandler();

private:
  int _dinPin;
  int _clkPin;
  int _pwrPin;

  int _channels;
  
  PDMDoubleBuffer _doubleBuffer;
  
  void (*_onReceive)(void);
};

extern PDMClass PDM;

#endif
