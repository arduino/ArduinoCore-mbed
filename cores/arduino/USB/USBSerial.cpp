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

#include "Arduino.h"

#if DEVICE_USBDEVICE && defined(SERIAL_CDC)

#include "stdint.h"
#include "PluggableUSBSerial.h"
#include "usb_phy_api.h"
#include "mbed.h"

using namespace arduino;

static rtos::EventFlags event;

static void waitForPortClose() {

    event.wait_any(0xFF);
    // wait for DTR be 0 (port closed) and timeout to be over
    long start = millis();
    static const int WAIT_TIMEOUT = 200;
    while (_SerialUSB.connected() || (millis() - start) < WAIT_TIMEOUT) {
        // the delay is needed to handle other "concurrent" IRQ events
        delay(1);
    }
    _ontouch1200bps_();
}

void usbPortChanged(int baud, int bits, int parity, int stop) {
    if (baud == 1200) {
        event.set(1);
    }
}

USBSerial::USBSerial(bool connect_blocking, const char* name, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBCDC(get_usb_phy(), name, vendor_id, product_id, product_release)
{
    _settings_changed_callback = 0;

    if (connect_blocking) {
        wait_ready();
    }
}

USBSerial::USBSerial(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBCDC(phy, NULL, vendor_id, product_id, product_release)
{
    _settings_changed_callback = 0;
}

USBSerial::~USBSerial()
{
}

void USBSerial::begin(unsigned long) {
    this->attach(usbPortChanged);
    this->attach(::mbed::callback(this, &USBSerial::onInterrupt));
    t = new rtos::Thread(osPriorityNormal, 256, nullptr, "USBevt");
    t->start(waitForPortClose);
    onInterrupt();
}

int USBSerial::_putc(int c)
{
    if (send((uint8_t *)&c, 1)) {
        return c;
    } else {
        return -1;
    }
}

int USBSerial::_getc()
{
    uint8_t c = 0;
    if (receive(&c, sizeof(c))) {
        return c;
    } else {
        return -1;
    }
}

void USBSerial::data_rx()
{
    USBCDC::assert_locked();

    //call a potential handler
    for (size_t i = 0 ; i < _howManyCallbacks ; i++) {
        if (_rx[i]) {
            _rx[i].call();
        } else {
            break;
        }
    }
}

uint32_t USBSerial::_available()
{
    USBCDC::lock();

    uint32_t size = 0;
    if (!_rx_in_progress) {
        size = _rx_size > CDC_MAX_PACKET_SIZE ? CDC_MAX_PACKET_SIZE : _rx_size;
    }

    USBCDC::unlock();
    return size;
}

bool USBSerial::connected()
{
    return _terminal_connected;
}

USBSerial _SerialUSB(false);

#endif
