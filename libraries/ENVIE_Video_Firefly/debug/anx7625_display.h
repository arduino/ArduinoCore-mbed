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


#ifndef __DISPLAY_H__
#define __DISPLAY_H__

typedef enum
{
	BW_675G = 0x19,
	BW_54G = 0x14,
	BW_27G = 0x0A,
	BW_162G = 0x06,
	BW_NULL = 0x00
} SP_LINK_BW;

struct MIPI_Video_Format {
	unsigned char timing_id;
	unsigned char MIPI_video_type[32];
	unsigned char MIPI_lane_count;
	unsigned long MIPI_pixel_frequency;  //Hz	
	unsigned long  M;
	unsigned long  N;
       unsigned char  post_divider;	   
	unsigned char  diff_ratio;  /* bit[7:4]: DIFF_I_RATIO, bit[3:0]: DIFF_K_RATIO; i.e. 0x84:0x1B. These settings affect ODFC PLL locking range. */
	unsigned char compress_ratio;
	unsigned char video_3D_type;
	unsigned char * pps_reg;
	struct RegisterValueConfig * custom_reg0;
	struct RegisterValueConfig * custom_reg1;

	struct TimingInfor {
		unsigned int MIPI_HTOTAL;
		unsigned int MIPI_HActive;
		unsigned int MIPI_VTOTAL;
		unsigned int MIPI_VActive;

		unsigned int MIPI_H_Front_Porch;
		unsigned int MIPI_H_Sync_Width;
		unsigned int MIPI_H_Back_Porch;


		unsigned int MIPI_V_Front_Porch;
		unsigned int MIPI_V_Sync_Width;
		unsigned int MIPI_V_Back_Porch;
	}MIPI_inputl[2];
};

typedef enum {
	AUDIO_I2S,
	AUDIO_TDM,

	AUDIO_BIST,
	AUDIO_SPDIF,

	AUDIO_SLIMBUS
} AudioType;

typedef enum {
	AUDIO_FS_441K = 0x00,
	AUDIO_FS_48K   = 0x02,
	AUDIO_FS_32K   = 0x03,
	AUDIO_FS_882K = 0x08,
	AUDIO_FS_96K   = 0x0a,
	AUDIO_FS_1764K= 0x0c,
	AUDIO_FS_192K =  0x0e
} AudioFs;

typedef enum {
	AUDIO_W_LEN_16_20MAX = 0x02,
	AUDIO_W_LEN_18_20MAX = 0x04,
	AUDIO_W_LEN_17_20MAX = 0x0c,
	AUDIO_W_LEN_19_20MAX = 0x08,
	AUDIO_W_LEN_20_20MAX = 0x0a,
	AUDIO_W_LEN_20_24MAX = 0x03,
	AUDIO_W_LEN_22_24MAX = 0x05,
	AUDIO_W_LEN_21_24MAX = 0x0d,
	AUDIO_W_LEN_23_24MAX = 0x09,
	AUDIO_W_LEN_24_24MAX = 0x0b
} AudioWdLen;

enum
{
	VIDEO_3D_NONE		= 0x00,
	VIDEO_3D_FRAME_PACKING		= 0x01,
	VIDEO_3D_TOP_AND_BOTTOM		= 0x02,
	VIDEO_3D_SIDE_BY_SIDE		= 0x03,
};

typedef enum {
	I2S_CH_2 =0x01,
	I2S_CH_4 =0x03,
	I2S_CH_6 =0x05,
	I2S_CH_8 =0x07
} I2SChNum;

typedef enum {
	I2S_LAYOUT_0,
	I2S_LAYOUT_1
} I2SLayOut;

typedef I2SChNum AudioChNum;


struct AudioFormat {
	unsigned char table_id;
	AudioType bAudioType;
	AudioChNum bAudioChNum;
	I2SLayOut  AUDIO_LAYOUT;
	unsigned char  bAudio_Fs;
	unsigned char bAudio_word_len;
};

struct RegisterValueConfig {
	unsigned char slave_addr;
	unsigned char reg;
	unsigned char val;
};

extern  code struct MIPI_Video_Format mipi_video_timing_table[];
extern code struct AudioFormat audio_format_table[];
extern code unsigned char PPS_4K[];

void sp_tx_show_infomation(void);
void sp_tx_pclk_calc(SP_LINK_BW hbr_rbr, unsigned long  *pclk, unsigned long  *M_val, unsigned long  *N_val);
void API_Configure_Audio_Input(unsigned char table_id);
void API_DSI_Configuration(unsigned char table_id);
void API_DPI_Configuration(unsigned char table_id);
void API_DSC_Configuration(unsigned char table_id);
void API_ODFC_Configuration(unsigned char table_id);
void API_3D_Structure_Configuration(unsigned char type);
void API_Custom_Register0_Configuration(unsigned char table_id);
void API_Custom_Register1_Configuration(unsigned char table_id);
void API_Video_Mute_Control(unsigned char status);
void API_Video_Output_Enable(void);
#endif  

