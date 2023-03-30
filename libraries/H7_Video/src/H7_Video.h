#ifndef _H7_VIDEO_H
#define _H7_VIDEO_H

#include <ArduinoGraphics.h>
#include "anx7625.h"
#include "dsi.h"

#define H7_VIDEO_MAX_WIDTH          1280 
#define H7_VIDEO_MAX_HEIGHT         1024 

enum DisplayShieldModel {
    NONE_SHIELD = 0,
    GIGA_DISPLAY_SHIELD = 1
};

class H7_Video : public ArduinoGraphics {
public:
  H7_Video(int width = H7_VIDEO_MAX_WIDTH, int heigth = H7_VIDEO_MAX_HEIGHT, DisplayShieldModel shield = NONE_SHIELD);
  virtual ~H7_Video();

  int begin();
  int begin(bool landscape);
  void end();

  void clear();
  
  virtual void beginDraw();
  virtual void endDraw();

  virtual void set(int x, int y, uint8_t r, uint8_t g, uint8_t b);

  void attachLVGLTouchCb(void (*touch_cb)(void*,void*));
private:
    DisplayShieldModel  _shield;
    bool                _landscape;
    uint32_t            _currFrameBufferAddr;
};

#endif /* _H7_VIDEO_H */