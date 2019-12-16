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
#ifndef CMDHANDLER_H
#define CMDHANDLER_H

enum
{
	VIDEO_DPI		= 0x00,
	VIDEO_DPI_DSC		= 0x01,
	VIDEO_DSI		= 0x02,
	VIDEO_DSI_DSC		= 0x03,
};

void CmdHandler(void);
unsigned char __i2c_read_byte(unsigned char dev,unsigned char offset);
unsigned char sp_tx_aux_dpcdwrite_bytes(unsigned char addrh, unsigned char addrm, 
	unsigned char addrl, unsigned char cCount, unsigned char *pBuf);
unsigned char sp_tx_aux_dpcdread_bytes(unsigned char addrh, unsigned char addrm,
	unsigned char addrl, unsigned char cCount, unsigned char *pBuf);
void edid_dump(void);
unsigned char __i2c_read_byte(unsigned char dev,unsigned char offset);
void reset_MI2(void);

#endif  // end of CMDHANDLER_H definition
