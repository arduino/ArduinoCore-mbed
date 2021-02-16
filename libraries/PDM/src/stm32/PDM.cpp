/*
  PDM.cpp - library to interface with STM32 PDM microphones
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2020 Arduino SA

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#ifdef TARGET_STM

#include "PDM.h"
#include "mbed.h"
extern "C" {
  #include "audio.h"
}

extern "C" uint16_t *g_pcmbuf;

PDMClass::PDMClass(int dinPin, int clkPin, int pwrPin) :
  _dinPin(dinPin),
  _clkPin(clkPin),
  _pwrPin(pwrPin),
  _onReceive(NULL)
{
}

PDMClass::~PDMClass()
{
}

static int gain_db = -1;
static int _samplerate = -1;

int PDMClass::begin(int channels, long sampleRate) {

  if (isBoardRev2()) {
    mbed::I2C i2c(PB_7, PB_6);
    char data[2];

    // SW2 to 3.3V (SW2_VOLT)
    data[0] = 0x3B;
    data[1] = 0xF;
    i2c.write(8 << 1, data, sizeof(data));

    // SW1 to 3.0V (SW1_VOLT)
    data[0] = 0x35;
    data[1] = 0xF;
    i2c.write(8 << 1, data, sizeof(data));
  }

  _channels = channels;
  _samplerate = sampleRate;

  if (gain_db == -1) {
    gain_db = -10;
  }

  //g_pcmbuf = (uint16_t*)_doubleBuffer.data();

  py_audio_init(channels, sampleRate, gain_db, 0.9883f);

  py_audio_start_streaming();

  return 1;
}

void PDMClass::end()
{
  py_audio_stop_streaming();
  py_audio_deinit();
}

int PDMClass::available()
{
  size_t avail = _doubleBuffer.available();
  return avail;
}

int PDMClass::read(void* buffer, size_t size)
{
  int read = _doubleBuffer.read(buffer, size);
  return read;
}

void PDMClass::onReceive(void(*function)(void))
{
  _onReceive = function;
}

void PDMClass::setGain(int gain)
{
  gain_db = gain;
  //end();
  //begin(_channels, _samplerate);
}

void PDMClass::setBufferSize(int bufferSize)
{
  _doubleBuffer.setSize(bufferSize);
}

void PDMClass::IrqHandler(bool halftranfer)
{
  if (_doubleBuffer.available() == 0) {
    g_pcmbuf = (uint16_t*)_doubleBuffer.data();
    audio_pendsv_callback();
    _doubleBuffer.swap(_doubleBuffer.availableForWrite());
  }

  if (_onReceive) {
      _onReceive();
  }
}

extern "C" {
void PDMIrqHandler(bool halftranfer)
{
  PDM.IrqHandler(halftranfer);
}

void PDMsetBufferSize(int size) {
  PDM.setBufferSize(size);
}
}

PDMClass PDM(0, 0, 0);

#endif