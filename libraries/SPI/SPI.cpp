#include "SPI.h"

arduino::MbedSPI::MbedSPI(int miso, int mosi, int sck) : _miso(miso), _mosi(mosi), _sck(sck) {
}

uint8_t arduino::MbedSPI::transfer(uint8_t data) {
    uint8_t ret;
    dev->write((const char*)&data, 1, (char*)&ret, 1);
    return ret;
}

uint16_t arduino::MbedSPI::transfer16(uint16_t data) {
    uint8_t ret[2];
    dev->write((const char*)&data, 2, (char*)ret, 2);
    return ret[0] << 8 | ret[1];
}

void arduino::MbedSPI::transfer(void *buf, size_t count) {
    dev->write((const char*)buf, count, (char*)buf, count);
}

void arduino::MbedSPI::usingInterrupt(int interruptNumber) {

}

void arduino::MbedSPI::notUsingInterrupt(int interruptNumber) {

}

void arduino::MbedSPI::beginTransaction(SPISettings settings) {
    if (settings != this->settings) {
        dev->format(8, settings.getDataMode());
        dev->frequency(settings.getClockFreq());
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
    dev = new mbed::SPI((PinName)_mosi, (PinName)_miso, (PinName)_sck);
}

void arduino::MbedSPI::end() {
    if (dev != NULL) {
        delete dev;
    }
}

#if !defined(ARDUINO_AS_MBED_LIBRARY)

#if DEVICE_SPI > 0
arduino::MbedSPI SPI(SPI_MISO, SPI_MOSI, SPI_SCK);
#endif
#if DEVICE_SPI > 1
arduino::MbedSPI SPI1(SPI_MISO1, SPI_MOSI1, SPI_SCK1);
#endif

#endif