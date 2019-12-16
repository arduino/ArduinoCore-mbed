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
#include "debug/cmd.h"
#include "debug/serial.h"
#include "MI2.h"
#include "ENVIE_Video_Firefly.h"

#define FW_MAJOR_VERSION 1 
#define FW_MINOR_VERSION 1

mbed::DigitalOut RESET_N(PJ_3);
mbed::DigitalOut POWER_EN(PK_2);

mbed::DigitalIn ALERT_N(PK_4);
mbed::DigitalIn CABLE_DET(PK_3);

unsigned char is_Cable_Detected(void);
void ANX7625_DRP_Enable(void);
void PROC_Main(void);

extern bit g_poweron;
bit g_bDebug;

char WriteReg(unsigned char DevAddr, unsigned char RegAddr, unsigned char RegVal);

void anx7625_setup(void)
{
    // The default state of POWER_EN should be Low at system startup.
    // However, due to pull-up resistor in the level shifter circuit, POWER_EN
    // will be High at system startup, thus add this firmware patch. Refer to JIRA issue MIS-161.
    ANX7625_POWER_DOWN(); // power down chip before port initialization
    ANX7625_RESET(); // reset chip before port initialization

    TRACE2("\nMI-2 EVB FW v%bd.%bd\n", (unsigned char)FW_MAJOR_VERSION, (unsigned char)FW_MINOR_VERSION);
    TRACE2("Build at: %s, %s\n\n", __DATE__, __TIME__);

  delay_ms(500);

#ifdef ANX7625_PD_DP_Support
  if (is_Cable_Detected() == 0)
{
    delay_ms(10);
    /* initial power sequence, refer to section 4.1 in ANX7625 datasheet */
    ANX7625_POWER_ON();
    delay_ms(10);
    ANX7625_RESET_RELEASE();
    delay_ms(10);
    // TODO: configures the PD role if ANX7625 needs to support DRP or DFP mode
    ANX7625_DRP_Enable();
    TRACE("ANX7625 powers on and DRP enable .\n");
    

		WriteReg(0x58, 0xA1, 0xA0);
		WriteReg(0x58, 0xA1, 0xE0);
		delay_ms(1);
		ANX7625_RESET();
		delay_ms(10);
		ANX7625_POWER_DOWN();
		delay_ms(10);
		
	}
#else
    ANX7625_POWER_ON();
    delay_ms(10);
    ANX7625_RESET_RELEASE();
    delay_ms(10);

    ANX7625_RESET();
    delay_ms(10);
    ANX7625_POWER_DOWN();
    delay_ms(10);
#endif
    g_poweron = 0;
    g_bDebug = 0;
}

void anx7625_main()
{
  if (!g_bDebug)
  {
    PROC_Main();
  }
  cmd();
}
