/*
  SPI.cpp - wrapper over mbed SPI class
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2018-2019 Arduino SA

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

#include "SPI.h"
#include "pinDefinitions.h"
#if !defined(ARDUINO_AS_MBED_LIBRARY)
#include "drivers/SPIMaster.h"
#else
#include "drivers/SPI.h"
#endif

struct _mbed_spi {
  mbed::SPI* obj;
};


arduino::MbedSPI::MbedSPI(int miso, int mosi, int sck) :
  _miso(digitalPinToPinName(miso)), _mosi(digitalPinToPinName(mosi)), _sck(digitalPinToPinName(sck)) {

}

arduino::MbedSPI::MbedSPI(PinName miso, PinName mosi, PinName sck) : _miso(miso), _mosi(mosi), _sck(sck) {

}

uint8_t arduino::MbedSPI::transfer(uint8_t data) {
    uint8_t ret;
    dev->obj->write((const char*)&data, 1, (char*)&ret, 1);
    return ret;
}

uint16_t arduino::MbedSPI::transfer16(uint16_t data) {

    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } t;
    t.val = data;

    if (settings.getBitOrder() == LSBFIRST) {
        t.lsb = transfer(t.lsb);
        t.msb = transfer(t.msb);
    } else {
        t.msb = transfer(t.msb);
        t.lsb = transfer(t.lsb);
    }
    return t.val;
}

void arduino::MbedSPI::transfer(void *buf, size_t count) {
    dev->obj->write((const char*)buf, count, (char*)buf, count);
}

void arduino::MbedSPI::usingInterrupt(int interruptNumber) {

}

void arduino::MbedSPI::notUsingInterrupt(int interruptNumber) {

}

void arduino::MbedSPI::beginTransaction(SPISettings settings) {
    if (settings != this->settings) {
        dev->obj->format(8, settings.getDataMode());
        dev->obj->frequency(settings.getClockFreq());
        this->settings = settings;
    }
}

void arduino::MbedSPI::endTransaction(void) {
    // spinlock until transmission is over (if using ASYNC transfer)
}

void arduino::MbedSPI::attachInterrupt() {

}

void arduino::MbedSPI::detachInterrupt() {

}

void arduino::MbedSPI::begin() {
    if (dev == NULL) {
      dev = new mbed_spi;
      dev->obj = NULL;
    }
    if (dev->obj == NULL) {
      dev->obj = new mbed::SPI(_mosi, _miso, _sck);
    }
}

void arduino::MbedSPI::end() {
    if (dev->obj != NULL) {
        delete dev->obj;
    }
}

#if !defined(ARDUINO_AS_MBED_LIBRARY)

#if SPI_HOWMANY > 0
arduino::MbedSPI SPI(SPI_MISO, SPI_MOSI, SPI_SCK);
#endif
#if SPI_HOWMANY > 1
arduino::MbedSPI SPI1(SPI_MISO1, SPI_MOSI1, SPI_SCK1);
#endif

#endif