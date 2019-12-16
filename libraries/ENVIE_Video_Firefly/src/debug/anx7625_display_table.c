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
#include  <stdlib.h>
#include  <string.h>
#include  "MI2_REG.h"
#include  "anx7625_display.h"

const unsigned char PPS_4K[]={//VC707 (DPI+DSC)
	0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x08, 0x70, 0x0f, 0x00, 0x00, 0x78, 0x07, 0x80, 0x07, 0x80, 
	0x02, 0x00, 0x04, 0xc0, 0x00, 0x20, 0x12, 0x6b, 0x00, 0x1a, 0x00, 0x0c, 0x00, 0xcf, 0x00, 0x3e, 
	0x18, 0x00, 0x10, 0xf0, 0x03, 0x0c, 0x20, 0x00, 0x06, 0x0b, 0x0b, 0x33, 0x0e, 0x1c, 0x2a, 0x38, 
	0x46, 0x54, 0x62, 0x69, 0x70, 0x77, 0x79, 0x7b, 0x7d, 0x7e, 0x01, 0x02, 0x01, 0x00, 0x09, 0x40, 
	0x09, 0xbe, 0x19, 0xfc, 0x19, 0xfa, 0x19, 0xf8, 0x1a, 0x38, 0x1a, 0x78, 0x1a, 0xb6, 0x2a, 0xf6, 
	0x2b, 0x34, 0x2b, 0x74, 0x3b, 0x74, 0x6b, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char PPS_1080P[]={
	0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x04, 0x38, 0x07, 0x80, 0x00, 0x08, 0x03, 0xc0, 0x03, 0xc0, 
	0x02, 0x00, 0x02, 0xe0, 0x00, 0x20, 0x00, 0xed, 0x00, 0x0d, 0x00, 0x0c, 0x0d, 0xb7, 0x07, 0x27, 
	0x18, 0x00, 0x10, 0xf0, 0x03, 0x0c, 0x20, 0x00, 0x06, 0x0b, 0x0b, 0x33, 0x0e, 0x1c, 0x2a, 0x38, 
	0x46, 0x54, 0x62, 0x69, 0x70, 0x77, 0x79, 0x7b, 0x7d, 0x7e, 0x01, 0x02, 0x01, 0x00, 0x09, 0x40, 
	0x09, 0xbe, 0x19, 0xfc, 0x19, 0xfa, 0x19, 0xf8, 0x1a, 0x38, 0x1a, 0x78, 0x1a, 0xb6, 0x2a, 0xf6, 
	0x2b, 0x34, 0x2b, 0x74, 0x3b, 0x74, 0x6b, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const struct RegisterValueConfig Bit_Matrix[]={
	{TX_P2,AUDIO_CONTROL_REGISTER,0x80},
	{TX_P2,VIDEO_BIT_MATRIX_12,0x18},
	{TX_P2,VIDEO_BIT_MATRIX_13,0x19},
	{TX_P2,VIDEO_BIT_MATRIX_14,0x1a},
	{TX_P2,VIDEO_BIT_MATRIX_15,0x1b},
	{TX_P2,VIDEO_BIT_MATRIX_16,0x1c},
	{TX_P2,VIDEO_BIT_MATRIX_17,0x1d},
	{TX_P2,VIDEO_BIT_MATRIX_18,0x1e},
	{TX_P2,VIDEO_BIT_MATRIX_19,0x1f},
	{TX_P2,VIDEO_BIT_MATRIX_20,0x20},
	{TX_P2,VIDEO_BIT_MATRIX_21,0x21},
	{TX_P2,VIDEO_BIT_MATRIX_22,0x22},
	{TX_P2,VIDEO_BIT_MATRIX_23,0x23},
	{0x00,0x00,0x00}
};

const struct RegisterValueConfig Regvalue_Cust[]={
	{RX_P2,0xa1,0x01},
	{RX_P2,0xa2,0x02},
	{RX_P2,0xa3,0x03},
	{RX_P2,0xa4,0x04},
	{RX_P2,0xa6,0x05},
	{RX_P2,0xa6,0x06},
	{RX_P2,0xa7,0x07},
	{RX_P2,0xa8,0x08},
	{0x00,0x00,0x00}
};
const struct MIPI_Video_Format mipi_video_timing_table[]={
//                               lane_count--pixel_clk-----M---N--div--diff--compr--3d strc--pps table--custom0--custom1
//	  original timing			    Htotal--H active--Vtotal--V active-HFP--HSW--HBP--VFP--VSW--VBP
//     decompressed timing              Htotal--H active--Vtotal--V active-HFP--HSW--HBP--VFP--VSW--VBP
	// VC707 (DPI+DSC)
	{0,"3840X2160P@24",	 3,       297000000,	    0,     0,    1,     52,    3, VIDEO_3D_NONE, PPS_4K,      Bit_Matrix,      NULL,
							  {{1466,   1280,     2250,     2160,	58,   28,      100,    8,	     10,    72},
							    {4398,   3840,     2250,     2160,	174,   84,      300,    8,	     10,    72}}},
	//(DPI)
	{1,"720x480@60",	        3,       27000000,	    0,      0,    0,     52,   0,   VIDEO_3D_NONE, NULL,    Bit_Matrix,   NULL,
	  						  {{858,     720,      525,       480,	    16,     60,      62,     10,      6,      29},
							    {858,     720,      525,       480,	    16,     60,      62,     10,      6,      29}}},
	{2,"1280X720P@60",	 3,       74250000,	   0,      0,    0,     52,  0, VIDEO_3D_NONE, NULL,   Bit_Matrix, NULL,
  							    {{1650,   1280,     750,       720,	 110,   40,      220,     5,	      5,      20},
							      {1650,   1280,     750,       720,	 110,   40,      220,     5,	      5,      20}}},
	{3, "1920x1080p@60",	 3,       148500000,    592,  27,  3,    52,    0,   VIDEO_3D_NONE, NULL,   Bit_Matrix, NULL,
							    {{2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36},
							      {2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36}}},
	// Smartech E7123(DSI)
	{4,"720x480@60",	 3,              27000000,    0xC00000,      0x100000,    0x0b,     0x37,  0,        VIDEO_3D_NONE, NULL,   NULL, NULL,
						           {{858,     720,      525,       480,      12,     62,      64,     8,      6,      31},
						             {858,     720,      525,       480,      12,     62,      64,     8,      6,      31}}},
	{5,"1280X720P@60",	 3,       73905768, 0xFA9A50, 0x0B71B0, 7, 0x3A, 0, VIDEO_3D_NONE, NULL, NULL, NULL,
       							 {{1650, 1280, 750, 720, 110, 40, 220, 5, 5, 20},
        							 {1650, 1280, 750, 720, 110, 40, 220, 5, 5, 20}}},
	{6, "1920x1080p@60",	 3,       148500000,	    592,  27,  3,    52,    0,        VIDEO_3D_NONE, NULL,   NULL, NULL,
	 						    {{2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36},
							      {2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36}}},  
       // Smartech sd8132(DSI)
       {7, "1920x1080p@60",	 3,       148500000,	    0xB00000,  0x080000,  3,    0x37,    0,        VIDEO_3D_NONE, NULL,   NULL, NULL,
	 						    {{2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36},
							      {2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36}}},  
	{8,"1280X720P@60",       3,       74250000,    0xB00000,      0x080000,    7,     0x37,  0,        VIDEO_3D_NONE, NULL,   NULL, NULL,
							    {{1650,   1280,     750,       720,      110,   40,      220,     5,          5,      20},
							    {1650,   1280,     750,       720,      110,   40,      220,     5,          5,      20}}},  
       {9,"720x480@60",     3,           27000000,    0xC00000,      0x100000,    0x0b,     0x37,  0,        VIDEO_3D_NONE, NULL,   NULL, NULL,
						           {{858,     720,      525,       480,      12,     62,      64,     8,      6,      31},
						             {858,     720,      525,       480,      12,     62,      64,     8,      6,      31}}},
    	{10,"1920X1080P@30",  3,       74250000,    0xB00000,      0x080000,    7,     0x37,  0,        VIDEO_3D_NONE, NULL,   NULL, NULL,
							     {{2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36},
							       {2200,   1920,     1125,     1080,    88,    44,     148,     4,       5,      36}}},  
       {11,"1920X1080P@24",  3,       74250000,    0xB00000,      0x080000,    7,     0x37,  0,        VIDEO_3D_NONE, NULL,   NULL, NULL,
						            {{2750,   1920,     1125,     1080,    638,    44,     148,     4,       5,      36},
						         {2750,   1920,     1125,     1080,    638,    44,     148,     4,       5,      36}}},
       //ThunderSoft's Qualcomm M8953 (DSI)
 	{12, "1920x1080p60", 3, 148142224, 0x8D4789, 0x066FF3, 3, 0x37, 0, VIDEO_3D_NONE, NULL, NULL, NULL,
       						   {{2200, 1920, 1125, 1080, 88, 44, 148, 4, 5, 36},
         						   {2200, 1920, 1125, 1080, 88, 44, 148, 4, 5, 36}}},
	//qcom820 platform - DSI
	{13, "1920x1080p60", 3, 148013144, 0x8D2805, 0x066FF3, 3, 0x37, 0, VIDEO_3D_NONE, NULL, NULL, NULL,
        {{2200, 1920, 1125, 1080, 88, 44, 148, 4, 5, 36},
         {2200, 1920, 1125, 1080, 88, 44, 148, 4, 5, 36}}},

};

const struct AudioFormat audio_format_table[] = {
	{0, 	 AUDIO_I2S,      I2S_CH_2,	I2S_LAYOUT_0,	 AUDIO_FS_48K,	 AUDIO_W_LEN_24_24MAX},
	{1, 	 AUDIO_TDM,     I2S_CH_4,	I2S_LAYOUT_0,	 AUDIO_FS_48K,	 AUDIO_W_LEN_24_24MAX},
	{2, 	 AUDIO_TDM,     I2S_CH_8,	I2S_LAYOUT_0,	 AUDIO_FS_48K,	 AUDIO_W_LEN_24_24MAX},
};

