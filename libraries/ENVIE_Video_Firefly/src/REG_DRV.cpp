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

mbed::I2C i2c(I2C_SDA , I2C_SCL); 

char WriteReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char RegVal) {

    char cmd[2];
    cmd[0] = RegAddr;
    cmd[1] = RegVal;
    return i2c.write(DevAddr, cmd, 2);
}

char WriteBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf) {

    char cmd[n +1];
    cmd[0] = RegAddr;
    memcpy(&cmd[1], pBuf, n);
    return i2c.write(DevAddr, cmd, n + 1);
}

char ReadBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf) {

    patch_for_read_reg(DevAddr);

    i2c.write(DevAddr, (char*)&RegAddr, 1);
    return i2c.read(DevAddr, (char*)pBuf, n);
}

char ReadReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char *pData) {

    patch_for_read_reg(DevAddr);

    *pData = 0xFF;

    i2c.write(DevAddr, (char*)&RegAddr, 1);
    i2c.read(DevAddr, (char*)pData, 1);
    return 0;
}

static unsigned char  last_read_DevAddr = 0xff;

void patch_for_read_reg(unsigned char DevAddr)
{
    unsigned char RegAddr;
    int ret = 0;

    if (DevAddr != last_read_DevAddr) {
        switch (DevAddr) {
        case  0x54:
        case  0x72:
        default:
            RegAddr = 0x00;
            break;

        case  0x58:
            RegAddr = 0x00;
            break;

        case  0x70:
            RegAddr = 0xD1;
            break;

        case  0x7A:
            RegAddr = 0x60;
            break;

        case  0x7E:
            RegAddr = 0x39;
            break;

        case  0x84:
            RegAddr = 0x7F;
            break;
        }
    }
    WriteReg(DevAddr, RegAddr, 0x00);
    last_read_DevAddr = DevAddr;
}
