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

#ifndef PD_FW_UPDATE_H
#define PD_FW_UPDATE_H

//#define USE_PDFU 1

#if USE_PDFU

#define PDFU_RAM xdata

#define PDFU_V092 1
#define PDFU_NumDataNR 0 // 1:support PDFU_DATA_NR, 0: not support

#define TEST_FW_VERSION1 0x0001
#define TEST_FW_VERSION2 0x0002
#define TEST_FW_VERSION3 0x0003
#define TEST_FW_VERSION4 0x0004
#define TEST_MAX_IMAGE_SIZE 0x901B

#define USB_PDFU_VERSION_1_0 0x01

#if PDFU_V092
#define USB_PDFU_HEADER_SIZE 2
#else
#define USB_PDFU_HEADER_SIZE 4
#endif

typedef enum {
	PHASE_Enumeration = 0x00, // Enumeration
	PHASE_Acquisition = 0x01, // Acquisition, Initiator only
	PHASE_Reconfiguration = 0x02, // Reconfiguration
	PHASE_Transfer = 0x03, // Transfer
	PHASE_Validation = 0x04, // Validation
	PHASE_Manifestation = 0x05, // Manifestation
	
} PDFU_FLOW_PHASE;

// Table 5-3, Table 5-13
typedef enum {
	GET_FW_ID = 0x81, // GET_FW_ID Request
	PDFU_INITIATE = 0x82, // PDFU_INITIATE Requests
	PDFU_DATA = 0x83, // PDFU_DATA Requests
	PDFU_DATA_NR = 0x84, // PDFU_DATA_NR Requests
	PDFU_VALIDATE = 0x85, // PDFU_VALIDATE Requests
	PDFU_ABORT = 0x86, // PDFU_ABORT Requests
	PDFU_DATA_PAUSE = 0x87, // PDFU_DATA_PAUSE Requests
	VENDOR_SPECIFIC = 0xFF, // VENDOR_SPECIFIC Requests

	RES_GET_FW_ID = 0x01, // GET_FW_ID Response
	RES_PDFU_INITIATE = 0x02, // PDFU_INITIATE Response
	RES_PDFU_DATA = 0x03, // PDFU_DATA Response
	RES_PDFU_VALIDATE = 0x05, // PDFU_VALIDATE Response
	RES_PDFU_DATA_PAUSE = 0x06, // PDFU_DATA_PAUSE Response
	RES_VENDOR_SPECIFIC = 0x7F, // VENDOR_SPECIFIC Response
	
} PDFU_MSG_TYPE;

// Table 5-24
typedef enum {
	RES_OK = 0x00,
	RES_errTarget = 0x01,
	RES_errFile = 0x02,
	RES_errWrite = 0x03,
	RES_errERASE = 0x04,
	RES_errCHECK_ERASED = 0x05,
	RES_errPROG = 0x06,
	RES_errVERIFY = 0x07,
	RES_errADDRESS = 0x08,
	RES_errNOTDONE = 0x09,
	RES_errFIRMWARE = 0x0A,
	RES_errPOR = 0x0D,
	RES_errUNKNOWN = 0x0E,
	RES_errUNEXPECTED_HARD_RESET = 0x80,
	RES_errUNEXPECTED_SOFT_RESET = 0x81,
	RES_errUNEXPECTED_REQUEST = 0x82,
	RES_errREJECT_PAUSE = 0x83,
	
} PDFU_RESPONSE_STATUS;


#if PDFU_V092
// Request/Response Header
// Table 5-1, Table 5-2
// Table 5-4 GET_FW_ID Request Header
// Table 5-5 PDFU_INITIATE Request Header 
// Table 5-7 PDFU_DATA Request Header
// Table 5-9 PDFU_DATA_NR Request Header 
// Table 5-11 PDFU_VALIDATE Request Header
// Table 5-12 PDFU_ABORT Request Header
// Table 5-13 PDFU_DATA_PAUSE Request Header
// Table 5-14: VENDOR_SPECIFIC Request Header
typedef struct {
	unsigned char ProtocolVersion; // 0x01: V1.0, others: Reserved
	unsigned char MessageType;
	
} PDFU_MSG_HEADER;
#else
typedef struct {
	unsigned char ProtocolVersion; // 0x01: V1.0, others: Reserved
	unsigned char MessageType;
	unsigned char Param1;
	unsigned char Param2;
	
} PDFU_MSG_HEADER;
#endif

#if PDFU_V092
// Table 5-18: GET_FW_ID Response Payload
typedef struct {
	unsigned char Status; // Table 5-24
	unsigned int VID;
	unsigned int PID;
	unsigned char HWRevision;
	unsigned char SiRevision;
	unsigned int FWVersion1;
	unsigned int FWVersion2;
	unsigned int FWVersion3;
	unsigned int FWVersion4;
	unsigned char ImageBank;
	unsigned char Flags1;
	unsigned char Flags2;
	unsigned char Flags3;
	unsigned char Flags4;
	
} PDFU_GET_FW_ID_RESP;
#else
// Table 5-15: GET_FW_ID Response Payload
typedef struct {
	unsigned char Status; // Table 5-24
	unsigned int VID;
	unsigned int PID;
	unsigned char HWRevision;
	unsigned char SiRevision;
	unsigned char FWMajorRevision;
	unsigned char FWMinorRevision;
	unsigned char ImageBank;
	unsigned char Flags1;
	unsigned char Flags2;
	unsigned char Flags3;
	unsigned char Flags4;
	
} PDFU_GET_FW_ID_RESP;
#endif
// Table 5-20: PDFU_INITIATE Response Payload
typedef struct {
	unsigned char Status; // Table 5-24
	unsigned char WaitTime; // n x 10ms
	unsigned char MaxImageSize[3];
	
} PDFU_INITIATE_RESP;

#if PDFU_V092
// Table 5-22: PDFU_DATA Response Payload
typedef struct {
	unsigned char Status; // Table 5-24
	unsigned char WaitTime; // n x 1ms
	unsigned char NumDataNR; // Number of PDFU_DATA_NR
	unsigned int DataBlockNum;
	
} PDFU_DATA_RESP;
typedef struct {
	unsigned char Status; // Table 5-24
	
} PDFU_DATA_PAUSE_RESP;
#else
// Table 5-19: PDFU_DATA Response Payload
typedef struct {
	unsigned char Status; // Table 5-24
	unsigned int DataBlockNum;
	
} PDFU_DATA_RESP;
#endif

#if PDFU_V092
// Table 5-24: PDFU_VALIDATE Response Payload
typedef struct {
	unsigned char Status; // Table 5-24
	unsigned char WaitTime; // n x 1ms
	unsigned char Flags;
	
} PDFU_VALIDATE_RESP;
#else
// Table 5-21: PDFU_VALIDATE Response Payload
typedef struct {
	unsigned char Status; // Table 5-24
	
} PDFU_VALIDATE_RESP;
#endif

// state machine for firmware update task
typedef enum {
	PDFU_STATE_IDLE = 0x00,
	PDFU_STATE_GET_FW_ID = 0x01,
	PDFU_STATE_WAIT_GET_FW_ID = 0x02,
	PDFU_STATE_IMG_VERIFY = 0x03,
	PDFU_STATE_INITIATE = 0x04,
	PDFU_STATE_WAIT_INITIATE = 0x05,
	PDFU_STATE_DATA = 0x06,
	PDFU_STATE_WAIT_DATA = 0x07,
	PDFU_STATE_VALIDATE = 0x08,
	PDFU_STATE_WAIT_VALIDATE = 0x09,
	PDFU_STATE_MANIFESTATION = 0x0A,
	PDFU_STATE_WAIT_MANIFESTATION = 0x0B,
	PDFU_STATE_RESET = 0x0C,
	PDFU_STATE_ABORT = 0x0D,
	PDFU_STATE_INITIATE_RESP = 0x10,
	PDFU_STATE_DATA_RESP = 0x11,
	PDFU_STATE_VALIDATE_RESP = 0x12,
	PDFU_STATE_DATA_PAUSE = 0x13,
	PDFU_STATE_DATA_PAUSE_WAIT_DATA = 0x14,
	PDFU_STATE_DATA_NR = 0x15,
	
} PDFU_STATE;

#if PDFU_V092
#define USB_PDFU_HEADER(sendbuf, type) \
	do { \
		((PDFU_MSG_HEADER *)sendbuf)->ProtocolVersion = USB_PDFU_VERSION_1_0; \
		((PDFU_MSG_HEADER *)sendbuf)->MessageType = type; \
	 } while(0)
#else
#define USB_PDFU_HEADER(sendbuf, type, para1, para2) \
	do { \
		((PDFU_MSG_HEADER *)sendbuf)->ProtocolVersion = USB_PDFU_VERSION_1_0; \
		((PDFU_MSG_HEADER *)sendbuf)->MessageType = type; \
		((PDFU_MSG_HEADER *)sendbuf)->Param1 = para1; \
		((PDFU_MSG_HEADER *)sendbuf)->Param2 = para2; \
	 } while(0)
#endif

extern unsigned int PDFU_RAM PDFUResponseRcvd_timer;
extern unsigned int PDFU_RAM PDFUNextRequestSent_timer;
extern unsigned int PDFU_RAM PDFUResponseSent_timer;
extern unsigned int PDFU_RAM PDFUNextRequestRcvd_timer;
extern unsigned int PDFU_RAM PDFUWait_timer;

unsigned char send_pdfu_request(unsigned char type, unsigned char type_sop);
unsigned char send_pdfu_response(unsigned char type, unsigned char type_sop);
unsigned char recv_pdfu_response(unsigned char *para, unsigned char para_len);
unsigned char recv_pdfu_request(unsigned char *para, unsigned int para_len);
void pdfu_initiator_start(void);
void pdfu_initiator_pause(void);
void pdfu_initiator_resume(void);
void pdfu_initiator_handling(void);
void pdfu_responder_set_state(unsigned char state);
void pdfu_responder_handling(void);
void TestFWUpdate(unsigned char flag);
#endif

#endif  // PD_FW_UPDATE_H
