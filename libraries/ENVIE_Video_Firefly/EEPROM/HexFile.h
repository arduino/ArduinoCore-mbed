/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V0.1

Filename : 

Project  : ANX7625 

Created  : 20 Sept. 2016

Devices  : ANX7625

Toolchain: Keil
 
Description:HEX file access function prototypes

Revision History:

******************************************************************************/
#ifndef __HEXFILE_H__
#define __HEXFILE_H__

/* record types; only I8HEX files are supported, so only record types 00 and 01 are used */
#define  HEX_RECORD_TYPE_DATA   0
#define  HEX_RECORD_TYPE_EOF    1

char GetLineData(unsigned char *pLine, unsigned char data *pByteCount, unsigned int data *pAddress, unsigned char data *pRecordType, unsigned char *pData);
void SetLineData(unsigned char *pLine, unsigned char ByteCount, unsigned int Address, unsigned char RecordType, unsigned char *pData);

#endif  /* __HEXFILE_H__ */

