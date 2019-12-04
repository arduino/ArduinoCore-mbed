/*
 * Copyright(c) 2016, Analogix Semiconductor. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */



#include "anx7625_driver.h"
#include "anx7625_private_interface.h"
#include "anx7625_public_interface.h"

#include "MI2_REG.h"
#include "Flash.h"
#include "anx7625_pd30_message.h"

#include "anx7625_pd_fw_update.h"

#ifdef USE_PD30

#if USE_PDFU

#define DEBUG_MSG 0

#define PARAM_tPDFUResponseRcvd (200) /* ms*/
#define PARAM_tPDFUNextRequestSent (135) /* ms*/
#define PARAM_tPDFUResponseSent (135) /* ms*/
#define PARAM_tPDFUNextRequestRcvd (200) /* ms*/

#define EnumerateResend 10
#define ReconfigureResend 3
#define DataResend 3
#define ValidationResend 3
#define PauseResend 3

#define PDFU_DATA_WRITE_DELAY (120) /* ms*/

unsigned char PDFU_RAM pd_block_buf[260];
unsigned char PDFU_RAM pd_chunk_buf[30];
unsigned char PDFU_RAM pd_fw_buf_index;
unsigned char PDFU_RAM pd_chunk_number;

unsigned char PDFU_RAM pdfu_initiator_phase = PHASE_Enumeration;
unsigned char PDFU_RAM pdfu_initiator_state = PDFU_STATE_IDLE;
unsigned char PDFU_RAM pdfu_initiator_prev_state = PDFU_STATE_IDLE;
unsigned char PDFU_RAM pdfu_initiator_request; /* for data pause*/
unsigned char PDFU_RAM pdfu_responder_phase = PHASE_Enumeration;
unsigned char PDFU_RAM pdfu_responder_state = PDFU_STATE_IDLE;
unsigned char PDFU_RAM pdfu_responder_prev_state = PDFU_STATE_IDLE;
unsigned char PDFU_RAM pdfu_responder_request; /* for data pause*/

/* Initiator*/
	/* tPDFUResponseRcvd*/
unsigned int PDFU_RAM PDFUResponseRcvd_timer;
	/* tPDFUNextRequestSent*/
unsigned int PDFU_RAM PDFUNextRequestSent_timer;
/* Responder*/
	/* tPDFUResponseSent*/
unsigned int PDFU_RAM PDFUResponseSent_timer;
	/* tPDFUNextRequestRcvd*/
unsigned int PDFU_RAM PDFUNextRequestRcvd_timer;

unsigned int PDFU_RAM PDFUWait_timer; /* WaitTime*/

unsigned char PDFU_RAM EnumerateResend_counter = EnumerateResend;
unsigned char PDFU_RAM ReconfigureResend_counter = ReconfigureResend;
unsigned char PDFU_RAM DataResend_counter = DataResend;
unsigned char PDFU_RAM ValidationResend_counter = ValidationResend;
unsigned char PDFU_RAM PauseResend_counter = PauseResend;


#if PDFU_V092
/* Initiator Data*/
/* Initiator Send Request data*/
/* Table 5-6: PDFU_INITIATE*/
unsigned int const Initiator_InitiatePayload[4] = {
	BYTE_CONST_SWAP(TEST_FW_VERSION1), /*FWVersion1*/
	BYTE_CONST_SWAP(TEST_FW_VERSION2), /*FWVersion2*/
	BYTE_CONST_SWAP(TEST_FW_VERSION3), /*FWVersion3*/
	BYTE_CONST_SWAP(TEST_FW_VERSION4), /*FWVersion4*/
};
#else
/* Table 5-5: PDFU_INITIATE*/
unsigned char const Initiator_InitiateParam[2] = {
	TEST_FW_MAJOR_REV, /*Param1: Major Firmware Revision*/
	TEST_FW_MINOR_REV  /*Param2: Minor Firmware Revision*/
};
/* Table 5-12: VENDOR_SPECIFIC*/
unsigned char const Initiator_VendorSpecificParam[2] = {
	(TEST_VENDOR_ID >> 8), /*Param1: MSB of VID*/
	(TEST_VENDOR_ID & 0xFF) /*Param2: LSB of VID*/
};
#endif


#if PDFU_V092
/* Responder Data*/
/* Responder Send Response data*/
/* Table 5-18: GET_FW_ID*/
struct PDFU_GET_FW_ID_RESP const Responder_GetFwIdPayload = {
	RES_OK,            /* Status*/
	BYTE_CONST_SWAP(TEST_VENDOR_ID),    /* VID*/
	BYTE_CONST_SWAP(TEST_PRODUCT_ID),   /* PID*/
	TEST_HW_REV,       /* HWRevision*/
	TEST_SI_REV,       /* SiRevision*/
	BYTE_CONST_SWAP(TEST_FW_VERSION1), /* FWVersion1*/
	BYTE_CONST_SWAP(TEST_FW_VERSION2), /* FWVersion2*/
	BYTE_CONST_SWAP(TEST_FW_VERSION3), /* FWVersion3*/
	BYTE_CONST_SWAP(TEST_FW_VERSION4), /* FWVersion4*/
	0, /* ImageBank*/
	1, /* Flags1, bit0: PDFU via USB PD flow*/
	4, /* Flags2, bit2: Responder is not fully functional*/
	1, /* Flags3, bit0: Hard Reset required to complete firmware update*/
	8  /* Flags4, bit3: power cycle is required to firmware update*/
};
#else
/* Table 5-15: GET_FW_ID*/
struct PDFU_GET_FW_ID_RESP const Responder_GetFwIdPayload = {
	RES_OK,            /* Status*/
	TEST_VENDOR_ID,    /* VID*/
	TEST_PRODUCT_ID,   /* PID*/
	TEST_HW_REV,       /* HWRevision*/
	TEST_SI_REV,       /* SiRevision*/
	TEST_FW_MAJOR_REV, /* FWMajorRevision*/
	TEST_FW_MINOR_REV, /* FWMinorRevision*/
	0, /* ImageBank*/
	1, /* Flags1, bit0: PDFU via USB PD flow*/
	4, /* Flags2, bit2: Responder is not fully functional*/
	1, /* Flags3, bit0: Hard Reset required to complete firmware update*/
	8 /* Flags4, bit3: power cycle is required to complete firmware update*/
};
#endif
/* Table 5-20: PDFU_INITIATE*/
struct PDFU_INITIATE_RESP PDFU_RAM Responder_InitiatePayload = {
	RES_OK, /* Status*/
	0,      /* WaitTime, n x 10ms*/
	{
		/* MaxImageSize[3];*/
		TEST_MAX_IMAGE_SIZE & 0xFF,
		(TEST_MAX_IMAGE_SIZE >> 8) & 0xFF,
		(TEST_MAX_IMAGE_SIZE >> 16) & 0x0F,
	}
};
#if PDFU_V092
/* Table 5-22: PDFU_DATA*/
struct PDFU_DATA_RESP PDFU_RAM Responder_DataPayload = {
	RES_OK, /* Status*/
	0, /* WaitTime, n x 1ms*/
	PDFU_NumDataNR, /* Number of PDFU_DATA_NR*/
	BYTE_CONST_SWAP(0)  /* DataBlockNum*/
};
struct PDFU_DATA_PAUSE_RESP PDFU_RAM Responder_DataPausePayload = {
	RES_OK, /* Status, Table 5-24*/
};
#else
/* Table 5-18: PDFU_DATA*/
unsigned char PDFU_RAM Responder_DataParam[2] = {
	0, /*Param1: WaitTime, n x 1ms*/
	0  /*Param2: Number of PDFU_DATA_NR*/
};
/* Table 5-19: PDFU_DATA*/
struct PDFU_DATA_RESP PDFU_RAM Responder_DataPayload = {
	RES_OK, /* Status*/
	0       /* DataBlockNum*/
};
#endif
#if PDFU_V092
/* Table 5-24: PDFU_VALIDATE*/
struct PDFU_VALIDATE_RESP PDFU_RAM Responder_ValidatePayload = {
	RES_OK, /* Status*/
	0, /* WaitTime, n x 1ms*/
	1  /* 1 if validation was successful*/
};
#else
/* Table 5-20: PDFU_ VALIDATE*/
unsigned char PDFU_RAM Responder_ValidateParam[2] = {
	0, /*Param1: WaitTime, n x 1ms*/
	1  /*Param2: 1 if validation was successful*/
};
struct PDFU_VALIDATE_RESP PDFU_RAM Responder_ValidatePayload = {
	RES_OK, /* Status, Table 5-24*/
};
#endif

/* Initiator Recv Response data*/
/* for GET_FW_ID*/
unsigned char PDFU_RAM ResponseFwIdReady;
struct PDFU_GET_FW_ID_RESP PDFU_RAM ResponseFwIdPayload;
/* for PDFU_INITIATE*/
unsigned char PDFU_RAM ResponseInitiateReady;
struct PDFU_INITIATE_RESP PDFU_RAM ResponseInitiatePayload;
/* for PDFU_DATA*/
unsigned char PDFU_RAM ResponseDataReady;
#if PDFU_V092
struct PDFU_DATA_RESP PDFU_RAM ResponseDataPayload;
#else
unsigned char PDFU_RAM ResponseDataParam[2];
struct PDFU_DATA_RESP PDFU_RAM ResponseDataPayload;
#endif
/* for PDFU_VALIDATE*/
unsigned char PDFU_RAM ResponseValidateReady;
#if PDFU_V092
struct PDFU_VALIDATE_RESP PDFU_RAM ResponseValidatePayload;
#else
unsigned char PDFU_RAM ResponseValidateParam[2];
struct PDFU_VALIDATE_RESP PDFU_RAM ResponseValidatePayload;
#endif
/* for PDFU_DATA_PAUSE*/
#if PDFU_V092
unsigned char PDFU_RAM ResponseDataPauseReady;
struct PDFU_DATA_PAUSE_RESP PDFU_RAM ResponseDataPausePayload;
#endif

/* for PDFU_DATA*/
#define DATA_BLOCK_SIZE 256
unsigned int PDFU_RAM DataBlockSize;
unsigned int PDFU_RAM DataBlockIndex;
unsigned int PDFU_RAM DataBlockTotal;
unsigned int PDFU_RAM DataBlockSizeRemain;
const unsigned char *FwImagePtr;
unsigned int PDFU_RAM FwImageOffset;
unsigned int PDFU_RAM FwImageSize;
unsigned int PDFU_RAM FwImageSizeMax; /* for debug*/

/* Responder DataBlockNum*/
unsigned int PDFU_RAM DataBlockNum;


unsigned char sim_write(unsigned int addr, unsigned char *pData,
	unsigned int len)
{
	TRACE1("W[%02X00]\n", addr);
	if (TraceDebugFlag & 0x02)
		TraceArray2(pData, len);
	return len;
}

/* PD FW Update command*/
void pdfu_initiator_start(void)
{
	pdfu_initiator_phase = PHASE_Enumeration;
	pdfu_initiator_prev_state = PDFU_STATE_IDLE;
	pdfu_initiator_request = 0; /* for data pause*/
	/*pdfu_initiator_prev_state = PDFU_STATE_IDLE;*/
	pdfu_initiator_state = PDFU_STATE_GET_FW_ID;
}

void pdfu_initiator_pause(void)
{
	if (pdfu_initiator_phase == PHASE_Transfer &&
		(pdfu_initiator_state == PDFU_STATE_DATA ||
		pdfu_initiator_state == PDFU_STATE_WAIT_DATA)) {
		TRACE("PDFU Initiator pause!\n");
		pdfu_initiator_request = PDFU_STATE_DATA_PAUSE;
	}
}

void pdfu_initiator_resume(void)
{
	/*pdfu_initiator_prev_state = PDFU_STATE_IDLE;*/
	if (pdfu_initiator_phase == PHASE_Transfer &&
	    (pdfu_initiator_state == PDFU_STATE_DATA_PAUSE ||
	    pdfu_initiator_state == PDFU_STATE_DATA_PAUSE_WAIT_DATA)) {
		TRACE("PDFU Initiator resume!\n");
		PDFUNextRequestSent_timer = PARAM_tPDFUNextRequestSent;
		pdfu_initiator_state = PDFU_STATE_DATA;
	}
	pdfu_initiator_request = 0;
}

/* PD FW Update handler*/
void pdfu_initiator_handling(void)
{
	switch (pdfu_initiator_phase) {
	case PHASE_Enumeration:
		if (pdfu_initiator_state != PDFU_STATE_IDLE) {
			if (pdfu_initiator_prev_state != pdfu_initiator_state) {
				if (pdfu_initiator_prev_state ==
					PDFU_STATE_IDLE) {
					TRACE("Initiator_Enumeration\n");
					EnumerateResend_counter =
						EnumerateResend;
				}
				pdfu_initiator_prev_state =
					pdfu_initiator_state;
			}

			switch (pdfu_initiator_state) {
			case PDFU_STATE_GET_FW_ID:
				/* send GET_FW_ID request*/
				send_pdfu_request(GET_FW_ID, 0);
				PDFUResponseRcvd_timer =
					PARAM_tPDFUResponseRcvd;
				pdfu_initiator_state =
					PDFU_STATE_WAIT_GET_FW_ID;
				break;
			case PDFU_STATE_WAIT_GET_FW_ID:
				/* wait GET_FW_ID respone*/
				if (ResponseFwIdReady) {
					PDFUNextRequestSent_timer =
						PARAM_tPDFUNextRequestSent;
					if (ResponseFwIdPayload.Status ==
						RES_OK) {
						pdfu_initiator_state =
							PDFU_STATE_IMG_VERIFY;
						pdfu_initiator_phase =
							PHASE_Acquisition;
					} else {
						pdfu_initiator_state =
							PDFU_STATE_ABORT;
					}
				} else if (!PDFUResponseRcvd_timer) {
					if (EnumerateResend_counter) {
						EnumerateResend_counter--;
						pdfu_initiator_state =
							PDFU_STATE_GET_FW_ID;
					} else {
						pdfu_initiator_state =
							PDFU_STATE_ABORT;
					}
				}
				break;
			case PDFU_STATE_ABORT:
				TRACE("PDFU abort!\n");
				pdfu_initiator_state = PDFU_STATE_IDLE;
				pdfu_initiator_phase = PHASE_Enumeration;
				break;
			default:
				break;
			}
		}
		break;
	case PHASE_Acquisition:
		if (pdfu_initiator_prev_state != pdfu_initiator_state) {
			if (pdfu_initiator_prev_state ==
				PDFU_STATE_WAIT_GET_FW_ID) {
				TRACE("Initiator_Acquisition\n");
			}
			pdfu_initiator_prev_state = pdfu_initiator_state;
		}
		switch (pdfu_initiator_state) {
		case PDFU_STATE_IMG_VERIFY:
			if (IsFwImgValid()) {
				pdfu_initiator_state = PDFU_STATE_INITIATE;
				pdfu_initiator_phase = PHASE_Reconfiguration;
			} else {
				TRACE("PDFU FW IMG incorrect!\n");
				pdfu_initiator_state = PDFU_STATE_ABORT;
			}
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_initiator_state = PDFU_STATE_IDLE;
			pdfu_initiator_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}

		break;
	case PHASE_Reconfiguration:
		if (pdfu_initiator_prev_state != pdfu_initiator_state) {
			if (pdfu_initiator_prev_state ==
				PDFU_STATE_IMG_VERIFY) {
				TRACE("Initiator_Reconfiguration\n");
				ReconfigureResend_counter =
					ReconfigureResend;
			}
			pdfu_initiator_prev_state = pdfu_initiator_state;
		}

		switch (pdfu_initiator_state) {
		case PDFU_STATE_INITIATE:
			if (!PDFUNextRequestSent_timer)
				TRACE("PDFUNextRequestSent timeout\n");
			send_pdfu_request(PDFU_INITIATE, 0);
			PDFUResponseRcvd_timer = PARAM_tPDFUResponseRcvd;
			pdfu_initiator_state = PDFU_STATE_WAIT_INITIATE;
			break;
		case PDFU_STATE_WAIT_INITIATE:
			if (ResponseInitiateReady) {
				if (ResponseInitiateReady == 1)
					PDFUNextRequestSent_timer =
					PARAM_tPDFUNextRequestSent;
				if (InitiateWaitTime() == 0) {
					pdfu_initiator_state = PDFU_STATE_DATA;
					pdfu_initiator_phase = PHASE_Transfer;
				} else if (InitiateWaitTime() == 255) {
					pdfu_initiator_state = PDFU_STATE_ABORT;
				} else {
				if (ResponseInitiateReady == 1) {
					PDFUWait_timer =
						InitiateWaitTime() * 10;
					ResponseInitiateReady = 2;
				} else if (ResponseInitiateReady == 2) {
					if (!PDFUWait_timer) {
						pdfu_initiator_state =
						PDFU_STATE_INITIATE;
					}
				}
				}
			} else if (!PDFUResponseRcvd_timer) { /* timeout*/
				if (ReconfigureResend_counter) { /* retry*/
					ReconfigureResend_counter--;
				pdfu_initiator_state = PDFU_STATE_INITIATE;
				} else {
					pdfu_initiator_state = PDFU_STATE_ABORT;
				}
			}
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_initiator_state = PDFU_STATE_IDLE;
			pdfu_initiator_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}
		break;
	case PHASE_Transfer:
		if (pdfu_initiator_prev_state != pdfu_initiator_state) {
			if (pdfu_initiator_prev_state ==
				PDFU_STATE_WAIT_INITIATE) {
				TRACE("Initiator_Transfer\n");
				InitDataBlock();
				DataResend_counter = DataResend;
			}
			pdfu_initiator_prev_state = pdfu_initiator_state;
		}

		switch (pdfu_initiator_state) {
		case PDFU_STATE_DATA:
			if (pdfu_initiator_request == PDFU_STATE_DATA_PAUSE) {
				send_pdfu_request(PDFU_DATA_PAUSE, 0);
				PDFUResponseRcvd_timer =
					PARAM_tPDFUResponseRcvd;
				pdfu_initiator_state = PDFU_STATE_DATA_PAUSE;
				pdfu_initiator_request = 0;
				break;
			}
			if (!PDFUNextRequestSent_timer)
				TRACE("PDFUNextRequestSent timeout\n");
			if (SendBlockIdle()) {
				send_pdfu_request(PDFU_DATA, 0);
				PDFUResponseRcvd_timer =
					PARAM_tPDFUResponseRcvd;
				pdfu_initiator_state = PDFU_STATE_WAIT_DATA;
			}
			break;
		case PDFU_STATE_WAIT_DATA:
			if (ResponseDataReady) {
				if (ResponseDataReady == 1)
					PDFUNextRequestSent_timer =
						PARAM_tPDFUNextRequestSent;
				if (DataWaitTime() == 0) {
					/* control by responder*/
				if (DataBlockIndex + 1 >= DataBlockTotal) {
					/* end of chunk*/
					pdfu_initiator_state =
						PDFU_STATE_VALIDATE;
					pdfu_initiator_phase =
						PHASE_Validation;
				} else {
				if ((BYTE_CONST_SWAP(
					ResponseDataPayload.DataBlockNum)) <
					DataBlockTotal)
					DataBlockIndex =
				BYTE_CONST_SWAP(
					ResponseDataPayload.DataBlockNum);
				/*else*/
				/*DataBlockIndex++;*/
				/* send next chunk*/
				pdfu_initiator_state = PDFU_STATE_DATA;
				}
				} else if (DataWaitTime() == 255) {
					TRACE("DataWaitTime() abort.\n");
					pdfu_initiator_state = PDFU_STATE_ABORT;
				} else {
					if (ResponseDataReady == 1) {
						PDFUWait_timer = DataWaitTime();
						ResponseDataReady = 2;
			} else if (ResponseDataReady == 2) {
				if (!PDFUWait_timer) {
					/* control by responder*/
					if (DataBlockIndex + 1 >=
						DataBlockTotal) {
						/* end of chunk*/
						pdfu_initiator_state =
							PDFU_STATE_VALIDATE;
			pdfu_initiator_phase = PHASE_Validation;
		} else {
			if ((BYTE_CONST_SWAP(
				ResponseDataPayload.DataBlockNum)) <
				DataBlockTotal)
				DataBlockIndex = BYTE_CONST_SWAP(
					ResponseDataPayload.DataBlockNum);
			/*else*/
			/*DataBlockIndex++;*/
			/* send next chunk*/
			pdfu_initiator_state = PDFU_STATE_DATA;
			}
					}
				}
				}
			} else if (!PDFUResponseRcvd_timer) { /* timeout*/
				if (DataResend_counter) { /* retry*/
					DataResend_counter--;
					TRACE1("PDFU_DATA TMO (%d)\n",
						DataResend_counter);
					pdfu_initiator_state = PDFU_STATE_DATA;
				} else {
					pdfu_initiator_state = PDFU_STATE_ABORT;
				}
			}
			break;
		case PDFU_STATE_DATA_PAUSE:
			if (ResponseDataPauseReady) {
				pdfu_initiator_state =
					PDFU_STATE_DATA_PAUSE_WAIT_DATA;
			} else if (!PDFUResponseRcvd_timer) { /* timeout*/
				if (PauseResend_counter) { /* retry*/
					PauseResend_counter--;
					TRACE1("PDFU_DATA_PAUSE TMO (%d)\n",
						PauseResend_counter);
					send_pdfu_request(PDFU_DATA_PAUSE, 0);
					PDFUResponseRcvd_timer =
						PARAM_tPDFUResponseRcvd;
				} else {
					pdfu_initiator_state = PDFU_STATE_ABORT;
				}
			}
			break;
		case PDFU_STATE_DATA_PAUSE_WAIT_DATA:
			/* wait command to resume.*/
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_initiator_state = PDFU_STATE_IDLE;
			pdfu_initiator_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}
		break;
	case PHASE_Validation:
		if (pdfu_initiator_prev_state != pdfu_initiator_state) {
			if (pdfu_initiator_prev_state == PDFU_STATE_WAIT_DATA) {
				TRACE("Initiator_Validation\n");
				ValidationResend_counter = ValidationResend;
			}
			pdfu_initiator_prev_state = pdfu_initiator_state;
		}

		switch (pdfu_initiator_state) {
		case PDFU_STATE_VALIDATE:
			if (!PDFUNextRequestSent_timer)
				TRACE("PDFUNextRequestSent timeout\n");
			send_pdfu_request(PDFU_VALIDATE, 0);
			PDFUResponseRcvd_timer = PARAM_tPDFUResponseRcvd;
			pdfu_initiator_state = PDFU_STATE_WAIT_VALIDATE;
			break;
		case PDFU_STATE_WAIT_VALIDATE:
			if (ResponseValidateReady) {
				if (ResponseValidateReady == 1)
					PDFUNextRequestSent_timer =
					PARAM_tPDFUNextRequestSent;
				if (ValidateWaitTime() == 0) {
					pdfu_initiator_state =
						PDFU_STATE_MANIFESTATION;
					pdfu_initiator_phase =
						PHASE_Manifestation;
				} else if (ValidateWaitTime() == 255) {
					pdfu_initiator_state =
						PDFU_STATE_ABORT;
				} else {
				if (ResponseValidateReady == 1) {
					PDFUWait_timer = ValidateWaitTime();
					ResponseValidateReady = 2;
				} else if (ResponseValidateReady == 2) {
					if (!PDFUWait_timer) {
						pdfu_initiator_state =
						PDFU_STATE_VALIDATE;
					}
				}
				}
			} else if (!PDFUResponseRcvd_timer) { /* timeout*/
				if (ValidationResend_counter) { /* retry*/
					ValidationResend_counter--;
					pdfu_initiator_state =
						PDFU_STATE_VALIDATE;
				} else {
					pdfu_initiator_state =
						PDFU_STATE_ABORT;
				}
			}
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_initiator_state = PDFU_STATE_IDLE;
			pdfu_initiator_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}
		break;
	case PHASE_Manifestation:
		if (pdfu_initiator_prev_state != pdfu_initiator_state) {
			if (pdfu_initiator_prev_state ==
				PDFU_STATE_WAIT_VALIDATE) {
				TRACE("Initiator_Manifestation\n");
			}
			pdfu_initiator_prev_state = pdfu_initiator_state;
		}

		switch (pdfu_initiator_state) {
		case PDFU_STATE_MANIFESTATION:
			/* send hard reset or cable reset*/
			pdfu_initiator_state = PDFU_STATE_WAIT_MANIFESTATION;
			break;
		case PDFU_STATE_WAIT_MANIFESTATION:
			/* nodify user*/
			pdfu_initiator_prev_state = PDFU_STATE_IDLE;
			pdfu_initiator_state = PDFU_STATE_IDLE;
			pdfu_initiator_phase = PHASE_Enumeration;
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_initiator_prev_state = PDFU_STATE_IDLE;
			pdfu_initiator_state = PDFU_STATE_IDLE;
			pdfu_initiator_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

}

void pdfu_responder_set_state(unsigned char state)
{
	if (state == PDFU_STATE_DATA) {
		/* reset counter*/
		DataResend_counter = DataResend;
			/* reset timer*/
		PDFUNextRequestRcvd_timer = PARAM_tPDFUNextRequestRcvd;
	} else if (state == PDFU_STATE_DATA_NR) {
		/* reset counter*/
		DataResend_counter = DataResend;
		/* reset timer*/
		PDFUNextRequestRcvd_timer = PARAM_tPDFUNextRequestRcvd;
	} else if (state == PDFU_STATE_DATA_PAUSE) {
		if (pdfu_responder_phase == PHASE_Transfer &&
			(pdfu_responder_state == PDFU_STATE_DATA ||
			pdfu_responder_state == PDFU_STATE_DATA_RESP)) {
			TRACE("PDFU Initiator pause!\n");
			pdfu_responder_request = PDFU_STATE_DATA_PAUSE;
			return;
		}
	}
	pdfu_responder_state = state;
}

void pdfu_responder_handling(void)
{
	switch (pdfu_responder_phase) {
	case PHASE_Enumeration:
		if (pdfu_responder_state != PDFU_STATE_IDLE) {
			if (pdfu_responder_prev_state != pdfu_responder_state) {
				if (pdfu_responder_prev_state ==
					PDFU_STATE_IDLE)
					TRACE("Responder_Enumeration\n");
				pdfu_responder_prev_state =
					pdfu_responder_state;
			}

			switch (pdfu_responder_state) {
			case PDFU_STATE_GET_FW_ID:
				/* recv GET_FW_ID request*/
				break;
			case PDFU_STATE_INITIATE:
			/* recv PDFU_INITATE request*/
			/* PDFUResponseSent_timer = PARAM_tPDFUResponseSent;*/
			if (Responder_InitiatePayload.WaitTime > 0 &&
				Responder_InitiatePayload.WaitTime < 255) {
				PDFUNextRequestRcvd_timer =
					PARAM_tPDFUNextRequestRcvd;
				pdfu_responder_state =
					PDFU_STATE_WAIT_INITIATE;
				pdfu_responder_phase =
					PHASE_Reconfiguration;
			} else {
				pdfu_responder_state =
					PDFU_STATE_WAIT_DATA;
				pdfu_responder_phase =
					PHASE_Reconfiguration;
			}
			break;
			case PDFU_STATE_ABORT:
				TRACE("PDFU abort!\n");
				pdfu_responder_state = PDFU_STATE_IDLE;
				pdfu_responder_phase = PHASE_Enumeration;
				break;
			default:
				break;
			}
		}
		break;
	case PHASE_Reconfiguration:
		if (pdfu_responder_prev_state != pdfu_responder_state) {
			if (pdfu_responder_prev_state == PDFU_STATE_INITIATE) {
				TRACE("Responder_Reconfiguration\n");
				ReconfigureResend_counter = ReconfigureResend;
			}
			pdfu_responder_prev_state = pdfu_responder_state;
		}

		switch (pdfu_responder_state) {
		case PDFU_STATE_WAIT_DATA:
			/* wait PDFU_DATA request*/
			break;
		case PDFU_STATE_DATA:
			/* recv PDFU_DATA request*/
			pdfu_responder_prev_state = PDFU_STATE_WAIT_DATA;
			pdfu_responder_state = PDFU_STATE_DATA;
			pdfu_responder_phase = PHASE_Transfer;
			break;
		case PDFU_STATE_INITIATE_RESP:
		send_pdfu_response(RES_PDFU_INITIATE, 0);
		if (Responder_InitiatePayload.WaitTime > 0 &&
			Responder_InitiatePayload.WaitTime < 255) {
			PDFUNextRequestRcvd_timer =
				PARAM_tPDFUNextRequestRcvd;
			pdfu_responder_state =
				PDFU_STATE_WAIT_INITIATE;
		} else {
			pdfu_responder_state =
				PDFU_STATE_WAIT_DATA;
		}
		break;
		case PDFU_STATE_WAIT_INITIATE:
		/* wait PDFU_INITATE request*/
		if (!PDFUNextRequestRcvd_timer) { /* timeout*/
			if (ReconfigureResend_counter) { /* retry*/
				ReconfigureResend_counter--;
				pdfu_responder_state =
					PDFU_STATE_INITIATE_RESP;
			} else {
				pdfu_responder_state =
					PDFU_STATE_ABORT;
			}
		}
		break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_responder_state = PDFU_STATE_IDLE;
			pdfu_responder_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}
		break;
	case PHASE_Transfer:
		if (pdfu_responder_prev_state != pdfu_responder_state) {
			if (pdfu_responder_prev_state == PDFU_STATE_WAIT_DATA) {
				TRACE("Responder_Transfer\n");
				DataResend_counter = DataResend;
			}
			pdfu_responder_prev_state = pdfu_responder_state;
		}

		switch (pdfu_responder_state) {
		case PDFU_STATE_DATA:
			if (pdfu_responder_request == PDFU_STATE_DATA_PAUSE) {
				pdfu_responder_state = PDFU_STATE_DATA_PAUSE;
				pdfu_responder_request = 0;
				break;
			}
			/* recv PDFU_DATA request*/
			if (!PDFUNextRequestRcvd_timer) { /* timeout*/
				if (DataResend_counter) { /* retry*/
					DataResend_counter--;
					pdfu_responder_state =
						PDFU_STATE_DATA_RESP;
				} else {
					pdfu_responder_state =
						PDFU_STATE_ABORT;
				}
			}
			break;
		case PDFU_STATE_DATA_RESP:
			send_pdfu_response(RES_PDFU_DATA, 0);
			PDFUNextRequestRcvd_timer = PARAM_tPDFUNextRequestRcvd;
			pdfu_responder_state = PDFU_STATE_DATA;
			break;
		case PDFU_STATE_VALIDATE:
			pdfu_responder_prev_state = PDFU_STATE_DATA;
			pdfu_responder_state = PDFU_STATE_VALIDATE;
			pdfu_responder_phase = PHASE_Validation;
			break;
		case PDFU_STATE_DATA_PAUSE:
			send_pdfu_response(RES_PDFU_DATA_PAUSE, 0);
			pdfu_responder_state = PDFU_STATE_DATA_PAUSE_WAIT_DATA;
			break;
		case PDFU_STATE_DATA_PAUSE_WAIT_DATA:
			/* Wait Data coming*/
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_responder_state = PDFU_STATE_IDLE;
			pdfu_responder_phase = PHASE_Enumeration;
			break;
		case PDFU_STATE_DATA_NR:
			pdfu_responder_state = PDFU_STATE_DATA;
			break;
		default:
			break;
		}
		break;
	case PHASE_Validation:
		if (pdfu_responder_prev_state != pdfu_responder_state) {
			if (pdfu_responder_prev_state == PDFU_STATE_DATA) {
				TRACE("Responder_Validation\n");
				ValidationResend_counter = ValidationResend;
			}
			pdfu_responder_prev_state = pdfu_responder_state;
		}

		switch (pdfu_responder_state) {
		case PDFU_STATE_VALIDATE:
			/* recv PDFU_DATA request*/
			if (Responder_ValidatePayload.WaitTime == 0 &&
				Responder_ValidatePayload.Flags == 1) {
				pdfu_responder_state = PDFU_STATE_MANIFESTATION;
				pdfu_responder_phase = PHASE_Manifestation;
			} else if (!PDFUNextRequestRcvd_timer) { /* timeout*/
				if (ValidationResend_counter) { /* retry*/
					ValidationResend_counter--;
					pdfu_responder_state =
						PDFU_STATE_VALIDATE_RESP;
				} else {
					pdfu_responder_state =
						PDFU_STATE_ABORT;
				}
			}
			break;
		case PDFU_STATE_VALIDATE_RESP:
			send_pdfu_response(RES_PDFU_VALIDATE, 0);
			PDFUNextRequestRcvd_timer = PARAM_tPDFUNextRequestRcvd;
			pdfu_responder_state = PDFU_STATE_VALIDATE;
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_responder_state = PDFU_STATE_IDLE;
			pdfu_responder_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}
		break;
	case PHASE_Manifestation:
		if (pdfu_responder_prev_state != pdfu_responder_state) {
			if (pdfu_responder_prev_state == PDFU_STATE_VALIDATE)
				TRACE("Responder_Manifestation\n");

			pdfu_responder_prev_state = pdfu_responder_state;
		}

		switch (pdfu_responder_state) {
		case PDFU_STATE_MANIFESTATION:
			/* do something for Manifestation*/
			pdfu_responder_prev_state = PDFU_STATE_IDLE;
			pdfu_responder_state = PDFU_STATE_IDLE;
			pdfu_responder_phase = PHASE_Enumeration;
			break;
		case PDFU_STATE_ABORT:
			TRACE("PDFU abort!\n");
			pdfu_responder_prev_state = PDFU_STATE_IDLE;
			pdfu_responder_state = PDFU_STATE_IDLE;
			pdfu_responder_phase = PHASE_Enumeration;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

unsigned char IsFwImgValid(void)
{
	if (((BYTE_CONST_SWAP(ResponseFwIdPayload.VID)) ==
		TEST_VENDOR_ID) &&
		((BYTE_CONST_SWAP(ResponseFwIdPayload.PID)) ==
		TEST_PRODUCT_ID)) {
		TRACE("Analogix Test Image!\n");
		return 1;
	}
	TRACE("non-Analogix vendor ID Image!\n");
	return 0;
}

unsigned char InitiateWaitTime(void)
{
	if (ResponseInitiatePayload.Status != 0)
		return 255;
	return ResponseInitiatePayload.WaitTime;
}

unsigned char DataWaitTime(void)
{
	if (ResponseDataPayload.Status != 0)
		return 255;
#if PDFU_V092
	if (ResponseDataPayload.WaitTime != 0 &&
		ResponseDataPayload.NumDataNR != 0)
		return 255;
	return ResponseDataPayload.WaitTime;
#else
	if (ResponseDataParam[0] != 0 && ResponseDataParam[1] != 0)
		return 255;
	return ResponseDataParam[0];
#endif
}

unsigned char ValidateWaitTime(void)
{
	if (ResponseValidatePayload.Status != 0)
		return 255;
#if PDFU_V092
	if (ResponseValidatePayload.WaitTime == 0 &&
		ResponseValidatePayload.Flags == 0)
		return 255;
	return ResponseValidatePayload.WaitTime;
#else
	if (ResponseValidateParam[0] == 0 && ResponseValidateParam[1] == 0)
		return 255;
	return ResponseValidateParam[0];
#endif
}

unsigned char send_pdfu_response(unsigned char type, unsigned char type_sop)
{
	unsigned char data_size;
	unsigned char buf_len = 0;

	switch (type) {
	case RES_GET_FW_ID:
		data_size = USB_PDFU_HEADER_SIZE +
			sizeof(struct PDFU_GET_FW_ID_RESP);
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
		/* 2 byte*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
		/* 2 byte*/
		USB_PDFU_HEADER((pd_chunk_buf + 2 + USB_PD_EXT_HEADER_SIZE),
			RES_GET_FW_ID);
		memcpy(pd_chunk_buf + 2 + USB_PD_EXT_HEADER_SIZE +
			USB_PDFU_HEADER_SIZE, &Responder_GetFwIdPayload,
			sizeof(struct PDFU_GET_FW_ID_RESP));
		break;
	case RES_PDFU_INITIATE:
		data_size = USB_PDFU_HEADER_SIZE +
			sizeof(struct PDFU_INITIATE_RESP);
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
			/* 2 byte*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
			/* 2 byte*/
		USB_PDFU_HEADER((pd_chunk_buf + 2 + USB_PD_EXT_HEADER_SIZE),
			RES_PDFU_INITIATE);
		memcpy(pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE,
			&Responder_InitiatePayload,
			sizeof(struct PDFU_INITIATE_RESP));
		break;
	case RES_PDFU_DATA:
		data_size = USB_PDFU_HEADER_SIZE +
			sizeof(struct PDFU_DATA_RESP);
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
			/* 2 byte*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
			/* 2 byte*/
		USB_PDFU_HEADER((pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE), RES_PDFU_DATA);
		memcpy(pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE,
			&Responder_DataPayload,
			sizeof(struct PDFU_DATA_RESP));
		break;
	case RES_PDFU_VALIDATE:
		data_size = USB_PDFU_HEADER_SIZE +
			sizeof(struct PDFU_VALIDATE_RESP);
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
			/* 2 byte*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
			/* 2 byte*/
		USB_PDFU_HEADER((pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE), RES_PDFU_VALIDATE);
		memcpy(pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE,
			&Responder_ValidatePayload,
			sizeof(struct PDFU_VALIDATE_RESP));
		break;
	case RES_PDFU_DATA_PAUSE: /* not implement*/
		data_size = USB_PDFU_HEADER_SIZE +
			sizeof(struct PDFU_DATA_PAUSE_RESP);
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
			/* 2 byte*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
			/* 2 byte*/
		USB_PDFU_HEADER((pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE), RES_PDFU_DATA_PAUSE);
		memcpy(pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE,
			&Responder_DataPausePayload,
			sizeof(struct PDFU_DATA_PAUSE_RESP));
		break;
	case RES_VENDOR_SPECIFIC: /* not implement*/
		break;
	default:
		break;
	}

	/* Padding*/
	if (buf_len & 0x03) {
		unsigned char delta = 4 - (buf_len & 0x03);

		memset(pd_chunk_buf + 2 + buf_len, 0, delta);
		buf_len += delta;
	}

	if (buf_len)
		return send_ext_msg(1, PD_EXT_FW_UPDATE_RESPONSE,
			pd_chunk_buf + 2, buf_len, type_sop);
	return 0;
}

unsigned char send_chunk_request(unsigned char chunk_num,
	unsigned char data_size)
{

	/*pd_chunk_buf[0] = 1; // is_ext*/
	/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
	USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 1, chunk_num, 1);
	pd_chunk_buf[4] = 0; /* Padding*/
	pd_chunk_buf[5] = 0; /* Padding*/

	return send_ext_msg(1, PD_EXT_FW_UPDATE_REQUEST, pd_chunk_buf + 2,
		4/*buf_len*/, 0/*type_sop*/);
}
unsigned char send_pdfu_request(unsigned char type, unsigned char type_sop)
{
	unsigned char data_size;
	unsigned char buf_len = 0;

	switch (type) {
	case GET_FW_ID:
		ResponseFwIdReady = 0;
		data_size = USB_PDFU_HEADER_SIZE;
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
		USB_PDFU_HEADER((pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE), type);
		break;
	case PDFU_INITIATE:
		ResponseInitiateReady = 0;
		data_size = USB_PDFU_HEADER_SIZE;
#if PDFU_V092
		data_size += sizeof(Initiator_InitiatePayload);
#endif
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);

#if PDFU_V092
		USB_PDFU_HEADER((pd_chunk_buf + 2 + USB_PD_EXT_HEADER_SIZE),
			type);
		memcpy(pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE,
			&Initiator_InitiatePayload,
			sizeof(Initiator_InitiatePayload));
#else
		USB_PDFU_HEADER((pd_chunk_buf + 2 +
			USB_PD_EXT_HEADER_SIZE),
			type, Initiator_InitiateParam[0],
			Initiator_InitiateParam[1]);
#endif
		break;
	case PDFU_DATA:
		ResponseDataReady = 0;
		SetDataBlock(DataBlockIndex);
		return SendBlock(PD_EXT_FW_UPDATE_REQUEST,
			pd_block_buf, 4 + DataBlockSize);
		break;
	case PDFU_VALIDATE:
		ResponseValidateReady = 0;
		data_size = USB_PDFU_HEADER_SIZE;
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
		USB_PDFU_HEADER((pd_chunk_buf + 2 + USB_PD_EXT_HEADER_SIZE),
			type);
		break;
	case PDFU_ABORT:
		data_size = USB_PDFU_HEADER_SIZE;
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
		USB_PDFU_HEADER((pd_chunk_buf + 2 + USB_PD_EXT_HEADER_SIZE),
			type);
		break;
	case PDFU_DATA_PAUSE:
		ResponseDataPauseReady = 0;
		data_size = USB_PDFU_HEADER_SIZE;
		buf_len = USB_PD_EXT_HEADER_SIZE + data_size;
		/*pd_chunk_buf[0] = 1; // is_ext*/
		/*pd_chunk_buf[1] = PD_EXT_FW_UPDATE_REQUEST; // type*/
		USB_PD_EXT_HEADER((pd_chunk_buf + 2), data_size, 0, 0, 1);
		USB_PDFU_HEADER((pd_chunk_buf + 2 + USB_PD_EXT_HEADER_SIZE),
			type);
		break;
	case PDFU_DATA_NR: /* not implement*/
		break;
	case VENDOR_SPECIFIC: /* not implement*/
		break;
	default:
		break;
	}

	/* Padding*/
	if (buf_len & 0x03) {
		unsigned char delta = 4 - (buf_len & 0x03);

		memset(pd_chunk_buf + 2 + buf_len, 0, delta);
		buf_len += delta;
	}

	if (buf_len)
		return send_ext_msg(1, PD_EXT_FW_UPDATE_REQUEST,
			pd_chunk_buf + 2, buf_len, type_sop);

	return 0;
}

unsigned char recv_pdfu_response(unsigned char *para, unsigned char para_len)
{
	struct PDFU_MSG_HEADER *pdfu_header =
		(struct PDFU_MSG_HEADER *)(para + USB_PD_EXT_HEADER_SIZE);
	unsigned char ret = 0;
#if DEBUG_MSG
	TRACE1("recv_pdfu_response: para_len = %d\n", para_len);
#else
	UNUSED_VAR(para_len);
#endif
	switch (pdfu_header->MessageType) {
	case RES_GET_FW_ID:
		memcpy(&ResponseFwIdPayload,
			(para + USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE),
			sizeof(struct PDFU_GET_FW_ID_RESP));
		ResponseFwIdReady = 1;
		break;
	case RES_PDFU_INITIATE:
		memcpy(&ResponseInitiatePayload,
			(para + USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE),
			sizeof(struct PDFU_INITIATE_RESP));
		ResponseInitiateReady = 1;
		break;
	case RES_PDFU_DATA:

#if PDFU_V092
		memcpy(&ResponseDataPayload,
			(para + USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE),
			sizeof(struct PDFU_DATA_RESP));
#else
		memcpy(ResponseDataParam,
			(para + USB_PD_EXT_HEADER_SIZE + 2),
			sizeof(ResponseDataParam));
		memcpy(&ResponseDataPayload,
			(para + USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE),
			sizeof(struct PDFU_DATA_RESP));
#endif
		ResponseDataReady = 1;
		break;
	case RES_PDFU_VALIDATE:
#if PDFU_V092
		memcpy(&ResponseValidatePayload,
			(para + USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE),
			sizeof(struct PDFU_VALIDATE_RESP));
#else
		memcpy(ResponseValidateParam,
			(para + USB_PD_EXT_HEADER_SIZE + 2),
			sizeof(ResponseValidateParam));
		memcpy(&ResponseValidatePayload,
			(para + USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE),
			sizeof(struct PDFU_VALIDATE_RESP));
#endif
		ResponseValidateReady = 1;
		break;
#if PDFU_V092
	case RES_PDFU_DATA_PAUSE:
		memcpy(&ResponseDataPausePayload,
			(para + USB_PD_EXT_HEADER_SIZE + USB_PDFU_HEADER_SIZE),
			sizeof(struct PDFU_DATA_PAUSE_RESP));
		ResponseDataPauseReady = 1;
		break;
#endif
	case RES_VENDOR_SPECIFIC:
		break;
	default:
		ret = 1;
		break;
	}
	return ret;
}

unsigned char recv_pdfu_request(unsigned char *para, unsigned int para_len)
{
	struct PDFU_MSG_HEADER *pdfu_header = (struct PDFU_MSG_HEADER *)(para);
	unsigned char ret = 0;
#if DEBUG_MSG
	TRACE1("recv_pdfu_request: para_len = %d\n", para_len);
#endif
	switch (pdfu_header->MessageType) {
	case GET_FW_ID:
#if DEBUG_MSG
		TRACE("recv_pdfu_request: GET_FW_ID\n");
#endif
		pdfu_responder_set_state(PDFU_STATE_GET_FW_ID);
		send_pdfu_response(RES_GET_FW_ID, 0);
		break;
	case PDFU_INITIATE:
#if DEBUG_MSG
		TRACE("recv_pdfu_request: PDFU_INITIATE\n");
#endif
		pdfu_responder_set_state(PDFU_STATE_INITIATE);
		send_pdfu_response(RES_PDFU_INITIATE, 0);
		break;
	case PDFU_DATA:
#if DEBUG_MSG
		TRACE("recv_pdfu_request: PDFU_DATA\n");
#endif
		pdfu_responder_set_state(PDFU_STATE_DATA);
#if PDFU_V092
	/* delay for write data*/
		Responder_DataPayload.WaitTime = PDFU_DATA_WRITE_DELAY;

		/* Little Endian*/
		DataBlockNum = para[USB_PDFU_HEADER_SIZE] +
		   (((unsigned int)(para[USB_PDFU_HEADER_SIZE + 1])) << 8) + 1;
		Responder_DataPayload.DataBlockNum = DataBlockNum;
		BYTE_SWAP(Responder_DataPayload.DataBlockNum);


		send_pdfu_response(RES_PDFU_DATA, 0);
		sim_write(DataBlockNum - 1,
			para + USB_PDFU_HEADER_SIZE + 2,
			para_len - USB_PDFU_HEADER_SIZE - 2);
#else
		Responder_DataParam[0] = 100; /* delay for write data*/

		Responder_DataPayload.DataBlockNum =
		    (((unsigned int)pdfu_header->Param1) << 8) +
		    pdfu_header->Param2 + 1;

		send_pdfu_response(RES_PDFU_DATA, 0);
		sim_write((((unsigned int)pdfu_header->Param1) << 8) +
			pdfu_header->Param2,
			para + USB_PDFU_HEADER_SIZE, para_len -
			USB_PDFU_HEADER_SIZE);
#endif
		break;
	case PDFU_VALIDATE:

		TRACE("recv_pdfu_request: PDFU_VALIDATE\n");

		pdfu_responder_set_state(PDFU_STATE_VALIDATE);
		send_pdfu_response(RES_PDFU_VALIDATE, 0);
		break;
	case PDFU_DATA_PAUSE:
		pdfu_responder_set_state(PDFU_STATE_DATA_PAUSE);
		break;
	case PDFU_ABORT:
#if DEBUG_MSG
		TRACE("recv_pdfu_request: PDFU_ABORT\n");
#endif
		pdfu_responder_set_state(PDFU_STATE_ABORT);
		break;
	case PDFU_DATA_NR:
#if DEBUG_MSG
		TRACE("recv_pdfu_request: PDFU_DATA_NR\n");
#endif
		pdfu_responder_set_state(PDFU_STATE_DATA_NR);
#if PDFU_V092
		Responder_DataPayload.WaitTime = 100; /* delay for write data*/
		/* Little Endian*/
		DataBlockNum = para[USB_PDFU_HEADER_SIZE]
		  + (((unsigned int)(para[USB_PDFU_HEADER_SIZE + 1])) << 8) + 1;
		Responder_DataPayload.DataBlockNum = DataBlockNum;
		BYTE_SWAP(Responder_DataPayload.DataBlockNum);

		sim_write(DataBlockNum - 1,
		   para + USB_PDFU_HEADER_SIZE + 2,
		   para_len - USB_PDFU_HEADER_SIZE - 2);
#endif
		break;
	case VENDOR_SPECIFIC:
	default:
		ret = 1;
		break;
	}

	return ret;
}

unsigned char SetDataBlock(unsigned int data_block_index)
{
	unsigned char ret = 0;

	if (data_block_index < DataBlockTotal) {
		DataBlockIndex = data_block_index;
		DataBlockSize = ((DataBlockIndex != DataBlockTotal - 1) ?
			DATA_BLOCK_SIZE : DataBlockSizeRemain);
		FwImageOffset = DataBlockIndex * DATA_BLOCK_SIZE;
#if DEBUG_MSG
		TRACE2("BlockIndex = %d (%d), ", DataBlockIndex, DataBlockSize);
		TRACE1("Offset = 0x%x\n", FwImageOffset);
#endif
#if PDFU_V092
		USB_PDFU_HEADER((pd_block_buf), PDFU_DATA);
		*(pd_block_buf + USB_PDFU_HEADER_SIZE) =
			(DataBlockIndex & 0xFF);
		*(pd_block_buf + USB_PDFU_HEADER_SIZE + 1) =
			(DataBlockIndex >> 8);

		if (DataBlockSize)
			memcpy(pd_block_buf + USB_PDFU_HEADER_SIZE + 2,
				FwImagePtr + FwImageOffset, DataBlockSize);
#else
		USB_PDFU_HEADER((pd_block_buf),
			PDFU_DATA, (DataBlockIndex >> 8),
			(DataBlockIndex & 0xFF));
		if (DataBlockSize)
			memcpy(pd_block_buf + USB_PDFU_HEADER_SIZE,
				FwImagePtr + FwImageOffset, DataBlockSize);
#endif
		ret = 1;
	}

	return ret;
}

/*extern unsigned char code OCM_CODE[3 * 9 + 27648 + 27648 / 8];test*/
#include "anx7625_ocm_code.c"
void InitDataBlock(void)
{
	FwImagePtr = OCM_CODE;
	FwImageSize = sizeof(OCM_CODE);
	if (FwImageSizeMax && (FwImageSizeMax < sizeof(OCM_CODE)))
		FwImageSize = FwImageSizeMax;

	FwImageOffset = 0;
	DataBlockSizeRemain = (FwImageSize % DATA_BLOCK_SIZE);
	DataBlockTotal = FwImageSize / DATA_BLOCK_SIZE +
		(DataBlockSizeRemain ? 1 : 1); /* less or zero*/
	DataBlockIndex = 0;
}

void TestFWUpdate(unsigned char flag)
{
	unsigned char ret;

	if (!flag)
		InitDataBlock();

	ret = SetDataBlock(DataBlockIndex);

	if (ret) {
		/*TRACE_ARRAY(pd_block_buf,4);*/
		TraceArray2(pd_block_buf + 4, DataBlockSize);
		SendBlock(PD_EXT_FW_UPDATE_REQUEST,
			pd_block_buf, 4 + DataBlockSize);

		if (DataBlockIndex < DataBlockTotal - 1) {
			DataBlockIndex++;
			TRACE("Next DataBlock\n");
		} else {
			TRACE("End DataBlock\n");
			/*break;*/
		}
	}

}

void PrintRecvPdfuData(unsigned char type)
{
	switch (type) {
	case RES_GET_FW_ID:
		if (ResponseFwIdReady) {
			TRACE("ResponseFwIdPayload:\n");
			TRACE1("Status     = 0x%02x\n",
				ResponseFwIdPayload.Status);

#if PDFU_V092
			TRACE1("VID        = 0x%04x\n",
				BYTE_CONST_SWAP(ResponseFwIdPayload.VID));
			TRACE1("PID        = 0x%04x\n",
				BYTE_CONST_SWAP(ResponseFwIdPayload.PID));
			TRACE1("HWRevision = 0x%02x\n",
				ResponseFwIdPayload.HWRevision);
			TRACE1("SiRevision = 0x%02x\n",
				ResponseFwIdPayload.SiRevision);
			TRACE1("FWVersion1 = 0x%04x\n",
				BYTE_CONST_SWAP(
				ResponseFwIdPayload.FWVersion1));
			TRACE1("FWVersion2 = 0x%04x\n",
				BYTE_CONST_SWAP(
				ResponseFwIdPayload.FWVersion2));
			TRACE1("FWVersion3 = 0x%04x\n",
				BYTE_CONST_SWAP(
				ResponseFwIdPayload.FWVersion3));
			TRACE1("FWVersion4 = 0x%04x\n",
				BYTE_CONST_SWAP(
				ResponseFwIdPayload.FWVersion4));
#else
			TRACE1("VID        = 0x%04x\n",
				ResponseFwIdPayload.VID);
			TRACE1("PID        = 0x%04x\n",
				ResponseFwIdPayload.PID);
			TRACE1("HWRevision = 0x%02x\n",
				ResponseFwIdPayload.HWRevision);
			TRACE1("SiRevision = 0x%02x\n",
				ResponseFwIdPayload.SiRevision);
			TRACE1("FWMajorRevision = 0x%02x\n",
				ResponseFwIdPayload.FWMajorRevision);
			TRACE1("FWMinorRevision = 0x%02x\n",
				ResponseFwIdPayload.FWMinorRevision);
#endif
			TRACE1("ImageBank = 0x%02x\n",
				ResponseFwIdPayload.ImageBank);
			TRACE1("Flags1 = 0x%02x\n",
				ResponseFwIdPayload.Flags1);
			TRACE1("Flags2 = 0x%02x\n",
				ResponseFwIdPayload.Flags2);
			TRACE1("Flags3 = 0x%02x\n",
				ResponseFwIdPayload.Flags3);
			TRACE1("Flags4 = 0x%02x\n",
				ResponseFwIdPayload.Flags4);
		} else
			TRACE("Not GET_FW_ID response yet!\n");
		break;
	case RES_PDFU_INITIATE:
		if (ResponseInitiateReady) {
			TRACE("ResponseInitiatePayload:\n");
			TRACE1("Status     = 0x%02x\n",
				ResponseInitiatePayload.Status);
			TRACE1("WaitTime   = 0x%02x\n",
				ResponseInitiatePayload.WaitTime);
			TRACE1("MaxImageSize[0] = 0x%02x\n",
				ResponseInitiatePayload.MaxImageSize[0]);
			TRACE1("MaxImageSize[1] = 0x%02x\n",
				ResponseInitiatePayload.MaxImageSize[1]);
			TRACE1("MaxImageSize[2] = 0x%02x\n",
				ResponseInitiatePayload.MaxImageSize[2]);
		} else
			TRACE("Not PDFU_INITIATE response yet!\n");
		break;
	case RES_PDFU_DATA:
		if (ResponseDataReady) {
#if PDFU_V092
			TRACE("ResponseDataPayload:\n");
			TRACE1("Status		  = 0x%02x\n",
				ResponseDataPayload.Status);
			TRACE1("WaitTime      = 0x%02x\n",
				ResponseDataPayload.WaitTime);
			TRACE1("NumDataNR     = 0x%02x\n",
				ResponseDataPayload.NumDataNR);
			TRACE1("DataBlockNum  = 0x%04x\n",
				BYTE_CONST_SWAP(
				ResponseDataPayload.DataBlockNum));
#else
			TRACE("ResponseDataParam:\n");
			TRACE1("Param1 = 0x%02x\n",
				ResponseDataParam[0]);
			TRACE1("Param2 = 0x%02x\n",
				ResponseDataParam[1]);
			TRACE("ResponseDataPayload:\n");
			TRACE1("Status        = 0x%02x\n",
				ResponseDataPayload.Status);
			TRACE1("DataBlockNum  = 0x%04x\n",
				ResponseDataPayload.DataBlockNum);
#endif
		} else
			TRACE("Not PDFU_DATA response yet!\n");
		break;
	case RES_PDFU_VALIDATE:
		if (ResponseValidateReady) {
#if PDFU_V092
			TRACE("ResponseValidatePayload:\n");
			TRACE1("Status	 = 0x%02x\n",
				ResponseValidatePayload.Status);

			TRACE1("WaitTime = 0x%02x\n",
				ResponseValidatePayload.WaitTime);
			TRACE1("Flags    = 0x%02x\n",
				ResponseValidatePayload.Flags);

#else
			TRACE("ResponseValidateParam:\n");
			TRACE1("Param1 = 0x%02x\n",
				ResponseValidateParam[0]);
			TRACE1("Param2 = 0x%02x\n",
				ResponseValidateParam[1]);
			TRACE("ResponseValidatePayload:\n");
			TRACE1("Status        = 0x%02x\n",
				ResponseValidatePayload.Status);
#endif
		} else
			TRACE("Not PDFU_VALIDATE response yet!\n");
		break;
#if PDFU_V092
	case RES_PDFU_DATA_PAUSE:
		if (ResponseDataPauseReady) {
			TRACE("ResponseDataPausePayload:\n");
			TRACE1("Status		  = 0x%02x\n",
				ResponseDataPausePayload.Status);
		} else
			TRACE("Not PDFU_DATA_PAUSE response yet!\n");
		break;
#endif
	default:
		TRACE("Unknow type!\n");
		break;
	}

}

#endif /* USE_PDFU*/

#endif /* USE_PD30*/
