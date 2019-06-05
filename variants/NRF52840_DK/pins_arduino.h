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

static inline void _ontouch1200bps_() {
	NRF_POWER->GPREGRET = DFU_MAGIC_SERIAL_ONLY_RESET;
	NVIC_SystemReset();
}

#endif //__PINS_ARDUINO__