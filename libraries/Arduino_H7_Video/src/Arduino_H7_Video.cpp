/*
 * Copyright 2023 Arduino SA
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * @file Arduino_H7_Video.cpp
 * @author Leonardo Cavagnis
 * @brief Source file for the Arduino H7 Video library.
 */

 /* Includes ------------------------------------------------------------------*/
#include "Arduino_H7_Video.h"

#include "dsi.h"
#include "SDRAM.h"
extern "C" {
#include "video_modes.h"
}

#if __has_include ("lvgl.h")
#include "lvgl.h"
#endif

/* Private function prototypes -----------------------------------------------*/
#if __has_include ("lvgl.h")
void lvgl_displayFlushing(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);
#endif

/* Functions -----------------------------------------------------------------*/
Arduino_H7_Video::Arduino_H7_Video(int width, int height, H7DisplayShield &shield)
#ifdef HAS_ARDUINOGRAPHICS
   : ArduinoGraphics(width, height)
#endif
{
  _height   = height;
  _width    = width;
  _shield   = &shield;
  _edidMode = _shield->getEdidMode(width, height);

  switch(_edidMode) {
    case EDID_MODE_640x480_60Hz ... EDID_MODE_800x600_59Hz: 
    case EDID_MODE_1024x768_60Hz ... EDID_MODE_1920x1080_60Hz:
      _rotated = (width < height) ? true : false;
      break;
    case EDID_MODE_480x800_60Hz:
      _rotated = (width >= height) ? true : false;
      break;
    default:
      _rotated = false;
      break;
  }
}

Arduino_H7_Video::~Arduino_H7_Video() {
}

int Arduino_H7_Video::begin() {
#ifdef HAS_ARDUINOGRAPHICS
  if (!ArduinoGraphics::begin()) {
    return 1; /* Unknown err */
  }

  textFont(Font_5x7);
#endif

  /* Video controller/bridge init */
  _shield->init(_edidMode);

  #if __has_include("lvgl.h")
    /* Initiliaze LVGL library */
    lv_init();

    /* Create a draw buffer */
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t * buf1;                                           
    buf1 = (lv_color_t*)malloc((width() * height() / 10) * sizeof(lv_color_t)); /* Declare a buffer for 1/10 screen size */
    if (buf1 == NULL) {
      return 2; /* Insuff memory err */
    }
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, width() * height() / 10);      /* Initialize the display buffer. */

    /* Initialize display features for LVGL library */
    static lv_disp_drv_t disp_drv;              /* Descriptor of a display driver */
    lv_disp_drv_init(&disp_drv);                /* Basic initialization */
    disp_drv.flush_cb = lvgl_displayFlushing;   /* Set your driver function */
    disp_drv.draw_buf = &draw_buf;              /* Assign the buffer to the display */
    if(_rotated) {
      disp_drv.hor_res = height();        /* Set the horizontal resolution of the display */
      disp_drv.ver_res = width();         /* Set the vertical resolution of the display */
      disp_drv.rotated  = LV_DISP_ROT_270;
    } else {
      disp_drv.hor_res = width();         /* Set the horizontal resolution of the display */
      disp_drv.ver_res = height();        /* Set the vertical resolution of the display */
      disp_drv.rotated  = LV_DISP_ROT_NONE;
    }
    disp_drv.sw_rotate = 1;
    lv_disp_drv_register(&disp_drv);        /* Finally register the driver */
  #endif

  /* Configure SDRAM */
  SDRAM.begin(dsi_getFramebufferEnd()); //FIXME: SDRAM init after video controller init can cause display glitch at start-up

  return 0;
}

int Arduino_H7_Video::width() {
  return _width;
}

int Arduino_H7_Video::height() {
  return _height;
}

bool Arduino_H7_Video::isRotated() {
  return _rotated;
}

void Arduino_H7_Video::end() {
#ifdef HAS_ARDUINOGRAPHICS
  ArduinoGraphics::end();
#endif
}

#ifdef HAS_ARDUINOGRAPHICS
void Arduino_H7_Video::beginDraw() {
  ArduinoGraphics::beginDraw();

  dsi_lcdClear(0); 
}

void Arduino_H7_Video::endDraw() {
  ArduinoGraphics::endDraw();

  dsi_drawCurrentFrameBuffer();
}

void Arduino_H7_Video::clear(){
  uint32_t bg = ArduinoGraphics::background();
  uint32_t x_size, y_size;

  if(_rotated) {
    x_size = (height() <= dsi_getDisplayXSize())? height() : dsi_getDisplayXSize();
    y_size = (width() <= dsi_getDisplayYSize())? width() : dsi_getDisplayYSize();
  } else {
    x_size = (width() <= dsi_getDisplayXSize())? width() : dsi_getDisplayXSize();
    y_size = (height() <= dsi_getDisplayYSize())? height() : dsi_getDisplayYSize();
  }

  dsi_lcdFillArea((void *)(dsi_getCurrentFrameBuffer()), x_size, y_size, bg);
}

void Arduino_H7_Video::set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t x_rot, y_rot;

    if (_rotated) {
      x_rot = ((height()-1) - y);
      y_rot = x;

      if (x_rot >= height() || y_rot >= width()) 
        return;
    } else {
      x_rot = x;
      y_rot = y;

      if (x_rot >= width() || y_rot >= height()) 
        return;
    }

    if (x_rot >= dsi_getDisplayXSize() || y_rot >= dsi_getDisplayYSize()) 
      return;

    uint32_t color =  (uint32_t)((uint32_t)(r << 16) | (uint32_t)(g << 8) | (uint32_t)(b << 0));
    dsi_lcdFillArea((void *)(dsi_getCurrentFrameBuffer() + ((x_rot + (dsi_getDisplayXSize() * y_rot)) * sizeof(uint16_t))), 1, 1, color);
}
#endif

#if __has_include("lvgl.h")
void lvgl_displayFlushing(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    uint32_t width      = lv_area_get_width(area);
    uint32_t height     = lv_area_get_height(area);
    uint32_t offsetPos  = (area->x1 + (dsi_getDisplayXSize() * area->y1)) * sizeof(uint16_t);

    dsi_lcdDrawImage((void *) color_p, (void *)(dsi_getActiveFrameBuffer() + offsetPos), width, height, DMA2D_INPUT_RGB565);
    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}
#endif

/**** END OF FILE ****/