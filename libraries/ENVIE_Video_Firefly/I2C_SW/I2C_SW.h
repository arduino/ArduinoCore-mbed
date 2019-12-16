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

#ifndef __I2C_SW_H__
#define __I2C_SW_H__

#include "I2C.h"

void I2C1_Start(void);
void I2C1_Restart(void);
unsigned char I2C1_ReadByte(bit ack);
bit I2C1_WriteByte(unsigned char byte);
void I2C1_Stop(void);

#endif  /* __I2C_SW_H__ */

