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

#include <Wire.h>

void patch_for_read_reg(unsigned char DevAddr);

char ReadReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char *pData)
{
    patch_for_read_reg(DevAddr);

    Wire.beginTransmission(DevAddr);
    Wire.write(RegAddr);
    Wire.endTransmission();
    Wire.requestFrom(DevAddr, 1);

    if(Wire.available()) {
        *pData = Wire.read();
        return 0;
    } else {
        *pData = -1;
        return -1;
    }
}

char ReadWordReg(unsigned char DevAddr, unsigned char RegAddr, unsigned int *pData)
{
    patch_for_read_reg(DevAddr);

    Wire.beginTransmission(DevAddr);
    Wire.write(RegAddr);
    Wire.endTransmission();
    Wire.requestFrom(DevAddr, 2);

    uint8_t* data = (uint8_t*)pData;

    if(Wire.available() >= 2) {
        data[0] = Wire.read();
        data[1] = Wire.read();
        return 0;
    } else {
        *pData = -1;
        return -1;
    }
}

char ReadBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf)
{
    patch_for_read_reg(DevAddr);
    
    Wire.beginTransmission(DevAddr);
    Wire.write(RegAddr);
    Wire.endTransmission();
    Wire.requestFrom(DevAddr, n);

    if(Wire.available() >= n) {
        for(unsigned char i = 0; i < n; i++) {
            pBuf[n] = Wire.read();
        }
        return 0;
    }
    return -1;
}

char WriteReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char RegVal)
{
    Wire.beginTransmission(DevAddr);
    Wire.write(RegAddr);
    Wire.write(RegVal);
    Wire.endTransmission();
    return 0;
}

char WriteWordReg(unsigned char DevAddr, unsigned char RegAddr, unsigned int RegVal)
{
    Wire.beginTransmission(DevAddr);
    Wire.write(RegAddr);
    Wire.write(RegVal & 0xFF);
    Wire.write((RegVal >> 8) & 0xFF);
    Wire.endTransmission();
    return 0;
}

char WriteBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf)
{
    Wire.beginTransmission(DevAddr);
    Wire.write(RegAddr);
    for(unsigned char i = 0; i < n; i++) {
        Wire.write(pBuf[i]);
    }
    Wire.endTransmission();
    return 0;
}

void patch_for_read_reg(unsigned char DevAddr)
{
	if(DevAddr == 0x70) WriteReg(0x70, 0xd1, 0x00);
	else if(DevAddr == 0x72)	WriteReg(0x72, 0x00, 0x00);
	else if(DevAddr == 0x7a)	WriteReg(0x7a, 0x60, 0x00);
	else if(DevAddr == 0x7e)	WriteReg(0x7E, 0x39, 0x00);
	else if(DevAddr == 0x54)	WriteReg(0x54, 0x00, 0x00);
	else if(DevAddr == 0x58)	WriteReg(0x58, 0x00, 0x00);
	else if(DevAddr == 0x84)	WriteReg(0x84, 0x7f, 0x00);
}
