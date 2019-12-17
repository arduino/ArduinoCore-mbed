/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V0.1

Filename :

Project  : ANX7625

Created  : 20 Sept. 2016

Devices  : ANX7625

Toolchain: Keil

Description:

Revision History:

******************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "Arduino.h"

extern mbed::DigitalOut RESET_N;
extern mbed::DigitalOut POWER_EN;

extern mbed::DigitalIn ALERT_N;
extern mbed::DigitalIn CABLE_DET;

#define DEBUG

typedef uint8_t bit;

#define _BIT0	(1 << 0)
#define _BIT1	(1 << 1)
#define _BIT2	(1 << 2)
#define _BIT3	(1 << 3)
#define _BIT4	(1 << 4)
#define _BIT5	(1 << 5)
#define _BIT6	(1 << 6)
#define _BIT7	(1 << 7)

#define SW5_1	0			// no idea what it means
#define SW5_2	0			// no idea what it means
#define SW5_3	0			// no idea what it means
#define SW5_4	0			// no idea what it means

#define xdata 
#define idata 

#define delay_ms 	delay
#define mdelay	 	delay

// From the other driver
#define CMD_LINE_SIZE 44
#define CMD_NAME_SIZE 44
#define MAX_BYTE_COUNT_PER_RECORD_FLASH    16
#define SERIAL_RECV_BUF_SIZE	64
#define SERIAL_SEND_BUF_SIZE	64

#define IS_ALERT()  (!ALERT_N)

#define XTAL_FRQ  27000000UL  // MI-2 clock frequency in Hz: 27 MHz
#define XTAL_FRQ_MHz  ((unsigned char)(XTAL_FRQ/1000000UL))      // MI-2 clock frequency in MHz

#define POWERCYCLE_DELAY      250  // in miliseconds
#define RESET_RELEASE_DELAY    50  // in miliseconds

/* note: On POWER_EN and RESET_N pins, there's an inverter on the EVB. */
#define ANX7625_POWER_ON()      POWER_EN = 0
#define ANX7625_POWER_DOWN()    POWER_EN = 1
#define ANX7625_RESET()          RESET_N = 1
#define ANX7625_RESET_RELEASE()  RESET_N = 0

#define ENABLE_5V_VBUS_OUT()
#define DISABLE_5V_VBUS_OUT()

#define ENABLE_5to20V_VBUS_IN()
#define DISABLE_5to20V_VBUS_IN()

/* ========================================================================== */
// I2C addresses of Analogix's chip modules, which is chip dependent
// TODO: update this when register spec is ready
#define TCPC_I2C_ADDR    (0x58)
#define USBC_I2C_ADDR    (0x50)
// END of I2C addresses of Analogix's chip modules, which is chip dependent
/* ========================================================================== */

#endif  /* __CONFIG_H__ */

