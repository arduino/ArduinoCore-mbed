/**
  ******************************************************************************
  * @file    video_modes.h
  * @author  
  * @version 
  * @date    
  * @brief  
  ******************************************************************************
  */

#ifndef _VIDEO_MODES_H 
#define _VIDEO_MODES_H

/* Includes ------------------------------------------------------------------*/
#include "edid.h"

/* Exported struct -----------------------------------------------------------*/
struct envie_edid_mode {
	const char *name;
	unsigned int pixel_clock;
	unsigned int refresh;
	unsigned int hactive;
	unsigned int hback_porch;
	unsigned int hfront_porch;
	unsigned int hsync_len;
	unsigned int vactive;
	unsigned int vsync_len;
	unsigned int vback_porch;
	unsigned int vfront_porch;
	unsigned int voffset;
	unsigned int hpol : 1;
	unsigned int vpol : 1;
};

/* Exported variables --------------------------------------------------------*/
extern struct envie_edid_mode envie_known_modes[];

/* Exported functions --------------------------------------------------------*/
enum edid_modes video_modes_get_edid(uint32_t h_check, uint32_t v_check);

#endif /* _VIDEO_MODES_H */ 