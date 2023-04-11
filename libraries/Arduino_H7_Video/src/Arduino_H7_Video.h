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

/* Exported defines ----------------------------------------------------------*/
#define H7_VIDEO_MAX_WIDTH          1280 
#define H7_VIDEO_MAX_HEIGHT         1024 

/* Exported enumeration ------------------------------------------------------*/
enum DisplayShieldModel {
    NONE_SHIELD = 0,
    GIGA_DISPLAY_SHIELD = 1
};

/* Class ----------------------------------------------------------------------*/
class Arduino_H7_Video : public ArduinoGraphics {
public:
  Arduino_H7_Video(int width = H7_VIDEO_MAX_WIDTH, int heigth = H7_VIDEO_MAX_HEIGHT, DisplayShieldModel shield = NONE_SHIELD);
  virtual ~Arduino_H7_Video();

  int begin();
  void end();

  void clear();
  
  virtual void beginDraw();
  virtual void endDraw();

  virtual void set(int x, int y, uint8_t r, uint8_t g, uint8_t b);

  void attachLVGLTouchCb(void (*touch_cb)(void*,void*));
private:
    DisplayShieldModel  _shield;
    bool                _rotated;
    int                 _edidMode;
};

#endif /* _ARDUINO_H7_VIDEO_H */