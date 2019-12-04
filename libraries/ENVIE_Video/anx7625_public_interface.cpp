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

#include "anx7625_private_interface.h"
#include "anx7625_public_interface.h"

#ifdef USE_PD30
#include "anx7625_pd30_message.h"
#endif

/**
 * @desc: The Interface that AP sends the specific USB PD command to anx7625
 *
 * @param:
 *	type: PD message type, define enum PD_MSG_TYPE.
 *	buf: the sepecific paramter pointer according to the message type
 *		eg: when AP update its source capability type=TYPE_PWR_SRC_CAP
 *		"buf" contains the content of PDO object,its format USB PD spec
 *		customer can easily packeted it through PDO_FIXED_XXX macro:
 *		default5Vsafe 5V,0.9A -> PDO_FIXED(5000,900, PDO_FIXED_FLAGS)
 *	size: the paramter content length. if buf is null, it should be 0
 *
 * @return:  0: success, 1: reject, 2: fail, 3: busy
 */
u8 send_pd_msg(enum PD_MSG_TYPE type, const uint8_t *buf, u8 size)
{
	u8 rst = 0;

	switch (type) {
	case TYPE_PWR_SRC_CAP:
		rst = send_src_cap(buf, size);
		break;
	case TYPE_PWR_SNK_CAP:
		rst = send_snk_cap(buf, size);
		break;
	case TYPE_DP_SNK_IDENTITY:
		rst = interface_send_msg_timeout(TYPE_DP_SNK_IDENTITY,
				(u8 *)buf, size, INTERFACE_TIMEOUT);
		break;
	case TYPE_SVID:
		rst = send_svid(buf, size);
		break;
	case TYPE_GET_DP_SNK_CAP:
		rst = interface_send_msg_timeout(TYPE_GET_DP_SNK_CAP, NULL, 0,
				INTERFACE_TIMEOUT);
		break;
	case TYPE_PSWAP_REQ:
		rst = send_power_swap();
		break;
	case TYPE_DSWAP_REQ:
		rst = send_data_swap();
		break;
	case TYPE_GOTO_MIN_REQ:
		rst = interface_send_gotomin();
		break;
	case TYPE_VDM:
		rst = send_vdm(buf, size);
		break;
	case TYPE_DP_SNK_CFG:
		rst = send_dp_snk_cfg(buf, size);
		break;
	case TYPE_PWR_OBJ_REQ:
		rst = send_rdo(buf, size);
		break;
	case TYPE_ACCEPT:
		rst = interface_send_accept();
		break;
	case TYPE_REJECT:
		rst = interface_send_reject();
		break;
	case TYPE_SOFT_RST:
		rst = interface_send_soft_rst();
		break;
	case TYPE_HARD_RST:
		rst = interface_send_hard_rst();
		break;
	default:
		pr_info("unknown type %x\n", type);
		rst = 0;
		break;
	}
	if (rst == CMD_FAIL) {
		pr_err("Cmd %x Fail.\n", type);
		return CMD_FAIL;
	}

	return rst;
}

/**
 * @desc:The Interface that AP handle the specific USB PD command from anx7625
 *
 * @param:
 *	type: PD message type, define enum PD_MSG_TYPE.
 *	para: the sepecific paramter pointer
 *	para_len: the paramter ponter's content length
 *		if buf is null, it should be 0
 *
 * @return:  0: success 1: fail
 *
 */
u8 dispatch_rcvd_pd_msg(enum PD_MSG_TYPE type, void *para, u8 para_len)
{
	u8 rst = 0;
	pd_callback_t fnc = get_pd_callback_fnc(type);

	if (fnc != 0) {
		rst = (*fnc) (para, para_len);
		return rst;
	}

	switch (type) {
	case TYPE_PWR_SRC_CAP:
		/* execute the receved source capability's  handle function */
#ifndef AUTO_RDO_ENABLE
		rst = recv_pd_source_caps_default_callback(para, para_len);
#endif

		break;
	case TYPE_PWR_SNK_CAP:
		/* received peer's sink caps */
		rst = recv_pd_sink_caps_default_callback(para, para_len);
		break;
	case TYPE_PWR_OBJ_REQ:
		/* evaluate RDO and give accpet or reject */
#ifndef AUTO_RDO_ENABLE
		rst = recv_pd_pwr_object_req_default_callback(para, para_len);
#endif

		break;
	case TYPE_DSWAP_REQ:
		/* execute the receved handle function */
		rst = recv_pd_dswap_default_callback(para, para_len);
		break;
	case TYPE_PSWAP_REQ:
		/* execute the receved handle function */
		rst = recv_pd_pswap_default_callback(para, para_len);
		break;
	case TYPE_VDM:
		break;
	case TYPE_ACCEPT:
		rst = recv_pd_accept_default_callback(para, para_len);
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
		/*TRACE(" Recv: PD3_MSG\n");*/
		return recv_ext_msg_callback(type, para, para_len);

#endif


	case TYPE_RESPONSE_TO_REQ:
		/* execute the receved handle function */
		rst = recv_pd_cmd_rsp_default_callback(para, para_len);
		break;

	case TYPE_DP_ALT_ENTER:
		break;
	case TYPE_DP_ALT_EXIT:
		break;
	case TYPE_HARD_RST:
		rst = recv_pd_hard_rst_default_callback(para, para_len);
		break;
	default:
		rst = 0;
		break;
	}
	return rst;
}
/**
 * @desc:  The Interface helps customer to register one's
 *	interesting callback function of the specific
 *	USB PD message type, when the REGISTERED message
 *	arrive, the customer's callback function will be executed.
 * !!!! Becarefully:
 *  Because the USB PD TIMING limatation, the callback function
 *  should be designed to follow USB PD timing requiment.
 *
 * @param:
 *	type: PD message type, define enum PD_MSG_TYPE.
 *	func: callback function pointer
 *		it's sepecific definaction is:u8 (*)(void *, u8)
 *
 * @return:  0: success.  1: fail
 *
 */
u8 register_pd_msg_callback_func(enum PD_MSG_TYPE type, pd_callback_t fnc)
{
	if (type > 256)
		return 1;
	set_pd_callback_fnc(type, fnc);

	return 0;
}

