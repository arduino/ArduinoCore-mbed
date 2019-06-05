#ifndef _PDM_H_INCLUDED
#define _PDM_H_INCLUDED

#include <Arduino.h>

extern "C" {
  #include "nrfx_pdm.h"
}

#define DEFAULT_PDM_BUFFER_SIZE 512

#define NUM_PDM_BUFFERS 2

class PDMClass
{
public:
  PDMClass(int pwrPin, int clkPin, int dinPin);
  virtual ~PDMClass();

  int begin(int channels, long sampleRate, int gain = 0x12);
  void end();

  virtual int available();

  int read(void* buffer, size_t size);

  void onReceive(void(*)(void));
  void onReceive(void(*)(void* buf, size_t size));

  void setBufferSize(int bufferSize);

private:
  static void onPdmEvent(nrfx_pdm_evt_t const* const evt);
  void handlePdmEvent(nrfx_pdm_evt_t const* const evt);

private:
  int _dinPin;
  int _clkPin;
  int _pwrPin;

  int _channels;
  int _bufferSize;
  int16_t* _buffer[2];
  volatile int _writeBufferIndex;
  volatile int _readBufferIndex;
  volatile int _readIndex;

  void (*_onReceive)(void);
  void (*_onReceiveParams)(void* buf, size_t size);
};

extern PDMClass PDM;

#endif
