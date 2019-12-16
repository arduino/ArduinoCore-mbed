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
#include "debug.h"

/**
  * @brief circular buffer
  * head pointer: point to next free buffer or alternately data to be written to the buffer
  * tail pointer: point to the end of valid data or alternately data to be read from the buffer
  * empty condition: both pointers point to the same element
  * full condition: tail - head = 1 or tail + size - head = 1
  */
extern bit g_bSerialBusy;

unsigned char xdata g_SerialRecvBuf[SERIAL_RECV_BUF_SIZE];
unsigned char xdata *g_pSerialRecvHead;
unsigned char xdata *g_pSerialRecvTail;
unsigned char xdata g_SerialSendBuf[SERIAL_SEND_BUF_SIZE];
unsigned char xdata *g_pSerialSendHead;
unsigned char xdata *g_pSerialSendTail;

bit g_bPutcharEnable;

unsigned char xdata g_CmdLineBuf[CMD_LINE_SIZE];

void SerialInit(void)
{
    g_pSerialRecvHead = &g_SerialRecvBuf[0];
    g_pSerialRecvTail = &g_SerialRecvBuf[0];
    g_pSerialSendHead = &g_SerialSendBuf[0];
    g_pSerialSendTail = &g_SerialSendBuf[0];
    g_bPutcharEnable = 1; // enable output
}

unsigned char deque(void)
{
    return Serial.read();
}

#define IS_RECV_EMPTY()     (g_pSerialRecvTail == g_pSerialRecvHead)
#define IS_RECV_NOT_EMPTY() (g_pSerialRecvTail != g_pSerialRecvHead)
#define GETC()              (*g_pSerialRecvTail)

unsigned char SerialRecv(void)
{
    int i = 0;
    while (Serial.available()) {
        g_CmdLineBuf[i++] = Serial.read();
    }
    g_CmdLineBuf[i] = '\0';
    return i;
}

