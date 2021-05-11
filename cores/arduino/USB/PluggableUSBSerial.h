/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef USBSERIAL_H
#define USBSERIAL_H

#include "Arduino.h"
#include "USBCDC.h"
#include "platform/Stream.h"
#include "rtos/rtos.h"
#include "Callback.h"

/**
* USBSerial example
*
* @code
* #include "mbed.h"
* #include "USBSerial.h"
*
* //Virtual serial port over USB
* USBSerial serial;
*
* int main(void) {
*
*    while(1)
*    {
*        serial.printf("I am a virtual serial port\n");
*        wait(1);
*    }
* }
* @endcode
*/

#define MAX_CALLBACKS_ON_IRQ    4

namespace arduino {


class USBSerial: public USBCDC, public ::mbed::Stream, public HardwareSerial {
public:

    /**
    * Basic constructor
    *
    * Construct this object optionally connecting and blocking until it is ready.
    *
    * @note Do not use this constructor in derived classes.
    *
    * @param connect_blocking true to perform a blocking connect, false to start in a disconnected state
    * @param vendor_id Your vendor_id (default: 0x1f00)
    * @param product_id Your product_id (default: 0x2012)
    * @param product_release Your product_release (default: 0x0001)
    *
    */
    USBSerial(bool connect_blocking = true, const char* name = NULL, uint16_t vendor_id = 0x1f00, uint16_t product_id = 0x2012, uint16_t product_release = 0x0001);

    /**
    * Fully featured constructor
    *
    * Construct this object with the supplied USBPhy and parameters. The user
    * this object is responsible for calling connect() or init().
    *
    * @note Derived classes must use this constructor and call init() or
    * connect() themselves. Derived classes should also call deinit() in
    * their destructor. This ensures that no interrupts can occur when the
    * object is partially constructed or destroyed.
    *
    * @param phy USB phy to use
    * @param vendor_id Your vendor_id (default: 0x1f00)
    * @param product_id Your product_id (default: 0x2012)
    * @param product_release Your product_release (default: 0x0001)
    *
    */
    USBSerial(USBPhy *phy, uint16_t vendor_id = 0x1f00, uint16_t product_id = 0x2012, uint16_t product_release = 0x0001);

    /**
     * Destroy this object
     *
     * Any classes which inherit from this class must call deinit
     * before this destructor runs.
     */
    virtual ~USBSerial();

    /**
    * Send a character. You can use puts, printf.
    *
    * @param c character to be sent
    * @returns true if there is no error, false otherwise
    */
    virtual int _putc(int c);

    /**
    * Read a character: blocking
    *
    * @returns character read
    */
    virtual int _getc();

    /**
    * Check the number of bytes available.
    *
    * @returns the number of bytes available
    */
    uint32_t _available();

    /**
    * Check if the terminal is connected.
    *
    * @returns connection status
    */
    bool connected();

    /** Determine if there is a character available to read
     *
     *  @returns
     *    1 if there is a character available to read,
     *    0 otherwise
     */
    int readable()
    {
        return available() ? 1 : 0;
    }

    /** Determine if there is space available to write a character
     *
     *  @returns
     *    1 if there is space to write a character,
     *    0 otherwise
     */
    int writeable()
    {
        return 1;    // always return 1, for write operation is blocking
    }

    /**
     *  Attach a member function to call when a packet is received.
     *
     *  @param tptr pointer to the object to call the member function on
     *  @param mptr pointer to the member function to be called
     */
    template<typename T>
    void attach(T *tptr, void (T::*mptr)(void))
    {
        USBCDC::lock();

        if ((mptr != NULL) && (tptr != NULL) && (_howManyCallbacks < sizeof(_rx)/sizeof(_rx[0]))) {
            _rx[_howManyCallbacks++] = ::mbed::Callback<void()>(mptr, tptr);
        }

        USBCDC::unlock();
    }

    /**
     * Attach a callback called when a packet is received
     *
     * @param fptr function pointer
     */
    void attach(void (*fptr)(void))
    {
        USBCDC::lock();

        if ((fptr != NULL) && (_howManyCallbacks < sizeof(_rx)/sizeof(_rx[0]))) {
            _rx[_howManyCallbacks++] = ::mbed::Callback<void()>(fptr);
        }

        USBCDC::unlock();
    }

    /**
     * Attach a Callback called when a packet is received
     *
     * @param cb Callback to attach
     */
    void attach(::mbed::Callback<void()> cb)
    {
        USBCDC::lock();

        if (_howManyCallbacks < sizeof(_rx)/sizeof(_rx[0])) {
            _rx[_howManyCallbacks++] = cb;
        }

        USBCDC::unlock();
    }

    /**
     * Attach a callback to call when serial's settings are changed.
     *
     * @param fptr function pointer
     */
    void attach(void (*fptr)(int baud, int bits, int parity, int stop))
    {
        USBCDC::lock();

        _settings_changed_callback = fptr;

        USBCDC::unlock();
    }

    // Arduino APIs
    void begin(unsigned long);

    void begin(unsigned long baudrate, uint16_t config) {
        begin(baudrate);
    }

    void end() {
    }

    int available(void) {
        USBCDC::lock();
        onInterrupt();
        auto ret = rx_buffer.available();
        USBCDC::unlock();
        return ret;
    }

    int peek(void) {
        USBCDC::lock();
        onInterrupt();
        auto ret = rx_buffer.peek();
        USBCDC::unlock();
        return ret;
    }

    int read(void) {
        USBCDC::lock();
        onInterrupt();
        auto ret = rx_buffer.read_char();
        USBCDC::unlock();
        return ret;
    }

    void flush(void) {}

    size_t write(uint8_t c) {
        if (!connected()) {
            return 0;
        }
        return _putc(c);
    }

    size_t write(const uint8_t* buf, size_t size) {
        if (!connected()) {
            return 0;
        }
        size_t sent = 0;
        while (sent < size) {
            size_t to_send = (size - sent) > CDC_MAX_PACKET_SIZE ? CDC_MAX_PACKET_SIZE : (size - sent);
            send((uint8_t*)&buf[sent], to_send);
            sent += to_send;
        }
        return sent;
    }
    using Print::write; // pull in write(str) and write(buf, size) from Print

    operator bool() {
        // call delay() to force rescheduing during while !Serial pattern
        delay(1);
        return connected();
    }

    uint32_t baud() {
        return _baud;
    }
    uint8_t stopbits() {
        return _stop;
    }
    uint8_t paritytype() {
        return _parity;
    }
    uint8_t numbits() {
        return _bits;
    }
    bool dtr() {
        return (_dtr != 0);
    }
    bool rts() {
        return (_rts != 0);
    }

private:
    RingBufferN<256> rx_buffer;
    rtos::Thread* t;
    int _baud, _bits, _parity, _stop;

    void onInterrupt() {
        uint8_t buf[256];
        int howMany = _available();
        uint32_t toRead;
        if (howMany > rx_buffer.availableForStore()) {
            howMany = rx_buffer.availableForStore();
        }
        receive_nb(buf, howMany, &toRead);
        while (rx_buffer.availableForStore() && toRead > 0) {
            rx_buffer.store_char(buf[howMany-toRead]);
            toRead --;
        }
    }

protected:
    virtual void data_rx();
    virtual void line_coding_changed(int baud, int bits, int parity, int stop)
    {
        USBCDC::assert_locked();

        if (_settings_changed_callback) {
            _settings_changed_callback(baud, bits, parity, stop);
        }
        this->_baud = baud;
        this->_bits = bits;
        this->_parity = parity;
        this->_stop = stop;
    }

private:
    void (*_settings_changed_callback)(int baud, int bits, int parity, int stop);
    ::mbed::Callback<void()> _rx[MAX_CALLBACKS_ON_IRQ];
    size_t _howManyCallbacks = 0;
};
}

extern arduino::USBSerial _SerialUSB;

#endif