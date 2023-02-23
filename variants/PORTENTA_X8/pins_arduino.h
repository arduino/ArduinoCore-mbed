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
// void bootM4();

extern PinName digitalPinToPinName(pin_size_t P);

// Pin count
// ----
#define PINS_COUNT           (PINCOUNT_fn())
#define NUM_DIGITAL_PINS     (36u)
#define NUM_ANALOG_INPUTS    (8u)
#define NUM_ANALOG_OUTPUTS   (1u)

// LEDs
// ----
#define PIN_LED     (33u)
#define LED_BUILTIN PIN_LED

// Analog pins
// -----------
#define PIN_A0 (7u)
#define PIN_A1 (8u)
#define PIN_A2 (9u)
#define PIN_A3 (10u)
#define PIN_A4 (11u)
#define PIN_A5 (12u)
#define PIN_A6 (13u)
#define PIN_A7 (14u)

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
#define D16 (16u)
#define D17 (17u)
#define D18 (18u)
#define D19 (19u)
#define D20 (20u)
#define D21 (21u)

// Serial
#define PIN_SERIAL_RX (22ul)
#define PIN_SERIAL_TX (21ul)

#define SERIAL_HOWMANY		1
#define SERIAL1_TX			(digitalPinToPinName(PIN_SERIAL_TX))
#define SERIAL1_RX			(digitalPinToPinName(PIN_SERIAL_RX))

// SPI
#define PIN_SPI_MISO  (0u)
#define PIN_SPI_MOSI  (A6)
#define PIN_SPI_SCK   (A5)
#define PIN_SPI_SS    (1u)

static const uint8_t SS   = PIN_SPI_SS;   // SPI Slave SS not used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// Wire
#define PIN_WIRE_SDA        (38u)
#define PIN_WIRE_SCL        (29u)

#define RPC_SERIAL				1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x0061
#define BOARD_NAME			"Portenta X8"

#define DFU_MAGIC_SERIAL_ONLY_RESET   0xb0

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

#define WIRE_HOWMANY		1

#define I2C_SDA				(PC_9)		// marked as PWM8
#define I2C_SCL				(PA_8)		// marked as PWM6

#define SPI_HOWMANY			1

#define SPI_MISO			(digitalPinToPinName(PIN_SPI_MISO))		// marked as 0
#define SPI_MOSI			(digitalPinToPinName(PIN_SPI_MOSI))		// marked as A6
#define SPI_SCK				(digitalPinToPinName(PIN_SPI_SCK))		// marked as A5

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

#define SERIAL_PORT_USBVIRTUAL      Serial
#define SERIAL_PORT_MONITOR         Serial
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define USB_MAX_POWER	(500)

#ifdef __cplusplus

#include "api/HardwareSerial.h"

#define MACRO_ERROR_SERIAL_STR "\n\n****\nPlease include SerialRPC library to use Serial\n****\n\n"
#define MACRO_ERROR_SERIAL __attribute__ ((error(MACRO_ERROR_SERIAL_STR)))

class ErrorSerialClass : public arduino::HardwareSerial {
public:
	ErrorSerialClass() {};
	~ErrorSerialClass() {};
	void MACRO_ERROR_SERIAL begin(long unsigned int);
	void MACRO_ERROR_SERIAL begin(long unsigned int, uint16_t);
	size_t MACRO_ERROR_SERIAL write(uint8_t);
	MACRO_ERROR_SERIAL operator bool();
	void MACRO_ERROR_SERIAL end();
	int MACRO_ERROR_SERIAL peek();
	int MACRO_ERROR_SERIAL read();
	int MACRO_ERROR_SERIAL available();
	void MACRO_ERROR_SERIAL flush();
};

extern ErrorSerialClass ErrorSerial;

#endif


#endif //__PINS_ARDUINO__
