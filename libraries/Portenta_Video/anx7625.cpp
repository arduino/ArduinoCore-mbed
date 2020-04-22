/*
 * This file is part of the coreboot project.
 *
 * Copyright 2019 Analogix Semiconductor.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

//#include <console/console.h>
//#include <delay.h>
//#include <device/i2c_simple.h>
#include <Arduino.h>
//#include <gpio.h>
//#include <timer.h>
#include <string.h>
#include <mbed/drivers/I2C.h>

#include "anx7625.h"
#include "video_modes.h"

#define ANXERROR(format, ...) \
		printk(BIOS_ERR, "ERROR: %s: " format, __func__, ##__VA_ARGS__)
#define ANXINFO(format, ...) \
		printk(BIOS_INFO, "%s: " format, __func__, ##__VA_ARGS__)
#define ANXDEBUG(format, ...) \
		printk(BIOS_DEBUG, "%s: " format, __func__, ##__VA_ARGS__)


mbed::I2C i2cx(I2C_SDA_INTERNAL , I2C_SCL_INTERNAL);

int i2c_writeb(uint8_t bus, uint8_t saddr, uint8_t offset, uint8_t val) {
	char cmd[2];
	cmd[0] = offset;
	cmd[1] = val;
	return i2cx.write(saddr, cmd, 2);
}

int i2c_read_bytes(uint8_t bus, uint8_t saddr, uint8_t offset, uint8_t* buf, size_t len) {

    i2cx.write(saddr, (char*)&offset, 1);
    return i2cx.read(saddr, (char*)buf, len);
}

int i2c_readb(uint8_t bus, uint8_t saddr, uint8_t offset, uint8_t* val) {

    i2cx.write(saddr, (char*)&offset, 1);
    return i2cx.read(saddr, (char*)val, 1);
}

/*
 * There is a sync issue while accessing I2C register between AP(CPU) and
 * internal firmware(OCM). To avoid the race condition, AP should access the
 * reserved slave address before slave address changes.
 */
static int i2c_access_workaround(uint8_t bus, uint8_t saddr)
{
	uint8_t offset;
	static uint8_t saddr_backup = 0;
	int ret = 0;

	if (saddr == saddr_backup)
		return ret;

	saddr_backup = saddr;

	switch (saddr) {
	case TCPC_INTERFACE_ADDR:
		offset = RSVD_00_ADDR;
		break;
	case TX_P0_ADDR:
		offset = RSVD_D1_ADDR;
		break;
	case TX_P1_ADDR:
		offset = RSVD_60_ADDR;
		break;
	case RX_P0_ADDR:
		offset = RSVD_39_ADDR;
		break;
	case RX_P1_ADDR:
		offset = RSVD_7F_ADDR;
		break;
	default:
		offset = RSVD_00_ADDR;
		break;
	}

	ret = i2c_writeb(bus, saddr, offset, 0x00);
	if (ret < 0)
		ANXERROR("Failed to access %#x:%#x\n", saddr, offset);
	return ret;
}

static int anx7625_reg_read(uint8_t bus, uint8_t saddr, uint8_t offset,
			    uint8_t *val)
{
	int ret;

	i2c_access_workaround(bus, saddr);
	ret = i2c_readb(bus, saddr, offset, val);
	if (ret < 0) {
		ANXERROR("Failed to read i2c reg=%#x:%#x\n", saddr, offset);
		return ret;
	}
	return *val;
}

static int anx7625_reg_block_read(uint8_t bus, uint8_t saddr, uint8_t reg_addr,
				  uint8_t len, uint8_t *buf)
{
	int ret;

	i2c_access_workaround(bus, saddr);
	ret = i2c_read_bytes(bus, saddr, reg_addr, buf, len);
	if (ret < 0)
		ANXERROR("Failed to read i2c block=%#x:%#x[len=%#x]\n", saddr,
			 reg_addr, len);
	return ret;
}

static int anx7625_reg_write(uint8_t bus, uint8_t saddr, uint8_t reg_addr,
			     uint8_t reg_val)
{
	int ret;

	i2c_access_workaround(bus, saddr);
	ret = i2c_writeb(bus, saddr, reg_addr, reg_val);
	if (ret < 0)
		ANXERROR("Failed to write i2c id=%#x:%#x\n", saddr, reg_addr);

	return ret;
}

static int anx7625_write_or(uint8_t bus, uint8_t saddr, uint8_t offset,
			    uint8_t mask)
{
	uint8_t val;
	int ret;

	ret = anx7625_reg_read(bus, saddr, offset, &val);
	if (ret < 0)
		return ret;

	return anx7625_reg_write(bus, saddr, offset, val | mask);
}

static int anx7625_write_and(uint8_t bus, uint8_t saddr, uint8_t offset,
			     uint8_t mask)
{
	int ret;
	uint8_t val;

	ret = anx7625_reg_read(bus, saddr, offset, &val);
	if (ret < 0)
		return ret;

	return anx7625_reg_write(bus, saddr, offset, val & mask);
}

static int wait_aux_op_finish(uint8_t bus)
{
	uint8_t val;
	int ret = -1;
	int loop;

	for (loop = 0; loop < 150; loop++) {
		mdelay(2);
		anx7625_reg_read(bus, RX_P0_ADDR, AP_AUX_CTRL_STATUS, &val);
		if (!(val & AP_AUX_CTRL_OP_EN)) {
			ret = 0;
			break;
		}
	}

	if (ret != 0) {
		ANXERROR("Timed out waiting aux operation.\n");
		return ret;
	}

	ret = anx7625_reg_read(bus, RX_P0_ADDR, AP_AUX_CTRL_STATUS, &val);
	if (ret < 0 || val & 0x0F) {
		ANXDEBUG("aux status %02x\n", val);
		ret = -1;
	}

	return ret;
}

static unsigned long gcd(unsigned long a, unsigned long b)
{
	if (a == 0)
		return b;

	while (b != 0) {
		if (a > b)
			a = a - b;
		else
			b = b - a;
	}

	return a;
}

/* Reduce fraction a/b */
static void anx7625_reduction_of_a_fraction(unsigned long *_a,
					    unsigned long *_b)
{
	unsigned long gcd_num;
	unsigned long a = *_a, b = *_b, old_a, old_b;
	u32 denom = 1;

	gcd_num = gcd(a, b);
	a /= gcd_num;
	b /= gcd_num;

	old_a = a;
	old_b = b;

	while (a > MAX_UNSIGNED_24BIT || b > MAX_UNSIGNED_24BIT) {
		denom++;
		a = old_a / denom;
		b = old_b / denom;
	}

	/* Increase a, b to have higher ODFC PLL output frequency accuracy. */
	while ((a << 1) < MAX_UNSIGNED_24BIT && (b << 1) < MAX_UNSIGNED_24BIT) {
		a <<= 1;
		b <<= 1;
	}

	*_a = a;
	*_b = b;
}

static int anx7625_calculate_m_n(u32 pixelclock,
				 unsigned long *m, unsigned long *n,
				 uint8_t *pd)
{
	uint8_t post_divider = *pd;
	if (pixelclock > PLL_OUT_FREQ_ABS_MAX / POST_DIVIDER_MIN) {
		/* pixel clock frequency is too high */
		ANXERROR("pixelclock %u higher than %lu, "
			 "output may be unstable\n",
			 pixelclock, PLL_OUT_FREQ_ABS_MAX / POST_DIVIDER_MIN);
		return 1;
	}

	if (pixelclock < PLL_OUT_FREQ_ABS_MIN / POST_DIVIDER_MAX) {
		/* pixel clock frequency is too low */
		ANXERROR("pixelclock %u lower than %lu, "
			 "output may be unstable\n",
			 pixelclock, PLL_OUT_FREQ_ABS_MIN / POST_DIVIDER_MAX);
		return 1;
	}

	post_divider = 1;

	for (post_divider = 1;
	     pixelclock < PLL_OUT_FREQ_MIN / post_divider;
	     post_divider++)
		;

	if (post_divider > POST_DIVIDER_MAX) {
		for (post_divider = 1;
		     pixelclock < PLL_OUT_FREQ_ABS_MIN / post_divider;
		     post_divider++)
			;

		if (post_divider > POST_DIVIDER_MAX) {
			ANXERROR("cannot find property post_divider(%d)\n",
				 post_divider);
			return 1;
		}
	}

	/* Patch to improve the accuracy */
	if (post_divider == 7) {
		/* 27,000,000 is not divisible by 7 */
		post_divider = 8;
	} else if (post_divider == 11) {
		/* 27,000,000 is not divisible by 11 */
		post_divider = 12;
	} else if (post_divider == 13 || post_divider == 14) {
		/*27,000,000 is not divisible by 13 or 14*/
		post_divider = 15;
	}

	if (pixelclock * post_divider > PLL_OUT_FREQ_ABS_MAX) {
		ANXINFO("act clock(%u) large than maximum(%lu)\n",
			pixelclock * post_divider, PLL_OUT_FREQ_ABS_MAX);
		return 1;
	}

	*m = (unsigned long long)pixelclock; // * 599 / 600;
	*n = XTAL_FRQ / post_divider;
	*pd = post_divider;

	anx7625_reduction_of_a_fraction(m, n);

	return 0;
}

static int anx7625_odfc_config(uint8_t bus, uint8_t post_divider)
{
	int ret;

	/* config input reference clock frequency 27MHz/19.2MHz */
	ret = anx7625_write_and(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_16,
			~(REF_CLK_27000kHz << MIPI_FREF_D_IND));
	ret |= anx7625_write_or(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_16,
			(REF_CLK_27000kHz << MIPI_FREF_D_IND));
	/* post divider */
	ret |= anx7625_write_and(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_8, 0x0f);
	ret |= anx7625_write_or(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_8,
			post_divider << 4);

	/* add patch for MIS2-125 (5pcs ANX7625 fail ATE MBIST test) */
	ret |= anx7625_write_and(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_7,
			~MIPI_PLL_VCO_TUNE_REG_VAL);

	/* reset ODFC PLL */
	ret |= anx7625_write_and(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_7,
			~MIPI_PLL_RESET_N);
	ret |= anx7625_write_or(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_7,
			MIPI_PLL_RESET_N);

	if (ret < 0)
		ANXERROR("IO error.\n");

	return ret;
}

static int anx7625_dsi_video_config(uint8_t bus, struct display_timing *dt)
{
	unsigned long m, n;
	u16 htotal;
	int ret;
	uint8_t post_divider = 0;

	ret = anx7625_calculate_m_n(dt->pixelclock * 1000, &m, &n,
				    &post_divider);

	if (ret != 0) {
		ANXERROR("cannot get property m n value.\n");
		return -1;
	}

	ANXINFO("compute M(%lu), N(%lu), divider(%d).\n", m, n, post_divider);

	/* configure pixel clock */
	ret = anx7625_reg_write(bus, RX_P0_ADDR, PIXEL_CLOCK_L,
		(dt->pixelclock / 1000) & 0xFF);
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, PIXEL_CLOCK_H,
		(dt->pixelclock / 1000) >> 8);
	/* lane count */
	ret |= anx7625_write_and(bus, RX_P1_ADDR, MIPI_LANE_CTRL_0, 0xfc);

	ret |= anx7625_write_or(bus, RX_P1_ADDR, MIPI_LANE_CTRL_0, 1);

	/* Htotal */
	htotal = dt->hactive + dt->hfront_porch +
		 dt->hback_porch + dt->hsync_len;
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_TOTAL_PIXELS_L, htotal & 0xFF);
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_TOTAL_PIXELS_H, htotal >> 8);
	/* Hactive */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_ACTIVE_PIXELS_L, dt->hactive & 0xFF);
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_ACTIVE_PIXELS_H, dt->hactive >> 8);
	/* HFP */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_FRONT_PORCH_L, dt->hfront_porch);
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_FRONT_PORCH_H,
				 dt->hfront_porch >> 8);
	/* HWS */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_SYNC_WIDTH_L, dt->hsync_len);
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_SYNC_WIDTH_H, dt->hsync_len >> 8);
	/* HBP */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_BACK_PORCH_L, dt->hback_porch);
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 HORIZONTAL_BACK_PORCH_H, dt->hback_porch >> 8);
	/* Vactive */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR, ACTIVE_LINES_L, dt->vactive);
	ret |= anx7625_reg_write(bus, RX_P2_ADDR, ACTIVE_LINES_H,
				 dt->vactive >> 8);
	/* VFP */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 VERTICAL_FRONT_PORCH, dt->vfront_porch);
	/* VWS */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 VERTICAL_SYNC_WIDTH, dt->vsync_len);
	/* VBP */
	ret |= anx7625_reg_write(bus, RX_P2_ADDR,
				 VERTICAL_BACK_PORCH, dt->vback_porch);
	/* M value */
	ret |= anx7625_reg_write(bus, RX_P1_ADDR,
				 MIPI_PLL_M_NUM_23_16, (m >> 16) & 0xff);
	ret |= anx7625_reg_write(bus, RX_P1_ADDR,
				 MIPI_PLL_M_NUM_15_8, (m >> 8) & 0xff);
	ret |= anx7625_reg_write(bus, RX_P1_ADDR,
				 MIPI_PLL_M_NUM_7_0, (m & 0xff));
	/* N value */
	ret |= anx7625_reg_write(bus, RX_P1_ADDR,
				 MIPI_PLL_N_NUM_23_16, (n >> 16) & 0xff);
	ret |= anx7625_reg_write(bus, RX_P1_ADDR,
				 MIPI_PLL_N_NUM_15_8, (n >> 8) & 0xff);
	ret |= anx7625_reg_write(bus, RX_P1_ADDR, MIPI_PLL_N_NUM_7_0,
				 (n & 0xff));
	/* diff */
	ret |= anx7625_reg_write(bus, RX_P1_ADDR, MIPI_DIGITAL_ADJ_1, 0x37);

	ret |= anx7625_odfc_config(bus, post_divider - 1);

	if (ret < 0)
		ANXERROR("mipi dsi setup IO error.\n");

	return ret;
}

static int anx7625_swap_dsi_lane3(uint8_t bus)
{
	int ret;
	uint8_t val;

	/* swap MIPI-DSI data lane 3 P and N */
	ret = anx7625_reg_read(bus, RX_P1_ADDR, MIPI_SWAP, &val);
	if (ret < 0) {
		ANXERROR("IO error: access MIPI_SWAP.\n");
		return -1;
	}

	val |= (1 << MIPI_SWAP_CH3);
	return anx7625_reg_write(bus, RX_P1_ADDR, MIPI_SWAP, val);
}

static int anx7625_api_dsi_config(uint8_t bus, struct display_timing *dt)

{
	int val, ret;

	/* swap MIPI-DSI data lane 3 P and N */
	ret = anx7625_swap_dsi_lane3(bus);
	if (ret < 0) {
		ANXERROR("IO error: swap dsi lane 3 failed.\n");
		return ret;
	}

	/* DSI clock settings */
	val = (0 << MIPI_HS_PWD_CLK)		|
		(0 << MIPI_HS_RT_CLK)		|
		(0 << MIPI_PD_CLK)		|
		(1 << MIPI_CLK_RT_MANUAL_PD_EN)	|
		(1 << MIPI_CLK_HS_MANUAL_PD_EN)	|
		(0 << MIPI_CLK_DET_DET_BYPASS)	|
		(0 << MIPI_CLK_MISS_CTRL)	|
		(0 << MIPI_PD_LPTX_CH_MANUAL_PD_EN);
	ret = anx7625_reg_write(bus, RX_P1_ADDR, MIPI_PHY_CONTROL_3, val);

	/*
	 * Decreased HS prepare tg delay from 160ns to 80ns work with
	 *     a) Dragon board 810 series (Qualcomm AP)
	 *     b) Moving Pixel DSI source (PG3A pattern generator +
	 *        P332 D-PHY Probe) default D-PHY tg 5ns/step
	 */
	//ret |= anx7625_reg_write(bus, RX_P1_ADDR, MIPI_TIME_HS_PRPR, 0x10);

	/* enable DSI mode */
	ret |= anx7625_write_or(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_18,
				SELECT_DSI << MIPI_DPI_SELECT);

	ret |= anx7625_dsi_video_config(bus, dt);
	if (ret < 0) {
		ANXERROR("dsi video tg config failed\n");
		return ret;
	}

	/* toggle m, n ready */
	ret = anx7625_write_and(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_6,
				~(MIPI_M_NUM_READY | MIPI_N_NUM_READY));
	mdelay(1);
	ret |= anx7625_write_or(bus, RX_P1_ADDR, MIPI_DIGITAL_PLL_6,
				MIPI_M_NUM_READY | MIPI_N_NUM_READY);

	/* configure integer stable register */
	ret |= anx7625_reg_write(bus, RX_P1_ADDR, MIPI_VIDEO_STABLE_CNT, 0x02);
	/* power on MIPI RX */
	ret |= anx7625_reg_write(bus, RX_P1_ADDR, MIPI_LANE_CTRL_10, 0x00);
	ret |= anx7625_reg_write(bus, RX_P1_ADDR, MIPI_LANE_CTRL_10, 0x80);

	if (ret < 0)
		ANXERROR("IO error: mipi dsi enable init failed.\n");

	return ret;
}

static int anx7625_dsi_config(uint8_t bus, struct display_timing *dt)
{
	int ret;

	ANXINFO("config dsi.\n");

	/* DSC disable */
	ret = anx7625_write_and(bus, RX_P0_ADDR, R_DSC_CTRL_0, ~DSC_EN);
	ret |= anx7625_api_dsi_config(bus, dt);

	if (ret < 0) {
		ANXERROR("IO error: api dsi config error.\n");
		return ret;
	}

	/* set MIPI RX EN */
	ret = anx7625_write_or(bus, RX_P0_ADDR, AP_AV_STATUS, AP_MIPI_RX_EN);
	/* clear mute flag */
	ret |= anx7625_write_and(bus, RX_P0_ADDR, AP_AV_STATUS, ~AP_MIPI_MUTE);

	if (ret < 0)
		ANXERROR("IO error: enable mipi rx failed.\n");
	else
		ANXINFO("success to config DSI\n");

	return ret;
}

static int sp_tx_rst_aux(uint8_t bus)
{
	int ret;

	ret = anx7625_write_or(bus, TX_P2_ADDR, RST_CTRL2, AUX_RST);
	ret |= anx7625_write_and(bus, TX_P2_ADDR, RST_CTRL2, ~AUX_RST);
	return ret;
}

static int sp_tx_aux_wr(uint8_t bus, uint8_t offset)
{
	int ret;

	ret = anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_BUFF_START, offset);
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_COMMAND, 0x04);
	ret |= anx7625_write_or(bus, RX_P0_ADDR,
				AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	return ret | wait_aux_op_finish(bus);
}

static int sp_tx_aux_rd(uint8_t bus, uint8_t len_cmd)
{
	int ret;

	ret = anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_COMMAND, len_cmd);
	ret |= anx7625_write_or(bus, RX_P0_ADDR,
				AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	return ret | wait_aux_op_finish(bus);
}

static int sp_tx_get_edid_block(uint8_t bus)
{
	int ret;
	uint8_t val = 0;

	sp_tx_aux_wr(bus, 0x7e);
	sp_tx_aux_rd(bus, 0x01);
	ret = anx7625_reg_read(bus, RX_P0_ADDR, AP_AUX_BUFF_START, &val);

	if (ret < 0) {
		ANXERROR("IO error: access AUX BUFF.\n");
		return -1;
	}

	ANXINFO("EDID Block = %d\n", val + 1);

	if (val > 3)
		val = 1;

	return val;
}

static int edid_read(uint8_t bus, uint8_t offset, uint8_t *pblock_buf)
{
	uint8_t c, cnt = 0;

	c = 0;
	for (cnt = 0; cnt < 3; cnt++) {
		sp_tx_aux_wr(bus, offset);
		/* set I2C read com 0x01 mot = 0 and read 16 bytes */
		c = sp_tx_aux_rd(bus, 0xf1);

		if (c == 1) {
			sp_tx_rst_aux(bus);
			ANXERROR("edid read failed, reset!\n");
			cnt++;
		} else {
			anx7625_reg_block_read(bus, RX_P0_ADDR,
					AP_AUX_BUFF_START,
					MAX_DPCD_BUFFER_SIZE, pblock_buf);
			return 0;
		}
	}

	return 1;
}

static int segments_edid_read(uint8_t bus, uint8_t segment, uint8_t *buf,
			      uint8_t offset)
{
	uint8_t c, cnt = 0;
	int ret;

	/* write address only */
	ret = anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_ADDR_7_0, 0x30);
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_COMMAND, 0x04);
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_CTRL_STATUS,
				 AP_AUX_CTRL_ADDRONLY | AP_AUX_CTRL_OP_EN);

	ret |= wait_aux_op_finish(bus);
	/* write segment address */
	ret |= sp_tx_aux_wr(bus, segment);
	/* data read */
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_ADDR_7_0, 0x50);

	if (ret < 0) {
		ANXERROR("IO error: aux initial failed.\n");
		return ret;
	}

	for (cnt = 0; cnt < 3; cnt++) {
		sp_tx_aux_wr(bus, offset);
		/* set I2C read com 0x01 mot = 0 and read 16 bytes */
		c = sp_tx_aux_rd(bus, 0xf1);

		if (c == 1) {
			ret = sp_tx_rst_aux(bus);
			ANXERROR("segment read failed, reset!\n");
			cnt++;
		} else {
			ret = anx7625_reg_block_read(bus, RX_P0_ADDR,
					AP_AUX_BUFF_START,
					MAX_DPCD_BUFFER_SIZE, buf);
			return ret;
		}
	}

	return ret;
}

static int sp_tx_edid_read(uint8_t bus, uint8_t *pedid_blocks_buf,
			   uint32_t size)
{
	uint8_t offset, edid_pos;
	int count, blocks_num;
	uint8_t pblock_buf[MAX_DPCD_BUFFER_SIZE];
	uint8_t i;
	uint8_t g_edid_break = 0;
	int ret;

	/* address initial */
	ret = anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_ADDR_7_0, 0x50);
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, AP_AUX_ADDR_15_8, 0);
	ret |= anx7625_write_and(bus, RX_P0_ADDR, AP_AUX_ADDR_19_16, 0xf0);

	if (ret < 0) {
		ANXERROR("access aux channel IO error.\n");
		return -1;
	}

	blocks_num = sp_tx_get_edid_block(bus);
	if (blocks_num < 0)
		return blocks_num;

	count = 0;

	do {
		switch (count) {
		case 0:
		case 1:
			for (i = 0; i < 8; i++) {
				offset = (i + count * 8) * MAX_DPCD_BUFFER_SIZE;
				g_edid_break = edid_read(bus, offset,
						pblock_buf);

				if (g_edid_break == 1)
					break;

				if (offset <= size - MAX_DPCD_BUFFER_SIZE)
					memcpy(&pedid_blocks_buf[offset],
					       pblock_buf,
					       MAX_DPCD_BUFFER_SIZE);
			}

			break;
		case 2:
		case 3:
			offset = (count == 2) ? 0x00 : 0x80;

			for (i = 0; i < 8; i++) {
				edid_pos = (i + count * 8) *
					MAX_DPCD_BUFFER_SIZE;

				if (g_edid_break == 1)
					break;

				segments_edid_read(bus, count / 2,
						pblock_buf, offset);
				if (edid_pos <= size - MAX_DPCD_BUFFER_SIZE)
					memcpy(&pedid_blocks_buf[edid_pos],
					       pblock_buf,
					       MAX_DPCD_BUFFER_SIZE);
				offset = offset + 0x10;
			}

			break;
		default:
			die("%s: count should be <= 3", __func__);
			break;
		}

		count++;

	} while (blocks_num >= count);

	/* reset aux channel */
	sp_tx_rst_aux(bus);

	return blocks_num;
}

static void anx7625_disable_pd_protocol(uint8_t bus)
{
	int ret;

	/* reset main ocm */
	ret = anx7625_reg_write(bus, RX_P0_ADDR, 0x88, 0x40);
	/* Disable PD */
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, AP_AV_STATUS, AP_DISABLE_PD);
	/* release main ocm */
	ret |= anx7625_reg_write(bus, RX_P0_ADDR, 0x88, 0x00);

	if (ret < 0)
		ANXERROR("Failed to disable PD feature.\n");
	else
		ANXINFO("Disabled PD feature.\n");
}

#define FLASH_LOAD_STA 0x05
#define FLASH_LOAD_STA_CHK	(1 << 7)

static int anx7625_power_on_init(uint8_t bus)
{
	int i, ret;
	uint8_t val, version, revision;

	anx7625_reg_write(bus, RX_P0_ADDR, XTAL_FRQ_SEL, XTAL_FRQ_27M);

	for (i = 0; i < OCM_LOADING_TIME; i++) {
		/* check interface */
		ret = anx7625_reg_read(bus, RX_P0_ADDR, FLASH_LOAD_STA, &val);
		if (ret < 0) {
			ANXERROR("Failed to load flash\n");
			return ret;
		}

		if ((val & FLASH_LOAD_STA_CHK) != FLASH_LOAD_STA_CHK) {
			mdelay(1);
			continue;
		}
		ANXINFO("Init interface.\n");

		//anx7625_disable_pd_protocol(bus);
		anx7625_reg_read(bus, RX_P0_ADDR, OCM_FW_VERSION, &version);
		anx7625_reg_read(bus, RX_P0_ADDR, OCM_FW_REVERSION, &revision);

		if (version == 0 && revision == 0) {
			continue;
		}

		ANXINFO("Firmware: ver %#02x, rev %#02x.\n", version, revision);
		return 0;
	}
	return -1;
}

static void anx7625_start_dp_work(uint8_t bus)
{
	int ret;
	uint8_t val;

	/* not support HDCP */
	ret = anx7625_write_and(bus, RX_P1_ADDR, 0xee, 0x9f);

	/* try auth flag */
	ret |= anx7625_write_or(bus, RX_P1_ADDR, 0xec, 0x10);
	/* interrupt for DRM */
	ret |= anx7625_write_or(bus, RX_P1_ADDR, 0xff, 0x01);
	if (ret < 0)
		return;

	ret = anx7625_reg_read(bus, RX_P1_ADDR, 0x86, &val);
	if (ret < 0)
		return;

	ANXINFO("Secure OCM version=%02x\n", val);
}

static int anx7625_hpd_change_detect(uint8_t bus)
{
	int ret;
	uint8_t status;

	ret = anx7625_reg_read(bus, RX_P0_ADDR, SYSTEM_STSTUS, &status);
	if (ret < 0) {
		ANXERROR("IO error: Failed to clear interrupt status.\n");
		return ret;
	}

	if (status & HPD_STATUS) {
		anx7625_start_dp_work(bus);
		ANXINFO("HPD received 0x7e:0x45=%#x\n", status);
		return 1;
	}
	return 0;
}

static void anx7625_parse_edid(const struct edid *edid,
			       struct display_timing *dt)
{
	dt->pixelclock = edid->mode.pixel_clock;

	dt->hactive = edid->mode.ha;
	dt->hsync_len = edid->mode.hspw;
	dt->hback_porch = (edid->mode.hbl - edid->mode.hso -
			   edid->mode.hborder - edid->mode.hspw);
	dt->hfront_porch = edid->mode.hso - edid->mode.hborder;

	dt->vactive = edid->mode.va;
	dt->vsync_len = edid->mode.vspw;
	dt->vfront_porch = edid->mode.vso - edid->mode.vborder;
	dt->vback_porch = (edid->mode.vbl - edid->mode.vso -
			   edid->mode.vspw - edid->mode.vborder);

	ANXINFO("pixelclock(%d).\n"
		" hactive(%d), hsync(%d), hfp(%d), hbp(%d)\n"
		" vactive(%d), vsync(%d), vfp(%d), vbp(%d)\n",
		dt->pixelclock,
		dt->hactive, dt->hsync_len, dt->hfront_porch, dt->hback_porch,
		dt->vactive, dt->vsync_len, dt->vfront_porch, dt->vback_porch);
}

extern struct envie_edid_mode envie_known_modes[];

int anx7625_dp_start(uint8_t bus, const struct edid *edid, enum edid_modes mode)
{
	int ret;
	struct display_timing dt;

	anx7625_parse_edid(edid, &dt);

	if (mode != EDID_MODE_AUTO) {

		dt.pixelclock = envie_known_modes[mode].pixel_clock;

		dt.hactive = envie_known_modes[mode].hactive;
		dt.hsync_len = envie_known_modes[mode].hsync_len;
		dt.hback_porch = envie_known_modes[mode].hback_porch;
		dt.hfront_porch = envie_known_modes[mode].hfront_porch;

		dt.vactive = envie_known_modes[mode].vactive;
		dt.vsync_len = envie_known_modes[mode].vsync_len;;
		dt.vback_porch = envie_known_modes[mode].vback_porch;
		dt.vfront_porch = envie_known_modes[mode].vfront_porch;
		dt.hpol = envie_known_modes[mode].hpol;
		dt.vpol = envie_known_modes[mode].vpol;
	}

	stm32_dsi_config(bus, (struct edid *)edid, &dt);

	ret = anx7625_dsi_config(bus, &dt);
	if (ret < 0)
		ANXERROR("MIPI phy setup error.\n");
	else
		ANXINFO("MIPI phy setup OK.\n");

	return ret;
}

#include "stm32h7xx_hal.h"

static DMA2D_HandleTypeDef dma2d;
static LTDC_HandleTypeDef  ltdc;
static DSI_HandleTypeDef   dsi;

static uint32_t lcd_x_size = 1280;
static uint32_t lcd_y_size = 1024;

static void stm32_LayerInit(uint16_t LayerIndex, uint32_t FB_Address)
{
	LTDC_LayerCfgTypeDef  Layercfg;

	/* Layer Init */
	Layercfg.WindowX0 = 0;
	Layercfg.WindowX1 = lcd_x_size;
	Layercfg.WindowY0 = 0;
	Layercfg.WindowY1 = lcd_y_size;
	Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565; //LTDC_PIXEL_FORMAT_ARGB8888;
	Layercfg.FBStartAdress = FB_Address;
	Layercfg.Alpha = 255;
	Layercfg.Alpha0 = 0;
	Layercfg.Backcolor.Blue = 0;
	Layercfg.Backcolor.Green = 0;
	Layercfg.Backcolor.Red = 0;
	Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	Layercfg.ImageWidth = lcd_x_size;
	Layercfg.ImageHeight = lcd_y_size;

	HAL_LTDC_ConfigLayer(&ltdc, &Layercfg, LayerIndex);
}

#define BYTES_PER_PIXEL		2
#define FB_BASE_ADDRESS 	((uint32_t)0xC0000000)
#define FB_ADDRESS_0 		(FB_BASE_ADDRESS)
#define FB_ADDRESS_1 		(FB_BASE_ADDRESS + (lcd_x_size * lcd_y_size * BYTES_PER_PIXEL))

uint32_t getFramebufferEnd() {
	return (FB_BASE_ADDRESS + 2 * (lcd_x_size * lcd_y_size * BYTES_PER_PIXEL));
}

extern "C" {
static uint32_t pend_buffer = 0;

/*
void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *ltdc)
{
	LTDC_LAYER(ltdc, 0)->CFBAR = (pend_buffer++ % 2) ? FB_ADDRESS_0 : FB_ADDRESS_1;
	__HAL_LTDC_RELOAD_CONFIG(ltdc); 

	HAL_LTDC_ProgramLineEvent(ltdc, 0);
}

void LTDC_IRQHandler(void)
{
	HAL_LTDC_IRQHandler(&ltdc);
}
*/

}

uint32_t getNextFrameBuffer() {

	int fb = pend_buffer++ % 2;

	__HAL_LTDC_LAYER_ENABLE(&(ltdc), fb);
  	__HAL_LTDC_LAYER_DISABLE(&(ltdc), !fb);
  	__HAL_LTDC_VERTICAL_BLANKING_RELOAD_CONFIG(&(ltdc));

	return fb ? FB_ADDRESS_0 : FB_ADDRESS_1;
}

uint32_t stm32_getXSize() {
	return lcd_x_size;
}

uint32_t stm32_getYSize() {
	return lcd_y_size;
}

int stm32_dsi_config(uint8_t bus, struct edid *edid, struct display_timing *dt) {

	static const uint32_t DSI_PLLNDIV = 40;
	static const uint32_t DSI_PLLIDF = DSI_PLL_IN_DIV2;
	static const uint32_t DSI_PLLODF = DSI_PLL_OUT_DIV1;
	static const uint32_t DSI_TXEXCAPECLOCKDIV = 4;

	static uint32_t LTDC_FREQ_STEP = 100;
	// set PLL3 to start from a 1MHz reference and increment by 100 or 200 KHz based on the frequency range

	if (dt->pixelclock/LTDC_FREQ_STEP > 512) LTDC_FREQ_STEP = 200;

	static const uint32_t LTDC_PLL3M = HSE_VALUE/1000000;
	static const uint32_t LTDC_PLL3N = dt->pixelclock/LTDC_FREQ_STEP;
	static const uint32_t LTDC_PLL3P = 2;
	static const uint32_t LTDC_PLL3Q = 7;
	static const uint32_t LTDC_PLL3R = 1000 / LTDC_FREQ_STEP; // expected pixel clock
	dt->pixelclock = (LTDC_PLL3N) *LTDC_FREQ_STEP; 	// real pixel clock

	static const uint32_t LANE_BYTE_CLOCK =	62500;


	// TODO: switch USB to use HSI48

/*
	static const uint32_t LTDC_PLL3M = 4;
	static const uint32_t LTDC_PLL3N = 100;
	static const uint32_t LTDC_PLL3P = 2;
	static const uint32_t LTDC_PLL3Q = 14;
	static const uint32_t LTDC_PLL3R = 25;
*/

	lcd_x_size = dt->hactive;
	lcd_y_size = dt->vactive;

	DSI_PLLInitTypeDef dsiPllInit;
	DSI_PHY_TimerTypeDef dsiPhyInit;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
	DSI_VidCfgTypeDef hdsivideo_handle;

	/** @brief Enable the LTDC clock */
	__HAL_RCC_LTDC_CLK_ENABLE();

	/** @brief Toggle Sw reset of LTDC IP */
	__HAL_RCC_LTDC_FORCE_RESET();
	__HAL_RCC_LTDC_RELEASE_RESET();

	/** @brief Enable the DMA2D clock */
	__HAL_RCC_DMA2D_CLK_ENABLE();

	/** @brief Toggle Sw reset of DMA2D IP */
	__HAL_RCC_DMA2D_FORCE_RESET();
	__HAL_RCC_DMA2D_RELEASE_RESET();

	/** @brief Enable DSI Host and wrapper clocks */
	__HAL_RCC_DSI_CLK_ENABLE();

	/** @brief Soft Reset the DSI Host and wrapper */
	__HAL_RCC_DSI_FORCE_RESET();
	__HAL_RCC_DSI_RELEASE_RESET();

	/** @brief NVIC configuration for LTDC interrupt that is now enabled */
	HAL_NVIC_SetPriority(LTDC_IRQn, 0x0F, 0);
	HAL_NVIC_EnableIRQ(LTDC_IRQn);

	/** @brief NVIC configuration for DMA2D interrupt that is now enabled */
	HAL_NVIC_SetPriority(DMA2D_IRQn, 0x0F, 0);
	HAL_NVIC_EnableIRQ(DMA2D_IRQn);

	/** @brief NVIC configuration for DSI interrupt that is now enabled */
	HAL_NVIC_SetPriority(DSI_IRQn, 0x0F, 0);
	HAL_NVIC_EnableIRQ(DSI_IRQn);

	/*************************DSI Initialization***********************************/

	/* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
	dsi.Instance = DSI;

	HAL_DSI_DeInit(&(dsi));

	/* Configure the DSI PLL */
	dsiPllInit.PLLNDIV    = DSI_PLLNDIV;
	dsiPllInit.PLLIDF     = DSI_PLLIDF;
	dsiPllInit.PLLODF     = DSI_PLLODF;

	/* Set number of Lanes */
	dsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
	/* Set the TX escape clock division ratio */
	dsi.Init.TXEscapeCkdiv = DSI_TXEXCAPECLOCKDIV;
	/* Disable the automatic clock lane control (the ANX7265 must be clocked) */
	dsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;

	/* Init the DSI */
	HAL_DSI_Init(&dsi, &dsiPllInit);

#if 0
	/* Configure the D-PHY Timings */
	dsiPhyInit.ClockLaneHS2LPTime = 0x14;
	dsiPhyInit.ClockLaneLP2HSTime = 0x14;
	dsiPhyInit.DataLaneHS2LPTime = 0x0A;
	dsiPhyInit.DataLaneLP2HSTime = 0x0A;
	dsiPhyInit.DataLaneMaxReadTime = 0x00;
	dsiPhyInit.StopWaitTime = 0x0;
	HAL_DSI_ConfigPhyTimer(&dsi, &dsiPhyInit);
#endif

	hdsivideo_handle.VirtualChannelID     = 0;

	/* Timing parameters for Video modes
	 Set Timing parameters of DSI depending on its chosen format */
	hdsivideo_handle.ColorCoding          = DSI_RGB565;					// may choose DSI_RGB888
	hdsivideo_handle.LooselyPacked        = DSI_LOOSELY_PACKED_DISABLE;
	hdsivideo_handle.VSPolarity           = DSI_VSYNC_ACTIVE_LOW;
	hdsivideo_handle.HSPolarity           = DSI_HSYNC_ACTIVE_LOW;
	hdsivideo_handle.DEPolarity           = DSI_DATA_ENABLE_ACTIVE_HIGH;
	hdsivideo_handle.Mode                 = DSI_VID_MODE_BURST;
	hdsivideo_handle.NullPacketSize       = 0xFFF;
	hdsivideo_handle.NumberOfChunks       = 1;
	hdsivideo_handle.PacketSize           = lcd_x_size;
	hdsivideo_handle.HorizontalSyncActive = dt->hsync_len*LANE_BYTE_CLOCK/dt->pixelclock;
	hdsivideo_handle.HorizontalBackPorch  = dt->hback_porch*LANE_BYTE_CLOCK/dt->pixelclock;
	hdsivideo_handle.HorizontalLine       = (dt->hactive + dt->hsync_len + dt->hback_porch + dt->hfront_porch)*LANE_BYTE_CLOCK/dt->pixelclock;
	hdsivideo_handle.VerticalSyncActive   = dt->vsync_len;
	hdsivideo_handle.VerticalBackPorch    = dt->vback_porch;
	hdsivideo_handle.VerticalFrontPorch   = dt->vfront_porch;
	hdsivideo_handle.VerticalActive       = dt->vactive;

	/* Enable or disable sending LP command while streaming is active in video mode */
	hdsivideo_handle.LPCommandEnable      = DSI_LP_COMMAND_ENABLE; /* Enable sending commands in mode LP (Low Power) */

	/* Largest packet size possible to transmit in LP mode in VSA, VBP, VFP regions */
	/* Only useful when sending LP packets is allowed while streaming is active in video mode */
	hdsivideo_handle.LPLargestPacketSize          = 16;

	/* Largest packet size possible to transmit in LP mode in HFP region during VACT period */
	/* Only useful when sending LP packets is allowed while streaming is active in video mode */
	hdsivideo_handle.LPVACTLargestPacketSize      = 0;

	/* Specify for each region, if the going in LP mode is allowed */
	/* while streaming is active in video mode                     */
	hdsivideo_handle.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;   /* Allow sending LP commands during HFP period */
	hdsivideo_handle.LPHorizontalBackPorchEnable  = DSI_LP_HBP_ENABLE;   /* Allow sending LP commands during HBP period */
	hdsivideo_handle.LPVerticalActiveEnable = DSI_LP_VACT_ENABLE;  /* Allow sending LP commands during VACT period */
	hdsivideo_handle.LPVerticalFrontPorchEnable = DSI_LP_VFP_ENABLE;   /* Allow sending LP commands during VFP period */
	hdsivideo_handle.LPVerticalBackPorchEnable = DSI_LP_VBP_ENABLE;   /* Allow sending LP commands during VBP period */
	hdsivideo_handle.LPVerticalSyncActiveEnable = DSI_LP_VSYNC_ENABLE; /* Allow sending LP commands during VSync = VSA period */

	/* Configure DSI Video mode timings with settings set above */
	HAL_DSI_ConfigVideoMode(&dsi, &hdsivideo_handle);

	/* Configure DSI PHY HS2LP and LP2HS timings */
	dsiPhyInit.ClockLaneHS2LPTime = 35;
	dsiPhyInit.ClockLaneLP2HSTime = 35;
	dsiPhyInit.DataLaneHS2LPTime = 35;
	dsiPhyInit.DataLaneLP2HSTime = 35;
	dsiPhyInit.DataLaneMaxReadTime = 0;
	dsiPhyInit.StopWaitTime = 10;
	HAL_DSI_ConfigPhyTimer(&dsi, &dsiPhyInit);

	/*************************End DSI Initialization*******************************/


	/************************LTDC Initialization***********************************/

	/* LCD clock configuration */
	/* LCD clock configuration */
	/* PLL3_VCO Input = HSE_VALUE/PLL3M = 1 Mhz */
	/* PLL3_VCO Output = PLL3_VCO Input * PLL3N */
	/* PLLLCDCLK = PLL3_VCO Output/PLL3R */
	/* LTDC clock frequency = PLLLCDCLK */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLL3.PLL3M = LTDC_PLL3M;
	PeriphClkInitStruct.PLL3.PLL3N = LTDC_PLL3N;
	PeriphClkInitStruct.PLL3.PLL3P = LTDC_PLL3P;
	PeriphClkInitStruct.PLL3.PLL3Q = LTDC_PLL3Q;
	PeriphClkInitStruct.PLL3.PLL3R = LTDC_PLL3R;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	/* Base address of LTDC registers to be set before calling De-Init */
	ltdc.Instance = LTDC;

	HAL_LTDC_DeInit(&(ltdc));

	/* Timing Configuration */
	ltdc.Init.HorizontalSync = (dt->hsync_len -1);
	ltdc.Init.AccumulatedHBP = (dt->hsync_len + dt->hback_porch -1);
	ltdc.Init.AccumulatedActiveW = (dt->hactive + dt->hsync_len + dt->hback_porch -1);
	ltdc.Init.TotalWidth = (dt->hactive + dt->hsync_len + dt->hback_porch + dt->hfront_porch -1);
	ltdc.Init.VerticalSync = (dt->vsync_len -1);
	ltdc.Init.AccumulatedVBP = (dt->vsync_len + dt->vback_porch-1);
	ltdc.Init.AccumulatedActiveH = (dt->vactive + dt->vsync_len + dt->vback_porch-1);
	ltdc.Init.TotalHeigh = (dt->vactive + dt->vsync_len + dt->vback_porch + dt->vfront_porch-1);

	/* background value */
	ltdc.Init.Backcolor.Blue = 0x00;
	ltdc.Init.Backcolor.Green = 0x00;
	ltdc.Init.Backcolor.Red = 0x00;

	ltdc.LayerCfg->ImageWidth  = lcd_x_size;
	ltdc.LayerCfg->ImageHeight = lcd_y_size;

	/* Polarity */
	ltdc.Init.HSPolarity = dt->hpol ? LTDC_HSPOLARITY_AH : LTDC_HSPOLARITY_AL;
	ltdc.Init.VSPolarity = dt->vpol ? LTDC_VSPOLARITY_AH : LTDC_VSPOLARITY_AL;
	ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	/* Initialize & Start the LTDC */
	HAL_LTDC_Init(&ltdc);

	/* Enable the DSI host and wrapper : but LTDC is not started yet at this stage */
	HAL_DSI_Start(&dsi);

	stm32_LayerInit(0, FB_ADDRESS_0);
	stm32_LayerInit(1, FB_ADDRESS_1);

	//HAL_LTDC_ProgramLineEvent(&ltdc, 0);

	HAL_DSI_PatternGeneratorStart(&dsi, 0, 1);
	HAL_DSI_PatternGeneratorStop(&dsi);
	stm32_LCD_Clear(0);
	stm32_LCD_Clear(0);
}

static void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex)
{
	/* Register to memory mode with ARGB8888 as color Mode */
	dma2d.Init.Mode         = DMA2D_R2M;
	dma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565;	//DMA2D_OUTPUT_ARGB8888
	dma2d.Init.OutputOffset = OffLine;

	dma2d.Instance = DMA2D;

	/* DMA2D Initialization */
	if(HAL_DMA2D_Init(&dma2d) == HAL_OK)
	{
		if(HAL_DMA2D_ConfigLayer(&dma2d, 1) == HAL_OK)
		{
			if (HAL_DMA2D_Start(&dma2d, ColorIndex, (uint32_t)pDst, xSize, ySize) == HAL_OK)
			{
				/* Polling For DMA transfer */
				HAL_DMA2D_PollForTransfer(&dma2d, 25);
			}
		}
	}
}

DMA2D_HandleTypeDef* stm32_get_DMA2D(void)
{
	return &dma2d;
}

void stm32_LCD_Clear(uint32_t Color)
{
	/* Clear the LCD */
	LL_FillBuffer(pend_buffer%2, (uint32_t *)(ltdc.LayerCfg[pend_buffer%2].FBStartAdress), lcd_x_size, lcd_y_size, 0, Color);
	getNextFrameBuffer();
}


void stm32_LCD_FillArea(void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode)
{
	LL_FillBuffer(pend_buffer%2, pDst, xSize, ySize, lcd_x_size - xSize, ColorMode);
}

void stm32_LCD_DrawImage(void *pSrc, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode)
{
	/* Configure the DMA2D Mode, Color Mode and output offset */
	dma2d.Init.Mode         = DMA2D_M2M_PFC;
	dma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565;
	dma2d.Init.OutputOffset = lcd_x_size - xSize;

	if (pDst == NULL) {
		pDst = (uint32_t *)(ltdc.LayerCfg[pend_buffer%2].FBStartAdress);
	}

	/* Foreground Configuration */
	dma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	dma2d.LayerCfg[1].InputAlpha = 0xFF;
	dma2d.LayerCfg[1].InputColorMode = ColorMode;
	dma2d.LayerCfg[1].InputOffset = 0;

	dma2d.Instance = DMA2D;

	/* DMA2D Initialization */
	if(HAL_DMA2D_Init(&dma2d) == HAL_OK)
	{
		if(HAL_DMA2D_ConfigLayer(&dma2d, 1) == HAL_OK)
		{
			if (HAL_DMA2D_Start(&dma2d, (uint32_t)pSrc, (uint32_t)pDst, xSize, ySize) == HAL_OK)
			{
				/* Polling For DMA transfer */
				HAL_DMA2D_PollForTransfer(&dma2d, 25);
			}
		}
	}
}

/*
static uint32_t  ActiveLayer = 0;
static LCD_DrawPropTypeDef DrawProp[2];

void stm32_BriefDisplay(void)
{
	LCD_Clear(LCD_COLOR_WHITE);
	LCD_SetBackColor(LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_FillRect(0, 0, lcd_x_size, 112);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_DisplayStringAt(0, LINE(2), (uint8_t *)"LCD_DSI_VideoMode_SingleBuffer", CENTER_MODE);
	LCD_SetFont(&Font16);
	LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"This example shows how to display images", CENTER_MODE);
	LCD_DisplayStringAt(0, LINE(6), (uint8_t *)"on LCD DSI using same buffer for display and for draw", CENTER_MODE);
}
*/

int anx7625_dp_get_edid(uint8_t bus, struct edid *out)
{
	int block_num;
	int ret;
	u8 edid[FOUR_BLOCK_SIZE];

	block_num = sp_tx_edid_read(bus, edid, FOUR_BLOCK_SIZE);
	if (block_num < 0) {
		ANXERROR("Failed to get eDP EDID.\n");
		return -1;
	}

	ret = decode_edid(edid, (block_num + 1) * ONE_BLOCK_SIZE, out);
	if (ret != EDID_CONFORMANT) {
		ANXERROR("Failed to decode EDID.\n");
		return -1;
	}

	return 0;
}

int anx7625_init(uint8_t bus)
{
	int retry_hpd_change = 500;
	int retry_power_on = 3;

	while (--retry_power_on) {
		if (anx7625_power_on_init(bus) == 0)
			break;
	}
	if (!retry_power_on) {
		ANXERROR("Failed to power on.\n");
		return -1;
	}

	while (--retry_hpd_change) {
		mdelay(10);
		int detected = anx7625_hpd_change_detect(bus);
		if (detected < 0)
			return -1;
		if (detected > 0)
			return 0;
	}

	ANXERROR("Timed out to detect HPD change on bus %d.\n", bus);
	return -1;
}
