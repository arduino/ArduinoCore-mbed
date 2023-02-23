#pragma once
#include <macros.h>
#include "PeripheralPins.h"

#ifndef __PINS_ARDUINO__
#define __PINS_ARDUINO__

#ifdef __cplusplus
extern "C" unsigned int PINCOUNT_fn();
extern "C" bool isBetaBoard();
#endif

// Booting
// ----
void bootM4();

extern PinName digitalPinToPinName(pin_size_t P);

// Pin count
// ----
#define PINS_COUNT           (PINCOUNT_fn())
#define NUM_DIGITAL_PINS     (22u)
#define NUM_ANALOG_INPUTS    (7u)
#define NUM_ANALOG_OUTPUTS   (1u)

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

static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
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

// Serial
#define PIN_SERIAL_RX (2ul)
#define PIN_SERIAL_TX (1ul)

// SPI
#define PIN_SPI_MISO  (10u)
#define PIN_SPI_MOSI  (8u)
#define PIN_SPI_SCK   (9u)
#define PIN_SPI_SS    (7u)

#define PIN_SPI_MISO1  (22u)
#define PIN_SPI_MOSI1  (20u)
#define PIN_SPI_SCK1   (21u)
#define PIN_SPI_SS1    (6u)

static const uint8_t SS   = PIN_SPI_SS;   // SPI Slave SS not used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// QSPI
#define QSPI_SO0        PD_11
#define QSPI_SO1        PF_9
#define QSPI_SO2        PE_2
#define QSPI_SO3        PD_13
#define QSPI_SCK        PF_10
#define QSPI_CS         PG_6

// Wire
#define PIN_WIRE_SDA        (11u)
#define PIN_WIRE_SCL        (12u)

#define SERIAL_HOWMANY		2
#define SERIAL1_TX			PA_9_ALT0
#define SERIAL1_RX			PA_10_ALT0

#define SERIAL2_TX			PB_6
#define SERIAL2_RX			PB_7
#define SERIAL2_RTS			PA_12
#define SERIAL2_CTS			PA_11

#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x025f
#define BOARD_NAME			"Nicla Vision"

#define DFU_MAGIC_SERIAL_ONLY_RESET   0xb0

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

#define WIRE_HOWMANY		3

#define I2C_SDA				(digitalPinToPinName(PIN_WIRE_SDA))
#define I2C_SCL				(digitalPinToPinName(PIN_WIRE_SCL))

#define I2C_SDA_INTERNAL	(PF_0)
#define I2C_SCL_INTERNAL	(PF_1)
#define I2C_SDA1			I2C_SDA_INTERNAL
#define I2C_SCL1			I2C_SCL_INTERNAL

#define I2C_SDA2			(13u)
#define I2C_SCL2			(14u)

//#define I2C_SDA3			(18u)
//#define I2C_SCL3			(19u)

#define SPI_HOWMANY			2

#define SPI_MISO			(digitalPinToPinName(PIN_SPI_MISO))
#define SPI_MOSI			(digitalPinToPinName(PIN_SPI_MOSI))
#define SPI_SCK				(digitalPinToPinName(PIN_SPI_SCK))
#define SPI_SS        (digitalPinToPinName(PIN_SPI_SS))

#define SPI_MISO1     (digitalPinToPinName(PIN_SPI_MISO1))
#define SPI_MOSI1     (digitalPinToPinName(PIN_SPI_MOSI1))
#define SPI_SCK1      (digitalPinToPinName(PIN_SPI_SCK1))
#define SPI_SS1       (digitalPinToPinName(PIN_SPI_SS1))

#define LSM6DS_DEFAULT_SPI SPI1
#define LSM6DS_INT         PA_1

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define USB_MAX_POWER	(500)

#endif //__PINS_ARDUINO__
