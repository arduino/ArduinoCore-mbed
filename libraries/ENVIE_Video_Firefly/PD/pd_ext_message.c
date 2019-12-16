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

#ifdef USE_PD30

#include "private_interface.h"
#include "public_interface.h"
#include "pd_ext_message.h"
#include "config.h"
#include <STRING.H>
//#include "debug.h"

#define DEBUG_MSG 0 

#define bool bit
typedef unsigned char u8;

#ifdef USE_POWER_RULE

// Update Source Power Rules
unsigned int pd30_src_pwr_rules_ma(unsigned char watts,unsigned char voltage)
{
	unsigned int current_ma = 900;

	switch(voltage)
	{
		case 5: // 5V	
		if(watts >= 1 && watts <= 30) // 0.5 <= x <= 15
		{
		   current_ma = (unsigned long)1000 * watts / 10; // x/5
		}
		else if(watts > 30 && watts <= 200) // 15 < x <= 100
		{
			current_ma = 3000; // 3A
		}
		break;
		case 9: // 9V	
		if(watts >= 1 && watts <= 30) // 0.5 <= x <= 15
		{
		   // not required
		}
		else if(watts > 30 && watts <= 54) // 15 < x <= 27
		{
		   current_ma = (unsigned long)1000 * watts / 18; // x/9
		}
		else if(watts >54 && watts <= 200) // 27 < x <= 100
		{
			current_ma = 3000; // 3A
		}
		break;
		case 15: // 15V 
		if(watts >= 1 && watts <= 54) // 0.5 <= x <= 27
		{
		   // not required
		}
		else if(watts > 54 && watts <= 90) // 27 < x <= 45
		{
		   current_ma = (unsigned long)1000 * watts / 30; // x/15
		}
		else if(watts >90 && watts <= 200) // 45 < x <= 100
		{
			current_ma = 3000; // 3A
		}
		break;
		case 20: // 20V 
		if(watts >= 1 && watts <= 90) // 0.5 <= x <= 45
		{
		   // not required
		}
		else if(watts > 90 && watts <= 200) // 45 < x <= 100
		{
		   current_ma = (unsigned long)1000 * watts / 40; // x/20
		}
		break;
		default:
			current_ma = 0;
			break;
	}
	return current_ma;
}

#endif

#define CHUNK_SIZE 26
#define CHUNK_WAIT_REQUEST_TIMEOUT (20<<1) // 80ms

unsigned int PD_EXT_RAM PDExtSend_timer=0;
//unsigned int chunk_send_delay=0;

static unsigned char PD_EXT_RAM ChunkSize;
static unsigned char PD_EXT_RAM ChunkNumber;
static unsigned char PD_EXT_RAM ChunkTotal;
static unsigned char PD_EXT_RAM ChunkRequestWaiting;
static unsigned char PD_EXT_RAM SendBlockType;
static unsigned char PD_EXT_RAM *SendBlockDataBuf;
static unsigned int PD_EXT_RAM SendBlockDataSize;
static unsigned char PD_EXT_RAM SendBlockState = SEND_BLOCK_IDLE;

unsigned char PD_EXT_RAM pd_block_recv_type;
unsigned int PD_EXT_RAM pd_block_recv_len = 0;
unsigned char PD_EXT_RAM pd_block_recv_state = RECV_BLOCK_IDLE;
unsigned char PD_EXT_RAM pd_block_recv_buf[260];
static unsigned char PD_EXT_RAM chunk_buf[30];

// Battery Cap
struct battery_cap code pd_battery_cap[8] = {
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 0 }, // Battery 0 
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 1 // Invalid Battery reference, Battery's cap unknown
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 2 }, // Battery 2 // Hot Swappable Battery, Battery's cap unknown
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 0 }, // Battery 3
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 0 }, // Battery 4
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 0 }, // Battery 5
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 0 }, // Battery 6
	{ TEST_VENDOR_ID, TEST_PRODUCT_ID, 0xFFFF, 0xFFFF, 0 }, // Battery 7
};
struct battery_cap recv_battery_cap;

// Battery Status
struct battery_status_data_obj code pd_battery_status[8] = {
	{ 0, 0, 0xFFFF }, // Battery 0 
	{ 0, 1, 0xFFFF }, // Battery 1 // Reserved, Invalid Battery reference, Battery'sSOC unknown
	{ 0, 2, 0xFFFF }, // Battery 2 // Reserved, Battery charging, Battery'sSOC unknown
	{ 0, 0, 0xFFFF }, // Battery 3
	{ 0, 0, 0xFFFF }, // Battery 4
	{ 0, 0, 0xFFFF }, // Battery 5
	{ 0, 0, 0xFFFF }, // Battery 6
	{ 0, 0, 0xFFFF }, // Battery 7
};
struct battery_status_data_obj PD_EXT_RAM recv_battery_status;


// Alert Message
struct alert_data_obj code pd_alert_data_obj = {
	0, // Reserved
	0, // Hot Swappable Battery 0~3
	1, // Fixed Battery 0~3
	2  // Battery Status Change
};
struct alert_data_obj PD_EXT_RAM recv_alert_data_obj;


// Get Battery Status
unsigned char PD_EXT_RAM pd_get_battery_status_ref = 4; // hot swap battery 0

// Source Capabilities Extended
struct source_cap_extended code pd_source_cap_extended = {
	TEST_VENDOR_ID, // VID
	TEST_PRODUCT_ID, // PID
	0, // XID
	TEST_FW_MAJOR_REV, // FW Ver
	TEST_HW_REV, // HW Ver
	// ....
};
struct source_cap_extended PD_EXT_RAM recv_source_cap_extended;

// Status Message
struct status_data PD_EXT_RAM pd_status_data = {
	0, // not supported
	2, // External DC power
	0  // no battery
};
struct status_data PD_EXT_RAM recv_status_data;

// Get Battery Cap
unsigned char PD_EXT_RAM pd_get_battery_cap_ref = 4; // hot swap battery 0

// Manufacturer Info
struct manufacturer_info PD_EXT_RAM pd_manufacturer_info = {
	0, // Port/Cable
	0  // Not battery
};

// Manufacturer Info Data
struct manufacturer_info_data code pd_manufacturer_info_data = {
	TEST_VENDOR_ID, // VID
	TEST_PRODUCT_ID, // PID
	"Analogix" // String
};
struct manufacturer_info_data PD_EXT_RAM recv_manufacturer_info_data;
	

unsigned char SendBlock(unsigned char type, unsigned char *pData,unsigned int BlockSize)
{
	unsigned char buf_len;
	unsigned char ret;

	if(SendBlockState == SEND_BLOCK_IDLE)
	{
	    if(pData && BlockSize)
    	{
   			SendBlockType = type;
			SendBlockDataBuf = (unsigned char PD_EXT_RAM *)pData;
			SendBlockDataSize = BlockSize;
			ChunkTotal = SendBlockDataSize/CHUNK_SIZE + ((SendBlockDataSize%CHUNK_SIZE) ? 1:0);
			ChunkNumber = 0;
			ChunkRequestWaiting = 0;
			SendBlockState = SEND_BLOCK_CHUNKING;
    	}
	}

	if(SendBlockState == SEND_BLOCK_CHUNKING)
	{
		if(ChunkNumber<ChunkTotal)
		{
			if(ChunkRequestWaiting == 1)
			{
				if(!PDExtSend_timer)
					SendBlockState = SEND_BLOCK_TIMEOUT;
			}
			else	
			{
				if((ChunkNumber == ChunkTotal-1) && (SendBlockDataSize%CHUNK_SIZE != 0))
					ChunkSize = (SendBlockDataSize%CHUNK_SIZE);
				else
					ChunkSize = CHUNK_SIZE;

				USB_PD_EXT_HEADER((chunk_buf),SendBlockDataSize,0,ChunkNumber,1);	 
				if(ChunkSize)
					memcpy(chunk_buf + USB_PD_EXT_HEADER_SIZE, SendBlockDataBuf + ChunkNumber*CHUNK_SIZE,ChunkSize);
				buf_len = USB_PD_EXT_HEADER_SIZE + ChunkSize;

				if(buf_len&0x03)
				{
					unsigned char delta = 4 - (buf_len&0x03);
					memset(chunk_buf+buf_len,0,delta);
					buf_len += delta;
				}
				#if DEBUG_MSG
				TRACE_ARRAY(chunk_buf,buf_len);  
				#endif
				if(buf_len)
				{
					ret = send_ext_msg(1, SendBlockType, chunk_buf, buf_len, 0/*type_sop*/);

					if(ChunkNumber == ChunkTotal-1)
						SendBlockState = SEND_BLOCK_FINISH;
					else
						{
						ChunkRequestWaiting = 1; // wait chunk request
						PDExtSend_timer = CHUNK_WAIT_REQUEST_TIMEOUT;
						}
				}
			}
		}
	}
	else if(SendBlockState == SEND_BLOCK_FINISH)
	{
		// do somethig, if need
		SendBlockState = SEND_BLOCK_IDLE;
	}
	else if(SendBlockState == SEND_BLOCK_TIMEOUT)
	{
		// do somethig, if need
		SendBlockState = SEND_BLOCK_IDLE;
	}

	return SendBlockState;
}

unsigned char RecvBlock(unsigned char type, unsigned char *pData, unsigned char para_len)
{
	//unsigned char ret = RECV_BLOCK_IDLE;
	unsigned char ChunkNum;
	unsigned int word = *(unsigned int *)pData;
	struct ext_message_header *pExtHeader;
	BYTE_SWAP(word);
	pExtHeader = (struct ext_message_header *)&word;

#if DEBUG_MSG
	TRACE1("data_siz = %d\n",(unsigned int)(pExtHeader->data_size));
#endif
	//DBG_MSG2("data_siz = %d\n",(unsigned int)(pExtHeader->data_size));

	ChunkNum = pExtHeader->data_size/CHUNK_SIZE + ((pExtHeader->data_size%CHUNK_SIZE) ? 1:0);
	if(pExtHeader->request_chunk)
	{
#if DEBUG_MSG
		TRACE1("Get Next Chunk request (%d)\n",(unsigned int)(pExtHeader->chunk_number));
#endif
		//DBG_MSG2("Get Next Chunk request (%d)\n",(unsigned int)(pExtHeader->chunk_number));
		ChunkNumber = pExtHeader->chunk_number;
		ChunkRequestWaiting = 0;
		//pd_block_recv_state = RECV_BLOCK_CHUNK_REQUEST;
	}
	else
	{
		if(pd_block_recv_len == 0)
			{
			pd_block_recv_state = RECV_BLOCK_CHUNKING;
			pd_block_recv_type = type;
			}
		memcpy(pd_block_recv_buf+pd_block_recv_len, pData+2, para_len-2);
		#if 1 // joseph
		if(TraceDebugFlag&0x04)
			TraceArray2(pData+2, para_len-2);
		#endif
		pd_block_recv_len += para_len-2;
		
		if(pExtHeader->chunk_number+1 < ChunkNum)
		{
		// send chunk request
		USB_PD_EXT_HEADER(chunk_buf,pExtHeader->data_size,1,(pExtHeader->chunk_number+1),1);
		// Padding
		chunk_buf[2] = 0;
		chunk_buf[3] = 0;
		send_ext_msg(1, type, chunk_buf, 4, 0/*type_sop*/);
#if DEBUG_MSG
		TRACE1("Req Next Chunk (%d)\n",(unsigned int)(pExtHeader->chunk_number+1));
#endif
		//DBG_MSG2("Req Next Chunk (%d)\n",(unsigned int)(pExtHeader->chunk_number+1));
		pd_block_recv_state = RECV_BLOCK_CHUNKING;
		}
		else
		{
		unsigned char delta = (pExtHeader->data_size)%CHUNK_SIZE;
#if DEBUG_MSG
		TRACE1("Last Chunk (%d)\n",(unsigned int)(pExtHeader->chunk_number));
#endif
		//DBG_MSG2("Last Chunk (%d)\n",(unsigned int)(pExtHeader->chunk_number));
		if(delta)
		{
		    delta += 2;
			delta &= 0x03;
			if(delta)
			{
				delta = 4 - delta;
				pd_block_recv_len -= delta;
			}
		}
		pd_block_recv_state = RECV_BLOCK_FINISH;
#if DEBUG_MSG
			TRACE1("RecvBlock size = %d\n",pd_block_recv_len);
#endif
		//DBG_MSG2("RecvBlock size = %d\n",pd_block_recv_len);
		}
	}
	return pd_block_recv_state;
}

unsigned char SendBlockIdle(void)
{
  return (SendBlockState == SEND_BLOCK_IDLE);
}
void pd_ext_message_handling(void)
{
	if(!SendBlockIdle())
		SendBlock(0,0,0);
	
	#if USE_PDFU
	// PD Firmware Update
	pdfu_initiator_handling();
	pdfu_responder_handling();
	#endif

}

unsigned char send_ext_message(unsigned char type, unsigned char type_sop)
{
	unsigned char data_size;
	unsigned char buf_len = 0;
	
	switch(type)
	{
	case PD_EXT_GET_BATTERY_CAP: // request
		data_size = 1;
		buf_len = 4;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		chunk_buf[2] = pd_get_battery_cap_ref;
		chunk_buf[3] = 0; // Padding
		break;
	case PD_EXT_GET_BATTERY_STATUS: // request
		data_size = 1;
		buf_len = 4;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		chunk_buf[2] = pd_get_battery_status_ref;
		chunk_buf[3] = 0; // Padding
		break;
	case PD_EXT_GET_MANUFACTURER_INFO: // request
		data_size = 2;
		buf_len = 4;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		chunk_buf[2] = pd_manufacturer_info.manufacturer_info_target;
		chunk_buf[3] = pd_manufacturer_info.manufacturer_info_ref;
		break;
	case PD_EXT_SOURCE_CAP: // response
		data_size = 24;
		buf_len = 25+3;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		memcpy(chunk_buf+2, &pd_source_cap_extended, 23/*sizeof(pd_source_cap_extended)*/);
		chunk_buf[25] = 0; // Padding
		chunk_buf[26] = 0; // Padding
		chunk_buf[27] = 0; // Padding
		break;
	case PD_EXT_STATUS: // response
		data_size = 5;
		buf_len = 5+3;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		memcpy(chunk_buf+2, &pd_status_data, 3/*sizeof(pd_status_data)*/);
		chunk_buf[5] = 0; // Padding
		chunk_buf[6] = 0; // Padding
		chunk_buf[7] = 0; // Padding
		break;
	case PD_EXT_BATTERY_CAP: // response
		data_size = 9;
		buf_len = 11+1;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		memcpy(chunk_buf+2, &pd_battery_cap[pd_get_battery_cap_ref], 9/*sizeof(pd_battery_cap)*/);
		chunk_buf[11] = 0; // Padding
		break;
	case PD_EXT_MANUFACTURER_INFO: // response 
		data_size = 26;
		buf_len = 28;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		memcpy(chunk_buf+2, &pd_manufacturer_info_data, 26-12/*sizeof(pd_manufacturer_info_data)*/);
		chunk_buf[10] = 0; // Padding
		chunk_buf[11] = 0; // Padding
		chunk_buf[12] = 0; // Padding
		chunk_buf[13] = 0; // Padding
		break;
	case PD_EXT_SINK_CAP: // response 
		data_size = 23;
		buf_len = 25+3;
		USB_PD_EXT_HEADER(chunk_buf,data_size,0,0,1);
		memcpy(chunk_buf+2, &pd_source_cap_extended, 23/*sizeof(pd_source_cap_extended)*/);
		chunk_buf[25] = 0; // Padding
		chunk_buf[26] = 0; // Padding
		chunk_buf[27] = 0; // Padding
		break;
	default:
		break;
	}
	
	if(buf_len)
		return send_ext_msg(1, type, chunk_buf, buf_len, type_sop);

	return 0;
}

unsigned char send_unext_message(unsigned char type, unsigned char type_sop)
{
	unsigned char ret = CMD_FAIL;
	unsigned char buf_len = 0;

	switch(type)
	{
	case PD_CTRL_NOT_SUPPORTED:
		break;
	case PD_CTRL_GET_SOURCE_CAP_EXTENDED: // request
	case PD_CTRL_GET_SINK_CAP_EXTENDED:
		break;
	case PD_CTRL_GET_STATUS: // request
		break;
	case PD_CTRL_FR_SWAP:
		break;

	case PD_DATA_ALERT: // request
		buf_len = 4;
		memcpy(chunk_buf, &pd_alert_data_obj, 4/*sizeof(pd_alert_data_obj)*/);
		break;
	case PD_DATA_BATTERY_STATUS: // respone
		buf_len = 4;
		memcpy(chunk_buf, &pd_battery_status[pd_get_battery_status_ref], 4/*sizeof(pd_battery_status)*/);
		break;

	default:
		break;
	}

	if(ret)
		ret = send_ext_msg(0, type, chunk_buf, buf_len, type_sop);

	return ret;
}

unsigned char recv_ext_message(unsigned char type, unsigned char *para, unsigned int para_len)
{
	unsigned char ret = 0;
#if DEBUG_MSG
	TRACE1("recv_ext_message: para_len = %d\n",para_len);
#else
	UNUSED_VAR(para_len);
#endif	
	switch(type)
	{
		case PD_EXT_GET_BATTERY_CAP: // request
#if DEBUG_MSG
		  TRACE("recv_ext_message: PD_EXT_GET_BATTERY_CAP\n");
#endif
		  pd_get_battery_cap_ref = para[2];
		  send_ext_message(PD_EXT_BATTERY_CAP, 0);
		  break;
		case PD_EXT_GET_BATTERY_STATUS: // request
#if DEBUG_MSG
		  TRACE("recv_ext_message: PD_EXT_GET_BATTERY_STATUS\n");
#endif
		  pd_get_battery_status_ref = para[2];
		  send_unext_message(PD_DATA_BATTERY_STATUS, 0);
		  break;
	  case PD_EXT_GET_MANUFACTURER_INFO: // request
#if DEBUG_MSG
		TRACE("recv_ext_message: PD_EXT_GET_MANUFACTURER_INFO\n");
#endif
		pd_manufacturer_info.manufacturer_info_target = para[2];
		pd_manufacturer_info.manufacturer_info_ref = para[3];
		send_ext_message(PD_EXT_MANUFACTURER_INFO, 0);
		break;
	  case PD_EXT_MANUFACTURER_INFO: // response
#if DEBUG_MSG
		TRACE("recv_ext_message: PD_EXT_MANUFACTURER_INFO\n");
#endif
		memcpy(&recv_manufacturer_info_data,para+2,para_len-2);
		TRACE1("manufacturer info, VID = %x\n",recv_manufacturer_info_data.VID);
		TRACE1("manufacturer info, PID = %x\n",recv_manufacturer_info_data.PID);
		TRACE1("manufacturer info, string = %s\n",recv_manufacturer_info_data.manufacturer_string);
		break;
	  case PD_EXT_BATTERY_CAP: // response
#if DEBUG_MSG
		TRACE("recv_ext_message: PD_EXT_BATTERY_CAP\n");
#endif
		memcpy(&recv_battery_cap,para+2,9);
		TRACE1("batter cap, VID = %x\n",recv_battery_cap.VID);
		TRACE1("batter cap, PID = %x\n",recv_battery_cap.PID);
		TRACE1("batter cap, design = %x\n",recv_battery_cap.battery_design_cap);
		TRACE1("batter cap, last full charge = %x\n",recv_battery_cap.battery_last_full_charge_cap);
		TRACE1("batter cap, type = %bx\n",recv_battery_cap.battery_type);
		break;
		case PD_EXT_SOURCE_CAP: // response
#if DEBUG_MSG
		  TRACE("recv_ext_message: PD_EXT_SOURCE_CAP\n");
#endif
		  memcpy(&recv_source_cap_extended,para+2,23);
		  TRACE1("source cap extended, VID = %x\n",recv_source_cap_extended.VID);
		  TRACE1("source cap extended, PID = %x\n",recv_source_cap_extended.PID);
		  break;
		case PD_EXT_STATUS: // response
#if DEBUG_MSG
			TRACE("recv_ext_message: PD_EXT_SOURCE_CAP\n");
#endif
			memcpy(&recv_status_data,para+2,3);
			TRACE1("status temp = %bx\n",recv_status_data.internal_temp);
			TRACE1("status input = %bx\n",recv_status_data.present_input);
			TRACE1("status battery = %bx\n",recv_status_data.present_battery_input);
			break;
		case PD_EXT_SINK_CAP: // response
#if DEBUG_MSG
			TRACE("recv_ext_message: PD_EXT_SINK_CAP\n");
#endif

			break;
		default:
			ret = 1;
			break;
	}
	  
	return ret;
}

unsigned char recv_unext_message(unsigned char type, unsigned char *para, unsigned char para_len)
{
	unsigned char ret = 0;
#if DEBUG_MSG
	TRACE1("recv_unext_message: para_len = %bd\n",para_len);
#else
	UNUSED_VAR(para_len);
#endif	
	switch(type)
  	{
	case PD_CTRL_GET_SOURCE_CAP_EXTENDED: // request
#if DEBUG_MSG
		TRACE("recv_unext_message: PD_CTRL_GET_SOURCE_CAP_EXTENDED\n");
#endif
		send_ext_message(PD_EXT_SOURCE_CAP, 0);
		break;
		case PD_CTRL_GET_SINK_CAP_EXTENDED: // request
#if DEBUG_MSG
		TRACE("recv_unext_message: PD_CTRL_GET_SINK_CAP_EXTENDED\n");
#endif
		send_ext_message(PD_CTRL_GET_SINK_CAP_EXTENDED, 0);
		break;
	case PD_CTRL_GET_STATUS: // request
#if DEBUG_MSG
		TRACE("recv_unext_message: PD_CTRL_GET_STATUS\n");
#endif
		send_ext_message(PD_EXT_STATUS, 0);
		break;
	case PD_CTRL_FR_SWAP: // request
#if DEBUG_MSG
		TRACE("recv_unext_message: PD_CTRL_FR_SWAP\n");
#endif
		//send_unext_message(PD_CTRL_NOT_SUPPORTED, 0);
		break;
  	case PD_DATA_BATTERY_STATUS: // response
		memcpy(&recv_battery_status,para,4);
		TRACE1("battery_info = %bx\n",recv_battery_status.battery_info);
		TRACE1("battery_pc = %x\n",recv_battery_status.battery_pc);
		break;
	case PD_DATA_ALERT: // request
		memcpy(&recv_alert_data_obj,para,4);
		TRACE1("Hot swappable battery = %bx\n",recv_alert_data_obj.hot_swappable_batteries);
		TRACE1("Fixed battery = %bx\n",recv_alert_data_obj.fixed_batteries);
		TRACE1("Alert type = %bx\n",recv_alert_data_obj.type_of_alert);
		send_unext_message(PD_CTRL_GET_STATUS, 0);
		break;
	default:
		ret = 1;
		break;
	}
	return ret;

}

unsigned char PD_EXT_RAM TraceDebugFlag = 0;
void TraceDebug(unsigned char flag)
{
   TraceDebugFlag = flag;
}
	
void TraceArray2(unsigned char *array, unsigned int len)
{
  unsigned int offset = 0;
  
  while(len)
  {
  if(len>=16)
  	{
       TraceArray(array+offset,16);
	   offset += 16;
	   len -= 16; 
  	}
  else
  	{
	  TraceArray(array+offset,len);
  	  break;
  	}
  }

}


void GetBatteryCap(unsigned char index)
{
    pd_get_battery_cap_ref = index;
	send_ext_message(PD_EXT_GET_BATTERY_CAP, 0);
	
}

void GetBatteryStatus(unsigned char index)
{
    pd_get_battery_status_ref = index;
	send_ext_message(PD_EXT_GET_BATTERY_STATUS, 0);
	
}

void GetSrcCapExt(void)
{
	send_unext_message(PD_CTRL_GET_SOURCE_CAP_EXTENDED, 0);
	
}

void GetSinkCapExt(void)
{
	send_unext_message(PD_CTRL_GET_SINK_CAP_EXTENDED, 0);
	//send_ext_message(PD_EXT_SINK_CAP, 0);
	
}

void GetStatus(void)
{
	send_unext_message(PD_CTRL_GET_STATUS, 0);
	
}

void StatusAlert(void)
{
	send_unext_message(PD_DATA_ALERT, 0);
	
}

void GetManufacturerInfo(void)
{
	send_ext_message(PD_EXT_GET_MANUFACTURER_INFO, 0);
	
}

void SendFRSwap(void)
{
	send_unext_message(PD_CTRL_FR_SWAP, 0);
	
}

#endif // USE_PD30
