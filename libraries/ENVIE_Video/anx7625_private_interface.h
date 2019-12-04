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


#ifndef anx7625_PRIVATE_INTERFACE_H
#define anx7625_PRIVATE_INTERFACE_H
#include "anx7625_public_interface.h"


#define InterfaceSendBuf_Addr 0xc0
#define InterfaceRecvBuf_Addr 0xe0



#define YES     1
#define NO      0
#define ERR_CABLE_UNPLUG -1
#define PD_ONE_DATA_OBJECT_SIZE  4
#define PD_MAX_DATA_OBJECT_NUM  7
#define VDO_SIZE (PD_ONE_DATA_OBJECT_SIZE * PD_MAX_DATA_OBJECT_NUM)

#define PDO_FIXED_FLAGS (PDO_FIXED_DUAL_ROLE | PDO_FIXED_DATA_SWAP)

#define MV 1
#define MA 1
#define MW 1

/*5000mv voltage*/
#define PD_VOLTAGE_5V 5000

#define PD_MAX_VOLTAGE_20V 20000
#define PD_MAX_VOLTAGE_21V 21000

/*0.9A current */
#define PD_CURRENT_900MA   900
#define PD_CURRENT_1500MA 1500

#define PD_CURRENT_3A   3000

#define PD_POWER_15W  15000

#define PD_POWER_60W  60000

/* RDO : Request Data Object */
#define RDO_OBJ_POS(n)             (((u32)(n) & 0x7) << 28)
#define RDO_POS(rdo)               ((((32)rdo) >> 28) & 0x7)
#define RDO_GIVE_BACK              ((u32)1 << 27)
#define RDO_CAP_MISMATCH           ((u32)1 << 26)
#define RDO_COMM_CAP               ((u32)1 << 25)
#define RDO_NO_SUSPEND             ((u32)1 << 24)
#define RDO_FIXED_VAR_OP_CURR(ma)  (((((u32)ma) / 10) & 0x3FF) << 10)
#define RDO_FIXED_VAR_MAX_CURR(ma) (((((u32)ma) / 10) & 0x3FF) << 0)

#define RDO_BATT_OP_POWER(mw)      (((((u32)mw) / 250) & 0x3FF) << 10)
#define RDO_BATT_MAX_POWER(mw)     (((((u32)mw) / 250) & 0x3FF) << 10)

#define RDO_FIXED(n, op_ma, max_ma, flags)	\
	(RDO_OBJ_POS(n) | (flags) |		\
	RDO_FIXED_VAR_OP_CURR(op_ma) |		\
	RDO_FIXED_VAR_MAX_CURR(max_ma))

#define EXTERNALLY_POWERED  YES
/* Source Capabilities */
/* 1 to 5 */
#define SOURCE_PROFILE_NUMBER   1
/* 0 = Fixed, 1 = Battery, 2 = Variable */
#define SRC_PDO_SUPPLY_TYPE1    0
/* 0 to 3 */
#define SRC_PDO_PEAK_CURRENT1   0
/* 5000mV (5V) */
#define SRC_PDO_VOLTAGE1        5000
/* 500mA (0.5A) */
#define SRC_PDO_MAX_CURRENT1    500


#define IRQ_STATUS 0xcc
#define IRQ_EXT_MASK_2 0x3d
#define IRQ_EXT_SOFT_RESET_BIT 0x04
#define IRQ_EXT_SOURCE_2 0x4F


#define INTERACE_TIMEOUT_MS 26

#define OCM_SLAVE_I2C_ADDR 0x7e

#define OCM_SLAVE_I2C_ADDR1 0x7e
#define OCM_SLAVE_I2C_ADDR2 0x7e

#define OCM_FW_VERSION 0x31
#define OCM_FW_REVERSION 0x32


#define INTERFACE_INTR_MASK 0x43
#define RECEIVED_MSG_MASK 1
#define RECEIVED_ACK_MASK 2
#define VCONN_CHANGE_MASK 4
#define VBUS_CHANGE_MASK 8
#define CC_STATUS_CHANGE_MASK 16
#define DATA_ROLE_CHANGE_MASK 32

#define INTERFACE_CHANGE_INT 0x44
#define RECEIVED_MSG 0x01
#define RECEIVED_ACK 0x02
#define VCONN_CHANGE 0x04
#define VBUS_CHANGE 0x08
#define CC_STATUS_CHANGE 0x10
#define DATA_ROLE_CHANGE 0x20
#define PR_CONSUMER_GOT_POWER 0x40
#define HPD_STATUS_CHANGE 0x80


#define SYSTEM_STSTUS 0x45
/*0: VCONN off; 1: VCONN on*/
#define VCONN_STATUS 0x04
/*0: vbus off; 1: vbus on*/
#define VBUS_STATUS 0x08
/*0: host; 1:device*/
#define DATA_ROLE 0x20

#define HPD_STATUS 0x80


#define NEW_CC_STATUS 0x46

/*
* Delay time for OCM
* bit4-bit7 Reserved
* bit0-bit3 Aelay time ocm disable vbus after receiving hardreset
*           According USB PD spec, this time shall be 25ms~35ms,
*           Real value: 30ms - (T_TIME_1 & 0x0F)
*/
#define T_TIME_1 0x6C
#define T_HARDREST_VBUS_OFF_MASK 0x0F

#define VBUS_DELAY_TIME 0x06/*0x69*/
#define TRY_UFP_TIMER 0x23/*0x6A*/

#define AUTO_PD_MODE 0x2f/*0x6e*/
#define no_skip_check_vbus 0x01
#define AUTO_PD_ENABLE 0x02
#define trysrc_en 0x04
#define trysnk_en 0x08
#define support_goto_min_power 0x10
#define slimport_mode_mode 0x20


#define MAX_VOLTAGE_SETTING 0x29/*0xd0*/
#define MAX_POWER_SETTING 0x2A/*0xd1*/
#define MIN_POWER_SETTING 0x2B/*0xd2*/

/*Show Maximum voltage of RDO*/
#define RDO_MAX_VOLTAGE 0x2C /* 0x7E:0x2C // 0xD3*/
/*Show Maximum Powe of RDO*/
#define RDO_MAX_POWER 0x2D /* 0x7E:0x2D // 0xD4*/
/*Show Maximum current of RDO*/
#define RDO_MAX_CURRENT 0x2E /* 0x7E:0x2E // 0xD5*/

#define FW_STATE_MACHINE 0x30 /* 0x7E:0x30*/


unsigned char ReadReg(unsigned char Devaddr, unsigned char RegAddr);
void WriteReg(unsigned char Devaddr,
	unsigned char RegAddr, unsigned char RegVal);

struct tagInterfaceHeader {
	unsigned char Indicator:1;	/* indicator */
	unsigned char Type:3;		/* data type */
	unsigned char Length:4;		/* data length */
};

struct tagInterfaceData {
	unsigned long SrcPDO[7];
	unsigned long SnkPDO[7];
	unsigned long RDO;
	unsigned long VDMHeader;
	unsigned long IDHeader;
	unsigned long CertStatVDO;
	unsigned long ProductVDO;
	unsigned long CableVDO;
	unsigned long AMM_VDO;
};

/*Comands status*/
enum interface_status {
	CMD_SUCCESS,
	CMD_REJECT,
	CMD_FAIL,
	CMD_BUSY,
	CMD_STATUS
};

#define MAX_INTERFACE_COUNT 32
#define MAX_INTERFACE_MSG_LEN  32

#define INTERFACE_TIMEOUT 30
/*extern u8 pd_src_pdo_cnt;*/
/*extern u8 pd_src_pdo[];*/
/*extern u8 pd_snk_pdo_cnt;*/
/*extern u8 pd_snk_pdo[];*/
extern u8 pd_rdo[];
extern u8 DP_caps[];
extern u8 configure_DP_caps[];
extern u8 src_dp_caps[];
extern atomic_t anx7625_power_status;

/* check soft interrupt happens or not */
#define is_soft_reset_intr() \
(ReadReg(OCM_SLAVE_I2C_ADDR1, IRQ_EXT_SOURCE_2) & IRQ_EXT_SOFT_RESET_BIT)

/* clear the anx7625's soft  interrupt bit */
#define clear_soft_interrupt()	\
	WriteReg(OCM_SLAVE_I2C_ADDR1, IRQ_EXT_SOURCE_2, IRQ_EXT_SOFT_RESET_BIT)
/*control cmd*/
#define interface_pr_swap() \
	interface_send_msg_timeout(TYPE_PSWAP_REQ, 0, 0, INTERFACE_TIMEOUT)
#define interface_dr_swap() \
	interface_send_msg_timeout(TYPE_DSWAP_REQ, 0, 0, INTERFACE_TIMEOUT)
#define interface_vconn_swap() \
	interface_send_msg_timeout(TYPE_VCONN_SWAP_REQ, 0, 0, INTERFACE_TIMEOUT)
#define interface_get_dp_caps() \
	interface_send_msg_timeout(TYPE_GET_DP_SNK_CAP, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_gotomin() \
	interface_send_msg_timeout(TYPE_GOTO_MIN_REQ, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_soft_rst() \
	interface_send_msg_timeout(TYPE_SOFT_RST, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_hard_rst() \
	interface_send_msg_timeout(TYPE_HARD_RST, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_restart() \
	interface_send_msg_timeout(TYPE_RESTART, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_accept() \
	interface_send_msg_timeout(TYPE_ACCEPT, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_reject() \
	interface_send_msg_timeout(TYPE_REJECT, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_dp_enter() \
	interface_send_msg_timeout(TYPE_DP_ALT_ENTER, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_dp_exit() \
	interface_send_msg_timeout(TYPE_DP_ALT_EXIT, 0, 0, INTERFACE_TIMEOUT)
#define interface_send_src_cap() \
	interface_send_msg_timeout(TYPE_PWR_SRC_CAP, pd_src_pdo,\
	pd_src_pdo_cnt * 4, INTERFACE_TIMEOUT)
#define interface_send_snk_cap() \
	interface_send_msg_timeout(TYPE_PWR_SNK_CAP, pd_snk_pdo,\
	pd_snk_pdo_cnt * 4, INTERFACE_TIMEOUT)
#define interface_send_src_dp_cap() \
	interface_send_msg_timeout(TYPE_DP_SNK_IDENTITY, src_dp_caps,\
	4, INTERFACE_TIMEOUT)
#define interface_config_dp_caps() \
	interface_send_msg_timeout(TYPE_DP_SNK_CFG, configure_DP_caps,\
	4, INTERFACE_TIMEOUT)
#define interface_send_request() \
	interface_send_msg_timeout(TYPE_PWR_OBJ_REQ, pd_rdo,\
	4, INTERFACE_TIMEOUT)
#define interface_send_vdm_data(buf, len)	\
	interface_send_msg_timeout(TYPE_VDM, buf, len, INTERFACE_TIMEOUT)

void send_initialized_setting(void);
void chip_register_init(void);
u8 polling_interface_msg(int timeout_ms);
u8 send_dp_snk_cfg(const u8 *dp_snk_caps, u8 dp_snk_caps_size);
u8 send_dp_snk_identity(const u8 *, u8);
u8 send_vdm(const u8 *vdm, u8 size);
u8 send_svid(const u8 *svid, u8 size);
u8 recv_pd_pwr_object_req_default_callback(void *para, u8 para_len);
u8 recv_pd_dswap_default_callback(void *para, u8 para_len);
u8 recv_pd_pswap_default_callback(void *para, u8 para_len);
u8 recv_pd_sink_caps_default_callback(void *para, u8 para_len);
u8 recv_pd_source_caps_default_callback(void *para, u8 para_len);
u8 recv_pd_cmd_rsp_default_callback(void *para, u8 para_len);
u8 recv_pd_goto_min_default_callback(void *para, u8 para_len);
u8 recv_pd_accept_default_callback(void *para, u8 para_len);
u8 recv_pd_reject_default_callback(void *para, u8 para_len);
u8 recv_pd_hard_rst_default_callback(void *para, u8 para_len);
const char *interface_to_str(unsigned char header_type);

int ReadBlockReg(u8 DevAddr, u8 RegAddr, u8 len, u8 *dat);
int WriteBlockReg(u8 DevAddr, u8 RegAddr, u8 len, const u8 *dat);
void anx7625_vbus_control(bool value);

void pd_vbus_control_default_func(bool on);
void pd_vconn_control_default_func(bool on);
void pd_cc_status_default_func(u8 cc_status);
void pd_drole_change_default_func(bool on);
void dp_hpd_change_default_func(bool on);

void handle_intr_vector(void);
void handle_msg_rcv_intr(void);

/* 0, send interface msg timeout
  * 1 successful
  */
u8 interface_send_msg_timeout(u8 type, u8 *pbuf, u8 len, int timeout_ms);
pd_callback_t get_pd_callback_fnc(enum PD_MSG_TYPE type);
void set_pd_callback_fnc(enum PD_MSG_TYPE type, pd_callback_t fnc);

unsigned char send_get_var(unsigned char type, unsigned int addr);
unsigned char send_set_var(unsigned char type,
	unsigned int addr, unsigned char val);

#ifdef USE_PD30
unsigned char send_ext_msg(unsigned char is_ext, unsigned char type,
	unsigned char *pbuf, unsigned char buf_len, unsigned char type_sop);
#endif

#ifdef USE_PD30
unsigned char recv_ext_msg_callback(unsigned char type,
	unsigned char *para, unsigned char para_len);
#endif

unsigned char recv_debug_callback(unsigned char type,
	unsigned char *para, unsigned char para_len);


/**
 * @desc:   The interface AP will set(fill) the source capability to anx7625
 *
 * @param:
 *	src_caps: PDO buffer pointer of source capability
 *		whose bit formats follow the rules:
 *		it's little-endian, defined in USB PD spec 5.5
 *		Transmitted Bit Ordering
 *		source capability's specific format defined in
 *		USB PD spec 6.4.1 Capabilities Message
 *              PDO refer to Table 6-4 Power Data Object
 *              Variable PDO : Table 6-8 Variable Supply (non-battery)
 *              Battery PDO : Table 6-9 Battery Supply PDO --source
 *              Fixed PDO refer to  Table 6-6 Fixed Supply PDO --source
 *	eg: default5Vsafe src_cap(5V, 0.9A fixed)
 *		PDO_FIXED(5000,900, PDO_FIXED_FLAGS)
 *
 *	src_caps_size: source capability's size
 *		if the source capability obtains one PDO object
 *		src_caps_size is 4, two PDO objects, src_caps_size is 8
 *
 * @return:  0: success.  1: fail
 *
 */
u8 send_src_cap(const u8 *src_caps, u8 src_caps_size);

/**
 * @desc:   The interface AP will set(fill) the sink capability to anx7625
 *
 * @param:
 *	snk_caps: PDO buffer pointer of sink capability
 *		whose bit formats follow the rules:
 *		it's little-endian, defined in USB PD spec 5.5
 *		Transmitted Bit Ordering
 *		source capability's specific format defined in
 *		USB PD spec 6.4.1 Capabilities Message
 *		PDO refer to Table 6-4 Power Data Object
 *		Variable PDO : Table 6-8 Variable Suppl
 *              Battery PDO : Table 6-9 Battery Supply PDO
 *		Fixed PDO refer to  Table 6-6 Fixed Supply PDO
 *	eg: default5Vsafe snk_cap(5V, 0.9A fixed) -->
 *		PDO_FIXED(5000,900, PDO_FIXED_FLAGS)
 *
 *	snk_caps_size: sink capability's size
 *		if the sink capability obtains
 *		one PDO object, snk_caps_size is 4
 *		two PDO objects, snk_caps_size is 8
 *
 * @return:  0: success.  1: fail
 *
 */
u8 send_snk_cap(const u8 *snk_caps, u8 snk_caps_size);
u8 send_rdo(const u8 *rdo, u8 size);
u8 send_data_swap(void);
u8 send_power_swap(void);
u8 send_accept(void);
void clear_sys_sta_bak(void);



/**
 * @desc:   The interface AP will get the anx7625's data role
 *
 * @param:  none
 *
 * @return:  data role , dfp 1 , ufp 0, other error: -1, not ready
 *
 */
s8 get_data_role(void);

/**
 * @desc:   The interface AP will get the anx7625's power role
 *
 * @param:  none
 *
 * @return:  data role , source 1 , sink 0, other error, -1, not ready
 *
 */
s8 get_power_role(void);


#endif
