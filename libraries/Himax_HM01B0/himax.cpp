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
#include "Wire.h"
#include "himax.h"

// Register set
// Read only registers
#define MODEL_ID_H                  0x0000
#define MODEL_ID_L                  0x0001
#define FRAME_COUNT                 0x0005
#define PIXEL_ORDER                 0x0006
// R&W registers
// Sensor mode control
#define MODE_SELECT                 0x0100
#define IMG_ORIENTATION             0x0101
#define SW_RESET                    0x0103
#define GRP_PARAM_HOLD              0x0104
// Sensor exposure gain control
#define INTEGRATION_H               0x0202
#define INTEGRATION_L               0x0203
#define ANALOG_GAIN                 0x0205
#define DIGITAL_GAIN_H              0x020E
#define DIGITAL_GAIN_L              0x020F
// Frame timing control
#define FRAME_LEN_LINES_H           0x0340
#define FRAME_LEN_LINES_L           0x0341
#define LINE_LEN_PCK_H              0x0342
#define LINE_LEN_PCK_L              0x0343
// Binning mode control
#define READOUT_X                   0x0383
#define READOUT_Y                   0x0387
#define BINNING_MODE                0x0390
// Test pattern control
#define TEST_PATTERN_MODE           0x0601
// Black level control
#define BLC_CFG                     0x1000
#define BLC_TGT                     0x1003
#define BLI_EN                      0x1006
#define BLC2_TGT                    0x1007
//  Sensor reserved
#define DPC_CTRL                    0x1008
#define SINGLE_THR_HOT              0x100B
#define SINGLE_THR_COLD             0x100C
// VSYNC,HSYNC and pixel shift register
#define VSYNC_HSYNC_PIXEL_SHIFT_EN  0x1012
// Automatic exposure gain control
#define AE_CTRL                     0x2100
#define AE_TARGET_MEAN              0x2101
#define AE_MIN_MEAN                 0x2102
#define CONVERGE_IN_TH              0x2103
#define CONVERGE_OUT_TH             0x2104
#define MAX_INTG_H                  0x2105
#define MAX_INTG_L                  0x2106
#define MIN_INTG                    0x2107
#define MAX_AGAIN_FULL              0x2108
#define MAX_AGAIN_BIN2              0x2109
#define MIN_AGAIN                   0x210A
#define MAX_DGAIN                   0x210B
#define MIN_DGAIN                   0x210C
#define DAMPING_FACTOR              0x210D
#define FS_CTRL                     0x210E
#define FS_60HZ_H                   0x210F
#define FS_60HZ_L                   0x2110
#define FS_50HZ_H                   0x2111
#define FS_50HZ_L                   0x2112
#define FS_HYST_TH                  0x2113
// Motion detection control
#define MD_CTRL                     0x2150
#define I2C_CLEAR                   0x2153
#define WMEAN_DIFF_TH_H             0x2155
#define WMEAN_DIFF_TH_M             0x2156
#define WMEAN_DIFF_TH_L             0x2157
#define MD_THH                      0x2158
#define MD_THM1                     0x2159
#define MD_THM2                     0x215A
#define MD_THL                      0x215B
#define STATISTIC_CTRL              0x2000
#define MD_LROI_X_START_H           0x2011
#define MD_LROI_X_START_L           0x2012
#define MD_LROI_Y_START_H           0x2013
#define MD_LROI_Y_START_L           0x2014
#define MD_LROI_X_END_H             0x2015
#define MD_LROI_X_END_L             0x2016
#define MD_LROI_Y_END_H             0x2017
#define MD_LROI_Y_END_L             0x2018
#define MD_INTERRUPT                0x2160
//  Sensor timing control
#define QVGA_WIN_EN                 0x3010
#define SIX_BIT_MODE_EN             0x3011
#define PMU_AUTOSLEEP_FRAMECNT      0x3020
#define ADVANCE_VSYNC               0x3022
#define ADVANCE_HSYNC               0x3023
#define EARLY_GAIN                  0x3035
//  IO and clock control
#define BIT_CONTROL                 0x3059
#define OSC_CLK_DIV                 0x3060
#define ANA_Register_11             0x3061
#define IO_DRIVE_STR                0x3062
#define IO_DRIVE_STR2               0x3063
#define ANA_Register_14             0x3064
#define OUTPUT_PIN_STATUS_CONTROL   0x3065
#define ANA_Register_17             0x3067
#define PCLK_POLARITY               0x3068
/*
 * Useful value of Himax registers
 */
#define HIMAX_RESET                 0x01
#define PCLK_RISING_EDGE            0x00
#define PCLK_FALLING_EDGE           0x01
#define AE_CTRL_ENABLE              0x00
#define AE_CTRL_DISABLE             0x01

#define HIMAX_LINE_LEN_PCK_FULL     0x178
#define HIMAX_FRAME_LENGTH_FULL     0x109

/**
 * @}
 */
/* Private variables ---------------------------------------------------------*/

/** @defgroup GAPUINO_HIMAX_Private_Variables I2C Private Variables
 * @{
 */
#define HIMAX_LINE_LEN_PCK_FULL     0x178
#define HIMAX_FRAME_LENGTH_FULL     0x109

#define HIMAX_LINE_LEN_PCK_QVGA     0x178
#define HIMAX_FRAME_LENGTH_QVGA     0x104

#define HIMAX_LINE_LEN_PCK_QQVGA    0x178
#define HIMAX_FRAME_LENGTH_QQVGA    0x084

enum {
    HIMAX_Standby    = 0x0,
    HIMAX_Streaming  = 0x1,       // I2C triggered streaming enable
    HIMAX_Streaming2 = 0x3,       // Output N frames
    HIMAX_Streaming3 = 0x5        // Hardware Trigger
};

typedef struct regval_list_ {
    uint16_t reg_num;
    uint8_t  value;
} regval_list_t;

static uint16_t himax_default_regs[][2] = {
    {BLC_TGT,               0x08},          //  BLC target :8  at 8 bit mode
    {BLC2_TGT,              0x08},          //  BLI target :8  at 8 bit mode
    {0x3044,                0x0A},          //  Increase CDS time for settling
    {0x3045,                0x00},          //  Make symetric for cds_tg and rst_tg
    {0x3047,                0x0A},          //  Increase CDS time for settling
    {0x3050,                0xC0},          //  Make negative offset up to 4x
    {0x3051,                0x42},
    {0x3052,                0x50},
    {0x3053,                0x00},
    {0x3054,                0x03},          //  tuning sf sig clamping as lowest
    {0x3055,                0xF7},          //  tuning dsun
    {0x3056,                0xF8},          //  increase adc nonoverlap clk
    {0x3057,                0x29},          //  increase adc pwr for missing code
    {0x3058,                0x1F},          //  turn on dsun
    {0x3059,                0x1E},
    {0x3064,                0x00},
    {0x3065,                0x04},          //  pad pull 0
    {ANA_Register_17,       0x00},          //  Disable internal oscillator

    {BLC_CFG,               0x43},          //  BLC_on, IIR

    {0x1001,                0x43},          //  BLC dithering en
    {0x1002,                0x43},          //  blc_darkpixel_thd
    {0x0350,                0x7F},          //  Dgain Control
    {BLI_EN,                0x01},          //  BLI enable
    {0x1003,                0x00},          //  BLI Target [Def: 0x20]

    {DPC_CTRL,              0x01},          //  DPC option 0: DPC off   1 : mono   3 : bayer1   5 : bayer2
    {0x1009,                0xA0},          //  cluster hot pixel th
    {0x100A,                0x60},          //  cluster cold pixel th
    {SINGLE_THR_HOT,        0x90},          //  single hot pixel th
    {SINGLE_THR_COLD,       0x40},          //  single cold pixel th
    {0x1012,                0x00},          //  Sync. shift disable
    {STATISTIC_CTRL,        0x07},          //  AE stat en | MD LROI stat en | magic
    {0x2003,                0x00},
    {0x2004,                0x1C},
    {0x2007,                0x00},
    {0x2008,                0x58},
    {0x200B,                0x00},
    {0x200C,                0x7A},
    {0x200F,                0x00},
    {0x2010,                0xB8},
    {0x2013,                0x00},
    {0x2014,                0x58},
    {0x2017,                0x00},
    {0x2018,                0x9B},

    {AE_CTRL,               0x01},          //Automatic Exposure
    {AE_TARGET_MEAN,        0x64},          //AE target mean          [Def: 0x3C]
    {AE_MIN_MEAN,           0x0A},          //AE min target mean      [Def: 0x0A]
    {CONVERGE_IN_TH,        0x03},          //Converge in threshold   [Def: 0x03]
    {CONVERGE_OUT_TH,       0x05},          //Converge out threshold  [Def: 0x05]
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QVGA-2)>>8},          //Maximum INTG High Byte  [Def: 0x01]
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QVGA-2)&0xFF},        //Maximum INTG Low Byte   [Def: 0x54]
    {MAX_AGAIN_FULL,        0x04},          //Maximum Analog gain in full frame mode [Def: 0x03]
    {MAX_AGAIN_BIN2,        0x04},          //Maximum Analog gain in bin2 mode       [Def: 0x04]
    {MAX_DGAIN,             0xC0},

    {INTEGRATION_H,         0x01},          //Integration H           [Def: 0x01]
    {INTEGRATION_L,         0x08},          //Integration L           [Def: 0x08]
    {ANALOG_GAIN,           0x00},          //Analog Global Gain      [Def: 0x00]
    {DAMPING_FACTOR,        0x20},          //Damping Factor          [Def: 0x20]
    {DIGITAL_GAIN_H,        0x01},          //Digital Gain High       [Def: 0x01]
    {DIGITAL_GAIN_L,        0x00},          //Digital Gain Low        [Def: 0x00]

    {FS_CTRL,               0x00},          //Flicker Control

    {FS_60HZ_H,             0x00},
    {FS_60HZ_L,             0x3C},
    {FS_50HZ_H,             0x00},
    {FS_50HZ_L,             0x32},

    {MD_CTRL,               0x00},
    {FRAME_LEN_LINES_H,     HIMAX_FRAME_LENGTH_QVGA>>8},
    {FRAME_LEN_LINES_L,     HIMAX_FRAME_LENGTH_QVGA&0xFF},
    {LINE_LEN_PCK_H,        HIMAX_LINE_LEN_PCK_QVGA>>8},
    {LINE_LEN_PCK_L,        HIMAX_LINE_LEN_PCK_QVGA&0xFF},
    {QVGA_WIN_EN,           0x01},          // Enable QVGA window readout
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {0x3011,                0x70},
    {0x3059,                0x02},
    {OSC_CLK_DIV,           0x0B},
    {IMG_ORIENTATION,       0x00},          // change the orientation
    {0x0104,                0x01},
    {0x0000,                0x00},  // EOF
};

static const uint16_t himax_full_regs[][2] = {
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {QVGA_WIN_EN,           0x00},// Disable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_FULL-2)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_FULL-2)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_FULL>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_FULL&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_FULL>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_FULL&0xFF)},
    {GRP_PARAM_HOLD,        0x01},
    {0x0000,                0x00}, // EOF
};

static const uint16_t himax_qvga_regs[][2] = {
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {QVGA_WIN_EN,           0x01},// Enable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QVGA-2)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QVGA-2)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QVGA>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QVGA&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QVGA>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QVGA&0xFF)},
    {GRP_PARAM_HOLD,        0x01},
    {0x0000,                0x00},  // EOF

};

static const uint16_t himax_qqvga_regs[][2] = {
    {0x0383,                0x03},
    {0x0387,                0x03},
    {0x0390,                0x03},
    {QVGA_WIN_EN,           0x01},// Enable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QQVGA-2)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QQVGA-2)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QQVGA>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QQVGA&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QQVGA>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QQVGA&0xFF)},
    {GRP_PARAM_HOLD,        0x01},
    {0x0000,                0x00},  // EOF
};

HM01B0::HM01B0(arduino::MbedI2C &i2c) : 
    _i2c(&i2c),
    md_irq(PC_15),
    _md_callback(NULL)
{
}

void HM01B0::irqHandler()
{
  if (_md_callback) {
    _md_callback();
  }
}

int HM01B0::init()
{
    _i2c->begin();
    _i2c->setClock(100000);

    if (reset() != 0 ) {
        return -1;
    }

    for (uint32_t i=0; himax_default_regs[i][0]; i++) {
        regWrite(HM01B0_I2C_ADDR, himax_default_regs[i][0], himax_default_regs[i][1], true);
    }

    regWrite(HM01B0_I2C_ADDR, PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE), true);

    regWrite(HM01B0_I2C_ADDR, MODE_SELECT, HIMAX_Streaming, true);

    HAL_Delay(200);

    return 0;
}

int HM01B0::reset()
{
    uint32_t max_timeout=100;
    do {
        regWrite(HM01B0_I2C_ADDR, SW_RESET, HIMAX_RESET, true);
        delay(1);
    } while (regRead(HM01B0_I2C_ADDR, MODE_SELECT, true) != HIMAX_Standby && ((--max_timeout)>0) );

    return (max_timeout > 0) ? 0 : -1;
}

int HM01B0::setVerticalFlip(bool flip_enable)
{
  return -1;
}

int HM01B0::setHorizontalMirror(bool mirror_enable)
{
  return -1;
}

int HM01B0::setResolution(int32_t resolution)
{
    setResolutionWithZoom(resolution, resolution, 0, 0);
}

int HM01B0::setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y)
{
  int ret = 0;

  if (resolution != zoom_resolution)
  {
    return -1;
  }

  switch (resolution) {
    case CAMERA_R160x120:
      for(uint32_t i = 0; himax_qqvga_regs[i][0]; i++) {
        ret |= regWrite(HM01B0_I2C_ADDR, himax_qqvga_regs[i][0], himax_qqvga_regs[i][1], true);
      }
      break;
    case CAMERA_R320x240:
      for(uint32_t i = 0; himax_qvga_regs[i][0]; i++) {
        ret |= regWrite(HM01B0_I2C_ADDR, himax_qvga_regs[i][0], himax_qvga_regs[i][1], true);
      }
      break;
    case CAMERA_R320x320:
      for(uint32_t i = 0; himax_full_regs[i][0]; i++) {
        ret |= regWrite(HM01B0_I2C_ADDR, himax_full_regs[i][0], himax_full_regs[i][1], true);
      }
      break;  
    default:
      return -1;
  }

  return ret;
}

int HM01B0::setFrameRate(int32_t framerate)
{
  uint8_t osc_div = 0;
  // binning is enabled for QQVGA
  uint8_t binning = regRead(HM01B0_I2C_ADDR, BINNING_MODE, true) & 0x03;

  switch (framerate) {
    case 15:
      osc_div = (binning) ? 0x00 : 0x01;
      break;
    case 30:
      osc_div = (binning) ? 0x01 : 0x02;
      break;
    case 60:
      osc_div = (binning) ? 0x02 : 0x03;
      break;
    case 120:
      // Set to max FPS for QVGA and QQVGA.
      osc_div = 0x03;
      break;
    default:
      return -1;
  }

  return regWrite(HM01B0_I2C_ADDR, OSC_CLK_DIV, 0x08 | osc_div, true);
}

int HM01B0::setPixelFormat(int32_t pixformat)
{
    return (pixformat == CAMERA_GRAYSCALE) ? 0 : -1;
}

int HM01B0::setTestPattern(bool enable, bool walking)
{
    uint8_t reg = 0;
    regWrite(HM01B0_I2C_ADDR, PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE), true);
    regWrite(HM01B0_I2C_ADDR, 0x2100,  0, true); //AE
    regWrite(HM01B0_I2C_ADDR, 0x1000,  0, true); //BLC
    regWrite(HM01B0_I2C_ADDR, 0x1008,  0, true); //DPC
    regWrite(HM01B0_I2C_ADDR, 0x0205,  0, true); //AGAIN
    regWrite(HM01B0_I2C_ADDR, 0x020e,  1, true); //DGAINH
    regWrite(HM01B0_I2C_ADDR, 0x020f,  0, true); //DGAINL

    if (enable) {
        reg = 1 | (walking ? (1 << 4) : 0);
    }
    regWrite(HM01B0_I2C_ADDR, 0x0601,  reg, true);
    regWrite(HM01B0_I2C_ADDR, 0x0104,  1, true); //group hold

    HAL_Delay(100);

    return 0;
}

int HM01B0::setMotionDetectionThreshold(uint32_t threshold)
{
  // Set motion detection threshold/sensitivity.
  // The recommended threshold range is 0x03 to 0xF0.
  //
  // Motion is detected according to the following:
  //    |MD_LROI_MEAN – MD_LROI_IIR_MEAN| > ( MD_LROI_MEAN * MD_THL / 64)
  //
  // In other words, motion is detected if the abs difference of the ROI mean and the
  // average ROI mean of the last 8 or 16 frames is higher than (ROI mean * threshold / 64).
  return regWrite(HM01B0_I2C_ADDR, MD_THL, (threshold < 3) ? 3 : (threshold > 0xF0) ? 0xF0 : threshold, true);
}

int HM01B0::setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  int ret = 0;
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_X_START_H, (x>>8), true);
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_X_START_L, (x&0xff), true);
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_Y_START_H, (y>>8), true);
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_Y_START_L, (y&0xff), true);
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_X_END_H,   (w>>8), true);
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_X_END_L,   (w&0xff), true);
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_Y_END_H,   (h>>8), true);
  ret |= regWrite(HM01B0_I2C_ADDR, MD_LROI_Y_END_L,   (h&0xff), true);
  return ret;
}

int HM01B0::enableMotionDetection(md_callback_t callback)
{
  md_irq.rise(0);
  _md_callback = callback;
  md_irq.rise(mbed::Callback<void()>(this, &HM01B0::irqHandler));
  md_irq.enable_irq();

  int ret = clearMotionDetection();
  ret |= regWrite(HM01B0_I2C_ADDR, MD_CTRL, 1, true);
  return ret;
}

int HM01B0::disableMotionDetection()
{
  _md_callback = NULL;
  int ret = clearMotionDetection();
  ret |= regWrite(HM01B0_I2C_ADDR, MD_CTRL, 0, true);
  return ret;
}

int HM01B0::motionDetected()
{
  int ret = 0;

  ret = pollMotionDetection();
  if (ret == 1) {
    clearMotionDetection();
  }
  return ret;
}

int HM01B0::pollMotionDetection()
{
  return regRead(HM01B0_I2C_ADDR, MD_INTERRUPT, true);
}

int HM01B0::clearMotionDetection()
{
  return regWrite(HM01B0_I2C_ADDR, I2C_CLEAR, 0x01, true);
}

uint8_t HM01B0::printRegs()
{
    for (uint32_t i=0; himax_default_regs[i][0]; i++) {
        printf("0x%04X: 0x%02X  0x%02X \n",
                himax_default_regs[i][0],
                himax_default_regs[i][1],
                regRead(HM01B0_I2C_ADDR, himax_default_regs[i][0], true));
    }
    return 0;
}

int HM01B0::regWrite(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr)
{
    _i2c->beginTransmission(dev_addr);
    uint8_t buf[3] = {(uint8_t) (reg_addr >> 8), (uint8_t) (reg_addr & 0xFF), reg_data};
    if (wide_addr == true) {
        _i2c->write(buf, 1);
    }
    _i2c->write(&buf[1], 2);
    return _i2c->endTransmission();
}

uint8_t HM01B0::regRead(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr)
{
    uint8_t reg_data = 0;
    uint8_t buf[2] = {(uint8_t) (reg_addr >> 8), (uint8_t) (reg_addr & 0xFF)};
    _i2c->beginTransmission(dev_addr);
    if (wide_addr) {
        _i2c->write(buf, 2);
    } else {
        _i2c->write(&buf[1], 1);
    }
    _i2c->endTransmission(false);
    _i2c->requestFrom(dev_addr, 1);
    if (_i2c->available()) {
        reg_data = _i2c->read();
    }
    while (_i2c->available()) {
        _i2c->read();
    }
    return reg_data;
}

void HM01B0::debug(Stream &stream)
{
  _debug = &stream;
}

