#ifdef __cplusplus
extern "C" {
#endif
void Error_Handler(void);
void CopyBuffer(uint32_t *pSrc, 
                           uint32_t *pDst, 
                           uint16_t x, 
                           uint16_t y, 
                           uint16_t xsize, 
                           uint16_t ysize);
void LCD_BriefDisplay(void);
void MPU_Config_Video();
#ifdef __cplusplus
}
#endif

#define LCD_FB_START_ADDRESS       ((uint32_t)0xD0000000)
#define LAYER0_ADDRESS             (LCD_FB_START_ADDRESS)
