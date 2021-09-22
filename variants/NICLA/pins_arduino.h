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
#define NUM_DIGITAL_PINS     (10u)
#define NUM_ANALOG_INPUTS    (2u)
#define NUM_ANALOG_OUTPUTS   (0u)

extern PinName digitalPinToPinName(pin_size_t P);

/*
 * Serial interfaces
 */
// External SPI
#define PIN_SPI_MISO  (7u)
#define PIN_SPI_MOSI  (8u)
#define PIN_SPI_SCK   (9u)
#define PIN_SPI_SS    (6u)

// Wire
#define PIN_WIRE_SDA        (p22)
#define PIN_WIRE_SCL        (p23)

#define PIN_WIRE_SDA1       (p15)
#define PIN_WIRE_SCL1       (p16)

// Analog pins
// -----------
#define PIN_A0 (10u)
#define PIN_A1 (11u)

static const uint8_t A0  = PIN_A0;   //ADC1
static const uint8_t A1  = PIN_A1;   //ADC2

static const uint8_t SS   = PIN_SPI_SS;   // SPI Slave SS not used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// ESLOV
#define PIN_ESLOV_INT	(p19)

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


// Mbed specific defines
#define SERIAL_HOWMANY		1
#define SERIAL1_TX			(TX_PIN_NUMBER)
#define SERIAL1_RX			(RX_PIN_NUMBER)

//#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x8060
#define BOARD_NAME			"Nicla Sense ME"

#define DFU_MAGIC_SERIAL_ONLY_RESET   0xb0

#define WIRE_HOWMANY		2

#define I2C_SDA				(p22)
#define I2C_SCL				(p23)
#define I2C_SDA1			(p15)
#define I2C_SCL1			(p16)

#define RESET_N				(BUTTON1)

#define SPI_HOWMANY			2

#define SPI_MISO			(p28)
#define SPI_MOSI			(p27)
#define SPI_SCK				(p11)

#define SPI_MISO1			(p5)
#define SPI_MOSI1			(p4)
#define SPI_SCK1			(p3)

#define digitalPinToPort(P)		(digitalPinToPinName(P)/32)

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();

#if __has_include("Nicla_System.h")
#  define NICLA_SYSTEM_ATTRIBUTE
#else
#  define NICLA_SYSTEM_ATTRIBUTE __attribute__ ((error("Please include Nicla_System.h to use this pin")))
#endif

class I2CLed {
public:
  I2CLed(int _pin) : pin(_pin) {};
  int get() {
    return pin;
  };
  bool operator== (I2CLed const & other) const {
    return pin == other.pin;
  }
  //operator int() = delete;
  __attribute__ ((error("Change me to a #define"))) operator int();
private:
  int pin;
};

extern I2CLed  LEDR;
extern I2CLed  LEDG;
extern I2CLed  LEDB;
extern I2CLed  LED_BUILTIN;

void      NICLA_SYSTEM_ATTRIBUTE pinMode     (I2CLed pin, PinMode mode);
PinStatus NICLA_SYSTEM_ATTRIBUTE digitalRead (I2CLed pin);
void      NICLA_SYSTEM_ATTRIBUTE digitalWrite(I2CLed pin, PinStatus value);

#endif //__PINS_ARDUINO__
