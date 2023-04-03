#ifndef _DSI_H
#define _DSI_H

extern DSI_HandleTypeDef dsi;

int                     dsi_init(uint8_t bus, struct edid *edid, struct display_timing *dt);

void                    dsi_lcdClear(uint32_t color);
void                    dsi_lcdDrawImage(void *pSrc, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode);
void                    dsi_lcdFillArea(void *pDst, uint32_t xSize, uint32_t ySize, uint32_t ColorMode);

void                    dsi_configueCLUT(uint32_t* clut);

uint32_t                dsi_getNextFrameBuffer(void);
uint32_t                dsi_getCurrentFrameBuffer(void);
uint32_t                dsi_getFramebufferEnd(void);

#endif /* _DSI_H */