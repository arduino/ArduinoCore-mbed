#include "Portenta_Video.h"
#include "Portenta_lvgl.h"

arduino::Portenta_Video::Portenta_Video() { }

int arduino::Portenta_Video::begin() {
    portenta_init_video();
    
    _displayWidth   = stm32_getXSize();
    _displayHeight  = stm32_getYSize();

    _currFrameBufferAddr = getCurrentFrameBuffer();
}

void arduino::Portenta_Video::fillScreen(uint32_t color) {
    stm32_LCD_Clear(color); 
}

void arduino::Portenta_Video::updateDisplay() {
    _currFrameBufferAddr = getNextFrameBuffer();
}

void arduino::Portenta_Video::drawImage(void *imgBuffer, uint32_t imgWidth, uint32_t imgHeight, uint32_t posWidht, uint32_t posHeight) {
    uint32_t offsetPos = 0;

    offsetPos = (posWidht + (_displayWidth * posHeight)) * sizeof(uint16_t);
    stm32_LCD_DrawImage(imgBuffer, (void *)(_currFrameBufferAddr + offsetPos), imgWidth, imgHeight, DMA2D_INPUT_RGB565);
}

uint32_t arduino::Portenta_Video::getWidthSize() {
    return _displayWidth;
}

uint32_t arduino::Portenta_Video::getHeightSize() {
    return _displayHeight;
}