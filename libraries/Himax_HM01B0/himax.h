/*
 * TODO: Add license.
 * Copyright (c) 2021
 *
 * This work is licensed under <>, see the file LICENSE for details.
 *
 * HM01B0 driver.
 */
#ifndef __HIMAX_H
#define __HIMAX_H

#include "camera.h"

class HM01B0: public ImageSensor {
   public:
        int Init();
        int Reset();
        int GetID() { return HM01B0_I2C_ADDR; };
        uint32_t GetClockFrequency() { return 6000000; };
        int SetFrameRate(int32_t framerate);
        int SetResolution(int32_t resolution);
        int SetPixelFormat(int32_t pixformat);
        int SetTestPattern(bool enable, bool walking);
        int EnableMD(bool enable);
        int SetMDThreshold(uint32_t threshold);
        int SetLROI(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
        int PollMD();
        int ClearMD();
        uint8_t PrintRegs();
};
#endif /* __HIMAX_H */
