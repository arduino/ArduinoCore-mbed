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
#define NUM_DIGITAL_PINS     (103u)
#define NUM_ANALOG_INPUTS    (14u)
#define NUM_ANALOG_OUTPUTS   (2u)

// LEDs
// ----
#define PIN_LED     (87u)
#define LED_BUILTIN PIN_LED
#define LEDR        (86u)
#define LEDG        (87u)
#define LEDB        (88u)

// Analog pins
// -----------
#define PIN_A0 (76u)
#define PIN_A1 (77u)
#define PIN_A2 (78u)
#define PIN_A3 (79u)
#define PIN_A4 (80u)
#define PIN_A5 (81u)
#define PIN_A6 (82u)
#define PIN_A7 (83u)
#define PIN_A12 (84u)
#define PIN_A13 (85u)

static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
static const uint8_t A4  = PIN_A4;
static const uint8_t A5  = PIN_A5;
static const uint8_t A6  = PIN_A6;
static const uint8_t A7  = PIN_A7;
static const uint8_t A12  = PIN_A12;
static const uint8_t A13  = PIN_A13;
#define ADC_RESOLUTION 12

#ifdef __cplusplus
#include "pure_analog_pins.h"
#endif

//DACs
#define DAC           A12
#define DAC_0         A12
#define DAC_1         A13

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
#define D22 (22u)
#define D23 (23u)
#define D24 (24u)
#define D25 (25u)
#define D26 (26u)
#define D27 (27u)
#define D28 (28u)
#define D29 (29u)
#define D30 (30u)
#define D31 (31u)
#define D32 (32u)
#define D33 (33u)
#define D34 (34u)
#define D35 (35u)
#define D36 (36u)
#define D37 (37u)
#define D38 (38u)
#define D39 (39u)
#define D40 (40u)
#define D41 (41u)
#define D42 (42u)
#define D43 (43u)
#define D44 (44u)
#define D45 (45u)
#define D46 (46u)
#define D47 (47u)
#define D48 (48u)
#define D49 (49u)
#define D50 (50u)
#define D51 (51u)
#define D52 (52u)
#define D53 (53u)
#define D54 (54u)
#define D55 (55u)
#define D56 (56u)
#define D57 (57u)
#define D58 (58u)
#define D59 (59u)
#define D60 (60u)
#define D61 (61u)
#define D62 (62u)
#define D63 (63u)
#define D64 (64u)
#define D65 (65u)
#define D66 (66u)
#define D67 (67u)
#define D68 (68u)
#define D69 (69u)
#define D70 (70u)
#define D71 (71u)
#define D72 (72u)
#define D73 (73u)
#define D74 (74u)
#define D75 (75u)
#define D76 (76u)
#define D77 (77u)
#define D78 (78u)
#define D79 (79u)
#define D80 (80u)
#define D81 (81u)
#define D82 (82u)
#define D83 (83u)
#define D84 (84u)
#define D85 (85u)
#define D86 (86u)
#define D87 (87u)
#define D88 (88u)
#define D89 (89u)
#define D90 (90u)
#define D91 (91u)
#define D92 (92u)
#define D93 (93u)
#define D94 (94u)
#define D95 (95u)
#define D96 (96u)
#define D97 (97u)
#define D98 (98u)
#define D99 (99u)
#define D100 (100u)
#define D101 (101u)

// SPI
#define PIN_SPI_MISO  (89u)
#define PIN_SPI_MOSI  (90u)
#define PIN_SPI_SCK   (91u)
// Same as SPI1 for backwards compatibility
#define PIN_SPI_SS    (10u)

#define PIN_SPI_MISO1  (12u)
#define PIN_SPI_MOSI1  (11u)
#define PIN_SPI_SCK1   (13u)
#define PIN_SPI_SS1    (10u)

static const uint8_t SS   = PIN_SPI_SS;   // SPI Slave SS not used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// QSPI
#define QSPI_SO0        PD_11
#define QSPI_SO1        PD_12
#define QSPI_SO2        PE_2
#define QSPI_SO3        PF_6
#define QSPI_SCK        PF_10
#define QSPI_CS         PG_6

#define SERIAL_HOWMANY		4

// Serial
#define SERIAL1_RX 			(0ul)
#define SERIAL1_TX 			(1ul)

#define SERIAL2_TX			(18u)
#define SERIAL2_RX			(19u)

#define SERIAL3_TX			(16u)
#define SERIAL3_RX			(17u)

#define SERIAL4_TX			(14u)
#define SERIAL4_RX			(15u)

#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x0266
#define BOARD_NAME			"Giga"

#define DFU_MAGIC_SERIAL_ONLY_RESET   0xb0

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

// Wire

#define WIRE_HOWMANY		3

#define I2C_SDA				(digitalPinToPinName(20))
#define I2C_SCL				(digitalPinToPinName(21))

#define I2C_SDA1			(PH_12)
#define I2C_SCL1			(PB_6_ALT0)

#define I2C_SDA2			(digitalPinToPinName(9))
#define I2C_SCL2			(digitalPinToPinName(8))

#define PIN_WIRE_SCL         (21u)
#define PIN_WIRE_SDA         (20u)

#define SPI_HOWMANY			2

#define SPI_MISO			(digitalPinToPinName(PIN_SPI_MISO))
#define SPI_MOSI			(digitalPinToPinName(PIN_SPI_MOSI))
#define SPI_SCK				(digitalPinToPinName(PIN_SPI_SCK))
#define SPI_SS        (digitalPinToPinName(PIN_SPI_SS))

#define SPI_MISO1     (digitalPinToPinName(PIN_SPI_MISO1))
#define SPI_MOSI1     (digitalPinToPinName(PIN_SPI_MOSI1))
#define SPI_SCK1      (digitalPinToPinName(PIN_SPI_SCK1))
#define SPI_SS1       (digitalPinToPinName(PIN_SPI_SS1))

//#define LSM6DS_DEFAULT_SPI SPI1
//#define LSM6DS_INT         PA_1

#define CRYPTO_WIRE   Wire1

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define USB_MAX_POWER	(500)

#define CAN_HOWMANY       2

#define PIN_CAN0_TX       (PB_13)
#define PIN_CAN0_RX       (PB_5)

#define PIN_CAN1_TX       (PH_13)
#define PIN_CAN1_RX       (PB_8)

#endif //__PINS_ARDUINO__
