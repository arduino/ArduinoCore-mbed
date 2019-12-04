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


#define _SP_C_

#include "display.h"
#include "anx7625_driver.h"
#include "MI2_REG.h"


static void sp_tx_rst_aux(void);



int sp_read_reg(uint8_t slave_addr, uint8_t offset, uint8_t *buf)
{
	return  Read_Reg(slave_addr, offset, buf);
}

int sp_write_reg(uint8_t slave_addr, uint8_t offset, uint8_t value)
{

	WriteReg(slave_addr, offset, value);
	return 0;
}


unchar sp_get_rx_bw(void)
{
	return sp_tx_bw;
}

unchar sp_get_lane_count(void)
{
	return sp_tx_lane_count;
}



/********************************************/
/* Check if it is ANALOGIX dongle.
*/
/********************************************/
const unsigned char ANX_OUI[3] = { 0x00, 0x22, 0xB9 };

unsigned char is_anx_dongle(void)
{
	unsigned char buf[3];

	/*  0x0500~0x0502: BRANCH_IEEE_OUI*/
	sp_tx_aux_dpcdread_bytes(0x00, 0x05, 0x00, 3, buf);
	if (buf[0] == ANX_OUI[0] && buf[1] == ANX_OUI[1]
		&& buf[2] == ANX_OUI[2]) {
		return 1;
	}
	return 0;
}

void sp_tx_get_rx_bw(unsigned char *bw)
{
	unchar data_buf[4];
	/*When ANX dongle connected, if CHIP_ID = 0x7750, bandwidth is 6.75G,
	* because ANX7750 DPCD 0x052x was not available.
	*/
	if (is_anx_dongle()) {
		sp_tx_aux_dpcdread_bytes(0x00, 0x05, 0x03, 0x04, data_buf);
		if ((data_buf[0] == 0x37) && (data_buf[1] == 0x37)
			&& (data_buf[2] == 0x35) && (data_buf[3] == 0x30)) {
			*bw = BW_675G;
		} else {
			sp_tx_aux_dpcdread_bytes(0x00, 0x05, 0x21, 1, bw);
	/* some ANX dongle read out value 0, retry standard register. */
		if (((*bw) == 0) || ((*bw) == 0xff))
			sp_tx_aux_dpcdread_bytes(0x00, 0x00, DPCD_MAX_LINK_RATE,
				1, bw);
		}
	} else {
		sp_tx_aux_dpcdread_bytes(0x00, 0x00, DPCD_MAX_LINK_RATE, 1, bw);

	}

}



void SP_CTRL_Dump_Reg(void)
{
	unsigned int BEGIN = 0x70, END = 0xFF, i = 0;
	uint8_t temp = 0, D3 = 0, D4 = 0, D5 = 0, D6 = 0, D7 = 0, D8 = 0, c = 0;
	unsigned int Maud = 0, Naud = 0;


	for (i = BEGIN; i <= END; i++) {
		sp_read_reg(SP_TX_PORT0_ADDR, i, &temp);
		pr_err("reg:%x,offset:%x, temp:%d\n",
			SP_TX_PORT0_ADDR, i, temp);
	}
	for (i = BEGIN; i <= END; i++) {
		sp_read_reg(SP_TX_PORT2_ADDR, i, &temp);
		pr_err("reg:%x,offset:%x, temp:%d\n",
			SP_TX_PORT2_ADDR, i, temp);
	}

	sp_read_reg(SP_TX_PORT0_ADDR, 0xD3, &D3);
	sp_read_reg(SP_TX_PORT0_ADDR, 0xD4, &D4);
	sp_read_reg(SP_TX_PORT0_ADDR, 0xD5, &D5);
	Maud = D5 << 16 | D4 << 8 | D3;

	sp_read_reg(SP_TX_PORT0_ADDR, 0xD6, &D6);
	sp_read_reg(SP_TX_PORT0_ADDR, 0xD7, &D7);
	sp_read_reg(SP_TX_PORT0_ADDR, 0xD8, &D8);
	Naud = D8 << 16 | D7 << 8 | D6;

	sp_read_reg(MIPI_RX_PORT1_ADDR, MIPI_ANALOG_PWD_CTRL1, &c);
	if (c & 0x10) {
		sp_read_reg(SP_TX_PORT0_ADDR, SP_TX_LINK_BW_SET_REG, &c);
		if (c == 0x06)
			pr_err("   BW = 1.62G");
		else if (c == 0x0a)
			pr_err("   BW = 2.7G");
		else if (c == 0x14)
			pr_err("   BW = 5.4G");
		else if (c == 0x19)
			pr_err("   BW = 6.75G");
	} else {
		sp_read_reg(SP_TX_PORT0_ADDR, SP_TX_LINK_BW_SET_REG, &c);
		if (c == 0x06)
			pr_err("   BW = 1.62G");
		else if (c == 0x0a)
			pr_err("   BW = 2.7G");
		else if (c == 0x14)
			pr_err("   BW = 5.4G");
		else if (c == 0x19)
			pr_err("   BW = 6.75G");
	}

	/*freqency = (Maud / Naud) * (540000 / 512);*/
	pr_err("Maud:%d, Naud:%d, (Maud / Naud * 540 000 / 512) KHz\n",
		Maud, Naud);
}




void DP_Process_Start(void)
{

	unsigned char edid_blocks[FOUR_BLOCK_SIZE];
	unsigned char  blocks_num, c;

	/*Read EDID*/
	if ((mute_video_flag == 0) || (delay_video_cfg == 0)) {

		TRACE("HPD high, and create context\n");

		if (slimport_edid_p == NULL) {
			slimport_edid_p =
				malloc(sizeof(struct s_edid_data));
			memset(slimport_edid_p, 0, sizeof(struct s_edid_data));
		} else	{
			TRACE("context already existed.\n");
		}

		/*reset edid timing information*/
		memset(&v_edid_timing, 0, sizeof(v_edid_timing));

		blocks_num = sp_tx_edid_read(edid_blocks);

		if (cable_connected == 0) {
			hpd_status = 0;
			TRACE("DP_Process_Start HPD low,destroy context\n");

			if (slimport_edid_p != NULL)
				free(slimport_edid_p);
			slimport_edid_p = NULL;
			return;
		}

		/*read EDID*/
		if (slimport_edid_p != NULL) {

			struct s_edid_data *p_edid =
				(struct s_edid_data *)slimport_edid_p;

			memcpy((uint8_t *)(p_edid->EDID_block_data),
				edid_blocks, (blocks_num + 1) * 128);

			p_edid->edid_block_num = blocks_num;



#ifdef DEBUG_LOG_OUTPUT
	{
		int j, k;
		unsigned char  pLine[50];

		for (k = 0, j = 0; k < (128 * ((uint)blocks_num + 1)); k++) {
			if ((k & 0x0f) == 0) {
				snprintf((char*)&pLine[j], 14, "edid:[%02hhx] %02hhx ",
			(uint)(k / 0x10),
			(uint) *((uint8_t *)(p_edid->EDID_block_data) + k));
				j = j + 13;
			} else {
				snprintf((char*)&pLine[j], 4, "%02hhx ",
			(uint) *((uint8_t *)(p_edid->EDID_block_data) + k));
				j = j + 3;
				}
			if ((k & 0x0f) == 0x0f) {
				TRACE("%s\n", pLine);
				j = 0;
			}
		}
	}
#endif

			sp_tx_get_rx_bw(&sp_tx_bw);

			sp_tx_aux_dpcdread_bytes(
				0x00, 0x00, DPCD_MAX_LANE_COUNT,
				1, &sp_tx_lane_count);

			sp_tx_lane_count = sp_tx_lane_count & 0x1f;

			sp_tx_aux_dpcdread_bytes(0x06, 0x80, 0x28,
				1, ByteBuf);  /* read Bcap*/

			if ((ByteBuf[0] & 0x01) == 1)
				hdcp_cap = HDCP14_SUPPORT;
			else
				hdcp_cap = NO_HDCP_SUPPORT;

		/*not support HDCP*/
			sp_write_reg_and(RX_P1, 0xee, 0x9f);

			if (hdcp_cap == HDCP14_SUPPORT) {
				/*sp_write_reg_or(RX_P1, 0xee, 0x20);*/
				TRACE("HDCP1.4\n");
			} else if (hdcp_cap == HDCP22_SUPPORT) {
				/*sp_write_reg_or(RX_P1, 0xee, 0x40);*/
				TRACE("HDCP2.2\n");
			}

			/*try auth flag*/
			sp_write_reg_or(RX_P1, 0xec, 0x10);
			/* interrupt for DRM*/
			sp_write_reg_or(RX_P1, 0xff, 0x01);

	TRACE("\nThe dongle MAX BW=%02x, MAX Lane cnt=%02x, HDCP=%02x\n",
			(unsigned int)sp_tx_bw, (unsigned int)sp_tx_lane_count,
			(unsigned int)hdcp_cap);

			sp_read_reg(MIPI_RX_PORT1_ADDR, 0x86, &c);

			TRACE("Secure OCM version=%02x\n", (unsigned int)c);

			if (cable_connected == 0) {
				hpd_status = 0;
				TRACE("second check HPD low,destroy context\n");
				if (slimport_edid_p != NULL)
					free(slimport_edid_p);
				slimport_edid_p = NULL;
				return;
			}

		}
	} else {

		TRACE("HPD high agin, check HDCP.\n");

		sp_write_reg_and(RX_P1, 0xee, 0x9f);  /*not support HDCP*/
		if (hdcp_cap == HDCP14_SUPPORT) {
			sp_write_reg_or(RX_P1, 0xee, 0x20);
			TRACE("HDCP1.4\n");
		} else if (hdcp_cap == HDCP22_SUPPORT) {
			/*sp_write_reg_or(RX_P1, 0xee, 0x40);*/
			TRACE("HDCP2.2\n");
		}
		sp_write_reg_or(RX_P1, 0xec, 0x10);  /*try auth flag*/
		sp_write_reg_or(RX_P1, 0xff, 0x01);  /* interrupt for DRM*/

	}

	if ((mute_video_flag == 1) && (delay_video_cfg != 0)) {
		/*config video*/
		delay_video_cfg(delay_tab_id);
		delay_video_cfg = 0;

		TRACE("##### delay video config called.\n");

	}

}

void DP_Process_Stop(void)
{


	TRACE("HPD low, and destroy context\n");

	if (slimport_edid_p != NULL)
		free(slimport_edid_p);
	slimport_edid_p = NULL;

}


static void wait_aux_op_finish(unchar *err_flag);
#define write_dpcd_addr(addrh, addrm, addrl) \
	do { \
		unchar temp; \
		if (ReadReg(RX_P0, AP_AUX_ADDR_7_0) != (unchar)addrl) \
			WriteReg(RX_P0, AP_AUX_ADDR_7_0, (unchar)addrl); \
		if (ReadReg(RX_P0, AP_AUX_ADDR_15_8) != (unchar)addrm) \
			WriteReg(RX_P0, AP_AUX_ADDR_15_8, (unchar)addrm); \
		Read_Reg(RX_P0, AP_AUX_ADDR_19_16, &temp); \
		if ((unchar)(temp & 0x0F)  != ((unchar)addrh & 0x0F)) \
			WriteReg(RX_P0, AP_AUX_ADDR_19_16, \
				(temp  & 0xF0) | ((unchar)addrh)); \
	} while (0)

#define sp_tx_aux_polling_enable() \
	sp_write_reg_or(TX_P0, TX_DEBUG1, POLLING_EN)
#define sp_tx_aux_polling_disable() \
	sp_write_reg_and(TX_P0, TX_DEBUG1, ~POLLING_EN)



unchar sp_tx_aux_dpcdread_bytes(unchar addrh, unchar addrm,
		unchar addrl, unchar cCount, unchar *pBuf)
{
	unchar c, i;
	unchar bOK;

	/*command and length*/
	c = ((cCount - 1) << 4) | 0x09;
	WriteReg(RX_P0, AP_AUX_COMMAND, c);
	/*address*/
	write_dpcd_addr(addrh, addrm, addrl);
	/*aux en*/
	sp_write_reg_or(RX_P0, AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	usleep_range(2000, 2100);
	/* TRACE3("auxch addr = 0x%02x%02x%02x\n", addrh,addrm,addrl);*/
	wait_aux_op_finish(&bOK);
	if (bOK == AUX_ERR) {
		TRACE("aux read failed\n");
		return AUX_ERR;
	}

	for (i = 0; i < cCount; i++) {
		Read_Reg(RX_P0, AP_AUX_BUFF_START + i, &c);
		*(pBuf + i) = c;
		/*TRACE2("Buf[%d] = 0x%02x\n", (uint)i, *(pBuf + i));*/
		if (i >= MAX_BUF_CNT)
			break;
	}

	return AUX_OK;
}


unchar sp_tx_aux_dpcdwrite_bytes(unchar addrh, unchar addrm, unchar addrl,
		unchar cCount, unchar *pBuf)
{
	unchar c, i, ret;

	/*command and length*/
	c =  ((cCount - 1) << 4) | 0x08;
	WriteReg(RX_P0, AP_AUX_COMMAND, c);
	/*address*/
	write_dpcd_addr(addrh, addrm, addrl);
	/*data*/
	for (i = 0; i < cCount; i++) {
		c = *pBuf;
		pBuf++;
		WriteReg(RX_P0, AP_AUX_BUFF_START + i, c);

		if (i >= 15)
			break;
	}
	/*aux en*/
	sp_write_reg_or(RX_P0,  AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	wait_aux_op_finish(&ret);
	TRACE("aux write done\n");
	return ret;
}
static void wait_aux_op_finish(unchar *err_flag)
{
	unchar cnt;
	unchar c;

	*err_flag = 0;
	cnt = 150;
	while (ReadReg(RX_P0, AP_AUX_CTRL_STATUS) & AP_AUX_CTRL_OP_EN) {
		usleep_range(2000, 2100);
		if ((cnt--) == 0) {
			TRACE("aux operate failed!\n");
			*err_flag = 1;
			break;
		}
	}

	Read_Reg(RX_P0, AP_AUX_CTRL_STATUS, &c);
	if (c & 0x0F) {
		TRACE1("wait aux operation status %02x\n", (uint)c);
		*err_flag = 1;
	}
}



static void sp_tx_rst_aux(void)
{
	sp_write_reg_or(TX_P2, RST_CTRL2, AUX_RST);
	sp_write_reg_and(TX_P2, RST_CTRL2, ~AUX_RST);
}

unchar  sp_tx_aux_wr(unchar offset)
{
	unchar c;

	WriteReg(RX_P0, AP_AUX_BUFF_START, offset);
	WriteReg(RX_P0, AP_AUX_COMMAND, 0x04);
	sp_write_reg_or(RX_P0, AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	wait_aux_op_finish(&c);

	return c;
}

unchar  sp_tx_aux_rd(unchar len_cmd)
{
	unchar c;

	WriteReg(RX_P0, AP_AUX_COMMAND, len_cmd);
	sp_write_reg_or(RX_P0, AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	wait_aux_op_finish(&c);

	return c;
}

unchar sp_tx_get_edid_block(void)
{
	unchar c;

	sp_tx_aux_wr(0x7e);
	sp_tx_aux_rd(0x01);
	Read_Reg(RX_P0, AP_AUX_BUFF_START, &c);
	TRACE1(" EDID Block = %d\n", (int)(c + 1));

	if (c > 3)
		c = 1;

	return c;
}

unchar edid_read(unchar offset, unchar *pblock_buf)
{
	unchar c, cnt = 0;

	c = 0;
	while (cnt < 3) {
		sp_tx_aux_wr(offset);
		/* set I2C read com 0x01 mot = 0 and read 16 bytes          */
		sp_tx_aux_rd(0xf1);

		if (c == 1) {
			sp_tx_rst_aux();
			TRACE("edid read failed, aux reset!\n");
			cnt++;
		} else {
			for (c = 0; c < 16; c++)
				Read_Reg(RX_P0, AP_AUX_BUFF_START + c,
					&(pblock_buf[c]));
			return 0;
		}

	}

	return 1;

}

unchar segments_edid_read(unchar segment, unchar offset)
{
	unchar c, cnt = 0;
	int i;

	/*write address only*/
	WriteReg(RX_P0, AP_AUX_ADDR_7_0, 0x30);
	WriteReg(RX_P0, AP_AUX_COMMAND, 0x04);
	WriteReg(RX_P0, AP_AUX_CTRL_STATUS,
		AP_AUX_CTRL_ADDRONLY | AP_AUX_CTRL_OP_EN);

	wait_aux_op_finish(&c);
	/*write segment address*/
	sp_tx_aux_wr(segment);
	/*data read*/
	WriteReg(RX_P0, AP_AUX_ADDR_7_0, 0x50);

	while (cnt < 3) {
		sp_tx_aux_wr(offset);
		/* set I2C read com 0x01 mot = 0 and read 16 bytes*/
		sp_tx_aux_rd(0xf1);

		if (c == 1) {
			sp_tx_rst_aux();
			TRACE("segment edid read failed, aux reset!\n");
			cnt++;
		} else {
			for (i = 0; i < 16; i++)
				Read_Reg(RX_P0, AP_AUX_BUFF_START + i, &c);
			return 0;
		}

	}

	return 1;
}

static bool edid_checksum_result(unchar *pBuf)
{
	unsigned char cnt, checksum;
	unsigned char g_edid_checksum;

	checksum = 0;

	for (cnt = 0; cnt < 0x80; cnt++)
		checksum = checksum + pBuf[cnt];

	g_edid_checksum = checksum - pBuf[0x7f];
	g_edid_checksum = ~g_edid_checksum + 1;

	return checksum == 0 ? 1 : 0;
}

void check_edid_data(unchar *pblock_buf)
{
	unchar i;

	for (i = 0; i <= ((pblock_buf[0x7e] > 1) ? 1 : pblock_buf[0x7e]); i++) {
		if (!edid_checksum_result(pblock_buf + i * 128))
			TRACE1("Block %x edid checksum error\n", (uint) i);
		else
			TRACE1("Block %x edid checksum OK\n", (uint) i);
	}
}

unchar sp_tx_edid_read(unchar *pedid_blocks_buf)
{
	unchar offset = 0;
	unchar count, blocks_num;
	unchar pblock_buf[16];
	unchar i, j;
	unchar g_edid_break = 0;

	/*address initial*/
	WriteReg(RX_P0, AP_AUX_ADDR_7_0, 0x50);
	WriteReg(RX_P0, AP_AUX_ADDR_15_8, 0);
	sp_write_reg_and(RX_P0, AP_AUX_ADDR_19_16, 0xf0);

	blocks_num = sp_tx_get_edid_block();

	count = 0;

	do {
		switch (count) {
		case 0:
		case 1:

			for (i = 0; i < 8; i++) {
				offset = (i + count * 8) * 16;
				g_edid_break = edid_read(offset, pblock_buf);

				if (g_edid_break == 1)
					break;
				for (j = 0; j < 16; j++) {
					pedid_blocks_buf[offset + j] =
					    pblock_buf[j];
				}
			}

			break;

		case 2:
			offset = 0x00;

			for (j = 0; j < 8; j++) {
				if (g_edid_break == 1)
					break;

				segments_edid_read(count / 2, offset);
				offset = offset + 0x10;
			}

			break;
		case 3:
			offset = 0x80;

			for (j = 0; j < 8; j++) {
				if (g_edid_break == 1)
					break;

				segments_edid_read(count / 2, offset);
				offset = offset + 0x10;
			}

			break;

		default:
			break;
		}

		count++;

	} while ((blocks_num >= count) && (cable_connected == 1));

	/* check edid data */
	check_edid_data(pedid_blocks_buf);
	/*  test edid << */

/*reset aux channel*/
	sp_tx_rst_aux();

	/*  test edid  >> */
	return blocks_num;
}




void sp_tx_set_3d_format(u8 format3d)
{
	/*video changed flag*/
	sp_write_reg_or(RX_P0, AP_AV_STATUS, format3d & (_BIT1 | _BIT0));

}




#define XTAL_FRQ  27000000UL  /* MI-2 clock frequency in Hz: 27 MHz*/
#define XTAL_FRQ_MHz  27      /* MI-2 clock frequency in MHz: 27 MHz*/


/* calculate the Greatest Common Divisor of a and b */
static unsigned long GCD(unsigned long a, unsigned long b)
{
	unsigned long  iterations = 0L;

	if (a == 0) {
		/*printf("ite=%lu\n", iterations);  how many iterations are needed*/
		return  b;
	}

	while (b != 0) {
		if (a > b)
			a = a - b;
		else
			b = b - a;

		iterations++;
	}

	/*printf("ite=%lu\n", iterations);  how many iterations are needed*/
	return a;
}


#define  MAX_UNSIGNED_24BIT   16777215UL
/* reduction of fraction a/b */
void reduction_of_a_fraction(unsigned long *a, unsigned long *b)
{
	unsigned long  gcd;
	unsigned long  tmp_a, tmp_b;
	unsigned int   i = 1;

	gcd = GCD(*a, *b);
	*a /= gcd;
	*b /= gcd;

	tmp_a = *a;
	tmp_b = *b;

	while ((*a > MAX_UNSIGNED_24BIT) || (*b > MAX_UNSIGNED_24BIT)) {
		i++;
		*a = tmp_a / i;
		*b = tmp_b / i;
	}

	/* in the end, make a, b larger to have higher ODFC PLL output frequency accuracy */
	while ((*a < MAX_UNSIGNED_24BIT) && (*b < MAX_UNSIGNED_24BIT)) {
		*a <<= 1;
		*b <<= 1;
	}

	*a >>= 1;
	*b >>= 1;
}


/* calculate ODFC PLL settings for DSI */
/* given a pixel_frequency which belongs to [18750000, 297000000],
   decide integer values M, N, and post_divider to satisfy
     pixel_frequency = XTAL_FRQ * M / N / post_divider (if not exactly equal, try to minimize the error)
     post_divider belongs to [1, 16]
     M belongs to [1, 2^24-1]
     N belongs to [1, 2^24-1]
     XTAL_FRQ * M / N belongs to [300000000, 800000000]
 */
/* f_vco frequency:
   band 0:    1 ~ 1.15 GHz
   band 1: 1.15 ~ 1.3  GHz
   band 2:  1.3 ~ 1.5  GHz
   band 3:  1.5 ~ 2.0  GHz

   band 3 is usable but not recommended, as using band 3:
     a) The power consumption is higher.
     b) For SS corner chips, VCO may not work at 2GHz.

   let f_vco = 1~1.5GHz ==> f2 = f_vco = 1~1.5GHz
   ==> f1 = 0.5~0.75GHz ==> add some margin, let f1 = 0.52~0.73GHz
 */
#define  POST_DIVIDER_MIN    1
#define  POST_DIVIDER_MAX   16
#define  PLL_OUT_FREQ_MIN    520000000UL
#define  PLL_OUT_FREQ_MAX    730000000UL
#define  PLL_OUT_FREQ_ABS_MIN    300000000UL
#define  PLL_OUT_FREQ_ABS_MAX    800000000UL
static char  cal_ODFC_PLL_settings_DSI(unsigned long pixel_frequency,
	unsigned long *M, unsigned long *N, unsigned char *post_divider)
{
	if (pixel_frequency > PLL_OUT_FREQ_ABS_MAX / POST_DIVIDER_MIN) {
		TRACE("ERROR! cal_ODFC_PLL_settings_DSI: pixel clock frequency = %lu > %lu Hz\n",
		      pixel_frequency, PLL_OUT_FREQ_ABS_MAX / POST_DIVIDER_MIN);
		return  -AUX_ERR;  /* pixel clock frequency is too high*/
	}

	if (pixel_frequency < PLL_OUT_FREQ_ABS_MIN / POST_DIVIDER_MAX) {
		TRACE("ERROR! cal_ODFC_PLL_settings_DSI: pixel clock frequency = %lu < %lu Hz\n",
		      pixel_frequency, PLL_OUT_FREQ_ABS_MIN / POST_DIVIDER_MAX);
		return  -AUX_ERR;  /* pixel clock frequency is too low*/
	}

	*post_divider = 1;

	while (1) {
		if (pixel_frequency >= (PLL_OUT_FREQ_MIN / (*post_divider)))
			break;


		*post_divider += 1;
	}

	if (*post_divider > POST_DIVIDER_MAX) {
		*post_divider = 1;

		while (1) {
			if (pixel_frequency >= (PLL_OUT_FREQ_ABS_MIN / (*post_divider)))
				break;

			*post_divider += 1;
		}

		if (*post_divider > POST_DIVIDER_MAX)
			return  -AUX_ERR;  /* internal error A (theoretically speaking, should not happen)*/

	}

	/* patch to improve the accuracy */
	if (*post_divider == 7) {      /* 27,000,000 is not divisible by 7*/
		*post_divider = 8;
	} else if (*post_divider == 11) { /* 27,000,000 is not divisible by 11*/
		*post_divider = 12;
	} else if ((*post_divider == 13) || (*post_divider == 14)) { /*27,000,000 is not divisible by 13 or 14*/
		*post_divider = 15;
	}

	if (pixel_frequency * (*post_divider) > PLL_OUT_FREQ_ABS_MAX)
		return  -AUX_ERR;  /* internal error B*/


	/* Theoretical analysis of why "internal error B" should not happen: */
	/* 730MHz/520MHz = 1.404
	  The impact of "patch to improve the accuracy":
	  In the worst case, e.g.
	    pixel_frequency == PLL_OUT_FREQ_MIN / (6+¦Ä) or
	    pixel_frequency == PLL_OUT_FREQ_ABS_MIN / (6+¦Ä)
	  0<¦Ä<<1, "¦Ä<<1" means ¦Ä is much less than 1, say 1/1000.
	  After such "patch to improve the accuracy", (6+¦Ä)
	  will be corrected to 8.
	    8/(7-1) = 1.333
	    12/(11-1) = 1.2
	    15/(13-1) = 1.25
	  All of these ratios are lower than 1.404, not to mention 800MHz/520MHz = 1.538
	*/

	*M = pixel_frequency;
	*N = XTAL_FRQ / (*post_divider);
	reduction_of_a_fraction(M, N);

	return  AUX_OK;
}

void reconfig_current_pclk(unsigned char id, unsigned char direction, unsigned int delta_clk)
{
	unsigned long temp_m, temp_n;
	unsigned char temp_div;

	if (direction)
		mipi_video_timing_table[id].MIPI_pixel_frequency += (delta_clk);
	else
		mipi_video_timing_table[id].MIPI_pixel_frequency -= (delta_clk);

	if (cal_ODFC_PLL_settings_DSI(mipi_video_timing_table[id].MIPI_pixel_frequency, &temp_m,
		&temp_n, &temp_div) != AUX_OK) {
			TRACE("Calculate M N value failed!\n");
			return;
	}
	mipi_video_timing_table[id].M = temp_m;
	mipi_video_timing_table[id].N = temp_n;
	mipi_video_timing_table[id].post_divider = temp_div - 1;

	TRACE("Config New Pixel CLK=%ld\n", mipi_video_timing_table[id].MIPI_pixel_frequency);
	TRACE("new M=0x%lx\n", mipi_video_timing_table[id].M);
	TRACE("new N=0x%lx\n", mipi_video_timing_table[id].N);
	TRACE("new post_divider=%d\n", mipi_video_timing_table[id].post_divider);

}



#undef _SP_TX_DRV_C_

