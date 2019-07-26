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
#include "stdint.h"
#include "PluggableUSBDevice.h"
#include "EndpointResolver.h"
#include "usb_phy_api.h"

void arduino::internal::PluggableUSBModule::lock() {
    PluggableUSBD().lock();
}

void arduino::internal::PluggableUSBModule::unlock() {
    PluggableUSBD().unlock();
}

void arduino::internal::PluggableUSBModule::assert_locked() {
    PluggableUSBD().assert_locked();
}

void arduino::internal::PluggableUSBModule::endpoint_abort(usb_ep_t endpoint) {
    PluggableUSBD().endpoint_abort(endpoint);
}

bool arduino::internal::PluggableUSBModule::read_start(usb_ep_t endpoint, uint8_t *buffer, uint32_t size) {
    return PluggableUSBD().read_start(endpoint, buffer, size);
}

uint32_t arduino::internal::PluggableUSBModule::read_finish(usb_ep_t endpoint) {
    return PluggableUSBD().read_finish(endpoint);
}

bool arduino::internal::PluggableUSBModule::write_start(usb_ep_t endpoint, uint8_t *buffer, uint32_t size) {
    return PluggableUSBD().write_start(endpoint, buffer, size);
}

uint32_t arduino::internal::PluggableUSBModule::write_finish(usb_ep_t endpoint) {
    return PluggableUSBD().write_finish(endpoint);
}

PluggableUSBDevice::PluggableUSBDevice(uint16_t vendor_id, uint16_t product_id)
    : USBDevice(get_usb_phy(), vendor_id, product_id, 1 << 8)
{
}

PluggableUSBDevice::PluggableUSBDevice(USBPhy *phy, uint16_t vendor_id, uint16_t product_id)
    : USBDevice(phy, vendor_id, product_id, 1 << 8)
{
}

PluggableUSBDevice::~PluggableUSBDevice()
{
    deinit();
}

void PluggableUSBDevice::begin()
{
    init();
    connect();
}

bool PluggableUSBDevice::plug(internal::PluggableUSBModule*node)
{
    if (!rootNode) {
        rootNode = node;
    } else {
        internal::PluggableUSBModule*current = rootNode;
        while (current->next) {
            current = current->next;
        }
        current->next = node;
    }

    node->pluggedInterface = lastIf;
    lastIf += node->numInterfaces;
    return true;
}

void PluggableUSBDevice::init()
{
    EndpointResolver resolver(endpoint_table());
    resolver.endpoint_ctrl(64);

    // calls all plugged modules init()
    arduino::internal::PluggableUSBModule* node;
    for (node = rootNode; node; node = node->next) {
        node->init(resolver);
    }
}

void PluggableUSBDevice::callback_reset()
{
    // calls all plugged modules callback_reset()
    arduino::internal::PluggableUSBModule* node;
    for (node = rootNode; node; node = node->next) {
        node->callback_reset();
    }
};

void PluggableUSBDevice::callback_state_change(DeviceState new_state)
{
    arduino::internal::PluggableUSBModule* node;
    for (node = rootNode; node; node = node->next) {
        node->callback_state_change(new_state);
    }
}

void PluggableUSBDevice::callback_request(const setup_packet_t *setup)
{
    arduino::internal::PluggableUSBModule* node;
    USBDevice::RequestResult result;
    uint8_t *data;
    uint32_t size = 0;

    for (node = rootNode; node; node = node->next) {
        size = node->callback_request(setup, &result, &data);
    }
    complete_request(result, data, size);
}

void PluggableUSBDevice::callback_request_xfer_done(const setup_packet_t *setup, bool aborted)
{
    if (aborted) {
        complete_request_xfer_done(false);
        return;
    }

    arduino::internal::PluggableUSBModule* node;
    for (node = rootNode; node; node = node->next) {
        node->callback_request_xfer_done(setup, aborted);
    }
    // FIXME!
    complete_request_xfer_done(true);
}

void PluggableUSBDevice::callback_set_configuration(uint8_t configuration)
{
    arduino::internal::PluggableUSBModule* node;
    bool ret = false;
    for (node = rootNode; node; node = node->next) {
        ret = node->callback_set_configuration(configuration);
    }
    complete_set_configuration(ret);
}

void PluggableUSBDevice::callback_set_interface(uint16_t interface, uint8_t alternate)
{
    arduino::internal::PluggableUSBModule* node;
    for (node = rootNode; node; node = node->next) {
        node->callback_set_interface(interface, alternate);
    }
    complete_set_interface(true);
}

const uint8_t *PluggableUSBDevice::device_desc()
{
    uint8_t ep0_size = endpoint_max_packet_size(0x00);
    uint8_t device_descriptor_temp[] = {
        DEVICE_DESCRIPTOR_LENGTH,       /* bLength */
        DEVICE_DESCRIPTOR,              /* bDescriptorType */
        LSB(USB_VERSION_2_0),           /* bcdUSB (LSB) */
        MSB(USB_VERSION_2_0),           /* bcdUSB (MSB) */
        239,                            /* bDeviceClass */
        2,                              /* bDeviceSubClass */
        1,                              /* bDeviceprotocol */
        (uint8_t)ep0_size,              /* bMaxPacketSize0 */
        (uint8_t)(LSB(vendor_id)),                 /* idVendor (LSB) */
        (uint8_t)(MSB(vendor_id)),                 /* idVendor (MSB) */
        (uint8_t)(LSB(product_id)),                /* idProduct (LSB) */
        (uint8_t)(MSB(product_id)),                /* idProduct (MSB) */
        (uint8_t)(LSB(product_release)),           /* bcdDevice (LSB) */
        (uint8_t)(MSB(product_release)),           /* bcdDevice (MSB) */
        STRING_OFFSET_IMANUFACTURER,    /* iManufacturer */
        STRING_OFFSET_IPRODUCT,         /* iProduct */
        STRING_OFFSET_ISERIAL,          /* iSerialNumber */
        0x01                            /* bNumConfigurations */
    };
    MBED_ASSERT(sizeof(device_descriptor_temp) == sizeof(device_descriptor));
    memcpy(device_descriptor, device_descriptor_temp, sizeof(device_descriptor));
    return device_descriptor;
}

const uint8_t *PluggableUSBDevice::string_iinterface_desc()
{
    static const uint8_t stringIinterfaceDescriptor[] = {
        0x08,
        STRING_DESCRIPTOR,
        'C', 0, 'D', 0, 'C', 0,
    };
    return stringIinterfaceDescriptor;
}

const uint8_t *PluggableUSBDevice::string_iproduct_desc()
{
    static const uint8_t stringIproductDescriptor[] = {
        0x16,
        STRING_DESCRIPTOR,
        'C', 0, 'D', 0, 'C', 0, ' ', 0, 'D', 0, 'E', 0, 'V', 0, 'I', 0, 'C', 0, 'E', 0
    };
    return stringIproductDescriptor;
}

const uint8_t *PluggableUSBDevice::string_imanufacturer_desc()
{
    static const uint8_t string_imanufacturer_descriptor[] = {
        0x12,                                            /*bLength*/
        STRING_DESCRIPTOR,                               /*bDescriptorType 0x03*/
        'm', 0, 'b', 0, 'e', 0, 'd', 0, '.', 0, 'o', 0, 'r', 0, 'g', 0, /*bString iManufacturer - mbed.org*/
    };
    return string_imanufacturer_descriptor;
}

#ifdef HAS_UNIQUE_ISERIAL_DESCRIPTOR
const uint8_t *PluggableUSBDevice::string_iserial_desc()
{
    _config_iserial[0] = 2;
    _config_iserial[1] = STRING_DESCRIPTOR;
    _config_iserial[0] += getUniqueSerialNumber(&_config_iserial[2]);
    return _config_iserial;
}
#endif

const uint8_t *PluggableUSBDevice::configuration_desc(uint8_t index)
{
    #define TOTAL_DESCRIPTOR_LENGTH 0xFFFF
    // Create a huge configuration descriptor using all the pluggable ones
    // jump the first 9 bytes (config descriptor) for each "module" and recompose it later
    uint8_t configuration_descriptor_temp[] = {
        CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
        CONFIGURATION_DESCRIPTOR,           // bDescriptorType
        LSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (LSB)
        MSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (MSB)
        lastIf,                             // bNumInterfaces
        0x01,                               // bConfigurationValue
        0x00,                               // iConfiguration
        C_RESERVED | C_SELF_POWERED,        // bmAttributes
        C_POWER(100),                       // bMaxPower
    };
    memcpy(_config_descriptor, configuration_descriptor_temp, sizeof(configuration_descriptor_temp));
    int size = sizeof(configuration_descriptor_temp);
    arduino::internal::PluggableUSBModule* node;
    for (node = rootNode; node; node = node->next) {
        uint8_t* desc = (uint8_t*)node->configuration_desc(0);
        int len = (desc[3] << 8 | desc[2]) - 9;
        memcpy(&_config_descriptor[size], &desc[9], len);
        size += len;
    }
    _config_descriptor[2] = (size & 0xFF);
    _config_descriptor[3] = (size & 0xFF00) >> 8;
    return _config_descriptor;
}

PluggableUSBDevice& PluggableUSBD()
{
    static arduino::PluggableUSBDevice obj(BOARD_VENDORID, BOARD_PRODUCTID);
    return obj;
}
