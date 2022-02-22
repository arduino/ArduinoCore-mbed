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

class FrameBuffer {
    private:
        int32_t _fb_size;
        uint8_t *_fb;
        bool _isAllocated;
    public:
        FrameBuffer(int32_t x, int32_t y, int32_t bpp);
        FrameBuffer(int32_t address);
        FrameBuffer();
        uint32_t getBufferSize();
        uint8_t* getBuffer();
        void setBuffer(uint8_t *buffer);
        bool hasFixedSize();
        bool isAllocated();
};

typedef void (*md_callback_t)();

class ImageSensor {
    public:
        virtual ~ImageSensor() { }
        virtual int init() = 0;
        virtual int reset() = 0;
        virtual int getID() = 0;
        virtual uint32_t getClockFrequency() = 0;
        virtual int setFrameRate(int32_t framerate) = 0;
        virtual int setResolution(int32_t resolution) = 0;
        virtual int setPixelFormat(int32_t pixelformat) = 0;
        virtual int enableMotionDetection(md_callback_t callback) = 0;
        virtual int disableMotionDetection() = 0;
        virtual int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) = 0;
        virtual int setMotionDetectionThreshold(uint32_t threshold) = 0;
        virtual int motionDetected() = 0;
        virtual void debug(Stream &stream) = 0;

        int setStandby(bool enable) {
            return -1;
        }

        int setTestPattern(bool enable, bool walking) {
            return -1;
        }
};

class Camera {
    private:
        int32_t pixformat;
        int32_t resolution;
        int32_t framerate;
        ImageSensor *sensor;
        int reset();
        int probeSensor();
        Stream *_debug;
        arduino::MbedI2C *_i2c;
        FrameBuffer *_framebuffer;
    public:
        Camera(ImageSensor &sensor);
        bool begin(int32_t resolution=CAMERA_R320x240, int32_t pixformat=CAMERA_GRAYSCALE, int32_t framerate=30);
        int getID();
        int setFrameRate(int32_t framerate);
        int setResolution(int32_t resolution);
        int setPixelFormat(int32_t pixelformat);
        int setStandby(bool enable);
        int setTestPattern(bool enable, bool walking);
        int frameSize();
        int grabFrame(FrameBuffer &fb, uint32_t timeout=5000);
        int enableMotionDetection(md_callback_t callback=NULL);
        int disableMotionDetection();
        int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        int setMotionDetectionThreshold(uint32_t threshold);
        int motionDetected();
        void debug(Stream &stream);
};

#endif // __CAMERA_H

extern arduino::MbedI2C CameraWire;
