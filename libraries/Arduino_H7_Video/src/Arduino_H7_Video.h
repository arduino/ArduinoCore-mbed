/**
  ******************************************************************************
  * @file    Arduino_H7_Video.h
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  */

#ifndef _ARDUINO_H7_VIDEO_H
#define _ARDUINO_H7_VIDEO_H

/* Includes ------------------------------------------------------------------*/
#include <ArduinoGraphics.h>
#include "H7DisplayShield.h"

/* Exported defines ----------------------------------------------------------*/
#define H7V_OK                      1
#define H7V_ERR_INSUFFMEM           2

/* Exported enumeration ------------------------------------------------------*/

/* Class ----------------------------------------------------------------------*/
class Arduino_H7_Video : public ArduinoGraphics {
public:
#if defined(ARDUINO_PORTENTA_H7_M7)
  Arduino_H7_Video(int width = 1024, int heigth = 768, H7DisplayShield &shield = USBCVideo);
#elif defined(ARDUINO_GIGA)
  Arduino_H7_Video(int width = 800, int heigth = 480, H7DisplayShield &shield = GigaDisplayShield);
#endif
  virtual ~Arduino_H7_Video();

  int begin();
  void end();

  void clear();
  
  virtual void beginDraw();
  virtual void endDraw();

  virtual void set(int x, int y, uint8_t r, uint8_t g, uint8_t b);

  void attachLVGLTouchCb(void (*touch_cb)(void*,void*));
private:
    H7DisplayShield*    _shield;
    bool                _rotated;
    int                 _edidMode;
};

#endif /* _ARDUINO_H7_VIDEO_H */