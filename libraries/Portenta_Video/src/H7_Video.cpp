#include "H7_Video.h"
#include "video_driver.h"
#include "display.h"

H7_Video::H7_Video(int width, int heigth) :
  ArduinoGraphics(width, heigth) {
}

H7_Video::~H7_Video() {
}

int H7_Video::begin() {
  if (!ArduinoGraphics::begin()) {
    return 0;
  }

  textFont(Font_5x7);
  
  giga_init_video(); 
  LCD_ST7701_Init();
  
  clear();

  _currFrameBufferAddr = getCurrentFrameBuffer();

  return 1;
}

void H7_Video::end() {
  ArduinoGraphics::end();
}

void H7_Video::beginDraw() {
  ArduinoGraphics::beginDraw();

  clear();
}

void H7_Video::endDraw() {
  ArduinoGraphics::endDraw();

  _currFrameBufferAddr = getNextFrameBuffer();
}

void H7_Video::set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color =  (uint32_t)((uint32_t)(r << 16) | (uint32_t)(g << 8) | (uint32_t)(b << 0));
    stm32_LCD_FillArea((void *)(_currFrameBufferAddr + ((x + (width() * y)) * sizeof(uint16_t))), 1, 1, color);
}