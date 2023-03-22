#include "Portenta_Video.h"
#include "video_driver.h"
#include "display.h"
#include "glcdfont.c"

#if __has_include ("lvgl.h")
#include "lvgl.h"

void lvgl_displayFlush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);
#endif

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

arduino::Portenta_Video::Portenta_Video(DisplayShieldModel shield) { 
  _shield = shield;
}

int arduino::Portenta_Video::begin(bool landscape) {
  #if defined(ARDUINO_PORTENTA_H7_M7)
    if (_shield == NONE_SHIELD) {
      portenta_init_video();
    } else if (_shield == GIGA_DISPLAY_SHIELD) {
      //@TODO Init portenta w/o ANX
    }
  #elif defined(ARDUINO_GIGA)
    giga_init_video(); 
    LCD_ST7701_Init();
  #else 
    #error Board not compatible with this library
  #endif

    _displayWidth   = stm32_getXSize();
    _displayHeight  = stm32_getYSize();
    _landscapeMode  = landscape;

    clear();
    
  #if __has_include("lvgl.h")
    update();

    /* Initiliaze LVGL library */
    lv_init();

    /* Create a draw buffer */
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t * buf1;                                           
    buf1 = (lv_color_t*)malloc((_displayWidth * _displayHeight / 10) * sizeof(lv_color_t)); /* Declare a buffer for 1/10 screen size */
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, _displayWidth * _displayHeight / 10);      /* Initialize the display buffer. */

    /* Initialize display features for LVGL library */
    static lv_disp_drv_t disp_drv;          /* Descriptor of a display driver */
    lv_disp_drv_init(&disp_drv);            /* Basic initialization */
    disp_drv.flush_cb = lvgl_displayFlush;  /* Set your driver function */
    disp_drv.draw_buf = &draw_buf;          /* Assign the buffer to the display */
    disp_drv.hor_res = _displayWidth;       /* Set the horizontal resolution of the display */
    disp_drv.ver_res = _displayHeight;      /* Set the vertical resolution of the display */
    disp_drv.rotated = (landscape) ? LV_DISP_ROT_270 : LV_DISP_ROT_NONE;
    disp_drv.sw_rotate = 1;
    lv_disp_drv_register(&disp_drv);        /* Finally register the driver */
  #endif

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
    uint32_t x_rot, y_rot, h_rot, w_rot;

    if (_landscapeMode) {
      x_rot = ((_displayWidth-1) - y) - height;
      y_rot = x;
      h_rot = width;
      w_rot = height;
    } else {
      x_rot = x;
      y_rot = y;
      h_rot = height;
      w_rot = width;
    }

    offsetPos = (x_rot + (_displayWidth * y_rot)) * sizeof(uint16_t);
    stm32_LCD_DrawImage(img, (void *)(_currFrameBufferAddr + offsetPos), w_rot, h_rot, DMA2D_INPUT_RGB565);
}

uint32_t arduino::Portenta_Video::getWidth() {
    return ((_landscapeMode) ? _displayHeight : _displayWidth);
}

uint32_t arduino::Portenta_Video::getHeight() {
    return ((_landscapeMode) ? _displayWidth : _displayHeight);
}

void arduino::Portenta_Video::drawFilledRectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    uint32_t offsetPos = 0;
    uint32_t x_rot, y_rot, h_rot, w_rot;

    if (_landscapeMode) {
      x_rot = ((_displayWidth-1) - y) - height;
      y_rot = x;
      h_rot = width;
      w_rot = height;
    } else {
      x_rot = x;
      y_rot = y;
      h_rot = height;
      w_rot = width;
    }

    offsetPos = (x_rot + (_displayWidth * y_rot)) * sizeof(uint16_t);
    stm32_LCD_FillArea((void *)(_currFrameBufferAddr + offsetPos), w_rot, h_rot, color);
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

void arduino::Portenta_Video::drawChar(uint32_t x, uint32_t y, unsigned char c, uint32_t color, uint32_t bg, uint8_t size) {
  //Reference: https://github.com/arduino-libraries/TFT/
  if((x >= _displayHeight)		|| // Clip right
     (y >= _displayHeight)		|| // Clip bottom
     ((x + 6 * size - 1) < 0)	|| // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          drawFilledRectangle(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          drawFilledRectangle(x+i*size, y+j*size, size, size, bg);
        } 	
      }
      line >>= 1;
    }
  }
}

void arduino::Portenta_Video::drawText(uint32_t x, uint32_t y, const char *text, uint32_t color, uint32_t bg, uint8_t size) {
    while (*text) {  // loop until end of string
        drawChar(x, y, *text, color, bg, size);
        x += size * 6;  // advance x position by 6 pixels per character
        ++text;  // move to next character in string
    }
}

#if __has_include("lvgl.h")
void lvgl_displayFlush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    uint32_t width      = lv_area_get_width(area);
    uint32_t height     = lv_area_get_height(area);
    uint32_t offsetPos  = (area->x1 + (disp->hor_res * area->y1)) * sizeof(uint16_t);

    stm32_LCD_DrawImage((void *) color_p, (void *)(getCurrentFrameBuffer() + offsetPos), width, height, DMA2D_INPUT_RGB565);
    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}
#endif