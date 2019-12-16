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
#include "MCUInit.h"
#include "debug/serial.h"
#include "EFM8UB2/EFM8UB2_helper.h"
#include "MI2.h"
#include "ENVIE_Video_Firefly.h"

#define FW_MAJOR_VERSION 1 
#define FW_MINOR_VERSION 1

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

    /* ========== MCU init ========== */
    Monitor_Init();
    SYSCLKInit();   // <===== further study from here!
    SMBusRecover();
    PortInit();
    TimerInit();
    SMBusInit();
    PCAInit();
    UARTInit();
    SerialInit();

    IE_EA = 1;

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
		
    while (1)
    {
        if (!g_bDebug)
        {
            PROC_Main();
        }
        cmd();
    }
}

