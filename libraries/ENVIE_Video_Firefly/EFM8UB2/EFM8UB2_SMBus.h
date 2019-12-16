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

#ifndef __EFM8UB2_SMBUS_H__
#define __EFM8UB2_SMBUS_H__

#include "I2C.h"

void I2C_Start(void);
void I2C_Restart(void);
unsigned char I2C_ReadByte(bit ack);
bit I2C_WriteByte(unsigned char byte);
void I2C_Stop(void);

#endif  /* __EFM8UB2_SMBUS_H__ */

