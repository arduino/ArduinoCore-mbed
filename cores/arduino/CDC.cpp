#include "CDC.h"

#ifdef SERIAL_CDC
USBSerial arduino::internal::_serial(false, BOARD_VENDORID, BOARD_PRODUCTID);

const uint8_t *USBCDC::device_desc()
{
    uint8_t ep0_size = endpoint_max_packet_size(0x00);
    uint8_t device_descriptor_temp[] = {
        18,                   // bLength
        1,                    // bDescriptorType
        0x00, 0x02,           // bcdUSB
        2,                    // bDeviceClass
        0,                    // bDeviceSubClass
        0,                    // bDeviceProtocol
        ep0_size,             // bMaxPacketSize0
        (uint8_t)(LSB(vendor_id)), (uint8_t)(MSB(vendor_id)),  // idVendor
        (uint8_t)(LSB(product_id)), (uint8_t)(MSB(product_id)),// idProduct
        0x00, 0x01,           // bcdDevice
        1,                    // iManufacturer
        2,                    // iProduct
        3,                    // iSerialNumber
        1                     // bNumConfigurations
    };
    MBED_ASSERT(sizeof(device_descriptor_temp) == sizeof(device_descriptor));
    memcpy(device_descriptor, device_descriptor_temp, sizeof(device_descriptor));
    return device_descriptor;
}

const uint8_t *USBCDC::string_iinterface_desc()
{
    static const uint8_t stringIinterfaceDescriptor[] = {
        0x08,
        STRING_DESCRIPTOR,
        'C', 0, 'D', 0, 'C', 0,
    };
    return stringIinterfaceDescriptor;
}

const uint8_t *USBCDC::string_iproduct_desc()
{
    static const uint8_t stringIproductDescriptor[] = {
        0x18,
        STRING_DESCRIPTOR,
        'N', 0, 'A', 0, 'N', 0, 'O', 0, ' ', 0, '3', 0, '3', 0, ' ', 0, 'B', 0, 'L', 0, 'E', 0
    };
    return stringIproductDescriptor;
}

const uint8_t *USBDevice::string_imanufacturer_desc()
{
    static const uint8_t string_imanufacturer_descriptor[] = {
        0x10,                                            /*bLength*/
        STRING_DESCRIPTOR,                               /*bDescriptorType 0x03*/
        'A', 0, 'r', 0, 'd', 0, 'u', 0, 'i', 0, 'n', 0, 'o', 0, /*bString iManufacturer - Arduino*/
    };
    return string_imanufacturer_descriptor;
}

#ifdef HAS_UNIQUE_ISERIAL_DESCRIPTOR

static uint8_t _internal_string_iserial_descriptor[34] = {
    0x2,                                             /*bLength*/
    STRING_DESCRIPTOR,                               /*bDescriptorType 0x03*/
    /*bString iSerial - memcpy at runtime*/
};
uint8_t *USBDevice::string_iserial_desc()
{
    _internal_string_iserial_descriptor[0] += getUniqueSerialNumber(&_internal_string_iserial_descriptor[2]);

    return _internal_string_iserial_descriptor;
}
#endif

#endif