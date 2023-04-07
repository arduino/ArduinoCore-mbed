# Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class` [`Camera`](#class-camera) | The main class for controlling a camera using the provided [ImageSensor](#class-imagesensor).
`class` [`FrameBuffer`](#class-framebuffer) | Frame buffer class for storing captured frames.
`class` [`ImageSensor`](#class-imagesensor) | Abstract base class for image sensor drivers.
`class` [`ScanResults`](#class-scanresults) | A template class used to store the results from an I2C scan.

# class `Camera` 

The main class for controlling a camera using the provided [ImageSensor](#).

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public ` [`Camera`](#)`(`[`ImageSensor`](#)` & sensor)` | Pointer to the frame buffer.
`public bool` [`begin`](#)`(int32_t resolution,int32_t pixformat,int32_t framerate)` | Initialize the camera.
`public int` [`getID`](#)`()` | Get the I2C address of the image sensor.
`public int` [`setFrameRate`](#)`(int32_t framerate)` | Set the frame rate of the image sensor.
`public int` [`setResolution`](#)`(int32_t resolution)` | Set the resolution of the image sensor.
`public int` [`setPixelFormat`](#)`(int32_t pixelformat)` | Set the pixel (color) format of the image sensor.
`public int` [`setStandby`](#)`(bool enable)` | Set the sensor in standby mode.
`public int` [`setTestPattern`](#)`(bool enable,bool walking)` | Set the test pattern mode for the sensor.
`public int` [`frameSize`](#)`()` | Get the frame size. This is the number of bytes in a frame as determined by the resolution and pixel format.
`public int` [`grabFrame`](#)`(`[`FrameBuffer`](#)` & fb,uint32_t timeout)` | Capture a frame.
`public int` [`enableMotionDetection`](#)`(`[`md_callback_t`](#)` callback)` | Enable motion detection with the specified callback.
`public int` [`disableMotionDetection`](#)`()` | Disable motion detection.
`public int` [`setMotionDetectionWindow`](#)`(uint32_t x,uint32_t y,uint32_t w,uint32_t h)` | Set the motion detection window.
`public int` [`setMotionDetectionThreshold`](#)`(uint32_t threshold)` | Set the motion detection threshold. On the Himax HM01B0, the recommended threshold range is 3 - 240 (0x03 to 0xF0).
`public int` [`motionDetected`](#)`()` | Check if motion was detected and clear the motion detection flag.
`public void` [`debug`](#)`(Stream & stream)` | Output debug information to a stream. You can use this function to output debug information to the serial port by passing Serial as the stream.

## Members

### `public ` [`Camera`](#)`(`[`ImageSensor`](#)` & sensor)` 

Pointer to the frame buffer.

Construct a new [Camera](#) object.

#### Parameters
* `sensor` Reference to the [ImageSensor](#) object that is the driver for the camera sensor.

### `public bool` [`begin`](#)`(int32_t resolution,int32_t pixformat,int32_t framerate)` 

Initialize the camera.

#### Parameters
* `resolution` Initial resolution (default: CAMERA_R320x240). Note that not all resolutions are supported by all cameras. 

* `pixformat` Initial pixel format (default: CAMERA_GRAYSCALE). Note that not all pixel formats are supported by all cameras. 

* `framerate` Initial frame rate (default: 30) 

#### Returns
true If the camera is successfully initialized 

#### Returns
false Otherwise

### `public int` [`getID`](#)`()` 

Get the I2C address of the image sensor.

#### Returns
int The sensor ID

### `public int` [`setFrameRate`](#)`(int32_t framerate)` 

Set the frame rate of the image sensor.

This has no effect on cameras that do not support variable frame rates. 

#### Parameters
* `framerate` The desired frame rate in frames per second 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setResolution`](#)`(int32_t resolution)` 

Set the resolution of the image sensor.

This has no effect on cameras that do not support variable resolutions. 

#### Parameters
* `resolution` The desired resolution, as defined in the resolution enum 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setPixelFormat`](#)`(int32_t pixelformat)` 

Set the pixel (color) format of the image sensor.

This has no effect on cameras that do not support variable pixel formats. e.g. the Himax HM01B0 only supports grayscale. 

#### Parameters
* `pixelformat` The desired pixel format, as defined in the pixel format enum 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setStandby`](#)`(bool enable)` 

Set the sensor in standby mode.

This has no effect on cameras that do not support standby mode. 

#### Parameters
* `enable` true to enable standby mode, false to disable 

#### Returns
int 0 on success, non-zero on failure (or not implemented)

### `public int` [`setTestPattern`](#)`(bool enable,bool walking)` 

Set the test pattern mode for the sensor.

#### Parameters
* `enable` true to enable test pattern, false to disable 

* `walking` true for walking test pattern, false for other test pattern (if supported) The walking test pattern alternates between black and white pixels which is useful for detecting stuck-at faults 

#### Returns
int 0 on success, non-zero on failure (or not implemented)

### `public int` [`frameSize`](#)`()` 

Get the frame size. This is the number of bytes in a frame as determined by the resolution and pixel format.

#### Returns
int The frame size in bytes

### `public int` [`grabFrame`](#)`(`[`FrameBuffer`](#)` & fb,uint32_t timeout)` 

Capture a frame.

#### Parameters
* `fb` Reference to a [FrameBuffer](#) object to store the frame data 

* `timeout` Time in milliseconds to wait for a frame (default: 5000) 

#### Returns
int 0 if successful, non-zero otherwise

### `public int` [`enableMotionDetection`](#)`(`[`md_callback_t`](#)` callback)` 

Enable motion detection with the specified callback.

This has no effect on cameras that do not support motion detection. 

#### Parameters
* `callback` Function to be called when motion is detected 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`disableMotionDetection`](#)`()` 

Disable motion detection.

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setMotionDetectionWindow`](#)`(uint32_t x,uint32_t y,uint32_t w,uint32_t h)` 

Set the motion detection window.

#### Parameters
* `x` The x-coordinate of the window origin 

* `y` The y-coordinate of the window origin 

* `w` The width of the window 

* `h` The height of the window 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setMotionDetectionThreshold`](#)`(uint32_t threshold)` 

Set the motion detection threshold. On the Himax HM01B0, the recommended threshold range is 3 - 240 (0x03 to 0xF0).

#### Parameters
* `threshold` The motion detection threshold 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`motionDetected`](#)`()` 

Check if motion was detected and clear the motion detection flag.

This function must be called after the motion detection callback was executed to clear the motion detection flag. 

#### Returns
int 0 if no motion is detected, non-zero if motion is detected

### `public void` [`debug`](#)`(Stream & stream)` 

Output debug information to a stream. You can use this function to output debug information to the serial port by passing Serial as the stream.

#### Parameters
* `stream` Stream to output the debug information

# class `FrameBuffer` 

Frame buffer class for storing captured frames.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public ` [`FrameBuffer`](#)`(int32_t x,int32_t y,int32_t bpp)` | Flag indicating if the buffer is allocated on the heap.
`public ` [`FrameBuffer`](#)`(int32_t address)` | Construct a new [FrameBuffer](#) object with a given address. This is useful if a resolution higher than 320x240 is required and external RAM shall be used. In thast case, the following code can be used:
`public ` [`FrameBuffer`](#)`()` | Construct a new [FrameBuffer](#) object with no custom size or address.
`public uint32_t` [`getBufferSize`](#)`()` | Get the buffer size in bytes.
`public uint8_t *` [`getBuffer`](#)`()` | Get a pointer to the frame buffer. This can be used to read the pixels from the frame buffer. E.g. to print all pixels to the serial monitor as hex values:
`public void` [`setBuffer`](#)`(uint8_t * buffer)` | Set the frame buffer pointer.
`public bool` [`hasFixedSize`](#)`()` | Check if the frame buffer has a fixed size. This is the case if the frame buffer is constructed with a width, height, and bits per pixel.
`public bool` [`isAllocated`](#)`()` | Check if the frame buffer is allocated on the heap.

## Members

### `public ` [`FrameBuffer`](#)`(int32_t x,int32_t y,int32_t bpp)` 

Flag indicating if the buffer is allocated on the heap.

Construct a new [FrameBuffer](#) object with a fixed size.

#### Parameters
* `x` Width of the frame buffer 

* `y` Height of the frame buffer 

* `bpp` Bits per pixel

### `public ` [`FrameBuffer`](#)`(int32_t address)` 

Construct a new [FrameBuffer](#) object with a given address. This is useful if a resolution higher than 320x240 is required and external RAM shall be used. In thast case, the following code can be used:

```cpp
#include "SDRAM.h"
[FrameBuffer](#) fb(SDRAM_START_ADDRESS);
...
// In setup() add:
SDRAM.begin();
```

#### Parameters
* `address` The memory address of the frame buffer

### `public ` [`FrameBuffer`](#)`()` 

Construct a new [FrameBuffer](#) object with no custom size or address.

### `public uint32_t` [`getBufferSize`](#)`()` 

Get the buffer size in bytes.

#### Returns
uint32_t The buffer size in bytes

### `public uint8_t *` [`getBuffer`](#)`()` 

Get a pointer to the frame buffer. This can be used to read the pixels from the frame buffer. E.g. to print all pixels to the serial monitor as hex values:

```cpp
uint8_t *fb = fb.getBuffer();
for (int i = 0; i < fb.getBufferSize(); i++) {
   Serial.print(fb[i], HEX);
  Serial.print(" ");
}
```

#### Returns
uint8_t* Pointer to the frame buffer

### `public void` [`setBuffer`](#)`(uint8_t * buffer)` 

Set the frame buffer pointer.

#### Parameters
* `buffer` Pointer to the frame buffer

### `public bool` [`hasFixedSize`](#)`()` 

Check if the frame buffer has a fixed size. This is the case if the frame buffer is constructed with a width, height, and bits per pixel.

#### Returns
true If the frame buffer has a fixed size 

#### Returns
false Otherwise

### `public bool` [`isAllocated`](#)`()` 

Check if the frame buffer is allocated on the heap.

#### Returns
true If the frame buffer is allocated 

#### Returns
false Otherwise

# class `ImageSensor` 

Abstract base class for image sensor drivers.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual ` [`~ImageSensor`](#)`()` | 
`public int` [`init`](#)`()` | Initialize the image sensor. This prepares the sensor for capturing frames by setting the registers to their default values.
`public int` [`reset`](#)`()` | Reset the image sensor. This is useful if the sensor is stuck.
`public int` [`getID`](#)`()` | Get the I2C address of the image sensor.
`public bool` [`getMono`](#)`()` | Check if the image sensor is monochrome aka grayscale.
`public uint32_t` [`getClockFrequency`](#)`()` | Get the clock frequency of the image sensor.
`public int` [`setFrameRate`](#)`(int32_t framerate)` | Set the frame rate of the image sensor.
`public int` [`setResolution`](#)`(int32_t resolution)` | Set the resolution of the image sensor.
`public int` [`setPixelFormat`](#)`(int32_t pixelformat)` | Set the pixel (color) format of the image sensor.
`public int` [`enableMotionDetection`](#)`(`[`md_callback_t`](#)` callback)` | Enable motion detection with the specified callback.
`public int` [`disableMotionDetection`](#)`()` | Disable motion detection.
`public int` [`setMotionDetectionWindow`](#)`(uint32_t x,uint32_t y,uint32_t w,uint32_t h)` | Set the motion detection window.
`public int` [`setMotionDetectionThreshold`](#)`(uint32_t threshold)` | Set the motion detection threshold. On the Himax HM01B0, the recommended threshold range is 3 - 240 (0x03 to 0xF0).
`public int` [`motionDetected`](#)`()` | Check if motion was detected and clear the motion detection flag.
`public void` [`debug`](#)`(Stream & stream)` | Output debug information to a stream. You can use this function to output debug information to the serial port by passing Serial as the stream.
`public inline virtual int` [`setStandby`](#)`(bool enable)` | Set the sensor in standby mode.
`public inline virtual int` [`setTestPattern`](#)`(bool enable,bool walking)` | Set the test pattern mode for the sensor.

## Members

### `public inline virtual ` [`~ImageSensor`](#)`()` 

### `public int` [`init`](#)`()` 

Initialize the image sensor. This prepares the sensor for capturing frames by setting the registers to their default values.

#### Returns
int 0 on success, non-zero on failure

### `public int` [`reset`](#)`()` 

Reset the image sensor. This is useful if the sensor is stuck.

#### Returns
int 0 on success, non-zero on failure

### `public int` [`getID`](#)`()` 

Get the I2C address of the image sensor.

#### Returns
int The sensor ID

### `public bool` [`getMono`](#)`()` 

Check if the image sensor is monochrome aka grayscale.

#### Returns
true If the sensor is monochrome 

#### Returns
false Otherwise

### `public uint32_t` [`getClockFrequency`](#)`()` 

Get the clock frequency of the image sensor.

#### Returns
uint32_t The clock frequency in Hz

### `public int` [`setFrameRate`](#)`(int32_t framerate)` 

Set the frame rate of the image sensor.

This has no effect on cameras that do not support variable frame rates. 

#### Parameters
* `framerate` The desired frame rate in frames per second 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setResolution`](#)`(int32_t resolution)` 

Set the resolution of the image sensor.

This has no effect on cameras that do not support variable resolutions. 

#### Parameters
* `resolution` The desired resolution, as defined in the resolution enum 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setPixelFormat`](#)`(int32_t pixelformat)` 

Set the pixel (color) format of the image sensor.

This has no effect on cameras that do not support variable pixel formats. e.g. the Himax HM01B0 only supports grayscale. 

#### Parameters
* `pixelformat` The desired pixel format, as defined in the pixel format enum 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`enableMotionDetection`](#)`(`[`md_callback_t`](#)` callback)` 

Enable motion detection with the specified callback.

This has no effect on cameras that do not support motion detection. 

#### Parameters
* `callback` Function to be called when motion is detected 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`disableMotionDetection`](#)`()` 

Disable motion detection.

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setMotionDetectionWindow`](#)`(uint32_t x,uint32_t y,uint32_t w,uint32_t h)` 

Set the motion detection window.

#### Parameters
* `x` The x-coordinate of the window origin 

* `y` The y-coordinate of the window origin 

* `w` The width of the window 

* `h` The height of the window 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`setMotionDetectionThreshold`](#)`(uint32_t threshold)` 

Set the motion detection threshold. On the Himax HM01B0, the recommended threshold range is 3 - 240 (0x03 to 0xF0).

#### Parameters
* `threshold` The motion detection threshold 

#### Returns
int 0 on success, non-zero on failure

### `public int` [`motionDetected`](#)`()` 

Check if motion was detected and clear the motion detection flag.

This function must be called after the motion detection callback was executed to clear the motion detection flag. 

#### Returns
int 0 if no motion is detected, non-zero if motion is detected

### `public void` [`debug`](#)`(Stream & stream)` 

Output debug information to a stream. You can use this function to output debug information to the serial port by passing Serial as the stream.

#### Parameters
* `stream` Stream to output the debug information

### `public inline virtual int` [`setStandby`](#)`(bool enable)` 

Set the sensor in standby mode.

This has no effect on cameras that do not support standby mode. 

#### Parameters
* `enable` true to enable standby mode, false to disable 

#### Returns
int 0 on success, non-zero on failure (or not implemented)

### `public inline virtual int` [`setTestPattern`](#)`(bool enable,bool walking)` 

Set the test pattern mode for the sensor.

#### Parameters
* `enable` true to enable test pattern, false to disable 

* `walking` true for walking test pattern, false for other test pattern (if supported) The walking test pattern alternates between black and white pixels which is useful for detecting stuck-at faults 

#### Returns
int 0 on success, non-zero on failure (or not implemented)

# class `ScanResults` 

A template class used to store the results from an I2C scan.

#### Parameters
* `T` Data type for the device address (e.g. uint8_t)

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline bool` [`operator==`](#)`(T toBeFound)` | Equality operator to check if a given device address is found in the [ScanResults](#).
`public inline bool` [`operator!=`](#)`(T toBeFound)` | Inequality operator to check if a given device address is not found in the [ScanResults](#).
`public inline void` [`push`](#)`(T obj)` | Add a device address to the [ScanResults](#).

## Members

### `public inline bool` [`operator==`](#)`(T toBeFound)` 

Equality operator to check if a given device address is found in the [ScanResults](#).

#### Parameters
* `toBeFound` The device address to be checked 

#### Returns
true If the device address is found in the [ScanResults](#)

#### Returns
false Otherwise

### `public inline bool` [`operator!=`](#)`(T toBeFound)` 

Inequality operator to check if a given device address is not found in the [ScanResults](#).

#### Parameters
* `toBeFound` The device address to be checked 

#### Returns
true If the device address is not found in the [ScanResults](#)

#### Returns
false Otherwise

### `public inline void` [`push`](#)`(T obj)` 

Add a device address to the [ScanResults](#).

#### Parameters
* `obj` The device address to be added
