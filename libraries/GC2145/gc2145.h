/*
 * TODO: Add license.
 * Copyright (c) 2021
 *
 * This work is licensed under <>, see the file LICENSE for details.
 *
 * GC2145 driver.
 */
#ifndef __GC2145_H
#define __GC2145_H

#include "camera.h"

class GC2145: public ImageSensor {
   private:
    int SetWindow(uint16_t reg, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
   public:
       int Init();
       int Reset();
       int GetID() { return GC2145_I2C_ADDR; };
       uint32_t GetClockFrequency() { return 12000000; };
       int SetFrameRate(uint32_t framerate);
       int SetResolution(uint32_t resolution);
       int SetPixelFormat(uint32_t pixelformat);
};
 
#endif /* __GC2145_H */
