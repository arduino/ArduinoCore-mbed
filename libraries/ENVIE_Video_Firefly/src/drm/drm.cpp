/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V1.0

Filename : drm.c

Project  : ANX7625 

Created  : 08 Nov. 2016

Devices  : ANX7625

Toolchain: Keil
 
Description:

Revision History:

******************************************************************************/
#include <string.h>
#include "drm.h"
#include "drm_reg.h"
#include  "config.h"
#include "MI2_REG.h"
#include "REG_DRV.h"
#include "debug.h"
#include "debug/CmdHandler.h"

unsigned char xdata tmp_190b[155 + 3 + 32];

static void drm_SendCommand(unsigned int command)
{
	unsigned int cmd;
	
	ReadWordReg(RX_P1, DRM_COMMAND, &cmd);
	cmd &= (M2_Auth_Okay |Get_Auth_Status);

	/** Send Command: */
	cmd |= command;
	TRACE1("@>. Send Command: [%04x]. \n",(unsigned int)cmd);
	WriteWordReg(RX_P1, DRM_COMMAND, cmd);

	/** Set INTR */
	WriteReg(RX_P1, DRM_Interrupt, INTR);	
}

static void drm_ClearCommand(unsigned int command)
{
	unsigned int cmd;
	
	/** Clear Command: */
	ReadWordReg(RX_P1, DRM_COMMAND, &cmd);
	cmd &= ~(Auth_Status_Read | Status_Read);
	cmd &= ~command;
	WriteWordReg(RX_P1, DRM_COMMAND, cmd);
}

static void drm_GetAuthStatus(void)
{
	unsigned int status;
	unsigned char val;

	ReadWordReg(RX_P1, Auth_Status, &status);
	TRACE1("@>. Auth Status = %04x \n", (unsigned int)status);

	val = (status & Authentication_Status) >> 0;
	if (val == 0x00) {
		TRACE("@>. Unauthenticated. \n");
	} else if (val == 0x01) {
		TRACE("@>. Authentication in Progress. \n");
	} else if (val == 0x02) {
		TRACE("@>. Authentication was successful in HDCP 1.x. \n");

		drm_SendCommand(Start_M2_Auth);
	} else if (val == 0x03) {
		TRACE("@>. Authentication was successful in HDCP 2.2. \n");

		drm_SendCommand(Start_M2_Auth);
	} else if (val == 0x04) {
		TRACE("@>. Authentication failed. \n");
	} else if (val == 0x05) {
		TRACE("@>. Re-authenticating. \n");
	} else if (val == 0x06) {
		TRACE("@>. Authentication.KSV.Timeout. \n");
	}
	
	val = (status & Packet_Inactivity_Timer) >> 3;
	if (val == 0x00) {
		TRACE("@>. Packet Inactivity Timer not Expired. \n");
	} else if (val == 0x01) {
		TRACE("@>. Packet Inactivity Timer Expired. \n");
	}
	
	val = (status & Unstable_HS_Link_Timer) >> 4;
	if (val == 0x00) {
		TRACE("@>. Unstable HS Link Timer not Expired. \n");
	} else if (val == 0x01) {
		TRACE("@>. Unstable HS Link Timer Expired. \n");
	}
	
	val = (status & MIN_ENC_LVL) >> 5;
	if (val == 0x00) {
		TRACE("@>. There is no minimum encryption level enforcement in process. \n");
	} else if (val == 0x01) {
		TRACE("@>. There is minimum encryption level enforcement in process. \n");
	}
	
	val = (status & Encrypton_Status) >> 6;
	if (val == 0x00) {
		TRACE("@>. No encryption. \n");
	} else if (val == 0x01) {
		TRACE("@>. Stream is encrypted. \n");
	}
	
	val = (status & Repeater_Mode) >> 7;
	if (val == 0x00) {
		TRACE("@>. There are no repeaters in the chain. \n");
	} else if (val == 0x01) {
		TRACE("@>. There is at least one repeaters in the chain. \n");
	}
	
	val = (status & Status_Lock) >> 8;
	if (val == 0x00) {
		TRACE("@>. All I2C COMMAND are permitted. \n");
	} else if (val == 0x01) {
		TRACE("@>. I2C COMMAND are restricted to DSI mode commands. \n");
	}
	
	val = (status & Enc_Level_Error) >> 9;
	if (val == 0x00) {
		TRACE("@>. No error. \n");
	} else if (val == 0x01) {
		TRACE("@>. Error. \n");
	}
	
	val = (status & MI_2_Auth_Status) >> 10;
	if (val == 0x00) {
		TRACE("@>. MI-2 Unauthenticated. \n");
	} else if (val == 0x01) {
		TRACE("@>. MI-2_Auth_Okay. \n");
	} else if (val == 0x02) {
		TRACE("@>. MI-2_Auth_Fail. \n");
	}
}

static unsigned char compare_hash_value(void)
{
	unsigned char i;
	unsigned char up_hash, down_hash;

	for(i = 0; i < 32; i++)
	{
		ReadReg(RX_P1, Up_HASH + i, &up_hash);
		if (i < 16) {
			ReadReg(RX_P1, Down_HASH1 + i, &down_hash);
			if (up_hash !=  down_hash)
				return 1;
		} else {
			ReadReg(RX_P1, Down_HASH2 + i - 16, &down_hash);
			if (up_hash !=  down_hash)
				return 1;
		}
	}

	return 0;
}

static void drm_UpHASHReady(void)
{
	static unsigned char mismatch_cnt = 0;
	unsigned char *ptr = (unsigned char *)tmp_190b;
	unsigned int command;

	memset(ptr, 0x00, 12 + 32);

	ReadBlockReg(RX_P1, OUI, 3, ptr);
	TRACE3("@>. OUI = %02x%02x%02x \n", (unsigned int)ptr[2], (unsigned int)ptr[1], (unsigned int)ptr[0]);

	ReadBlockReg(RX_P1, Chip_ID, 2, ptr + 3);
	TRACE2("@>. Chip_ID = %02x%02x \n", (unsigned int)ptr[4], (unsigned int)ptr[3]);

	ReadBlockReg(RX_P1, Hard_Rev, 1, ptr + 5);
	TRACE1("@>. Hard_Rev = %02x \n", (unsigned int)ptr[5]);

	ReadBlockReg(RX_P1, Soft_Rev, 1, ptr + 6);
	TRACE1("@>. Soft_Rev = %02x \n", (unsigned int)ptr[6]);

	ReadBlockReg(RX_P1, Auth_Status, 2, ptr + 7);
	TRACE2("@>. Auth_Status = %02x%02x \n", (unsigned int)ptr[8], (unsigned int)ptr[7]);

	// DRM Engine get Checksum
	ReadBlockReg(RX_P2, 0x48, 3, ptr + 9);
	TRACE3("@>. Checksum = %02x%02x%02x \n", (unsigned int)ptr[9], (unsigned int)ptr[10], (unsigned int)ptr[11]);

	ReadWordReg(RX_P1, DRM_COMMAND, &command);

	/* 
	*  DRM Engine hash(OUI || Chip_ID || Hard_Rev) ||
	*  Soft_Rev || Auth_Status || checksum)
	*  Compare Up_HASH to local hash
	*/
	//if (compare_hash_value()) {//Compare = FALSE
	if (0) {//Compare = FALSE
		if ((++mismatch_cnt) >= 3) {
			TRACE("@>. DRM Engine disable video. \n");
			mismatch_cnt = 0;
		}	else {
			if (command & Get_Auth_Status)
			{
				drm_ClearCommand(Get_Auth_Status);
				drm_SendCommand(Get_Auth_Status);
			}
			else
				drm_SendCommand(Start_M2_Auth);
		}
	}	
	else 
	{//Compare = TRUE
		mismatch_cnt = 0;
		if (command & Get_Auth_Status)
		{
			drm_ClearCommand(Get_Auth_Status);
			drm_GetAuthStatus();
		}
		else
		{
			/*
			*  hash(Enc_Level || checksum)
			*  Write Down_HASH(1 & 2)
			*/
			drm_SendCommand(M2_Auth_Okay | Hash_Ready);
		}
	}
}

static void drm_MI2AuthDone(void)
{
	unsigned int status;
	unsigned char val;

	ReadWordReg(RX_P1, Auth_Status, &status);
	TRACE1("@>. Auth Status = %04x \n", (unsigned int)status);

	val = (status & MI_2_Auth_Status) >> 10;
	if (val == 0x00) {
		TRACE("@>. MI-2 Unauthenticated. \n");
	} else if (val == 0x01)	{
		TRACE("@>. MI-2_Auth_Okay. \n");
		drm_SendCommand(Get_KSVs);
	}	else if (val == 0x02)	{
		TRACE("@>. MI-2_Auth_Fail. \n");
		TRACE("@>. DRM Engine disable video. \n");
	}
}

static void drm_KSVReady(void)
{
	unsigned char *ptr = (unsigned char *)tmp_190b;
	unsigned char val;
	unsigned char dev_count, last_ksv_block = 0;
	static unsigned char current_dev_count = 0;

	ReadReg(RX_P1, Num_KSVs, &val);
	dev_count = val & Total_RXIDs;

	if ((dev_count - current_dev_count) <= 3) {
		TRACE3("$$. %u : %u : %u \n",(unsigned int)dev_count, (unsigned int)current_dev_count, (unsigned int)(dev_count - current_dev_count));
		ReadBlockReg(RX_P1, KSV_Buffer, (dev_count - current_dev_count) * 5, ptr + current_dev_count * 5);

		current_dev_count = 0;
		last_ksv_block = 1;
	}	else {
		TRACE3("##. %u : %u : %u \n",(unsigned int)dev_count, (unsigned int)current_dev_count, (unsigned int)(dev_count - current_dev_count));
		ReadBlockReg(RX_P1, KSV_Buffer, 15, ptr + current_dev_count * 5);

		current_dev_count += 3;
		drm_SendCommand(KSV_Block_Received);
	}
	
	//if (val & Last_KSV_Block) {
	if (last_ksv_block) {
		/* 
		*  DRM Engine hash(KSVs || checksum)
		*  Compare Up_HASH to local hash
		*/

		ReadReg(RX_P1, Enc_Level, &val);
		val &= ~KSV_Check;
		//if (compare_hash_value()) {//Compare = FALSE
		if (0) {//Compare = FALSE
				val |= (KSV_Fail << 2);
		}	else {//Compare = TRUE
				val |= (KSV_Okay << 2);
		}
		WriteReg(RX_P1, Enc_Level, val);
		
		drm_SendCommand(M2_Auth_Okay);
	}
}

static void drm_RequestChecksum(void)
{
	unsigned char val;
	unsigned int command;

	sp_write_reg_and(RX_P2, 0x22, 0x30);
	sp_write_reg_or(RX_P2, 0x22, 0x30);

	ReadWordReg(RX_P1, DRM_COMMAND, &command);

	if (command & M2_Auth_Okay)
	{
		/*
		*  hash(Enc_Level || checksum)
		*  Write Down_HASH(1 & 2)
		*/
	}
	else
	{
		/*
		*  Write Enc_Level
		*/
		ReadReg(RX_P1, Enc_Level, &val);
		val &= ~Encrypt_Level_Required;
		val |= ((val & Try_Enc_Level) >> 5);
		WriteReg(RX_P1, Enc_Level, val);
	}

	drm_SendCommand(Start_Checksum);
}

static void drm_DispatchStatus(void)
{
	unsigned char status;

	ReadReg(RX_P1, STATUS, &status);
	TRACE1("@>. Get Status:   [%02x] \n", (unsigned int)status);

	drm_SendCommand(Status_Read);
	mdelay(20);
	
	switch (status & 0x3F) {
		case Auth_Status_Changed:
			drm_SendCommand(Get_Auth_Status);
			break;
		case Up_HASH_Ready:
			drm_UpHASHReady();
			break;
		case MI_2_Auth_Done:
			drm_MI2AuthDone();
			break;
		case KSV_Ready:
			drm_KSVReady();
			break;
		case KSV_Buffer_Ready:
			break;
		case Request_Checksum:
			drm_RequestChecksum();
			break;
	}
}

void drm_MainProc(void)
{
	unsigned char intr;
	
	ReadReg(TX_P2, 0xF4, &intr);
	if (intr & 0x80) {
		WriteReg(TX_P2, 0xF4, 0x80);

		drm_DispatchStatus();
	}
}
//********************************************************************************
//********************************************************************************
//********************************************************************************
//********************************************************************************
