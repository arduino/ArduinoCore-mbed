#pragma once
#include <macros.h>
#include "PeripheralPins.h"

#ifndef __PINS_ARDUINO__
#define __PINS_ARDUINO__

#ifdef __cplusplus
extern "C" unsigned int PINCOUNT_fn();
extern "C" bool isBetaBoard();
#endif
#define PINS_COUNT           (PINCOUNT_fn())
#define NUM_DIGITAL_PINS     (22u)
#define NUM_ANALOG_INPUTS    (7u)
#define NUM_ANALOG_OUTPUTS   (1u)

extern PinName digitalPinToPinName(pin_size_t P);

// LEDs
// ----
#define PIN_LED     (24u)
#define LED_BUILTIN PIN_LED
#define LEDR        (23u)
#define LEDG        (24u)
#define LEDB        (25u)

// Analog pins
// -----------
#define PIN_A0 (15u)
#define PIN_A1 (16u)
#define PIN_A2 (17u)
#define PIN_A3 (18u)
#define PIN_A4 (19u)
#define PIN_A5 (20u)
#define PIN_A6 (21u)
#define PIN_A7 (22u)

static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
static const uint8_t A4  = PIN_A4;
static const uint8_t A5  = PIN_A5;
static const uint8_t A6  = PIN_A6;
static const uint8_t A7  = PIN_A7;
#define ADC_RESOLUTION 12

// Digital pins
// -----------
#define D0  (0u)
#define D1  (1u)
#define D2  (2u)
#define D3  (3u)
#define D4  (4u)
#define D5  (5u)
#define D6  (6u)
#define D7  (7u)
#define D8  (8u)
#define D9  (9u)
#define D10 (10u)
#define D11 (11u)
#define D12 (12u)
#define D13 (13u)
#define D14 (14u)
#define D15 (15u)
#define D16 (u16)
#define D17 (u17)
#define D18 (u18)
#define D19 (u19)
#define D20 (u20)
#define D21 (u21)

//DACs
#define DAC           (A6)

// Serial
#define PIN_SERIAL_RX (13ul)
#define PIN_SERIAL_TX (14ul)

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

#define SerialHCI			UART2

#define SERIAL_HOWMANY		2

#define SERIAL2_TX			PA_15
#define SERIAL2_RX			PF_6
#define SERIAL2_RTS			PF_8
#define SERIAL2_CTS			PF_9

#define SERIAL1_TX			(digitalPinToPinName(PIN_SERIAL_TX))
#define SERIAL1_RX			(digitalPinToPinName(PIN_SERIAL_RX))

//#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x025b
#define BOARD_NAME			"Envie M4"

#define DFU_MAGIC_SERIAL_ONLY_RESET   0xb0

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

#define WIRE_HOWMANY		3

#define I2C_SDA				(digitalPinToPinName(PIN_WIRE_SDA))
#define I2C_SCL				(digitalPinToPinName(PIN_WIRE_SCL))

#define I2C_SDA_INTERNAL	(PB_7)
#define I2C_SCL_INTERNAL	(PB_6)
#define I2C_SDA1			I2C_SDA_INTERNAL
#define I2C_SCL1			I2C_SCL_INTERNAL
#define I2C_SDA2			(PH_12)
#define I2C_SCL2			(PH_11)

#define SPI_HOWMANY			1

#define SPI_MISO			(digitalPinToPinName(PIN_SPI_MISO))
#define SPI_MOSI			(digitalPinToPinName(PIN_SPI_MOSI))
#define SPI_SCK				(digitalPinToPinName(PIN_SPI_SCK))

#define digitalPinToPort(P)		(digitalPinToPinName(P)/16)

#define CRYPTO_WIRE		Wire1

#endif //__PINS_ARDUINO__
