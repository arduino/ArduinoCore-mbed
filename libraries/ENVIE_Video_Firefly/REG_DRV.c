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

#include "config.h"
#include "EFM8UB2/EFM8UB2_SMBus.h"
void patch_for_read_reg(unsigned char DevAddr);
char ReadReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char data *pData)
{
		patch_for_read_reg(DevAddr);
    I2C_Start();
    if(I2C_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            I2C_Restart();
            if(I2C_WriteByte(DevAddr | I2C_READ) == I2C_ACK)
            {// ACK
                *pData = I2C_ReadByte(I2C_NACK);
                I2C_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C_Stop();
    *pData = -1;
    return -1;
}

char ReadWordReg(unsigned char DevAddr, unsigned char RegAddr, unsigned int data *pData)
{
		patch_for_read_reg(DevAddr);
    I2C_Start();
    if(I2C_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            I2C_Restart();
            if(I2C_WriteByte(DevAddr | I2C_READ) == I2C_ACK)
            {// ACK
                DBYTE1(*pData) = I2C_ReadByte(I2C_ACK); // low byte
                DBYTE0(*pData) = I2C_ReadByte(I2C_NACK); // high byte
                I2C_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C_Stop();
    *pData = -1;
    return -1;
}

char ReadBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf)
{
		patch_for_read_reg(DevAddr);
    I2C_Start();
    if(I2C_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            I2C_Restart();
            if(I2C_WriteByte(DevAddr | I2C_READ) == I2C_ACK)
            {// ACK
                while(n != 1)
                {
                    *pBuf = I2C_ReadByte(I2C_ACK);
                    pBuf++;
                    n--;
                }
                *pBuf = I2C_ReadByte(I2C_NACK);
                I2C_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C_Stop();
    do
    {
        *pBuf = -1;
        pBuf++;
    }while(--n);
    return -1;
}

char WriteReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char RegVal)
{
    I2C_Start();
    if(I2C_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            if(I2C_WriteByte(RegVal) == I2C_ACK)
            {// ACK
                I2C_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C_Stop();
    return -1;
}

char WriteWordReg(unsigned char DevAddr, unsigned char RegAddr, unsigned int RegVal)
{
    I2C_Start();
    if(I2C_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            if(I2C_WriteByte(LOBYTE(RegVal)) == I2C_ACK) // low byte
            {// ACK
                if(I2C_WriteByte(HIBYTE(RegVal)) == I2C_ACK) // high byte
                {
                    I2C_Stop();
                    return 0;
                }
            }
        }
    }
    // NAK
    I2C_Stop();
    return -1;
}

char WriteBlockReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char *pBuf)
{
    I2C_Start();
    if(I2C_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            do
            {
                if(I2C_WriteByte(*pBuf) == I2C_NACK)
                {// NAK
                    break;
                }
                pBuf++;
            }while(--n);
            I2C_Stop();
            return 0;
        }
    }
    // NAK
    I2C_Stop();
    return -1;
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
