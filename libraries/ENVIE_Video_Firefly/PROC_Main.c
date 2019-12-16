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
#include "EFM8UB2_helper.h"
#include "MI2.h"
#include "Flash.h"
#include "MI2_REG.h"
#include "REG_DRV.h"
#include "REG_DRV1.h"
#include "private_interface.h"
#ifdef USE_PD30
#include "pd_ext_message.h"
#endif 
#ifdef HAVE_DRM
#include "drm.h"
#endif 

#define DFP_UFP_CHECK 1
#define VBUS_MONITOR 0
//#define XTAL_26M
#define LED_DISPLAY 0 // 1: display voltage/current

bit g_poweron = 0;
bit HPD_status = 0;

void alert();
void interface_recvd_msg_handler(void);
void auto_pd_init(void);
 void AP_TX_process(void);
 
/* cable detection */
#define CABLE_CHECK_TIMES 0
#if CABLE_CHECK_TIMES
unsigned char xdata prev_cable_det = 0;
unsigned char xdata cable_det_count = 0;
unsigned char is_Cable_Detected(void)
{
    if(cable_det_count >= CABLE_CHECK_TIMES) {
		prev_cable_det = 1;
        //return 1;
    }
	else if(cable_det_count == 0){
		prev_cable_det = 0;
        //return 0;
	}
		
    return prev_cable_det;
}
#else
unsigned char is_Cable_Detected(void)
{
#ifdef ANX7625_PD_DP_Support
    if(CABLE_DET) {
        delay_ms(2);
        return CABLE_DET;
    }
    return 0;
#else
    return 1;
#endif
}
#endif

/*configure DPR toggle*/
void ANX7625_DRP_Enable(void)
{
    WriteReg(TCPC_INTERFACE, TCPC_ROLE_CONTROL, 0x45);
    WriteReg(TCPC_INTERFACE, TCPC_COMMAND, 0x99);
}
/* basic configurations of ANX7625 */
void ANX7625_config(void)
{
    flash_basic_config();


}

unsigned char wait_ocm_ready(void)
{
   unsigned int count = 3200;
   unsigned char val;
   
   do{
   	if(!count)
   		{
   		TRACE("ocm not ready!\n");
		break;
   		}
   	ReadReg(0x7E, 0x05, &val);
   	} while(!(val & _BIT7));
   return((val & _BIT7) ? 1:0);
}

bit Is_Auto_PD_Supported(void)
{
	return !SW5_4;
}

#if LED_DISPLAY
code unsigned char hex_display[16]= {0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b,0x77,0x1f,0x4e,0x3d,0x4f,0x47};
code unsigned char prefix_display[3]= {0x00,0x01,0x3e}; // null,-(minus), u(V)
code unsigned char dot_display[4]= {0x04,0x08,0x01,0x02};

// ctrl
//   bit7: LED SEG turn off
//   bit6: hex base
//   bit5~4: prefix turn on
//   bit3~0: dot turn on
void LED_Display_SEG(unsigned int value, unsigned char ctrl)
{
    unsigned char segment;
	unsigned char base;

    if(ctrl&_BIT7)
	{
		// LED SEG off
		WriteReg1(MAX6958_ADDR, 0x01, 0x0); //HEX mode
		WriteReg1(MAX6958_ADDR, 0x04, 0x1); //Enable
		WriteReg1(MAX6958_ADDR, 0x20, 0); //led0
		WriteReg1(MAX6958_ADDR, 0x21, 0); //led1
		WriteReg1(MAX6958_ADDR, 0x22, 0); //led2
		WriteReg1(MAX6958_ADDR, 0x23, 0); //led3
		WriteReg1(MAX6958_ADDR, 0x24, 0); //segment
	}
	else
	{
		if(ctrl&_BIT6)
			base = 16;
		else
			base = 10;
	    
		segment = (ctrl&_BIT0 ? _BIT1:0)|(ctrl&_BIT1 ? _BIT0:0)|(ctrl&_BIT2 ? _BIT3:0)|(ctrl&_BIT3 ? _BIT2:0);
		WriteReg1(MAX6958_ADDR, 0x01, 0x0); //HEX mode
		WriteReg1(MAX6958_ADDR, 0x04, 0x1); //Enable
		WriteReg1(MAX6958_ADDR, 0x20, hex_display[value%base]); //led0
		value /= base;
		WriteReg1(MAX6958_ADDR, 0x21, hex_display[value%base]); //led1
		value /= base;
		WriteReg1(MAX6958_ADDR, 0x22, hex_display[value%base]); //led2
		if(ctrl&_BIT5 || ctrl&_BIT4)
		{
			WriteReg1(MAX6958_ADDR, 0x23, prefix_display[((ctrl&(_BIT5|_BIT4))>>4)-1]); //led3
		}
		else
		{
			value /= base;
			WriteReg1(MAX6958_ADDR, 0x23, hex_display[value%base]); //led3
		}
		WriteReg1(MAX6958_ADDR, 0x24, segment); //segment
	}
}

char Get_Voltage_Current(unsigned int *pVoltage, unsigned int *pCurrent)
{
	static bit init = 0;
	unsigned int voltage;
	unsigned int current;
	unsigned char minus_flag;

	voltage = 0;
	current = 0;
	if(!init)
	{
		WriteReg1(LTC2943_ADDR, 0x01, 0xfC); //automatic conversion.
		delay_ms(100);
	}
	ReadWordReg1(LTC2943_ADDR, 0x08, &voltage); //voltage
	ReadWordReg1(LTC2943_ADDR, 0x0e, &current); //current

	/*can't read out voltage&current*/
	if (((voltage==0xffff)&&(current==0xffff))||((voltage==0x0)&&(current==0x0)))
	{
		return -1;
	}

	*pVoltage= voltage * 0.0036; //(236/65535=0.0036);

	if (current>32767) {
	minus_flag=0;
	current=(current-32767)*0.183; //(60/32767=0.00183A/10=0.183 (mA))
	}else{
	minus_flag=1;
	current=(32767-current)*0.183; // 60/32767/10;
	}
	*pCurrent= current;
	//TRACE2(" voltage: %d, current: %d  \n",voltage,current );
	return minus_flag;
}
#endif

#if DFP_UFP_CHECK
unsigned char xdata auto_pd_support_flag;//1:support auto pd. 0: not support
unsigned char xdata dfp_or_ufp; // 0:DFP, 1:UFP
unsigned char xdata dfp_or_ufp_printed; // 0:no, 1:yes
#endif
#if VBUS_MONITOR
unsigned int xdata prev_vbus_value = 0;
unsigned int xdata curr_vbus_value = 0;
unsigned char xdata vbus_value_printed = 0; // 0:no, 1:yes
#endif

#define ON_TIME_TASK_TIMEOUT 20 // 20 ms
#define ON_TIME_TASK_DR_COUNT (500/20)
unsigned int xdata on_time_task_timer = ON_TIME_TASK_TIMEOUT;
unsigned char xdata on_time_task_dr_count = ON_TIME_TASK_DR_COUNT;
#if LED_DISPLAY
#define ON_TIME_TASK_LD_COUNT (3000/20)
bit  LED_IS_ON = 0;
bit  LED_V_OR_C = 0;
unsigned char xdata on_time_task_ld_count = ON_TIME_TASK_LD_COUNT;
#endif

void on_time_task(void)
{
	if(!on_time_task_timer)
	{
#if CABLE_CHECK_TIMES
	    // cable detectc
		if(CABLE_DET)
		{
			if(cable_det_count < CABLE_CHECK_TIMES)
			 cable_det_count++;
		}
		else
	   {
			if(cable_det_count > 0)
			cable_det_count--;
	   }
#endif

		if(g_poweron) // power on tasks
		{
#if DFP_UFP_CHECK		
			// DFP/UFP check
			if(on_time_task_dr_count)
			{
				on_time_task_dr_count--;
			}
			else
			{
				if(dfp_or_ufp != ((i2c_ReadReg(TCPC_INTERFACE, ANALOG_CTRL_0) & DFP_OR_UFP) ? 1:0))
				{
				    dfp_or_ufp = !dfp_or_ufp;
					dfp_or_ufp_printed = 0;
				}
				if(!dfp_or_ufp_printed)
				{
					if(dfp_or_ufp) 
					{
						TRACE("UFP\n");
					} else {
						TRACE("DFP\n");
					}	
					dfp_or_ufp_printed = 1;
				}
				on_time_task_dr_count = ON_TIME_TASK_DR_COUNT;
			}
#endif			
#if VBUS_MONITOR
		curr_vbus_value = ((unsigned int)(i2c_ReadReg(TCPC_INTERFACE, 0x71)&0x03)<<8) + i2c_ReadReg(TCPC_INTERFACE, 0x70);
        curr_vbus_value *= 25;
		if(prev_vbus_value != curr_vbus_value)
		{
			vbus_value_printed = 0;
		}
		if(!vbus_value_printed)
		{
		    if(ABS(prev_vbus_value,curr_vbus_value) > 100)
		    {
			TRACE1("VBUS = %d mV\n",curr_vbus_value);
			prev_vbus_value = curr_vbus_value;
		    }
			vbus_value_printed = 1;
		}
#endif
#if LED_DISPLAY
		if(on_time_task_ld_count)
		{
			on_time_task_ld_count--;
		}
		else
		{
			unsigned int xdata val_V,val_C;
			char xdata minus;
			
			minus = Get_Voltage_Current(&val_V, &val_C);
			if(minus >= 0)
			{
			    if(LED_V_OR_C)
		    	{
					LED_Display_SEG(val_V,0x32);
		    	}
				else
				{
					if(!minus)
						LED_Display_SEG(val_C,0x14);
					else
						LED_Display_SEG(val_C,0x24);
				}
				if(!LED_IS_ON)
					LED_IS_ON = 1;
			}
			LED_V_OR_C = !LED_V_OR_C;
			on_time_task_ld_count = ON_TIME_TASK_LD_COUNT;
		}
#endif
		}
		else // power off tasks
		{
#if LED_DISPLAY
			if(LED_IS_ON)
			{
				LED_Display_SEG(0,0x80);
				LED_IS_ON = 0;
			}
#endif
		}

		on_time_task_timer = ON_TIME_TASK_TIMEOUT;
	}

}

void PROC_Main(void)
{
	unsigned char val;
	
#ifdef ANX7625_PD_DP_Support
	if(g_poweron == 0) {

        if (is_Cable_Detected())
        {
            TRACE("Cable attached!\n");
            delay_ms(10);
            ANX7625_POWER_ON();
            TRACE("ANX7625 powers on from STANDBY state.\n");
            delay_ms(10);
            ANX7625_RESET_RELEASE();
            TRACE("ANX7625 reset release\n");
            ANX7625_config();
			auto_pd_support_flag = (unsigned char)Is_Auto_PD_Supported();
				//	auto_pd_support_flag = 1;
			TRACE1("auto_pd = %bd\n",auto_pd_support_flag);
			wait_ocm_ready();
			#ifdef XTAL_26M
			WriteReg(RX_P0, XTAL_FRQ_SEL, 0x60);
			WriteReg(TCPC_INTERFACE, PD_1US_PERIOD, 0x1a);
			WriteReg(TCPC_INTERFACE, PD_TX_BIT_PERIOD, 0x56);
			WriteReg(TCPC_INTERFACE, PD_RX_HALF_MAX, 0x3e);
			#endif
			if(!auto_pd_support_flag)
			{
				interface_init();
				//send_initialized_setting(1);
				ReadReg(RX_P0, FIRMWARE_CTRL, &val);			
				WriteReg(RX_P0, FIRMWARE_CTRL, (val & (~auto_pd_en)));
			}
			else
			{
				TRACE("Auto PD mode\n");
				auto_pd_init();
				interface_init();
				send_initialized_setting(1);
				  
#if 0
				/* MIS2-308 slimport mode enable VBUS output */
				if ((i2c_ReadReg(RX_P0, FIRMWARE_CTRL) & slimport_mode_mode))
				{	
					ENABLE_5V_VBUS_OUT();
					TRACE("VBUS 5V OUT.\n");
					AP_TX_process();
				}
#endif
			}
#if DFP_UFP_CHECK
			dfp_or_ufp = ((i2c_ReadReg(TCPC_INTERFACE, ANALOG_CTRL_0) & DFP_OR_UFP) ? 1:0);
			dfp_or_ufp_printed = 0;
			on_time_task_dr_count = ON_TIME_TASK_DR_COUNT;
#endif
			g_poweron = 1;
#if CABLE_CHECK_TIMES
			prev_cable_det = 1;
#endif
#if LED_DISPLAY
			on_time_task_ld_count = ON_TIME_TASK_LD_COUNT;
#endif
						
        }
	} else {

		if(is_Cable_Detected() == 0) {
				 TRACE("Cable detached!\n");
					WriteReg(0x58, 0xA1, 0xA0);
					WriteReg(0x58, 0xA1, 0xE0);
				 ANX7625_RESET();
				 delay_ms(10);
				 ANX7625_POWER_DOWN();
				 delay_ms(10);
				 TRACE("ANX7625 powers down and enter STANDBY state.\n");
				 g_poweron = 0;
				 HPD_status = 0;
#if CABLE_CHECK_TIMES
				 prev_cable_det = 0;
#endif
				 DISABLE_5V_VBUS_OUT();
		} else {
				alert();
				interface_recvd_msg_handler();
#ifdef USE_PD30
				pd_ext_message_handling();
#endif
				if (HPD_status)
				{
#ifdef HAVE_DRM
					drm_MainProc();
#endif
				}
		}
	}
	
	on_time_task();
#else   //for standard DP or slimport mode
    if(g_poweron == 0) {
      delay_ms(10);
      ANX7625_POWER_ON();
      TRACE("ANX7625 powers on from STANDBY state.\n");
      delay_ms(10);
       ANX7625_RESET_RELEASE();
       TRACE("ANX7625 reset release\n");
       ANX7625_config();
	wait_ocm_ready();
	//disable PD function
	ReadReg(RX_P0, AP_AV_STATUS, &val);
	WriteReg(RX_P0, AP_AV_STATUS, (val | AP_DISABLE_PD));
	g_poweron = 1;
    }
	alert(); 
#endif

 	

}

