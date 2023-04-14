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
 * HM0360 driver.
 */
#ifndef __HIMAX_H
#define __HIMAX_H

#include "camera.h"
#include "drivers/InterruptIn.h"

class HM0360: public ImageSensor {
   private:
        Stream *_debug;
        arduino::MbedI2C *_i2c;
        mbed::InterruptIn md_irq;
        md_callback_t _md_callback;
        void irqHandler();
        int regWrite(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr = false);
        uint8_t regRead(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr = false);

   public:
        HM0360(arduino::MbedI2C &i2c = CameraWire);
        int init();
        int reset();
        int getID() { return HM0360_I2C_ADDR; };
        bool getMono() { return true; };
        uint32_t getClockFrequency() { return 24000000; };
        int setFrameRate(int32_t framerate);
        int setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y);
        int setResolution(int32_t resolution);
        int setPixelFormat(int32_t pixformat);
        int setTestPattern(bool enable, bool walking) override;
        int enableMotionDetection(md_callback_t callback=NULL);
        int disableMotionDetection();
        int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        int setMotionDetectionThreshold(uint32_t threshold);
        int motionDetected();
        int pollMotionDetection();
        int clearMotionDetection();
        int setVerticalFlip(bool flip_enable);
        int setHorizontalMirror(bool mirror_enable);

        uint8_t printRegs();
        void debug(Stream &stream);
};
#endif /* __HIMAX_H */
