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
#include "anx7625_display.h"
#include "display.h"
#include "Flash.h"

/* Use device tree structure data when defined "CONFIG_OF"  */
/* #define CONFIG_OF */



static int create_sysfs_interfaces(struct device *dev);
static int destory_sysfs_interfaces(struct device *dev);

/* to access global platform data */
static struct anx7625_platform_data *g_pdata;



atomic_t anx7625_power_status;
unsigned char cable_connected;
unsigned char alert_arrived;
unsigned char vbus_en;

struct i2c_client *anx7625_client;

struct anx7625_platform_data {
	PinName gpio_p_on;
	PinName gpio_reset;
	PinName gpio_cbl_det;
	mbed::InterruptIn* cbl_det_irq;
#ifdef SUP_INT_VECTOR
	PinName gpio_intr_comm;
	mbed::InterruptIn* irq;
#endif
#ifdef SUP_VBUS_CTL
	PinName gpio_vbus_ctrl;
#endif
};

struct anx7625_data {
	struct anx7625_platform_data *pdata;
	struct mutex lock;
#ifdef DYNAMIC_CONFIG_MIPI
	struct msm_dba_device_info dev_info;
#endif
};

/* to access global platform data */
static struct anx7625_data *the_chip_anx7625;


unsigned char debug_on;
static unsigned char auto_start = 1; /* auto update OCM FW*/

unsigned char hpd_status;

static unsigned char default_dpi_config = 0xff; /*1:720p  3:1080p*/
static unsigned char default_dsi_config = RESOLUTION_QCOM820_1080P60_DSI;
static unsigned char default_audio_config = AUDIO_I2S_2CH_48K; /*I2S 2 channel*/

static unsigned char  last_read_DevAddr = 0xff;

#ifdef DYNAMIC_CONFIG_MIPI

struct timer_list mytimer;
int DBA_init_done = 0;
static void DelayDisplayFunc(unsigned long data)

{
	TRACE("Delay Display Triggered!\n");
	DBA_init_done = 1;
	/*downstream already inserted, trigger cable-det isr to check. */

#ifdef DISABLE_PD
		cable_connected = 1;
		anx7625_restart_work(10);
#else
/*trigger chip restart*/
		anx7625_restart_work(10);
#endif

}



static int __init displaytimer_init(void)

{
	setup_timer(&mytimer, DelayDisplayFunc, (unsigned long)"Delay DBA Timer!");

	TRACE("Starting timer to fire in 20s (%ld)\n", jiffies);
	mod_timer(&mytimer, jiffies + msecs_to_jiffies(20000));

	return 0;
}

#endif



/* software workaround for silicon bug MIS2-124 */
static void Reg_Access_Conflict_Workaround(unsigned char DevAddr)
{
	unsigned char RegAddr;
	int ret = 0;

	if (DevAddr != last_read_DevAddr) {
		switch (DevAddr) {
		case  0x54:
		case  0x72:
		default:
			RegAddr = 0x00;
			break;

		case  0x58:
			RegAddr = 0x00;
			break;

		case  0x70:
			RegAddr = 0xD1;
			break;

		case  0x7A:
			RegAddr = 0x60;
			break;

		case  0x7E:
			RegAddr = 0x39;
			break;

		case  0x84:
			RegAddr = 0x7F;
			break;
		}


		anx7625_client->addr = (DevAddr >> 1);
		ret = i2c_smbus_write_byte_data(anx7625_client, RegAddr, 0x00);
		if (ret < 0) {
			pr_err("%s %s: failed to write i2c addr=%x\n:%x",
				LOG_TAG, __func__, DevAddr, RegAddr);

		}
		last_read_DevAddr = DevAddr;
	}

}



/* anx7625 power status, sync with interface and cable detection thread */

inline unsigned char ReadReg(unsigned char DevAddr, unsigned char RegAddr)
{
	int ret = 0;

	Reg_Access_Conflict_Workaround(DevAddr);

	anx7625_client->addr = DevAddr >> 1;
	ret = i2c_smbus_read_byte_data(anx7625_client, RegAddr);
	if (ret < 0) {
		pr_err("%s %s: failed to read i2c addr=%x:%x\n", LOG_TAG,
			__func__, DevAddr, RegAddr);
	}
	return (uint8_t) ret;

}
unsigned char GetRegVal(unsigned char DevAddr, unsigned char RegAddr)
{
	return ReadReg(DevAddr, RegAddr);
}

int Read_Reg(uint8_t slave_addr, uint8_t offset, uint8_t *buf)
{
	int ret = 0;

	Reg_Access_Conflict_Workaround(slave_addr);

	anx7625_client->addr = (slave_addr >> 1);
	ret = i2c_smbus_read_byte_data(anx7625_client, offset);
	if (ret < 0) {
		pr_err("%s %s: failed to read i2c addr=%x:%x\n", LOG_TAG,
			__func__, slave_addr, offset);
		return ret;
	}
	*buf = (uint8_t) ret;

	return 0;
}


int ReadBlockReg(unsigned char DevAddr, u8 RegAddr, u8 len, u8 *dat)
{
	int ret = 0;

	Reg_Access_Conflict_Workaround(DevAddr);

	anx7625_client->addr = (DevAddr >> 1);
	ret = i2c_smbus_read_i2c_block_data(anx7625_client, RegAddr, len, dat);
	if (ret < 0) {
		pr_err("%s %s: failed to read i2c block addr=%x:%x\n", LOG_TAG,
			__func__, DevAddr, RegAddr);
		return -EPERM;
	}

	return (int)ret;
}


int WriteBlockReg(unsigned char DevAddr, u8 RegAddr, u8 len,
	const u8 *dat)
{
	int ret = 0;

	Reg_Access_Conflict_Workaround(DevAddr);

	anx7625_client->addr = (DevAddr >> 1);
	ret = i2c_smbus_write_i2c_block_data(anx7625_client, RegAddr, len, (u8*) dat);
	if (ret < 0) {
		pr_err("%s %s: failed to write i2c block addr=%x:%x\n", LOG_TAG,
			__func__, DevAddr, RegAddr);
		return -EPERM;
	}

	return (int)ret;
}

inline void WriteReg(unsigned char DevAddr, unsigned char RegAddr,
	unsigned char RegVal)
{
	int ret = 0;

	Reg_Access_Conflict_Workaround(DevAddr);

	anx7625_client->addr = (DevAddr >> 1);
	ret = i2c_smbus_write_byte_data(anx7625_client, RegAddr, RegVal);
	if (ret < 0) {
		pr_err("%s %s: failed to write i2c addr=%x\n:%x", LOG_TAG,
			__func__, DevAddr, RegAddr);
	}
}

void MI2_power_on(void)
{

	struct anx7625_platform_data *pdata = g_pdata;

	/*power on pin enable */
	gpio_set_value(pdata->gpio_p_on, 1);
	usleep_range(10000, 11000);
	/*power reset pin enable */
	gpio_set_value(pdata->gpio_reset, 1);
	usleep_range(10000, 11000);


	TRACE("%s %s: Anx7625 power on !\n", LOG_TAG, __func__);
}

void anx7625_hardware_reset(int enable)
{

	struct anx7625_platform_data *pdata = g_pdata;

	gpio_set_value(pdata->gpio_reset, enable);
}


void anx7625_power_standby(void)
{

	struct anx7625_platform_data *pdata = g_pdata;

	gpio_set_value(pdata->gpio_reset, 0);
	usleep_range(1000, 1100);
	gpio_set_value(pdata->gpio_p_on, 0);
	usleep_range(1000, 1100);


	TRACE("%s %s: anx7625 power down\n", LOG_TAG, __func__);
}

/*configure DPR toggle*/
void ANX7625_DRP_Enable(void)
{
	/*reset main OCM*/
	WriteReg(RX_P0, OCM_DEBUG_REG_8, 1<<STOP_MAIN_OCM);
	/*config toggle.*/
	WriteReg(TCPC_INTERFACE, TCPC_ROLE_CONTROL, 0x45);
	WriteReg(TCPC_INTERFACE, TCPC_COMMAND, 0x99);
	WriteReg(TCPC_INTERFACE, ANALOG_CTRL_1, 0xA0);
	WriteReg(TCPC_INTERFACE, ANALOG_CTRL_1, 0xE0);

	TRACE("Enable DRP!");
}
/* basic configurations of ANX7625 */
void ANX7625_config(void)
{
	WriteReg(RX_P0, XTAL_FRQ_SEL, XTAL_FRQ_27M);

}


BYTE ANX7625_Chip_Located(void)
{
	BYTE c1, c2;

	MI2_power_on();
	Read_Reg(TCPC_INTERFACE, PRODUCT_ID_L, &c1);
	Read_Reg(TCPC_INTERFACE, PRODUCT_ID_H, &c2);
	anx7625_power_standby();
	if ((c1 == 0x25) && (c2 == 0x76)) {
		TRACE("ANX7625 is detected!\n");
		return 1;
	}
	TRACE("No ANX7625 found!\n");
	return 0;
}


#define FLASH_LOAD_STA 0x05
#define FLASH_LOAD_STA_CHK	(1<<7)

void anx7625_hardware_poweron(void)
{

	int retry_count, i;


	for (retry_count = 0; retry_count < 3; retry_count++) {

		MI2_power_on();

		ANX7625_config();



		for (i = 0; i < OCM_LOADING_TIME; i++) {
			/*Interface work? */
			if ((ReadReg(OCM_SLAVE_I2C_ADDR, FLASH_LOAD_STA)&
				FLASH_LOAD_STA_CHK) == FLASH_LOAD_STA_CHK) {
				TRACE("%s %s: interface initialization\n",
					LOG_TAG, __func__);

#ifdef DISABLE_PD
	/*reset main ocm*/
	WriteReg(RX_P0, 0x88,  0x40);
	/*Disable PD*/
	WriteReg(OCM_SLAVE_I2C_ADDR, AP_AV_STATUS, AP_DISABLE_PD);
	/*release main ocm*/
	WriteReg(RX_P0, 0x88,  0x00);
	TRACE("%s: Disable PD\n", LOG_TAG);
#else
	chip_register_init();
	send_initialized_setting();
#endif
	mute_video_flag = 0;

	TRACE("Firmware version %02x%02x,Driver version %s\n",
		ReadReg(OCM_SLAVE_I2C_ADDR, OCM_FW_VERSION),
		ReadReg(OCM_SLAVE_I2C_ADDR, OCM_FW_REVERSION),
		ANX7625_DRV_VERSION);
				return;
			}
			usleep_range(1000, 1100);
		}
		anx7625_power_standby();

	}

}

#define RG_EN_OTG	 (0x1<<0x3)

void anx7625_vbus_control(bool on)
{
#ifdef SUP_VBUS_CTL

	struct anx7625_platform_data *pdata = g_pdata;

	if (on)
		gpio_set_value(pdata->gpio_vbus_ctrl, ENABLE_VBUS_OUTPUT);
	else
		gpio_set_value(pdata->gpio_vbus_ctrl, DISABLE_VBUS_OUTPUT);
#endif


}

static void anx7625_free_gpio(struct anx7625_data *platform)
{

}

static int anx7625_init_gpio(struct anx7625_data *platform)
{

	platform->pdata->gpio_p_on = gpio_request("anx7625_p_on_ctl");
	gpio_direction_output(platform->pdata->gpio_p_on, 0);

	platform->pdata->gpio_reset = gpio_request("anx7625_reset_n");
	gpio_direction_output(platform->pdata->gpio_reset, 0);

#ifndef DISABLE_PD
	/*  gpio for cable detect  */
	platform->pdata->gpio_cbl_det = gpio_request("anx7625_cbl_det");
	gpio_direction_input(platform->pdata->gpio_cbl_det);
#endif

#ifdef SUP_INT_VECTOR
	/*  gpio for chip interface communaction */
	platform->pdata->gpio_intr_comm = gpio_request("anx7625_intr_comm");
	gpio_direction_input(platform->pdata->gpio_intr_comm);

#endif

#ifdef SUP_VBUS_CTL
	/*  gpio for vbus control  */
	platform->pdata->gpio_vbus_ctrl = gpio_request("anx7625_vbus_ctrl");
	gpio_direction_output(platform->pdata->gpio_vbus_ctrl, DISABLE_VBUS_OUTPUT);
#endif

	return 0;
}



void anx7625_main_process(void)
{

#ifdef DYNAMIC_CONFIG_MIPI
	struct anx7625_data *td;

	td = the_chip_anx7625;
#endif

	TRACE("%s %s:cable_connected=%d power_status=%d\n",
		LOG_TAG, __func__, cable_connected,
		(unsigned int)atomic_read(&anx7625_power_status));


	/* do main loop, do what you want to do */
	if (auto_start) {
		auto_start = 0;
		mute_video_flag = 0;
		if (ANX7625_Chip_Located() == 0) {
			debug_on = 1;
			return;
		}

#if AUTO_UPDATE_OCM_FW
		burnhexauto();
#endif

#if 0 // #ifndef DISABLE_PD
		MI2_power_on();
		ANX7625_DRP_Enable();
		usleep_range(1000, 1100);
		anx7625_power_standby();
#endif


#ifdef DYNAMIC_CONFIG_MIPI
		displaytimer_init();

#else
		/*dongle already inserted, trigger cable-det isr to check. */
		anx7625_cbl_det_isr(the_chip_anx7625);
#endif


		return;

	}

#ifdef DYNAMIC_CONFIG_MIPI
	if (DBA_init_done == 1){
		/*check dongle status. */
		anx7625_cbl_det_isr(the_chip_anx7625);
		DBA_init_done++;
		return;
		}
#endif

	if (atomic_read(&anx7625_power_status) == 0) {
		if (cable_connected == 1) {
			atomic_set(&anx7625_power_status, 1);
			anx7625_hardware_poweron();
			return;
		}
	} else {
		if (cable_connected == 0) {
			atomic_set(&anx7625_power_status, 0);
#ifdef SUP_VBUS_CTL
			/*Disable VBUS supply.*/
			anx7625_vbus_control(0);

			/*gpio_set_value(platform->pdata->gpio_vbus_ctrl,
				DISABLE_VBUS_OUTPUT);*/
#endif


			if (hpd_status >= 1)
				anx7625_stop_dp_work();

			ANX7625_DRP_Enable();
			usleep_range(1000, 1100);
			clear_sys_sta_bak();
			mute_video_flag = 0;
			anx7625_power_standby();
			return;
		}


		if (alert_arrived)
			anx7625_handle_intr_comm();

	}
}

static void anx7625_work_func()
{
	struct anx7625_data *td;
	td = the_chip_anx7625;

	mutex_lock(&td->lock);
	anx7625_main_process();
	mutex_unlock(&td->lock);

}



void anx7625_restart_work(int workqueu_timer)
{
	struct anx7625_data *td;

	td = the_chip_anx7625;
	if (td != NULL) {
		//wait(workqueu_timer / 1000.0f);
		anx7625_work_func();
	}
}



void anx7625_stop_dp_work(void)
{

#ifdef DYNAMIC_CONFIG_MIPI
	struct anx7625_data *td;

	td = the_chip_anx7625;
	/* Notify DBA framework disconnect event */
	if (td != NULL)
		anx7625_notify_clients(&td->dev_info,
						MSM_DBA_CB_HPD_DISCONNECT);
#endif

	/* hpd changed */
	TRACE("anx7625_stop_dp_work: mute_flag: %d\n",
		(unsigned int)mute_video_flag);

	DP_Process_Stop();
	hpd_status = 0;
	mute_video_flag = 0;


	delay_tab_id = 0;


}


#ifndef DISABLE_PD
#ifdef CABLE_DET_PIN_HAS_GLITCH
static unsigned char confirmed_cable_det(void *data)
{
	struct anx7625_data *platform = (struct anx7625_data *)data;

	unsigned int count = 10;
	unsigned int cable_det_count = 0;
	u8 val = 0;

	do {

		val = gpio_get_value(platform->pdata->gpio_cbl_det);

		if (val == DONGLE_CABLE_INSERT)
			cable_det_count++;
			usleep_range(1000, 1100);
	} while (count--);

	if (cable_det_count > 7)
		return 1;
	else if (cable_det_count < 2)
		return 0;
	else
		return atomic_read(&anx7625_power_status);
}

#endif
#endif

void anx7625_start_dp(void)
{


	/* hpd changed */
	TRACE("anx7625_start_dp: mute_flag: %d\n",
		(unsigned int)mute_video_flag);

	if (hpd_status >= 2) {
		TRACE("anx7625 filter useless HPD\n");
		return;
	}

	hpd_status++;

	DP_Process_Start();

#ifdef DYNAMIC_CONFIG_MIPI

	/* Notify DBA framework connect event */
	if (delay_tab_id == 0) {
		anx7625_notify_clients(&(the_chip_anx7625->dev_info),
						MSM_DBA_CB_HPD_CONNECT);

	}

#else

	if (delay_tab_id == 0) {
		if (default_dpi_config < 0x20)
			command_DPI_Configuration(default_dpi_config);
		else if (default_dsi_config < 0x20)
			command_DSI_Configuration(default_dsi_config);

		if (default_audio_config < 0x20)
			API_Configure_Audio_Input(default_audio_config);
	}

#endif

}


irqreturn_t anx7625_cbl_det_isr(void *data)
{
	struct anx7625_data *platform = (struct anx7625_data *)data;

#ifdef DYNAMIC_CONFIG_MIPI
	if (DBA_init_done == 0)
		return IRQ_NONE;
#endif

#ifdef CABLE_DET_PIN_HAS_GLITCH
	cable_connected = confirmed_cable_det((void *)platform);
#else
	cable_connected = gpio_get_value(platform->pdata->
		gpio_cbl_det);
#endif

	TRACE("%s %s : cable plug pin status %d\n", LOG_TAG,
		__func__, cable_connected);

	if (cable_connected == DONGLE_CABLE_INSERT) {
		if (atomic_read(&anx7625_power_status) == 1)
			return IRQ_HANDLED;
		cable_connected = 1;
		anx7625_restart_work(10);
	} else {
		if (atomic_read(&anx7625_power_status) == 0)
			return IRQ_HANDLED;
		cable_connected = 0;
		anx7625_restart_work(10);
	}

	return IRQ_HANDLED;
}

#ifdef SUP_INT_VECTOR
irqreturn_t anx7625_intr_comm_isr(void *data)
{

/*
	if (atomic_read(&anx7625_power_status) != 1)
		return IRQ_NONE;
*/

	alert_arrived = 1;
	anx7625_restart_work(10);
	return IRQ_HANDLED;

}
void anx7625_handle_intr_comm(void)
{
	unsigned char c;

	c = ReadReg(TCPC_INTERFACE, INTR_ALERT_1);

	TRACE("%s %s : ======I=====alert=%02x\n",
		LOG_TAG, __func__, (uint)c);

	if (c & INTR_SOFTWARE_INT)
		handle_intr_vector();

	if (c & INTR_RECEIVED_MSG)
		/*Received interface message*/
		handle_msg_rcv_intr();

	while (ReadReg(OCM_SLAVE_I2C_ADDR,
		INTERFACE_CHANGE_INT) != 0)
		handle_intr_vector();

	WriteReg(TCPC_INTERFACE, INTR_ALERT_1, 0xFF);

	if ((gpio_get_value(the_chip_anx7625->pdata->gpio_intr_comm)) == 0) {

		alert_arrived = 1;
		anx7625_restart_work(1);
		TRACE("%s %s : comm isr pin still low, re-enter\n",
			LOG_TAG, __func__);
	} else {
		alert_arrived = 0;
		TRACE("%s %s : comm isr pin cleared\n",
			LOG_TAG, __func__);
	}

}
#endif

#ifdef CONFIG_OF
static int anx7625_parse_dt(struct device *dev,
	struct anx7625_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	pdata->gpio_p_on =
		of_get_named_gpio_flags(np, "analogix,p-on-gpio",
		0, NULL);

	pdata->gpio_reset =
		of_get_named_gpio_flags(np, "analogix,reset-gpio",
		0, NULL);
#ifndef DISABLE_PD
	pdata->gpio_cbl_det =
		of_get_named_gpio_flags(np, "analogix,cbl-det-gpio",
		0, NULL);
#endif

#ifdef SUP_VBUS_CTL
	/*reuse previous unless gpio(v33_ctrl) for vbus control*/
	pdata->gpio_vbus_ctrl =
		of_get_named_gpio_flags(np, "analogix,v33-ctrl-gpio",
		0, NULL);
#endif
#ifdef SUP_INT_VECTOR
	pdata->gpio_intr_comm =
		of_get_named_gpio_flags(np, "analogix,intr-comm-gpio",
		0, NULL);
#endif
	TRACE("%s gpio p_on : %d, reset : %d\n",
		LOG_TAG, pdata->gpio_p_on,
		pdata->gpio_reset);

	return 0;
}
#else
static int anx7625_parse_dt(struct device *dev,
	struct anx7625_platform_data *pdata)
{
	return -ENODEV;
}
#endif

#ifdef DYNAMIC_CONFIG_MIPI

static int anx7625_register_dba(struct anx7625_data *pdata)
{
	struct msm_dba_ops *client_ops;
	struct msm_dba_device_ops *dev_ops;

	if (!pdata)
		return -EINVAL;

	client_ops = &pdata->dev_info.client_ops;
	dev_ops = &pdata->dev_info.dev_ops;

	client_ops->power_on		= NULL;
	client_ops->video_on		= anx7625_mipi_timing_setting;
	client_ops->configure_audio = anx7625_audio_setting;
	client_ops->hdcp_enable	 = NULL;
	client_ops->hdmi_cec_on	 = NULL;
	client_ops->hdmi_cec_write  = NULL;
	client_ops->hdmi_cec_read   = NULL;
	client_ops->get_edid_size   = NULL;
	client_ops->get_raw_edid	= anx7625_get_raw_edid;
	client_ops->check_hpd		= NULL;

	strlcpy(pdata->dev_info.chip_name, "anx7625",
			sizeof(pdata->dev_info.chip_name));

	pdata->dev_info.instance_id = 0;

	mutex_init(&pdata->dev_info.dev_mutex);

	INIT_LIST_HEAD(&pdata->dev_info.client_list);

	return msm_dba_add_probed_device(&pdata->dev_info);
}
void anx7625_notify_clients(struct msm_dba_device_info *dev,
		enum msm_dba_callback_event event)
{
	struct msm_dba_client_info *c;
	struct list_head *pos = NULL;

	TRACE("%s++\n", __func__);

	if (!dev) {
		pr_err("%s: invalid input\n", __func__);
		return;
	}

	list_for_each(pos, &dev->client_list) {
		c = list_entry(pos, struct msm_dba_client_info, list);

		TRACE("%s:notifying event %d to client %s\n", __func__,
			event, c->client_name);

		if (c && c->cb)
			c->cb(c->cb_data, event);
	}

	TRACE("%s--\n", __func__);
}


int anx7625_mipi_timing_setting(void *client, bool on,
			struct msm_dba_video_cfg *cfg, u32 flags)
{

	if (cable_connected == 0)
		return 0;

	if (!client || !cfg) {
		pr_err("%s: invalid platform data\n", __func__);
		command_Mute_Video(1);
		return 0;
	}

	TRACE("%s: anx7625_mipi_timing_setting(), v_active=%d, h_active=%d, pclk=%d\n",
		__func__, cfg->v_active, cfg->h_active, cfg->pclk_khz);
	TRACE(" vic=%d, hdmi_mode=%d, num_of_input_lanes=%d, scaninfo=%d\n",
		cfg->vic, cfg->hdmi_mode, cfg->num_of_input_lanes, cfg->scaninfo);

	TRACE("cfg->pclk_khz h= %04x\n", ((cfg->pclk_khz)>>16)&0xffff);
	TRACE("cfg->pclk_khz l= %04x\n", ((cfg->pclk_khz))&0xffff);

	TRACE(" h_active = %d, hfp = %d, hpw = %d, hbp = %d\n",
		 cfg->h_active, cfg->h_front_porch,
		cfg->h_pulse_width, cfg->h_back_porch);

	TRACE(" v_active = %d, vfp = %d, vpw = %d, vbp = %d\n",
		 cfg->v_active, cfg->v_front_porch,
		cfg->v_pulse_width, cfg->v_back_porch);

	if (cfg->v_active <= 480) {
		TRACE("480P\n");
		default_dsi_config = RESOLUTION_480P_DSI;
	} else if ((cfg->v_active > 480) && ((cfg->v_active <= 720))) {
		TRACE("720P\n");
		default_dsi_config = RESOLUTION_720P_DSI;
	} else if ((cfg->v_active > 720) && ((cfg->v_active <= 1080))) {
		default_dsi_config = RESOLUTION_QCOM820_1080P60_DSI;
		TRACE("1080P\n");
	} else if (cfg->v_active > 1080) {
		default_dsi_config = RESOLUTION_QCOM820_1080P60_DSI;
		TRACE("Out max support, set to 1080P\n");
	} else {
		TRACE("unknown resolution\n");
		default_dsi_config = RESOLUTION_480P_DSI;
	}
	/*command_DSI_Configuration(default_dsi_config);*/
	DSI_Configuration(default_dsi_config);
	hpd_status = 1;
	return 0;
}

int anx7625_audio_setting(void *client,
	struct msm_dba_audio_cfg *cfg, u32 flags)
{

	if (cable_connected == 0)
		return 0;

	if (!client || !cfg) {
		pr_err("%s: invalid platform data\n", __func__);
		return 0;
	}

	if (cfg->interface != MSM_DBA_AUDIO_I2S_INTERFACE) {
		pr_err("%s: invalid i2s config.\n", __func__);
		return 0;
	}

	if (cfg->sampling_rate == MSM_DBA_AUDIO_32KHZ)
		default_audio_config = AUDIO_I2S_2CH_32K;
	else if (cfg->sampling_rate == MSM_DBA_AUDIO_44P1KHZ)
		default_audio_config = AUDIO_I2S_2CH_441K;
	else if (cfg->sampling_rate == MSM_DBA_AUDIO_48KHZ)
		default_audio_config = AUDIO_I2S_2CH_48K;
	else if (cfg->sampling_rate == MSM_DBA_AUDIO_88P2KHZ)
		default_audio_config = AUDIO_I2S_2CH_882K;
	else if (cfg->sampling_rate == MSM_DBA_AUDIO_96KHZ)
		default_audio_config = AUDIO_I2S_2CH_96K;
	else if (cfg->sampling_rate == MSM_DBA_AUDIO_176P4KHZ)
		default_audio_config = AUDIO_I2S_2CH_1764K;
	else if (cfg->sampling_rate == MSM_DBA_AUDIO_192KHZ)
		default_audio_config = AUDIO_I2S_2CH_192K;
	else
		default_audio_config = AUDIO_I2S_2CH_32K;

	command_Configure_Audio_Input(default_audio_config);

	return 0;
}


int anx7625_get_raw_edid(void *client,
			u32 size, char *buf, u32 flags)
{
	int block_num;
	struct s_edid_data *p_edid =
		(struct s_edid_data *)slimport_edid_p;

	if (!buf) {
		pr_err("%s: invalid data\n", __func__);
		goto end;
	}

	if (!p_edid) {
		pr_err("%s: invalid edid data\n", __func__);
		size = 0;
		goto end;
	}


	memcpy((uint8_t *)buf,
		(uint8_t *)(p_edid->EDID_block_data), (p_edid->edid_block_num + 1) * 128);
	block_num = p_edid->edid_block_num;

	if (block_num >= 0)
		size = min_t(u32, (block_num + 1) * 128, 4 * 128);
	else
		size = 0;

	TRACE("%s: memcpy EDID block, size=%d\n", __func__, size);

end:
	return 0;
}

#endif

events::EventQueue queue(32 * EVENTS_EVENT_SIZE);
static rtos::Thread t(osPriorityHigh, 32 * 1024, NULL, "msd");

int anx7625_i2c_probe(struct i2c_client *client)
{

	struct anx7625_data *platform;
	struct anx7625_platform_data *pdata;
	int ret = 0;

	platform = (struct anx7625_data *)malloc(sizeof(struct anx7625_data));
	if (!platform) {
		/*pr_err("%s: failed to allocate driver data\n", __func__);*/
		return -ENOMEM;
	}
	memset(platform, 0, sizeof(struct anx7625_data));

	platform->pdata = (struct anx7625_platform_data *)malloc(sizeof(struct anx7625_platform_data));
	memset(platform->pdata, 0, sizeof(struct anx7625_platform_data));

	/* to access global platform data */
	g_pdata = platform->pdata;
	the_chip_anx7625 = platform;
	anx7625_client = client;
	anx7625_client->addr = (OCM_SLAVE_I2C_ADDR1 >> 1);

	anx7625_client->dev.platform_data = g_pdata;

	atomic_set(&anx7625_power_status, 0);

	mutex_init(&platform->lock);

	if (!platform->pdata) {
		return -EINVAL;
	}

	ret = anx7625_init_gpio(platform);
	if (ret) {
		pr_err("%s: failed to initialize gpio\n", __func__);
		return ret;
	}

	platform->pdata->cbl_det_irq = new mbed::InterruptIn(platform->pdata->gpio_cbl_det);

#ifdef SUP_INT_VECTOR

	platform->pdata->irq = new mbed::InterruptIn(platform->pdata->gpio_intr_comm);

#endif

#ifdef DYNAMIC_CONFIG_MIPI

	/* Register msm dba device */
	ret = anx7625_register_dba(platform);
	if (ret) {
		pr_err("%s: Error registering with DBA %d\n",
			__func__, ret);
	}

#endif

	anx7625_work_func();

	platform->pdata->cbl_det_irq->rise(queue.event(mbed::callback(anx7625_cbl_det_isr, (void*)platform)));

#ifdef SUP_INT_VECTOR
	platform->pdata->irq->fall(queue.event(mbed::callback(anx7625_intr_comm_isr, (void*)platform)));
#endif

	TRACE("anx7625_i2c_probe successfully %s %s end\n",
		LOG_TAG, __func__);

	t.start(callback(&queue, &events::EventQueue::dispatch_forever));

	return ret;
}

int anx7625_i2c_remove(struct i2c_client *client)
{
	struct anx7625_data *platform = the_chip_anx7625;

#ifndef DISABLE_PD
	delete platform->pdata->cbl_det_irq;
#endif
	return 0;
}


ssize_t anx7625_send_pd_cmd(const char *buf, size_t count)
{
	int cmd;
	int result;

	result = sscanf(buf, "%d", &cmd);
	switch (cmd) {
	case TYPE_PWR_SRC_CAP:
		send_pd_msg(TYPE_PWR_SRC_CAP, 0, 0);
		break;

	case TYPE_DP_SNK_IDENTITY:
		send_pd_msg(TYPE_DP_SNK_IDENTITY, 0, 0);
		break;

	case TYPE_PSWAP_REQ:
		send_pd_msg(TYPE_PSWAP_REQ, 0, 0);
		break;
	case TYPE_DSWAP_REQ:
		send_pd_msg(TYPE_DSWAP_REQ, 0, 0);
		break;

	case TYPE_GOTO_MIN_REQ:
		send_pd_msg(TYPE_GOTO_MIN_REQ, 0, 0);
		break;

	case TYPE_PWR_OBJ_REQ:
		interface_send_request();
		break;
	case TYPE_ACCEPT:
		interface_send_accept();
		break;
	case TYPE_REJECT:
		interface_send_reject();
		break;
	case TYPE_SOFT_RST:
		send_pd_msg(TYPE_SOFT_RST, 0, 0);
		break;
	case TYPE_HARD_RST:
		send_pd_msg(TYPE_HARD_RST, 0, 0);
		break;

	}
	return count;
}


ssize_t anx7625_send_pswap(char *buf)
{
	return snprintf(buf, 3, "%d\n", send_power_swap());
}

ssize_t anx7625_send_dswap(char *buf)
{
	return snprintf(buf, 3, "%d\n", send_data_swap());
}

ssize_t anx7625_get_data_role(char *buf)
{
	return snprintf(buf, 3, "%d\n", get_data_role());
}
ssize_t anx7625_get_power_role(char *buf)
{
	return snprintf(buf, 3, "%d\n", get_power_role());
}

ssize_t anx7625_rd_reg(const char *buf)
{
	int v_addr, cmd;
	int result;

	result = sscanf(buf, "%x  %x", &v_addr, &cmd);
	pr_info("reg[%x] = %x\n", cmd, ReadReg(v_addr, cmd));

	return 1;

}

ssize_t anx7625_wr_reg(const char *buf)
{
	int v_addr, cmd, val;
	int result;

	result = sscanf(buf, "%x  %x  %x", &v_addr, &cmd, &val);
	pr_info("c %x val %x\n", cmd, val);
	WriteReg(v_addr, cmd, val);
	pr_info("reg[%x] = %x\n", cmd, ReadReg(v_addr, cmd));
	return 1;
}

ssize_t anx7625_dump_register(const char *buf)
{
	int i = 0;
	int val;
	int result;
	unsigned char  pLine[100];

	memset(pLine, 0, 100);
	result = sscanf(buf, "%x", &val);

	pr_info(" dump register (0x%x)......\n", val);
	pr_info("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	for (i = 0; i < 256; i++) {
		snprintf((char*)&(pLine[(i%0x10)*3]), 4, "%02X ", ReadReg(val, i));
		if ((i & 0x0f) == 0x0f)
			pr_info("[%02x] %s\n", i - 0x0f, pLine);
	}
	pr_info("\ndown!\n");
	return 1;
}


/* dump all registers */
/* Usage: dumpall */
void anx7625_dumpall(void)
{
	unsigned char DevAddr;
	char DevAddrString[6+2+1];  /* (6+2) characters + NULL terminator*/
	char addr_string[] = {
		0x54, 0x58, 0x70, 0x72, 0x7a, 0x7e, 0x84 };

	for (DevAddr = 0; DevAddr < sizeof(addr_string); DevAddr++) {
		snprintf(DevAddrString, 3, "%02x", addr_string[DevAddr]);
		anx7625_dump_register(DevAddrString);
	}

}


ssize_t anx7625_erase_hex()
{

	MI2_power_on();

	command_erase_mainfw();

	return 1;
}

ssize_t anx7625_burn_hex()
{
	burnhex(0);

	return 1;
}


ssize_t anx7625_read_hex(const char *buf)

{
	int cmd, val;
	int result;

	result = sscanf(buf, "%x  %x", &cmd, &val);
	pr_info("readhex()\n");
	MI2_power_on();
	command_flash_read((unsigned int)cmd, (unsigned long)val);
	pr_info("\n");

	return 1;
}

ssize_t anx7625_dpcd_read(const char *buf)
{
	int addrh, addrm, addrl;
	int result;

	result = sscanf(buf, "%x %x %x", &addrh, &addrm, &addrl);
	if (result == 3) {
		unsigned char buff[2];

		sp_tx_aux_dpcdread_bytes(addrh, addrm, addrl, 1, buff);
		pr_info("aux_value = 0x%02x\n", (uint)buff[0]);
	} else {
		pr_info("input parameter error");
	}


	return 1;
}
ssize_t anx7625_dpcd_write(const char *buf)


{
	int addrh, addrm, addrl, val;
	unsigned char buff[16];
	int result;

	result = sscanf(buf, "%x  %x  %x  %x",
		&addrh, &addrm, &addrl, &val);
	if (result == 4) {
		buff[0] = (unsigned char)val;
		sp_tx_aux_dpcdwrite_bytes(
			addrh, addrm, addrl, 1, buff);
	} else {
		pr_info("error input parameter.");
	}
	return 1;
}

ssize_t anx7625_dump_edid()
{
	uint k, j;
	unsigned char   blocks_num;
	unsigned char edid_blocks[256*2];
	unsigned char  pLine[50];

	blocks_num = sp_tx_edid_read(edid_blocks);

	for (k = 0, j = 0; k < (128 * ((uint)blocks_num + 1)); k++) {
		if ((k&0x0f) == 0) {
			snprintf((char*)&pLine[j], 14, "edid:[%02hhx] %02hhx ",
				(uint)(k / 0x10), (uint)edid_blocks[k]);
			j = j + 13;
		} else {
			snprintf((char*)&pLine[j], 4, "%02hhx ", (uint)edid_blocks[k]);
			j = j + 3;

		}
		if ((k&0x0f) == 0x0f) {
			pr_info("%s\n", pLine);
			j = 0;
		}
	}

	return 1;

}

void anx7625_dpi_config(int table_id)
{
	command_DPI_Configuration(table_id);
}


void anx7625_dsi_config(int table_id)
{
	command_DSI_Configuration(table_id);

}

void anx7625_audio_config(int table_id)
{
	command_Configure_Audio_Input(table_id);

}
void anx7625_dsc_config(int table_id, int ratio)
{
	pr_info("dsc configure table id %d, dsi config:%d\n",
		(uint)table_id, (uint)ratio);
	if (ratio == 0)
		command_DPI_DSC_Configuration(table_id);
	else
		command_DSI_DSC_Configuration(table_id);

}

ssize_t anx7625_debug(const char *buf)
{
	int param[4];
	int result, i;
	char CommandName[20];

	memset(param, 0, sizeof(param));
	result = sscanf(buf, "%s %d %d %d %d", CommandName,
		param, param+1, param+2, param+3);
	pr_info("anx7625 cmd[%s", CommandName);
	for (i = 0; i < result - 1; i++)
		pr_info(" 0x%x", param[i]);
		pr_info("]\n");

	if (strcmp(CommandName, "poweron") == 0) {
		pr_info("MI2_power_on\n");
		MI2_power_on();
	} else if (strcmp(CommandName, "powerdown") == 0) {
		anx7625_power_standby();
	} else if (strcmp(CommandName, "debugon") == 0) {
		debug_on = 1;
		pr_info("debug_on = %d\n", debug_on);
	} else if (strcmp(CommandName, "debugoff") == 0) {
		debug_on = 0;
		pr_info("debug_on = %d\n", debug_on);
	} else if (strcmp(CommandName, "erasehex") == 0) {
		if ((param[0] == 0) && (param[1] == 0))
			command_erase_mainfw();/*erase main fw*/
		else
			/*erase number of sector from index*/
			command_erase_sector(param[0], param[1]);

	} else if (strcmp(CommandName, "burnhex") == 0) {
		debug_on = 1;
		MI2_power_on();
		if (param[0] == 0) { /*update main OCM*/
			command_erase_mainfw();
			burnhex(0);
		} else if (param[0] == 1) { /*update secure OCM*/
			command_erase_securefw();
			burnhex(1);
		} else
			pr_info("Unknown parameter for burnhex.");
		debug_on = 0;
	} else if (strcmp(CommandName, "readhex") == 0) {
		if ((param[0] == 0) && (param[1] == 0))
			command_flash_read(0x1000, 0x100);
		else
			command_flash_read(param[0], param[1]);
	} else if (strcmp(CommandName, "dpidsiaudio") == 0) {
		default_dpi_config = param[0];
		default_dsi_config = param[1];
		default_audio_config = param[2];
		pr_info("default dpi:%d, default dsi:%d, default audio:%d\n",
			default_dpi_config, default_dsi_config,
			default_audio_config);
	} else if (strcmp(CommandName, "dpi") == 0) {
		default_dpi_config = param[0];
		command_Mute_Video(1);
		anx7625_dpi_config(param[0]);
	} else if (strcmp(CommandName, "dsi") == 0) {
		default_dsi_config = param[0];
		command_Mute_Video(1);
		anx7625_dsi_config(param[0]);
	} else if (strcmp(CommandName, "dsi+") == 0) {
		ulong new_val;

		new_val = param[0]*1000 + param[1];
		reconfig_current_pclk(default_dsi_config, 1, new_val);
		command_Mute_Video(1);
		anx7625_dsi_config(default_dsi_config);
	} else if (strcmp(CommandName, "dsi-") == 0) {
		ulong new_val;

		new_val = param[0]*1000 + param[1];
		reconfig_current_pclk(default_dsi_config, 0, new_val);
		command_Mute_Video(1);
		anx7625_dsi_config(default_dsi_config);
	} else if (strcmp(CommandName, "audio") == 0) {
		default_audio_config = param[0];
		anx7625_audio_config(param[0]);
	} else if (strcmp(CommandName, "dsc") == 0) {
		/*default_dpi_config = param[0];*/
		command_Mute_Video(1);
		anx7625_dsc_config(param[0], param[1]);
	} else if (strcmp(CommandName, "show") == 0) {
		sp_tx_show_information();
	} else if (strcmp(CommandName, "dumpall") == 0) {
		anx7625_dumpall();
	} else if (strcmp(CommandName, "edid") == 0) {
		anx7625_dump_edid();
	} else if (strcmp(CommandName, "mute") == 0) {
		command_Mute_Video(param[0]);
	} else {
		pr_info("Usage:\n");
		pr_info("  echo poweron > cmd             :");
		pr_info("			power on\n");
		pr_info("  echo powerdown > cmd           :");
		pr_info("			power off\n");
		pr_info("  echo debugon > cmd             :");
		pr_info("		debug on\n");
		pr_info("  echo debugoff > cmd            :");
		pr_info("			debug off\n");
		pr_info("  echo erasehex > cmd            :");
		pr_info("			erase main fw\n");
		pr_info("  echo burnhex [index] > cmd     :");
		pr_info("	burn FW into flash[0:Main OCM][1:Secure]\n");
		pr_info("  echo readhex [addr] [cnt]> cmd :");
		pr_info("			read bytes from flash\n");
		pr_info("  echo dpi [index] > cmd         :");
		pr_info("			configure dpi with table[index]\n");
		pr_info("  echo dsi [index] > cmd         :");
		pr_info("			configure dsi with table[index]\n");
		pr_info("  echo audio [index] > cmd       :");
		pr_info("			configure audio with table[index]\n");
		pr_info("  echo dpidsiaudio [dpi] [dsi] [audio]> cmd  :\n");
		pr_info("		configure default dpi dsi audio");
		pr_info("			dpi/dsi/audio function.\n");
		pr_info("  echo dsc [index][flag] > cmd         :");
		pr_info("			configure dsc with [index]&[flag]\n");
		pr_info("  echo dpstart > cmd         :");
		pr_info("			Start DP process\n");
		pr_info("  echo show > cmd            :");
		pr_info("			Show DP result information\n");
		pr_info("  echo dumpall > cmd            :");
		pr_info("			Dump anx7625 all register\n");
	}

	return 1;
}

/* for debugging
static struct device_attribute anx7625_device_attrs[] = {
	__ATTR(pdcmd,    S_IWUSR, NULL, anx7625_send_pd_cmd),
	__ATTR(rdreg,    S_IWUSR, NULL, anx7625_rd_reg),
	__ATTR(wrreg,    S_IWUSR, NULL, anx7625_wr_reg),
	__ATTR(dumpreg,  S_IWUSR, NULL, anx7625_dump_register),
	__ATTR(prole,    S_IRUGO, anx7625_get_power_role, NULL),
	__ATTR(drole,    S_IRUGO, anx7625_get_data_role, NULL),
	__ATTR(pswap,    S_IRUGO, anx7625_send_pswap, NULL),
	__ATTR(dswap,    S_IRUGO, anx7625_send_dswap, NULL),
	__ATTR(dpcdr,    S_IWUSR, NULL, anx7625_dpcd_read),
	__ATTR(dpcdw,    S_IWUSR, NULL, anx7625_dpcd_write),
	__ATTR(dumpedid, S_IRUGO, anx7625_dump_edid, NULL),
	__ATTR(cmd,      S_IWUSR, NULL, anx7625_debug)
};
*/