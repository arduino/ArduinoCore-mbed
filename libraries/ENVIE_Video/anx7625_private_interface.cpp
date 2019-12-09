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

#ifdef USE_PD30
#include "anx7625_pd30_message.h"
#include "anx7625_pd_fw_update.h"

#endif


/* init setting for TYPE_PWR_SRC_CAP */
static u32 init_src_caps[1] = {
	/*5V, 1.5A, Fixed */
	PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_1500MA, PDO_FIXED_FLAGS)
};

/* init setting for TYPE_PWR_SNK_CAP */
static u32 init_snk_cap[3] = {
	/*5V, 0.9A, Fixed */
	PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_900MA, PDO_FIXED_FLAGS),
	/*min 5V, max 20V, power 60W, battery */
	PDO_BATT(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_21V, PD_POWER_15W),
	/*min5V, max 5V, current 3A, variable */
	PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_21V, PD_CURRENT_3A)
};
/* init setting for TYPE_SVID */
static u8 init_svid[4] = { 0x00, 0x00, 0x01, 0xff };
/* init setting for TYPE_DP_SNK_IDENTITY */
static u8 init_snk_ident[16] = {
	0x00, 0x00, 0x00, 0xec,	/*snk_id_hdr */
	0x00, 0x00, 0x00, 0x00,	/*snk_cert */
	0x00, 0x00, 0x00, 0x00,	/*snk_prd*/
	0x39, 0x00, 0x00, 0x51		/*5snk_ama*/
};

static u8 pd_src_pdo_cnt = 2;
static u8 pd_src_pdo[VDO_SIZE] = {
	/*5V 0.9A , 5V 1.5 */
	0x5A, 0x90, 0x01, 0x2A, 0x96, 0x90, 0x01, 0x2A
};

static u8 pd_snk_pdo_cnt = 3;
static u8 pd_snk_pdo[VDO_SIZE];
u8 pd_rdo[PD_ONE_DATA_OBJECT_SIZE];
u8 DP_caps[PD_ONE_DATA_OBJECT_SIZE];
u8 configure_DP_caps[PD_ONE_DATA_OBJECT_SIZE];
u8 src_dp_caps[PD_ONE_DATA_OBJECT_SIZE];

#define INTR_MASK_SETTING 0x0

unsigned char InterfaceSendRetryCount = 3;

/**
 * @desc:   The interface AP will get the anx7625's data role
 *
 * @param:  none
 *
 * @return:  data role , dfp 1 , ufp 0, other error: -1, not ready
 *
 */
s8 get_data_role(void)
{
	u8 status;

	/*fetch the data role */
	status = ReadReg(OCM_SLAVE_I2C_ADDR1, SYSTEM_STSTUS);

	return (status & DATA_ROLE) != 0;

}

/**
 * @desc:   The interface AP will get the anx7625's power role
 *
 * @param:  none
 *
 * @return:  data role , source 1 , sink 0, other error, -1, not ready
 *
 */
s8 get_power_role(void)
{
	u8 status;

	/*fetch the power role */
	status = ReadReg(OCM_SLAVE_I2C_ADDR1, SYSTEM_STSTUS);

	return (status & VBUS_STATUS) == 0;
}

/**
 * @desc:   Interface AP fetch the source capability from anx7625
 *
 * @param:  pdo_buf: PDO buffer pointer of source capability in anx7625
 *          src_caps_size: source capability's size
 *
 * @return:  0: success 1: fail
 *
 */
u8 get_src_cap(const u8 *src_caps, u8 src_caps_size)
{

	return 1;
}

/**
 * @desc:   Interface that AP fetch the sink capability from
		anx7625's downstream device
 *
 * @param:  sink_caps: PDO buffer pointer of sink capability
 *            which will be responsed by anx7625's SINK Capablity Message
 *
 *          snk_caps_len: sink capability max length of the array
 *
 * @return:  sink capability array length>0: success. 1: fail
 *
 */
u8 get_snk_cap(u8 *snk_caps, u8 snk_caps_len)
{

	return 1;
}
/**
 * @desc:   The Interface AP set the source capability to anx7625
 *
 * @param:  pdo_buf: PDO buffer pointer of source capability,
 *                         which can be packed by PDO_FIXED_XXX macro
 *                eg: default5Vsafe src_cap(5V, 0.9A fixed) -->
 *			PDO_FIXED(5000,900, PDO_FIXED_FLAGS)
 *
 *                src_caps_size: source capability's size
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_src_cap(const u8 *src_caps, u8 src_caps_size)
{
	if (src_caps == NULL)
		return CMD_FAIL;
	if ((src_caps_size % PD_ONE_DATA_OBJECT_SIZE) != 0 ||
		(src_caps_size / PD_ONE_DATA_OBJECT_SIZE) >
		PD_MAX_DATA_OBJECT_NUM) {
		return CMD_FAIL;
	}
	memcpy(pd_src_pdo, src_caps, src_caps_size);
	pd_src_pdo_cnt = src_caps_size / PD_ONE_DATA_OBJECT_SIZE;

	/*send source capabilities message to anx7625 really */
	return interface_send_msg_timeout(TYPE_PWR_SRC_CAP, pd_src_pdo,
		pd_src_pdo_cnt * PD_ONE_DATA_OBJECT_SIZE, INTERFACE_TIMEOUT);
}

/**
 * @desc:   Interface that AP send(configure)
		the sink capability to anx7625's downstream device
 *
 * @param:  snk_caps: PDO buffer pointer of sink capability
 *
 *                snk_caps_size: sink capability length
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_snk_cap(const u8 *snk_caps, u8 snk_caps_size)
{
	memcpy(pd_snk_pdo, snk_caps, snk_caps_size);
	pd_snk_pdo_cnt = snk_caps_size / PD_ONE_DATA_OBJECT_SIZE;

	/*configure sink cap */
	return interface_send_msg_timeout(TYPE_PWR_SNK_CAP, pd_snk_pdo,
			pd_snk_pdo_cnt * 4, INTERFACE_TIMEOUT);
}

/**
 * @desc:   Interface that AP send(configure)
		the DP's sink capability to anx7625's downstream device
 *
 * @param:  dp_snk_caps: PDO buffer pointer of DP sink capability
 *
 *                dp_snk_caps_size: DP sink capability length
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_dp_snk_cfg(const u8 *dp_snk_caps, u8 dp_snk_caps_size)
{
	memcpy(configure_DP_caps, dp_snk_caps, dp_snk_caps_size);
	/*configure sink cap */
	return interface_send_msg_timeout(
		TYPE_DP_SNK_CFG, configure_DP_caps, 4, INTERFACE_TIMEOUT);
}

/**
 * @desc:   Interface that AP initialze
		the DP's capability of anx7625, as source device
 *
 * @param:  dp_caps: DP's capability  pointer of source
 *
 *                dp_caps_size: source DP capability length
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_src_dp_cap(const u8 *dp_caps, u8 dp_caps_size)
{
	if (dp_caps == NULL)
		return CMD_FAIL;
	if ((dp_caps_size % PD_ONE_DATA_OBJECT_SIZE) != 0 ||
	      (dp_caps_size / PD_ONE_DATA_OBJECT_SIZE) >
	      PD_MAX_DATA_OBJECT_NUM) {
		return CMD_FAIL;
	}

	memcpy(src_dp_caps, dp_caps, dp_caps_size);

	/*configure source DP cap */
	return interface_send_msg_timeout(TYPE_DP_SNK_IDENTITY,
			src_dp_caps, dp_caps_size, INTERFACE_TIMEOUT);
}


/**
 * @desc:   Interface that AP initialze
		the DP's sink identity of anx7625, as sink device
 *
 * @param:  snk_ident: DP's sink identity
 *
 *                snk_ident_size: DP's sink identity length
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_dp_snk_identity(const u8 *snk_ident, u8 snk_ident_size)
{
	return interface_send_msg_timeout(TYPE_DP_SNK_IDENTITY,
			(u8 *) snk_ident, snk_ident_size, INTERFACE_TIMEOUT);
}

/**
 * @desc:   The Interface AP set the VDM packet to anx7625
 *
 * @param:  vdm:  object buffer pointer of VDM
 *
 *                size: vdm packet size
 *
 *@return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_vdm(const u8 *vdm, u8 size)
{
	u8 tmp[32] = { 0 };

	if (vdm == NULL)
		return CMD_FAIL;
	if (size > 3 && size < 32) {
		memcpy(tmp, vdm, size);
		if (tmp[2] == 0x01 && tmp[3] == 0x00) {
			tmp[3] = 0x40;
			return interface_send_msg_timeout(TYPE_VDM, tmp, size,
				 INTERFACE_TIMEOUT);
		}
	}
	return 1;
}

/**
 * @desc:   The Interface AP set the SVID packet to anx7625
 *
 * @param:  svid:  object buffer pointer of svid
 *
 *                size: svid packet size
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_svid(const u8 *svid, u8 size)
{
	u8 tmp[4] = {
		0
	};
	if (svid == NULL || size != 4)
		return CMD_FAIL;
	memcpy(tmp, svid, size);
	return interface_send_msg_timeout(TYPE_SVID, tmp, size,
			INTERFACE_TIMEOUT);
}

/**
 * @desc:   Interface that AP send(configure)
		the sink capability to anx7625's downstream device
 *
 * @param:  snk_caps: PDO buffer pointer of sink capability
 *
 *                snk_caps_size: sink capability length
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 */
u8 send_rdo(const u8 *rdo, u8 size)
{
	u8 i;

	if (rdo == NULL)
		return CMD_FAIL;
	if ((size % PD_ONE_DATA_OBJECT_SIZE) != 0 ||
		(size / PD_ONE_DATA_OBJECT_SIZE) > PD_MAX_DATA_OBJECT_NUM) {
		return CMD_FAIL;
	}
	for (i = 0; i < size; i++)
		pd_rdo[i] = *rdo++;

	return interface_send_msg_timeout(TYPE_PWR_OBJ_REQ, pd_rdo, size,
			INTERFACE_TIMEOUT);
}



/**
 * @desc:   The interface AP will send  PR_Swap command to anx7625
 *
 * @param:  none
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_power_swap(void)
{
	return interface_pr_swap();
}

/**
 * @desc:   The interface AP will send DR_Swap command to anx7625
 *
 * @param:  none
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_data_swap(void)
{
	return interface_dr_swap();
}

/**
 * @desc:   The interface AP will send accpet command to anx7625
 *
 * @param:  none
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_accept(void)
{
	return interface_send_msg_timeout(TYPE_ACCEPT, 0, 0, INTERFACE_TIMEOUT);
}

/**
 * @desc:   The interface AP will send reject command to anx7625
 *
 * @param:  none
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_reject(void)
{
	return interface_send_msg_timeout(TYPE_REJECT, 0, 0, INTERFACE_TIMEOUT);
}

/**
 * @desc:   The interface AP will send soft reset command to anx7625
 *
 * @param:  none
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_soft_reset(void)
{
	return interface_send_soft_rst();
}

/**
 * @desc:   The interface AP will send hard reset command to anx7625
 *
 * @param:  none
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 *
 */
u8 send_hard_reset(void)
{
	return interface_send_hard_rst();
}

const char *interface_to_str(unsigned char header_type)
{
	return (header_type == TYPE_PWR_SRC_CAP) ? "src cap" :
	       (header_type == TYPE_PWR_SNK_CAP) ? "snk cap" :
	       (header_type == TYPE_PWR_OBJ_REQ) ? "RDO" :
	       (header_type == TYPE_DP_SNK_IDENTITY) ? "snk identity" :
	       (header_type == TYPE_SVID) ? "svid" :
	       (header_type == TYPE_PSWAP_REQ) ? "PR_SWAP" :
	       (header_type == TYPE_DSWAP_REQ) ? "DR_SWAP" :
	       (header_type == TYPE_GOTO_MIN_REQ) ? "GOTO_MIN" :
	       (header_type == TYPE_DP_ALT_ENTER) ? "DPALT_ENTER" :
	       (header_type == TYPE_DP_ALT_EXIT) ? "DPALT_EXIT" :
	       (header_type == TYPE_VCONN_SWAP_REQ) ? "VCONN_SWAP" :
	       (header_type == TYPE_GET_DP_SNK_CAP) ? "GET_SINK_DP_CAP" :
	       (header_type == TYPE_DP_SNK_CFG) ? "dp cap" :
	       (header_type == TYPE_SOFT_RST) ? "Soft Reset" :
	       (header_type == TYPE_HARD_RST) ? "Hard Reset" :
	       (header_type == TYPE_RESTART) ? "Restart" :
	       (header_type == TYPE_PD_STATUS_REQ) ? "PD Status" :
	       (header_type == TYPE_ACCEPT) ? "ACCEPT" :
	       (header_type == TYPE_REJECT) ? "REJECT" :
	       (header_type == TYPE_VDM) ? "VDM" :
	       (header_type == TYPE_RESPONSE_TO_REQ) ? "Response to Request" :
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

inline unsigned char cac_checksum(unsigned char *pSendBuf, unsigned char len)
{
	unsigned char i;
	unsigned char checksum;

	checksum = 0;
	for (i = 0; i < len; i++)
		checksum += *(pSendBuf + i);

	return (u8) (0 - checksum);
}



void send_initialized_setting(void)
{
	unsigned char send_init_setting_state, c;

	send_init_setting_state = 1;

	do {

		switch (send_init_setting_state) {
		default:
		case 0:
			break;
		case 1:
			/* send TYPE_PWR_SRC_CAP init setting */
			send_pd_msg(TYPE_PWR_SRC_CAP,
				(const uint8_t *)init_src_caps,
				sizeof(init_src_caps));
			send_init_setting_state++;
			break;
		case 2:
			c = ReadReg(OCM_SLAVE_I2C_ADDR2, InterfaceSendBuf_Addr);
			if (c == 0)
				send_init_setting_state++;
			else
				break;
		case 3:
			/* send TYPE_PWR_SNK_CAP init setting */
			send_pd_msg(TYPE_PWR_SNK_CAP,
				(const uint8_t *)init_snk_cap,
				sizeof(init_snk_cap));
			send_init_setting_state++;
			break;
		case 4:
			c = ReadReg(OCM_SLAVE_I2C_ADDR2, InterfaceSendBuf_Addr);
			if (c == 0)
				send_init_setting_state++;
			else
				break;
		case 5:
			/* send TYPE_DP_SNK_IDENTITY init setting */
			send_pd_msg(TYPE_DP_SNK_IDENTITY,
				init_snk_ident,
				sizeof(init_snk_ident));
			send_init_setting_state++;
			break;
		case 6:
			c = ReadReg(OCM_SLAVE_I2C_ADDR2, InterfaceSendBuf_Addr);
			if (c == 0)
				send_init_setting_state++;
			else
				break;
		case 7:
			/* send TYPE_SVID init setting */
			send_pd_msg(TYPE_SVID, init_svid, sizeof(init_svid));
			{/*Send SetVAR message to delay first CC message*/
			/*
			static u8 init_firstMsg_delay[5] = { 0x10, 0x00, 0x02, 0x01, 0x2f };
			interface_send_msg_timeout(0xfd, init_firstMsg_delay, sizeof(init_firstMsg_delay),
				INTERFACE_TIMEOUT);
			*/}
			send_init_setting_state++;
			break;
		case 8:
		case 9:
			send_init_setting_state = 0;
			break;
		}
	} while (send_init_setting_state != 0);

}

void chip_register_init(void)
{

#ifdef SUP_INT_VECTOR
/* set interrupt vector mask bit as platform needed  0: eable 1: disable*/
	WriteReg(OCM_SLAVE_I2C_ADDR1, INTERFACE_INTR_MASK, INTR_MASK_SETTING);
#else
	WriteReg(OCM_SLAVE_I2C_ADDR1, INTERFACE_INTR_MASK, 0xff);
#endif

#ifdef AUTO_RDO_ENABLE
	WriteReg(OCM_SLAVE_I2C_ADDR1, AUTO_PD_MODE,
		ReadReg(OCM_SLAVE_I2C_ADDR1, AUTO_PD_MODE) | AUTO_PD_ENABLE);
	/*Maximum Voltage in 100mV units*/
	WriteReg(OCM_SLAVE_I2C_ADDR1, MAX_VOLTAGE_SETTING, 0x32); /* 5V */
/* WriteReg(OCM_SLAVE_I2C_ADDR1,MAX_VOLTAGE_SETTING, 0xd2);   21V */
	/*Maximum Power in 500mW units*/
/*	WriteReg(OCM_SLAVE_I2C_ADDR1,MAX_POWER_SETTING, 0x0f);	  7.5W */
	WriteReg(OCM_SLAVE_I2C_ADDR1, MAX_POWER_SETTING, 0x78);	 /* 60W */
	/*Minimum Power in 500mW units*/
	WriteReg(OCM_SLAVE_I2C_ADDR1, MIN_POWER_SETTING, 0x002); /* 1W */
#endif

#if (defined(TRY_SNK_ENABLE) && (!defined(TRY_SRC_ENABLE)))
	WriteReg(OCM_SLAVE_I2C_ADDR1, AUTO_PD_MODE,
		ReadReg(OCM_SLAVE_I2C_ADDR1, AUTO_PD_MODE) | trysnk_en);
#endif

#if (defined(TRY_SRC_ENABLE) && (!defined(TRY_SNK_ENABLE)))
	WriteReg(OCM_SLAVE_I2C_ADDR1, AUTO_PD_MODE,
		ReadReg(OCM_SLAVE_I2C_ADDR1, AUTO_PD_MODE) | trysrc_en);
#endif

}


#define STS_HPD_CHANGE \
(((sys_status&HPD_STATUS) != (sys_sta_bak&HPD_STATUS)) ? HPD_STATUS_CHANGE:0)
#define STS_DATA_ROLE_CHANGE \
(((sys_status&DATA_ROLE) != (sys_sta_bak&DATA_ROLE)) ? DATA_ROLE_CHANGE:0)
#define STS_VCONN_CHANGE \
(((sys_status&VCONN_STATUS) != (sys_sta_bak&VCONN_STATUS)) ? VCONN_CHANGE:0)
#define STS_VBUS_CHANGE \
(((sys_status&VBUS_STATUS) != (sys_sta_bak&VBUS_STATUS)) ? VBUS_CHANGE:0)
static unsigned char sys_sta_bak;

void handle_intr_vector(void)
{
	unsigned char sys_status;
	u8 intr_vector = ReadReg(OCM_SLAVE_I2C_ADDR, INTERFACE_CHANGE_INT);
	u8 status;

	TRACE(" intr vector = 0x%x\n",  intr_vector);

	WriteReg(OCM_SLAVE_I2C_ADDR, INTERFACE_CHANGE_INT,
		intr_vector & (~intr_vector));

	if ((~INTR_MASK_SETTING) & intr_vector & CC_STATUS_CHANGE) {
		status = ReadReg(OCM_SLAVE_I2C_ADDR, NEW_CC_STATUS);
		pd_cc_status_default_func(status);
	}
	sys_status = ReadReg(OCM_SLAVE_I2C_ADDR, SYSTEM_STSTUS);
	TRACE(" system status = 0x%x\n",  sys_status);

	if ((~INTR_MASK_SETTING) &
		((intr_vector & VBUS_CHANGE) | STS_VBUS_CHANGE)) {
		status = ReadReg(OCM_SLAVE_I2C_ADDR, SYSTEM_STSTUS);
		pd_vbus_control_default_func(status & VBUS_STATUS);
		TRACE("VBUS_CHANGE 0x%x\n", status & VBUS_STATUS);
	}
	if ((~INTR_MASK_SETTING) &
		((intr_vector & VCONN_CHANGE) | STS_VCONN_CHANGE)) {
		status = ReadReg(OCM_SLAVE_I2C_ADDR, SYSTEM_STSTUS);
		pd_vconn_control_default_func(status & VCONN_STATUS);
		TRACE("VCONN_CHANGE 0x%x\n", status & VCONN_STATUS);
	}
	if ((~INTR_MASK_SETTING) &
		((intr_vector & DATA_ROLE_CHANGE) | STS_DATA_ROLE_CHANGE)) {
		status = ReadReg(OCM_SLAVE_I2C_ADDR, SYSTEM_STSTUS);
		pd_drole_change_default_func(status & DATA_ROLE);
		TRACE("DATA_ROLE_CHANGE 0x%x\n", status & DATA_ROLE);
	}

	if ((~INTR_MASK_SETTING) &
		((intr_vector & HPD_STATUS_CHANGE) | STS_HPD_CHANGE)) {
		status = ReadReg(OCM_SLAVE_I2C_ADDR, SYSTEM_STSTUS);
		dp_hpd_change_default_func(status & HPD_STATUS);
		TRACE("HPD_STATUS_CHANGE 0x%x\n", status & HPD_STATUS);
	}

	sys_sta_bak = sys_status;

}

void clear_sys_sta_bak(void)
{
	sys_sta_bak = 0;
}



void handle_msg_rcv_intr(void)
{
	if ((~INTR_MASK_SETTING)  & RECEIVED_MSG)
		polling_interface_msg(INTERACE_TIMEOUT_MS);

#ifdef USE_PD30
	pd_ext_message_handling();
#endif
}


/* Desc: send interface message to ocm
 * Args: timeout,  block timeout time
@return:  0: success, 1: reject, 2: fail, 3: busy
 */
u8 interface_send_msg_timeout(u8 type, u8 *pbuf, u8 len, int timeout_ms)
{
	unsigned char c, sending_len;
	unsigned char WriteDataBuf[32];


	/* full, return 0 */
	WriteDataBuf[0] = len + 1;	/* cmd */
	WriteDataBuf[1] = type;
	memcpy(WriteDataBuf + 2, pbuf, len);
	/* cmd + checksum */
	WriteDataBuf[len + 2] = cac_checksum(WriteDataBuf, len + 1 + 1);

	sending_len = WriteDataBuf[0] + 2;

	c = ReadReg(OCM_SLAVE_I2C_ADDR, InterfaceSendBuf_Addr);

	/* retry*/
	if (InterfaceSendRetryCount && c) {
		unsigned char count = InterfaceSendRetryCount;

		while (count) {
			usleep_range(1000, 1100);
			c = ReadReg(OCM_SLAVE_I2C_ADDR, InterfaceSendBuf_Addr);
			if (c == 0)
				break;
			count--;
		}
	}


	if (c == 0) {
		WriteBlockReg(OCM_SLAVE_I2C_ADDR,
			InterfaceSendBuf_Addr + 1 , sending_len - 1, &WriteDataBuf[1]);
		WriteReg(OCM_SLAVE_I2C_ADDR, InterfaceSendBuf_Addr, WriteDataBuf[0]);
	} else {
		TRACE("Tx Buf Full\n");
	}

	return 0;

}

/* Desc: polling private interface interrupt request message
 * Args: timeout,  block timeout time
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 * Interface's Format:
 *	1Byte Len + 1Byte Type + Len Bytes Data + 1Byte checksum
 */
u8 polling_interface_msg(int timeout_ms)
{
	unsigned char ReadDataBuf[32];
	unsigned char global_i, checksum;


	ReadBlockReg(OCM_SLAVE_I2C_ADDR, InterfaceRecvBuf_Addr,
		32, (unsigned char *)ReadDataBuf);

	if (ReadDataBuf[0] != 0) {

		WriteReg(OCM_SLAVE_I2C_ADDR, InterfaceRecvBuf_Addr, 0);


		checksum = 0;
		for (global_i = 0; global_i < ReadDataBuf[0] + 2; global_i++)
			checksum += ReadDataBuf[global_i];

		if (checksum == 0) {

			TRACE("\n>>%s\n", interface_to_str(ReadDataBuf[1]));
			dispatch_rcvd_pd_msg((enum PD_MSG_TYPE) ReadDataBuf[1],
				&(ReadDataBuf[2]), ReadDataBuf[0] - 1);
			return CMD_SUCCESS;


		} else {
			TRACE("checksum error!\n");
			return CMD_FAIL;
		}

	}

	return CMD_FAIL;

}

/* define max request current 3A and voltage 5V */
#define MAX_REQUEST_CURRENT 900
#define set_rdo_value(v0, v1, v2, v3)	\
	do {				\
		pd_rdo[0] = (v0);	\
		pd_rdo[1] = (v1);	\
		pd_rdo[2] = (v2);	\
		pd_rdo[3] = (v3);	\
	} while (0)

u8 sel_voltage_pdo_index = 0x02;
/* default request max RDO */
u8 build_rdo_from_source_caps(u8 obj_cnt, u8 *buf)
{
	u8 i = 0;
	u16 pdo_h = 0, pdo_l = 0, pdo_h_tmp = 0, pdo_l_tmp = 0;
	u16 max_request_ma = 0;
	u32 pdo_max, pdo_max_tmp = 0;

	pdo_max = 0;
	obj_cnt &= 0x07;

	/* find the max voltage pdo */
	for (i = 0; i < obj_cnt; i++) {
		pdo_l_tmp = buf[i * 4 + 0];
		pdo_l_tmp |= (u16) buf[i * 4 + 1] << 8;
		pdo_h_tmp = buf[i * 4 + 2];
		pdo_h_tmp |= (u16) buf[i * 4 + 3] << 8;

		/* get max voltage now */
		pdo_max_tmp =
		(u16) (((((pdo_h_tmp & 0xf) << 6) | (pdo_l_tmp >> 10)) &
			0x3ff) * 50);
		if (pdo_max_tmp > pdo_max) {
			pdo_max = pdo_max_tmp;
			pdo_l = pdo_l_tmp;
			pdo_h = pdo_h_tmp;
			sel_voltage_pdo_index = i;
		}
	}
	TRACE("maxV=%d, cnt %d index %d\n", pdo_max_tmp, obj_cnt,
	      sel_voltage_pdo_index);

	if ((pdo_h & (3 << 14)) != (PDO_TYPE_BATTERY >> 16)) {
		max_request_ma = (u16) ((pdo_l & 0x3ff) * 10);
		TRACE("maxMa %d\n", max_request_ma);

		/* less than 900mA */
		if (max_request_ma < MAX_REQUEST_CURRENT) {
			pdo_max =
			    RDO_FIXED(sel_voltage_pdo_index + 1, max_request_ma,
				max_request_ma, 0);
			pdo_max |= RDO_CAP_MISMATCH;
			set_rdo_value(pdo_max & 0xff, (pdo_max >> 8) & 0xff,
				(pdo_max >> 16) & 0xff,
				(pdo_max >> 24) & 0xff);
			return 1;
		}
		pdo_max =
		    RDO_FIXED(sel_voltage_pdo_index + 1,
			MAX_REQUEST_CURRENT, MAX_REQUEST_CURRENT,
			0);
		set_rdo_value(pdo_max & 0xff, (pdo_max >> 8) & 0xff,
			(pdo_max >> 16) & 0xff,
			(pdo_max >> 24) & 0xff);
		return 1;
	}
	pdo_max =
	    RDO_FIXED(sel_voltage_pdo_index + 1, MAX_REQUEST_CURRENT,
		MAX_REQUEST_CURRENT, 0);
	set_rdo_value(pdo_max & 0xff, (pdo_max >> 8) & 0xff,
		(pdo_max >> 16) & 0xff, (pdo_max >> 24) & 0xff);
	return 1;

	TRACE("RDO Mismatch !!!\n");
	set_rdo_value(0x0A, 0x28, 0x00, 0x10);

	return 0;
}

u32 change_bit_order(u8 *pbuf)
{
	return ((u32)pbuf[3] << 24) | ((u32)pbuf[2] << 16)
		| ((u32)pbuf[1] << 8) | pbuf[0];
}
static u8 pd_check_requested_voltage(u32 rdo)
{
	int max_ma = rdo & 0x3FF;
	int op_ma = (rdo >> 10) & 0x3FF;
	int idx = rdo >> 28;

	u32 pdo;
	u32 pdo_max;

	if (!idx || idx > pd_src_pdo_cnt) {
		TRACE("rdo = %x, Requested RDO is %d, RDO number is %d\n",
			rdo, (unsigned int)idx, (unsigned int)pd_src_pdo_cnt);
		return 0; /* Invalid index */
	}
	/*Update to pass TD.PD.SRC.E12 Reject Request*/
	pdo = change_bit_order(pd_src_pdo + ((idx - 1) * 4));
	pdo_max = (pdo & 0x3ff);
	TRACE("pdo_max = %x\n", pdo_max);


	/* check current ... */
	if (op_ma > pdo_max)/*Update to pass TD.PD.SRC.E12 Reject Request*/
		return 0; /* too much op current */
	if (max_ma > pdo_max)/*Update to pass TD.PD.SRC.E12 Reject Request*/
		return 0; /* too much max current */



	return 1;
}

/* Recieve Power Delivery Source Capability message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_source_caps_default_callback(void *para, u8 para_len)
{
	u8 *pdo = 0;
	u8 ret = 1;

	pdo = (u8 *) para;
	if (para_len % 4 != 0)
		return ret;
	if (build_rdo_from_source_caps(para_len / 4, (u8 *)para)) {
		ret = interface_send_request();
		TRACE("Snd RDO %x %x %x %x succ\n", pd_rdo[0], pd_rdo[1],
		      pd_rdo[2], pd_rdo[3]);
	}
	return ret;
}

/* Recieve Power Delivery Source Capability message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
   * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_sink_caps_default_callback(void *para, u8 para_len)
{
	u8 *pdo = 0;

	pdo = (u8 *) para;
	if (para_len % 4 != 0)
		return 0;
	if (para_len > VDO_SIZE)
		return 0;
	/*do what you want to do */
	return 1;
}

/* Recieve Power Delivery Source Capability message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_pwr_object_req_default_callback(void *para, u8 para_len)
{
	u8 *pdo = (u8 *) para;
	u8 ret = 1;
	u32 rdo = 0;

	if (para_len != 4)
		return ret;

	rdo = pdo[0] | (pdo[1] << 8) | (pdo[2] << 16) | (pdo[3] << 24);
	if (pd_check_requested_voltage(rdo))
		ret = send_accept();
	else
		ret = interface_send_reject();

	return ret;
}

/* Recieve accept message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func
  *  void *para : should be null
  *   para_len : 0
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_accept_default_callback(void *para, u8 para_len)
{

	return 1;
}

/* Recieve reject message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func
  *  void *para : should be null
  *   para_len : 0
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_reject_default_callback(void *para, u8 para_len)
{

	return 1;
}

/* Recieve reject message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func
  *  void *para : should be null
  *   para_len : 0
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_goto_min_default_callback(void *para, u8 para_len)
{

	return 1;
}

void pd_vbus_control_default_func(bool on)
{
	TRACE("=====vbus control %d\n", (int)on);

#ifdef SUP_VBUS_CTL
	anx7625_vbus_control(on);
#endif

}

void pd_vconn_control_default_func(bool on)
{
	/* to enable or disable VConn */

}

void pd_cc_status_default_func(u8 cc_status)
{
	/* cc status */
	TRACE("cc status %x\n", cc_status);

}

void pd_drole_change_default_func(bool on)
{
	/* data role changed */

}

void dp_hpd_change_default_func(bool on)
{
	/* hpd changed */
	TRACE("dp_hpd_change_default_func: %d\n", (unsigned int)on);

	if (on == 0) {
		TRACE(" HPD low\n");
		anx7625_stop_dp_work();
	} else {

		TRACE(" HPD high\n");
		anx7625_start_dp();

	}
}

/* Recieve comand response message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  *  void *para :
  *   para_len :
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_cmd_rsp_default_callback(void *para, u8 para_len)
{
	u8 pd_cmd, pd_response;

	pd_cmd =  *(u8 *)para;
	pd_response = *((u8 *)para + 1);

	switch (pd_cmd) {
	case TYPE_PWR_OBJ_REQ:

		if (pd_response == CMD_SUCCESS)
			TRACE("pd_cmd RDO request result is successful\n");
		else if (pd_response == CMD_REJECT)
			TRACE("pd_cmd RDO reques result is rejected\n");
		else if (pd_response == CMD_BUSY)
			TRACE("pd_cmd RDO reques result is busy\n");
		else if (pd_response == CMD_FAIL)
			TRACE("pd_cmd RDO reques result is fail\n");
		else
			TRACE("pd_cmd RDO reques result is unknown\n");
		break;
	case TYPE_VCONN_SWAP_REQ:

		if (pd_response == CMD_SUCCESS)
			TRACE("pd_cmd VCONN Swap result is successful\n");
		else if (pd_response == CMD_REJECT)
			TRACE("pd_cmd VCONN Swap result is rejected\n");
		else if (pd_response == CMD_BUSY)
			TRACE("pd_cmd VCONN Swap result is busy\n");
		else if (pd_response == CMD_FAIL)
			TRACE("pd_cmd VCONN Swap result is fail\n");
		else
			TRACE("pd_cmd VCONN Swap result is unknown\n");
		break;
	case TYPE_DSWAP_REQ:

		if (pd_response == CMD_SUCCESS)
			TRACE("pd_cmd DRSwap result is successful\n");
		else if (pd_response == CMD_REJECT)
			TRACE("pd_cmd DRSwap result is rejected\n");
		else if (pd_response == CMD_BUSY)
			TRACE("pd_cmd DRSwap result is busy\n");
		else if (pd_response == CMD_FAIL)
			TRACE("pd_cmd DRSwap result is fail\n");
		else
			TRACE("pd_cmd DRSwap result is unknown\n");
		break;
	case TYPE_PSWAP_REQ:

		if (pd_response == CMD_SUCCESS)
			TRACE("pd_cmd PRSwap result is successful\n");
		else if (pd_response == CMD_REJECT)
			TRACE("pd_cmd PRSwap result is rejected\n");
		else if (pd_response == CMD_BUSY)
			TRACE("pd_cmd PRSwap result is busy\n");
		else if (pd_response == CMD_FAIL)
			TRACE("pd_cmd PRSwap result is fail\n");
		else
			TRACE("pd_cmd PRSwap result is unknown\n");
		break;
	default:
		break;
	}

	return CMD_SUCCESS;
}

u8 recv_pd_hard_rst_default_callback(void *para, u8 para_len)
{
	TRACE("recv pd hard reset\n");


	return CMD_SUCCESS;
}

/* Recieve Data Role Swap message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through init_pd_msg_callback, it pd_callback is not 0, using the default
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_dswap_default_callback(void *para, u8 para_len)
{
	/* dswap just notice AP, do nothing */
	return 1;
}

/* Recieve Power Role Swap message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through init_pd_msg_callback, it pd_callback is not 0, using the default
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
 * @return:  0: success, 1: reject, 2: fail, 3: busy
  */
u8 recv_pd_pswap_default_callback(void *para, u8 para_len)
{
	/* pswap just notice AP, do nothing */
	return 1;
}
static pd_callback_t pd_callback_array[256] = { 0 };

pd_callback_t get_pd_callback_fnc(enum PD_MSG_TYPE type)
{
	pd_callback_t fnc = 0;

	if (type < 256)
		fnc = pd_callback_array[type];
	return fnc;
}

void set_pd_callback_fnc(enum PD_MSG_TYPE type, pd_callback_t fnc)
{
	pd_callback_array[type] = fnc;
}

void init_pd_msg_callback(void)
{
	u8 i = 0;

	for (i = 0; i < 256; i++)
		pd_callback_array[i] = 0x0;
}


#ifdef USE_PD30

char ConvertInterfaceTypeToPd3Type(unsigned char type,
	unsigned char *ext, unsigned char *pd3_type)
{
	switch (type) {
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
	default:
		/* unknown type*/
		return -EPERM;
	}
	return 0;
}

unsigned char recv_ext_msg_callback(unsigned char type,
	unsigned char *para, unsigned char para_len)
{
	unsigned char ret = 0;
	unsigned char ext;

	if (ConvertInterfaceTypeToPd3Type(type, &ext, &type) < 0)
		return -ESRCH;

	if (ext) {
		switch (type) {
		case PD_EXT_SOURCE_CAP:
			TRACE(" PD_EXT_SOURCE_CAP\n");
			recv_ext_message(PD_EXT_SOURCE_CAP,
				para + 1, para_len - 1);
			break;
		case PD_EXT_STATUS:
			TRACE(" PD_EXT_STATUS\n");
			recv_ext_message(PD_EXT_STATUS,
				para + 1, para_len - 1);
			break;
		case PD_EXT_GET_BATTERY_CAP:
			TRACE(" PD_EXT_GET_BATTERY_CAP\n");
			recv_ext_message(PD_EXT_GET_BATTERY_CAP,
				para + 1, para_len - 1);
			break;
		case PD_EXT_GET_BATTERY_STATUS:
			TRACE(" PD_EXT_GET_BATTERY_STATUS\n");
			recv_ext_message(PD_EXT_GET_BATTERY_STATUS,
				para + 1, para_len - 1);
			break;
		case PD_EXT_BATTERY_CAP:
			TRACE(" PD_EXT_BATTERY_CAP\n");
			recv_ext_message(PD_EXT_BATTERY_CAP,
				para + 1, para_len - 1);
			break;
		case PD_EXT_GET_MANUFACTURER_INFO:
			TRACE(" PD_EXT_GET_MANUFACTURER_INFO\n");
			recv_ext_message(PD_EXT_GET_MANUFACTURER_INFO,
				para + 1, para_len - 1);
			break;
		case PD_EXT_MANUFACTURER_INFO:
			TRACE(" PD_EXT_MANUFACTURER_INFO\n");
			recv_ext_message(PD_EXT_MANUFACTURER_INFO,
				para + 1, para_len - 1);
			break;
		case PD_EXT_SECURITY_REQUEST:
			TRACE(" PD_EXT_SECURITY_REQUEST\n");
			/* ToDo: Implement USBTypeCAuthentication 1.0 spec.*/
			send_unext_message(PD_CTRL_NOT_SUPPORTED, 0);
			break;
		case PD_EXT_SECURITY_RESPONSE:
			TRACE(" PD_EXT_SECURITY_RESPONSE\n");
			/* ToDo: Implement USBTypeCAuthentication 1.0 spec.*/
			break;
		case PD_EXT_FW_UPDATE_REQUEST:
			TRACE(" PD_EXT_FW_UPDATE_REQUEST\n");
#if USE_PDFU
			ret = RecvBlock(PD_EXT_FW_UPDATE_REQUEST,
				para + 1, para_len - 1);
			if (ret == RECV_BLOCK_IDLE) {
				recv_pdfu_request(para + 1 + 2,
					para_len - 1 - 2);
			} else if (ret == RECV_BLOCK_FINISH) {
				recv_pdfu_request(pd_block_recv_buf,
					pd_block_recv_len);
				pd_block_recv_len = 0;
			}
			ret = 0;
#endif
			break;
		case PD_EXT_FW_UPDATE_RESPONSE:
			TRACE(" PD_EXT_FW_UPDATE_RESPONSE\n");
#if USE_PDFU
			recv_pdfu_response(para + 1, para_len - 1);
#endif
			break;
		default:
			TRACE(" Unknown ext pd msg\n");
			ret = 1;
			break;

		}
	} else {
		switch (type) {
		case PD_CTRL_NOT_SUPPORTED:
			TRACE(" PD_CTRL_NOT_SUPPORTED\n");
			break;
		case PD_CTRL_GET_SOURCE_CAP_EXTENDED:
			TRACE(" PD_CTRL_GET_SOURCE_CAP_EXTENDED\n");
			recv_unext_message(PD_CTRL_GET_SOURCE_CAP_EXTENDED,
				para + 1, para_len - 1);
			break;
		case PD_CTRL_GET_STATUS:
			TRACE(" PD_CTRL_GET_STATUS\n");
			recv_unext_message(PD_CTRL_GET_STATUS,
				para + 1, para_len - 1);
			break;
		case PD_CTRL_FR_SWAP:
			TRACE(" PD_CTRL_FR_SWAP\n");
			recv_unext_message(PD_CTRL_FR_SWAP,
				para + 1, para_len - 1);
			break;
		case PD_DATA_BATTERY_STATUS:
			TRACE(" PD_DATA_BATTERY_STATUS\n");
			recv_unext_message(PD_DATA_BATTERY_STATUS,
				para + 1, para_len - 1);
			break;
		case PD_DATA_ALERT:
			TRACE(" PD_DATA_ALERT\n");
			recv_unext_message(PD_DATA_ALERT,
				para + 1, para_len - 1);
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


#ifdef USE_PD30

char ConvertPd3TypeToInterfaceType(
	unsigned char ext, unsigned char pd3_type, unsigned char *type)
{
	if (ext) {
		switch (pd3_type) {
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
			/* not support*/
			pd3_type = 0;
			break;
		case PD_EXT_FW_UPDATE_REQUEST:
			*type = TYPE_EXT_PDFU_REQUEST;
			break;
		case PD_EXT_FW_UPDATE_RESPONSE:
			*type = TYPE_EXT_PDFU_RESPONSE;
			break;
		default:
			pd3_type = 0;
			break;
		}
	} else {
		switch (pd3_type) {
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

		default:
			pd3_type = 0;
			break;
		}
	}
	if (pd3_type)
		return 0;
	else
		return -EPERM;
}

unsigned char send_ext_msg(unsigned char is_ext, unsigned char type,
	unsigned char *pbuf, unsigned char buf_len, unsigned char type_sop)
{

	u8 temp_buf[40];

	if (ConvertPd3TypeToInterfaceType(is_ext, type, &type) < 0)
		return CMD_FAIL;

	temp_buf[0] = type_sop;
	memcpy(&temp_buf[1], pbuf, buf_len);

	return interface_send_msg_timeout(type, temp_buf, buf_len + 1,
			INTERFACE_TIMEOUT);

}
#endif


