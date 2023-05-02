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
#define D15 #error Pin cannot be used as digital pin.
#define D16 #error Pin cannot be used as digital pin.
#define D17 #error Pin cannot be used as digital pin.
#define D18 #error Pin cannot be used as digital pin.
#define D19 (19u)
#define D20 (20u)
#define D21 (21u)

//DACs
#define DAC           A6

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

// QSPI
#define QSPI_SO0        PD_11
#define QSPI_SO1        PD_12
#define QSPI_SO2        PF_7
#define QSPI_SO3        PD_13
#define QSPI_SCK        PF_10
#define QSPI_CS         PG_6

// Wire
#define PIN_WIRE_SDA        (11u)
#define PIN_WIRE_SCL        (12u)

#define SERIAL_HOWMANY		3
#define SERIAL1_TX			(digitalPinToPinName(PIN_SERIAL_TX))
#define SERIAL1_RX			(digitalPinToPinName(PIN_SERIAL_RX))

#define SERIAL2_TX			PA_15
#define SERIAL2_RX			PF_6
#define SERIAL2_RTS			PF_8
#define SERIAL2_CTS			PF_9

#define SERIAL3_TX			(PJ_8)
#define SERIAL3_RX			(PJ_9)

#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x025b
#define BOARD_NAME			"Envie M7"

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

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define SerialLoRa		Serial3
#define LORA_BOOT0      (PG_7)
#define LORA_RESET      (PC_7)
#define LORA_IRQ_DUMB   (PJ_11)

#define CRYPTO_WIRE		Wire1

#define USB_MAX_POWER	(500)

#define CAN_HOWMANY       1

#define PIN_CAN0_TX       (PH_13) /* Labeled CAN1_TX on high-density connector. */
#define PIN_CAN0_RX       (PB_8)  /* Labeled CAN1_RX on high-density connector. */

#endif //__PINS_ARDUINO__
