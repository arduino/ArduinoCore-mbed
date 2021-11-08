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

class HM01B0: public ImageSensor {
   public:
        int Init();
        int Reset();
        int GetID() { return HM01B0_I2C_ADDR; };
        uint32_t GetClockFrequency() { return 6000000; };
        int SetFrameRate(int32_t framerate);
        int SetResolution(int32_t resolution);
        int SetPixelFormat(int32_t pixformat);
        int reg_write(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr = false);
        uint8_t reg_read(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr = false);
        int SetTestPattern(bool enable, bool walking);
        int EnableMD(bool enable);
        int SetMDThreshold(uint32_t threshold);
        int SetLROI(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
        int PollMD();
        int ClearMD();
        uint8_t PrintRegs();
};
#endif /* __HIMAX_H */
