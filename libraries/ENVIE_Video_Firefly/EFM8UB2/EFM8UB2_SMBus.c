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

void I2C_Start(void)
{
    SMB0CN0_STA = 1; // Start transfer
    while(!SMB1CN0_SI)
    {
        // wait for START has been transmitted.
    }
    SMB0CN0_STA = 0; // Manually clear START bit
}


void I2C_Restart(void)
{
    SMB0CN0_STA = 1; // Start transfer
    SMB1CN0_SI = 0; // Clear interrupt flag
    while(!SMB1CN0_SI)
    {
        // wait for START has been transmitted.
    }
    SMB0CN0_STA = 0; // Manually clear START bit
}


unsigned char I2C_ReadByte(bit ack)
{
    SMB1CN0_SI = 0; // Clear interrupt flag
    while(!SMB1CN0_SI)
    {
        // wait for data has been received.
    }
    SMB1CN0_ACK = ~ack; // Send ACK/NACK
    return SMB0DAT; // Return received byte
}


bit I2C_WriteByte(unsigned char byte)
{
    SMB0DAT = byte;
    SMB1CN0_SI = 0; // Clear interrupt flag
    while(!SMB1CN0_SI)
    {
        // wait for data byte has been sent and acknowledged
    }
    return ~SMB1CN0_ACK;
}

void I2C_Stop(void)
{
    SMB0CN0_STO = 1; // Set STO to terminate transfer
    SMB1CN0_SI = 0; // Clear interrupt flag
    while(SMB0CN0_STO)
    {
        // wait for STOP has been transmitted.
    }
}

