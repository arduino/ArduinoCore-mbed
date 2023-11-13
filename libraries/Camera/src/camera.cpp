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
#include "Arduino.h"
#include "camera.h"
#include "Wire.h"
#include "stm32h7xx_hal_dcmi.h"

// Workaround for the broken UNUSED macro.
#undef UNUSED
#define UNUSED(x) ((void)((uint32_t)(x)))

#define ALIGN_PTR(p,a)   ((p & (a-1)) ?(((uintptr_t)p + a) & ~(uintptr_t)(a-1)) : p)

// Include all image sensor drivers here.
#if defined (ARDUINO_PORTENTA_H7_M7)

#define DCMI_TIM                    (TIM1)
#define DCMI_TIM_PIN                (GPIO_PIN_1)
#define DCMI_TIM_PORT               (GPIOK)
#define DCMI_TIM_AF                 (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL            (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()       __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()      __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()        HAL_RCC_GetPCLK2Freq()
#define DCMI_TIM_FREQUENCY          (6000000)
#define DCMI_RESET_PIN              (PC_13)
arduino::MbedI2C CameraWire(I2C_SDA, I2C_SCL);

#elif defined(ARDUINO_NICLA_VISION)

#define DCMI_TIM                    (TIM3)
#define DCMI_TIM_PIN                (GPIO_PIN_7)
#define DCMI_TIM_PORT               (GPIOA)
#define DCMI_TIM_AF                 (GPIO_AF2_TIM3)
#define DCMI_TIM_CHANNEL            (TIM_CHANNEL_2)
#define DCMI_TIM_CLK_ENABLE()       __TIM3_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()      __TIM3_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()        HAL_RCC_GetPCLK1Freq()
#define DCMI_TIM_FREQUENCY          (12000000)
arduino::MbedI2C CameraWire(I2C_SDA2, I2C_SCL2);

#elif defined(ARDUINO_GIGA)

#define DCMI_TIM                    (TIM1)
#define DCMI_TIM_PIN                (GPIO_PIN_9)
#define DCMI_TIM_PORT               (GPIOJ)
#define DCMI_TIM_AF                 (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL            (TIM_CHANNEL_3)
#define DCMI_TIM_CLK_ENABLE()       __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()      __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()        HAL_RCC_GetPCLK2Freq()
#define DCMI_TIM_FREQUENCY          (6000000)
#define DCMI_RESET_PIN              (PA_1)
arduino::MbedI2C CameraWire(I2C_SDA1, I2C_SCL1);

#endif

#define DCMI_IRQ_PRI                NVIC_EncodePriority(NVIC_PRIORITYGROUP_4, 2, 0)

#define DCMI_DMA_CLK_ENABLE()       __HAL_RCC_DMA2_CLK_ENABLE()
#define DCMI_DMA_STREAM             DMA2_Stream3
#define DCMI_DMA_IRQ                DMA2_Stream3_IRQn
#define DCMI_DMA_IRQ_PRI            NVIC_EncodePriority(NVIC_PRIORITYGROUP_4, 3, 0)

// DCMI GPIO pins struct
static const struct { GPIO_TypeDef *port; uint16_t pin; } dcmi_pins[] = {
    #if defined (ARDUINO_PORTENTA_H7_M7)
    {GPIOA,     GPIO_PIN_4  },
    {GPIOA,     GPIO_PIN_6  },
    {GPIOI,     GPIO_PIN_4  },
    {GPIOI,     GPIO_PIN_5  },
    {GPIOI,     GPIO_PIN_6  },
    {GPIOI,     GPIO_PIN_7  },
    {GPIOH,     GPIO_PIN_9  },
    {GPIOH,     GPIO_PIN_10 },
    {GPIOH,     GPIO_PIN_11 },
    {GPIOH,     GPIO_PIN_12 },
    {GPIOH,     GPIO_PIN_14 },
    #elif defined(ARDUINO_NICLA_VISION)
    {GPIOA,     GPIO_PIN_4  },
    {GPIOA,     GPIO_PIN_6  },
    {GPIOC,     GPIO_PIN_6  },
    {GPIOC,     GPIO_PIN_7  },
    {GPIOD,     GPIO_PIN_3  },
    {GPIOE,     GPIO_PIN_0  },
    {GPIOE,     GPIO_PIN_1  },
    {GPIOE,     GPIO_PIN_4 },
    {GPIOE,     GPIO_PIN_5 },
    {GPIOE,     GPIO_PIN_6 },
    {GPIOG,     GPIO_PIN_9 },
    #elif defined(ARDUINO_GIGA)
    {GPIOH,     GPIO_PIN_9  },
    {GPIOH,     GPIO_PIN_10  },
    {GPIOH,     GPIO_PIN_11  },
    {GPIOG,     GPIO_PIN_11  },
    {GPIOH,     GPIO_PIN_14  },
    {GPIOI,     GPIO_PIN_4  },
    {GPIOI,     GPIO_PIN_6  },
    {GPIOI,     GPIO_PIN_7 },
    {GPIOA,     GPIO_PIN_6 },
    {GPIOH,     GPIO_PIN_8 },
    {GPIOI,     GPIO_PIN_5 },
    #endif
};
#define NUM_DCMI_PINS   (sizeof(dcmi_pins)/sizeof(dcmi_pins[0]))

static TIM_HandleTypeDef  htim  = {0};
static DMA_HandleTypeDef  hdma  = {0};
static DCMI_HandleTypeDef hdcmi = {0};

/// Table to store the amount of bytes per pixel for each pixel format
const uint32_t pixtab[CAMERA_PMAX] = {
    1, // CAMERA_GRAYSCALE
    1, // CAMERA_BAYER
    2, // CAMERA_RGB565
};

/// Table to store the resolution width and height for each resolution
const uint32_t restab[CAMERA_RMAX][2] = {
    {160,   120 }, // QQVGA
    {320,   240 }, // QVGA
    {320,   320 },
    {640,   480 }, // VGA
    {0,       0 }, // Empty entry because there's a jump in the resolution enum initializers
    {800,   600 }, // SVGA
    {1600,  1200}, // UXGA
};

extern "C" {

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == DCMI_TIM) {
        // Enable DCMI timer clock
        DCMI_TIM_CLK_ENABLE();

        // Timer GPIO configuration
        GPIO_InitTypeDef  hgpio;
        hgpio.Pin       = DCMI_TIM_PIN;
        hgpio.Pull      = GPIO_NOPULL;
        hgpio.Speed     = GPIO_SPEED_HIGH;
        hgpio.Mode      = GPIO_MODE_AF_PP;
        hgpio.Alternate = DCMI_TIM_AF;
        HAL_GPIO_Init(DCMI_TIM_PORT, &hgpio);
    }
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef *hdcmi)
{
    // Enable DCMI clock
    __HAL_RCC_DCMI_CLK_ENABLE();

    // Enable DCMI GPIO clocks
    GPIO_InitTypeDef hgpio;

    // Configure DCMI GPIOs
    hgpio.Mode      = GPIO_MODE_AF_PP;
    hgpio.Pull      = GPIO_PULLUP;
    hgpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    hgpio.Alternate = GPIO_AF13_DCMI;

    #if defined (ARDUINO_PORTENTA_H7_M7)
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    #elif defined(ARDUINO_NICLA_VISION)
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    #elif defined(ARDUINO_GIGA)
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    #endif

    for (uint32_t i=0; i<NUM_DCMI_PINS; i++) {
        hgpio.Pin = dcmi_pins[i].pin;
        HAL_GPIO_Init(dcmi_pins[i].port, &hgpio);
    }
}

void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef* hdcmi)
{
    // Disable DCMI IRQs.
    HAL_NVIC_DisableIRQ(DCMI_IRQn);

    // Disable DMA IRQs.
    HAL_NVIC_DisableIRQ(DCMI_DMA_IRQ);

    // Deinit the DMA stream.
    if (hdcmi->DMA_Handle != NULL) {
        HAL_DMA_DeInit(hdcmi->DMA_Handle);
    }

    // Disable DCMI clock.
    __HAL_RCC_DCMI_CLK_DISABLE();

    // Deinit DCMI GPIOs.
    for (uint32_t i=0; i<NUM_DCMI_PINS; i++) {
        HAL_GPIO_DeInit(dcmi_pins[i].port, dcmi_pins[i].pin);
    }
}

__weak int camera_extclk_config(int frequency)
{
    // TCLK (PCLK * 2).
    uint32_t tclk = DCMI_TIM_PCLK_FREQ() * 2;

    // Period should be even.
    uint32_t period = (tclk / frequency) - 1;

    if (htim.Init.Period && (htim.Init.Period != period)) {
        __HAL_TIM_SET_AUTORELOAD(&htim, period);
        __HAL_TIM_SET_COMPARE(&htim, DCMI_TIM_CHANNEL, period / 2);
        return 0;
    }

    // Timer base configuration.
    htim.Instance               = DCMI_TIM;
    htim.Init.Period            = period;
    htim.Init.Prescaler         = 0;
    htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim.Init.RepetitionCounter = 0;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    // Timer channel configuration.
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse           = period / 2;
    TIMOCHandle.OCMode          = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity      = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCNPolarity     = TIM_OCNPOLARITY_HIGH;
    TIMOCHandle.OCFastMode      = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState     = TIM_OCIDLESTATE_RESET;
    TIMOCHandle.OCNIdleState    = TIM_OCNIDLESTATE_RESET;

    // Init, config and start the timer.
    if ((HAL_TIM_PWM_Init(&htim) != HAL_OK)
    || (HAL_TIM_PWM_ConfigChannel(&htim, &TIMOCHandle, DCMI_TIM_CHANNEL) != HAL_OK)
    || (HAL_TIM_PWM_Start(&htim, DCMI_TIM_CHANNEL) != HAL_OK)) {
        return -1;
    }

    return 0;
}

uint8_t camera_dcmi_config(bool bsm_skip)
{
    // DMA Stream configuration
    hdma.Instance                 = DCMI_DMA_STREAM;
    hdma.Init.Request             = DMA_REQUEST_DCMI;
    hdma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma.Init.MemInc              = DMA_MINC_ENABLE;
    hdma.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode                = DMA_NORMAL;
    hdma.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma.Init.MemBurst            = DMA_MBURST_INC4;
    hdma.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    // Enable DMA clock
    DCMI_DMA_CLK_ENABLE();

    // Initialize the DMA stream
    HAL_DMA_Init(&hdma);

    // Configure and enable DMA IRQ Channel
    NVIC_SetPriority(DCMI_DMA_IRQ, DCMI_DMA_IRQ_PRI);
    HAL_NVIC_EnableIRQ(DCMI_DMA_IRQ);

    // Configure the DCMI interface.
    hdcmi.Instance              = DCMI;
    hdcmi.Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.VSPolarity       = DCMI_VSPOLARITY_LOW;
    hdcmi.Init.PCKPolarity      = DCMI_PCKPOLARITY_FALLING;
    hdcmi.Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.CaptureRate      = DCMI_CR_ALL_FRAME;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.JPEGMode         = DCMI_JPEG_DISABLE;
    hdcmi.Init.ByteSelectMode   = bsm_skip ? DCMI_BSM_OTHER : DCMI_BSM_ALL;
    hdcmi.Init.ByteSelectStart  = DCMI_OEBS_ODD;    // Ignored unless BSM != ALL
    hdcmi.Init.LineSelectMode   = DCMI_LSM_ALL;     // Capture all received lines
    hdcmi.Init.LineSelectStart  = DCMI_OELS_ODD;    // Ignored, unless LSM != ALL

    // Link the DMA handle to the DCMI handle.
    __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma);

    // Initialize the DCMI
    HAL_DCMI_Init(&hdcmi);
    __HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_LINE);

    // Configure and enable DCMI IRQ Channel
    NVIC_SetPriority(DCMI_IRQn, DCMI_IRQ_PRI);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    return 0;
}

void DCMI_IRQHandler(void)
{
    HAL_DCMI_IRQHandler(&hdcmi);
}

void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma);
}

} // extern "C"

FrameBuffer::FrameBuffer(int32_t x, int32_t y, int32_t bpp) : 
    _fb_size(x*y*bpp),
    _isAllocated(true)
{
    uint8_t *buffer = (uint8_t *)malloc(x*y*bpp);
    _fb = (uint8_t *)ALIGN_PTR((uintptr_t)buffer, 32);
}

FrameBuffer::FrameBuffer(int32_t address) : 
    _fb_size(0),
    _isAllocated(true)
{
    _fb = (uint8_t *)ALIGN_PTR((uintptr_t)address, 32);
}

FrameBuffer::FrameBuffer() : 
    _fb_size(0),
    _isAllocated(false)
{
}

uint32_t FrameBuffer::getBufferSize()
{
    return _fb_size;
}

uint8_t* FrameBuffer::getBuffer()
{
    return _fb;
}

void FrameBuffer::setBuffer(uint8_t *buffer)
{
    _isAllocated = true;
    _fb = buffer;
}

bool FrameBuffer::hasFixedSize()
{
    if (_fb_size) {
        return true;
    }
    return false;
}

bool FrameBuffer::isAllocated()
{
    return _isAllocated;
}

Camera::Camera(ImageSensor &sensor) : 
    pixformat(-1),
    resolution(-1),
    framerate(-1),
    sensor(&sensor),
    _debug(NULL)
{
}

int Camera::reset()
{
    // Reset sensor.
    #if defined(DCMI_RESET_PIN)
    digitalWrite(DCMI_RESET_PIN, LOW);
    HAL_Delay(10);

    digitalWrite(DCMI_RESET_PIN, HIGH);
    HAL_Delay(20);
    #endif
    return 0;
}

ScanResults<uint8_t> Camera::i2cScan()
{
    uint8_t addr;
    ScanResults<uint8_t> res;

    for (addr=1; addr<127; addr++) {
        CameraWire.beginTransmission(addr);
        if (CameraWire.endTransmission() == 0) {
            res.push(addr);
            if (_debug) {
                _debug->print("Sensor address: 0x");
                _debug->println(addr, HEX);
            }
        }
    }
    return res;
}

bool Camera::begin(int32_t resolution, int32_t pixformat, int32_t framerate)
{  
    if (resolution >= CAMERA_RMAX || pixformat >= CAMERA_PMAX) {
        return false;
    }

    // Configure the initial sensor clock.
    camera_extclk_config(DCMI_TIM_FREQUENCY);
    HAL_Delay(10);

    // Reset the image sensor.
    reset();

    if (sensor->getClockFrequency() != DCMI_TIM_FREQUENCY) {
        // Reconfigure the sensor clock frequency.
        camera_extclk_config(sensor->getClockFrequency());
        HAL_Delay(10);
    }

    if (this->sensor->init() != 0) {
        if (_debug) {
            _debug->print("Sensor init failed");
        }
        return false;
    }

    if (i2cScan() != this->sensor->getID()) {
        if (_debug) {
            _debug->print("Detected SensorID: 0x");
            _debug->println(this->sensor->getID(), HEX);
        }
        return false;
    }

    // If the pixel format is Grayscale and sensor is Not monochrome, the
    // actual pixel format will be YUV (i.e 2 bytes per pixel) and the DCMI
    // needs to be configured to skip every other byte to extract the Y channel.
    bool gs_from_yuv = (pixformat == CAMERA_GRAYSCALE) && !this->sensor->getMono();
    if (camera_dcmi_config(gs_from_yuv) != 0) {
        return false;
    }

    // NOTE: The pixel format must be set first before the resolution,
    // to lookup the BPP for this format to set the DCMI cropping.
    if (setPixelFormat(pixformat) != 0) {
        return false;
    }

    if (setResolution(resolution) != 0) {
        return false;
    }

    if (setFrameRate(framerate) != 0) {
        return false;
    }

    return true;
}

int Camera::getID()
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->getID();
}

int Camera::setFrameRate(int32_t framerate)
{
    if (this->sensor == NULL) {
        return -1;
    }

    if (this->sensor->setFrameRate(framerate) == 0) {
        this->framerate = framerate;
        return 0;
    }

    return -1;
}

int Camera::setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, int32_t zoom_x, int32_t zoom_y)
{
    if (this->sensor == NULL || resolution >= CAMERA_RMAX
            || pixformat >= CAMERA_PMAX || pixformat == -1) {
        return -1;
    }

    // resolution = the full resolution to set the camera to
    // zoom_resolution = the resolution to crop to when zooming (set equal to resolution for no zoom)
    // final_resolution = the resolution to crop to (depends on zoom or not)
    int32_t final_resolution;
    // Check if zooming is asked for
    if (resolution != zoom_resolution)
    {
        // Can't zoom into a larger window than the original
        if (zoom_resolution > resolution)
        {
            return -1;
        }
        final_resolution = zoom_resolution;
    }
    else
    {
        final_resolution = resolution;
    }

    /*
     * @param  X0    DCMI window X offset
     * @param  Y0    DCMI window Y offset
     * @param  XSize DCMI Pixel per line
     * @param  YSize DCMI Line number
     */
    HAL_DCMI_EnableCROP(&hdcmi);
    uint32_t bpl = restab[final_resolution][0];
    if (pixformat == CAMERA_RGB565 ||
       (pixformat == CAMERA_GRAYSCALE && !this->sensor->getMono())) {
        // If the pixel format is Grayscale and sensor is Not monochrome,
        // the actual pixel format will be YUV (i.e 2 bytes per pixel).
        bpl *= 2;
    }
    HAL_DCMI_ConfigCROP(&hdcmi, 0, 0, bpl - 1, restab[final_resolution][1] - 1);

    if (this->sensor->setResolutionWithZoom(resolution, zoom_resolution, zoom_x, zoom_y) == 0) {
        this->resolution = final_resolution;
        return 0;
    }
    return -1;
}

int Camera::setResolution(int32_t resolution)
{
    // Check for resolutions that would cause out-of-bounds indexing of restab
    // This check is here because original_resolution will be trusted in all other code
    if ((resolution < 0) || (resolution >= CAMERA_RMAX))
    {
        return -1;
    }
    original_resolution = resolution;
    return setResolutionWithZoom(resolution, resolution, 0, 0);
}

int Camera::zoomTo(int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y)
{
    // Check for zoom resolutions that would cause out-of-bounds indexing of restab
    if ((zoom_resolution < 0) || (zoom_resolution >= CAMERA_RMAX))
    {
        return -1;
    }
    // Check if the zoom window goes outside the frame on the x axis
    // Notice that this form prevents uint32_t wraparound, so don't change it
    if (zoom_x >= (restab[this->original_resolution][0]) - (restab[zoom_resolution][0]))
    {
         return -1;
    }
    // Check if the zoom window goes outside the frame on the y axis
    // Notice that this form prevents uint32_t wraparound, so don't change it
    if (zoom_y >= (restab[this->original_resolution][1]) - (restab[zoom_resolution][1]))
    {
        return -1;
    }
    return setResolutionWithZoom(this->original_resolution, zoom_resolution, zoom_x, zoom_y);
}

int Camera::zoomToCenter(int32_t zoom_resolution)
{
    // Check for zoom resolutions that would cause out-of-bounds indexing of restab
    if ((zoom_resolution < 0) || (zoom_resolution >= CAMERA_RMAX))
    {
        return -1;
    }
    uint32_t zoom_x = (restab[this->original_resolution][0] - restab[zoom_resolution][0]) / 2;
    uint32_t zoom_y = (restab[this->original_resolution][1] - restab[zoom_resolution][1]) / 2;
    return setResolutionWithZoom(this->original_resolution, zoom_resolution, zoom_x, zoom_y);
}

int Camera::setVerticalFlip(bool flip_enable)
{
    return (this->sensor->setVerticalFlip(flip_enable));
}

int Camera::setHorizontalMirror(bool mirror_enable)
{
    return (this->sensor->setHorizontalMirror(mirror_enable));
}

uint32_t Camera::getResolutionWidth()
{
    return (restab[this->original_resolution][0]);
}
uint32_t Camera::getResolutionHeight()
{
    return (restab[this->original_resolution][1]);
}

int Camera::setPixelFormat(int32_t pixformat)
{
    if (this->sensor == NULL || pixformat >= CAMERA_PMAX) {
        return -1;
    }

    if (this->sensor->setPixelFormat(pixformat) == 0) {
        this->pixformat = pixformat;
        return 0;
    }
    return -1;
}

int Camera::setStandby(bool enable)
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->setStandby(enable);
}

int Camera::setTestPattern(bool enable, bool walking)
{
    if (this->sensor == NULL) {
        return -1;
    }

    return this->sensor->setTestPattern(enable, walking);
}

int Camera::frameSize()
{
    if (this->sensor == NULL
            || this->pixformat == -1
            || this->resolution == -1) {
        return -1;
    }

    return restab[this->resolution][0] * restab[this->resolution][1] * pixtab[this->pixformat];
}

int Camera::grabFrame(FrameBuffer &fb, uint32_t timeout)
{
    if (this->sensor == NULL
            || this->pixformat == -1
            || this->resolution == -1) {
        return -1;
    }

    uint32_t framesize = frameSize();

    if (fb.isAllocated()) {
        //A buffer has already been allocated
        //Check buffer size
        if (fb.hasFixedSize()) {
            uint32_t fbSize = fb.getBufferSize();
            if (_debug) {
                _debug->print("fbSize: ");
                _debug->println(fbSize);
            }
            if (fbSize < framesize) {
                if (_debug) {
                    _debug->println("The allocated buffer is too small!");
                }
                return -1;
            }
        }
    } else {
        uint8_t *buffer = (uint8_t *)malloc(framesize+32);
        uint8_t *alignedBuff = (uint8_t *)ALIGN_PTR((uintptr_t)buffer, 32);
        fb.setBuffer(alignedBuff);
    }

    uint8_t *framebuffer = fb.getBuffer();

    // Ensure FB is aligned to 32 bytes cache lines.
    if ((uint32_t) framebuffer & 0x1F) {
        if (_debug) {
            _debug->println("Framebuffer not aligned to 32 bytes cache lines");
        }
        return -1;
    }

    // Start the Camera Snapshot Capture.
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT,
                (uint32_t) framebuffer, framesize / 4) != HAL_OK) {
        if (_debug) {
            _debug->println("HAL_DCMI_Start_DMA FAILED!");
        }
    }

    // Wait until camera frame is ready.
    for (uint32_t start = millis(); (hdcmi.Instance->CR & DCMI_CR_CAPTURE);) {
        __WFI();
        if ((millis() - start) > timeout) {
            if (_debug) {
                _debug->println("Timeout expired!");
            }
            HAL_DCMI_Stop(&hdcmi);
            return -1;
        }
    }

    HAL_DCMI_Stop(&hdcmi);

    #if defined(__CORTEX_M7)  // only invalidate buffer for Cortex M7
    // Invalidate buffer after DMA transfer.
    SCB_InvalidateDCache_by_Addr((uint32_t*) framebuffer, framesize);
    #endif
    return 0;
}

int Camera::setMotionDetectionThreshold(uint32_t threshold)
{
  return this->sensor->setMotionDetectionThreshold(threshold);
}

int Camera::setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  uint32_t width, height;

  width = restab[this->resolution][0];
  height= restab[this->resolution][1];

  if (((x+w) > width) || ((y+h) > height)) {
      return -1;
  }
  return this->sensor->setMotionDetectionWindow(x, y, x+w, y+h);
}

int Camera::enableMotionDetection(md_callback_t callback)
{
  return this->sensor->enableMotionDetection(callback);
}

int Camera::disableMotionDetection()
{
  return this->sensor->disableMotionDetection();
}

int Camera::motionDetected()
{
  return this->sensor->motionDetected();
}


void Camera::debug(Stream &stream)
{
  _debug = &stream;
  this->sensor->debug(stream);
}
