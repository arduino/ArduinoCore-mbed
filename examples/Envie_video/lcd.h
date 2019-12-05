#ifdef __cplusplus
extern "C" {
#endif

  int doStuff(void);

  void CopyBuffer(uint32_t *pSrc,
                  uint32_t *pDst,
                  uint16_t x,
                  uint16_t y,
                  uint16_t xsize,
                  uint16_t ysize);

#ifdef __cplusplus
}
#endif

#define LCD_FB_START_ADDRESS       ((uint32_t)0xD0000000)
#define LAYER0_ADDRESS               (LCD_FB_START_ADDRESS)
