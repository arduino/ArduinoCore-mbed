#pragma once
#include <macros.h>
#include <stdint.h>

#ifndef __PINS_ARDUINO__
#define __PINS_ARDUINO__

#ifdef __cplusplus
extern "C" unsigned int PINCOUNT_fn();
#endif

// Pin count
// ----
#define PINS_COUNT           (PINCOUNT_fn())
#define NUM_DIGITAL_PINS     (30u)
#define NUM_ANALOG_INPUTS    (4u)
#define NUM_ANALOG_OUTPUTS   (0u)

extern PinName digitalPinToPinName(pin_size_t P);

// LEDs
// ----
#define PIN_LED     (13u)
#define LED_BUILTIN PIN_LED
//#define LEDR        (20u)
//#define LEDG        (21u)
//#define LEDB        (13u)

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

// PDM Interfaces
// ---------------
#define PIN_PDM_CLK	 (23)
#define PIN_PDM_DIN	 (22)

// IMU MLC Interrupt
#define INT_IMU      (21)

// Serial
#define PIN_SERIAL_RX (0ul)
#define PIN_SERIAL_TX (1ul)

// SPI
#define PIN_SPI_MISO  (12u)
#define PIN_SPI_MOSI  (11u)
#define PIN_SPI_SCK   (13u)
#define PIN_SPI_SS    (10u)

static const uint8_t SS   = PIN_SPI_SS;   // SPI Slave SS not used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// Wire
#define PIN_WIRE_SDA        (18u)
#define PIN_WIRE_SCL        (19u)

#define SERIAL_HOWMANY		3
#define SERIAL1_TX			(digitalPinToPinName(PIN_SERIAL_TX))
#define SERIAL1_RX			(digitalPinToPinName(PIN_SERIAL_RX))
#define SERIAL2_TX			(digitalPinToPinName(25))
#define SERIAL2_RX			(digitalPinToPinName(26))
#define SERIAL2_CTS			(digitalPinToPinName(27))
#define SERIAL2_RTS			(digitalPinToPinName(28))
#define SERIAL3_TX			(digitalPinToPinName(25))
#define SERIAL3_RX			(digitalPinToPinName(26))

#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x005e
#define BOARD_NAME			"Nano RP2040 Connect"

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

#define SPI_HOWMANY		(2)
#define SPI_MISO		(digitalPinToPinName(PIN_SPI_MISO))
#define SPI_MOSI		(digitalPinToPinName(PIN_SPI_MOSI))
#define SPI_SCK			(digitalPinToPinName(PIN_SPI_SCK))

#define SPI_MISO1		(digitalPinToPinName(25))
#define SPI_MOSI1		(digitalPinToPinName(28))
#define SPI_SCK1		(digitalPinToPinName(29))

#define NINA_RESETN		(24u)
#define SerialNina		Serial3
#define SerialHCI		Serial2

//#define NINA_GPIOIRQ	(21u) // LEDG pin (GPIO26 on NINA)
#define NINA_GPIO0		(20u) // real GPIO0 on NINA

#define SPIWIFI_SS		(26u)
#define SPIWIFI_ACK		(27u)
#define SPIWIFI_RESET	(NINA_RESETN)
#define SPIWIFI 		(SPI1)

#define WIRE_HOWMANY	(1)
#define I2C_HW			(i2c0)
#define I2C_SDA			(digitalPinToPinName(PIN_WIRE_SDA))
#define I2C_SCL			(digitalPinToPinName(PIN_WIRE_SCL))

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define CRYPTO_WIRE		Wire

#define USB_MAX_POWER	(500)

#ifdef __cplusplus
#include "nina_pins.h"
#endif

#endif //__PINS_ARDUINO__
