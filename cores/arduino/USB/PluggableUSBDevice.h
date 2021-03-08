/*
 * Copyright (c) 2019, Arduino SA
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

#ifndef PLUGGABLEUSBDEVICE_H
#define PLUGGABLEUSBDEVICE_H

#include "Arduino.h"

/* These headers are included for child class. */
#include "USBDescriptor.h"
#include "USBDevice_Types.h"

#include "USBDevice.h"
#include "EndpointResolver.h"

namespace arduino {

// forward declaration
class PluggableUSBDevice;

namespace internal {
class PluggableUSBModule {
public:
    PluggableUSBModule(uint8_t numIfs) :
        numInterfaces(numIfs)
    { }
    void lock();
    void unlock();
    void assert_locked();
    void endpoint_abort(usb_ep_t endpoint);
    void endpoint_stall(usb_ep_t endpoint);
    bool read_start(usb_ep_t endpoint, uint8_t *buffer, uint32_t size);
    uint32_t read_finish(usb_ep_t endpoint);
    bool write_start(usb_ep_t endpoint, uint8_t *buffer, uint32_t size);
    uint32_t write_finish(usb_ep_t endpoint);

protected:
    virtual const uint8_t *configuration_desc(uint8_t index);
    virtual void callback_reset() {};
    virtual void callback_state_change(USBDevice::DeviceState new_state);
    virtual uint32_t callback_request(const USBDevice::setup_packet_t *setup, USBDevice::RequestResult *result, uint8_t** data);
    virtual bool callback_request_xfer_done(const USBDevice::setup_packet_t *setup, bool aborted);
    virtual bool callback_set_configuration(uint8_t configuration);
    virtual void callback_set_interface(uint16_t interface, uint8_t alternate);
    virtual void init(EndpointResolver& resolver);
    virtual const uint8_t *string_iinterface_desc();
    virtual uint8_t getProductVersion() = 0;

    uint8_t pluggedInterface;

    const uint8_t numInterfaces;

    arduino::internal::PluggableUSBModule *next = NULL;

    friend class ::arduino::PluggableUSBDevice;
};
}

class PluggableUSBDevice: public USBDevice {
public:

    /**
    * Basic constructor
    *
    * Construct this object optionally connecting and blocking until it is ready.
    *
    * @note Do not use this constructor in derived classes.
    *
    * @param connect_blocking true to perform a blocking connect, false to start in a disconnected state
    * @param vendor_id Your vendor_id
    * @param product_id Your product_id
    * @param product_release Your product_release
    */
    PluggableUSBDevice(uint16_t vendor_id = 0x1f00, uint16_t product_id = 0x2012);

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
    * @param vendor_id Your vendor_id
    * @param product_id Your product_id
    * @param product_release Your product_release
    */
    PluggableUSBDevice(USBPhy *phy, uint16_t vendor_id, uint16_t product_id);

    /**
     * Destroy this object
     *
     * Any classes which inherit from this class must call deinit
     * before this destructor runs.
     */
    virtual ~PluggableUSBDevice();

    void begin();
    bool plug(internal::PluggableUSBModule*node);

    /**
     * Acquire exclusive access to this instance USBDevice
     */
    void lock() {
        USBDevice::lock();
    }

    /**
     * Release exclusive access to this instance USBDevice
     */
    void unlock() {
        USBDevice::unlock();
    }

    /**
     * Assert that the current thread of execution holds the lock
     *
     */
    void assert_locked() {
        USBDevice::assert_locked();
    }

    uint8_t* find_descriptor(int a) {
        return USBDevice::find_descriptor(a);
    }

protected:
    /*
    * Get device descriptor. Warning: this method has to store the length of the report descriptor in reportLength.
    *
    * @returns pointer to the device descriptor
    */
    virtual const uint8_t *device_desc();

    /*
    * Get string product descriptor
    *
    * @returns pointer to the string product descriptor
    */
    virtual const uint8_t *string_iproduct_desc();

    /*
    * Get string interface descriptor
    *
    * @returns pointer to the string interface descriptor
    */
    virtual const uint8_t *string_iinterface_desc();

    /*
    * Get string interface descriptor
    *
    * @returns pointer to the string interface descriptor
    */
    virtual const uint8_t *string_iserial_desc();

    /*
    * Get string interface descriptor
    *
    * @returns pointer to the string interface descriptor
    */
    virtual const uint8_t *string_imanufacturer_desc();

    /*
    * Get configuration descriptor
    *
    * @param index descriptor index
    * @returns pointer to the configuration descriptor
    */
    virtual const uint8_t *configuration_desc(uint8_t index);

protected:

    virtual void callback_reset();
    virtual void callback_state_change(DeviceState new_state);
    virtual void callback_request(const setup_packet_t *setup);
    virtual void callback_request_xfer_done(const setup_packet_t *setup, bool aborted);
    virtual void callback_set_configuration(uint8_t configuration);
    virtual void callback_set_interface(uint16_t interface, uint8_t alternate);

private:
    void init();

    uint8_t _config_descriptor[400];
    uint8_t _config_iserial[64];
    uint8_t _config_iproductdescriptor[64];
    uint8_t _config_iinterfacedescriptor[64];

    uint8_t lastIf;
    internal::PluggableUSBModule* rootNode;
    friend class ::arduino::internal::PluggableUSBModule;
};
}

arduino::PluggableUSBDevice& PluggableUSBD();

#endif
