#include "Portenta_lvgl.h"
#include "Portenta_Video.h"
#include "image.h"

// Alternatively, any raw RGB565 image can be included on demand using this macro
/*
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(test, "/home/user/Downloads/test.bin");
*/

int offset;

void setup() {
  portenta_init_video();

  stm32_LCD_Clear(0);
  stm32_LCD_Clear(0);

  offset = ((stm32_getXSize() - 300)) + (stm32_getXSize() * (stm32_getYSize() - 300) / 2) * sizeof(uint16_t);
}

void loop() {
  // Replace texture_raw with testData if using the INCBIN method
  // Also, replace 300x300 resolution with the actual one
  stm32_LCD_DrawImage((void*)texture_raw, (void *)(getNextFrameBuffer() + offset), 300, 300, DMA2D_INPUT_RGB565);
}
