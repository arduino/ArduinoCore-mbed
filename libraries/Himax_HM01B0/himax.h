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
 * HM01B0 driver.
 */
#ifndef __HIMAX_H
#define __HIMAX_H

#include "camera.h"
#include "drivers/InterruptIn.h"

class HM01B0: public ImageSensor {
   private:
        Stream *_debug;
        arduino::MbedI2C *_i2c;
        mbed::InterruptIn md_irq;
        md_callback_t _md_callback;
        void irqHandler();
        int regWrite(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr = false);
        uint8_t regRead(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr = false);

   public:
        HM01B0(arduino::MbedI2C &i2c = CameraWire);
        int init();
        int reset();
        int getID() { return HM01B0_I2C_ADDR; };
        uint32_t getClockFrequency() { return 6000000; };
        int setFrameRate(int32_t framerate);
        int setResolution(int32_t resolution);
        int setPixelFormat(int32_t pixformat);
        int setTestPattern(bool enable, bool walking);
        int enableMotionDetection(md_callback_t callback=NULL);
        int disableMotionDetection();
        int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        int setMotionDetectionThreshold(uint32_t threshold);
        int motionDetected();
        int pollMotionDetection();
        int clearMotionDetection();

        uint8_t printRegs();
        void debug(Stream &stream);
};
#endif /* __HIMAX_H */
