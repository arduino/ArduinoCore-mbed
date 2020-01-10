#include "mbed_config.h"

#define SERIAL_HOWMANY		1
#define SERIAL1_TX			SERIAL_TX
#define SERIAL1_RX			SERIAL_RX

#define SERIAL_CDC			1
#define HAS_UNIQUE_ISERIAL_DESCRIPTOR
#define BOARD_VENDORID		0x2341
#define BOARD_PRODUCTID		0x025b
#define BOARD_NAME			"Envie M7"

#define DFU_MAGIC_SERIAL_ONLY_RESET   0xb0

uint8_t getUniqueSerialNumber(uint8_t* name);
void _ontouch1200bps_();
