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

#ifndef __MCUINIT_H__
#define __MCUINIT_H__

void Monitor_Init(void);
void SYSCLKInit(void);
void TimerInit(void);
void PortInit(void);
void UARTInit(void);
void PCAInit(void);
void SMBusRecover(void);
void SMBusInit(void);
void INT0Init(void);

#endif  /* __MCUINIT_H__ */

