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

void arduino::Portenta_Video::update() {
    _currFrameBufferAddr = getNextFrameBuffer();
}

void arduino::Portenta_Video::drawImage(uint32_t x, uint32_t y, void *img, uint32_t width, uint32_t height) {
    uint32_t offsetPos = 0;

    offsetPos = (x + (_displayWidth * y)) * sizeof(uint16_t);
    stm32_LCD_DrawImage(img, (void *)(_currFrameBufferAddr + offsetPos), width, height, DMA2D_INPUT_RGB565);
}

uint32_t arduino::Portenta_Video::getWidth() {
    return _displayWidth;
}

uint32_t arduino::Portenta_Video::getHeight() {
    return _displayHeight;
}

void arduino::Portenta_Video::drawFilledRectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    uint32_t offsetPos = 0;

    offsetPos = (x + (_displayWidth * y)) * sizeof(uint16_t);
    stm32_LCD_FillArea((void *)(_currFrameBufferAddr + offsetPos), width, height, color);
}

void arduino::Portenta_Video::drawRectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
  int x1 = x;               // Top-left corner x-coordinate
  int y1 = y;               // Top-left corner y-coordinate
  int x2 = x + width - 1;   // Bottom-right corner x-coordinate
  int y2 = y + height - 1;  // Bottom-right corner y-coordinate

  drawLine(x1, y1, x2, y1, color); // Top
  drawLine(x1, y2, x2, y2, color);// Bottom
  drawLine(x1, y1, x1, y2, color); // Left
  drawLine(x2, y1, x2, y2, color); // Right
}

void arduino::Portenta_Video::drawPixel(uint32_t x, uint32_t y, uint32_t color) {
    drawFilledRectangle(x, y, 1, 1, color);
}

void arduino::Portenta_Video::drawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color) {
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
