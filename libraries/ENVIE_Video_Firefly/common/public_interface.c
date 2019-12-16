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

#include "private_interface.h"
#include "public_interface.h"
#ifdef USE_PD30
#include "pd_ext_message.h"
#endif
#include <STRING.H>
#include "debug.h"

//extern bit need_notice_pd_cmd;
//extern unsigned char usb_pd_cmd_status;
#define PSWAP_TIMEOUT 1000>>2// OHO-393: Refer to USB PD Sepc Figure 7-17, t1+t2+t3 = tSnkStadby+tSrcSwapStdby+tNewSrc = 15+650+275 = 925ms
#define GOTOMIN_TIMEOUT 350>>2 //OHO-421: Refer to USB PD Spec Figure 7-19, t1+t2 = tSnkNewPower+tSrcReady = 15+285 = 300ms
#define RESPONSE_TIMEOUT 200>>2

unsigned char IF_RAM wait_send_pswap_response_timer = 0;
unsigned int IF_RAM Try_snk_timer=0;
unsigned int IF_RAM Try_src_timer=0;
unsigned int IF_RAM Try_WaitVbus_timer=0;
#ifndef DEL_UNUSE_FEATURE
unsigned char IF_RAM wait_send_dswap_response_timer = 0;
unsigned char IF_RAM wait_send_gotomin_response_timer = 0;
#endif
unsigned char IF_RAM wait_send_vswap_response_timer = 0;
unsigned char IF_RAM wait_send_rdo_response_timer = 0; 


extern bit dswap_response_got;
extern bit pswap_response_got;
extern bit vswap_response_got;
extern bit gotomin_response_got;
extern bit rdo_response_got;

bit need_check_pswap_response = 0;
bit need_check_dswap_response = 0;
bit need_check_gotomin_response = 0;
bit need_check_vswap_response = 0;
bit need_check_rdo_response = 0;

extern unsigned char xdata auto_pd_support_flag;

/**
 * @desc:   The Interface that AP sends the specific USB PD command to Ohio 
 *
 * @param: 
 * 		type: PD message type, define enum PD_MSG_TYPE. 
 * 		buf: the sepecific paramter pointer according to the message type:
 *                      eg: when AP update its source capability type=TYPE_PWR_SRC_CAP, 
 *			"buf" contains the content of PDO object,its format USB PD spec
 *                      customer can easily packeted it through PDO_FIXED_XXX macro:
 *                     default5Vsafe 5V, 0.9A fixed --> PDO_FIXED(5000,900, PDO_FIXED_FLAGS) 
 *                size: the paramter ponter's content length, if buf is null, it should be 0 
 *           		
 * @return:  0: success Error value 1: reject 2: fail, 3, busy
 */ 
unsigned char send_pd_msg(PD_MSG_TYPE type, const char *buf, unsigned char size)
{
	//unsigned char rst = 0;
	
	TRACE1("->%s: \n", interface_to_str(type));

	
	switch (type) {
		case TYPE_PWR_SRC_CAP:
			return  send_src_cap(buf, size);//send 0
			break;

		case TYPE_PWR_SNK_CAP:
			return send_snk_cap(buf, size);//send 1
			
			break;
		case TYPE_DP_SNK_IDENDTITY:
			return  send_dp_snk_identity(buf, size);//send 2
			break;
		case TYPE_SVID:
			return send_svid(buf, size); 
			break;
		case TYPE_GET_DP_SNK_CAP:
			return send_get_dp_snk_cap();//interface_send_ctr_msg(TYPE_GET_DP_SNK_CAP);//send 4			
			break;		
		case TYPE_PSWAP_REQ://send 10
			if(send_power_swap() == CMD_SUCCESS)
			{
				pswap_response_got = 0;
				need_check_pswap_response = 1;
				wait_send_pswap_response_timer= PSWAP_TIMEOUT;//ms
				return CMD_SUCCESS;
			}
			else 
				return CMD_FAIL;
			break;
#ifndef DEL_UNUSE_FEATURE			
		case TYPE_DSWAP_REQ://send 11
			if ( send_data_swap() == CMD_SUCCESS)
			{
				dswap_response_got = 0;
				need_check_dswap_response = 1;
				wait_send_dswap_response_timer= RESPONSE_TIMEOUT; //ms
				return 1;
			}
			else 
				return CMD_FAIL;
			break;

		case TYPE_GOTO_MIN_REQ://send 12
			if( send_gotomin() == CMD_SUCCESS)
			{
				gotomin_response_got = 0;
				need_check_gotomin_response = 1;
				wait_send_gotomin_response_timer= GOTOMIN_TIMEOUT; //ms
				return CMD_SUCCESS;
			}
			else
				return CMD_FAIL;
			break;
		case TYPE_VDM://send 14
			return send_vdm(buf, size);
			break;
		case TYPE_SOP_PRIME://send 1c
			return send_sop_prime(buf, size);
			break;
		case TYPE_SOP_DOUBLE_PRIME://send 1d
			return send_sop_double_prime(buf, size);
			break;
#endif
		case TYPE_VCONN_SWAP_REQ://send 13
			if(send_vconn_swap() == CMD_SUCCESS)
			{
				vswap_response_got= 0;
				need_check_vswap_response = 1;
				//need_notice_pd_cmd = 0;
				wait_send_vswap_response_timer= RESPONSE_TIMEOUT; //ms
				return CMD_SUCCESS;
			}
			else
				return CMD_FAIL;
			break;


		case TYPE_DP_SNK_CFG://send 15
			return send_dp_snk_cfg(buf, size);				
			break;
			

		case TYPE_PD_STATUS_REQ://send 17
			return send_get_pd_status();//interface_get_pd_status();
			//need_notice_pd_cmd = 0;
			//wait_send_dswap_response_time= 200; //ms
			break;
		case TYPE_PWR_OBJ_REQ://send 16
			if(send_rdo(buf, size) == CMD_SUCCESS)
			{
				rdo_response_got= 0;
				need_check_rdo_response = 1;
				wait_send_rdo_response_timer= RESPONSE_TIMEOUT; //ms
				return CMD_SUCCESS;
			}
			else
				return CMD_FAIL;
			break;
		case TYPE_GET_SNK_CAP://0x1B
			return send_get_sink_cap();//interface_send_get_sink_cap();//added for OHO-295
			
		case TYPE_ACCEPT://send 5
			return send_accept();//interface_send_accept();
			break;
			
	       case TYPE_REJECT://send 6
			return send_reject();//interface_send_reject();
			break;

		case TYPE_SOFT_RST://send f1
			return send_soft_rst();
			//wait_cmd_response_time = 1;
			break;
		case TYPE_HARD_RST://send f2
			return send_hard_rst();
			break;
#ifdef USE_PD30
		case TYPE_EXT_GET_BATT_CAP://send a3
		    if(buf != NULL)
				pd_get_battery_cap_ref = buf[0]; // battery index
			return send_ext_message(PD_EXT_GET_BATTERY_CAP, 0);
			break;
		case TYPE_EXT_GET_BATT_STS://send a4
			if(buf != NULL)
				pd_get_battery_status_ref = buf[0]; // battery index
			return send_ext_message(PD_EXT_GET_BATTERY_STATUS, 0);
			break;
		case TYPE_EXT_GET_MFR_INFO://send a6
			return send_ext_message(PD_EXT_GET_MANUFACTURER_INFO, 0);
			break;
		case TYPE_EXT_ALERT://send ab
			return send_unext_message(PD_DATA_ALERT, 0);
			break;
		case TYPE_EXT_GET_SRC_CAP://send ad
			return send_unext_message(PD_CTRL_GET_SOURCE_CAP_EXTENDED, 0);
			break;
		case TYPE_EXT_GET_SRC_STS://send ae
			return send_unext_message(PD_CTRL_GET_STATUS, 0);
			break;
		case TYPE_FR_SWAP_SIGNAL://send b0
			return send_frswap_signal();
			break;
		case TYPE_GET_SINK_CAP_EXT://send b1
			return send_unext_message(PD_CTRL_GET_SINK_CAP_EXTENDED, 0);
			break;
#endif
		default:
			TRACE1("unknown type %2BX\n", type);
			return CMD_FAIL;
			break;
	}
}




/**
 * @desc:   The Interface that AP handle the specific USB PD command from Ohio 
 *
 * @param: 
 * 		type: PD message type, define enum PD_MSG_TYPE. 
 * 		buf: the sepecific paramter pointer according to the message type:
 *                      eg: when AP update its source capability type=TYPE_PWR_SRC_CAP, 
 *			"buf" contains the content of PDO object,its format USB PD spec
 *                      customer can easily packeted it through PDO_FIXED_XXX macro:
 *                     default5Vsafe 5V, 0.9A fixed --> PDO_FIXED(5000,900, PDO_FIXED_FLAGS) 
 *                size: the paramter ponter's content length, if buf is null, it should be 0 
 *           		
 * @return:  0: success 1: fail 
 *  
 */ 
unsigned char dispatch_rcvd_pd_msg(PD_MSG_TYPE type, void *para, unsigned char para_len)
{	  
	
	//unsigned char rst = 0;
	//TRACE1("recv %02bx\n\n", type);
	TRACE1("<- %s\n", interface_to_str(type));
	switch (type) {
		case TYPE_PWR_SRC_CAP://0
			//TRACE(" Recv: TYPE_PWR_SRC_CAP\n");
			return recv_pd_source_caps_default_callback(para, para_len);
			//rst = 1;
	
			//execute the receved handle function,just request default the first pdo
			break;

		case TYPE_PWR_SNK_CAP://0x01
			//TRACE(" Recv: TYPE_PWR_SNK_CAP\n");
			//received peer's sink caps, just store it in SINK_PDO array
			return recv_pd_sink_caps_default_callback(para, para_len);
			//rst = 1;
			break;
		case TYPE_DP_SNK_IDENDTITY://0x02
			return recv_pd_dp_sink_identity_default_callback(para, para_len);
			//rst = 1;
			break;
		case TYPE_SVID://0x03
			return recv_pd_svid_default_callback(para, para_len);
			//rst = 1;
			break;
		case TYPE_PWR_OBJ_REQ:	//0x16
			//TRACE(" Recv: TYPE_PWR_OBJ_REQ\n");
			  if(auto_pd_support_flag) 
			  	return 1;
			  else
			return recv_pd_pwr_object_req_default_callback(para, para_len);
			
			 //rst = 1;
			break;
#ifndef DEL_UNUSE_FEATURE	
		case TYPE_DSWAP_REQ://0x11
			//TRACE(" Recv: TYPE_DSWAP_REQ\n");
			return recv_pd_dswap_default_callback(para, para_len);	
			//rst = 1;
			break;
		case TYPE_GOTO_MIN_REQ://0x12
			//TRACE(" Recv: TYPE_GOTO_MIN_REQ\n");
			return recv_pd_goto_min_default_callback(para, para_len);
			//rst = 1;
			break;
		case TYPE_SOP_PRIME://0x1C
			return recv_pd_sop_prime_default_callback(para, para_len);
			break;
		case TYPE_SOP_DOUBLE_PRIME://0x1D
			return recv_pd_sop_double_prime_default_callback(para, para_len);
			break;
		case TYPE_VDM: //0x14
			return recv_pd_vdm_defalut_callback(para, para_len);
			//rst = 1;
			break;
#endif
		case TYPE_PSWAP_REQ://0x10
			//TRACE(" Recv: TYPE_PSWAP_REQ\n");
			return recv_pd_pswap_default_callback(para, para_len);
			//rst = 1;
			break;

		case TYPE_ACCEPT:	//0x05	
			//TRACE(" Recv: TYPE_ACCEPT\n");
			return recv_pd_accept_default_callback(para, para_len);  
			//rst = 1;
			break;
		case TYPE_REJECT:	//0x06
			//TRACE(" Recv: TYPE_REJECT\n");
			return recv_pd_reject_default_callback(para, para_len);
			//rst = 1;
			break;



			
		case TYPE_PD_STATUS_REQ://0x17
			//TRACE(" Recv: TYPE_PD_STATUS_REQ\n");
			return 1;
			//rst = 1;
			break;

		case TYPE_DP_ALT_ENTER://0x19
			//TRACE(" Recv: TYPE_RESPONSE_TO_REQ\n");
			return recv_pd_dp_alt_enter_default_callback(para, para_len);//added for OHO-223	  
			//rst = 1;
				
	          break;
		case TYPE_DP_ALT_EXIT://0x1A
			//TRACE(" Recv: TYPE_RESPONSE_TO_REQ\n");
			return recv_pd_dp_alt_exit_default_callback(para, para_len);	//added for OHO-223 
			//rst = 1;
	        break;
#ifdef USE_PD30
		case TYPE_EXT_SRC_CAP:
		case TYPE_EXT_SRC_STS:
		case TYPE_EXT_GET_BATT_CAP:
		case TYPE_EXT_GET_BATT_STS:
		case TYPE_EXT_BATT_CAP:
		case TYPE_EXT_GET_MFR_INFO:
		case TYPE_EXT_MFR_INFO:
		case TYPE_EXT_PDFU_REQUEST:
		case TYPE_EXT_PDFU_RESPONSE:
		case TYPE_EXT_BATT_STS:
		case TYPE_EXT_ALERT:
		case TYPE_EXT_NOT_SUPPORTED:
		case TYPE_EXT_GET_SRC_CAP:
		case TYPE_EXT_GET_SRC_STS:
		case TYPE_EXT_FR_SWAP:
		case TYPE_GET_SINK_CAP_EXT:
		case TYPE_EXT_SINK_CAP_EXT:
			//TRACE(" Recv: PD3_MSG\n");
			return recv_ext_msg_callback(type, para, para_len);
			break;
#endif

		case TYPE_RESPONSE_TO_REQ://0xf0
			//TRACE(" Recv: TYPE_RESPONSE_TO_REQ\n");
			return recv_pd_cmd_rsp_default_callback(para, para_len);	  
			//rst = 1;				
	        break;
#if 1 // debug
		case TYPE_GET_VAR://0xfc
			return recv_debug_callback(type, para, para_len);	  
			break;
#endif
		default:
			//TRACE1("can't recognize the type %2BX\n", type);
			//rst = 0;
			break;
	}
			return CMD_FAIL;
} 

