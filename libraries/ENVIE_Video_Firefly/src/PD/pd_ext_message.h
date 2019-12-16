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

#ifndef PD_EXT_MESSAGE_H
#define PD_EXT_MESSAGE_H

#define PD_EXT_RAM xdata

#define PD_REV10 0
#define PD_REV20 1
#define PD_REV30 2

#define USB_PD_EXT_HEADER_SIZE 2

struct message_header
{
    unsigned char message_type              :5; // Message Type
    unsigned char port_data_role            :1; // Port Data Role
    unsigned char specification_revision    :2; // Specification Revision
    unsigned char port_power_role           :1; // Port Power Role
    unsigned char message_id                :3; // MessageID
    unsigned char number_of_data_objects    :3; // Number of Data Objects
    unsigned char extended                  :1; // Extended
};

struct ext_message_header
{
    unsigned int data_size                  :9; // Data Size
    unsigned int reserved                   :1; // Reserved
    unsigned int request_chunk              :1; // Request Chunk
    unsigned int chunk_number               :4; // Chunk Number
    unsigned int chunked                    :1; // Chunked
};

#define BYTE_CONST_SWAP(word) ((word)<<8)|((word)>>8)
#define BYTE_CONST_SWAP4(dword) \
	               ((((dword) & 0x000000FF) << 24)|(((dword) & 0x0000FF00) <<  8)| \
		            (((dword) & 0x00FF0000) >>  8)|(((dword) & 0xFF000000) >> 24))

#define BYTE_SWAP(word) do { (word) = ((word)<<8)|((word)>>8); } while(0)
#define BYTE_SWAP4(dword) \
	do { (dword) = (((dword) & 0x000000FF) << 24)|(((dword) & 0x0000FF00) <<  8)| \
		           (((dword) & 0x00FF0000) >>  8)|(((dword) & 0xFF000000) >> 24); \
	} while(0)

#define USB_PD_EXT_HEADER(sendbuf, size, request, number, chunk) \
	do { \
		((struct ext_message_header *)sendbuf)->data_size = size; \
		((struct ext_message_header *)sendbuf)->reserved = 0; \
		((struct ext_message_header *)sendbuf)->request_chunk = request; \
		((struct ext_message_header *)sendbuf)->chunk_number = number; \
		((struct ext_message_header *)sendbuf)->chunked = chunk; \
		BYTE_SWAP(*(unsigned int *)sendbuf); \
	 } while(0)

/* Control Message type */
enum pd_ctrl_msg_type {
	/* 0 Reserved */
	PD_CTRL_GOOD_CRC = 1,
	PD_CTRL_GOTO_MIN = 2,
	PD_CTRL_ACCEPT = 3,
	PD_CTRL_REJECT = 4,
	PD_CTRL_PING = 5,
	PD_CTRL_PS_RDY = 6,
	PD_CTRL_GET_SOURCE_CAP = 7,
	PD_CTRL_DR_SWAP = 9,	PD_CTRL_GET_SINK_CAP = 8,

	PD_CTRL_PR_SWAP = 10,
	PD_CTRL_VCONN_SWAP = 11,
	PD_CTRL_WAIT = 12,
	PD_CTRL_SOFT_RESET = 13,
	/* 14-15 Reserved */
	PD_CTRL_NOT_SUPPORTED = 16,
	PD_CTRL_GET_SOURCE_CAP_EXTENDED = 17,
	PD_CTRL_GET_STATUS = 18,
	PD_CTRL_FR_SWAP = 19,
	PD_CTRL_GET_SINK_CAP_EXTENDED = 22,
	/* 20-31 Reserved */
};
/* Data message type */
enum pd_data_msg_type {
	/* 0 Reserved */
	PD_DATA_SOURCE_CAP = 1,
	PD_DATA_REQUEST = 2,
	PD_DATA_BIST = 3,
	PD_DATA_SINK_CAP = 4,
	PD_DATA_BATTERY_STATUS = 5,
	PD_DATA_ALERT = 6,
	/* 7-14 Reserved */
	PD_DATA_VENDOR_DEF = 15,
};
/* Extended message type */
enum pd_ext_msg_type {
	/* 0 Reserved */
	PD_EXT_SOURCE_CAP = 1,
	PD_EXT_STATUS = 2,
	PD_EXT_GET_BATTERY_CAP = 3,
	PD_EXT_GET_BATTERY_STATUS = 4,
	PD_EXT_BATTERY_CAP = 5,
	PD_EXT_GET_MANUFACTURER_INFO = 6,
	PD_EXT_MANUFACTURER_INFO = 7,
	PD_EXT_SECURITY_REQUEST = 8,
	PD_EXT_SECURITY_RESPONSE = 9,
	PD_EXT_FW_UPDATE_REQUEST = 10,
	PD_EXT_FW_UPDATE_RESPONSE = 11,
	PD_EXT_SINK_CAP = 15,
	/* 12-31 Reserved */
};


// SendBlock() state
enum send_block_state_type {
	SEND_BLOCK_IDLE = 0,
	SEND_BLOCK_CHUNKING = 1,
	SEND_BLOCK_FINISH = 2,
	SEND_BLOCK_TIMEOUT = 3,
};

// RecvBlock() status
enum recv_block_status_type {
	RECV_BLOCK_IDLE = 0,
	RECV_BLOCK_CHUNKING = 1,
	RECV_BLOCK_FINISH = 2,
	RECV_BLOCK_CHUNK_REQUEST = 3,
};

// Battery Status
struct battery_status_data_obj
{
    unsigned char reserved; // Reserved
    unsigned char battery_info; // Battery Info
    unsigned int battery_pc; // Battery PC
};

// Alert Message
struct alert_data_obj
{
    unsigned int reserved     :16; // Reserved
    unsigned char hot_swappable_batteries :4; // Battery Info
    unsigned char fixed_batteries :4; // Battery Info
    unsigned char type_of_alert   :8; // Battery PC
};

// Source Capabilities Extended
struct source_cap_extended
{
    unsigned int VID;
    unsigned int PID;
    unsigned long XID;
	unsigned char fw_version;
	unsigned char hw_version;
    unsigned char voltage_regulation;
    unsigned char holdup_time;
    unsigned char compliance;
    unsigned char touch_current;
    unsigned int peak_current1;
    unsigned int peak_current2;
    unsigned int peak_current3;
    unsigned char touch_temp;
    unsigned char source_inputs;
    unsigned char batteries;
};

// Status Message
struct status_data
{
	unsigned char internal_temp;
	unsigned char present_input;
    unsigned char present_battery_input;
};

// Battery Cap
struct battery_cap
{
    unsigned int VID;
    unsigned int PID;
    unsigned int battery_design_cap;
    unsigned int battery_last_full_charge_cap;
    unsigned char battery_type;
};

// Manufacturer Info
struct manufacturer_info
{
	unsigned char manufacturer_info_target;
	unsigned char manufacturer_info_ref;
};

// Manufacturer Info Data
struct manufacturer_info_data
{
    unsigned int VID;
    unsigned int PID;
    unsigned char manufacturer_string[22];
};

extern unsigned int PD_EXT_RAM PDExtSend_timer;
extern unsigned int PD_EXT_RAM pd_block_recv_len;
extern unsigned char PD_EXT_RAM pd_block_recv_buf[260];

unsigned char SendBlock(unsigned char type, unsigned char *pData,unsigned int BlockSize);
unsigned char RecvBlock(unsigned char type, unsigned char *pData, unsigned char para_len);
unsigned char SendBlockIdle(void);
void pd_ext_message_handling(void);
unsigned char send_ext_message(unsigned char type, unsigned char type_sop);
unsigned char send_unext_message(unsigned char type, unsigned char type_sop);
unsigned char recv_ext_message(unsigned char type, unsigned char *para, unsigned int para_len);
unsigned char recv_unext_message(unsigned char type, unsigned char *para, unsigned char para_len);
void TraceArray2(unsigned char *array, unsigned int len);
void GetBatteryCap(unsigned char index);
void GetBatteryStatus(unsigned char index);
void GetSrcCapExt(void);
void GetSinkCapExt(void);
void GetStatus(void);
void StatusAlert(void);
void GetManufacturerInfo(void);
void SendFRSwap(void);

extern unsigned char PD_EXT_RAM pd_get_battery_cap_ref;
extern unsigned char PD_EXT_RAM pd_get_battery_status_ref;

// debug
#define DBG_MSG if(TraceDebugFlag&0x01) printf
#define DBG_MSG2 if(TraceDebugFlag&0x04) printf
extern unsigned char PD_EXT_RAM TraceDebugFlag;
void TraceDebug(unsigned char flag);

#define USE_POWER_RULE

#ifdef USE_POWER_RULE
unsigned int pd30_src_pwr_rules_ma(unsigned char watts,unsigned char voltage);
#endif

#define TEST_VENDOR_ID  0x1F29
#define TEST_PRODUCT_ID 0x7428
#define TEST_HW_REV 0x01
#define TEST_SI_REV 0x04
#define TEST_FW_MAJOR_REV 0x02
#define TEST_FW_MINOR_REV 0x01

#define USE_PDFU 1
#if USE_PDFU
#include "pd_fw_update.h"
#endif

#endif  // PD_EXT_MESSAGE_H
