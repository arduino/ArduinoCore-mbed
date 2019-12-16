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
#include "serial.h"
#include "CmdHandler.h"
#include <string.h>

extern unsigned char xdata g_CmdLineBuf[CMD_LINE_SIZE];
extern const bit g_bPutcharEnable;

void cmd(void)
{
    if(SerialRecv() != 0)
    {
        CmdHandler();
    }
}

