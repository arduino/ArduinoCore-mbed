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

   public:
        GC2145(arduino::MbedI2C &i2c = CameraWire);
        int init();
        int reset();
        int getID() { return GC2145_I2C_ADDR; };
        uint32_t getClockFrequency() { return 12000000; };
        int setFrameRate(int32_t framerate);
        int setResolution(int32_t resolution);
        int setPixelFormat(int32_t pixformat);
        int reg_write(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr = false);
        uint8_t reg_read(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr = false);
        void debug(Stream &stream);
};
 
#endif /* __GC2145_H */
