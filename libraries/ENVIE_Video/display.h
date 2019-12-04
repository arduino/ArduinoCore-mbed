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


#ifndef _SP_H
#define _SP_H

#include "anx7625_display.h"

#ifdef _SP_C_
#define _SP_TX_DRV_EX_C_
#else
#define _SP_TX_DRV_EX_C_ \
extern
#endif

#define BYTE unsigned char
#define unchar unsigned char
#define uint unsigned int
#define ulong unsigned long

/*
*typedef unsigned char BYTE;
typedef unsigned char unchar;
typedef unsigned int uint;
typedef unsigned long ulong;
*/

#define MAX_BUF_CNT 6





enum Def_Resolution {
	RESOLUTION_480P = 0,
	RESOLUTION_720P = 1,
	RESOLUTION_1080P30 = 2,
	RESOLUTION_1080P60 = 3,
	RESOLUTION_DPI_4K24 = 4,
	RESOLUTION_DPI_4K30 = 5,
	RESOLUTION_480P_DSI = 6,
	RESOLUTION_720P_DSI = 7,
	RESOLUTION_1080P30_DSI = 8,
	RESOLUTION_1080P60_DSI = 9,
	RESOLUTION_DSI_4K24 = 10,
	RESOLUTION_DSI_4K30 = 11,
	RESOLUTION_DPI_DSC_2K70 = 12,
	RESOLUTION_QCOM820_1080P60_DSI = 13,
	RESOLUTION_QCOM820_720P60_DSI = 14,
	RESOLUTION_CUSTOM = 15,
};

enum Def_Audio {
	AUDIO_I2S_2CH_48K = 0,
	AUDIO_I2S_2CH_441K = 1,
	AUDIO_I2S_2CH_32K = 2,
	AUDIO_I2S_2CH_882K = 3,
	AUDIO_I2S_2CH_96K = 4,
	AUDIO_I2S_2CH_1764K = 5,
	AUDIO_I2S_2CH_192K = 6,
	AUDIO_TDM_8CH_48K = 7,
	AUDIO_TDM_8CH_441K = 8,
	AUDIO_TDM_8CH_32K = 9,
	AUDIO_TDM_8CH_882K = 10,
	AUDIO_TDM_8CH_96K = 11,
	AUDIO_TDM_8CH_1764K = 12,
	AUDIO_TDM_8CH_192K = 13
};




#define SP_TX_PORT0_ADDR 0x70
#define SP_TX_PORT1_ADDR 0x7A
#define SP_TX_PORT2_ADDR 0x72
#define MIPI_RX_PORT1_ADDR 0x84



#define AUX_ERR  1
#define AUX_OK   0

void SP_CTRL_Dump_Reg(void);
int slimport_anx7625_init(void);

/*Should align to mhl_linux_tx.h*/
#define SLIMPORT_TX_EVENT_DISCONNECTION	0x01
#define SLIMPORT_TX_EVENT_CONNECTION		0x02
#define SLIMPORT_TX_EVENT_SMB_DATA		0x40
#define SLIMPORT_TX_EVENT_HPD_CLEAR		0x41
#define SLIMPORT_TX_EVENT_HPD_GOT		0x42
#define SLIIPORT_TX_EVENT_DEV_CAP_UPDATE 0x43
#define SLIMPORT_TX_EVENT_EDID_UPDATE	0x44
#define SLIMPORT_TX_EVENT_EDID_DONE		0x45
#define SLIMPORT_TX_EVENT_CALLBACK		0x46
#define SLIMPORT_TX_EVENT_DEV_CAP_UPDATE 0x47

/*
 * Below five GPIOs are example  for AP to control the Slimport chip anx7625.
 * Different AP needs to configure these control pins to GPIOs of AP.
 */

/*******************Slimport Control************************/
/*
*#define SP_TX_PWR_V10_CTRL              (104) AP IO Control - Power+V12
#define SP_TX_HW_RESET                      (84) AP IO Control - Reset
#define SLIMPORT_CABLE_DETECT         (103) AP IO Input - Cable detect
#define SP_TX_CHIP_PD_CTRL                (109) AP IO Control - CHIP_PW_HV

#define CONFIG_I2C_GPIO
#define SET_INT_MODE_EN
*/





#define delay_ms(time) usleep_range(time*1000, time*1100)


#ifndef LOG_TAG

#if defined CONFIG_SLIMPORT_ANX7808
#define LOG_TAG "[anx7808]"
#elif defined CONFIG_SLIMPORT_ANX7816
#define LOG_TAG "[anx7816]"
#elif defined CONFIG_SLIMPORT_anx7625
#define LOG_TAG "[anx7625]"
#else
#define LOG_TAG "[anxNULL]"
#endif

#endif


#ifdef CONFIG_SLIMPORT_DYNAMIC_HPD
void slimport_set_hdmi_hpd(int on);
#endif


_SP_TX_DRV_EX_C_ BYTE ByteBuf[MAX_BUF_CNT];



enum HDCP_CAP_TYPE {
	NO_HDCP_SUPPORT = 0x00,
	HDCP14_SUPPORT = 0x01,
	HDCP22_SUPPORT = 0x02,
	HDCP_ALL_SUPPORT = 0x03
};
_SP_TX_DRV_EX_C_ BYTE hdcp_cap;



_SP_TX_DRV_EX_C_ unsigned long pclk; /*input video pixel clock*/
_SP_TX_DRV_EX_C_ long int M_val, N_val;
_SP_TX_DRV_EX_C_ BYTE sp_tx_bw;  /*linktraining banwidth*/
_SP_TX_DRV_EX_C_ BYTE sp_tx_lane_count; /*link training lane count*/


_SP_TX_DRV_EX_C_ BYTE bMIPIFormatIndex;


#define ONE_BLOCK_SIZE      128
#define FOUR_BLOCK_SIZE      (128*4)

struct s_edid_data {
	uint8_t edid_block_num;
	uint8_t EDID_block_data[FOUR_BLOCK_SIZE];
};


_SP_TX_DRV_EX_C_ void *slimport_edid_p;

_SP_TX_DRV_EX_C_ BYTE mute_video_flag;


_SP_TX_DRV_EX_C_ BYTE slimport_log_on;

_SP_TX_DRV_EX_C_ void (*delay_video_cfg)(unsigned char table_id);
_SP_TX_DRV_EX_C_ BYTE delay_tab_id;

_SP_TX_DRV_EX_C_ int sp_read_reg(uint8_t slave_addr,
	uint8_t offset, uint8_t *buf);
_SP_TX_DRV_EX_C_ int sp_write_reg(uint8_t slave_addr,
	uint8_t offset, uint8_t value);
_SP_TX_DRV_EX_C_ unchar sp_get_rx_bw(void);
_SP_TX_DRV_EX_C_ unchar sp_get_lane_count(void);

_SP_TX_DRV_EX_C_ unchar sp_tx_edid_read(unchar *pedid_blocks_buf);
_SP_TX_DRV_EX_C_ unchar sp_tx_aux_dpcdwrite_bytes(unchar addrh,
	unchar addrm, unchar addrl, unchar cCount, unchar *pBuf);
_SP_TX_DRV_EX_C_ unchar sp_tx_aux_dpcdread_bytes(unchar addrh,
	unchar addrm, unchar addrl, unchar cCount, unchar *pBuf);

_SP_TX_DRV_EX_C_ void sp_tx_set_3d_format(u8 format3d);

_SP_TX_DRV_EX_C_ void DP_Process_Start(void);
_SP_TX_DRV_EX_C_ void DP_Process_Stop(void);
_SP_TX_DRV_EX_C_ void reconfig_current_pclk(unsigned char id, unsigned char direction, unsigned int delta_clk);





#endif
