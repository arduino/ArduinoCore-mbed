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

extern unsigned char xdata g_SerialSendBuf[SERIAL_SEND_BUF_SIZE];
extern unsigned char xdata *g_pSerialSendHead;
extern const unsigned char xdata * const g_pSerialSendTail;
extern const bit g_bSerialBusy;
extern const bit g_bPutcharEnable;

void putc(char c)
{
    unsigned char xdata *pNext; /**< point to the next free space */
    
    *g_pSerialSendHead = c;
    if(g_pSerialSendHead == g_SerialSendBuf + SERIAL_SEND_BUF_SIZE - 1)
    {
        pNext = &g_SerialSendBuf[0];
    }
    else
    {
        pNext = g_pSerialSendHead + 1;
    }
    while(pNext == g_pSerialSendTail)
    {
        // buffer full, wait for at least one byte space
    }
    g_pSerialSendHead = pNext;
    if(!g_bSerialBusy)
    {// serial tx is idle
        TRIGGER_TRANSMIT(); // start transmition loop by triggering the serial interrupt
    }
}

char putchar(char c)
{
    if(g_bPutcharEnable)
    {// putchar function enabled
        if(c == '\n')
        {
            putc('\r'); // insert \r
        }
        putc(c);
    }
    
    return c;
}

