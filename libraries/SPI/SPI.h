/*
  Copyright (c) 2016 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include "Arduino.h"
#if !defined(ARDUINO_AS_MBED_LIBRARY)
#include "drivers/SPIMaster.h"
#else
#include "drivers/SPI.h"
#endif

namespace arduino {

class MbedSPI : public SPIClass
{
public:
    MbedSPI(int miso, int mosi, int sck);
    virtual uint8_t transfer(uint8_t data);
    virtual uint16_t transfer16(uint16_t data);
    virtual void transfer(void *buf, size_t count);

    // Transaction Functions
    virtual void usingInterrupt(int interruptNumber);
    virtual void notUsingInterrupt(int interruptNumber);
    virtual void beginTransaction(SPISettings settings);
    virtual void endTransaction(void);

    // SPI Configuration methods
    virtual void attachInterrupt();
    virtual void detachInterrupt();

    virtual void begin();
    virtual void end();

private:
    SPISettings settings = SPISettings(0, MSBFIRST, SPI_MODE0);
    mbed::SPI* dev;
    int _miso;
    int _mosi;
    int _sck;
};

}

#if !defined(ARDUINO_AS_MBED_LIBRARY)

#if DEVICE_SPI > 0
extern arduino::MbedSPI SPI;
#endif
#if DEVICE_SPI > 1
extern arduino::MbedSPI SPI1;
#endif

#endif