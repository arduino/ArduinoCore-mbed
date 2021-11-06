/*
 * TODO: Add license.
 * Copyright (c) 2021
 *
 * This work is licensed under <>, see the file LICENSE for details.
 *
 * Camera driver.
 */
#ifndef __CAMERA_H
#define __CAMERA_H
#include "Wire.h"

#define HM01B0_I2C_ADDR         (0x24)
#define GC2145_I2C_ADDR         (0x3C)

enum {
    CAMERA_R160x120 = 0x00,   /* QQVGA Resolution   */
    CAMERA_R320x240 = 0x01,   /* QVGA Resolution    */
    CAMERA_R320x320 = 0x02,   /* 320x320 Resolution */
    CAMERA_RMAX
};

class ImageSensor {
    public:
        virtual int Init() = 0;
        virtual int Reset() = 0;
        virtual int GetID() = 0;
        virtual uint32_t GetClockFrequency() = 0;
        virtual int SetFrameRate(uint32_t framerate) = 0;
        virtual int SetResolution(uint32_t resolution) = 0;
        virtual int SetPixelFormat(uint32_t pixelformat) = 0;

        int SetStandby(bool enable) {
            return -1;
        }

        int IOCTL(int request, ...) {
            return -1;
        }

        int SetTestPattern(bool enable, bool walking) {
            return -1;
        }

        int reg_write(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr = false) {
            Wire.beginTransmission(dev_addr);
            uint8_t buf[3] = {(uint8_t) (reg_addr >> 8), (uint8_t) (reg_addr & 0xFF), reg_data};
            if (wide_addr == true) {
                Wire.write(buf, 1);
            }
            Wire.write(&buf[1], 2);
            return Wire.endTransmission();
        }

        uint8_t reg_read(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr = false) {
            uint8_t reg_data = 0;
            uint8_t buf[2] = {(uint8_t) (reg_addr >> 8), (uint8_t) (reg_addr & 0xFF)};
            Wire.beginTransmission(dev_addr);
            if (wide_addr) {
                Wire.write(buf, 2);
            } else {
                Wire.write(&buf[1], 1);
            }
            Wire.endTransmission(false);
            Wire.requestFrom(dev_addr, 1);
            if (Wire.available()) {
                reg_data = Wire.read();
            }
            while (Wire.available()) {
                Wire.read();
            }
            return reg_data;
        }
};

class Camera {
    private:
        uint32_t resolution;
        ImageSensor *sensor;
        int Reset();
        int ProbeSensor();
    public:
        Camera(ImageSensor *sensor): sensor(sensor){}
        int begin(uint32_t resolution=CAMERA_R320x240, uint32_t framerate=30);
        int GetID();
        int SetFrameRate(uint32_t framerate);
        int SetResolution(uint32_t resolution);
        int SetPixelFormat(uint32_t pixelformat);
        int SetStandby(bool enable);
        int SetTestPattern(bool enable, bool walking);
        int GrabFrame(uint8_t *buffer, uint32_t timeout=5000);
};
#endif // __CAMERA_H
