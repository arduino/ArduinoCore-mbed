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
#include "private_interface.h"	
#include "public_interface.h"	
#ifdef USE_PD30
#include "pd_ext_message.h"
#endif
#include "I2C_SW.h"
#include "REG_DRV.h"
#include <STRING.H>
#include "debug.h"
#include "EFM8UB2_helper.h"

#define Interface_Base_Addr 0x7E
#define InterfaceSendBuf_Addr 0xC0
#define InterfaceRecvBuf_Addr 0xE0

#if 1 // test

#ifdef DEL_UNUSE_FEATURE
//In order to optimal the RAM size, it is ok to change the pd_src_pdo length to 4*X,
//X = the source capability's PDO number in the real application of customer side.
//e.g. if only 2 PDO are required, please define as "unsigned char pd_src_pdo[4*2];"
unsigned char IF_RAM pd_src_pdo[4*1];
#else
unsigned char IF_RAM pd_src_pdo[4*7];
#endif
unsigned char IF_RAM pd_src_pdo_cnt;

unsigned char IF_RAM sending_len;

extern unsigned char sw1_rdo_index;

unsigned char IF_RAM InterfaceSendBuf[INTERFACE_SEND_BUF_SIZE];
unsigned char IF_RAM InterfaceRecvBuf[INTERFACE_RECV_BUF_SIZE];
unsigned char IF_RAM InterfaceSendRetryCount = 0;
bit recv_msg_success = 0;
bit interface_recv_int;


bit pswap_response_got = 0;
bit dswap_response_got = 0;
bit vswap_response_got = 0;
bit gotomin_response_got = 0;
bit rdo_response_got = 0;

extern bit need_check_pswap_response;
extern bit need_check_dswap_response;
extern bit need_check_gotomin_response;
extern bit need_check_vswap_response;
extern bit need_check_rdo_response;

//#define set_rdo_value(pd_rdo,v0,v1,v2,v3)    do { pd_rdo[0] = (v0); pd_rdo[1] = (v1);  pd_rdo[2] = (v2); pd_rdo[3] = (v3);}while(0)


unsigned char IF_RAM global_i = 0;
unsigned char IF_RAM global_c0 = 0;
unsigned char IF_RAM global_c1 = 0;
unsigned long IF_RAM global_ulong0= 0;
unsigned int IF_RAM global_int0 = 0;
unsigned int IF_RAM global_int1 = 0;

#define is_global_xdata_avialable()	\
	((global_i == 0)&& (global_c0 == 0) && (global_c1 == 0) \
	&& (global_ulong0 == 0) &&(global_int0 == 0)&&(global_int1 == 0))

#define clear_global_xdata() \
	do{global_i = 0; global_c0 = 0;global_c1 = 0;\
		global_ulong0 = 0; global_int0 = 0; global_int1 = 0;}while(0)
		
#define sel_voltage_pdo_index global_c0
#define sel_pdo global_ulong0
#define pdo_max_voltage global_int0
#define pdo_max_current_or_power global_int1
#define max_ma global_int0
#define op_ma global_int1

void build_rdo_from_source_caps(unsigned char obj_cnt, unsigned char  *buf)
{
	//unsigned long sel_pdo = 0;
	//unsigned int pdo_max_voltage = 0;
	//unsigned int pdo_max_voltage_tmp = 0;
	//unsigned char
	//unsigned char i;
	//unsigned int pdo_max_current_or_power;
	//unsigned long rdo;	
	if(!is_global_xdata_avialable())
	{
		TRACE("build rdo: Global XDTA is NOT Avliable.\n");
		return;
	}

	if(sw1_rdo_index+ 1 > obj_cnt)
	{
		for (global_i= 0; global_i < obj_cnt; global_i++){
		{
			//find the max voltage pdo
			((unsigned char *)&sel_pdo)[0] = buf[4*global_i+3];
			((unsigned char *)&sel_pdo)[1] = buf[4*global_i+2];
			((unsigned char *)&sel_pdo)[2] = buf[4*global_i+1];
			((unsigned char *)&sel_pdo)[3] = buf[4*global_i+0];

			switch(GET_PDO_TYPE(sel_pdo))
			{	
				case (PDO_TYPE_FIXED >> 30):
					if((GET_PDO_FIXED_VOLT(sel_pdo)) > pdo_max_voltage)
					{
						pdo_max_voltage = GET_PDO_FIXED_VOLT(sel_pdo);
						sel_voltage_pdo_index = global_i;
					}
					break;
				case(PDO_TYPE_VARIABLE >> 30):
					if(GET_VAR_MAX_VOLT(sel_pdo) > pdo_max_voltage)
					{
						pdo_max_voltage = GET_VAR_MAX_VOLT(sel_pdo);
						sel_voltage_pdo_index = global_i;
					}
					break;
				case(PDO_TYPE_BATTERY >> 30):
					if(GET_BATT_MAX_VOLT(sel_pdo) > pdo_max_voltage)
					{
						pdo_max_voltage = GET_BATT_MAX_VOLT(sel_pdo);
						sel_voltage_pdo_index = global_i;
					}
					break;
				default:				
					TRACE("Reserved PDO Type\n");
					break;
			}

			}
		}

	}
	else
	{
		((unsigned char *)&sel_pdo)[0] = buf[sw1_rdo_index*4+3];
		((unsigned char *)&sel_pdo)[1] = buf[sw1_rdo_index*4+2];
		((unsigned char *)&sel_pdo)[2] = buf[sw1_rdo_index*4+1];
		((unsigned char *)&sel_pdo)[3] = buf[sw1_rdo_index*4+0];

		switch(GET_PDO_TYPE(sel_pdo))
		{
			case(PDO_TYPE_FIXED >> 30):
				pdo_max_voltage = GET_PDO_FIXED_VOLT(sel_pdo);
				break;
			case(PDO_TYPE_VARIABLE >> 30):
				pdo_max_voltage = GET_VAR_MAX_VOLT(sel_pdo);
				break;
			case(PDO_TYPE_BATTERY >> 30):
				pdo_max_voltage = GET_BATT_MAX_VOLT(sel_pdo);
				break;
			default:
				TRACE("Reserved PDO Type\n");
				break;
		}

		sel_voltage_pdo_index = sw1_rdo_index;
	}

	((unsigned char *)&sel_pdo)[0] = buf[sel_voltage_pdo_index*4+3];
	((unsigned char *)&sel_pdo)[1] = buf[sel_voltage_pdo_index*4+2];
	((unsigned char *)&sel_pdo)[2] = buf[sel_voltage_pdo_index*4+1];
	((unsigned char *)&sel_pdo)[3] = buf[sel_voltage_pdo_index*4+0];

	switch(GET_PDO_TYPE(sel_pdo))
	{
		case(PDO_TYPE_FIXED >> 30):
		case(PDO_TYPE_VARIABLE>>30):
			pdo_max_current_or_power = GET_PDO_FIXED_CURR(sel_pdo);
			if(pdo_max_current_or_power >= MAX_REQUEST_CURRENT)
			{
				sel_pdo = RDO_FIXED(sel_voltage_pdo_index+1, MAX_REQUEST_CURRENT, MAX_REQUEST_CURRENT, 0);
			}
			else
			{
				sel_pdo = RDO_FIXED(sel_voltage_pdo_index+1, pdo_max_current_or_power, pdo_max_current_or_power, RDO_CAP_MISMATCH);
			}
			break;
		case (PDO_TYPE_BATTERY >> 30):
			pdo_max_current_or_power = GET_BATT_OP_POWER(sel_pdo)>>2;
			if(pdo_max_current_or_power >= (MAX_REQUEST_CURRENT*(MAX_REQUEST_VOLTAGE/1000))>>2)
			{
				sel_pdo = RDO_BATT(sel_voltage_pdo_index+1, MAX_REQUEST_CURRENT*(MAX_REQUEST_VOLTAGE/1000), MAX_REQUEST_CURRENT*(MAX_REQUEST_VOLTAGE/1000), 0);
			}
			else
			{
				sel_pdo = RDO_BATT(sel_voltage_pdo_index+1, (unsigned long)pdo_max_current_or_power*4, (unsigned long)pdo_max_current_or_power*4, RDO_CAP_MISMATCH);
			}
			break;
		default:
			break;
	}
	
	
	//set_rdo_value(pd_rdo,sel_pdo & 0xff,(sel_pdo >> 8) & 0xff, (sel_pdo >> 16) & 0xff, (sel_pdo >> 24) & 0xff);

	clear_global_xdata();

	//return pd_rdo;
	
}
#if 0
unsigned long change_bit_order(unsigned char *pbuf)
{
    return ((unsigned long)pbuf[3] << 24) | ((unsigned long)pbuf[2] << 16) 
        | ((unsigned long)pbuf[1] << 8) | pbuf[0];
}

unsigned char pd_check_requested_voltage(unsigned long rdo)
{
	//unsigned int max_ma = rdo & 0x3FF;
	//unsigned int op_ma = (rdo >> 10) & 0x3FF;
	//unsigned char idx = rdo >> 28;
	
	//unsigned long pdo;
	//unsigned int pdo_max;
	
	if (!(rdo >> 28) || (rdo >> 28)> pd_src_pdo_cnt)
	{
	       TRACE3("rdo = %lu, Requested RDO is %d, Provided RDO number is %d\n", rdo, (unsigned int)(rdo >> 28), (unsigned int)pd_src_pdo_cnt);
		return 0; /* Invalid index */
	}
	//Update to pass TD.PD.SRC.E12 Reject Request
	//pdo = change_bit_order(pd_src_pdo + ((idx - 1) * 4));
	//pdo_max = (unsigned int)(( change_bit_order(pd_src_pdo + ((idx - 1) * 4))) & 0x3ff);
	//TRACE1("pdo_max = %lu\n", pdo_max);

	//TRACE3("Requested  %d/~%d mA, idx %d\n",	(unsigned int)op_ma * 10, (unsigned int)max_ma *10, (unsigned int)idx);
	/* check current ... */
	if (( (rdo >> 10) & 0x3FF)> (( change_bit_order(pd_src_pdo + (((rdo >> 28) - 1) * 4))) & 0x3ff))//Update to pass TD.PD.SRC.E12 Reject Request
		return 0; /* too much op current */
	if ((rdo & 0x3FF) > (( change_bit_order(pd_src_pdo + (((rdo >> 28) - 1) * 4))) & 0x3ff))//Update to pass TD.PD.SRC.E12 Reject Request
		return 0; /* too much max current */



	return 1;
}
#endif

/* Recieve Power Delivery Source Capability message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_source_caps_default_callback(void *para, unsigned char para_len)
{
	//unsigned char pd_rdo[PD_ONE_DATA_OBJECT_SIZE];
	unsigned char pd_rdo[4];
	if ( para_len %4 != 0)
		return 0;
    build_rdo_from_source_caps(para_len/4, para);
	//((unsigned char *)&global_ulong0)[0] = ((unsigned char*)para)[3];
	//TRACE1("sel_pdo = %lu\n", sel_pdo);

	pd_rdo[3] = (unsigned char)((sel_pdo>>24)&0xFF);
	pd_rdo[2] = (unsigned char)((sel_pdo>>16)&0xFF);
	pd_rdo[1] = (unsigned char)((sel_pdo>>8)&0xFF);
	pd_rdo[0] = (unsigned char)((sel_pdo>>0)&0xFF);
	

	//sel_pdo = ((unsigned long)global_c0)|((unsigned long)global_c1<<8)|((unsigned long)global_int0<<16)|((unsigned long)global_int1<<24);

	clear_global_xdata();
	//send_pd_msg(TYPE_PWR_OBJ_REQ, pd_rdo, sizeof(pd_rdo));
  	send_pd_msg(TYPE_PWR_OBJ_REQ, pd_rdo, 4);


	 #if 0
	if (build_rdo_from_source_caps(para_len/4, para, &pd_rdo)) {
		send_pd_msg(TYPE_PWR_OBJ_REQ, pd_rdo, sizeof(pd_rdo));
	}
	else
	{
		TRACE("Failed to bulied RDO\n");
	}
	#endif
	return 1;
}

/* Recieve Power Delivery Sink Capability message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_sink_caps_default_callback(void *para, unsigned char para_len)
{
	para=para;
	if ( para_len %4 != 0)
		return 0;
	if(para_len > VDO_SIZE)
		return 0;
	
	//OHO-407: When Received Sink Capability, API Shall Not Overwrite Its Sink Capability By the Received One
	//memcpy(pd_snk_pdo, para, para_len);
	//pd_snk_pdo_cnt = para_len /4;
	return 1;
}
/* Recieve DP Sink Identity message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_dp_sink_identity_default_callback(void *para, unsigned char para_len)
{
	para = para;
	para_len = para_len;
	//TRACE_ARRAY(para, para_len);
	return 1;
}

/* Recieve SVID message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_svid_default_callback(void *para, unsigned char para_len)
{
	para = para;
	para_len = para_len;
	//TRACE_ARRAY(para, para_len);
	return 1;
}

/* Recieve Power Delivery Power Object Request message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_pwr_object_req_default_callback(void *para, unsigned char para_len)
{
	
	//unsigned long rdo = 0;

	//TRACE("enter recv_pd_pwr_object_req_default_callback\n");
	if ( para_len %4 != 0)
		return 0;

	if(para_len != 4){
		return 0;
	}

	if(!is_global_xdata_avialable())
	{
		TRACE("recv_rdo: Global XDTA is NOT Avliable.\n");
		return 0;
	}

	//rdo = pdo[0] | (pdo[1] << 8) |  (pdo[2] <<16) | (pdo[3] <<24);
	((unsigned char *)&global_ulong0)[0] = ((unsigned char*)para)[3];
	((unsigned char *)&global_ulong0)[1] = ((unsigned char*)para)[2];
	((unsigned char *)&global_ulong0)[2] = ((unsigned char*)para)[1];
	((unsigned char *)&global_ulong0)[3] = ((unsigned char*)para)[0];
	//TRACE5(	"rdo = %08LX, %2BX, %2BX, %2BX, %2BX\n", rdo, pdo[0], pdo[1], pdo[2], pdo[3]);

	global_c0 =(unsigned char) (global_ulong0 >> 28);

	if (!global_c0 || global_c0> pd_src_pdo_cnt)
	{
	    TRACE3("rdo = %lu, Requested RDO is %2BX, Provided RDO number is %2BX\n", global_ulong0, global_c0, pd_src_pdo_cnt);
		clear_global_xdata();
		interface_send_reject(); /* Invalid index */
		//return 1;
	}

	max_ma = global_ulong0& 0x3FF;//rdo & 0x3FF;
	op_ma = (global_ulong0 >> 10) & 0x3FF;//(rdo >> 10) & 0x3FF;
	global_ulong0 = (((unsigned long)( pd_src_pdo + ((global_c0- 1) * 4))[3] << 24) | ((unsigned long)( pd_src_pdo + ((global_c0- 1) * 4))[2] << 16) 
        | ((unsigned long)( pd_src_pdo + ((global_c0- 1) * 4))[1] << 8) | ( pd_src_pdo + ((global_c0- 1) * 4))[0]);//change_bit_order( pd_src_pdo + ((global_c0- 1) * 4)) & 0x3ff;
	global_ulong0 &= 0x3FF;
	

	/* check current ... */
	if ( op_ma>  (unsigned int)global_ulong0)//Update to pass TD.PD.SRC.E12 Reject Request
	{
		clear_global_xdata();
		interface_send_reject(); /* too much op current */
		//return 1;
	}
	if ( max_ma>(unsigned int)global_ulong0)//Update to pass TD.PD.SRC.E12 Reject Request
	{
		clear_global_xdata();
		interface_send_reject(); /* too much max current */		
		//return 1;
	}
	clear_global_xdata();

	interface_send_accept();    


	/*if (pd_check_requested_voltage(rdo)) {
		interface_send_accept();    
	}
	else {
		interface_send_reject();   
	}*/

	//return 1;
}



/* Recieve accept message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_accept_default_callback(void *para, unsigned char para_len)
{
	para = para; 
	para_len = para_len;

	return 1;
}


/* Recieve reject message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_reject_default_callback(void *para, unsigned char para_len)
{
	para = para; 
	para_len = para_len;


	return 1;
}




/*PD Status command response, default callback function.
  *It can be change by customer for redevelopment
  * Byte0: CC status from ohio, 
  * Byte1: misc status from ohio 
  * Byte2: debug ocm FSM state from ohio
  */
void interface_get_status_result(void) 
{	 
	//unsigned char cc_status  = 0;
	//unsigned char misc_status = 0;
	//unsigned char pd_fsm_status = 0;
	//cc_status = InterfaceRecvBuf[3];
	//TRACE1("Ohio CC Status:%x\n", cc_status);
	TRACE1("Ohio CC Status:%x\n", InterfaceRecvBuf[3]);
	
	//misc_status = InterfaceRecvBuf[4];
	//TRACE1("Ohio misc status:%x\n", misc_status);
	TRACE1("Ohio misc status:%x\n", InterfaceRecvBuf[4]);

	//pd_fsm_status = InterfaceRecvBuf[5];
	//TRACE1("Ohio pd_fsm_status:%x\n", pd_fsm_status);
	TRACE1("Ohio pd_fsm_status:%x\n", InterfaceRecvBuf[5]);
	
}
/* Recieve response message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_cmd_rsp_default_callback(void *para, unsigned char para_len)
{
	para = para;
	para_len = para_len;
	TRACE2("RESPONSE for %s is %s\n",
		interface_to_str(RESPONSE_REQ_TYPE()),result_to_str(RESPONSE_REQ_RESULT()));

	switch(RESPONSE_REQ_TYPE()){
		case TYPE_PD_STATUS_REQ:
			//need_notice_pd_cmd =1;
			//usb_pd_cmd_status = CMD_SUCCESS;
		 	//if (  InterfaceRecvBuf[2] == (unsigned char)TYPE_PD_STATUS_REQ){
		 	/*get cc result to Ohio */
	        	interface_get_status_result();
			break;
		case TYPE_DSWAP_REQ:
			//need_notice_pd_cmd  =1;
			//usb_pd_cmd_status = RESPONSE_REQ_RESULT();
			dswap_response_got = 1;
			break;
			
		case TYPE_PSWAP_REQ:
			pswap_response_got = 1;
			//need_notice_pd_cmd = 1;			
			//usb_pd_cmd_status = RESPONSE_REQ_RESULT();
		
			break;
		case TYPE_VCONN_SWAP_REQ:
			vswap_response_got = 1;
			//need_notice_pd_cmd = 1;			
			//usb_pd_cmd_status = RESPONSE_REQ_RESULT();
			break;
		case TYPE_GOTO_MIN_REQ:
			gotomin_response_got = 1;
			//need_notice_pd_cmd = 1;			
			//usb_pd_cmd_status = RESPONSE_REQ_RESULT();
			break;
		case TYPE_PWR_OBJ_REQ:
			rdo_response_got = 1;
			//need_notice_pd_cmd = 1;			
			//usb_pd_cmd_status = RESPONSE_REQ_RESULT();
			break;
			
		default:
			break;
	} 

	//if (need_notice_pd_cmd){
		//check pd cmd has been locked ?
		//write_lock_irq(&usb_pd_cmd_rwlock);
		//if (usb_pd_cmd_counter){
		//	usb_pd_cmd_counter=0;
		//}
		//write_unlock_irq(&usb_pd_cmd_rwlock);
	//}
	return 1;
}

/* Recieve dp alternate mode enter message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_dp_alt_enter_default_callback(void *para, unsigned char para_len)
{
	para = para; 
	para_len = para_len;


	return 1;
}

/* Recieve dp alternate mode exit message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_dp_alt_exit_default_callback(void *para, unsigned char para_len)
{
	para = para; 
	para_len = para_len;


	return 1;
}
#ifndef DEL_UNUSE_FEATURE
/* Recieve sop_1 message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_sop_prime_default_callback(void *para, unsigned char para_len)
{
	para = para; 
	para_len = para_len;

	//TRACE_ARRAY(para, para_len);

	return 1;
}

/* Recieve sop_2 message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_sop_double_prime_default_callback(void * para, unsigned char para_len)
{
	para = para; 
	para_len = para_len;
	
//	TRACE_ARRAY(para, para_len);

	return 1;
}

/* Recieve Data Role Swap message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through init_pd_msg_callback, it it pd_callback is not 0, using the default   
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_dswap_default_callback(void *para, unsigned char para_len)
{
	para = para;
	para_len = para_len;

	
	return 1;
}
/* Recieve GotoMini message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_goto_min_default_callback(void *para, unsigned char para_len)
{
	para = para; 
	para_len = para_len;

	

	return 1;
}
/* Recieve Power Delivery Unstructured VDM message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_vdm_defalut_callback(void *para, unsigned char para_len)
{
	//TRACE_ARRAY(para, para_len);
	para = para;
	para_len = para_len;

	return 1;
}
#endif
/* Recieve Power Role Swap message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through init_pd_msg_callback, it it pd_callback is not 0, using the default   
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
unsigned char recv_pd_pswap_default_callback(void *para, unsigned char para_len)
{
	para = para;
	para_len = para_len;

	return 1;
}

#ifdef USE_PD30

char ConvertInterfaceTypeToPd3Type(unsigned char type, unsigned char *ext, unsigned char *pd3_type)
{ 
	switch(type)
	{
		case TYPE_EXT_SRC_CAP:
			*ext = 1;
			*pd3_type = PD_EXT_SOURCE_CAP;
			break;
		case TYPE_EXT_SRC_STS:
			*ext = 1;
			*pd3_type = PD_EXT_STATUS;
			break;
		case TYPE_EXT_GET_BATT_CAP:
			*ext = 1;
			*pd3_type = PD_EXT_GET_BATTERY_CAP;
			break;
		case TYPE_EXT_GET_BATT_STS:
			*ext = 1;
			*pd3_type = PD_EXT_GET_BATTERY_STATUS;
			break;
		case TYPE_EXT_BATT_CAP:
			*ext = 1;
			*pd3_type = PD_EXT_BATTERY_CAP;
			break;
		case TYPE_EXT_GET_MFR_INFO:
			*ext = 1;
			*pd3_type = PD_EXT_GET_MANUFACTURER_INFO;
			break;
		case TYPE_EXT_MFR_INFO:
			*ext = 1;
			*pd3_type = PD_EXT_MANUFACTURER_INFO;
			break;
		case TYPE_EXT_PDFU_REQUEST:
			*ext = 1;
			*pd3_type = PD_EXT_FW_UPDATE_REQUEST;
			break;
		case TYPE_EXT_PDFU_RESPONSE:
			*ext = 1;
			*pd3_type = PD_EXT_FW_UPDATE_RESPONSE;
			break;
		case TYPE_EXT_BATT_STS:
			*ext = 0;
			*pd3_type = PD_DATA_BATTERY_STATUS;
			break;
		case TYPE_EXT_ALERT:
			*ext = 0;
			*pd3_type = PD_DATA_ALERT;
			break;
		case TYPE_EXT_NOT_SUPPORTED:
			*ext = 0;
			*pd3_type = PD_CTRL_NOT_SUPPORTED;
			break;
		case TYPE_EXT_GET_SRC_CAP:
			*ext = 0;
			*pd3_type = PD_CTRL_GET_SOURCE_CAP_EXTENDED;
			break;
		case TYPE_EXT_GET_SRC_STS:
			*ext = 0;
			*pd3_type = PD_CTRL_GET_STATUS;
			break;
		case TYPE_EXT_FR_SWAP:
			*ext = 0;
			*pd3_type = PD_CTRL_FR_SWAP;
			break;
		case TYPE_GET_SINK_CAP_EXT:
			*ext = 0;
			*pd3_type = PD_CTRL_GET_SINK_CAP_EXTENDED;
			break;
		case TYPE_EXT_SINK_CAP_EXT:
			*ext = 1;
			*pd3_type = PD_EXT_SINK_CAP;
			break;
		default:
			// unknow type
			return -1;
	}
	return 0;
}

unsigned char recv_ext_msg_callback(unsigned char type, unsigned char *para, unsigned char para_len)
{
    unsigned char ret = 0;
	unsigned char ext;

	#if 0 // debug
	TRACE_ARRAY(para, para_len);	  
	//TRACE3(" SOP(%bx),TYPE(%bx) \n",para[0],type);
	#endif
	if(ConvertInterfaceTypeToPd3Type(type, &ext, &type) < 0)
			return -3;
	
	if(ext) {
	switch(type)
		{
		case PD_EXT_SOURCE_CAP:
			//TRACE(" PD_EXT_SOURCE_CAP\n");
			recv_ext_message(PD_EXT_SOURCE_CAP,para+1,para_len-1);
			break;
		case PD_EXT_STATUS:
			//TRACE(" PD_EXT_STATUS\n");
			recv_ext_message(PD_EXT_STATUS,para+1,para_len-1);
			break;
		case PD_EXT_GET_BATTERY_CAP:
			//TRACE(" PD_EXT_GET_BATTERY_CAP\n");
			recv_ext_message(PD_EXT_GET_BATTERY_CAP,para+1,para_len-1);
			break;
		case PD_EXT_GET_BATTERY_STATUS:
			//TRACE(" PD_EXT_GET_BATTERY_STATUS\n");
			recv_ext_message(PD_EXT_GET_BATTERY_STATUS,para+1,para_len-1);
			break;
		case PD_EXT_BATTERY_CAP:
			//TRACE(" PD_EXT_BATTERY_CAP\n");
			recv_ext_message(PD_EXT_BATTERY_CAP,para+1,para_len-1);
			break;
		case PD_EXT_GET_MANUFACTURER_INFO:
			//TRACE(" PD_EXT_GET_MANUFACTURER_INFO\n");
			recv_ext_message(PD_EXT_GET_MANUFACTURER_INFO,para+1,para_len-1);
			break;
		case PD_EXT_MANUFACTURER_INFO:
			//TRACE(" PD_EXT_MANUFACTURER_INFO\n");
			recv_ext_message(PD_EXT_MANUFACTURER_INFO,para+1,para_len-1);
			break;
		case PD_EXT_SECURITY_REQUEST:
			//TRACE(" PD_EXT_SECURITY_REQUEST\n");
			// ToDo: Implement USBTypeCAuthentication 1.0 spec.
			send_unext_message(PD_CTRL_NOT_SUPPORTED, 0); 
			break;
		case PD_EXT_SECURITY_RESPONSE:
			//TRACE(" PD_EXT_SECURITY_RESPONSE\n");
			// ToDo: Implement USBTypeCAuthentication 1.0 spec.
			break;
		case PD_EXT_FW_UPDATE_REQUEST:
			//TRACE(" PD_EXT_FW_UPDATE_REQUEST\n");
#if USE_PDFU
            ret=RecvBlock(PD_EXT_FW_UPDATE_REQUEST,para+1,para_len-1);
            if(ret == RECV_BLOCK_IDLE)
        	{
				recv_pdfu_request(para+1+2,para_len-1-2);
        	}
			else if(ret == RECV_BLOCK_FINISH)
			{
				recv_pdfu_request(pd_block_recv_buf,pd_block_recv_len);
				pd_block_recv_len = 0;
			}
			ret = 0;
#endif		
			break;
		case PD_EXT_FW_UPDATE_RESPONSE:
			//TRACE(" PD_EXT_FW_UPDATE_RESPONSE\n");
#if USE_PDFU
			recv_pdfu_response(para+1,para_len-1);
#endif
			break;
		case PD_EXT_SINK_CAP:
			//TRACE(" PD_EXT_SINK_CAP\n");
			recv_ext_message(PD_EXT_SINK_CAP, para+1,para_len-1);
			break;
		default:
			TRACE(" Unknown ext pd msg\n");
			ret = 1;
			break;
		
		}
	}
	else {
		switch(type)
			{
			case PD_CTRL_NOT_SUPPORTED:
				//TRACE(" PD_CTRL_NOT_SUPPORTED\n");
				break;
			case PD_CTRL_GET_SOURCE_CAP_EXTENDED:
				//TRACE(" PD_CTRL_GET_SOURCE_CAP_EXTENDED\n");
				recv_unext_message(PD_CTRL_GET_SOURCE_CAP_EXTENDED,para+1,para_len-1);
				break;
			case PD_CTRL_GET_STATUS:
				//TRACE(" PD_CTRL_GET_STATUS\n");
				recv_unext_message(PD_CTRL_GET_STATUS,para+1,para_len-1);
				break;
			case PD_CTRL_FR_SWAP:
				//TRACE(" PD_CTRL_FR_SWAP\n");
				recv_unext_message(PD_CTRL_FR_SWAP,para+1,para_len-1);
				break;
			case PD_DATA_BATTERY_STATUS:
				//TRACE(" PD_DATA_BATTERY_STATUS\n");
				recv_unext_message(PD_DATA_BATTERY_STATUS,para+1,para_len-1);
				break;
			case PD_DATA_ALERT:
				//TRACE(" PD_DATA_ALERT\n");
				recv_unext_message(PD_DATA_ALERT,para+1,para_len-1);
				break;
			case PD_CTRL_GET_SINK_CAP_EXTENDED:
				//TRACE(" PD_DATA_ALERT\n");
				recv_unext_message(PD_CTRL_GET_SINK_CAP_EXTENDED,para+1,para_len-1);
				break;
			default:
				TRACE(" Unknown pd msg\n");
				ret = 1;
				break;
			
			}
	}

	return ret;
}
#endif
#if 1 // debug
unsigned char recv_debug_callback(unsigned char type, unsigned char *para, unsigned char para_len)
{
	unsigned int addr;
	unsigned char mem_type;
	unsigned char len;
  mem_type = para[0];
  switch(type)
	{
	  	case TYPE_GET_VAR:
			TRACE(" Recv: TYPE_GET_VAR\n");
			addr = *(unsigned int *)(&para[1]);
			len = para[3];
			para_len = para_len;
			if(mem_type == 0) // idata
			{
				TRACE1("read 0x%x idata = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == 1) // xdata
			{
				TRACE1("read 0x%x xdata = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_fw_var_reg) // REG_FW_VAR
			{
				TRACE1("read REG_FW_VAR[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_pd_src_pdo)
			{
				TRACE1("read pd_src_pdo[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_pd_snk_pdo)
			{
				TRACE1("read pd_snk_pdo[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_pd_rdo_bak)
			{
				TRACE1("read pd_rdo_bak[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_pd_rdo)
			{
				TRACE1("read pd_rdo[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_DP_caps)
			{
				TRACE1("read DP_cap[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_configure_DP_caps) // REG_FW_VAR
			{
				TRACE1("read configure_DP_caps[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_src_dp_status) // REG_FW_VAR
			{
				TRACE1("read src_dp_status[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_sink_svid_vdo) // REG_FW_VAR
			{
				TRACE1("read sink_svid_vdo[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else if(mem_type == IF_VAR_sink_identity) // REG_FW_VAR
			{
				TRACE1("read sink_identity[0x%x] = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			else // ?data
			{
				TRACE1("read 0x%x ?data = ",addr);
				TRACE_ARRAY(para+4,len);
			}
			break;
		default:
			break;
  	}

	return 1;
}
#endif

/**
 * @desc:   The Interface AP set the source capability to Ohio 
 *
 * @param:  pdo_buf: PDO buffer pointer of source capability,
 *                              which can be packed by PDO_FIXED_XXX macro
 *                eg: default5Vsafe src_cap(5V, 0.9A fixed) --> PDO_FIXED(5000,900, PDO_FIXED_FLAGS) 
 * 
 *                src_caps_size: source capability's size
 *           		
 * @return:  1: success 0: fail 
 *  
 */ 
unsigned char send_src_cap(const unsigned char *src_caps, unsigned char src_caps_size)
{
	//unsigned char i;
	//unsigned int little_big_endian = 0x1234;

	//TRACE("send_src_cap\n");
	if ( NULL == src_caps )
	{
		//TRACE("src cap is NULL\n");
		return CMD_FAIL;
	}
	if ((src_caps_size%PD_ONE_DATA_OBJECT_SIZE) != 0 || 
	     (src_caps_size/PD_ONE_DATA_OBJECT_SIZE) > PD_MAX_DATA_OBJECT_NUM){
	     //TRACE1("src_caps_size = %2BX\n", src_caps_size);
		return CMD_FAIL;
	}

	#if 0
	
	if ((little_big_endian&0xff) == 0x12)
	{
		TRACE("A\n");
		for (i = 0; i < src_caps_size; i++)
			pd_src_pdo[i] = src_caps[i];
	}
	else{
				TRACE("B\n");
		for (i = 0; i < src_caps_size; i+=4){
			pd_src_pdo[i] = src_caps[i+3];
			pd_src_pdo[i+1] = src_caps[i+2];
			pd_src_pdo[i+2] = src_caps[i+1];
			pd_src_pdo[i+3] = src_caps[i];

		}
	}
	#endif

	if(!is_global_xdata_avialable())
	{
		TRACE("send_src: Global XDTA is NOT Avliable.\n");
		return 0;
	}	
	
	for (global_i = 0; global_i < src_caps_size; global_i+=4)
	{
		pd_src_pdo[global_i] = src_caps[global_i+3];
		pd_src_pdo[global_i+1] = src_caps[global_i+2];
		pd_src_pdo[global_i+2] = src_caps[global_i+1];
		pd_src_pdo[global_i+3] = src_caps[global_i];

	}
	pd_src_pdo_cnt = src_caps_size/PD_ONE_DATA_OBJECT_SIZE;
	//TRACE1("pd_src_pdo_cnt = %2BX\n", pd_src_pdo_cnt);

	//TRACE_ARRAY(pd_src_pdo,   pd_src_pdo_cnt * PD_ONE_DATA_OBJECT_SIZE);
	//TRACE("msg_e\n");
	/*send source capabilities message to Ohio really*/
	InterfaceSendBuf[0] = src_caps_size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_PWR_SRC_CAP;
    memcpy(InterfaceSendBuf + 2, pd_src_pdo, src_caps_size);
	clear_global_xdata();

	return interface_send_msg();
	

}



//unsigned char sink_id_header[PD_ONE_DATA_OBJECT_SIZE] = {0x00, 0x00, 0x00, 0x6c};
//unsigned char sink_cert_stat_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x00, 0x00, 0x00, 0x00};
//unsigned char sink_prd_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x58, 0x01, 0x13, 0x10};
//unsigned char sink_ama_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x39, 0x00, 0x00, 0x51};

unsigned char send_dp_snk_identity(const unsigned char *dp_snk_identity, unsigned char dp_snk_identity_size)
{
	//unsigned char  tmp[32] = { 0} ;
   	//memcpy(tmp, sink_id_header, 4);
   	//memcpy(tmp + 4, sink_cert_stat_vdo, 4);
   	//memcpy(tmp+ 8, sink_prd_vdo, 4);
   	//memcpy(tmp + 12, sink_ama_vdo, 4);
    InterfaceSendBuf[0] = dp_snk_identity_size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_DP_SNK_IDENDTITY;
    memcpy(InterfaceSendBuf + 2, dp_snk_identity, dp_snk_identity_size);
   	return interface_send_msg();
}
/**
 * @desc:   Interface that AP send(configure) the sink capability to Ohio's downstream device 
 *
 * @param:  snk_caps: PDO buffer pointer of sink capability 
 *                          
 *                snk_caps_size: sink capability length
 *           		
 * @return:  sink capability array length>0: success.  0: fail 
 *  
 */ 
unsigned char send_snk_cap( unsigned char *snk_caps, unsigned char snk_caps_size){
	//unsigned char i;
	//unsigned char c0,c1;
	//unsigned int little_big_endian = 0x1234;
	//unsigned char pd_snk_pdo[VDO_SIZE];
	//unsigned char pd_snk_pdo_cnt ;	
	
	if ( NULL == snk_caps )
		return 0;
	
	if ((snk_caps_size%PD_ONE_DATA_OBJECT_SIZE) != 0 || 
	     (snk_caps_size/PD_ONE_DATA_OBJECT_SIZE) > PD_MAX_DATA_OBJECT_NUM){
		return 0;
	}
	
	/*if ((little_big_endian&0xff) == 0x12){
		for (i = 0; i < snk_caps_size; i++)
			pd_snk_pdo[i] = snk_caps[i];
	}
	else{
		for (i = 0; i < snk_caps_size; i+=4){
			pd_snk_pdo[i] = snk_caps[i+3];
			pd_snk_pdo[i+1] = snk_caps[i+2];
			pd_snk_pdo[i+2] = snk_caps[i+1];
			pd_snk_pdo[i+3] = snk_caps[i];

		}
	}
	pd_snk_pdo_cnt = snk_caps_size/PD_ONE_DATA_OBJECT_SIZE;
	
	//configure sink cap
	return     interface_send_msg(pd_snk_pdo, pd_snk_pdo_cnt * 4, TYPE_PWR_SNK_CAP );
	*/
	if(!is_global_xdata_avialable())
	{
		TRACE("send_snk_cap:Global XDTA is NOT Avliable.\n");
		return 0;
	}	
	for (global_i = 0; global_i < snk_caps_size; global_i+=4)
	{
		global_c0= snk_caps[global_i];
		global_c1 = snk_caps[global_i+1];
		snk_caps[global_i] = snk_caps[global_i+3];
		snk_caps[global_i+1] = snk_caps[global_i+2];
		snk_caps[global_i + 2] = global_c1;
		snk_caps[global_i + 3] = global_c0;
	}
	clear_global_xdata();
	InterfaceSendBuf[0] = snk_caps_size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_PWR_SNK_CAP;
    memcpy(InterfaceSendBuf + 2, snk_caps, snk_caps_size);
	return interface_send_msg();
	

}



/**
 * @desc:   Interface that AP send(configure) the DP's sink capability to Ohio's downstream device 
 *
 * @param:  dp_snk_caps: PDO buffer pointer of DP sink capability 
 *                          
 *                dp_snk_caps_size: DP sink capability length
 *           		
 * @return:  1: success.  0: fail 
 *  
 */ 
unsigned char send_dp_snk_cfg(const unsigned char *dp_snk_caps, unsigned char dp_snk_caps_size){
	//unsigned char i;
	//unsigned char configure_DP_caps[PD_ONE_DATA_OBJECT_SIZE];
	unsigned char code DP_caps[PD_ONE_DATA_OBJECT_SIZE] = {0, 0, 0, 0};

	if ( NULL == dp_snk_caps )
		return CMD_FAIL;
	
	if ((dp_snk_caps_size%PD_ONE_DATA_OBJECT_SIZE) != 0 || 
	     (dp_snk_caps_size/PD_ONE_DATA_OBJECT_SIZE) > PD_MAX_DATA_OBJECT_NUM){
		return CMD_FAIL;
	}
	
	//for (i = 0; i < dp_snk_caps_size; i++)
	//	configure_DP_caps[i] = *dp_snk_caps++;
	//memcpy(InterfaceSendBuf + 2, configure_DP_caps, 4);
	

	memcpy(InterfaceSendBuf + 2, dp_snk_caps, 4);	
	
	memcpy(InterfaceSendBuf + 2 + 4, DP_caps, 4);

	InterfaceSendBuf[0] = 4 + 4 + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_DP_SNK_CFG;
	return interface_send_msg();
}


unsigned char send_svid(const unsigned char *svid, unsigned char size)
{
	if(NULL == svid)
		return CMD_FAIL;
	if(size%4 !=0)
		return CMD_FAIL;
	InterfaceSendBuf[0] = size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_SVID;
    memcpy(InterfaceSendBuf + 2, svid, size);
	return	interface_send_msg();
}




/**
 * @desc:   Interface that AP send(configure) the sink capability to Ohio's downstream device 
 *
 * @param:  snk_caps: PDO buffer pointer of sink capability 
 *                          
 *                snk_caps_size: sink capability length
 *           		
 * @return:  1: success.  0: fail 
 *  
 */ 
unsigned char send_rdo(const unsigned char *rdo, unsigned char size)
{
//	unsigned char i;

	if (NULL == rdo )
		return CMD_FAIL;
	
	if ((size%PD_ONE_DATA_OBJECT_SIZE) != 0 || 
	     (size/PD_ONE_DATA_OBJECT_SIZE) > PD_MAX_DATA_OBJECT_NUM){
		return CMD_FAIL;
	}
	
	//for (i = 0; i < size; i++)
	//	pd_rdo[i] = *rdo++;

	
	//send rdo
	InterfaceSendBuf[0] = size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_PWR_OBJ_REQ;
    memcpy(InterfaceSendBuf + 2, rdo, size);
	return interface_send_msg();

}

#if 1 // debug
unsigned char send_get_var(unsigned char type, unsigned int addr, unsigned char len)
{
	InterfaceSendBuf[0] = 4 + 1;
    InterfaceSendBuf[1] = TYPE_GET_VAR;
    InterfaceSendBuf[2] = type;
    *(unsigned int *)(&InterfaceSendBuf[3]) = addr;
    InterfaceSendBuf[5] = len;
	return interface_send_msg();
}

unsigned char send_set_var(unsigned char type, unsigned int addr, unsigned char *buf, unsigned char len)
{
	InterfaceSendBuf[0] = 4 + len + 1;
    InterfaceSendBuf[1] = TYPE_SET_VAR;
    InterfaceSendBuf[2] = type;
    *(unsigned int *)(&InterfaceSendBuf[3]) = addr;
	InterfaceSendBuf[5] = len;
	memcpy(&InterfaceSendBuf[6], buf, len);
	return interface_send_msg();
}
#endif
#ifdef USE_PD30

char ConvertPd3TypeToInterfaceType(unsigned char ext, unsigned char pd3_type, unsigned char *type)
{ 
	if(ext)
	{
		switch(pd3_type)
		{
			case PD_EXT_SOURCE_CAP:
				*type = TYPE_EXT_SRC_CAP;
				break;
			case PD_EXT_STATUS:
				*type = TYPE_EXT_SRC_STS;
				break;
			case PD_EXT_GET_BATTERY_CAP:
				*type = TYPE_EXT_GET_BATT_CAP;
				break;
			case PD_EXT_GET_BATTERY_STATUS:
				*type = TYPE_EXT_GET_BATT_STS;
				break;
			case PD_EXT_BATTERY_CAP:
				*type = TYPE_EXT_BATT_CAP;
				break;
			case PD_EXT_GET_MANUFACTURER_INFO:
				*type = TYPE_EXT_GET_MFR_INFO;
				break;
			case PD_EXT_MANUFACTURER_INFO:
				*type = TYPE_EXT_MFR_INFO;
				break;
			case PD_EXT_SECURITY_REQUEST:
			case PD_EXT_SECURITY_RESPONSE:
				// not support
				pd3_type = 0;
				break;
			case PD_EXT_FW_UPDATE_REQUEST:
				*type = TYPE_EXT_PDFU_REQUEST;
				break;
			case PD_EXT_FW_UPDATE_RESPONSE:
				*type = TYPE_EXT_PDFU_RESPONSE;
				break;
			case PD_EXT_SINK_CAP:
				*type = TYPE_EXT_SINK_CAP_EXT;
				break;
			default:
				pd3_type = 0;
				break;
		}
	}
	else
	{
		switch(pd3_type)
		{
			case PD_DATA_BATTERY_STATUS:
				*type = TYPE_EXT_BATT_STS;
				break;
			case PD_DATA_ALERT:
				*type = TYPE_EXT_ALERT;
				break;

			case PD_CTRL_NOT_SUPPORTED:
				*type = TYPE_EXT_NOT_SUPPORTED;
				break;
			case PD_CTRL_GET_SOURCE_CAP_EXTENDED:
				*type = TYPE_EXT_GET_SRC_CAP;
				break;
			case PD_CTRL_GET_STATUS:
				*type = TYPE_EXT_GET_SRC_STS;
				break;
			case PD_CTRL_FR_SWAP:
				*type = TYPE_EXT_FR_SWAP;
				break;
			case PD_CTRL_GET_SINK_CAP_EXTENDED:
				*type = TYPE_GET_SINK_CAP_EXT;
				break;
			default:
				pd3_type = 0;
				break;
		}
	}
	if(pd3_type)
		return 0;
	else
		return -1;
}

unsigned char send_ext_msg(unsigned char is_ext, unsigned char type, unsigned char *pbuf, unsigned char buf_len,unsigned char type_sop)
{
	if(ConvertPd3TypeToInterfaceType(is_ext, type, &type) < 0)
		return CMD_FAIL;

	InterfaceSendBuf[0] = buf_len + 1 + 1;
    InterfaceSendBuf[1] = type;
    InterfaceSendBuf[2] = type_sop;
    memcpy(InterfaceSendBuf + 3, pbuf, buf_len);
	return interface_send_msg();
}
#endif

#ifndef DEL_UNUSE_FEATURE	
/**
 * @desc:   The Interface AP set the VDM packet to Ohio 
 *
 * @param:  vdm:  object buffer pointer of VDM,
 *                              
 *                
 * 
 *                size: vdm packet size
 *           		
 * @return:  0: success 1: fail 
 *  
 */ 
unsigned char send_vdm(const unsigned char *vdm, unsigned char size)
{
	if ( NULL == vdm )
		return CMD_FAIL;
	if((size<4) || (size>28) || (size%4 != 0))
		return CMD_FAIL;

	InterfaceSendBuf[0] = size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_VDM;
    memcpy(InterfaceSendBuf + 2, vdm, size);
	return interface_send_msg();//updated to fix OHO-423
}
unsigned char send_sop_prime(const unsigned char *sop_prime, unsigned char size)
{
	if (NULL == sop_prime)
		return CMD_FAIL;
	

	if ((size%PD_ONE_DATA_OBJECT_SIZE) != 0 || 
     (size/PD_ONE_DATA_OBJECT_SIZE) > PD_MAX_DATA_OBJECT_NUM || size < PD_ONE_DATA_OBJECT_SIZE)
    {
		return CMD_FAIL;
	}
	
	InterfaceSendBuf[0] = size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_SOP_PRIME;
    memcpy(InterfaceSendBuf + 2, sop_prime, size);

	return interface_send_msg();
	
}
unsigned char send_sop_double_prime(const unsigned char *sop_double_prime, unsigned char size)
{
	if (NULL == sop_double_prime )
		return CMD_FAIL;
	

	if ((size%PD_ONE_DATA_OBJECT_SIZE) != 0 || 
     (size/PD_ONE_DATA_OBJECT_SIZE) > PD_MAX_DATA_OBJECT_NUM || size < PD_ONE_DATA_OBJECT_SIZE)
    {
		return CMD_FAIL;
	}

	InterfaceSendBuf[0] = size + 1; // + cmd
    InterfaceSendBuf[1] = TYPE_SOP_DOUBLE_PRIME;
    memcpy(InterfaceSendBuf + 2, sop_double_prime, size);
	
	return 	interface_send_msg();
	
}
/**
 * @desc:   The interface AP will send DR_Swap command to Ohio 
 *
 * @param:  none
 *           		
 * @return:  1: success.  0: fail 
 *  
 */
unsigned char send_data_swap(void)
{
	interface_dr_swap();
}

unsigned char send_gotomin()
{
	interface_send_gotomin();
}
#endif
/**
 * @desc:   The interface AP will send  PR_Swap command to Ohio 
 *
 * @param:  none
 *           		
 * @return:  1: success.  0: fail 
 *  
 */
unsigned char send_power_swap(void)
{
	// return interface_send_ctr_msg(TYPE_PSWAP_REQ);
	 interface_pr_swap();	
}




/**
 * @desc:   The interface AP will send DR_Swap command to Ohio 
 *
 * @param:  none
 *           		
 * @return:  1: success.  0: fail 
 *  
 */
unsigned char send_vconn_swap(void)
{
	interface_vconn_swap();
}




unsigned char send_soft_rst()
{
	interface_send_soft_rst();
}

unsigned char send_hard_rst()
{
	 interface_send_hard_rst();
}
unsigned char send_accept()
{
	interface_send_accept();
}
unsigned char  send_reject()
{
	interface_send_reject();
}

unsigned char  send_get_pd_status()
{
	interface_get_pd_status();
}

unsigned char send_get_sink_cap()
{
	interface_send_get_sink_cap();
}

unsigned char send_get_dp_snk_cap()
{
	interface_send_get_dp_snk_cap();
}

unsigned char send_frswap_signal()
{
	interface_send_frswap_signal();
}


char *interface_to_str(unsigned char header_type)
{
return  (header_type == TYPE_PWR_SRC_CAP) ? "PWR_SRC_CAP" :
        (header_type == TYPE_PWR_SNK_CAP) ? "PWR_SNK_CAP" :
        (header_type == TYPE_PWR_OBJ_REQ) ? "PWR_OBJ_REQ" :
        (header_type == TYPE_DP_SNK_IDENDTITY) ? "DP_SNK_IDENDTITY" :
        (header_type == TYPE_SVID) ? "SVID" :
        (header_type == TYPE_PSWAP_REQ) ? "PSWAP_REQ" : 
        (header_type == TYPE_DSWAP_REQ) ? "DSWAP_REQ" : 
        (header_type == TYPE_GOTO_MIN_REQ) ? "GOTO_MIN_REQ" : 
		(header_type == TYPE_DP_ALT_ENTER) ? "DPALT_ENTER" : 
		(header_type == TYPE_DP_ALT_EXIT) ? "DPALT_EXIT" : 
		(header_type == TYPE_GET_SNK_CAP) ? "GET_SNK_CAP" : 
        (header_type == TYPE_VCONN_SWAP_REQ) ? "VCONN_SWAP_REQ" : 
        (header_type == TYPE_GET_DP_SNK_CAP) ? "GET_DP_SINK_CAP" :
        (header_type == TYPE_DP_SNK_CFG) ? "DP_SNK_CFG" :          
        (header_type == TYPE_SOFT_RST) ? "Software Reset" :          
        (header_type == TYPE_HARD_RST) ? "Hardware Reset" :
        (header_type == TYPE_RESTART) ? "Restart" :
        (header_type == TYPE_PD_STATUS_REQ) ? "PD_STATUS_REQ" :
        (header_type == TYPE_ACCEPT) ? "ACCEPT" :
        (header_type == TYPE_REJECT) ? "REJECT" :
        (header_type == TYPE_VDM) ? "VDM" :
        (header_type == TYPE_RESPONSE_TO_REQ) ? "RESPONSE_TO_REQ" :	
        (header_type == TYPE_SOP_PRIME) ? "SOP'" :	
        (header_type == TYPE_SOP_DOUBLE_PRIME) ? "SOP\"" :	
#ifdef USE_PD30
		(header_type == TYPE_EXT_SRC_CAP) ? "PD3_SRC_CAP" :	
		(header_type == TYPE_EXT_SRC_STS) ? "PD3_SRC_STS" : 
		(header_type == TYPE_EXT_GET_BATT_CAP) ? "PD3_GET_BATT_CAP" : 
		(header_type == TYPE_EXT_GET_BATT_STS) ? "PD3_GET_BATT_STS" : 
		(header_type == TYPE_EXT_BATT_CAP) ? "PD3_BATT_CAP" : 
		(header_type == TYPE_EXT_GET_MFR_INFO) ? "PD3_GET_MFR_INFO" : 
		(header_type == TYPE_EXT_MFR_INFO) ? "PD3_MFR_INFO" : 
		(header_type == TYPE_EXT_PDFU_REQUEST) ? "PD3_PDFU_REQUEST" : 
		(header_type == TYPE_EXT_PDFU_RESPONSE) ? "PD3_PDFU_RESPONSE" : 
		(header_type == TYPE_EXT_BATT_STS) ? "PD3_BATT_STS" : 
		(header_type == TYPE_EXT_ALERT) ? "PD3_ALERT" : 
		(header_type == TYPE_EXT_NOT_SUPPORTED) ? "PD3_NOT_SUPPORTED" : 
		(header_type == TYPE_EXT_GET_SRC_CAP) ? "PD3_GET_SRC_CAP" : 
		(header_type == TYPE_EXT_GET_SRC_STS) ? "PD3_GET_SRC_STS" : 
		(header_type == TYPE_EXT_FR_SWAP) ? "PD3_FR_SWAP" : 
		(header_type == TYPE_FR_SWAP_SIGNAL) ? "PD3_FR_SWAP_SIGNAL" : 
#endif
        "Unknown";
}

char *result_to_str(unsigned result_type)
{
	return  (result_type == CMD_SUCCESS) ? "Accept" :
        (result_type == CMD_FAIL) ? "Fail" :
        (result_type == CMD_BUSY) ? "Busy" :
        (result_type == CMD_REJECT) ? "Reject":        	
        "Unknown";
}

/**
 * @desc:   send initalized setting
 *
 * @param:  src_flag: 1 or 0 (1: PD_CURRENT_1500MA, 0:PD_CURRENT_100MA)
 *           		
 * @return:  none
 *  
 */
void send_initialized_setting(bit src_flag)
{
	unsigned long code init_src_caps[1] = { 
	   	/*5V, 0.9A, Fixed*/
	   	//PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_900MA, PDO_FIXED_FLAGS),
	   	/*5V, 1.5A, Fixed*/
	   	PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_1500MA, PDO_FIXED_FLAGS)
	   #if 0
	 	/*min 5V, max 20V, power 15W, battery*/
	    	PDO_BATT(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_POWER_15W)
		/*min5V, max 5V, current 3A, variable*/ 
	    	PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_CURRENT_3A)
	    #endif
	};
	unsigned long code init_src_caps_100mA[1] =  {
		PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_100MA, PDO_FIXED_FLAGS)
		};
#ifndef DEL_UNUSE_FEATURE
	//init setting for TYPE_PWR_SNK_CAP
	unsigned long init_snk_cap[3] = { 
	   	/*5V, 0.9A, Fixed*/
	   	PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_900MA, PDO_FIXED_FLAGS ),
	 	/*min 5V, max 21V, power 15W, battery*/
	    	PDO_BATT(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_21V, PD_POWER_15W),
		/*min5V, max 21V, current 3A, variable*/ 
	    	PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_21V, PD_CURRENT_3A)
	};
#else
	//In order to optimal the RAM size, fill the known sink capability data directly.
	unsigned long code init_snk_cap[3] = { 
	   	/*5V, 0.9A, Fixed*/
	   	0x5A90012A,//PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_900MA, PDO_FIXED_FLAGS ),
	 	/*min 5V, max 21V, power 15W, battery*/
	    0x3C90415A,//PDO_BATT(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_21V, PD_POWER_15W),
		/*min5V, max 21V, current 3A, variable*/ 
	    0x2C91419A//PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_21V, PD_CURRENT_3A)
	};
#endif
	//init setting for TYPE_DP_SNK_CFG
	//unsigned char init_dp_snk_cfg[PD_ONE_DATA_OBJECT_SIZE] = { 0x06, 0x08,0x08, 0x00 };
	//init setting for TYPE_SVID
	unsigned char code init_svid[PD_ONE_DATA_OBJECT_SIZE]= {0x00, 0x00, 0x01, 0xff};

#ifndef DEL_UNUSE_FEATURE
	//init setting for TYPE_DP_SNK_IDENDTITY
	unsigned char init_dp_snk_identity[12];// = {0};   //MIS2-286 TD.PD.VDMU.E1 
	unsigned char code init_sink_id_header[PD_ONE_DATA_OBJECT_SIZE] = {0x00,0x00, 0x00, 0x90}; //{0x00,0x00, 0x00, 0x10};
	unsigned char code init_sink_cert_stat_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x00, 0x00, 0x00, 0x00};
	unsigned char code init_sink_prd_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x00, 0x00, 0x00, 0x00};
	unsigned char code init_sink_ama_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x00, 0x00, 0x00, 0x00};//{0x39, 0x00, 0x00, 0x51};	

	memcpy(init_dp_snk_identity, init_sink_id_header, 4);
	memcpy(init_dp_snk_identity + 4, init_sink_cert_stat_vdo, 4);
	memcpy(init_dp_snk_identity+ 8, init_sink_prd_vdo, 4);
	//memcpy(init_dp_snk_identity + 12, init_sink_ama_vdo, 4);	
#else
	//In order to optimal the RAM size, fill the known sink identity data directly.
	unsigned char code init_dp_snk_identity[16] = 
	{
		0x00, 0x00, 0x00, 0x2c,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x39, 0x00, 0x00, 0x51
	};
#endif
	//send TYPE_PWR_SRC_CAP init setting
/*	if(src_flag)
		send_pd_msg(TYPE_PWR_SRC_CAP, (const char *)init_src_caps, sizeof(init_src_caps));
	else//OHO-229, When do the PD CTS test by EVB, set switch S1_5 to ¡°ON¡± , then API will send source capability with current = 100mA.
		send_pd_msg(TYPE_PWR_SRC_CAP, (const char *)init_src_caps_100mA, sizeof(init_src_caps_100mA));
	//send TYPE_PWR_SNK_CAP init setting		
	send_pd_msg(TYPE_PWR_SNK_CAP, (const char *)init_snk_cap, sizeof(init_snk_cap));
*/
	//send TYPE_DP_SNK_IDENDTITY init setting
	send_pd_msg(TYPE_DP_SNK_IDENDTITY, init_dp_snk_identity, sizeof(init_dp_snk_identity));
	//send TYPE_DP_SNK_CFG init setting		
	//send_pd_msg(TYPE_DP_SNK_CFG, init_dp_snk_cfg, sizeof(init_dp_snk_cfg));
	//send TYPE_SVID init setting
//	send_pd_msg(TYPE_SVID, init_svid, sizeof(init_svid));		

}
 

void interface_init(void)
{
	sending_len = 0;

	need_check_dswap_response = 0;
	need_check_pswap_response = 0;
	need_check_vswap_response = 0;
	need_check_gotomin_response = 0;
	need_check_rdo_response = 0;

	vswap_response_got = 0;
	pswap_response_got = 0;
	gotomin_response_got = 0;
	dswap_response_got = 0;
	rdo_response_got= 0;

	recv_msg_success = 0;
	InterfaceSendRetryCount = 10;
}

#endif // test

void interface_recvd_msg(void)
{
    unsigned char i;
    unsigned char checksum;
	unsigned char idata ReadDataBuf[32];

	ReadBlockReg(Interface_Base_Addr, InterfaceRecvBuf_Addr, 32, (unsigned char *)ReadDataBuf);

	if (ReadDataBuf[0]!=0) {
		TRACE("->RecvMSG ");
		memcpy(InterfaceRecvBuf,ReadDataBuf,ReadDataBuf[0]+2);

		WriteReg(Interface_Base_Addr,InterfaceRecvBuf_Addr,0);
		//TRACE_ARRAY(InterfaceRecvBuf,ReadDataBuf[0]+2);
		checksum = 0;
		for(i = 0; i < InterfaceRecvBuf[0] + 2; i++) {
			checksum += InterfaceRecvBuf[i];
		}
		if(checksum == 0) {
			recv_msg_success = 1;
			TRACE1("0x%bx\n",InterfaceRecvBuf[1]);
		} else {
			TRACE_ARRAY(InterfaceRecvBuf,ReadDataBuf[0]+2);
			TRACE("checksum error: \n");
		}
	}

}

unsigned char cac_checksum(unsigned char  *pSendBuf, unsigned char len)
{
    unsigned char i;
    unsigned char checksum;
    checksum = 0;
    for(i = 0; i < len; i++) {
        checksum += *(pSendBuf + i);
    }
    //TRACE1("%x \n", (u16)checksum);
    return (0 - checksum);
}

unsigned char interface_send_msg(void)
{
    unsigned char c;
    unsigned char idata WriteDataBuf[32];

    // send data is prepare in InterfaceSendBuf[]
    InterfaceSendBuf[InterfaceSendBuf[0] + 1] = cac_checksum(InterfaceSendBuf, InterfaceSendBuf[0]+1); //cmd + checksum
    sending_len = InterfaceSendBuf[0] + 2;
	TRACE1("<-SendMSG 0x%bx\n",InterfaceSendBuf[1]);
	//TRACE_ARRAY(InterfaceSendBuf, sending_len);
    ReadReg(Interface_Base_Addr, InterfaceSendBuf_Addr,&c);
	// retry
	if(InterfaceSendRetryCount && c) {
	    unsigned char count = InterfaceSendRetryCount;
		while(count) { 
		   delay_ms(1);
		   ReadReg(Interface_Base_Addr, InterfaceSendBuf_Addr,&c);
		   if (c == 0)
		   	break;
		   count--;
		}
	}
    if (c == 0) {
        memcpy(WriteDataBuf,InterfaceSendBuf,sending_len);
        WriteBlockReg(Interface_Base_Addr, InterfaceSendBuf_Addr+1 , sending_len-1, &WriteDataBuf[1]);
        WriteReg(Interface_Base_Addr,InterfaceSendBuf_Addr, WriteDataBuf[0]);
    } else {
        TRACE("Tx Buf Full\n");
		return CMD_FAIL;
    }

    return CMD_SUCCESS;
}

unsigned char interface_send_ctr_msg(void)
{
    unsigned char c;
    unsigned char idata WriteDataBuf[3];

    InterfaceSendBuf[0] = 1; // + cmd
    //InterfaceSendBuf[1] = type;
    InterfaceSendBuf[2] = 0-(InterfaceSendBuf[0]+InterfaceSendBuf[1]);//cac_checksum(InterfaceSendBuf, 1 + 1); //cmd + checksum
    sending_len = 3;
	TRACE1("<-SendMSG 0x%bx\n",InterfaceSendBuf[1]);
	//TRACE_ARRAY(InterfaceSendBuf, sending_len);
    ReadReg(Interface_Base_Addr, InterfaceSendBuf_Addr,&c);
	// retry
	if(InterfaceSendRetryCount && c) {
	    unsigned char count = InterfaceSendRetryCount;
		while(count) { 
		   delay_ms(1);
		   ReadReg(Interface_Base_Addr, InterfaceSendBuf_Addr,&c);
		   if (c == 0)
		   	break;
		   count--;
		}
	}
    if (c == 0) {
        memcpy(WriteDataBuf,InterfaceSendBuf,sending_len);
        WriteBlockReg(Interface_Base_Addr, InterfaceSendBuf_Addr+1 , sending_len-1, &WriteDataBuf[1]);
        WriteReg(Interface_Base_Addr,InterfaceSendBuf_Addr, WriteDataBuf[0]);
    } else {
        TRACE("Tx Buf Full\n");
		return CMD_FAIL;
    }

    return CMD_SUCCESS;
}

