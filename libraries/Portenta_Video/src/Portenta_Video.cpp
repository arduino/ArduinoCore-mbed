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

void arduino::Portenta_Video::clear(){
    fillScreen(PV_COLOR_BLACK);
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

void arduino::Portenta_Video::drawRectangle(uint32_t rectWidth, uint32_t rectHeight, uint32_t posWidht, uint32_t posHeight, uint32_t color){
    uint32_t offsetPos = 0;

    offsetPos = (posWidht + (_displayWidth * posHeight)) * sizeof(uint16_t);
    stm32_LCD_FillArea((void *)(_currFrameBufferAddr + offsetPos), rectWidth, rectHeight, color);
}

void arduino::Portenta_Video::drawPixel(uint32_t posWidht, uint32_t posHeight, uint32_t color) {
    drawRectangle(1, 1, posWidht, posHeight, color);
}

void arduino::Portenta_Video::drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
  //Bresenham's line algorithm
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    drawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) {
      break;
    }
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void arduino::Portenta_Video::drawFilledCircle(uint32_t centerX, uint32_t centerY, uint32_t radius, uint32_t color) {
  //Bresenham's circle algorithm
  int x = 0;
  int y = radius;
  int d = 3 - 2 * radius;

  while (x <= y) {
    // Draw horizontal lines between the points on the circumference
    drawLine(centerX - y, centerY + x, centerX + y, centerY + x, color);
    drawLine(centerX - y, centerY - x, centerX + y, centerY - x, color);
    drawLine(centerX - x, centerY + y, centerX + x, centerY + y, color);
    drawLine(centerX - x, centerY - y, centerX + x, centerY - y, color);

    if (d <= 0) {
      d += 4 * x + 6;
    } else {
      d += 4 * (x - y) + 10;
      y--;
    }
    x++;
  }
}

void arduino::Portenta_Video::drawCircle(uint32_t centerX, uint32_t centerY, uint32_t radius, uint32_t color) {
  //Bresenham's circle algorithm
  int x = 0;
  int y = radius;
  int d = 3 - 2 * radius;

  while (x <= y) {
    drawPixel(centerX + x, centerY + y, color);
    drawPixel(centerX + y, centerY + x, color);
    drawPixel(centerX - x, centerY + y, color);
    drawPixel(centerX - y, centerY + x, color);
    drawPixel(centerX + x, centerY - y, color);
    drawPixel(centerX + y, centerY - x, color);
    drawPixel(centerX - x, centerY - y, color);
    drawPixel(centerX - y, centerY - x, color);

    if (d <= 0) {
      d += 4 * x + 6;
    } else {
      d += 4 * (x - y) + 10;
      y--;
    }
    x++;
  }
}

