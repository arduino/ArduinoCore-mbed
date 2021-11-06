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
    CAMERA_GRAYSCALE    = 0,
    CAMERA_BAYER        = 1,
    CAMERA_RGB565       = 2,
    CAMERA_PMAX
};

enum {
    CAMERA_R160x120     = 0,   /* QQVGA Resolution   */
    CAMERA_R320x240     = 1,   /* QVGA Resolution    */
    CAMERA_R320x320     = 2,   /* 320x320 Resolution */
    CAMERA_R640x480     = 3,   /* VGA                */
    CAMERA_R800x600     = 5,   /* SVGA               */
    CAMERA_R1600x1200   = 6,   /* UXGA               */
    CAMERA_RMAX
};

// Resolution table
extern const uint32_t restab[CAMERA_RMAX][2];

class ImageSensor {
    public:
        virtual int Init() = 0;
        virtual int Reset() = 0;
        virtual int GetID() = 0;
        virtual uint32_t GetClockFrequency() = 0;
        virtual int SetFrameRate(int32_t framerate) = 0;
        virtual int SetResolution(int32_t resolution) = 0;
        virtual int SetPixelFormat(int32_t pixelformat) = 0;

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
        int32_t pixformat;
        int32_t resolution;
        int32_t framerate;
        ImageSensor *sensor;
        int Reset();
        int ProbeSensor();
    public:
        Camera(ImageSensor *sensor): pixformat(-1), resolution(-1), framerate(-1), sensor(sensor) {}
        int begin(int32_t resolution=CAMERA_R320x240, int32_t pixformat=CAMERA_GRAYSCALE, int32_t framerate=30);
        int GetID();
        int SetFrameRate(int32_t framerate);
        int SetResolution(int32_t resolution);
        int SetPixelFormat(int32_t pixelformat);
        int SetStandby(bool enable);
        int SetTestPattern(bool enable, bool walking);
        int FrameSize();
        int GrabFrame(uint8_t *buffer, uint32_t timeout=5000);
};
#endif // __CAMERA_H
