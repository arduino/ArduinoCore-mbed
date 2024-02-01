/**
  ******************************************************************************
  * @file    dsi.h
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#ifndef _DSI_H
#define _DSI_H

/* Exported struct -----------------------------------------------------------*/
struct display_timing {
	unsigned int pixelclock;
	unsigned int hactive;
	unsigned int hfront_porch;
	unsigned int hback_porch;
	unsigned int hsync_len;
	unsigned int vactive;
	unsigned int vfront_porch;
	unsigned int vback_porch;
	unsigned int vsync_len;
	unsigned int hpol : 1;
	unsigned int vpol : 1;
};

/* Exported variables --------------------------------------------------------*/
extern DSI_HandleTypeDef dsi;

/* Exported functions --------------------------------------------------------*/
int			dsi_init(uint8_t bus, struct edid *edid, struct display_timing *dt);
void		dsi_lcdClear(uint32_t color);
void		dsi_lcdDrawImage(void *pSrc, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode);
void		dsi_lcdFillArea(void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode);
void		dsi_configueCLUT(uint32_t* clut);
void		dsi_drawCurrentFrameBuffer(bool reload = true);
uint32_t	dsi_getCurrentFrameBuffer(void);
uint32_t 	dsi_getActiveFrameBuffer(void);
uint32_t	dsi_getFramebufferEnd(void);
uint32_t 	dsi_getDisplayXSize(void);
uint32_t 	dsi_getDisplayYSize(void);

#endif /* _DSI_H */