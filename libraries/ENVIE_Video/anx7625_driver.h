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


#ifndef _anx7625_DRV_H
#define _anx7625_DRV_H

#include "arduino_hal.h"

#include "MI2_REG.h"

#ifndef LOG_TAG
#define LOG_TAG "anx7625"
#endif
#ifdef NEVER
#ifndef __func__
#define __func__  ""
#endif
#endif /* NEVER */


#define ANX7625_DRV_VERSION "1.3.00"

/*Dynamic config timing format by up-layer parsing edid data.*/
// #define DYNAMIC_CONFIG_MIPI

/*Disable Power Delivery Function.*/
#define DISABLE_PD


/*Loading OCM re-trying times*/
#define OCM_LOADING_TIME 10

/*S/W do deglitch on cable-detect pin.*/
// #define CABLE_DET_PIN_HAS_GLITCH

/*Debug log output enabled.*/
#define DEBUG_LOG_OUTPUT

/*Support Communicate Interrupt processing */
#define SUP_INT_VECTOR

/*Support VBUS Control by AP*/
// #define SUP_VBUS_CTL

/*Support Auto-PowerDelivery Communication*/
#define AUTO_RDO_ENABLE

/*Support Try-Sink in default*/
/*#define TRY_SRC_ENABLE*/
/*Support Try-Source in default*/
/*#define TRY_SNK_ENABLE*/

/*Auto Update OCM FW Enabled.*/
#define AUTO_UPDATE_OCM_FW 1 /* 0:disable, 1:enable*/

/*VBUS Control logic status.*/
#define ENABLE_VBUS_OUTPUT 0
#define DISABLE_VBUS_OUTPUT 1

/*Cable-Detect logic status*/
#define DONGLE_CABLE_INSERT  1


int Read_Reg(uint8_t slave_addr, uint8_t offset, uint8_t *buf);
unsigned char ReadReg(unsigned char DevAddr, unsigned char RegAddr);
unsigned char GetRegVal(unsigned char DevAddr, unsigned char RegAddr);

void WriteReg(unsigned char DevAddr, unsigned char RegAddr,
		unsigned char RegVal);
int ReadBlockReg(unsigned char DevAddr, u8 RegAddr,
		u8 len, u8 *dat);
int WriteBlockReg(unsigned char DevAddr, u8 RegAddr,
		u8 len, const u8 *dat);
unsigned char is_cable_detected(void);

irqreturn_t anx7625_cbl_det_isr(void *data);
irqreturn_t anx7625_intr_comm_isr(void *data);

extern atomic_t anx7625_power_status;
void anx7625_power_standby(void);
void anx7625_hardware_poweron(void);
void anx7625_vbus_control(bool value);
void anx7625_hardware_reset(int enable);
void MI2_power_on(void);

#define sp_write_reg_or(address, offset, mask) \
{ WriteReg(address, offset, ((unsigned char)GetRegVal(address, offset) \
	| (mask))); }
#define sp_write_reg_and(address, offset, mask) \
{ WriteReg(address, offset, ((unsigned char)GetRegVal(address, offset) \
	&(mask))); }

#define sp_write_reg_and_or(address, offset, and_mask, or_mask) \
{ WriteReg(address, offset, (((unsigned char)GetRegVal(address, offset)) \
	&and_mask) | (or_mask)); }
#define sp_write_reg_or_and(address, offset, or_mask, and_mask) \
{ WriteReg(address, offset, (((unsigned char)GetRegVal(address, offset)) \
	| or_mask) & (and_mask)); }


extern unsigned char cable_connected;
extern unsigned char hpd_status;

void anx7625_restart_work(int workqueu_timer);
void anx7625_start_dp(void);
void anx7625_stop_dp_work(void);
void anx7625_handle_intr_comm(void);

#ifdef DYNAMIC_CONFIG_MIPI
#include "../msm_dba/msm_dba_internal.h"
#include <video/msm_dba.h>
int anx7625_mipi_timing_setting(
	void *client, bool on, struct msm_dba_video_cfg *cfg,
	    u32 flags);
int anx7625_get_raw_edid(
	void *client, u32 size, char *buf, u32 flags);
void anx7625_notify_clients(struct msm_dba_device_info *dev,
	enum msm_dba_callback_event event);
int anx7625_audio_setting(void *client,
	struct msm_dba_audio_cfg *cfg, u32 flags);

#endif

#ifdef DEBUG_LOG_OUTPUT
#define TRACE pr_info
#define TRACE1 pr_info
#define TRACE2 pr_info
#define TRACE3 pr_info
#else
#define TRACE(fmt, arg...)
#define TRACE1(fmt, arg...)
#define TRACE2(fmt, arg...)
#define TRACE3(fmt, arg...)
#endif

#define UNUSED_VAR(x) ((x) = (x))

//#define USE_PD30

#endif
