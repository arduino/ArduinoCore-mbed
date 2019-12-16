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

#include "config.h"
#include "debug.h"
#include <string.h>
#include "REG_DRV.h"
#include "Flash/Flash.h"
 #include "MI2_REG.h"
#include  "anx7625_display.h"
#ifdef USE_PD30
#include "pd_ext_message.h"
#endif
#include  "CmdHandler.h"
#include "MI2.h"

#define TEST_IF_SEND_CMD 1 // interface send command test function

extern bit g_bDebug;
extern unsigned char xdata g_CmdLineBuf[CMD_LINE_SIZE];

extern unsigned char xdata force_pd_rev20;

bit g_bBurnHex;
bit g_bFlashWrite;

tagFlashRWinfo g_FlashRWinfo;

typedef unsigned char unchar;
typedef unsigned long ulong;
typedef unsigned int uint;


// basic commands
static void rd(void);
static void wr(void);
static void wait_aux_op_finish(unchar * err_flag);
static void sp_tx_rst_aux(void);
unchar sp_tx_aux_dpcdread_bytes(unchar addrh, unchar addrm, unchar addrl, unchar cCount, unchar *pBuf);
unchar sp_tx_aux_dpcdwrite_bytes(unchar addrh, unchar addrm, unchar addrl, unchar cCount, unchar *pBuf);
static void errchk(void);
static void dump(void);
static void poweron(void);
static void poweroff(void);
static void poweronsp(void); //for anx3625
static void vbuson(void);
static void vbusoff(void);
void edid_dump(void);
void check_edid_data(unchar *pblock_buf);
void GetSinkCapExt(void);
 void AP_TX_process(void);//for anx3625

#define AUX_ERR  1
#define AUX_OK   0

#define DPCD_LANE0_1_STATUS  0x02

#define DPCD_LANE_ALIGN_UD  0x04

#define MAX_BUF_CNT 16
unchar xdata bytebuf[16];
unchar xdata video_table_id = 3;  //initial video table id
unchar xdata mipi_output_mode = VIDEO_DSI;   
unchar xdata audio_table_id = AUDIO_I2S;  //initial audiotable id

unsigned char __i2c_read_byte(unsigned char dev,unsigned char offset)
{
	unsigned char temp;
	ReadReg(dev, offset, &temp);
	return temp;
}

#if 0
#define write_dpcd_addr(addrh, addrm, addrl) \
	do{ \
		unchar temp; \
		if (__i2c_read_byte(TX_P0, AUX_ADDR_7_0) != (unchar)addrl) \
			WriteReg(TX_P0, AUX_ADDR_7_0, (unchar)addrl); \
			if (__i2c_read_byte(TX_P0, AUX_ADDR_15_8) != (unchar)addrm) \
			WriteReg(TX_P0, AUX_ADDR_15_8, (unchar)addrm); \
		ReadReg(TX_P0, AUX_ADDR_19_16, &temp); \
		if ((unchar)(temp & 0x0F)  != ((unchar)addrh & 0x0F)) \
			WriteReg(TX_P0, AUX_ADDR_19_16, (temp  & 0xF0) | ((unchar)addrh)); \
	}while(0)
#else


#define write_dpcd_addr(addrh, addrm, addrl) \
	do{ \
		unchar temp; \
		if (__i2c_read_byte(RX_P0, AP_AUX_ADDR_7_0) != (unchar)addrl) \
			WriteReg(RX_P0, AP_AUX_ADDR_7_0, (unchar)addrl); \
			if (__i2c_read_byte(RX_P0, AP_AUX_ADDR_15_8) != (unchar)addrm) \
			WriteReg(RX_P0, AP_AUX_ADDR_15_8, (unchar)addrm); \
		ReadReg(RX_P0, AP_AUX_ADDR_19_16, &temp); \
		if ((unchar)(temp & 0x0F)  != ((unchar)addrh & 0x0F)) \
			WriteReg(RX_P0, AP_AUX_ADDR_19_16, (temp  & 0xF0) | ((unchar)addrh)); \
	}while(0)
#endif

#define sp_tx_addronly_set(enable) \
	reg_bit_ctl(TX_P0, AUX_CTRL2, ADDR_ONLY_BIT, enable)
#define reg_bit_ctl(addr, offset, data, enable) \
	do{ \
		unchar c; \
		ReadReg(addr, offset, &c); \
		if (enable) { \
			if ((c & data) != data){ \
				c |= data; \
				WriteReg(addr, offset, c); \
			} \
		} else { \
			if ((c & data) == data){ \
				c &= ~data; \
				WriteReg(addr, offset, c); \
			} \
		} \
	}while(0)


/* modify a character string, changes all letters to lower case */
void MakeLower(unsigned char *p)
{
    for(; *p; p++)
    {
        if (*p >= 'A' && *p <= 'Z')
        {
            *p += 0x20;
        }
    }
}
#if TEST_IF_SEND_CMD
#include "common/private_interface.h"
#include "common/public_interface.h"
#define PD_VOLTAGE_5V 5000
#define PD_VOLTAGE_6V 6000
#define PD_VOLTAGE_9V 9000
#define PD_VOLTAGE_12V 12000
#define PD_VOLTAGE_15V 15000
#define PD_VOLTAGE_20V 20000
#define PD_CURRENT_5A 5000
#define PD_VOLTAGE_21V 21000
/*
  * example 1: udpate power's source capability from AP to ohio
  *
  */
unsigned char update_pwr_src_caps(void)
{
	/*
	  *example 1: source capability for customer's reference,
	  *you can change it using PDO_XXX() macro easily
	  */
	unsigned long IF_RAM new_src_cap[7] = {
			/*5V, 1.5A, Fixed*/
			PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_1500MA, PDO_FIXED_DATA_SWAP),
			/*5V, 0.1A, Fixed*/
			PDO_FIXED(PD_VOLTAGE_6V, PD_CURRENT_100MA, 0),
			PDO_FIXED(PD_VOLTAGE_12V, PD_CURRENT_3A, 0),
			PDO_FIXED(PD_VOLTAGE_20V, PD_CURRENT_5A, 0),
			PDO_VAR(PD_VOLTAGE_5V, PD_VOLTAGE_20V, PD_CURRENT_3A),
			PDO_VAR(PD_VOLTAGE_5V, PD_VOLTAGE_21V, PD_CURRENT_3A),
			PDO_BATT(PD_VOLTAGE_5V, PD_VOLTAGE_21V, PD_POWER_15W)
		   #if 0
			/*min 5V, max 20V, power 15W, battery*/
				PDO_BATT(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_POWER_15W),
			/*min5V, max 5V, current 3A, variable*/
				PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_CURRENT_3A)
			#endif
		};
#ifdef USE_PD30
#ifdef USE_POWER_RULE
	// PDP of x Watts, 1 for 0.5w
	#define PDP_WATTS_100W 200 // 200 x 0.5W = 100W
   	new_src_cap[0] = PDO_FIXED(PD_VOLTAGE_5V, pd30_src_pwr_rules_ma(PDP_WATTS_100W, 5), PDO_FIXED_DATA_SWAP);
   	new_src_cap[1] = PDO_FIXED(PD_VOLTAGE_9V, pd30_src_pwr_rules_ma(PDP_WATTS_100W, 9), 0);
   	new_src_cap[2] = PDO_FIXED(PD_VOLTAGE_15V, pd30_src_pwr_rules_ma(PDP_WATTS_100W, 15), 0);
   	new_src_cap[3] = PDO_FIXED(PD_VOLTAGE_20V, pd30_src_pwr_rules_ma(PDP_WATTS_100W, 20), 0);
#endif
#endif

	TRACE("update_pwr_src_caps\n");
	/* send source capability from AP to ohio. */
	return send_pd_msg(TYPE_PWR_SRC_CAP, (const unsigned char *)new_src_cap, sizeof(new_src_cap));
}

/*
  * example 2: send sink capability from AP to ohio
  *
  */
unsigned char update_pwr_sink_caps(void){
	/*
	  *example 2: sink power capability for customer's reference,
	  *you can change it using PDO_XXX() macro easily
	  */
	unsigned long IF_RAM new_snk_cap[2] = {
		   	/*5V, 0.9A, Fixed*/
		   	PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_900MA, PDO_FIXED_FLAGS),
		   	/*5V, 3A, Fixed*/
		   	PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_3A, PDO_FIXED_FLAGS)
		   #if 0
		 	/*min 5V, max 20V, power 15W, battery*/
		    	PDO_BATT(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_POWER_15W),
			/*min5V, max 5V, current 3A, variable*/
		    	PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_CURRENT_3A)
		    #endif
		};

	TRACE("update_pwr_sink_caps\n");
	/* send source capability from AP to ohio. */
	return send_pd_msg(TYPE_PWR_SNK_CAP, (const unsigned char *)new_snk_cap, sizeof(new_snk_cap));
}

unsigned char update_dp_snk_identity(void){
	//new setting for TYPE_DP_SNK_IDENDTITY
	unsigned char IF_RAM new_dp_snk_identity[16] = {0};
	unsigned char IF_RAM new_sink_id_header[PD_ONE_DATA_OBJECT_SIZE] = {0x00, 0x00, 0x00, 0x6c};
	unsigned char IF_RAM new_sink_cert_stat_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x00, 0x00, 0x00, 0x00};
	unsigned char IF_RAM new_sink_prd_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x58, 0x01, 0x13, 0x10};
	unsigned char IF_RAM new_sink_ama_vdo[PD_ONE_DATA_OBJECT_SIZE] = {0x39, 0x00, 0x00, 0x51};

	//send TYPE_DP_SNK_IDENDTITY new setting
	memcpy(new_dp_snk_identity, new_sink_id_header, 4);
   	memcpy(new_dp_snk_identity + 4, new_sink_cert_stat_vdo, 4);
   	memcpy(new_dp_snk_identity+ 8, new_sink_prd_vdo, 4);
   	memcpy(new_dp_snk_identity + 12, new_sink_ama_vdo, 4);
	return send_pd_msg(TYPE_DP_SNK_IDENDTITY, (const unsigned char *)new_dp_snk_identity, sizeof(new_dp_snk_identity));
}
unsigned char update_svid()
{
	const unsigned char IF_RAM init_svid[PD_ONE_DATA_OBJECT_SIZE]= {0x00, 0xff, 0x01, 0xff};//{0x00, 0x00, 0x01, 0xff};


		//send TYPE_SVID init setting
	return send_pd_msg(TYPE_SVID, init_svid, sizeof(init_svid));
}

/*
  * example 3: send VDM  from AP to ohio
  *
  */
unsigned char update_VDM(void)
{

	unsigned char IF_RAM vdm[] = {
	   	0x00,0x00,0x01,0x00, 0x43, 0x45,0x54,0x056
	};

	TRACE("update_vdm\n");
	/* send sink capability from AP to ohio. */
	return send_pd_msg(TYPE_VDM, (const unsigned char *)vdm, sizeof(vdm));
}

unsigned char update_sop_prime()
{
	const unsigned char IF_RAM sop_prime[] = {
	   	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
		0x09,0x10,0x11,0x12, 0x13, 0x14,0x15,0x16,
		0x17,0x18,0x19,0x20, 0x21, 0x22,0x23,0x24,
		0x25,0x26,0x27,0x28
	};

	TRACE("update sop prime\n");
	return send_pd_msg(TYPE_SOP_PRIME, sop_prime, sizeof(sop_prime));
}

unsigned char update_sop_double_prime()
{
	const unsigned char IF_RAM sop_double_prime[] = {
	   	0x0AA,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
		0x09,0x10,0x11,0x12, 0x13, 0x14,0x15,0x16,
		0x17,0x18,0x19,0x20, 0x21, 0x22,0x23,0x24,
		0x25,0x26,0x27,0x28
	};

	TRACE("update sop_double_prime\n");
	return send_pd_msg(TYPE_SOP_DOUBLE_PRIME,sop_double_prime, sizeof(sop_double_prime));
}

unsigned char update_dp_sink_cfg()
{
	const unsigned char IF_RAM dcap[] = { 0x06, 0x08,0x08, 0x00 };
	TRACE("update_dp_sink_cfg\n");
	return send_pd_msg(TYPE_DP_SNK_CFG, dcap, sizeof(dcap));
}

unsigned char update_pwr_obj_req()
{
	const unsigned char IF_RAM rdo[] = { 0x0A,0x78,0x00,0x10 };
	TRACE("update_pwr_obj_req\n");
	return send_pd_msg(TYPE_PWR_OBJ_REQ, rdo, sizeof(rdo));

}

unsigned char send_process()
{
	unsigned char type;
	unsigned char rst = CMD_FAIL;

	if(sscanf((const char*)g_CmdLineBuf, "\\%*s %bx", &type) == 1)
       {

	TRACE1("type %2BX\n", type);

	switch (type) {
		case TYPE_PWR_SRC_CAP:
			//rst = send_src_cap(buf, size);//send 0
			rst = update_pwr_src_caps();
			break;

		case TYPE_PWR_SNK_CAP:
			//rst = send_snk_cap(buf, size);//send 1
			rst = update_pwr_sink_caps();
			//buf[0] = 0x96;
			//buf[1] = 0x90;
			//buf[2] = 0x01;
			//buf[3] = 0x00;//{0x96, 0x90, 0x01, 0x00};//Fixed 5V, 1.5A.
			//size = 4;
			//send_pd_msg( TYPE_PWR_SNK_CAP, buf, 4);
			break;
		case TYPE_DP_SNK_IDENDTITY://send 2
			  rst = update_dp_snk_identity();//send_pd_msg(TYPE_DP_SNK_IDENDTITY, NULL, 0);//send 2
			break;
		case TYPE_SVID:
			rst = update_svid();
			break;
		case TYPE_GET_DP_SNK_CAP:
			rst = send_pd_msg(TYPE_GET_DP_SNK_CAP, NULL, 0);//send 4
			break;


		case TYPE_PSWAP_REQ://send 10
			rst = send_pd_msg(TYPE_PSWAP_REQ, NULL, 0);
			//wait_cmd_response_time = 200; //ms
			break;

		case TYPE_DSWAP_REQ://send 11
			rst = send_pd_msg(TYPE_DSWAP_REQ, NULL, 0);
		//wait_cmd_response_time = 200; //ms
			break;

		case TYPE_GOTO_MIN_REQ://send 12
			rst = send_pd_msg(TYPE_GOTO_MIN_REQ, NULL, 0);
			break;

		case TYPE_VCONN_SWAP_REQ: //send 13
			rst = send_pd_msg(TYPE_VCONN_SWAP_REQ, NULL, 0);
			break;

		case TYPE_VDM://send 14
			//send_vdm(buf, size);
			rst = update_VDM();
			break;
		case TYPE_DP_SNK_CFG://send 15
			//send_dp_snk_cfg(buf, size);
			rst = update_dp_sink_cfg();
			break;


		case TYPE_PD_STATUS_REQ://send 17
			rst = send_pd_msg(TYPE_PD_STATUS_REQ, NULL, 0);
			break;
		case TYPE_PWR_OBJ_REQ://send 16
			 //send_rdo(buf, size);
			rst = update_pwr_obj_req();
			 break;
		case TYPE_GET_SNK_CAP:// send 1b
			rst = send_get_sink_cap();
			break;

		case TYPE_SOP_PRIME://send 1c
			rst = update_sop_prime();
			break;
		case TYPE_SOP_DOUBLE_PRIME://send 1d
			rst = update_sop_double_prime();
			break;

		case TYPE_ACCEPT://send 5
			rst = send_pd_msg(TYPE_ACCEPT, NULL, 0);
			break;

	       case TYPE_REJECT://send 6
			rst = send_pd_msg(TYPE_REJECT, NULL, 0);
			break;

#ifdef USE_PD30
		case TYPE_EXT_GET_BATT_CAP://send a3
	     	{
			 unsigned char index;
			 if(sscanf(g_CmdLineBuf, "\\%*s %*bx %bx", &index) == 1)
				rst = send_pd_msg(TYPE_EXT_GET_BATT_CAP, &index, 0);
			}
			break;
		case TYPE_EXT_GET_BATT_STS://send a4
	     	{
			 unsigned char index;
			 if(sscanf(g_CmdLineBuf, "\\%*s %*bx %bx", &index) == 1)
				rst = send_pd_msg(TYPE_EXT_GET_BATT_STS, &index, 0);
			}
			break;
		case TYPE_EXT_GET_MFR_INFO://send a6
			rst = send_pd_msg(TYPE_EXT_GET_MFR_INFO, NULL, 0);
			break;
		case TYPE_EXT_ALERT://send ab
			rst = send_pd_msg(TYPE_EXT_ALERT, NULL, 0);
			break;
		case TYPE_EXT_GET_SRC_CAP://send ad
			rst = send_pd_msg(TYPE_EXT_GET_SRC_CAP, NULL, 0);
			break;
		case TYPE_EXT_GET_SRC_STS://send ae
			rst = send_pd_msg(TYPE_EXT_GET_SRC_STS, NULL, 0);
			break;
		case TYPE_FR_SWAP_SIGNAL://send b0
			rst = send_pd_msg(TYPE_FR_SWAP_SIGNAL, NULL, 0);
		case TYPE_GET_SINK_CAP_EXT://send b1
			rst = send_pd_msg(TYPE_GET_SINK_CAP_EXT, NULL, 0);
			break;
#endif

		case TYPE_SOFT_RST://send f1
			rst = send_soft_rst();//interface_send_soft_rst();
			//wait_cmd_response_time = 1;
			break;
		case TYPE_HARD_RST://send f2
			 rst = send_hard_rst();//interface_send_hard_rst();
			break;
#if 1 // debug
		case TYPE_GET_VAR://send fc
	     	{
			 unsigned int addr;
			 unsigned char mem_type;
			 unsigned char len;
			 if(sscanf((const char*)g_CmdLineBuf, "\\%*s %*bx %bx %x %bx", &mem_type, &addr, &len) == 3)
			 	{
				 if((mem_type >= IF_VAR_fw_var_reg && mem_type <= IF_VAR_sink_identity)) // REG_FW_VAR...
				 {
					 rst = send_get_var(mem_type,addr,len);
				 }
			 	}
			}
			break;
		case TYPE_SET_VAR://send fd
			{
			 unsigned int addr;
			 unsigned char mem_type;
			 unsigned char len;
			 unsigned char buf[4];

			 len = sscanf((const char*)g_CmdLineBuf, "\\%*s %*bx %bx %x", &mem_type, &addr);
			 if(len < 2)
			 	break;
			 len = sscanf((const char*)g_CmdLineBuf, "\\%*s %*bx %*bx %*x %bx %bx %bx %bx", &buf[0], &buf[1], &buf[2], &buf[3]);
			 TRACE1("len = %bd\n",len);
			 if(len)
				{
				 if((mem_type >= IF_VAR_fw_var_reg && mem_type <= IF_VAR_sink_identity)) // REG_FW_VAR...
				 {
					 if(mem_type == 0) // idata
					 {
						 TRACE1("write 0x%x idata = ",addr);
						 TRACE_ARRAY(buf,len);
					 }
					 else if(mem_type == 1) // xdata
					 {
						 TRACE1("write 0x%x xdata = ",addr);
						 TRACE_ARRAY(buf,len);
					 }
					 else if(mem_type == IF_VAR_fw_var_reg) // REG_FW_VAR
					 {
						 TRACE1("write REG_FW_VAR[0x%x] = ",addr);
						 TRACE_ARRAY(buf,len);
					 }
					 else
					 {
						 TRACE2("write type 0x%bx, offset 0x%x = ",mem_type,addr);
						 TRACE_ARRAY(buf,len);
					 }
					 rst = send_set_var(mem_type,addr,buf,len);
				 }
				}
			}
			break;
#endif
		default:
			TRACE1("unknown type %2BX\n", type);

			break;
	}
	 }
    else
    {
        TRACE("Bad parameter!\n");
    }
	return rst;
}
#endif
	    
/* command handler */
void CmdHandler(void)
{
    unsigned char xdata CommandName[CMD_NAME_SIZE];
    unsigned char c, addrh,addrm,addrl;

    if (g_bFlashWrite)
    {
        flash_program();
    }
    else
    {
        if (sscanf((const char*)g_CmdLineBuf, "\\%15s", CommandName) == 1)
        {
            MakeLower(CommandName);
            if (strcmp((const char*)CommandName, "rd") == 0)
            {
                rd();
            }
            else if (strcmp((const char*)CommandName, "wr") == 0)
            {
                wr();
            }
            else if (strcmp((const char*)CommandName, "dpcdw") == 0)
            {
                if (sscanf((const char*)g_CmdLineBuf,"\\%*s %bx %bx %bx %bx", &addrh, &addrm, &addrl, &c)==4)
                {
                    sp_tx_aux_dpcdwrite_bytes(addrh, addrm, addrl, 1, &c);
                }
            }
            else if (strcmp((const char*)CommandName, "dpcdr") == 0)
            {
                if (sscanf((const char*)g_CmdLineBuf,"\\%*s %bx %bx %bx %bx", &addrh, &addrm, &addrl, &c)==4)
                {
                	unsigned char xdata buff[16];
                    if (c >16)
                        c =1;
                    sp_tx_aux_dpcdread_bytes(addrh, addrm, addrl, c, buff);
                }
            }
			else if (strcmp((const char*)CommandName, "errchk") == 0)
            {
                errchk();
            }
			else if (strcmp((const char*)CommandName, "show") == 0)
            {
                sp_tx_show_infomation();
            }
            else if (strcmp((const char*)CommandName, "dump") == 0)
            {
                dump();
            }
            else if (strcmp((const char*)CommandName, "poweron") == 0)
            {
                poweron();
            }
            else if (strcmp((const char*)CommandName, "poweroff") == 0)
            {
                poweroff();
            }
	#ifdef ANX3625_Support
		else if (strcmp((const char*)CommandName, "sp") == 0)
            {
                poweronsp();
            }
	#endif
           else if (strcmp((const char*)CommandName, "reset") == 0)
            {
                reset_MI2();
            } 
            else if (strcmp((const char*)CommandName, "vbuson") == 0)
            {
                vbuson();
            }
            else if (strcmp((const char*)CommandName, "vbusoff") == 0)
            {
                vbusoff();
            } 
            else if (strcmp((const char*)CommandName, "debugon") == 0)
            {
                g_bDebug = 1;
                TRACE("debug ON mode\n");
            }
            else if (strcmp((const char*)CommandName, "debugoff") == 0)
            { 
                g_bDebug = 0;
                TRACE("debug OFF mode\n");
            }
            else if (strcmp((const char*)CommandName, "fl_se") == 0)
            {
                command_flash_SE();
            }
            else if (strcmp((const char*)CommandName, "fl_ce") == 0)
            {
                command_flash_CE();
            }
            else if (strcmp((const char*)CommandName, "erase") == 0)
            {
                poweron();
                command_erase_FW(MAIN_OCM);
            }
            else if (strcmp((const char*)CommandName, "burnhex") == 0)
            {
                poweron();
                burn_hex_prepare();
            }
            else if (strcmp((const char*)CommandName, "readhex") == 0)
            {
                poweron();
                command_flash_read();
            }
	#if 0
            else if (strcmp(CommandName, "dpi") == 0)
            {

            	if (sscanf(g_CmdLineBuf, "\\%*s %bx", &video_table_id) == 1)
                {
                    TRACE1("DPI input configure table id %d\n", (uint)video_table_id);
			mipi_output_mode = 0; 
                    //ODFC power off
                    //sp_write_reg_or(TCPC_INTERFACE, CHIP_POWER_CTRL, (1 << PD_V10_ODFC));
                    //DSC disable
                    sp_write_reg_and(RX_P0, R_DSC_CTRL_0, ~DSC_EN);
                    API_Custom_Register0_Configuration(video_table_id);
                    API_DPI_Configuration(video_table_id);
            	}
                else
                {
                    TRACE("Bad parameter! Usage:\n");
                    TRACE("dpi <table_id>\n\n");
                }
            }
            else if (strcmp(CommandName, "dpi+") == 0)
            {

                if (sscanf(g_CmdLineBuf, "\\%*s %bx", &video_table_id) == 1)
                {
                	mipi_output_mode = 1; 
                    TRACE1("DPI +DSC input configure table id %d\n", (uint)video_table_id);
                    API_DSC_Configuration(video_table_id);
                    API_ODFC_Configuration(video_table_id);
                    API_Custom_Register0_Configuration(video_table_id);
                    API_DPI_Configuration(video_table_id);
                }
                else
                {
                    TRACE("Bad parameter! Usage:\n");
                    TRACE("dpi+ <table_id>\n\n");
                }
            }
            else if (strcmp(CommandName, "dsi") == 0)
            {

            	if (sscanf(g_CmdLineBuf, "\\%*s %bx", &video_table_id) == 1)
                {
                	mipi_output_mode = 2; 
                    TRACE1("DSI input configure table id %d\n", (uint)video_table_id);
                    //DSC disable
                    sp_write_reg_and(RX_P0, R_DSC_CTRL_0, ~DSC_EN);
                    API_DSI_Configuration(video_table_id);
                }
                else
                {
                    TRACE("Bad parameter! Usage:\n");
                    TRACE("dsi <table_id>\n\n");
                }
            }
            else if (strcmp(CommandName, "dsi+") == 0)
            {

                if (sscanf(g_CmdLineBuf, "\\%*s %bx", &video_table_id) == 1)
                {
                	mipi_output_mode = 3; 
                    TRACE1("DSI +DSC input configure table id %d\n", (uint)video_table_id);
                    API_DSC_Configuration(video_table_id);
                    API_DSI_Configuration(video_table_id);
                }
                else
                {
                    TRACE("Bad parameter! Usage:\n");
                    TRACE("dsi+ <table_id>\n\n");
                }
            }
	#endif
	  else if (strcmp((const char*)CommandName, "sleep") == 0)
            {
                 sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_DISABLE_DISPLAY);  //disable
            }
	    else if (strcmp((const char*)CommandName, "wake") == 0)
            {
                 sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_DISABLE_DISPLAY);  //clear 
            }
            else if (strcmp((const char*)CommandName, "audio") == 0)
            {
                unsigned char xdata table_id;

                if (sscanf((const char*)g_CmdLineBuf, "\\%*s %bx", &table_id) == 1)
                {
                    TRACE1("Audio input configure table id %d\n", (uint)table_id);
			audio_table_id = table_id;
                    API_Configure_Audio_Input(table_id);
                }
                else
                {
                    TRACE("Bad parameter! Usage:\n");
                    TRACE("audio <table_id>\n\n");
                }
            }
	  else if (strcmp((const char*)CommandName, "mute") == 0)
            {
            	 unsigned char xdata status;

            	if (sscanf((const char*)g_CmdLineBuf, "\\%*s %bx", &status) == 1)
                {
                    TRACE1("mute status %d\n", (uint)status);
                    API_Video_Mute_Control(status);
                }
                else
                {
                    TRACE("Bad parameter! Usage:\n");
                }
            }
	   else if (strcmp((const char*)CommandName, "vc") == 0)
            {
              TRACE("video changed\n");
		 if (sscanf((const char*)g_CmdLineBuf, "\\%*s %bx %bx", &mipi_output_mode, &video_table_id) == 2)
                {
                    TRACE2("video mode =  %d, video table id =  %d\n", (uint)mipi_output_mode, (uint)video_table_id);
			 //clear mipi RX en
			sp_write_reg_and(RX_P0, AP_AV_STATUS, ~AP_MIPI_RX_EN);  
                     //video changed flag
			sp_write_reg_or(RX_P0, AP_AV_STATUS, AP_VIDEO_CHG);

								
		 
			
                }
                else
                {
                    TRACE("Bad parameter! Usage:\n");
                }
            }
	    else if (strcmp((const char*)CommandName, "dumpedid") == 0)
            {
                 edid_dump();
            }
        /*  else if (strcmp(CommandName, "load0") == 0)
            {
               unsigned char c;
                TRACE("load secure ocm flash\n");
                flash_section_loader(0x52, HDCP22TXFW_START_ADDR, HDCP22TXFW_SIZE);
		 delay_ms(100);
		  WriteReg(TX_P0, 0x80, 0x10); //enable secure ocm
		  ReadReg(RX_P0,R_RAM_CTRL, &c);
		  TRACE1("load  status %x\n", (uint)c);

            }
	   else if (strcmp(CommandName, "load1") == 0)
            {
               unsigned char c;
                TRACE("load HDCP key  flash\n");
		  flash_section_loader(0x12, HDCP14KEY_START_ADDR, HDCP14KEY_SIZE);
  		  delay_ms(100);
		   ReadReg(RX_P0,R_RAM_CTRL, &c);
		    TRACE1("load  status %x\n", (uint)c);
            }
	    else if (strcmp(CommandName, "cal") == 0)
	   {
			//debugon mode
			g_bDebug = 1;
			//Full power on chip
			ANX7625_POWER_ON();
			delay_ms(10);
			ANX7625_RESET_RELEASE();
			delay_ms(10);
			WriteReg(0x58, 0xb6, 0x0d);
			delay_ms(10);
			WriteReg(0x58, 0xb6, 0x0f);
			WriteReg(0x7e, 0x80, 0x85);
			delay_ms(10);
			WriteReg(0x72, 0x05, 0x00);
			WriteReg(0x84, 0x0f, 0x00);
			//Reset OCM
			WriteReg(0x7e, 0x88,  0x40); //reset main ocm
			WriteReg(0x70, 0x80,  0x00); //reset secure ocm
			TRACE("Calibration mode:\n");
			TRACE("¢Ù RD ---Add precisely 4.7K-3.3V Rp on CC1 and measure voltage on CC1. Change 0x58:0xa6 bit[4:0] to set V_CC1 as closer as 1.717V. Record register value as Fuse_Rd\n\n");
			WriteReg(0x58, 0xbe, 0x05);
			WriteReg(0x58, 0xbf, 0x11);
			WriteReg(0x58, 0xa9, 0x34);
			TRACE("¢Ú ADC---Measure voltage on SBU2, Change 0x58:0xc0 bit[4:0] to set V_SBU2 as closer as 1.6V. Record reigster value as Fuse_ADC\n\n");
			TRACE("¢Û Use command fusew +Fuse_Rd +Fuse_ADC to write Fuse data\n");
		}
		else if (strcmp(CommandName, "fusew") == 0)
		{
			if (sscanf(g_CmdLineBuf,"\\%*s %bx %bx",&addrh,&addrl)==2)
			{
				//Write fuse data to buffer
				WriteReg(0x7e, 0x0a, addrh);
				WriteReg(0x7e, 0x0c, addrl);
				//Send flash write enable and write fuse_write
				WriteReg(0x7e, 0x36, 0x06);
				WriteReg(0x7e, 0x33, 0x40);
				WriteReg(0x7e, 0x00, 0x0a);
				ReadReg(0x7e, 0x00, &addrm);
				while((addrm&0x04)==0x04)
				{
					ReadReg(0x7e, 0x00, &addrm);
				}
				TRACE("Fuse data write done\n");
			}
		}*/
		 else if(strcmp((const char*)CommandName, "usepd2") == 0)
		{
			unsigned char xdata flag;
			flag = force_pd_rev20;
			sscanf((const char*)g_CmdLineBuf, "\\%*s %bx", &flag);
			force_pd_rev20 = flag;
			if(force_pd_rev20)
				TRACE(" USE PD2.0 when next time chip power on.\n");
			else
				TRACE(" USE default PD version.\n");
		}
#if TEST_IF_SEND_CMD
		 else if(strcmp((const char*)CommandName, "send") == 0)
		{
			TRACE1(" send result : %2BX \n", send_process());
		}
	    else if (strcmp((const char*)CommandName, "iftest") == 0)
            {
			unsigned char interface_send_msg_test(void);
	        interface_send_msg_test();
			TRACE("interface_send_msg_test()\n");
            }
#ifdef USE_PD30
		else if(strcmp((const char*)CommandName, "pd3test") == 0)
		{
			unsigned char xdata flag;
			unsigned char xdata type;
			char xdata res;

			res = sscanf((const char*)g_CmdLineBuf, "\\%*s %bx %bx", &flag, &type);

			if(res <= 0)
				flag = 0xFF;
			else if(res == 1)
				type = 0;

			if(flag == 0x00)
				{
         #ifdef USE_POWER_RULE
				 TRACE("Send PD3.0 Power rule src caps.\n");
				 update_pwr_src_caps();
		 #endif
				}
			else if(flag == 0x01)
				{
				TRACE1("Send PD3.0 Get battery status(%bx).\n",type);
				 GetBatteryStatus(type);
				}
			else if(flag == 0x02)
				{
				TRACE("Send PD3.0 Get source cap extended.\n");
				 GetSrcCapExt();
				}
			else if(flag == 0x03)
				{
				TRACE("Send PD3.0 Get status.\n");
				 GetStatus();
				}
			else if(flag == 0x04)
				{
				TRACE("Send PD3.0 Alert.\n");
				 StatusAlert();
				}
			else if(flag == 0x05)
				{
				TRACE1("Send PD3.0 Get battery cap(%bx).\n",type);
				 GetBatteryCap(type);
				}
			else if(flag == 0x06)
				{
				TRACE("Send PD3.0 Get Manufacturer Info.\n");
				 GetManufacturerInfo();
				}
			else if(flag == 0x07)
				{
		#if USE_PDFU
				extern unsigned int PDFU_RAM FwImageSizeMax;
				unsigned int size;
				sscanf((const char*)g_CmdLineBuf, "\\%*s %*bx %d", &size);
				if(size)
				{
					FwImageSizeMax = size;
				}
				TRACE1("FwImageSizeMax = %d\n",FwImageSizeMax);
				pdfu_initiator_start();
		#endif
				}
			else if(flag == 0x08)
				{
#if USE_PDFU
				if(type == 0)
				{
					pdfu_initiator_resume();
				}
				else if(type == 1)
				{
					pdfu_initiator_pause();
				}
#endif
				}
			else if(flag == 0x09)
				{
					if(type == 1)
					{
						TRACE("Send PD3.0 FR_Swap.\n");
						 SendFRSwap();
					}
					else
					{
						TRACE("Send Fast Role Swap signal.\n");
						 send_frswap_signal();
					}
				}
			else if(flag == 0x10)
				{
					TRACE1("TraceDebugFlag = 0x%bx\n",type);
					TraceDebug(type);
				}
			else if(flag == 0x11)
				{
				extern unsigned char IF_RAM InterfaceSendRetryCount;
					TRACE1("InterfaceSendRetryCount = 0x%bx\n",type);
					InterfaceSendRetryCount = type;
				}
			else if(flag == 0x12)
				{
				
				void PrintRecvPdfuData(unsigned char type);
					TRACE1("PrintRecvPdfuData(0x%bx)\n",type);
					PrintRecvPdfuData(type);
				}
		       else if(flag == 0x13)
				{
				TRACE("Send PD3.0 Get sink cap extended.\n");
				 GetSinkCapExt();
				}
			else
				{
				TRACE("Usage:\n");
				TRACE("0 : PD3.0 Power rule source caps.\n");
				TRACE("1 : PD3.0 Get battery status. <0|1|2|>\n");
				TRACE("2 : PD3.0 Get source cap extended.\n");
				TRACE("3 : PD3.0 Get status.\n");
				TRACE("4 : PD3.0 Alert.\n");
				TRACE("5 : PD3.0 Get battery cap. <0|1|2|>\n");
				TRACE("6 : PD3.0 Get Manufacturer Info.\n");
#if USE_PDFU
				TRACE("7 : PD3.0 Firmware update demo. <0:default size|other:fix size>\n");
				TRACE("8 : PD3.0 Firmware update ctrl. <0:resume|1:pause>\n");
#endif
				TRACE("9 : PD3.0 Fast Role Swap <0:FRSwap signal|1:FR_Swap msg>\n");
				}

		}
#endif
#endif
            else
            {
                TRACE("Unsupported command!\n\n");
            }
        }
    }
}

/* read a register */
/* Usage: rd <Device_Addr> <Register_Addr> */
static void rd(void)
{
    unsigned char DevAddr;
    unsigned char RegAddr;
    unsigned char RegData;
    char RetVal;

    if (sscanf((const char*)g_CmdLineBuf, "\\%*s %bx %bx", &DevAddr, &RegAddr) == 2)
    {
        RetVal = ReadReg(DevAddr, RegAddr, &RegData);
        if (RetVal == 0)
        {
            TRACE3("%02BX:%02BX=%02BX\n", DevAddr, RegAddr, RegData);
        }
        else
        {
            TRACE2("%s(%u): NAK!\n", __FILE__, (unsigned int)__LINE__);
        }
    }
    else
    {
        TRACE("Bad parameter! Usage:\n");
        TRACE("rd <Device_Addr> <Register_Addr>\n");
    }
}


/* write a register */
/* Usage: wr <Device_Addr> <Register_Addr> <value_to_be_written> */
static void wr(void)
{
    unsigned char DevAddr;
    unsigned char RegAddr;
    unsigned char RegData;
    char RetVal;

    if (sscanf((const char*)g_CmdLineBuf, "\\%*s %bx %bx %bx", &DevAddr, &RegAddr, &RegData) == 3)
    {
        RetVal = WriteReg(DevAddr, RegAddr, RegData);
        if (RetVal == 0)
        {
            RetVal = ReadReg(DevAddr, RegAddr, &RegData);
            if (RetVal == 0)
            {
                TRACE3("read back %02BX:%02BX=%02BX\n", DevAddr, RegAddr, RegData);
            }
            else
            {
                TRACE2("%s(%u): NAK!\n", __FILE__, (unsigned int)__LINE__);
            }
        }
        else
        {
            TRACE2("%s(%u): NAK!\n", __FILE__, (unsigned int)__LINE__);
        }
    }
    else
    {
        TRACE("Bad parameter! Usage:\n");
        TRACE("wr <Device_Addr> <Register_Addr> <value_to_be_written>\n");
    }
}

/* dump all registers in a specified I2C address (256 registers) */
/* Usage: dump <Device_Addr> */
static void dump(void)
{
    unsigned char DevAddr;
    unsigned char i;
    unsigned char buf[16];
    char RetVal;

    if (sscanf((const char*)g_CmdLineBuf, "\\%*s %bx", &DevAddr) == 1)
    {
        TRACE("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        i = 0;
        do
        {
            RetVal = ReadBlockReg(DevAddr, i, 16, buf);
            if (RetVal == 0)
            {
                TRACE1("[%02BX]:", i);
                TRACE_ARRAY(buf, 16);
                i += 16;
            }
            else
            {
                TRACE2("%s(%u): NAK!\n", __FILE__, (unsigned int)__LINE__);
                break;
            }
        }while(i != 0);
    }
    else
    {
        TRACE("Bad parameter! Usage:\n");
        TRACE("dump <Device_Addr>\n");
    }
}

#if 0
static unchar sp_tx_aux_dpcdread_bytes(unchar addrh, unchar addrm,
	unchar addrl, unchar cCount, unchar *pBuf)
{
	unchar c,c1, i;
	unchar bOK;
	WriteReg(TX_P0, BUF_DATA_COUNT, 0x80);//clear buffer
	//command and length
	c = ((cCount - 1) << 4) | 0x09;
	WriteReg(TX_P0, AUX_CTRL, c);

	write_dpcd_addr(addrh, addrm, addrl);
	sp_write_reg_or(TX_P0, AUX_CTRL2, AUX_OP_EN);
	mdelay(2);
       TRACE3("auxch addr = 0x%.2bx%.2bx%.2bx\n", addrh,addrm,addrl);
	wait_aux_op_finish(&bOK);
	if (bOK == AUX_ERR) {
		TRACE("aux read failed\n");
		//add by span 20130217.
		ReadReg(TX_P2, SP_TX_INT_STATUS1, &c);
		ReadReg(TX_P0, TX_DEBUG1, &c1);
		//if polling is enabled, wait polling error interrupt
		if (c1&POLLING_EN){
			if (c & POLLING_ERR)
				sp_tx_rst_aux();
		}else
			sp_tx_rst_aux();
		return AUX_ERR;
	}

	for (i = 0; i < cCount; i++) {
		ReadReg(TX_P0, BUF_DATA_0 + i, &c);
		*(pBuf + i) = c;
		if (i >= MAX_BUF_CNT)
			break;
	}
       TRACE2("c = 0x%.2bx pBuf[0] = 0x%.2bx\n", c,pBuf[0]);
	return AUX_OK;
}


static unchar sp_tx_aux_dpcdwrite_bytes(unchar addrh, unchar addrm, unchar addrl, unchar cCount, unchar *pBuf)
{
	unchar c, i,ret;
	c =  ((cCount - 1) << 4) | 0x08;
	WriteReg(TX_P0, AUX_CTRL, c);
	write_dpcd_addr(addrh, addrm, addrl);
	for (i = 0; i < cCount; i++) {
		c = *pBuf;
		pBuf++;
		WriteReg(TX_P0, BUF_DATA_0 + i, c);

		if (i >= 15)
			break;
	}
	sp_write_reg_or(TX_P0, AUX_CTRL2, AUX_OP_EN);
	wait_aux_op_finish(&ret);
	return ret;
}
static void wait_aux_op_finish(unchar * err_flag)
{
	unchar cnt;
	unchar c;

	*err_flag = 0;
	cnt = 150;
	while (__i2c_read_byte(TX_P0, AUX_CTRL2) & AUX_OP_EN) {
				mdelay(2);
				if ((cnt--) == 0) {
					TRACE("aux operate failed!\n");
					*err_flag = 1;
					break;
				}
			}

	ReadReg(TX_P0, SP_TX_AUX_STATUS, &c);
	if (c & 0x0F) {
			TRACE1("wait aux operation status %.2x\n", (uint)c);
			*err_flag = 1;
	}
}
#else
unchar sp_tx_aux_dpcdread_bytes(unchar addrh, unchar addrm,
	unchar addrl, unchar cCount, unchar *pBuf)
{
	unchar c, i;
	unchar bOK;

	//command and length
	c = ((cCount - 1) << 4) | 0x09;
	WriteReg(RX_P0, AP_AUX_COMMAND, c);
      //address
	write_dpcd_addr(addrh, addrm, addrl);
	//aux en
	sp_write_reg_or(RX_P0, AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	mdelay(2);
       TRACE3("auxch addr = 0x%.2bx%.2bx%.2bx\n", addrh,addrm,addrl);
	wait_aux_op_finish(&bOK);
	if (bOK == AUX_ERR) {
		TRACE("aux read failed\n");
		return AUX_ERR;
	}

	for (i = 0; i < cCount; i++) {
		ReadReg(RX_P0, AP_AUX_BUFF_START + i, &c);
		*(pBuf + i) = c;
		TRACE2("Buf[%d] = 0x%.2bx\n", (uint)i, *(pBuf + i));
		if (i >= MAX_BUF_CNT)
			break;
	}

	return AUX_OK;
}


unchar sp_tx_aux_dpcdwrite_bytes(unchar addrh, unchar addrm, unchar addrl, unchar cCount, unchar *pBuf)
{
	unchar c, i,ret;

	//command and length
	c =  ((cCount - 1) << 4) | 0x08;
	WriteReg(RX_P0, AP_AUX_COMMAND, c);
	//address
	write_dpcd_addr(addrh, addrm, addrl);
	//data
	for (i = 0; i < cCount; i++) {
		c = *pBuf;
		pBuf++;
		WriteReg(RX_P0, AP_AUX_BUFF_START + i, c);

		if (i >= 15)
			break;
	}
	//aux en
	sp_write_reg_or(RX_P0,  AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	wait_aux_op_finish(&ret);
	TRACE("DPCD write done\n");
	return ret;
}
static void wait_aux_op_finish(unchar * err_flag)
{
	unchar cnt;
	unchar c;

	*err_flag = 0;
	cnt = 150;
	while (__i2c_read_byte(RX_P0, AP_AUX_CTRL_STATUS) & AP_AUX_CTRL_OP_EN) {
				mdelay(2);
				if ((cnt--) == 0) {
					TRACE("aux operate failed!\n");
					*err_flag = 1;
					break;
				}
			}

	ReadReg(RX_P0, AP_AUX_CTRL_STATUS, &c);
	if (c & 0x0F) {
			TRACE1("wait aux operation status %.2x\n", (uint)c);
			*err_flag = 1;
	}
}

#endif

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
	WriteReg(RX_P0, AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	wait_aux_op_finish(&c);

	return c;
}

unchar  sp_tx_aux_rd(unchar len_cmd)
{
	unchar c;

	WriteReg(RX_P0, AP_AUX_COMMAND, len_cmd);
	WriteReg(RX_P0, AP_AUX_CTRL_STATUS, AP_AUX_CTRL_OP_EN);
	wait_aux_op_finish(&c);

	return c;
}

unchar sp_tx_get_edid_block(void)
{
	unchar c;

	sp_tx_aux_wr(0x7e);
	sp_tx_aux_rd(0x01);
	ReadReg(RX_P0, AP_AUX_BUFF_START, &c);
	TRACE1(" EDID Block = %d\n", (int)(c + 1));

	if (c > 3)
		c = 1;

	return c;
}

unchar edid_read(unchar offset, unchar *pblock_buf)
{
	unchar c, cnt = 0;

	while (cnt < 3) {
		sp_tx_aux_wr(offset);
		/* set I2C read com 0x01 mot = 0 and read 16 bytes          */
		sp_tx_aux_rd(0xf1);

		if( c == 1) {
			sp_tx_rst_aux();
			TRACE("edid read failed, aux reset!\n");
	              cnt ++;
		}
		else {
			for (c = 0; c < 16; c++)
				ReadReg(RX_P0, AP_AUX_BUFF_START + c,
						    &(pblock_buf[c ]));
			return 0;
		}

	}

	return 1;

}

unchar segments_edid_read(unchar segment, unchar offset)
{
	unchar c, cnt = 0;
	int i;

	//write address only
	WriteReg(RX_P0, AP_AUX_ADDR_7_0, 0x30);
	WriteReg(RX_P0, AP_AUX_COMMAND, 0x04);
	WriteReg(RX_P0, AP_AUX_CTRL_STATUS, AP_AUX_CTRL_ADDRONLY | AP_AUX_CTRL_OP_EN);

	wait_aux_op_finish(&c);
	//write segment address
	sp_tx_aux_wr(segment);
	//data read
	WriteReg(RX_P0, AP_AUX_ADDR_7_0, 0x50);

	while (cnt < 3) {
		sp_tx_aux_wr(offset);
		/* set I2C read com 0x01 mot = 0 and read 16 bytes          */
		c = sp_tx_aux_rd(0xf1);
		if( c == 1) {
			sp_tx_rst_aux();
			TRACE("segment edid read failed, aux reset!\n");
	              cnt ++;
		}
		else {
			for (i = 0; i < 16; i++)
				ReadReg(RX_P0, AP_AUX_BUFF_START + i, &c);
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
	/*pr_info("%s %s : g_edid_checksum%x\n",
			LOG_TAG, __func__, (uint)g_edid_checksum); */
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

void sp_tx_edid_read(unchar *pedid_blocks_buf)
{
	unchar offset = 0;
	unchar count, blocks_num;
	unchar pblock_buf[16];
	unchar i, j;
	unchar g_edid_break = 0;

	//address initial
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

	} while (blocks_num >= count);

	/* check edid data */
	check_edid_data(pedid_blocks_buf);

	/*  test edid << */
	sp_tx_aux_dpcdread_bytes(0x00, 0x02, 0x18, 1, &i);

	if (i & 0x04) {
		//TRACE1("%s %s : check sum = %.2x\n", (uint) g_edid_checksum);
		if(pedid_blocks_buf[0x7e]  > 1)
			sp_tx_aux_dpcdwrite_bytes(0x00, 0x02, 0x61, 1, (unchar *)(pedid_blocks_buf + 0xff));
		else
			sp_tx_aux_dpcdwrite_bytes(0x00, 0x02, 0x61, 1, (unchar *)(pedid_blocks_buf + 0x7f));
		j = 0x04;
		sp_tx_aux_dpcdwrite_bytes(0x00, 0x02, 0x60, 1, &j);
		TRACE(" Test EDID done\n");

	}
	/*  test edid  >> */

}
void edid_dump(void)
{
	//uint k;
	unsigned char   blocks_num=1;
       unsigned char xdata  edid_blocks[256];
      TRACE("EDID read \n");
	sp_tx_edid_read(edid_blocks);
	sp_tx_rst_aux();
	#if 0
	TRACE("            0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F\n");

	for(k=0; k<(128*((uint)blocks_num+1)); k++)
	{
	  	if((k&0x0f)==0)
	  		TRACE2("\nedid: [%.2x]  %.2x  ", (uint)(k % 0x0f), (uint)edid_blocks[k]);
	  	else
	  		TRACE1("%.2x   ", (uint)edid_blocks[k]);

	  	if((k&0x0f)==0x0f)
	  		TRACE("\n");
	 }

	  //check_edid_data(edid_blocks);
	TRACE(" \n################EDID read finished########################\n");
	  #endif
}
static void errchk(void)
{
    uint errl,errh;
    sp_tx_aux_dpcdread_bytes(0x00,0x02,0x10,2,bytebuf);//LANE_ALIGN__STATUS_UPDATED
    sp_tx_aux_dpcdread_bytes(0x00,0x02,0x10,2,bytebuf);//LANE0_1_STATUS : Lane0 and Lane1 Status

        errh = bytebuf[1];
        if (errh & 0x80)
        {
            errl = bytebuf[0];
            errh = (errh &0x7f) << 8;
            errl = errh + errl;
            TRACE1(" Err of Lane[0] = %d\n", (uint)errl);
        }
}

/* force powering on ANX7625 */
/* Usage: poweron */
static void poweron(void)
{
    ANX7625_POWER_ON();
    TRACE("force powering on ANX7625\n");
    delay_ms(10);
    ANX7625_RESET_RELEASE();
    TRACE("ANX7625 reset release\n");
    delay_ms(10);
}


/* force powering off ANX7625 */
/* Usage: poweroff */
static void poweroff(void)
{
    ANX7625_POWER_DOWN();
    TRACE("force powering off ANX7625\n");
    ANX7625_RESET();
    TRACE("ANX7625 reset asserted\n");
}
#ifdef ANX3625_Support
/* force powering on ANX7625 */
/* Usage: poweron */
static void poweronsp(void)
{
       WriteReg(RX_P0, FIRMWARE_CTRL, auto_pd_en | slimport_mode_mode);
	WriteReg(RX_P0, AP_AV_STATUS, AP_DISABLE_PD);  //disable PD function
	ENABLE_5V_VBUS_OUT();
	TRACE("VBUS 5V OUT.\n");
	delay_ms(500);
	AP_TX_process();//jack
}
#endif

/* reset MI-2, and then after RESET_RELEASE_DELAY ms, reset release */
/* Usage: reset */
void reset_MI2(void)
{
    ANX7625_RESET();
    TRACE("ANX7625 reset\n");
    delay_ms((unsigned int)RESET_RELEASE_DELAY);
    ANX7625_RESET_RELEASE();
    TRACE("ANX7625 reset release\n");
}

/* enable 5V Vbus output and disable 5~20V Vbus charge input */
/* Usage: vbuson */
static void vbuson(void)
{
    ENABLE_5V_VBUS_OUT();
    TRACE("5V Vbus output: enabled\n");
    TRACE("5~20V Vbus charge input: disabled\n");
}


/* disable 5V Vbus output and enable 5~20V Vbus charge input */
/* Usage: vbusoff */
static void vbusoff(void)
{
    DISABLE_5V_VBUS_OUT();
    TRACE("5V Vbus output: disabled\n");
    TRACE("5~20V Vbus charge input: enabled\n");
}

 void AP_TX_process(void)
{
	unchar c;

	/* 0x84:0xEE [6:5] = Try.Enc_Level
	  b00 = No encryption. (Default)
	  b01 = Try authentication at HDCP 1.x
	  b10 = Try authentication at HDCP 2.2
	  b11 = Reserved
	*/
	sp_write_reg_and(RX_P1, 0xee, 0x9f);  //not support HDCP

	if(SW5_1) {   // HDCP 1.4
		sp_tx_aux_dpcdread_bytes(0x06, 0x80, 0x28, 1, &c);  // read Bcap
		if( (c & 0x01) == 1) {
			sp_write_reg_or(RX_P1, 0xee, 0x20);
			TRACE("HDCP1.4\n");
		}
		else
			TRACE("not support HDCP\n");
	}
	else { // HDCP 2.2
		sp_write_reg_or(RX_P1, 0xee, 0x40);
		TRACE("HDCP2.2\n");
	}

	sp_write_reg_or(RX_P1, 0xec, 0x10);  //try auth flag
	sp_write_reg_or(RX_P1, 0xff, 0x01);  // interrupt for DRM

	 API_Custom_Register0_Configuration(video_table_id);
	switch (mipi_output_mode) {
		default:
			/* DPI */
		case VIDEO_DPI:
			 sp_write_reg_and(RX_P0, R_DSC_CTRL_0, ~DSC_EN);
			API_DPI_Configuration(video_table_id);
			TRACE1("DPI input configure,  table id %d\n", (uint)video_table_id);
			break;
			/* DPI + DSC*/
		case VIDEO_DPI_DSC:
			API_DSC_Configuration(video_table_id);
                    API_ODFC_Configuration(video_table_id);
                    API_DPI_Configuration(video_table_id);
			TRACE1("DPI +DSC input configure,  table id %d\n", (uint)video_table_id);
			break;
			/* DSI */
		case VIDEO_DSI:
		   //DSC disable
                    sp_write_reg_and(RX_P0, R_DSC_CTRL_0, ~DSC_EN);
                    API_DSI_Configuration(video_table_id);
			TRACE1("DSI input configure,  table id %d\n", (uint)video_table_id);
			break;
			/* DSI + DSC*/
		case VIDEO_DSI_DSC:
			 API_DSC_Configuration(video_table_id);
                      API_DSI_Configuration(video_table_id);
			TRACE1("DSI + DSC input configure,  table id %d\n", (uint)video_table_id);
			break;
	}
	API_Configure_Audio_Input(audio_table_id);
	API_Custom_Register1_Configuration(video_table_id);
	API_Video_Output_Enable();
}

