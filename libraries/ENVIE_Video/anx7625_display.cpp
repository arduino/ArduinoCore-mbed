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


#include "arduino_hal.h"

#include "display.h"
#include "anx7625_driver.h"
#include "MI2_REG.h"


#include  "anx7625_display.h"

#define ReadReg Read_Reg
#define xdata



/*#define REV_AA*/
#define REV_AB
/* misc macros which are MIPI-DSI source (signal generator) dependent*/
#define  HS_Swing_300mV
/*#define  HS_Swing_400mV*/

#define XTAL_FRQ  27000000UL  /* MI-2 clock frequency in Hz: 27 MHz*/


#define mipi_pixel_frequency(id)   \
	mipi_video_timing_table[id].MIPI_pixel_frequency
#define mipi_lane_count(id)   \
	mipi_video_timing_table[id].MIPI_lane_count
#define mipi_m_value(id)   \
	mipi_video_timing_table[id].M
#define mipi_n_value(id)   \
	mipi_video_timing_table[id].N
#define mipi_post_divider(id)   \
	mipi_video_timing_table[id].post_divider
#define mipi_diff_ratio(id)   \
	mipi_video_timing_table[id].diff_ratio
#define mipi_compress_ratio(id)   \
	mipi_video_timing_table[id].compress_ratio

#define mipi_original_htotal(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_HTOTAL
#define mipi_original_hactive(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_HActive
#define mipi_original_vtotal(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_VTOTAL
#define mipi_original_vactive(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_VActive
#define mipi_original_hfp(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_H_Front_Porch
#define mipi_original_hsw(id)  \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_H_Sync_Width
#define mipi_original_hbp(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_H_Back_Porch
#define mipi_original_vfp(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_V_Front_Porch
#define mipi_original_vsw(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_V_Sync_Width
#define mipi_original_vbp(id)   \
	mipi_video_timing_table[id].MIPI_inputl[0].MIPI_V_Back_Porch

#define mipi_decompressed_htotal(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_HTOTAL
#define mipi_decompressed_hactive(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_HActive
#define mipi_decompressed_vtotal(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_VTOTAL
#define mipi_decompressed_vactive(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_VActive
#define mipi_decompressed_hfp(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_H_Front_Porch
#define mipi_decompressed_hsw(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_H_Sync_Width
#define mipi_decompressed_hbp(id)  \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_H_Back_Porch
#define mipi_decompressed_vfp(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_V_Front_Porch
#define mipi_decompressed_vsw(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_V_Sync_Width
#define mipi_decompressed_vbp(id)   \
	mipi_video_timing_table[id].MIPI_inputl[1].MIPI_V_Back_Porch

#define video_3d(id)   mipi_video_timing_table[id].video_3D_type

struct EDID_Timing_Format v_edid_timing;

void sp_tx_show_information(void)
{
	unchar c, c1;
	uint h_res, h_act, v_res, v_act;
	uint h_fp, h_sw, h_bp, v_fp, v_sw, v_bp;

	ulong xdata fresh_rate;
	ulong xdata pclk;
	ulong xdata M_val;
	ulong xdata N_val;

	TRACE("\n******************SP Video Information*******************\n");

	ReadReg(TX_P0, SP_TX_LANE_COUNT_SET_REG, &c);
	TRACE1("LC = %02X\n", c);

	ReadReg(TX_P0, SP_TX_LINK_BW_SET_REG, &c);
	switch (c) {
	case 0x06:
		TRACE("BW = 1.62G\n");
		/* str_clk = 162; */
		sp_tx_pclk_calc(BW_162G, &pclk, &M_val, &N_val);

		break;
	case 0x0a:
		TRACE("BW = 2.7G\n");
		/* str_clk = 270; */
		sp_tx_pclk_calc(BW_27G, &pclk, &M_val, &N_val);
		break;
	case 0x14:
		TRACE("BW = 5.4G\n");
		/* str_clk = 540; */
		sp_tx_pclk_calc(BW_54G, &pclk, &M_val, &N_val);
		break;
	case 0x19:
		TRACE("BW = 6.75G\n");
		/* str_clk = 675; */
		sp_tx_pclk_calc(BW_675G, &pclk, &M_val, &N_val);
		break;
	default:
		break;
	}

	delay_ms(1);

	TRACE3("M =%lu, N=%lu, PCLK=%d MHz\n", M_val, N_val, (uint)pclk);

	ReadReg(TX_P2, SP_TX_TOTAL_LINE_STA_L, &c);
	ReadReg(TX_P2, SP_TX_TOTAL_LINE_STA_H, &c1);

	v_res = c1;
	v_res = v_res << 8;
	v_res = v_res + c;


	ReadReg(TX_P2, SP_TX_ACT_LINE_STA_L, &c);
	ReadReg(TX_P2, SP_TX_ACT_LINE_STA_H, &c1);

	v_act = c1;
	v_act = v_act << 8;
	v_act = v_act + c;


	ReadReg(TX_P2, SP_TX_TOTAL_PIXEL_STA_L, &c);
	ReadReg(TX_P2, SP_TX_TOTAL_PIXEL_STA_H, &c1);

	h_res = c1;
	h_res = h_res << 8;
	h_res = h_res + c;


	ReadReg(TX_P2, SP_TX_ACT_PIXEL_STA_L, &c);
	ReadReg(TX_P2, SP_TX_ACT_PIXEL_STA_H, &c1);

	h_act = c1;
	h_act = h_act << 8;
	h_act = h_act + c;

	ReadReg(TX_P2, SP_TX_H_F_PORCH_STA_L, &c);
	ReadReg(TX_P2, SP_TX_H_F_PORCH_STA_H, &c1);

	h_fp = c1;
	h_fp = h_fp << 8;
	h_fp = h_fp + c;

	ReadReg(TX_P2, SP_TX_H_SYNC_STA_L, &c);
	ReadReg(TX_P2, SP_TX_H_SYNC_STA_H, &c1);

	h_sw = c1;
	h_sw = h_sw << 8;
	h_sw = h_sw + c;

	ReadReg(TX_P2, SP_TX_H_B_PORCH_STA_L, &c);
	ReadReg(TX_P2, SP_TX_H_B_PORCH_STA_H, &c1);

	h_bp = c1;
	h_bp = h_bp << 8;
	h_bp = h_bp + c;

	ReadReg(TX_P2, SP_TX_V_F_PORCH_STA, &c);
	v_fp = c;

	ReadReg(TX_P2, SP_TX_V_SYNC_STA, &c);
	v_sw = c;

	ReadReg(TX_P2, SP_TX_V_B_PORCH_STA, &c);
	v_bp = c;

	TRACE2("Total resolution is %d * %d\n", h_res, v_res);

	TRACE3("HF=%d, HSW=%d, HBP=%d\n",  h_fp, h_sw, h_bp);
	TRACE3("VF=%d, VSW=%d, VBP=%d\n", v_fp, v_sw, v_bp);
	TRACE2("Active resolution is %d * %d",  h_act, v_act);

	if (h_res == 0 || v_res == 0)
		fresh_rate = 0;
	else {
		fresh_rate = pclk * 1000;
		fresh_rate = fresh_rate / h_res;
		fresh_rate = fresh_rate * 1000;
		fresh_rate = fresh_rate / v_res;
	}
	TRACE1("   @ %ldHz\n", fresh_rate);

	ReadReg(TX_P0, SP_TX_VID_CTRL, &c);

	if ((c & 0x06) == 0x00)
		TRACE("ColorSpace: RGB,");
	else if ((c & 0x06) == 0x02)
		TRACE("ColorSpace: YCbCr422,");
	else if ((c & 0x06) == 0x04)
		TRACE("ColorSpace: YCbCr444,");

	ReadReg(TX_P0, SP_TX_VID_CTRL, &c);

	if ((c & 0xe0) == 0x00)
		TRACE("6 BPC\n");
	else if ((c & 0xe0) == 0x20)
		TRACE("8 BPC\n");
	else if ((c & 0xe0) == 0x40)
		TRACE("10 BPC\n");
	else if ((c & 0xe0) == 0x60)
		TRACE("12 BPC\n");


	TRACE("\n**************************************************\n");

}

void sp_tx_pclk_calc(
	enum SP_LINK_BW hbr_rbr, unsigned long *pclk,
	unsigned long *M_val, unsigned long *N_val)
{
	ulong xdata str_clk, temp;
	unchar c;

	switch (hbr_rbr) {
	case BW_675G:
		str_clk = 675;
		break;
	case BW_54G:
		str_clk = 540;
		break;
	case BW_27G:
		str_clk = 270;
		break;
	case BW_162G:
		str_clk = 162;
		break;
	default:
		str_clk = 540;
		break;
	}
	ReadReg(TX_P0, M_VID_2, &c);
	temp = c * 0x10000;
	ReadReg(TX_P0, M_VID_1, &c);
	temp = temp + c * 0x100;
	ReadReg(TX_P0, M_VID_0, &c);
	*M_val = temp + c;

	ReadReg(TX_P0, N_VID_2, &c);
	temp = c * 0x10000;
	ReadReg(TX_P0, N_VID_1, &c);
	temp = temp + c * 0x100;
	ReadReg(TX_P0, N_VID_0, &c);
	*N_val = temp + c;

	str_clk = str_clk * (*M_val);
	*pclk = str_clk / temp;
}

void API_Video_Mute_Control(unsigned char status)
{

	if (status) { /*mute on*/
		mute_video_flag = 1;
		/*set mute flag*/
		sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_MIPI_MUTE);
		/*clear mipi RX en*/
		sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_MIPI_RX_EN);
	} else { /*mute off*/
		mute_video_flag = 0;
		/*clear mute flag*/
		sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_MIPI_MUTE);
		/*set MIPI RX  EN*/
		sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_MIPI_RX_EN);
	}

}

void API_Configure_Audio_Input(unsigned char table_id)
{
	/*channel num*/
	WriteReg(TX_P2, AUDIO_CHANNEL_STATUS_6,
		audio_format_table[table_id].bAudioChNum << 5);
	/*layout*/
	if (audio_format_table[table_id].bAudioChNum > I2S_CH_2)
		sp_write_reg_or(TX_P2, AUDIO_CHANNEL_STATUS_6, 0x01);
	/*FS*/
	sp_write_reg_and_or(TX_P2, AUDIO_CHANNEL_STATUS_4, 0xf0,
		audio_format_table[table_id].bAudio_Fs);
	/*word length*/
	sp_write_reg_and_or(TX_P2, AUDIO_CHANNEL_STATUS_5, 0xf0,
		audio_format_table[table_id].bAudio_word_len);
	/*I2S/TDM*/
	if (audio_format_table[table_id].bAudioType == AUDIO_I2S) {
		sp_write_reg_or(TX_P2, AUDIO_CHANNEL_STATUS_6, I2S_SLAVE_MODE);
		sp_write_reg_and(TX_P2, AUDIO_CONTROL_REGISTER,
			~TDM_TIMING_MODE);
	} else {
		sp_write_reg_or(TX_P2, AUDIO_CHANNEL_STATUS_6, TDM_SLAVE_MODE);
	/*sp_write_reg_or(TX_P2, AUDIO_CONTROL_REGISTER, TDM_TIMING_MODE);*/
	}
	/*audio change flag*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_AUDIO_CHG);

}

void API_3D_Structure_Configuration(unsigned char table_id)
{
	/* 3D video structure setting*/
	sp_write_reg_and(RX_P0, AP_AV_STATUS, 0xfc);
	if (video_3d(table_id) > VIDEO_3D_SIDE_BY_SIDE)
		return;
	sp_write_reg_or(RX_P0, AP_AV_STATUS, video_3d(table_id));
}

void API_Custom_Register0_Configuration(unsigned char table_id)
{
	unchar i = 0;
	/*custom specific register*/
	if (mipi_video_timing_table[table_id].custom_reg0 != NULL) {
		while (mipi_video_timing_table[table_id].custom_reg0[i]
			.slave_addr) {
			WriteReg(mipi_video_timing_table[table_id]
				.custom_reg0[i].slave_addr,
				mipi_video_timing_table[table_id]
				.custom_reg0[i].reg,
				mipi_video_timing_table[table_id]
				.custom_reg0[i].val);
			i++;
		}
	}
}

void API_Custom_Register1_Configuration(unsigned char table_id)
{
	unchar i = 0;
	/*custom specific register*/
	if (mipi_video_timing_table[table_id].custom_reg1 != NULL) {
		while (mipi_video_timing_table[table_id].custom_reg1[i]
				.slave_addr) {
			WriteReg(mipi_video_timing_table[table_id]
				.custom_reg1[i].slave_addr,
				mipi_video_timing_table[table_id]
				.custom_reg1[i].reg,
				mipi_video_timing_table[table_id]
				.custom_reg1[i].val);
			i++;
		}
	}
}

void DSI_Video_Timing_Configuration(unsigned char table_id)
{
	/*configure pixel clock*/
	WriteReg(RX_P0, PIXEL_CLOCK_L,
		(mipi_pixel_frequency(table_id) / 1000000) & 0xFF);
	WriteReg(RX_P0, PIXEL_CLOCK_H,
		(mipi_pixel_frequency(table_id) / 1000000) >> 8);
	/*lane count*/
	sp_write_reg_and(RX_P1, MIPI_LANE_CTRL_0, 0xfc);
	sp_write_reg_or(RX_P1, MIPI_LANE_CTRL_0,
		mipi_lane_count(table_id));
	/*Htotal*/
	WriteReg(RX_P2, HORIZONTAL_TOTAL_PIXELS_L,
		mipi_original_htotal(table_id) & 0xFF);
	WriteReg(RX_P2, HORIZONTAL_TOTAL_PIXELS_H,
		mipi_original_htotal(table_id) >> 8);
	/*Hactive*/
	WriteReg(RX_P2, HORIZONTAL_ACTIVE_PIXELS_L,
		mipi_original_hactive(table_id) & 0xFF);
	WriteReg(RX_P2, HORIZONTAL_ACTIVE_PIXELS_H,
		mipi_original_hactive(table_id) >> 8);
	/*HFP*/
	WriteReg(RX_P2, HORIZONTAL_FRONT_PORCH_L,
		mipi_original_hfp(table_id) & 0xFF);
	WriteReg(RX_P2, HORIZONTAL_FRONT_PORCH_H,
		mipi_original_hfp(table_id) >> 8);
	/*HWS*/
	WriteReg(RX_P2, HORIZONTAL_SYNC_WIDTH_L,
		mipi_original_hsw(table_id) & 0xFF);
	WriteReg(RX_P2, HORIZONTAL_SYNC_WIDTH_H,
		mipi_original_hsw(table_id) >> 8);
	/*HBP*/
	WriteReg(RX_P2, HORIZONTAL_BACK_PORCH_L,
		mipi_original_hbp(table_id) & 0xFF);
	WriteReg(RX_P2, HORIZONTAL_BACK_PORCH_H,
		mipi_original_hbp(table_id) >> 8);
	/*Vactive*/
	WriteReg(RX_P2, ACTIVE_LINES_L,
		mipi_original_vactive(table_id) & 0xFF);
	WriteReg(RX_P2, ACTIVE_LINES_H,
		mipi_original_vactive(table_id)  >> 8);
	/*VFP*/
	WriteReg(RX_P2, VERTICAL_FRONT_PORCH,
		mipi_original_vfp(table_id));
	/*VWS*/
	WriteReg(RX_P2, VERTICAL_SYNC_WIDTH,
		mipi_original_vsw(table_id));
	/*VBP*/
	WriteReg(RX_P2, VERTICAL_BACK_PORCH,
		mipi_original_vbp(table_id));
	/*M value*/
	WriteReg(RX_P1, MIPI_PLL_M_NUM_23_16,
		(mipi_m_value(table_id) >> 16) & 0xff);
	WriteReg(RX_P1, MIPI_PLL_M_NUM_15_8,
		(mipi_m_value(table_id) >> 8) & 0xff);
	WriteReg(RX_P1, MIPI_PLL_M_NUM_7_0,
		mipi_m_value(table_id) & 0xff);
	/*N value*/
	WriteReg(RX_P1, MIPI_PLL_N_NUM_23_16,
		(mipi_n_value(table_id) >> 16) & 0xff);
	WriteReg(RX_P1, MIPI_PLL_N_NUM_15_8,
		(mipi_n_value(table_id) >> 8) & 0xff);
	WriteReg(RX_P1, MIPI_PLL_N_NUM_7_0,
		mipi_n_value(table_id) & 0xff);
	/*diff*/
	WriteReg(RX_P1, MIPI_DIGITAL_ADJ_1,
		mipi_diff_ratio(table_id));
}

void DSC_Video_Timing_Configuration(unsigned char table_id)
{
	unchar i;

	/*config uncompressed video format*/
	/*Htotal*/
	WriteReg(TX_P2, HORIZONTAL_TOTAL_PIXELS_L,
		(mipi_original_htotal(table_id) * mipi_compress_ratio(table_id))
		& 0xFF);
	WriteReg(TX_P2, HORIZONTAL_TOTAL_PIXELS_H,
		(mipi_original_htotal(table_id) * mipi_compress_ratio(table_id))
		>> 8);
	/*Hactive*/
	WriteReg(TX_P2, HORIZONTAL_ACTIVE_PIXELS_L,
		(mipi_original_hactive(table_id)
		 * mipi_compress_ratio(table_id)) & 0xFF);
	WriteReg(TX_P2, HORIZONTAL_ACTIVE_PIXELS_H,
		(mipi_original_hactive(table_id)
		 * mipi_compress_ratio(table_id)) >> 8);
	/*HFP*/
	WriteReg(TX_P2, HORIZONTAL_FRONT_PORCH_L,
		(mipi_original_hfp(table_id) * mipi_compress_ratio(table_id))
		& 0xFF);
	WriteReg(TX_P2, HORIZONTAL_FRONT_PORCH_H,
		(mipi_original_hfp(table_id) * mipi_compress_ratio(table_id))
		>> 8);
	/*HWS*/
	WriteReg(TX_P2, HORIZONTAL_SYNC_WIDTH_L,
		(mipi_original_hsw(table_id) * mipi_compress_ratio(table_id))
		& 0xFF);
	WriteReg(TX_P2, HORIZONTAL_SYNC_WIDTH_H,
		(mipi_original_hsw(table_id) * mipi_compress_ratio(table_id))
		>> 8);
	/*HBP*/
	WriteReg(TX_P2, HORIZONTAL_BACK_PORCH_L,
		(mipi_original_hbp(table_id) * mipi_compress_ratio(table_id))
		& 0xFF);
	WriteReg(TX_P2, HORIZONTAL_BACK_PORCH_H,
		(mipi_original_hbp(table_id) * mipi_compress_ratio(table_id))
		>> 8);
	/*Vtotal*/
	WriteReg(TX_P2, TOTAL_LINES_L,
		mipi_original_vtotal(table_id) & 0xFF);
	WriteReg(TX_P2, TOTAL_LINES_H,
		mipi_original_vtotal(table_id) >> 8);
	/*Vactive*/
	WriteReg(TX_P2, ACTIVE_LINES_L,
		mipi_original_vactive(table_id) & 0xFF);
	WriteReg(TX_P2, ACTIVE_LINES_H,
		mipi_original_vactive(table_id) >> 8);
	/*VFP*/
	WriteReg(TX_P2, VERTICAL_FRONT_PORCH,
		mipi_original_vfp(table_id));
	/*VWS*/
	WriteReg(TX_P2, VERTICAL_SYNC_WIDTH,
		mipi_original_vsw(table_id));
	/*VBP*/
	WriteReg(TX_P2, VERTICAL_BACK_PORCH,
		mipi_original_vbp(table_id));

	/*config uncompressed video format to woraround */
	/* downstream compatibility issues*/
	/*Htotal*/
	WriteReg(RX_P0, TOTAL_PIXEL_L_7E,
		mipi_decompressed_htotal(table_id) & 0xFF);
	WriteReg(RX_P0, TOTAL_PIXEL_H_7E,
		mipi_decompressed_htotal(table_id) >> 8);
	/*Hactive*/
	WriteReg(RX_P0, ACTIVE_PIXEL_L_7E,
		mipi_decompressed_hactive(table_id) & 0xFF);
	WriteReg(RX_P0, ACTIVE_PIXEL_H_7E,
		mipi_decompressed_hactive(table_id) >> 8);
	/*HFP*/
	WriteReg(RX_P0, HORIZON_FRONT_PORCH_L_7E,
		mipi_decompressed_hfp(table_id) & 0xFF);
	WriteReg(RX_P0, HORIZON_FRONT_PORCH_H_7E,
		mipi_decompressed_hfp(table_id) >> 8);
	/*HWS*/
	WriteReg(RX_P0, HORIZON_SYNC_WIDTH_L_7E,
		mipi_decompressed_hsw(table_id) & 0xFF);
	WriteReg(RX_P0, HORIZON_SYNC_WIDTH_H_7E,
		mipi_decompressed_hsw(table_id) >> 8);
	/*HBP*/
	WriteReg(RX_P0, HORIZON_BACK_PORCH_L_7E,
		mipi_decompressed_hbp(table_id)  & 0xFF);
	WriteReg(RX_P0, HORIZON_BACK_PORCH_H_7E,
		mipi_decompressed_hbp(table_id)  >> 8);

	/*config DSC decoder internal blank timing for decoder to start*/
	WriteReg(RX_P1, H_BLANK_L, ((mipi_original_htotal(table_id)
		- mipi_original_hactive(table_id))) & 0xFF);
	WriteReg(RX_P1, H_BLANK_H, ((mipi_original_htotal(table_id)
		- mipi_original_hactive(table_id))) >> 8);

	/*compress ratio  RATIO [7:6] 3:div2; 0,1,2:div3*/
	sp_write_reg_and(RX_P0, R_I2C_1, 0x3f);
	sp_write_reg_or(RX_P0, R_I2C_1,
		(5 - mipi_compress_ratio(table_id)) << 6);

	/*PPS table*/
	if (mipi_video_timing_table[table_id].pps_reg != NULL) {
		for (i = 0; i < 0x80; i += 0x10)
			WriteBlockReg(RX_P2, R_PPS_REG_0 + i, 0x10,
				(unsigned char *)mipi_video_timing_table
				[table_id].pps_reg + i);
	}
}

void API_ODFC_Configuration(unsigned char table_id)
{

	/*config input reference clock frequency 27MHz/19.2MHz*/
	sp_write_reg_and(RX_P1, MIPI_DIGITAL_PLL_16,
		~(REF_CLK_27000kHz << MIPI_FREF_D_IND));
	sp_write_reg_or(RX_P1, MIPI_DIGITAL_PLL_16,
		(((XTAL_FRQ >= 26000000UL) && (XTAL_FRQ <= 27000000UL)) ?
		(REF_CLK_27000kHz << MIPI_FREF_D_IND)
		: (REF_CLK_19200kHz << MIPI_FREF_D_IND)));
	/*post divider*/
	sp_write_reg_and(RX_P1, MIPI_DIGITAL_PLL_8, 0x0f);
	sp_write_reg_or(RX_P1, MIPI_DIGITAL_PLL_8,
		mipi_post_divider(table_id) << 4);

	/*add patch for MIS2-125 (5pcs ANX7625 fail ATE MBIST test)*/
	sp_write_reg_and(RX_P1, MIPI_DIGITAL_PLL_7,
	~MIPI_PLL_VCO_TUNE_REG_VAL);

	/*reset ODFC PLL*/
	sp_write_reg_and(RX_P1, MIPI_DIGITAL_PLL_7,
		~MIPI_PLL_RESET_N);
	sp_write_reg_or(RX_P1, MIPI_DIGITAL_PLL_7,
		MIPI_PLL_RESET_N);
	/*force PLL lock*/
	/*WriteReg(TX_P0, DP_CONFIG_24, 0x0c);*/ /*Force HPD*/

}
void API_DSC_Configuration(unsigned char table_id)
{
	DSC_Video_Timing_Configuration(table_id);
	/*DSC enable*/
	sp_write_reg_or(RX_P0, R_DSC_CTRL_0, DSC_EN);
}


/* workaround from analog team; to deal with issue MIS2-88 */
static void  DSI_analog_workaround(void)
{
#ifdef REV_AA
	unsigned char  RegValue;

#ifdef HS_Swing_300mV
	/* lpcd reference voltage: 400 mV, lprx reference voltage: 800 mV,
	* lptx termination: 200 Ohm (default)
	*/
	RegValue = (3 << ref_sel_lpcd) | (3 << ref_sel_lprx)
		| (1 << sel_lptx_term);
	WriteReg(RX_P1, MIPI_RX_REG5, RegValue);
	TRACE("300mV HS swing analog workaround applied.\n");
#endif /* HS_Swing_300mV */

#ifdef HS_Swing_400mV
	/* power on ODFC PLL block*/
	ReadReg(TCPC_INTERFACE, CHIP_POWER_CTRL, &RegValue);
	RegValue &= ~(1 << PD_V10_ODFC);
	WriteReg(TCPC_INTERFACE, CHIP_POWER_CTRL, RegValue);
	delay_ms(1);

	/* power on MIPI-DSI RX PHY and digital block*/
	ReadReg(RX_P1, MIPI_LANE_CTRL_10, &RegValue);
	RegValue &= ~(1 << MIPI_POWER_DOWN);
	WriteReg(RX_P1, MIPI_LANE_CTRL_10, RegValue);
	delay_ms(1);

	/* DSI I/O mode*/
	ReadReg(RX_P1, MIPI_DIGITAL_PLL_18, &RegValue);
	RegValue |= (1 << MIPI_DPI_SELECT);
	WriteReg(RX_P1, MIPI_DIGITAL_PLL_18, RegValue);

	/* optional, this only enables SUB2_AUXN to output test signals*/
	ReadReg(TCPC_INTERFACE, ANALOG_CTRL_9, &RegValue);
	RegValue |= (1 << TEST_EN_MI);
	WriteReg(TCPC_INTERFACE, ANALOG_CTRL_9, RegValue);

	/* atesto is vref_lprx*/
	ReadReg(RX_P1, MIPI_RX_REG7, &RegValue);
	RegValue &= 0xE0;
	RegValue |= (1 << test_enable) | (0 << test_signal_sel);
	WriteReg(RX_P1, MIPI_RX_REG7, RegValue);

	/* atesto is avdd10*/
	ReadReg(TX_P1, DPPLL_REG4, &RegValue);
	RegValue &= 0xF4;
	RegValue |= (1 << atest_enable) | (2 << test_sig_sel);
	WriteReg(TX_P1, DPPLL_REG4, RegValue);

	TRACE("400mV HS swing analog workaround applied.\n");
#endif /* HS_Swing_400mV */

#endif /* REV_AA */
}


static void  swap_DSI_lane3(void)
{
#if (defined(REV_AB) || defined(BONDING_CHANGED_BASED_ON_REV_AA))
	unsigned char  RegValue;
	/* swap MIPI-DSI data lane 3 P and N */
	ReadReg(RX_P1, MIPI_SWAP, &RegValue);
	RegValue |= (1 << MIPI_SWAP_CH3);
	WriteReg(RX_P1, MIPI_SWAP, RegValue);
#endif
}


void API_DSI_Configuration(unsigned char table_id)

{
	unsigned char  RegValue;

	/* apply workaround from analog team */
	DSI_analog_workaround();

	/* swap MIPI-DSI data lane 3 P and N */
	swap_DSI_lane3();

	/* DSI clock settings */
	RegValue = (0 << MIPI_HS_PWD_CLK) |
		(0 << MIPI_HS_RT_CLK)  |
		(0 << MIPI_PD_CLK)     |
		(1 << MIPI_CLK_RT_MANUAL_PD_EN) |
		(1 << MIPI_CLK_HS_MANUAL_PD_EN) |
		(0 << MIPI_CLK_DET_DET_BYPASS)  |
		(0 << MIPI_CLK_MISS_CTRL)       |
		(0 << MIPI_PD_LPTX_CH_MANUAL_PD_EN);
	WriteReg(RX_P1, MIPI_PHY_CONTROL_3, RegValue);

	/* Decreased HS prepare timing delay from 160ns to 80ns work with
	*     a) Dragon board 810 series (Qualcomm AP)
	*     b) Moving Pixel DSI source (PG3A pattern generator +
	*	P332 D-PHY Probe) default D-PHY timing
	*/
	WriteReg(RX_P1, MIPI_TIME_HS_PRPR, 0x10);  /* 5ns/step */

	sp_write_reg_or(RX_P1, MIPI_DIGITAL_PLL_18,
		SELECT_DSI<<MIPI_DPI_SELECT); /* enable DSI mode*/

	DSI_Video_Timing_Configuration(table_id);

	API_ODFC_Configuration(table_id);

	/*toggle m, n ready*/
	sp_write_reg_and(RX_P1, MIPI_DIGITAL_PLL_6,
		~(MIPI_M_NUM_READY | MIPI_N_NUM_READY));
	usleep_range(1000, 1100);
	sp_write_reg_or(RX_P1, MIPI_DIGITAL_PLL_6,
		MIPI_M_NUM_READY | MIPI_N_NUM_READY);

	/*configure integer stable register*/
	WriteReg(RX_P1, MIPI_VIDEO_STABLE_CNT, 0x02);
	/*power on MIPI RX*/
	WriteReg(RX_P1, MIPI_LANE_CTRL_10, 0x00);
	WriteReg(RX_P1, MIPI_LANE_CTRL_10, 0x80);
	usleep_range(10000, 11000);
}

void API_DPI_Configuration(unsigned char table_id)
{
	/*configure pixel clock*/
	WriteReg(RX_P0, PIXEL_CLOCK_L,
		(mipi_pixel_frequency(table_id) / 1000000) & 0xFF);
	WriteReg(RX_P0, PIXEL_CLOCK_H,
		(mipi_pixel_frequency(table_id) / 1000000) >> 8);
	/*set DPI mode*/
	/* set to DPI PLL module sel*/
	WriteReg(RX_P1, MIPI_DIGITAL_PLL_9, 0x20);
	/*power down MIPI*/
	WriteReg(RX_P1, MIPI_LANE_CTRL_10, 0x08);
	/*enable DPI mode*/
	WriteReg(RX_P1, MIPI_DIGITAL_PLL_18, 0x1C);
	/*set first edge*/
	WriteReg(TX_P2, VIDEO_CONTROL_0, 0x05);

}


void DPI_Configuration(unsigned char table_id)
{
	TRACE1("DPI_Configuration Input Index = %02X\n", table_id);

	/*DSC disable*/
	sp_write_reg_and(RX_P0, R_DSC_CTRL_0, ~DSC_EN);
	API_Custom_Register0_Configuration(table_id);
	API_DPI_Configuration(table_id);
	API_Custom_Register1_Configuration(table_id);

	/*set MIPI RX  EN*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_MIPI_RX_EN);
	/*clear mute flag*/
	sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_MIPI_MUTE);
	mute_video_flag = 0;
}

void command_DPI_Configuration(unsigned char table_id)
{
	TRACE1("command_DPI_Configuration Input Index = %02X\n", table_id);

	delay_tab_id = table_id;
	delay_video_cfg = DPI_Configuration;
	mute_video_flag = 1;
	/*set video change*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_VIDEO_CHG);
	return;

}

void DPI_DSC_Configuration(unsigned char table_id)
{
	TRACE1("DPI_DSC_Configuration Input Index=%02X\n", table_id);

	API_Custom_Register0_Configuration(table_id);
	API_DSC_Configuration(table_id);
	API_ODFC_Configuration(table_id);
	API_DPI_Configuration(table_id);
	API_Custom_Register1_Configuration(table_id);

	/*set MIPI RX  EN*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_MIPI_RX_EN);
	/*clear mute flag*/
	sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_MIPI_MUTE);
	mute_video_flag = 0;


}

void command_DPI_DSC_Configuration(unsigned char table_id)
{
	TRACE1("command_DPI_DSC_Configuration Input Index=%02X\n", table_id);


	delay_tab_id = table_id;
	delay_video_cfg = DPI_DSC_Configuration;

	mute_video_flag = 1;
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_VIDEO_CHG); /*set video change*/
	return;



}
void DSI_Configuration(unsigned char table_id)
{
	TRACE1("DSI_Configuration Input Index = %02X\n", table_id);

	API_Custom_Register0_Configuration(table_id);
	/*DSC disable*/
	sp_write_reg_and(RX_P0, R_DSC_CTRL_0, ~DSC_EN);
	API_DSI_Configuration(table_id);
	API_Custom_Register1_Configuration(table_id);

	/*set MIPI RX  EN*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_MIPI_RX_EN);
	/*clear mute flag*/
	sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_MIPI_MUTE);
	mute_video_flag = 0;

}

void command_DSI_Configuration(unsigned char table_id)
{
	TRACE1("command_DSI_Configuration Input Index = %02X\n", table_id);

	delay_tab_id = table_id;
	delay_video_cfg = DSI_Configuration;
	mute_video_flag = 1;
	/*set video change*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_VIDEO_CHG);

}

void DSI_DSC_Configuration(unsigned char table_id)

{
	TRACE1("DSI_DSC_Configuration Input Index = %02X\n", table_id);

	API_Custom_Register0_Configuration(table_id);

	API_DSC_Configuration(table_id);

	API_DSI_Configuration(table_id);
	API_Custom_Register1_Configuration(table_id);

	/*set MIPI RX  EN*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_MIPI_RX_EN);
	/*clear mute flag*/
	sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_MIPI_MUTE);
	mute_video_flag = 0;

}

void command_DSI_DSC_Configuration(unsigned char table_id)

{
	TRACE1("command_DSI_DSC_Configuration Input Index = %02X\n", table_id);

	delay_tab_id = table_id;
	delay_video_cfg = DSI_DSC_Configuration;
	mute_video_flag = 1;
	/*set video change*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_VIDEO_CHG);
	return;

}

void command_Configure_Audio_Input(unsigned char table_id)
{
	TRACE1("command_Configure_Audio_Input Input Index = %02X\n", table_id);

	API_Configure_Audio_Input(table_id);

}

void command_Mute_Video(unsigned char mute_flag)
{
	TRACE1("command_Mute_Video: %d\n", mute_flag);

	if (mute_flag == 1) {
		/*disable video input*/
		API_Video_Mute_Control(1);
	} else {
		/*enable video input*/
		API_Video_Mute_Control(0);
	}

}



