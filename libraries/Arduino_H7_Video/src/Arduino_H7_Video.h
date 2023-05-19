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
#include "H7DisplayShield.h"
#if __has_include ("HasIncludeArduinoGraphics.h")
#include "ArduinoGraphics.h"
#define HAS_ARDUINOGRAPHICS
#endif

/* Exported defines ----------------------------------------------------------*/

/* Exported enumeration ------------------------------------------------------*/

/* Class ----------------------------------------------------------------------*/
class Arduino_H7_Video
#ifdef HAS_ARDUINOGRAPHICS
 : public ArduinoGraphics
#endif
{
public:
#if defined(ARDUINO_PORTENTA_H7_M7)
  Arduino_H7_Video(int width = 1024, int height = 768, H7DisplayShield &shield = USBCVideo);
#elif defined(ARDUINO_GIGA)
  Arduino_H7_Video(int width = 800, int height = 480, H7DisplayShield &shield = GigaDisplayShield);
#endif
  ~Arduino_H7_Video();

  int begin();
  void end();

  int width();
  int height();

#ifdef HAS_ARDUINOGRAPHICS
  void clear();

  virtual void beginDraw();
  virtual void endDraw();

  virtual void set(int x, int y, uint8_t r, uint8_t g, uint8_t b);
#endif
private:
    H7DisplayShield*    _shield;
    bool                _rotated;
    int                 _edidMode;
    uint32_t            _width;
    uint32_t            _height;
};

#endif /* _ARDUINO_H7_VIDEO_H */