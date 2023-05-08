/**
  ******************************************************************************
  * @file    Arduino_H7_Video.cpp
  * @author  
  * @version 
  * @date    
  * @brief
  ******************************************************************************
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
Arduino_H7_Video::Arduino_H7_Video(int width, int heigth, H7DisplayShield &shield) :
  ArduinoGraphics(width, heigth) {
  _shield   = &shield;
  _edidMode = _shield->getEdidMode(width, heigth);

  switch(_edidMode) {
    case EDID_MODE_640x480_60Hz ... EDID_MODE_800x600_59Hz: 
    case EDID_MODE_1024x768_60Hz ... EDID_MODE_1920x1080_60Hz:
      _rotated = (width < heigth) ? true : false;
      break;
    case EDID_MODE_480x800_60Hz:
      _rotated = (width >= heigth) ? true : false;
      break;
    default:
      _rotated = false;
      break;
  }
}

Arduino_H7_Video::~Arduino_H7_Video() {
}

int Arduino_H7_Video::begin() {
  if (!ArduinoGraphics::begin()) {
    return H7V_ERR_UNKNOWN;
  }

  textFont(Font_5x7);

  /* Configure SDRAM */
  SDRAM.begin();

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
      return H7V_ERR_INSUFFMEM;
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

  return 0;
}

void Arduino_H7_Video::end() {
  ArduinoGraphics::end();
}

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

#if __has_include("lvgl.h")
void lvgl_displayFlushing(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    uint32_t width      = lv_area_get_width(area);
    uint32_t height     = lv_area_get_height(area);
    uint32_t offsetPos  = (area->x1 + (dsi_getDisplayXSize() * area->y1)) * sizeof(uint16_t);

    dsi_lcdDrawImage((void *) color_p, (void *)(dsi_getActiveFrameBuffer() + offsetPos), width, height, DMA2D_INPUT_RGB565);
    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}

void Arduino_H7_Video::attachLVGLTouchCb(void (*touch_cb)(void*,void*)) {
  static lv_indev_drv_t indev_drv;                                                /* Descriptor of a input device driver */
  lv_indev_drv_init(&indev_drv);                                                  /* Basic initialization */
  indev_drv.type = LV_INDEV_TYPE_POINTER;                                         /* Touch pad is a pointer-like device */
  indev_drv.read_cb = (void(*)(lv_indev_drv_t *, lv_indev_data_t *))(touch_cb);   /* Set your driver function */
  lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);                      /* Register the driver in LVGL and save the created input device object */
}
#endif

/**** END OF FILE ****/