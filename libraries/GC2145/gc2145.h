/*
 * Copyright 2021 Arduino SA
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
 * GC2145 driver.
 */
#ifndef __GC2145_H
#define __GC2145_H

#include "camera.h"

class GC2145: public ImageSensor {
   private:
        int setWindow(uint16_t reg, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        Stream *_debug;
        arduino::MbedI2C *_i2c;
        int regWrite(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr = false);
        uint8_t regRead(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr = false);
        bool vertical_flip_state = false;
        bool horizontal_mirror_state = false;

   public:
        GC2145(arduino::MbedI2C &i2c = CameraWire);
        int init();
        int reset();
        int getID() { return GC2145_I2C_ADDR; };
        bool getMono() { return false; };
        uint32_t getClockFrequency() { return 12000000; };
        int setFrameRate(int32_t framerate);
        int setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y);
        int setResolution(int32_t resolution);
        int setPixelFormat(int32_t pixformat);
        int enableMotionDetection(md_callback_t callback) { return 0; };
        int disableMotionDetection() { return 0; };
        int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) { return 0; };
        int setMotionDetectionThreshold(uint32_t threshold) { return 0; };
        int motionDetected() { return 0; };
        int setVerticalFlip(bool flip_enable);
        int setHorizontalMirror(bool mirror_enable);
        void debug(Stream &stream);
};

#endif /* __GC2145_H */
