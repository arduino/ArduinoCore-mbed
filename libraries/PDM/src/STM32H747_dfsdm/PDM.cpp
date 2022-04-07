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

#if defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_GIGA)

#include "PDM.h"
#include "mbed.h"
extern "C" {
  #include "audio.h"
}

extern "C" uint16_t *g_pcmbuf;
static PDMClass *_instance = NULL;

PDMClass::PDMClass(int dinPin, int clkPin, int pwrPin) :
  _dinPin(dinPin),
  _clkPin(clkPin),
  _pwrPin(pwrPin),
  _onReceive(NULL),
  _gain(-1),
  _channels(-1),
  _samplerate(-1),
  _init(-1)
{
  _instance = this;
}

PDMClass::~PDMClass()
{
  _instance = NULL;
}

int PDMClass::begin(int channels, int sampleRate) {
  if(_instance != this) {
    return 0;
  }

  _channels = channels;
  _samplerate = sampleRate;

  if (_gain == -1) {
    _gain = 3;
  }

  g_pcmbuf = (uint16_t*)_doubleBuffer.data();
  _doubleBuffer.swap(0);

  if(py_audio_init(channels, sampleRate)) {
    if (py_audio_start_streaming()) {
      _init = 1;
      return 1;
    }
  }
  return 0;
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
  if(_instance != this) {
    _instance = this;
  }
}

void PDMClass::setGain(int gain)
{
  _gain = gain;
  py_audio_gain_set(gain);
}

void PDMClass::setBufferSize(int bufferSize)
{
  _doubleBuffer.setSize(bufferSize);
}

size_t PDMClass::getBufferSize()
{
  return _doubleBuffer.getSize();
}

#define HALF_TRANSFER_SIZE  (256*_channels)
static int g_pcmbuf_size=0;

void PDMClass::IrqHandler(bool halftranfer)
{
  if (g_pcmbuf_size < _doubleBuffer.getSize()) {
    audio_pendsv_callback();
    g_pcmbuf += (HALF_TRANSFER_SIZE);
    g_pcmbuf_size += HALF_TRANSFER_SIZE*2;

    if(g_pcmbuf_size == _doubleBuffer.getSize()) {
      _doubleBuffer.swap(g_pcmbuf_size);
      g_pcmbuf = (uint16_t*)_doubleBuffer.data();
      g_pcmbuf_size = 0;
      if (_onReceive) {
        _onReceive();
      }
    }
  }
}

extern "C" {
void PDMIrqHandler(bool halftranfer)
{
  _instance->IrqHandler(halftranfer);
}

void PDMsetBufferSize(int size) {
  _instance->setBufferSize(size);
}

size_t PDMgetBufferSize() {
  return _instance->getBufferSize();
}
}

PDMClass PDM(0, 0, 0);

#endif
