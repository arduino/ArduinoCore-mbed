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
#define NUM_DIGITAL_PINS     (32u)
#define NUM_ANALOG_INPUTS    (4u)
#define NUM_ANALOG_OUTPUTS   (0u)

// LEDs
// ----
#define PIN_LED     (25u)
#define LED_BUILTIN PIN_LED
#define LEDR        (22u)
#define LEDG        (23u)
#define LEDB        (24u)

// Analog pins
// -----------
#define PIN_A0 (26u)
#define PIN_A1 (27u)
#define PIN_A2 (28u)
#define PIN_A3 (29u)
/*
static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
*/
#define ADC_RESOLUTION 12

// Serial
#define PIN_SERIAL_RX (1ul)
#define PIN_SERIAL_TX (0ul)

// SPI
#define PIN_SPI_MISO  (10u)
#define PIN_SPI_MOSI  (8u)
#define PIN_SPI_SCK   (9u)
#define PIN_SPI_SS    (7u)

static const uint8_t SS   = PIN_SPI_SS;   // SPI Slave SS not used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// Wire
#define PIN_WIRE_SDA        (11u)
#define PIN_WIRE_SCL        (12u)

#define SERIAL_HOWMANY		3
#define SERIAL1_TX			(digitalPinToPinName(PIN_SERIAL_TX))
#define SERIAL1_RX			(digitalPinToPinName(PIN_SERIAL_RX))
#define SERIAL2_TX			(8)
#define SERIAL2_RX			(9)
#define SERIAL2_CTS			(10)
#define SERIAL2_RTS			(11)
#define SERIAL3_HW			(uart1)
#define SERIAL3_TX			(8)
#define SERIAL3_RX			(9)

#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x005e
#define BOARD_NAME			"Nano RP2040 Connect"

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

#define SPI_HOWMANY		(2)
#define SPI_HW			(spi0)
#define SPI_MISO		(4)
#define SPI_MOSI		(7)
#define SPI_SCK			(6)
#define SPI1_HW			(spi1)
#define SPI1_MISO		(8)
#define SPI1_MOSI		(11)
#define SPI1_SCK		(14)
#define SPI1_CS			(9)
#define SPI1_ACK		(10)

#define WIRE_HOWMANY	(1)
#define I2C_HW			(i2c0)
#define I2C_SDA			(12)
#define I2C_SCL			(13)

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define CRYPTO_WIRE		Wire

#define USB_MAX_POWER	(500)

#endif //__PINS_ARDUINO__
