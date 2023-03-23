#ifndef _H7_VIDEO_H
#define _H7_VIDEO_H

#include <ArduinoGraphics.h>
#include "anx7625.h"
#include "dsi.h"

#define H7_VIDEO_MAX_WIDTH          1280 
#define H7_VIDEO_MAX_HEIGHT         1024 

class H7_Video : public ArduinoGraphics {
public:
  H7_Video(int width = H7_VIDEO_MAX_WIDTH, int heigth = H7_VIDEO_MAX_HEIGHT);
  virtual ~H7_Video();

  int begin();
  void end();
  
  virtual void beginDraw();
  virtual void endDraw();

  virtual void set(int x, int y, uint8_t r, uint8_t g, uint8_t b);
private:
    uint32_t _currFrameBufferAddr;
};

#endif /* _H7_VIDEO_H */