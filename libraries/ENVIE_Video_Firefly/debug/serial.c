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
    unsigned char c;
    
    c = *g_pSerialRecvTail; // read receive data
    if(g_pSerialRecvTail == g_SerialRecvBuf + SERIAL_RECV_BUF_SIZE - 1)
    {
        g_pSerialRecvTail = &g_SerialRecvBuf[0];
    }
    else
    {
        g_pSerialRecvTail++;
    }
    return c;
}

#define IS_RECV_EMPTY()     (g_pSerialRecvTail == g_pSerialRecvHead)
#define IS_RECV_NOT_EMPTY() (g_pSerialRecvTail != g_pSerialRecvHead)
#define GETC()              (*g_pSerialRecvTail)

unsigned char SerialRecv(void)
{
    static unsigned char i;
    unsigned char c;
    unsigned char RetVal;
    
    while(IS_RECV_NOT_EMPTY())
    {
        g_bPutcharEnable = 1; // enable output
        c = deque();
        // control character
        if(c == '\b')
        {// backspace
            putchar(c); // echo
            putchar(' '); // space
            putchar(c);
            if(i != 0)
            {
                i--;
            }
            g_bPutcharEnable = 0; // disable output
        }
        else if(c == '\r')
        {// CR
            if(IS_RECV_EMPTY())
            {
                DELAY_US(US_PER_CHARACTER);
                if(IS_RECV_NOT_EMPTY())
                {
                    if(GETC() == '\n')
                    {
                        deque(); // discard \n
                    }
                }
            }
            else
            {
                if(GETC() == '\n')
                {
                    deque(); // discard \n
                }
            }
            putchar('\n'); // echo
            g_CmdLineBuf[i] = '\0'; // end of string
            RetVal = i;
            i = 0;
            return RetVal;    // return the string length
        }
        else if(c == '\n')
        {// LF
            if(IS_RECV_EMPTY())
            {
                DELAY_US(US_PER_CHARACTER);
                if(IS_RECV_NOT_EMPTY())
                {
                    if(GETC() == '\r')
                    {
                        deque(); // discard \r
                    }
                }
            }
            else
            {
                if(GETC() == '\r')
                {
                    deque(); // discard \r
                }
            }
            putchar('\n'); // echo
            g_CmdLineBuf[i] = '\0'; // end of string
            RetVal = i;
            i = 0;
            return RetVal;    // return the string length
        }
        else
        {// receive command data
            if(i < CMD_LINE_SIZE - 1)
            {// command line buffer not full
                if(c == '\\')
                {// command header
                    // wait for all data sent
                    while(g_bSerialBusy) ;
                }
                putchar(c); // echo
                g_CmdLineBuf[i] = c;
                i++;
            }
            else
            {// command line buffer full
                // discard the character
            }
            g_bPutcharEnable = 0; // disable output
        }
    }
    return 0;
}

