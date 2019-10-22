#pragma once
#include "mbed_config.h"
#include <stdint.h>
#include <macros.h>

#ifndef __PINS_ARDUINO__
#define __PINS_ARDUINO__

// Number of pins defined in PinDescription array
#ifdef __cplusplus
extern "C" unsigned int PINCOUNT_fn();
#endif
#define PINS_COUNT           (PINCOUNT_fn())
#define NUM_DIGITAL_PINS     (18u)
#define NUM_ANALOG_INPUTS    (4u)
#define NUM_ANALOG_OUTPUTS   (0u)

// LEDs
// ----
#define PIN_LED     (13u)
#define LED_BUILTIN PIN_LED

// Analog pins
// -----------
#define PIN_A0 (14u)
#define PIN_A1 (15u)
#define PIN_A2 (16u)
#define PIN_A3 (17u)
static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
#define ADC_RESOLUTION 12

#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define SERIAL_HOWMANY 2
#define SERIAL1_TX PA_2
#define SERIAL1_RX PA_3
#define SERIAL2_TX PA_10
#define SERIAL2_RX PA_11

//#define SERIAL_CDC	1
//#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x805b
#define BOARD_NAME			"Nano WAN"

void _ontouch1200bps_();
uint8_t getUniqueSerialNumber(uint8_t* name);

// should we apply https://github.com/ARMmbed/lorawan-fota-demo/blob/7c8e932b2c99fdc0763e3953a9bcddbdd473d0ab/profiles/release.json#L15 ?

//#define MBED_CONF_APP_LORA_RADIO			SX1276
#define MBED_CONF_APP_LORA_SPI_MOSI			PA_7
#define MBED_CONF_APP_LORA_SPI_MISO			PA_6
#define MBED_CONF_APP_LORA_SPI_SCLK			PB_3
#define MBED_CONF_APP_LORA_CS				PA_15
#define MBED_CONF_APP_LORA_RESET			PC_0
#define MBED_CONF_APP_LORA_DIO0				PB_4
#define MBED_CONF_APP_LORA_DIO1				PB_1
#define MBED_CONF_APP_LORA_DIO2				PB_0
#define MBED_CONF_APP_LORA_DIO3				PC_13
#define MBED_CONF_APP_LORA_DIO4				NC
#define MBED_CONF_APP_LORA_DIO5				NC
#define MBED_CONF_APP_LORA_RF_SWITCH_CTL1	NC
#define MBED_CONF_APP_LORA_RF_SWITCH_CTL2	NC
#define MBED_CONF_APP_LORA_TXCTL			PC_2
#define MBED_CONF_APP_LORA_RXCTL			PA_1
#define MBED_CONF_APP_LORA_ANT_SWITCH		NC
#define MBED_CONF_APP_LORA_PWR_AMP_CTL		PC_1
#define MBED_CONF_APP_LORA_TCXO				PA_8

#endif // __PINS_ARDUINO__