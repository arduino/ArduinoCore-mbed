/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V0.1

Filename : 

Project  : ANX7625 

Created  : 20 Sept. 2016

Devices  : ANX7625

Toolchain: Keil
 
Description:I2C related macros, common for hardware I2C and software-emulated I2C

Revision History:

******************************************************************************/
#ifndef __I2C_H__
#define __I2C_H__

// I2C R/W bit
#define I2C_WRITE   0x00 // I2C WRITE command
#define I2C_READ    0x01 // I2C READ command

// I2C acknowledge bit
#define I2C_ACK     0 // Acknowledge (ACK)
#define I2C_NACK    1 // Not Acknowledge (NACK)

#endif /* __I2C_H__ */

