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
#include  "Flash.h"
#include "MI2_REG.h"
#include " REG_DRV.h"
#include <string.h>
#include "private_interface.h"
#include "public_interface.h"

extern unsigned char xdata auto_pd_support_flag;
extern void AP_TX_process(void);
extern void edid_dump(void);
extern bit HPD_status;

unsigned char xdata prev_change_int = 0;
unsigned char xdata prev_change_status = 0;
unsigned char xdata prev_intr_alert_1 = 0;
unsigned char xdata force_pd_rev20 = 0; // MIS2-215

unsigned char i2c_ReadReg(unsigned char dev,unsigned char offset)
{
	unsigned char temp;
	ReadReg(dev, offset, &temp);
	return temp;
}

void alert(void)
{
	unsigned char change_int, change_status, intr_alert_1;
	
	if (ALERT_N == 0) {
		ReadReg(RX_P0, CHANGE_INT, &change_int);
		ReadReg(RX_P0, SYSTEM_STSTUS, &change_status);
		ReadReg(TCPC_INTERFACE, INTR_ALERT_1, &intr_alert_1);
		//clear
		WriteReg(RX_P0, CHANGE_INT, 0x00);
		WriteReg(TCPC_INTERFACE, INTR_ALERT_1, 0xFF);
		
		if(prev_change_int != change_int || prev_change_status != change_status ||
			prev_intr_alert_1 != intr_alert_1)
			{
			TRACE3("I1,I2,S1 = %x,%x, %x\n", 
				(int)change_int, (int)intr_alert_1, (int)change_status);
			prev_change_int = change_int;
			prev_change_status = change_status;
			prev_intr_alert_1 = intr_alert_1;
			}
		
		if (intr_alert_1 & INTR_SOFTWARE_INT) {
		/*Received software interrupt*/
			WriteReg(TCPC_INTERFACE, INTR_ALERT_1, INTR_SOFTWARE_INT);
			if (change_int & VBUS_CHANGE) {
				if (change_status & VBUS_CHANGE)
				{
					ENABLE_5V_VBUS_OUT();
					TRACE("VBUS 5V OUT.\n");
				}
				else
				{
					DISABLE_5V_VBUS_OUT();
					TRACE("VBUS change to input.\n");
				}
			}
			if (change_int & CC_STATUS_CHANGE) {
				
					//TRACE1("NEW_CC_STATUS = %bx\n",i2c_ReadReg(RX_P0,NEW_CC_STATUS));
			}
            if(change_int & PR_CONSUMER_GOT_POWER) {
                //clear_got_power();
                if(auto_pd_support_flag) {
                    TRACE1("Require voltage is: %u mV \n", (unsigned int)(((unsigned int)i2c_ReadReg(RX_P0, RDO_MAX_VOLTAGE)) * 100));
                    TRACE1("Require power is: %u mW \n", (unsigned int)(((unsigned int)i2c_ReadReg(RX_P0, RDO_MAX_POWER)) * 500));
                }
            }

		}
		if (intr_alert_1 & INTR_RECEIVED_MSG) {
		/*Received interface message*/
			WriteReg(TCPC_INTERFACE, INTR_ALERT_1, INTR_RECEIVED_MSG);
			//if(!auto_pd_support_flag)
			{ 
				 interface_recvd_msg();
			}
		}
		/*HDP changed */
		 if(change_int & DP_HPD_CHANGE) {
	               if(i2c_ReadReg(RX_P0, SYSTEM_STSTUS) & HPD_STATUS)  { //HPD high
	               	TRACE("HPD is high\n");
				TRACE("do tx process\n");
				HPD_status = 1;
				edid_dump();
				AP_TX_process();
			}
		      else {
				TRACE("HPD is low\n");
				HPD_status = 0;
		      }
            }
		 
		
	
	}

}


unsigned char sw1_rdo_index;
extern unsigned char xdata InterfaceSendBuf[INTERFACE_SEND_BUF_SIZE];
extern unsigned char xdata InterfaceRecvBuf[INTERFACE_RECV_BUF_SIZE];
extern bit recv_msg_success;

void interface_recvd_msg_handler(void)
{
	if(recv_msg_success)
	{
		dispatch_rcvd_pd_msg(InterfaceRecvBuf[1], &InterfaceRecvBuf[2], InterfaceRecvBuf[0]-1);
		recv_msg_success = 0;
	}
}

unsigned char interface_send_msg_test(void)
{
unsigned char buf[]={0x12,0x34,0x56,0x78};
InterfaceSendBuf[0] = sizeof(buf) + 1; // + cmd
InterfaceSendBuf[1] = 0xFC;
memcpy(InterfaceSendBuf + 2, buf, sizeof(buf));

return interface_send_msg();
}

void auto_pd_init(void)
{
#define InterfaceSendBuf_Addr 0xC0
   unsigned char val;

    /*Enable automatic PD function*/
	  WriteReg(RX_P0, FIRMWARE_CTRL, val | auto_pd_en);
       /*   MIS2-308 slimport mode enable */
#if 0
       WriteReg(RX_P0, FIRMWARE_CTRL, val | auto_pd_en | slimport_mode_mode);
	WriteReg(RX_P0, AP_AV_STATUS, AP_DISABLE_PD);  //disable PD function
#endif
	WriteReg(RX_P0, MAX_VOLTAGE, 0xD2); /*21V*/		
   	WriteReg(RX_P0, MAX_POWER, 0x78); /*60W*/
	    
    /*Minimum Power in 500mW units*/
    /*1W*/
    WriteReg(RX_P0, MIN_POWER, 0x02);
	
	#if 0
    /*Initialized Source Capability*/
    /*Type = source_capbility (0)*/
    WriteReg(RX_P0 ,InterfaceSendBuf_Addr+1, 0x00);
    /*
     * ANX7428/18 defalut source cpability: 5V@0.9A
     * 0x5A,0x90,0x01,0x26
     * Sample: EC/AP change defalut source capability to 5V@1.5A
     * 0x96,0x90,0x01,0x26
     * NOTE: caculate CheckSum.
     */
    WriteReg(RX_P0,InterfaceSendBuf_Addr+2, 0x96);
    WriteReg(RX_P0,InterfaceSendBuf_Addr+3, 0x90);
    WriteReg(RX_P0,InterfaceSendBuf_Addr+4, 0x01);
    WriteReg(RX_P0,InterfaceSendBuf_Addr+5, 0x26);
    /*Checksum = D0 + D1 + .. = 0*/
    WriteReg(RX_P0,InterfaceSendBuf_Addr+6, 0xAE);

    /*Length = 4 + 1*/
    WriteReg(RX_P0,InterfaceSendBuf_Addr, 0x05);
	
	#endif
	if(force_pd_rev20)
		WriteReg(RX_P0,0xDF, 0x01);

#if 1 // trySrc_trySnk_setting
	ReadReg(RX_P0, FIRMWARE_CTRL, &val);
	val = val & (~(trysrc_en | trysnk_en));

	if((!SW5_2)&&(SW5_3)) {
		WriteReg(RX_P0, FIRMWARE_CTRL, val | trysrc_en);
		TRACE("Prefer Try.Src\n");
	} else if((!SW5_3) &&(SW5_2)) {
		WriteReg(RX_P0, FIRMWARE_CTRL, val | trysnk_en);
		TRACE("Prefer Try.Snk\n");
	} else 
		WriteReg(RX_P0, FIRMWARE_CTRL, val);
	
#endif
	
ReadReg(RX_P0, FIRMWARE_CTRL, &val);
TRACE1("1 Try.* = %x\n", (int)val);
	
}


