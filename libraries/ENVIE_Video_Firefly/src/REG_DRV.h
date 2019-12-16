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

#ifndef __REG_DRV_H__
#define __REG_DRV_H__

////////////////////////////////////////////////////////////////////////////////
// register operations
// read operations
char ReadReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char *pData);
char ReadWordReg(unsigned char DevAddr, unsigned char RegAddr, unsigned int *pData);
char ReadBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf);
// write operations
char WriteReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char RegVal);
char WriteWordReg(unsigned char DevAddr, unsigned char RegAddr, unsigned int RegVal);
char WriteBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf);

#define sp_write_reg_or(address, offset, mask) \
		WriteReg(address, offset, ((unsigned char)__i2c_read_byte(address, offset) | (mask)))
#define sp_write_reg_and(address, offset, mask) \
	WriteReg(address, offset, ((unsigned char)__i2c_read_byte(address, offset) & (mask)))

#define sp_write_reg_and_or(address, offset, and_mask, or_mask) \
	WriteReg(address, offset, (((unsigned char)__i2c_read_byte(address, offset)) & and_mask) | (or_mask))
#define sp_write_reg_or_and(address, offset, or_mask, and_mask) \
	WriteReg(address, offset, (((unsigned char)__i2c_read_byte(address, offset)) | or_mask) & (and_mask))
	
#endif  /* __REG_DRV_H__ */

