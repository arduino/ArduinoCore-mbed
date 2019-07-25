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