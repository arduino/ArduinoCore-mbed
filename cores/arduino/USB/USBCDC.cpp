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
#include "USBCDC.h"
#include "EndpointResolver.h"
#include "AsyncOp.h"
#include "usb_phy_api.h"

using namespace arduino;

static const uint8_t cdc_line_coding_default[7] = {0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08};

#define DEFAULT_CONFIGURATION (1)

#define CDC_SET_LINE_CODING        0x20
#define CDC_GET_LINE_CODING        0x21
#define CDC_SET_CONTROL_LINE_STATE 0x22

// Control Line State bits
#define CLS_DTR   (1 << 0)
#define CLS_RTS   (1 << 1)

class USBCDC::AsyncWrite: public AsyncOp {
public:
    AsyncWrite(USBCDC *serial, uint8_t *buf, uint32_t size):
        serial(serial), tx_buf(buf), tx_size(size), result(false)
    {

    }

    virtual ~AsyncWrite()
    {

    }

    virtual bool process()
    {
        if (!serial->_terminal_connected) {
            result = false;
            return true;
        }

        uint32_t actual_size = 0;
        serial->send_nb(tx_buf, tx_size, &actual_size, true);
        tx_size -= actual_size;
        tx_buf += actual_size;
        if (tx_size == 0) {
            result = true;
            return true;
        }

        // Start transfer if it hasn't been
        serial->_send_isr_start();
        return false;
    }

    USBCDC *serial;
    uint8_t *tx_buf;
    uint32_t tx_size;
    bool result;
};

class USBCDC::AsyncRead: public AsyncOp {
public:
    AsyncRead(USBCDC *serial, uint8_t *buf, uint32_t size, uint32_t *size_read, bool read_all)
        :   serial(serial), rx_buf(buf), rx_size(size), rx_actual(size_read), all(read_all), result(false)
    {

    }

    virtual ~AsyncRead()
    {

    }

    virtual bool process()
    {
        if (!serial->_terminal_connected) {
            result = false;
            return true;
        }

        uint32_t actual_size = 0;
        serial->receive_nb(rx_buf, rx_size, &actual_size);
        rx_buf += actual_size;
        *rx_actual += actual_size;
        rx_size -= actual_size;
        if ((!all && *rx_actual > 0) || (rx_size == 0)) {
            // Wake thread if request is done
            result = true;
            return true;
        }

        serial->_receive_isr_start();
        return false;
    }

    USBCDC *serial;
    uint8_t *rx_buf;
    uint32_t rx_size;
    uint32_t *rx_actual;
    bool all;
    bool result;
};

class USBCDC::AsyncWait: public AsyncOp {
public:
    AsyncWait(USBCDC *serial)
        :   serial(serial)
    {

    }

    virtual ~AsyncWait()
    {

    }

    virtual bool process()
    {
        if (serial->_terminal_connected) {
            return true;
        }

        return false;
    }

    USBCDC *serial;
};

USBCDC::USBCDC(bool connect_blocking, const char* name, uint16_t vendor_id, uint16_t product_id, uint16_t product_release)
    : internal::PluggableUSBModule(2), extraDescriptor(name)
{
    PluggableUSBD().plug(this);
}

USBCDC::USBCDC(USBPhy *phy, const char* name, uint16_t vendor_id, uint16_t product_id, uint16_t product_release)
    : internal::PluggableUSBModule(2), extraDescriptor(name)
{
    PluggableUSBD().plug(this);
}

USBCDC::~USBCDC()
{
    PluggableUSBD().deinit();
}

void USBCDC::init(EndpointResolver& resolver)
{
    memcpy(_cdc_line_coding, cdc_line_coding_default, sizeof(_cdc_line_coding));

    _bulk_in = resolver.endpoint_in(USB_EP_TYPE_BULK, CDC_MAX_PACKET_SIZE);
    _bulk_out = resolver.endpoint_out(USB_EP_TYPE_BULK, CDC_MAX_PACKET_SIZE);
    _int_in = resolver.endpoint_in(USB_EP_TYPE_INT, CDC_MAX_PACKET_SIZE);
    MBED_ASSERT(resolver.valid());

    _terminal_connected = false;

    _tx_in_progress = false;
    _tx_buf = _tx_buffer;
    _tx_size = 0;

    _rx_in_progress = false;
    _rx_buf = _rx_buffer;
    _rx_size = 0;
}

void USBCDC::callback_reset()
{
    assert_locked();
    /* Called in ISR context */

    _change_terminal_connected(false);
};

void USBCDC::callback_state_change(USBDevice::DeviceState new_state)
{
    assert_locked();
    /* Called in ISR context */

    if (new_state != USBDevice::Configured) {
        _change_terminal_connected(false);
    }
}

uint32_t USBCDC::callback_request(const USBDevice::setup_packet_t *setup, USBDevice::RequestResult *result, uint8_t** data)
{
    assert_locked();
    /* Called in ISR context */

    *result = USBDevice::PassThrough;
    uint32_t size = 0;

    if (setup->wIndex != pluggedInterface) {
        return size;
    }

    /* Only process class-specific requests */
    if (setup->bmRequestType.Type == CLASS_TYPE) {
        switch (setup->bRequest) {
            case CDC_GET_LINE_CODING:
                *result = USBDevice::Send;
                *data = _cdc_line_coding;
                size = 7;
                break;
            case CDC_SET_LINE_CODING:
                *result = USBDevice::Receive;
                *data = _cdc_new_line_coding;
                size = 7;
                break;
            case CDC_SET_CONTROL_LINE_STATE:
                if (setup->wValue & CLS_DTR) {
                    _change_terminal_connected(true);
                } else {
                    _change_terminal_connected(false);
                }
                this->_dtr = setup->wValue & CLS_DTR;
                this->_rts = setup->wValue & CLS_RTS;
                *result = USBDevice::Success;
                break;
            default:
                *result = USBDevice::Failure;
                break;
        }
    }
    return size;
}


bool USBCDC::callback_request_xfer_done(const USBDevice::setup_packet_t *setup, bool aborted)
{
    assert_locked();
    /* Called in ISR context */

    bool success = false;

    if (setup->wIndex != pluggedInterface || aborted) {
        return success;
    }

    /* Process class-specific requests */
    if (setup->bmRequestType.Type == CLASS_TYPE) {
        if ((setup->bRequest == CDC_SET_LINE_CODING) && (setup->wLength == 7)) {
            if (memcmp(_cdc_line_coding, _cdc_new_line_coding, 7)) {
                memcpy(_cdc_line_coding, _cdc_new_line_coding, 7);

                const uint8_t *buf = _cdc_line_coding;
                int baud = buf[0] + (buf[1] << 8)
                           + (buf[2] << 16) + (buf[3] << 24);
                int stop = buf[4];
                int bits = buf[6];
                int parity = buf[5];

                line_coding_changed(baud, bits, parity, stop);
            }
            success = true;
        }
        if (setup->bRequest == CDC_GET_LINE_CODING) {
            success = true;
        }
    }

    return success;
}

bool USBCDC::callback_set_configuration(uint8_t configuration)
{
    assert_locked();
    /* Called in ISR context */

    bool ret = false;
    // Configure endpoints > 0
    PluggableUSBD().endpoint_add(_int_in, CDC_MAX_PACKET_SIZE, USB_EP_TYPE_INT);
    PluggableUSBD().endpoint_add(_bulk_in, CDC_MAX_PACKET_SIZE, USB_EP_TYPE_BULK, ::mbed::callback(this, &USBCDC::_send_isr));
    PluggableUSBD().endpoint_add(_bulk_out, CDC_MAX_PACKET_SIZE, USB_EP_TYPE_BULK, ::mbed::callback(this, &USBCDC::_receive_isr));

    PluggableUSBD().read_start(_bulk_out, _rx_buf, sizeof(_rx_buffer));
    _rx_in_progress = true;

    ret = true;

    return ret;
}

void USBCDC::callback_set_interface(uint16_t interface, uint8_t alternate)
{
    assert_locked();
}

void USBCDC::_change_terminal_connected(bool connected)
{
    assert_locked();

    _terminal_connected = connected;
    if (!_terminal_connected) {
        // Abort TX
        if (_tx_in_progress) {
            PluggableUSBD().endpoint_abort(_bulk_in);
            _tx_in_progress = false;
        }
        _tx_buf = _tx_buffer;
        _tx_size = 0;
        _tx_list.process();
        MBED_ASSERT(_tx_list.empty());

        // Abort RX
        if (_rx_in_progress) {
            PluggableUSBD().endpoint_abort(_bulk_in);
            _rx_in_progress = false;
        }
        _rx_buf = _rx_buffer;
        _rx_size = 0;
        _rx_list.process();
        MBED_ASSERT(_rx_list.empty());

    }
    _connected_list.process();
}

bool USBCDC::ready()
{
    lock();

    bool ready = _terminal_connected;

    unlock();
    return ready;
}

void USBCDC::wait_ready()
{
    lock();

    AsyncWait wait_op(this);
    _connected_list.add(&wait_op);

    unlock();

    wait_op.wait(NULL);
}

bool USBCDC::send(uint8_t *buffer, uint32_t size)
{
    lock();

    AsyncWrite write_op(this, buffer, size);
    _tx_list.add(&write_op);

    unlock();

    write_op.wait(NULL);
    return write_op.result;
}

void USBCDC::send_nb(uint8_t *buffer, uint32_t size, uint32_t *actual, bool now)
{
    lock();

    *actual = 0;
    if (_terminal_connected && !_tx_in_progress) {
        uint32_t free = sizeof(_tx_buffer) - _tx_size;
        uint32_t write_size = free > size ? size : free;
        if (size > 0) {
            memcpy(_tx_buf, buffer, write_size);
        }
        _tx_size += write_size;
        *actual = write_size;
        if (now) {
            _send_isr_start();
        }
    }

    unlock();
}

void USBCDC::_send_isr_start()
{
    assert_locked();

    if (!_tx_in_progress && _tx_size) {
        if (PluggableUSBD().write_start(_bulk_in, _tx_buffer, _tx_size)) {
            _tx_in_progress = true;
        }
    }
}

/*
* Called by when CDC data is sent
* Warning: Called in ISR
*/
void USBCDC::_send_isr()
{
    assert_locked();

    PluggableUSBD().write_finish(_bulk_in);
    _tx_buf = _tx_buffer;
    _tx_size = 0;
    _tx_in_progress = false;

    _tx_list.process();
    if (!_tx_in_progress) {
        data_tx();
    }
}

bool USBCDC::receive(uint8_t *buffer, uint32_t size,  uint32_t *size_read)
{
    lock();

    bool read_all = size_read == NULL;
    uint32_t size_read_dummy;
    uint32_t *size_read_ptr = read_all ? &size_read_dummy : size_read;
    *size_read_ptr = 0;
    AsyncRead read_op(this, buffer, size, size_read_ptr, read_all);
    _rx_list.add(&read_op);

    unlock();

    read_op.wait(NULL);
    return read_op.result;
}

void USBCDC::receive_nb(uint8_t *buffer, uint32_t size,  uint32_t *size_read)
{

    *size_read = 0;
    if (_terminal_connected && !_rx_in_progress) {
        // Copy data over
        uint32_t copy_size = _rx_size > size ? size : _rx_size;
        memcpy(buffer, _rx_buf, copy_size);
        *size_read = copy_size;
        _rx_buf += copy_size;
        _rx_size -= copy_size;
        if (_rx_size == 0) {
            _receive_isr_start();
        }
    }
}

void USBCDC::_receive_isr_start()
{
    if ((_rx_size == 0) && !_rx_in_progress) {
        // Refill the buffer
        PluggableUSBD().read_start(_bulk_out, _rx_buffer, sizeof(_rx_buffer));
        _rx_in_progress = true;
    }
}

/*
* Called by when CDC data is received
* Warning: Called in ISR
*/
void USBCDC::_receive_isr()
{
    assert_locked();

    MBED_ASSERT(_rx_size == 0);
    _rx_buf = _rx_buffer;
    _rx_size = PluggableUSBD().read_finish(_bulk_out);
    _rx_in_progress = false;
    _rx_list.process();
    if (!_rx_in_progress) {
        data_rx();
    }

}

const uint8_t *USBCDC::string_iinterface_desc()
{
    return (const uint8_t *)extraDescriptor;
}

uint8_t USBCDC::getProductVersion()
{
    return 1;
}

const uint8_t *USBCDC::string_iproduct_desc()
{
    static const uint8_t stringIproductDescriptor[] = {
        0x16,
        STRING_DESCRIPTOR,
        'C', 0, 'D', 0, 'C', 0, ' ', 0, 'D', 0, 'E', 0, 'V', 0, 'I', 0, 'C', 0, 'E', 0
    };
    return stringIproductDescriptor;
}


#define CONFIG1_DESC_SIZE (9+8+9+5+5+4+5+7+9+7+7)

const uint8_t *USBCDC::configuration_desc(uint8_t index)
{
    uint8_t config_descriptor_temp[] = {
        // configuration descriptor
        9,                      // bLength
        2,                      // bDescriptorType
        LSB(CONFIG1_DESC_SIZE), // wTotalLength
        MSB(CONFIG1_DESC_SIZE),
        2,                      // bNumInterfaces
        1,                      // bConfigurationValue
        0,                      // iConfiguration
        0x80,                   // bmAttributes
        50,                     // bMaxPower

        // IAD to associate the two CDC interfaces
        0x08,                   // bLength
        0x0b,                   // bDescriptorType
        pluggedInterface,       // bFirstInterface
        0x02,                   // bInterfaceCount
        0x02,                   // bFunctionClass
        0x02,                   // bFunctionSubClass
        0,                      // bFunctionProtocol
        0,                      // iFunction

        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                      // bLength
        4,                      // bDescriptorType
        pluggedInterface,       // bInterfaceNumber
        0,                      // bAlternateSetting
        1,                      // bNumEndpoints
        0x02,                   // bInterfaceClass
        0x02,                   // bInterfaceSubClass
        0x01,                   // bInterfaceProtocol
        static_cast<uint8_t>((extraDescriptor != NULL) ? 0x5 : 0x0), // iInterface

        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x00,                   // bDescriptorSubtype
        0x10, 0x01,             // bcdCDC

        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x01,                   // bDescriptorSubtype
        0x03,                   // bmCapabilities
        1,                      // bDataInterface

        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x02,                   // bDescriptorSubtype
        0x06,                   // bmCapabilities

        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x06,                   // bDescriptorSubtype
        pluggedInterface,       // bMasterInterface
        uint8_t(pluggedInterface + 1),  // bSlaveInterface0

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        _int_in,                        // bEndpointAddress
        E_INTERRUPT,                    // bmAttributes (0x03=intr)
        LSB(CDC_MAX_PACKET_SIZE),       // wMaxPacketSize (LSB)
        MSB(CDC_MAX_PACKET_SIZE),       // wMaxPacketSize (MSB)
        16,                             // bInterval

        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                          // bLength
        4,                          // bDescriptorType
        uint8_t(pluggedInterface + 1),  // bInterfaceNumber
        0,                          // bAlternateSetting
        2,                          // bNumEndpoints
        0x0A,                       // bInterfaceClass
        0x00,                       // bInterfaceSubClass
        0x00,                       // bInterfaceProtocol
        0,                          // iInterface

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        _bulk_in,                   // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (LSB)
        MSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (MSB)
        0,                          // bInterval

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        _bulk_out,                  // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (LSB)
        MSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (MSB)
        0                           // bInterval
    };

    if (index == 0) {
        MBED_ASSERT(sizeof(config_descriptor_temp) == sizeof(_config_descriptor));
        memcpy(_config_descriptor, config_descriptor_temp, sizeof(_config_descriptor));
        return _config_descriptor;
    } else {
        return NULL;
    }
}

#endif
