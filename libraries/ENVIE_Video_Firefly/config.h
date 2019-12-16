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

PinName RESET_N = PJ_3;
PinName POWER_EN = PK_2;

PinName ALERT_N   = PK_4;
PinName CABLE_DET = PK_3;

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

#define ENABLE_5V_VBUS_OUT()      AP_VBUS_CTRL = 0
#define DISABLE_5V_VBUS_OUT()     AP_VBUS_CTRL = 1

#define ENABLE_5to20V_VBUS_IN()   AP_VBUS_CTRL = 1
#define DISABLE_5to20V_VBUS_IN()  AP_VBUS_CTRL = 0


/* ========================================================================== */
// I2C addresses of Analogix's chip modules, which is chip dependent
// TODO: update this when register spec is ready
#define TCPC_I2C_ADDR    (0x58)
#define USBC_I2C_ADDR    (0x50)
// END of I2C addresses of Analogix's chip modules, which is chip dependent
/* ========================================================================== */

#endif  /* __CONFIG_H__ */

