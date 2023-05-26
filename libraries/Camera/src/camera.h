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
 */

/**
 * @file Camera.h
 * @brief Header file for the Arduino Camera library.
 *
 * This library allows capturing pixels from supported cameras on Arduino products.
 * Supported cameras: OV7670, HM0360, HM01B0, and GC2145.
 * The pixels are returned in a frame buffer from which frames can be retrieved continuously.
 */

#ifndef __CAMERA_H
#define __CAMERA_H
#include "Wire.h"

/// Camera I2C addresses
#define HM01B0_I2C_ADDR         (0x24)
#define HM0360_I2C_ADDR         (0x24)
#define GC2145_I2C_ADDR         (0x3C)

/** 
 * Camera pixel format enumeration
 * The different formats use different number of bits per pixel:
 * Grayscale (8-bit), Bayer (8-bit), RGB565 (16-bit)
 **/
enum {
    CAMERA_GRAYSCALE    = 0,
    CAMERA_BAYER        = 1,
    CAMERA_RGB565       = 2,
    CAMERA_PMAX                 /* Sentinel value */
};

/// Camera resolution enumeration
enum {
    CAMERA_R160x120     = 0,   /* QQVGA Resolution   */
    CAMERA_R320x240     = 1,   /* QVGA Resolution    */
    CAMERA_R320x320     = 2,   /* 320x320 Resolution */
    CAMERA_R640x480     = 3,   /* VGA                */
    CAMERA_R800x600     = 5,   /* SVGA               */
    CAMERA_R1600x1200   = 6,   /* UXGA               */
    CAMERA_RMAX                /* Sentinel value */
};

// Resolution table
extern const uint32_t restab[CAMERA_RMAX][2];


/**
 * @class FrameBuffer
 * @brief Frame buffer class for storing captured frames.
 */
class FrameBuffer {
    private:
        int32_t _fb_size;       /// Frame buffer size in bytes
        uint8_t *_fb;           /// Pointer to the frame buffer
        bool _isAllocated;      /// Flag indicating if the buffer is allocated on the heap

    public:
        /**
         * @brief Construct a new FrameBuffer object with a fixed size.
         *
         * @param x Width of the frame buffer
         * @param y Height of the frame buffer
         * @param bpp Bits per pixel
         */
        FrameBuffer(int32_t x, int32_t y, int32_t bpp);

        /**
         * @brief Construct a new FrameBuffer object with a given address.
         * This is useful if a resolution higher than 320x240 is required and external RAM shall be used. 
         * In that case, the following code can be used:
         * 
         * @code {.cpp}
         * #include "SDRAM.h"
         * FrameBuffer fb(SDRAM_START_ADDRESS);
         * ...
         * // In setup() add:
         * SDRAM.begin();
         * @endcode
         * It can also be used to store the frame buffer in a different location in the RAM while avoiding the use of malloc().
         * @param address The memory address of the frame buffer
         */
        FrameBuffer(int32_t address);

        /**
         * @brief Construct a new FrameBuffer object with no custom size or address.
         */
        FrameBuffer();

        /**
         * @brief Get the buffer size in bytes.
         *
         * @return uint32_t The buffer size in bytes
         */
        uint32_t getBufferSize();

        /**
         * @brief Get a pointer to the frame buffer.
         * This can be used to read the pixels from the frame buffer.
         * E.g. to print all pixels to the serial monitor as hex values:
         * @code {.cpp}
         * uint8_t *fb = fb.getBuffer();
         * for (int i = 0; i < fb.getBufferSize(); i++) {
         *    Serial.print(fb[i], HEX);
         *   Serial.print(" ");
         * }
         * @endcode
         * @return uint8_t* Pointer to the frame buffer
         */
        uint8_t* getBuffer();

        /**
         * @brief Set the frame buffer pointer.
         *
         * @param buffer Pointer to the frame buffer
         */
        void setBuffer(uint8_t *buffer);

        /**
         * @brief Check if the frame buffer has a fixed size.
         * This is the case if the frame buffer is constructed with a width, height, and bits per pixel.
         * @return true If the frame buffer has a fixed size
         * @return false Otherwise
         */
        bool hasFixedSize();

        /**
         * @brief Check if the frame buffer is allocated on the heap.
         *
         * @return true If the frame buffer is allocated
         * @return false Otherwise
         */
        bool isAllocated();
};

/// Function type definition for motion detection callbacks
typedef void (*md_callback_t)();


/**
 * @class ImageSensor
 * @brief Abstract base class for image sensor drivers.
 */
class ImageSensor {
    public:
        virtual ~ImageSensor() { }

        /**
         * @brief Initialize the image sensor.
         * This prepares the sensor for capturing frames by setting the registers to their default values.
         * @return int 0 on success, non-zero on failure
         */
        virtual int init() = 0;

        /**
         * @brief Reset the image sensor.
         * The effect differs between the camera models. 
         * It has no effect on the GC2145. On the OV7670, it resets all configuration registers to the default values. 
         * On HM0360 and HM01B0 the registers are reset to the defaults. The camera also enters Standby Mode, where there is no video output and the power consumption is lowered.
         * @return int 0 on success, non-zero on failure
         */
        virtual int reset() = 0;

        /**
         * @brief Get the I2C address of the image sensor.
         * @return int The sensor ID
         */
        virtual int getID() = 0;

        /**
         * @brief Check if the image sensor is monochrome aka grayscale.
         *
         * @return true If the sensor is monochrome
         * @return false Otherwise
         */
        virtual bool getMono() = 0;

        /**
         * @brief Get the clock frequency of the image sensor.
         *
         * @return uint32_t The clock frequency in Hz
         */
        virtual uint32_t getClockFrequency() = 0;

        /**
         * @brief Set the frame rate of the image sensor.
         * @note This has no effect on cameras that do not support variable frame rates.
         * @param framerate The desired frame rate in frames per second
         * @return int 0 on success, non-zero on failure
         */
        virtual int setFrameRate(int32_t framerate) = 0;

        /**
         * @brief Set the resolution of the image sensor.
         * @note This has no effect on cameras that do not support variable resolutions.
         * @param resolution The desired resolution, as defined in the resolution enum
         * @param zoom_resolution The desired zoom window size.
         * @param zoom_x The desired x position of the zoom window.
         * @param zoom_y The desired y position of the zoom window.
         * @return int 0 on success, non-zero on failure
         */
        virtual int setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y) = 0;

        /**
         * @brief Set the resolution of the image sensor.
         * 
         * @note This has no effect on cameras that do not support variable resolutions.
         * @param resolution The desired resolution, as defined in the resolution enum
         * @return int 0 on success, non-zero on failure
         */
        virtual int setResolution(int32_t resolution) = 0;

        /**
         * @brief Set the pixel (color) format of the image sensor.
         * @note This has no effect on cameras that do not support variable pixel formats.
         * e.g. the Himax HM01B0 only supports grayscale.
         * @param pixelformat The desired pixel format, as defined in the pixel format enum
         * @return int 0 on success, non-zero on failure
         */
        virtual int setPixelFormat(int32_t pixelformat) = 0;

        /**
         * @brief Enable motion detection with the specified callback.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @note This has no effect on cameras that do not support motion detection.
         * @param callback Function to be called when motion is detected
         * @return int 0 on success, non-zero on failure
         */
        virtual int enableMotionDetection(md_callback_t callback) = 0;

        /**
         * @brief Disable motion detection.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @return int 0 on success, non-zero on failure
         */
        virtual int disableMotionDetection() = 0;

        /**
         * @brief Set the motion detection window.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @param x The x-coordinate of the window origin
         * @param y The y-coordinate of the window origin
         * @param w The width of the window
         * @param h The height of the window
         * @return int 0 on success, non-zero on failure
         */
        virtual int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) = 0;

        /**
         * @brief Set the motion detection threshold.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * On the Himax HM01B0, the recommended threshold range is 3 - 240 (0x03 to 0xF0).
         * @param threshold The motion detection threshold
         * @return int 0 on success, non-zero on failure
         */
        virtual int setMotionDetectionThreshold(uint32_t threshold) = 0;

        /**
         * @brief Check if motion was detected and clear the motion detection flag.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @note This function must be called after the motion detection callback was executed to clear the motion detection flag.
         * @return int 0 if no motion is detected, non-zero if motion is detected
         */
        virtual int motionDetected() = 0;
        virtual int setVerticalFlip(bool flip_enable) = 0;
        virtual int setHorizontalMirror(bool flip_enable) = 0;        
        /**
         * @brief Output debug information to a stream.
         * You can use this function to output debug information to the serial port by passing Serial as the stream.
         * @param stream Stream to output the debug information
         */ 
        virtual void debug(Stream &stream) = 0;

        /**
         * @brief Set the sensor in standby mode.
         * @note This has no effect on cameras that do not support standby mode.
         * @note None of the currently supported camera drivers implement this function.
         * The HM01B0 and HM0360 cameras can be set in standby mode by calling reset() instead.
         * @param enable true to enable standby mode, false to disable
         * @return int 0 on success, non-zero on failure (or not implemented)
         */
        virtual int setStandby(bool enable) {
            return -1;
        }

        /**
         * @brief Set the test pattern mode for the sensor.
         *
         * @note This has no effect on cameras that do not support test pattern mode.
         * @param enable true to enable test pattern, false to disable
         * @param walking true for walking test pattern, false for other test pattern (if supported)
         * The walking test pattern alternates between black and white pixels which is useful for detecting stuck-at faults
         * @return int 0 on success, non-zero on failure (or not implemented)
         */
        virtual int setTestPattern(bool enable, bool walking) {
            return -1;
        }
};


/**
 * @class ScanResults
 * @brief A template class used to store the results from an I2C scan.
 *
 * @param T Data type for the device address (e.g. uint8_t)
 */
template <typename T>
class ScanResults {
public:
    /**
     * @brief Equality operator to check if a given device address is found in the ScanResults.
     *
     * @param toBeFound The device address to be checked
     * @return true If the device address is found in the ScanResults
     * @return false Otherwise
     */
    bool operator==(T toBeFound) {
        for (int i = 0; i < howMany; i++) {
            if (toBeFound == data[i]) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Inequality operator to check if a given device address is not found in the ScanResults.
     *
     * @param toBeFound The device address to be checked
     * @return true If the device address is not found in the ScanResults
     * @return false Otherwise
     */
    bool operator!=(T toBeFound) {
        return !(*this == toBeFound);
    }

    /**
     * @brief Add a device address to the ScanResults.
     *
     * @param obj The device address to be added
     */
    void push(T obj) {
        data[howMany++] = obj;
    }

private:
    T data[20];     /// Array to store the device addresses (max 20).
    int howMany = 0; /// Number of device addresses in the ScanResults
};


/**
 * @class Camera
 * @brief The main class for controlling a camera using the provided ImageSensor.
 */
class Camera {
    private:
        int32_t pixformat;       /// Pixel format
        int32_t resolution;      /// Camera resolution
        int32_t original_resolution;    /// The resolution originally set through setResolution()
        int32_t framerate;       /// Frame rate
        ImageSensor *sensor;     /// Pointer to the camera sensor
        int reset();             /// Reset the camera
        ScanResults<uint8_t> i2cScan(); /// Perform an I2C scan
        Stream *_debug;          /// Pointer to the debug stream
        arduino::MbedI2C *_i2c;  /// Pointer to the I2C interface
        FrameBuffer *_framebuffer; /// Pointer to the frame buffer
        int setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, int32_t zoom_x, int32_t zoom_y);


    public:
        /**
         * @brief Construct a new Camera object.
         *
         * @param sensor Reference to the ImageSensor object that is the driver for the camera sensor.
         */
        Camera(ImageSensor &sensor);

        /**
         * @brief Initialize the camera.
         *
         * @param resolution Initial resolution (default: CAMERA_R320x240). Note that not all resolutions are supported by all cameras.
         * @param pixformat Initial pixel format (default: CAMERA_GRAYSCALE). Note that not all pixel formats are supported by all cameras.
         * @param framerate Initial frame rate (default: 30)
         * @return true If the camera is successfully initialized
         * @return false Otherwise
         */
        bool begin(int32_t resolution=CAMERA_R320x240, int32_t pixformat=CAMERA_GRAYSCALE, int32_t framerate=30);

        /**
         * @brief Get the I2C address of the image sensor.
         * @return int The sensor ID
         */
        int getID();
        
        /**
         * @brief Set the frame rate of the image sensor.
         * 
         * @note This has no effect on cameras that do not support variable frame rates.
         * @param framerate The desired frame rate in frames per second
         * @return int 0 on success, non-zero on failure
         */
        int setFrameRate(int32_t framerate);
        
        /**
         * @brief Set the resolution of the image sensor.
         * 
         * @note This has no effect on cameras that do not support variable resolutions.
         * @param resolution The desired resolution, as defined in the resolution enum
         * @return int 0 on success, non-zero on failure
         */
        int setResolution(int32_t resolution);
        
        /**
         * @brief Set the pixel (color) format of the image sensor.
         * 
         * @note This has no effect on cameras that do not support variable pixel formats.
         * e.g. the Himax HM01B0 only supports grayscale.
         * @param pixelformat The desired pixel format, as defined in the pixel format enum
         * @return int 0 on success, non-zero on failure
         */
        int setPixelFormat(int32_t pixelformat);

        /**
         * @brief Set the sensor in standby mode.
         * 
         * @note This has no effect on cameras that do not support standby mode.
         * @note None of the currently supported camera drivers implement this function.
         * The HM01B0 and HM0360 cameras can be set in standby mode by calling reset() instead.
         * @param enable true to enable standby mode, false to disable
         * @return int 0 on success, non-zero on failure (or not implemented)
         */
        int setStandby(bool enable);

        /**
         * @brief Set the test pattern mode for the sensor.
         *
         * @note This has no effect on cameras that do not support test pattern mode.
         * @param enable true to enable test pattern, false to disable
         * @param walking true for walking test pattern, false for other test pattern (if supported)
         * The walking test pattern alternates between black and white pixels which is useful for detecting stuck-at faults
         * @return int 0 on success, non-zero on failure (or not implemented)
         */
        int setTestPattern(bool enable, bool walking);

        /**
         * @brief Get the frame size. This is the number of bytes in a frame as determined by the resolution and pixel format.
         * 
         * @return int The frame size in bytes
         */
        int frameSize();

        /**
         * @brief Capture a frame.
         * 
         * @param fb Reference to a FrameBuffer object to store the frame data
         * @param timeout Time in milliseconds to wait for a frame (default: 5000)
         * @return int 0 if successful, non-zero otherwise
         */
        int grabFrame(FrameBuffer &fb, uint32_t timeout=5000);

        /**
         * @brief Enable motion detection with the specified callback.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @param callback Function to be called when motion is detected
         * @return int 0 on success, non-zero on failure
         */
        int enableMotionDetection(md_callback_t callback=NULL);

        /**
         * @brief Disable motion detection.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @return int 0 on success, non-zero on failure
         */
        int disableMotionDetection();

        /**
         * @brief Set the motion detection window.
         *
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @param x The x-coordinate of the window origin
         * @param y The y-coordinate of the window origin
         * @param w The width of the window
         * @param h The height of the window
         * @return int 0 on success, non-zero on failure
         */
        int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

        /**
         * @brief Set the motion detection threshold.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * On the Himax HM01B0, the recommended threshold range is 3 - 240 (0x03 to 0xF0).
         * @param threshold The motion detection threshold
         * @return int 0 on success, non-zero on failure
         */
        int setMotionDetectionThreshold(uint32_t threshold);

        /**
         * @brief Check if motion was detected and clear the motion detection flag.
         * 
         * @note This has no effect on cameras that do not support motion detection. 
         * Currently only the Himax HM01B0 and HM0360 support motion detection.
         * @note This function must be called after the motion detection callback was executed to clear the motion detection flag.
         * @return int 0 if no motion is detected, non-zero if motion is detected
         */
        int motionDetected();

        /**
         * @brief Zoom to a specific region of the image by setting the zoom window size and its position.
         * The camera resolution must be set to a higher resolution than the zoom resolution for this to work.
         * The zooming is done by cropping a higher resolution image to the zoom window.
         * @note This function is currently only supported by the GC2145 sensor on the Arduino Nicla Vision.
         * @param zoom_resolution The resolution of the zoom window. 
         * The resolution must be one of the following:
         * - CAMERA_R160x120
         * - CAMERA_R320x240
         * - CAMERA_R320x320
         * - CAMERA_R640x480
         * - CAMERA_R800x600
         * If the desired resolution doesn't fit in the built-in memory, 
         * the framebuffer should be allocated on external RAM.
         * @param zoom_x The x position of the zoom window. 
         * The value must be lower or equal to the width of the image minus the width of the zoom window.
         * @param zoom_y The y position of the zoom window. 
         * The value must be lower or equal to the height of the image minus the height of the zoom window.
         * @return 0 on success, -1 on failure.
         */
        int zoomTo(int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y);

        /**
         * @brief Zoom to the center of the image by setting the zoom window size.
         * 
         * @param zoom_resolution The resolution of the zoom window. 
         * The resolution must be one of the following:
         * - CAMERA_R160x120
         * - CAMERA_R320x240
         * - CAMERA_R320x320
         * - CAMERA_R640x480
         * - CAMERA_R800x600
         * If the desired resolution doesn't fit in the built-in memory, 
         * the framebuffer should be allocated on external RAM.
         * @return 0 on success, -1 on failure.
         */
        int zoomToCenter(int32_t zoom_resolution);

        /**
         * @brief Flips the camera image vertically.
         * 
         * @param flip_enable Set to true to enable vertical flip, false to disable.
         * @return 0 on success, -1 on failure.
         */
        int setVerticalFlip(bool flip_enable);

        /**
         * @brief Mirrors the camera image horizontally.
         * 
         * @param mirror_enable Set to true to enable horizontal mirror, false to disable.
         * @return 0 on success, -1 on failure.
         */
        int setHorizontalMirror(bool mirror_enable);

        /**
         * @brief Get the width of the current camera resolution.
         * This can for example be used to calculate the zoom window position and size.
         * In the following example, the camera is zoomed to the top right side of the image:
         * @code
         * // Calculate the zoom window position
         * uint32_t max_zoom_x = camera.getResolutionWidth() - 320;
         * // Zoom to the calculated position and size
         * camera.zoomTo(CAMERA_R320x240, max_zoom_x, 0);
         * @endcode
         * @return uint32_t The width of the camera resolution.
         */
        uint32_t getResolutionWidth();

        /**
         * @brief Get the height of the current camera resolution.
         * This can for example be used to calculate the zoom window position and size.
         * In the following example, the camera is zoomed to the bottom left side of the image:
         * @code
         * // Calculate the zoom window position
         * uint32_t max_zoom_y = camera.getResolutionHeight() - 240;
         * // Zoom to the calculated position and size
         * camera.zoomTo(CAMERA_R320x240, 0, max_zoom_y);
         * @endcode
         * @return uint32_t The height of the camera resolution.
         */
        uint32_t getResolutionHeight();

        /**
         * @brief Output debug information to a stream.
         * You can use this function to output debug information to the serial port by passing Serial as the stream.
         * @param stream Stream to output the debug information
         */
        void debug(Stream &stream);

};

#endif // __CAMERA_H

/// The I2C bus used to communicate with the camera
extern arduino::MbedI2C CameraWire;
