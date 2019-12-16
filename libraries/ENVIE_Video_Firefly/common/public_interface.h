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

#ifndef PUBLIC_INTERFACE_H
#define PUBLIC_INTERFACE_H

typedef enum {
	TYPE_PWR_SRC_CAP = 0x00,
	TYPE_PWR_SNK_CAP = 0x01,
	TYPE_DP_SNK_IDENDTITY= 0x02,
	TYPE_SVID = 0x03,
	TYPE_GET_DP_SNK_CAP = 0x04,
	TYPE_ACCEPT = 0x05,
	TYPE_REJECT  = 0x06,
	TYPE_DP_SNK_STATUS = 0x07,
	TYPE_PSWAP_REQ = 0x10,
	TYPE_DSWAP_REQ = 0x11,
	TYPE_GOTO_MIN_REQ =  0x12,
	TYPE_VCONN_SWAP_REQ = 0x13,
	TYPE_VDM = 0x14,
	TYPE_DP_SNK_CFG = 0x15,
	TYPE_PWR_OBJ_REQ = 0x16,
	TYPE_PD_STATUS_REQ = 0x17,
	TYPE_DP_ALT_ENTER = 0x19, 
	TYPE_DP_ALT_EXIT = 0x1A, 
	TYPE_GET_SNK_CAP = 0x1B, 
	TYPE_SOP_PRIME = 0x1C,
	TYPE_SOP_DOUBLE_PRIME = 0x1D,
	TYPE_RESPONSE_TO_REQ = 0xF0,
	TYPE_SOFT_RST = 0xF1,
	TYPE_HARD_RST = 0xF2,
	TYPE_RESTART = 0xF3,
#ifdef USE_PD30
	TYPE_EXT_SRC_CAP = 0xA1, // Source_Capabilities_Extended
	TYPE_EXT_SRC_STS = 0xA2, // Source_Status
	TYPE_EXT_GET_BATT_CAP  = 0xA3, // Get_Battery_Cap
	TYPE_EXT_GET_BATT_STS = 0xA4, // Get_Battery_ Status
	TYPE_EXT_BATT_CAP = 0xA5, // Battery_Capabilities
	TYPE_EXT_GET_MFR_INFO = 0xA6, // Get_Manufacturer_Info
	TYPE_EXT_MFR_INFO = 0xA7, // Manufacturer_Info
	TYPE_EXT_PDFU_REQUEST = 0xA8, // FW update Request
	TYPE_EXT_PDFU_RESPONSE = 0xA9, // FW update Response
	TYPE_EXT_BATT_STS = 0xAA, // PD_DATA_BATTERY_STATUS
	TYPE_EXT_ALERT = 0xAB, // PD_DATA_ALERT
	TYPE_EXT_NOT_SUPPORTED = 0xAC, // PD_CTRL_NOT_SUPPORTED
	TYPE_EXT_GET_SRC_CAP = 0xAD, // PD_CTRL_GET_SOURCE_CAP_EXTENDED
	TYPE_EXT_GET_SRC_STS = 0xAE, // PD_CTRL_GET_STATUS
	TYPE_EXT_FR_SWAP = 0xAF, // PD_CTRL_FR_SWAP
	TYPE_FR_SWAP_SIGNAL = 0xB0, // Fast Role Swap signal
	TYPE_GET_SINK_CAP_EXT = 0xB1, //PD_CTRL_GET_SINK_CAP_EXTENDED
	TYPE_EXT_SINK_CAP_EXT = 0xB2, // SINK_Capabilities_Extended
#endif
#if 1 // debug
	TYPE_GET_VAR = 0xFC, // get variable value
	TYPE_SET_VAR = 0xFD, // set variable value
#endif
	
} PD_MSG_TYPE;

typedef enum {
// for TYPE_GET_VAR and TYPE_SET_VAR
// use correct offset and length for each variable type
// do not use no define variable type
	IF_VAR_fw_var_reg = 0x10, // offset:0, length:8
	IF_VAR_pd_src_pdo = 0x11, // offset:0, length:28 (VDO_SIZE)
	IF_VAR_pd_snk_pdo = 0x12, // offset:0, length:28 (VDO_SIZE)
	IF_VAR_pd_rdo_bak = 0x13, // offset:0, length:4 (VDO_LEN)
	IF_VAR_pd_rdo = 0x14, // offset:0, length:4 (VDO_LEN)
	IF_VAR_DP_caps = 0x15, // offset:0, length:4 (VDO_LEN)
	IF_VAR_configure_DP_caps = 0x16, // offset:0, length:4 (VDO_LEN)
	IF_VAR_src_dp_status = 0x17, // offset:0, length:4 (VDO_LEN)
	IF_VAR_sink_svid_vdo = 0x18, // offset:0, length:4 (VDO_LEN)
	IF_VAR_sink_identity = 0x19, // offset:0, length:16
} PD_MSG_VAR_TYPE;

typedef enum {
	AttachWait_SNK = 0x00,
	AttachWait_SRC = 0x01,
	TRY_SNK= 0x02,
	TRY_SRC = 0x03,
	Attached_SNK  = 0x04,
	Attached_SRC = 0x05,
	Unattached_SNK  = 0x06,
	Unattached_SRC = 0x07,
    TRY_SNK_WaitVbus=0x08,
	TRY_SNK_WaitRp=0x09,
	TRY_SRC_WaitRd=0X0A,
	Error_SNK=0x0B

	
} TYPEC_STATE;


/* PDO : Power Data Object */
/*
* 1. The vSafe5V Fixed Supply Object shall always be the first object.
* 2. The remaining Fixed Supply Objects,
*    if present, shall be sent in voltage order; lowest to highest.
* 3. The Battery Supply Objects,
*    if present shall be sent in Minimum Voltage order; lowest to highest.
* 4. The Variable Supply (non battery) Objects,
*    if present, shall be sent in Minimum Voltage order; lowest to highest.
*/
#define PDO_TYPE_FIXED    ((unsigned long)0 << 30)
#define PDO_TYPE_BATTERY  ((unsigned long)1 << 30)
#define PDO_TYPE_VARIABLE ((unsigned long)2 << 30)
#define PDO_TYPE_MASK     ((unsigned long)3 << 30)

#define PDO_FIXED_DUAL_ROLE ((unsigned long)1 << 29) /* Dual role device */
#define PDO_FIXED_SUSPEND   ((unsigned long)1 << 28) /* USB Suspend supported */
#define PDO_FIXED_EXTERNAL  ((unsigned long)1 << 27) /* Externally powered */
#define PDO_FIXED_COMM_CAP  ((unsigned long)1 << 26) /* USB Communications Capable */
#define PDO_FIXED_DATA_SWAP ((unsigned long)1 << 25) /* Data role swap command supported */
#define PDO_FIXED_VOLT(mv)  (unsigned long)((((unsigned long)mv)/50) << 10) /* Voltage in 50mV units */
#define PDO_FIXED_CURR(ma)  (unsigned long)((((unsigned long)ma)/10))  /* Max current in 10mA units */

/*build a fixed PDO packet*/
#define PDO_FIXED(mv, ma, flags) (PDO_FIXED_VOLT(mv) |\
				  PDO_FIXED_CURR(ma) | (flags))

/*Pos in Data Object, the first index number begin from 0 */
#define PDO_INDEX(n, dat)  (dat << (n * PD_ONE_DATA_OBJECT_SIZE*sizeof(unsigned char)) )


#define PDO_VAR_MAX_VOLT(mv) (((((unsigned long)mv) / 50) & 0x3FF) << 20)
#define PDO_VAR_MIN_VOLT(mv) (((((unsigned long)mv) / 50) & 0x3FF) << 10)
#define PDO_VAR_OP_CURR(ma) (((((unsigned long)ma) / 10) & 0x3FF) << 0)

#define PDO_VAR(min_mv, max_mv, op_ma) \
(PDO_VAR_MIN_VOLT(min_mv) | \
PDO_VAR_MAX_VOLT(max_mv) | \
PDO_VAR_OP_CURR(op_ma) | \
PDO_TYPE_VARIABLE)

#define PDO_BATT_MAX_VOLT(mv) (((((unsigned long)mv) / 50) & 0x3FF) << 20)
#define PDO_BATT_MIN_VOLT(mv) (((((unsigned long)mv) / 50) & 0x3FF) << 10)
#define PDO_BATT_OP_POWER(mw) (((((unsigned long)mw) / 250) & 0x3FF))

#define PDO_BATT(min_mv, max_mv, op_mw) \
(PDO_BATT_MIN_VOLT(min_mv) | \
PDO_BATT_MAX_VOLT(max_mv) | \
PDO_BATT_OP_POWER(op_mw) | \
PDO_TYPE_BATTERY)


#define GET_PDO_TYPE(PDO) (((unsigned long)PDO & PDO_TYPE_MASK) >> 30)
#define GET_PDO_FIXED_DUAL_ROLE(PDO) (((unsigned long)PDO & PDO_FIXED_DUAL_ROLE) >> 29)
#define GET_PDO_FIXED_SUSPEND(PDO) (((unsigned long)PDO & PDO_FIXED_SUSPEND) >> 28)
#define GET_PDO_FIXED_EXTERNAL(PDO) (((unsigned long)PDO & PDO_FIXED_EXTERNAL) >> 27)
#define GET_PDO_FIXED_COMM_CAP(PDO) (((unsigned long)PDO & PDO_FIXED_COMM_CAP) >> 26)
#define GET_PDO_FIXED_DATA_SWAP(PDO) (((unsigned long)PDO & PDO_FIXED_DATA_SWAP) >> 25)
#define GET_PDO_FIXED_PEAK_CURR(PDO) (((unsigned long)PDO >> 20) & 0x03)

#define GET_PDO_FIXED_VOLT(PDO) ((((unsigned long)PDO >> 10) & 0x3FF) * 50)
#define GET_PDO_FIXED_CURR(PDO) (((unsigned long)PDO & 0x3FF) * 10)

#define GET_VAR_MAX_VOLT(PDO) ((((unsigned long)PDO >> 20) & 0x3FF) * 50)
#define GET_VAR_MIN_VOLT(PDO) ((((unsigned long)PDO >> 10) & 0x3FF) * 50)
#define GET_VAR_MAX_CURR(PDO) (((unsigned long)PDO & 0x3FF) * 10)

#define GET_BATT_MAX_VOLT(PDO) ((((unsigned long)PDO >> 20) & 0x3FF) * 50)
#define GET_BATT_MIN_VOLT(PDO) ((((unsigned long)PDO >> 10) & 0x3FF) * 50)
#define GET_BATT_OP_POWER(PDO) ((((unsigned long)PDO) & 0x3FF) * 250) 


/* RDO : Request Data Object */
#define RDO_OBJ_POS(n)             (((unsigned long)(n) & 0x7) << 28)
#define RDO_POS(rdo)               ((((32)rdo) >> 28) & 0x7)
#define RDO_GIVE_BACK              ((unsigned long)1 << 27)
#define RDO_CAP_MISMATCH           ((unsigned long)1 << 26)
#define RDO_COMM_CAP               ((unsigned long)1 << 25)
#define RDO_NO_SUSPEND             ((unsigned long)1 << 24)
#define RDO_FIXED_VAR_OP_CURR(ma)  (((((unsigned long)ma) / 10) & 0x3FF) << 10)
#define RDO_FIXED_VAR_MAX_CURR(ma) (((((unsigned long)ma) / 10) & 0x3FF) << 0)

#define RDO_BATT_OP_POWER(mw)      (((((unsigned long)mw) / 250) & 0x3FF) << 10)
#define RDO_BATT_MAX_POWER(mw)     (((((unsigned long)mw) / 250) & 0x3FF) << 0)

#define RDO_FIXED(n, op_ma, max_ma, flags) \
				(RDO_OBJ_POS(n) | (flags) | \
				RDO_FIXED_VAR_OP_CURR(op_ma) | \
				RDO_FIXED_VAR_MAX_CURR(max_ma))

//Added RDO_BATT for OHO-405
#define RDO_BATT(n, op_mw, max_mw, flags) \
			(RDO_OBJ_POS(n) | (flags)|\
			RDO_BATT_OP_POWER(op_mw)|\
			RDO_BATT_MAX_POWER(max_mw))


unsigned char send_pd_msg( PD_MSG_TYPE type, const char *buf, unsigned char size);

unsigned char dispatch_rcvd_pd_msg(PD_MSG_TYPE type, void *para, unsigned char para_len);


#endif  // end of PUBLIC_INTERFACE_H definition
