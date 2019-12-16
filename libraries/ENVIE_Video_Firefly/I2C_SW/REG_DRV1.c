/* file: REG_DRV1.c
 * description: register read/write functions via software-emulated I2C
 * note: Silicon Labs EFM8UB20F64G MCU has 2 I2C master modules,
 *       why do we use software-emulated I2C? Refer to JIRA issue
 *       MIS2-79
*/

#include "config.h"
#include "I2C_SW.h"

////////////////////////////////////////////////////////////////////////////////
// MAX6959/LTC2943 I2C driver
char ReadReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char data *pData)
{
    I2C1_Start();
    if(I2C1_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C1_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            I2C1_Restart();
            if(I2C1_WriteByte(DevAddr | I2C_READ) == I2C_ACK)
            {// ACK
                *pData = I2C1_ReadByte(I2C_NACK);
                I2C1_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C1_Stop();
    *pData = -1;
    return -1;
}

char ReadWordReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned int data *pData)
{
    I2C1_Start();
    if(I2C1_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C1_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            I2C1_Restart();
            if(I2C1_WriteByte(DevAddr | I2C_READ) == I2C_ACK)
            {// ACK
                DBYTE0(*pData) = I2C1_ReadByte(I2C_ACK); // high byte
                DBYTE1(*pData) = I2C1_ReadByte(I2C_NACK); // low byte
                I2C1_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C1_Stop();
    *pData = -1;
    return -1;
}

char ReadBlockReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char idata *pBuf)
{
    I2C1_Start();
    if(I2C1_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C1_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            I2C1_Restart();
            if(I2C1_WriteByte(DevAddr | I2C_READ) == I2C_ACK)
            {// ACK
                while(n != 1)
                {
                    *pBuf = I2C1_ReadByte(I2C_ACK);
                    pBuf++;
                    n--;
                }
                *pBuf = I2C1_ReadByte(I2C_NACK);
                I2C1_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C1_Stop();
    do
    {
        *pBuf = -1;
        pBuf++;
    }while(--n);
    return -1;
}

char WriteReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char RegVal)
{
    I2C1_Start();
    if(I2C1_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C1_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            if(I2C1_WriteByte(RegVal) == I2C_ACK)
            {// ACK
                I2C1_Stop();
                return 0;
            }
        }
    }
    // NAK
    I2C1_Stop();
    return -1;
}

char WriteBlockReg1(unsigned char DevAddr, unsigned char RegAddr, unsigned char n, unsigned char idata *pBuf)
{
    I2C1_Start();
    if(I2C1_WriteByte(DevAddr | I2C_WRITE) == I2C_ACK)
    {// ACK
        if(I2C1_WriteByte(RegAddr) == I2C_ACK)
        {// ACK
            do
            {
                if(I2C1_WriteByte(*pBuf) == I2C_NACK)
                {// NAK
                    break;
                }
                pBuf++;
            }while(--n);
            I2C1_Stop();
            return 0;
        }
    }
    // NAK
    I2C1_Stop();
    return -1;
}
