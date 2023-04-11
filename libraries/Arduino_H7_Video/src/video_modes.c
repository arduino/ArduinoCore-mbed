/**
  ******************************************************************************
  * @file    video_modes.c
  * @author  
  * @version 
  * @date    
  * @brief  
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "video_modes.h"

/* Exported variables --------------------------------------------------------*/
struct envie_edid_mode envie_known_modes[NUM_KNOWN_MODES] = {
	[EDID_MODE_640x480_60Hz] = {
		.name = "640x480@75Hz", .pixel_clock = 29400, .refresh = 60,
		.hactive = 640, .hback_porch = 160, .hfront_porch = 16, .hsync_len = 96,
		.vactive = 480, .vback_porch = 45, .vfront_porch = 10, .vsync_len = 2,
	},
	[EDID_MODE_720x480_60Hz] = {
		.name = "720x480@60Hz", .pixel_clock = 27800, .refresh = 60,
		.hactive = 720, .hback_porch = 60, .hfront_porch = 16, .hsync_len = 62, 
		.vactive = 480, .vback_porch = 45, .vfront_porch = 9, .vsync_len = 6,
	},
	[EDID_MODE_800x600_59Hz] = {
		.name = "800x600@60Hz", .pixel_clock = 37800, .refresh = 60,
		.hactive = 800, .hback_porch = 104, .hfront_porch = 24, .hsync_len = 80, 
		.vactive = 600, .vback_porch = 17, .vfront_porch = 3, .vsync_len = 4,
	},
	[EDID_MODE_480x800_60Hz] = {
		.name = "480x800@60Hz", .pixel_clock = 38000, .refresh = 60,
		.hactive = 480, .hback_porch = 30, .hfront_porch = 320, .hsync_len = 24, 
		.vactive = 800, .vback_porch = 50, .vfront_porch = 20, .vsync_len = 4,
		.hpol = 1, .vpol = 1,
	},
	[EDID_MODE_1024x768_60Hz] = {
		.name = "1024x768@60Hz", .pixel_clock = 57800, .refresh = 60,
		.hactive = 1024, .hback_porch = 80, .hfront_porch = 24, .hsync_len = 68, .hpol = 0,
		.vactive = 768, .vback_porch = 29, .vfront_porch = 3, .vsync_len = 6, .vpol = 0,
	},
	//NOTE: Not supported by STM32H747: Pixel Clock > 58MHz (LTDC + DMA2D + SDRAM 16-bit, source: AN4861rev4-Table12)
	[EDID_MODE_1280x768_60Hz] = {
		.name = "1280x768@60Hz", .pixel_clock = 68300, .refresh = 60,
		.hactive = 1280, .hback_porch = 120, .hfront_porch = 32, .hsync_len = 20, 
		.vactive = 768, .vback_porch = 10, .vfront_porch = 45, .vsync_len = 12,
	},
	//NOTE: Not supported by STM32H747: Pixel Clock > 58MHz (LTDC + DMA2D + SDRAM 16-bit, source: AN4861rev4-Table12)
	[EDID_MODE_1280x720_60Hz] = {
		.name = "1280x720@60Hz", .pixel_clock = 74300, .refresh = 60,
		.hactive = 1280, .hback_porch = 370, .hfront_porch = 110, .hsync_len = 40,
		.vactive = 720, .vback_porch = 30, .vfront_porch = 5, .vsync_len = 20,
	},
	//NOTE: Not supported by STM32H747: Pixel Clock > 58MHz (LTDC + DMA2D + SDRAM 16-bit, source: AN4861rev4-Table12)
	[EDID_MODE_1920x1080_60Hz] = { 
		.name = "1920x1080@60Hz", .pixel_clock = 148500, .refresh = 60,
		.hactive = 1920, .hback_porch = 280, .hfront_porch = 88, .hsync_len = 44,
		.vactive = 1080, .vback_porch = 45, .vfront_porch = 4, .vsync_len = 4,
	},
};

/* Functions -----------------------------------------------------------------*/

enum edid_modes video_modes_get_edid(uint32_t h_check, uint32_t v_check) {
	int sum = 0;
	int sel_mode = -1;
	int sel_sum = 0;

	for (int i = 0; i<NUM_KNOWN_MODES; i++) {
		if (h_check <= envie_known_modes[i].hactive && v_check <= envie_known_modes[i].vactive) {
			sum = ((int)envie_known_modes[i].hactive - h_check) + 
					((int)envie_known_modes[i].vactive - v_check);
		} else {
			sum = -1;
		}

		if (sum >= 0 && ((sel_mode == -1) || (sum < sel_sum))) {
			sel_mode = i;
			sel_sum = sum;
		}
	}

	for (int i = 0; i<NUM_KNOWN_MODES; i++) {
		if (h_check <= envie_known_modes[i].vactive && v_check <= envie_known_modes[i].hactive) {
			sum = ((int)envie_known_modes[i].vactive - h_check) + 
					((int)envie_known_modes[i].hactive - v_check);
		} else {
			sum = -1;
		}

		if (sum >= 0 && ((sel_mode == -1) || (sum < sel_sum))) {
			sel_mode = i;
			sel_sum = sum;
		}
	}

	if (sel_mode == -1 						|| 
		sel_mode == EDID_MODE_1280x768_60Hz ||
		sel_mode == EDID_MODE_1280x720_60Hz || 
		sel_mode == EDID_MODE_1920x1080_60Hz) {
		sel_mode = EDID_MODE_1024x768_60Hz;
	}

	return sel_mode;
}

/**** END OF FILE ****/