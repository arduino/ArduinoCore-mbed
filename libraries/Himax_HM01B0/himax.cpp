/*
 * TODO: Add license.
 * Copyright (c) 2021
 *
 * This work is licensed under <>, see the file LICENSE for details.
 *
 * HM01B0 driver.
 */
#include "Wire.h"
#include "himax.h"

#define ENABLE            0x01
#define DISABLE           0x00

// Register set
// Read only registers
#define         MODEL_ID_H          0x0000
#define         MODEL_ID_L          0x0001
#define         FRAME_COUNT         0x0005
#define         PIXEL_ORDER         0x0006
// R&W registers
// Sensor mode control
#define         MODE_SELECT         0x0100
#define         IMG_ORIENTATION     0x0101
#define         SW_RESET            0x0103
#define         GRP_PARAM_HOLD      0x0104
// Sensor exposure gain control
#define         INTEGRATION_H       0x0202
#define         INTEGRATION_L       0x0203
#define         ANALOG_GAIN         0x0205
#define         DIGITAL_GAIN_H      0x020E
#define         DIGITAL_GAIN_L      0x020F
// Frame timing control
#define         FRAME_LEN_LINES_H   0x0340
#define         FRAME_LEN_LINES_L   0x0341
#define         LINE_LEN_PCK_H      0x0342
#define         LINE_LEN_PCK_L      0x0343
// Binning mode control
#define         READOUT_X           0x0383
#define         READOUT_Y           0x0387
#define         BINNING_MODE        0x0390
// Test pattern control
#define         TEST_PATTERN_MODE   0x0601
// Black level control
#define         BLC_CFG             0x1000
#define         BLC_TGT             0x1003
#define         BLI_EN              0x1006
#define         BLC2_TGT            0x1007
//  Sensor reserved
#define         DPC_CTRL            0x1008
#define         SINGLE_THR_HOT      0x100B
#define         SINGLE_THR_COLD     0x100C
// VSYNC,HSYNC and pixel shift register
#define         VSYNC_HSYNC_PIXEL_SHIFT_EN  0x1012
// Automatic exposure gain control
#define         AE_CTRL             0x2100
#define         AE_TARGET_MEAN      0x2101
#define         AE_MIN_MEAN         0x2102
#define         CONVERGE_IN_TH      0x2103
#define         CONVERGE_OUT_TH     0x2104
#define         MAX_INTG_H          0x2105
#define         MAX_INTG_L          0x2106
#define         MIN_INTG            0x2107
#define         MAX_AGAIN_FULL      0x2108
#define         MAX_AGAIN_BIN2      0x2109
#define         MIN_AGAIN           0x210A
#define         MAX_DGAIN           0x210B
#define         MIN_DGAIN           0x210C
#define         DAMPING_FACTOR      0x210D
#define         FS_CTRL             0x210E
#define         FS_60HZ_H           0x210F
#define         FS_60HZ_L           0x2110
#define         FS_50HZ_H           0x2111
#define         FS_50HZ_L           0x2112
#define         FS_HYST_TH          0x2113
// Motion detection control
#define         MD_CTRL             0x2150
#define         I2C_CLEAR           0x2153
#define         WMEAN_DIFF_TH_H     0x2155
#define         WMEAN_DIFF_TH_M     0x2156
#define         WMEAN_DIFF_TH_L     0x2157
#define         MD_THH              0x2158
#define         MD_THM1             0x2159
#define         MD_THM2             0x215A
#define         MD_THL              0x215B
#define         STATISTIC_CTRL      0x2000
#define         MD_LROI_X_START_H   0x2011
#define         MD_LROI_X_START_L   0x2012
#define         MD_LROI_Y_START_H   0x2013
#define         MD_LROI_Y_START_L   0x2014
#define         MD_LROI_X_END_H     0x2015
#define         MD_LROI_X_END_L     0x2016
#define         MD_LROI_Y_END_H     0x2017
#define         MD_LROI_Y_END_L     0x2018
#define         MD_INTERRUPT        0x2160
//  Sensor timing control
#define         QVGA_WIN_EN         0x3010
#define         SIX_BIT_MODE_EN     0x3011
#define         PMU_AUTOSLEEP_FRAMECNT  0x3020
#define         ADVANCE_VSYNC       0x3022
#define         ADVANCE_HSYNC       0x3023
#define         EARLY_GAIN          0x3035
//  IO and clock control
#define         BIT_CONTROL         0x3059
#define         OSC_CLK_DIV         0x3060
#define         ANA_Register_11     0x3061
#define         IO_DRIVE_STR        0x3062
#define         IO_DRIVE_STR2       0x3063
#define         ANA_Register_14     0x3064
#define         OUTPUT_PIN_STATUS_CONTROL   0x3065
#define         ANA_Register_17     0x3067
#define         PCLK_POLARITY       0x3068

/*
 * Useful value of Himax registers
 */
#define         HIMAX_RESET         0x01
#define         PCLK_RISING_EDGE    0x00
#define         PCLK_FALLING_EDGE   0x01
#define         AE_CTRL_ENABLE      0x00
#define         AE_CTRL_DISABLE     0x01

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

typedef struct regval_list_ {
    uint16_t reg_num;
    uint8_t  value;
} regval_list_t;

enum {
    HIMAX_Standby    = 0x0,
    HIMAX_Streaming  = 0x1,       // I2C triggered streaming enable
    HIMAX_Streaming2 = 0x3,       // Output N frames
    HIMAX_Streaming3 = 0x5        // Hardware Trigger
};

static regval_list_t himax_default_regs[] = {
    {BLC_TGT,              0x08},          //  BLC target :8  at 8 bit mode
    {BLC2_TGT,             0x08},          //  BLI target :8  at 8 bit mode
    {0x3044,               0x0A},          //  Increase CDS time for settling
    {0x3045,               0x00},          //  Make symetric for cds_tg and rst_tg
    {0x3047,               0x0A},          //  Increase CDS time for settling
    {0x3050,               0xC0},          //  Make negative offset up to 4x
    {0x3051,               0x42},
    {0x3052,               0x50},
    {0x3053,               0x00},
    {0x3054,               0x03},          //  tuning sf sig clamping as lowest
    {0x3055,               0xF7},          //  tuning dsun
    {0x3056,               0xF8},          //  increase adc nonoverlap clk
    {0x3057,               0x29},          //  increase adc pwr for missing code
    {0x3058,               0x1F},          //  turn on dsun
    {0x3059,               0x1E},
    {0x3064,               0x00},
    {0x3065,               0x04},          //  pad pull 0

    {BLC_CFG,              0x43},          //  BLC_on, IIR

    {0x1001,               0x43},          //  BLC dithering en
    {0x1002,               0x43},          //  blc_darkpixel_thd
    {0x0350,               0x7F},          //  Dgain Control
    {BLI_EN,               0x01},          //  BLI enable
    {0x1003,               0x00},          //  BLI Target [Def: 0x20]

    {DPC_CTRL,             0x01},          //  DPC option 0: DPC off   1 : mono   3 : bayer1   5 : bayer2
    {0x1009,               0xA0},          //  cluster hot pixel th
    {0x100A,               0x60},          //  cluster cold pixel th
    {SINGLE_THR_HOT,       0x90},          //  single hot pixel th
    {SINGLE_THR_COLD,      0x40},          //  single cold pixel th
    {0x1012,               0x00},          //  Sync. shift disable
    {STATISTIC_CTRL,       0x07},          //  AE stat en | MD LROI stat en | magic
    {0x2003,               0x00},
    {0x2004,               0x1C},
    {0x2007,               0x00},
    {0x2008,               0x58},
    {0x200B,               0x00},
    {0x200C,               0x7A},
    {0x200F,               0x00},
    {0x2010,               0xB8},
    {0x2013,               0x00},
    {0x2014,               0x58},
    {0x2017,               0x00},
    {0x2018,               0x9B},

    {AE_CTRL,              0x01},          //Automatic Exposure
    {AE_TARGET_MEAN,       0x3C},          //AE target mean          [Def: 0x3C]
    {AE_MIN_MEAN,          0x0A},          //AE min target mean      [Def: 0x0A]
    {CONVERGE_IN_TH,       0x03},          //Converge in threshold   [Def: 0x03]
    {CONVERGE_OUT_TH,      0x05},          //Converge out threshold  [Def: 0x05]
    {MAX_INTG_H,           (HIMAX_FRAME_LENGTH_QVGA-2)>>8},          //Maximum INTG High Byte  [Def: 0x01]
    {MAX_INTG_L,           (HIMAX_FRAME_LENGTH_QVGA-2)&0xFF},        //Maximum INTG Low Byte   [Def: 0x54]
    {MAX_AGAIN_FULL,       0x03},          //Maximum Analog gain in full frame mode [Def: 0x03]
    {MAX_AGAIN_BIN2,       0x04},          //Maximum Analog gain in bin2 mode       [Def: 0x04]
    {MAX_DGAIN,            0xC0},

    {INTEGRATION_H,        0x01},          //Integration H           [Def: 0x01]
    {INTEGRATION_L,        0x08},          //Integration L           [Def: 0x08]
    {ANALOG_GAIN,          0x00},          //Analog Global Gain      [Def: 0x00]
    {DAMPING_FACTOR,       0x20},          //Damping Factor          [Def: 0x20]
    {DIGITAL_GAIN_H,       0x01},          //Digital Gain High       [Def: 0x01]
    {DIGITAL_GAIN_L,       0x00},          //Digital Gain Low        [Def: 0x00]

    {FS_CTRL,              0x00},          //Flicker Control

    {FS_60HZ_H,            0x00},
    {FS_60HZ_L,            0x3C},
    {FS_50HZ_H,            0x00},
    {FS_50HZ_L,            0x32},

    {MD_CTRL,              0x00},
    {FRAME_LEN_LINES_H,    HIMAX_FRAME_LENGTH_QVGA>>8},
    {FRAME_LEN_LINES_L,    HIMAX_FRAME_LENGTH_QVGA&0xFF},
    {LINE_LEN_PCK_H,       HIMAX_LINE_LEN_PCK_QVGA>>8},
    {LINE_LEN_PCK_L,       HIMAX_LINE_LEN_PCK_QVGA&0xFF},
    {QVGA_WIN_EN,          0x01},          // Enable QVGA window readout
    {0x0383,               0x01},
    {0x0387,               0x01},
    {0x0390,               0x00},
    {0x3011,               0x70},
    {0x3059,               0x02},
    {OSC_CLK_DIV,          0x0B},
    {IMG_ORIENTATION,      0x00},          // change the orientation
    {0x0104,               0x01},
};

static regval_list_t himax_full_regs[] = { // 'full' resolution is 320x320
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {QVGA_WIN_EN,           0x00},	      // Disable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_FULL-2)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_FULL-2)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_FULL>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_FULL&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_FRAME_LENGTH_FULL>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_FRAME_LENGTH_FULL&0xFF)},
    {GRP_PARAM_HOLD,        0x01},
};

static regval_list_t himax_full_regs[] = { // 'full' resolution is 320x320
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {QVGA_WIN_EN,           0x00},	      // Disable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_FULL-2)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_FULL-2)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_FULL>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_FULL&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_FRAME_LENGTH_FULL>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_FRAME_LENGTH_FULL&0xFF)},
    {GRP_PARAM_HOLD,        0x01},
};

static regval_list_t himax_qvga_regs[] = {
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QVGA-2)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QVGA-2)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QVGA>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QVGA&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QVGA>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QVGA&0xFF)},
};

static regval_list_t himax_qqvga_regs[] = {
    {0x0383,                0x03},
    {0x0387,                0x03},
    {0x0390,                0x03},
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QQVGA-2)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QQVGA-2)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QQVGA>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QQVGA&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QQVGA>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QQVGA&0xFF)},
};

int HM01B0::Init()
{
    if (Reset() != 0 ) {
        return -1;
    }

    for (uint32_t i=0; i<(sizeof(himax_default_regs) / sizeof(regval_list_t)); i++) {
        reg_write(HM01B0_I2C_ADDR, himax_default_regs[i].reg_num, himax_default_regs[i].value, true);
    }

    reg_write(HM01B0_I2C_ADDR, PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE), true);

    reg_write(HM01B0_I2C_ADDR, MODE_SELECT, HIMAX_Streaming, true);

    HAL_Delay(200);

    return 0;
}

int HM01B0::Reset()
{
    uint32_t max_timeout=100;
    do {
        reg_write(HM01B0_I2C_ADDR, SW_RESET, HIMAX_RESET, true);
        delay(1);
    } while (reg_read(HM01B0_I2C_ADDR, MODE_SELECT, true) != HIMAX_Standby && ((--max_timeout)>0) );

    return max_timeout>0 ? 0: -1 ;
}

int HM01B0::SetResolution(uint32_t resolution)
{
  int ret = 0;
  uint32_t regs_count = 0;
  regval_list_t *regs = NULL;

  switch (resolution) {
    case CAMERA_R160x120:
      regs = himax_qqvga_regs;
      regs_count = sizeof(himax_qqvga_regs) / sizeof(regval_list_t);
      break;
    case CAMERA_R320x240:
      regs = himax_qvga_regs;
      regs_count = sizeof(himax_qvga_regs) / sizeof(regval_list_t);
      break;
    case CAMERA_R320x320:
      regs = himax_full_regs;
      regs_count = sizeof(himax_full_regs) / sizeof(regval_list_t);
      break;  
    default:
      return -1;
  }

  for(uint32_t i = 0; i < regs_count; i++) {
    ret |= reg_write(HM01B0_I2C_ADDR, regs[i].reg_num, regs[i].value, true);
  }

  return ret;
}

int HM01B0::SetFrameRate(uint32_t framerate)
{
  uint8_t osc_div = 0;
  // binning is enabled for QQVGA
  uint8_t binning = reg_read(HM01B0_I2C_ADDR, BINNING_MODE, true) & 0x03;

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

  return reg_write(HM01B0_I2C_ADDR, OSC_CLK_DIV, 0x08 | osc_div, true);
}

int HM01B0::SetPixelFormat(uint32_t pixelformat)
{
    return -1;
}

int HM01B0::SetTestPattern(bool enable, bool walking)
{
    uint8_t reg = 0;
    reg_write(HM01B0_I2C_ADDR, PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE), true);
    reg_write(HM01B0_I2C_ADDR, 0x2100,  0, true); //AE
    reg_write(HM01B0_I2C_ADDR, 0x1000,  0, true); //BLC
    reg_write(HM01B0_I2C_ADDR, 0x1008,  0, true); //DPC
    reg_write(HM01B0_I2C_ADDR, 0x0205,  0, true); //AGAIN
    reg_write(HM01B0_I2C_ADDR, 0x020e,  1, true); //DGAINH
    reg_write(HM01B0_I2C_ADDR, 0x020f,  0, true); //DGAINL

    if (enable) {
        reg = 1 | (walking ? (1 << 4) : 0);
    }
    reg_write(HM01B0_I2C_ADDR, 0x0601,  reg, true);
    reg_write(HM01B0_I2C_ADDR, 0x0104,  1, true); //group hold

    HAL_Delay(100);

    return 0;
}

// These functions need to be reimplemented as IOCTLs.
#if 0
int HIMAX_Mode(uint8_t mode)
{
  return reg_write(MODE_SELECT, mode);
}

static void HIMAX_GrayScale(uint8_t value)
{
    reg_write(BLC_CFG,  ENABLE);
    reg_write(BLC_TGT,  value);
    reg_write(BLI_EN,   ENABLE);
    reg_write(BLC2_TGT, value);
}

int HIMAX_EnableMD(bool enable)
{
  int ret = HIMAX_ClearMD();
  ret |= reg_write(MD_CTRL, enable ? 1:0);
  return ret;
}

int HIMAX_SetMDThreshold(uint32_t threshold)
{
  // Set motion detection threshold/sensitivity.
  // The recommended threshold range is 0x03 to 0xF0.
  //
  // Motion is detected according to the following:
  //    |MD_LROI_MEAN – MD_LROI_IIR_MEAN| > ( MD_LROI_MEAN * MD_THL / 64)
  //
  // In other words, motion is detected if the abs difference of the ROI mean and the
  // average ROI mean of the last 8 or 16 frames is higher than (ROI mean * threshold / 64).
  return reg_write(MD_THL, (threshold < 3) ? 3 : (threshold > 0xF0) ? 0xF0 : threshold);
}

int HIMAX_SetLROI(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
  int ret = 0;
  ret |= reg_write(MD_LROI_X_START_H, (x1>>8));
  ret |= reg_write(MD_LROI_X_START_L, (x1&0xff));
  ret |= reg_write(MD_LROI_Y_START_H, (y1>>8));
  ret |= reg_write(MD_LROI_Y_START_L, (y1&0xff));
  ret |= reg_write(MD_LROI_X_END_H,   (x2>>8));
  ret |= reg_write(MD_LROI_X_END_L,   (x2&0xff));
  ret |= reg_write(MD_LROI_Y_END_H,   (y2>>8));
  ret |= reg_write(MD_LROI_Y_END_L,   (y2&0xff));
  return ret;
}

int HIMAX_PollMD()
{
  return reg_read(MD_INTERRUPT);
}

int HIMAX_ClearMD()
{
  return reg_write(I2C_CLEAR, 0x01);
}
static uint8_t HIMAX_PrintReg()
{
    unsigned int i;
    for (i=0; i<(sizeof(himax_default_regs)/sizeof(regval_list_t)); i++) {
        printf("0x%04X: 0x%02X  0x%02X \n",
                himax_default_regs[i].reg_num,
                himax_default_regs[i].value,
                reg_read(himax_default_regs[i].reg_num));
    }
    return 0;
}
#endif
