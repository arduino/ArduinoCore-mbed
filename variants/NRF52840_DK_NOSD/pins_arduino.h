#include "mbed_config.h"

#ifndef __PINS_ARDUINO__
#define __PINS_ARDUINO__

#define PIN_PDM_DIN			25
#define PIN_PDM_CLK			26
#define PIN_PDM_PWR			20

#define SERIAL_HOWMANY		1
#define SERIAL1_TX			((PinName)35)
#define SERIAL1_RX			((PinName)42)

#define SERIAL_CDC			1
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x005a

#define DFU_MAGIC_SERIAL_ONLY_RESET   0x4e

#define I2C_SDA				31
#define I2C_SCL				2

#define SPI_MISO			40
#define SPI_MOSI			33
#define SPI_SCK				13

static inline void _ontouch1200bps_() {
	NRF_POWER->GPREGRET = DFU_MAGIC_SERIAL_ONLY_RESET;
	NVIC_SystemReset();
}

#define MBED_CONF_FLASHIAP_BLOCK_DEVICE_BASE_ADDRESS	0x10000
#define MBED_CONF_FLASHIAP_BLOCK_DEVICE_SIZE			0xF0000

#endif //__PINS_ARDUINO__