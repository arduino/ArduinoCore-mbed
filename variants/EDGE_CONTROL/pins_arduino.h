#pragma once
#include <stdint.h>
#include <macros.h>

#ifndef __PINS_ARDUINO__
#define __PINS_ARDUINO__

#define ANALOG_CONFIG

/* Analog reference options 
 * Different possibilities available combining Reference and Gain
 */
enum _AnalogReferenceMode
{
  AR_VDD,         // 3.3 V
  AR_INTERNAL,    // 0.6 V
  AR_INTERNAL1V2, // 1.2 V
  AR_INTERNAL2V4  // 2.4 V
};

/* Analog acquisition time options */
enum _AnalogAcquisitionTime
{
  AT_3_US,         
  AT_5_US,    
  AT_10_US, // Default value
  AT_15_US,
  AT_20_US,  
  AT_40_US  
};

// Frequency of the board main oscillator
#define VARIANT_MAINOSC (32768ul)

// Master clock frequency
#define VARIANT_MCK     (64000000ul)

// Pins
// ----

// Number of pins defined in PinDescription array
#ifdef __cplusplus
extern "C" unsigned int PINCOUNT_fn();
#endif
#define PINS_COUNT           (PINCOUNT_fn())
#define NUM_DIGITAL_PINS     (32u)
#define NUM_ANALOG_INPUTS    (8u)
#define NUM_ANALOG_OUTPUTS   (0u)

extern PinName digitalPinToPinName(pin_size_t P);

// Triac pins
// ----------
#define CMD_TRIAC_1 (0u)
#define CMD_TRIAC_2 (1u)
#define CMD_TRIAC_3 (2u)
#define CMD_TRIAC_4 (3u)

#define CMD_TRIAC_CLK (0u)
#define CMD_TRIAC_DATA (1u)

// IRQ Channels
// ------------
#define IRQ_CH1 (4u)
#define IRQ_CH2 (5u)
#define IRQ_CH3 (6u)
#define IRQ_CH4 (7u)
#define IRQ_CH5 (8u)
#define IRQ_CH6 (9u)

// Sensors
// -------
#define SENSOR_CAPTURE (10u)
#define SENSOR_COMMON (11u)
#define SENSOR_CALIB (12u)
#define SENSOR_CAPTURE_A (13u)
#define SENSOR_INPUT_ADC (14u)

// Pulse
// -------
#define PULSE_DIRECTION (19u)
#define PULSE_STROBE (20u)

// MKRs
// ----
#define ON_MKR1 (21u)
#define RXD_MKR1 (22u)
#define TXD_MKR1 (23u)

#define ON_MKR2 (24u)
#define RXD_MKR2 (25u)
#define TXD_MKR2 (26u)


// SD
// --
#define PIN_SD_CS (29u)
#define PIN_SD_CLK (31u)
#define PIN_SD_MOSI (32u)
#define PIN_SD_MISO (33u)

// QSPI
// ----
#define PIN_QSPIDCS (30u)
#define PIN_GPIOCLK (31u)
#define PIN_QSPID0 (32u)
#define PIN_QSPID1 (33u)
#define PIN_QSPID2 (34u)
#define PIN_QSPID3 (35u)

// Power
// -----
#define V_REF (17u)
#define VBAT_PROBE (18u)
#define GATED_19V_ENABLE (37u)
#define GATED_VBAT_ENABLE (38u)
#define GATED_3V3_ENABLE_N (39u)
#define POWER_ON (36u)

// Analog pins
// -----------
#define PIN_A0 (11u) // SENSOR_COMMON
#define PIN_A1 (12u) // SENSOR_CALIB
#define PIN_A2 (13u) // SENSOR_CAPTURE_A
#define PIN_A3 (14u) // SENSOR_INPUT_ADC
#define PIN_A4 (15u) // I2C_SDA_2
#define PIN_A5 (16u) // I2C_SCL_2
#define PIN_A6 (17u) // V_REF
#define PIN_A7 (18u) // VBAT_PROBE
static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
static const uint8_t A4  = PIN_A4;
static const uint8_t A5  = PIN_A5;
static const uint8_t A6  = PIN_A6;
static const uint8_t A7  = PIN_A7;
#define ADC_RESOLUTION 12

/*
 * Serial interfaces
 */
// Serial (EDBG)
#define PIN_SERIAL_RX (22u)
#define PIN_SERIAL_TX (23u)
#define PIN_SERIAL2_RX (25u)
#define PIN_SERIAL2_TX (26u)

// SPI
#define PIN_SPI_MISO  (33u)
#define PIN_SPI_MOSI  (32u)
#define PIN_SPI_SCK   (31u)
#define PIN_SPI_SS    (30u)

static const uint8_t SS   = PIN_SPI_SS;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// Wire
#define PIN_WIRE_SDA        (27u)
#define PIN_WIRE_SCL        (28u)

#define PIN_WIRE_SDA1       (15u)
#define PIN_WIRE_SCL1       (16u)

// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial1
#define SERIAL_PORT_HARDWARE1       Serial2
#define SERIAL_PORT_HARDWARE1_OPEN  Serial2

#define SerialMKR1                  Serial1
#define SerialMKR2                  Serial2

// Mbed specific defines
#define SERIAL_HOWMANY		2
#define SERIAL1_TX			(digitalPinToPinName(PIN_SERIAL_TX))
#define SERIAL1_RX			(digitalPinToPinName(PIN_SERIAL_RX))
#define SERIAL2_TX			(digitalPinToPinName(PIN_SERIAL2_TX))
#define SERIAL2_RX			(digitalPinToPinName(PIN_SERIAL2_RX))

#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x805D
#define BOARD_NAME			  "Arduino Edge Control"

#define DFU_MAGIC_SERIAL_ONLY_RESET   0xb0

#define WIRE_HOWMANY		2

#define I2C_SDA				(digitalPinToPinName(PIN_WIRE_SDA))
#define I2C_SCL				(digitalPinToPinName(PIN_WIRE_SCL))
#define I2C_SDA1			(digitalPinToPinName(PIN_WIRE_SDA1))
#define I2C_SCL1			(digitalPinToPinName(PIN_WIRE_SCL1))

#define QSPIDCS             (digitalPinToPinName(PIN_QSPIDCS))
#define GPIOCLK             (digitalPinToPinName(PIN_GPIOCLK))
#define QSPID0              (digitalPinToPinName(PIN_QSPID0))
#define QSPID1              (digitalPinToPinName(PIN_QSPID1))
#define QSPID2              (digitalPinToPinName(PIN_QSPID2))
#define QSPID3              (digitalPinToPinName(PIN_QSPID3))

#define SPI_HOWMANY			1

#define SPI_MISO			(digitalPinToPinName(PIN_SPI_MISO))
#define SPI_MOSI			(digitalPinToPinName(PIN_SPI_MOSI))
#define SPI_SCK				(digitalPinToPinName(PIN_SPI_SCK))
#define SPI_SS				(digitalPinToPinName(PIN_SPI_SS))

#define SD_MISO			(digitalPinToPinName(PIN_SD_MISO))
#define SD_MOSI			(digitalPinToPinName(PIN_SD_MOSI))
#define SD_CLK			(digitalPinToPinName(PIN_SD_CLK))
#define SD_CS				(digitalPinToPinName(PIN_SD_CS))

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

#endif //__PINS_ARDUINO__
