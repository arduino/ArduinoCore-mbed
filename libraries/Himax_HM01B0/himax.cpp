/**
******************************************************************************
* @file    gapuino_himax.c
* @author  GreenWave Technologies Application Team
* @brief   This file includes a standard driver for the HIMAX I2C
*          camera mounted on GAPUINO board.
@verbatim
==============================================================================
##### How to use this driver #####
==============================================================================
[..]
(#) This driver is used to drive the HIMAX I2C external
memory mounted on GAPUINO board.

(#) This driver need a specific component driver (HIMAX) to be included with.

(#) Initialization steps:

**/

/*
* Copyright (c) 2018, GreenWaves Technologies, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of GreenWaves Technologies, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Includes ------------------------------------------------------------------*/
#include "himax.h"
#include "camera.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup GAPUINO
 * @{
 */

/** @defgroup GAPUINO_QSPI QSPI
 * @{
 */

/* Private constants --------------------------------------------------------*/
/** @defgroup GAPUINO_QSPI_Private_Constants QSPI Private Constants
 * @{
 */


/**
 * @}
 */
/* Private variables ---------------------------------------------------------*/

/** @defgroup GAPUINO_HIMAX_Private_Variables I2C Private Variables
 * @{
 */
#define HIMAX_LINE_LEN_PCK_QVGA     0x178
#define HIMAX_FRAME_LENGTH_QVGA     0x104

#define HIMAX_LINE_LEN_PCK_QQVGA    0x178
#define HIMAX_FRAME_LENGTH_QQVGA    0x084

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
  {0x2000,               0x07},
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

  {MD_CTRL,              0x30},
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

/* SPI transfer command sequence array */
regval_list_t reg;

/**
 * @}
 */


/* Private functions ---------------------------------------------------------*/

/** @defgroup GAPUINO_QSPI_Private_Functions QSPI Private Functions
  * @{
  */
static int            HIMAX_RegWrite          (uint16_t addr, uint8_t value);
static uint8_t        HIMAX_RegRead           (uint16_t addr);
static uint8_t        HIMAX_Reset             (void);
static uint8_t        HIMAX_Boot              (void);
static uint8_t        HIMAX_PrintReg          (void);
static void           HIMAX_FrameRate         (void);


/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @addtogroup GAPUINO_HIMAX_Exported_Functions
  * @{
  */

/**
 * @brief  Initializes the I2C interface.
 * @retval HIMAX status
 */
uint8_t HIMAX_Open(void)
{
    Wire.begin();

    //printf("Model: %x:%x\n", HIMAX_RegRead(MODEL_ID_H), HIMAX_RegRead(MODEL_ID_L));

    if (HIMAX_Reset()!=0) {
        return -1;
    }

    HIMAX_Boot();

    //For debugging camera Configuration
    //HIMAX_PrintReg();
    HAL_Delay(200);

    return 0;
}

/**
 * @brief  This function selects HIMAX camera mode.
 * @retval None
 */
int HIMAX_Mode(uint8_t mode)
{
  return HIMAX_RegWrite(MODE_SELECT, mode);
}

int HIMAX_SetResolution(uint32_t resolution)
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
    default:
      return -1;
  }

  for(uint32_t i = 0; i < regs_count; i++) {
    ret |= HIMAX_RegWrite(regs[i].reg_num, regs[i].value);
  }

  return ret;
}

int HIMAX_SetFramerate(uint32_t framerate)
{
  uint8_t osc_div = 0;
  // binning is enabled for QQVGA
  uint8_t binning = HIMAX_RegRead(BINNING_MODE) & 0x03;

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

  return HIMAX_RegWrite(OSC_CLK_DIV, 0x08 | osc_div);
}

int HIMAX_EnableMD(bool enable)
{
  int ret = HIMAX_ClearMD();
  if (enable) {
    ret |= HIMAX_RegWrite(MD_CTRL, 0x03);
  } else {
    ret |= HIMAX_RegWrite(MD_CTRL, 0x30);
  }
  return ret;
}

int HIMAX_SetMDThreshold(uint32_t low, uint32_t high)
{
  int ret = 0;
  ret |= HIMAX_RegWrite(MD_THL, low  & 0xff);
  ret |= HIMAX_RegWrite(MD_THH, high & 0xff);
  return ret;
}

int HIMAX_SetLROI(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
  int ret = 0;
  ret |= HIMAX_RegWrite(MD_LROI_X_START_H, (x1>>8));
  ret |= HIMAX_RegWrite(MD_LROI_X_START_L, (x1&0xff));
  ret |= HIMAX_RegWrite(MD_LROI_Y_START_H, (y1>>8));
  ret |= HIMAX_RegWrite(MD_LROI_Y_START_L, (y1&0xff));
  ret |= HIMAX_RegWrite(MD_LROI_X_END_H,   (x2>>8));
  ret |= HIMAX_RegWrite(MD_LROI_X_END_L,   (x2&0xff));
  ret |= HIMAX_RegWrite(MD_LROI_Y_END_H,   (y2>>8));
  ret |= HIMAX_RegWrite(MD_LROI_Y_END_L,   (y2&0xff));
  return ret;
}

int HIMAX_PollMD()
{
  return HIMAX_RegRead(MD_INTERRUPT);
}

int HIMAX_ClearMD()
{
  return HIMAX_RegWrite(I2C_CLEAR, 0x01);
}

/**
 * @brief  This function writes HIMAX camera registers.
 * @retval None
 */
static int HIMAX_RegWrite(uint16_t addr, uint8_t value)
{
    uint8_t addr_high = (addr >> 8) & 0xFF;
    uint8_t addr_low  = addr & 0xFF;

    reg.reg_num = (addr_low << 8) | addr_high;
    reg.value = value;

    Wire.beginTransmission(HIMAX_I2C_ADDR);
    Wire.write((const char *)&reg, 3);
    int ret = Wire.endTransmission();

    return ret;
}

/**
 * @brief  This function reads HIMAX camera registers.
 * @retval None
 */
static uint8_t HIMAX_RegRead(uint16_t addr)
{
    uint8_t addr_high = (addr >> 8) & 0xFF;
    uint8_t addr_low  = addr & 0xFF;

    reg.reg_num = (addr_low << 8) | addr_high;

    Wire.beginTransmission(HIMAX_I2C_ADDR);
    Wire.write((const char *)&reg.reg_num, 2);
    Wire.endTransmission(false);
    int ret = Wire.requestFrom(HIMAX_I2C_ADDR, 1);
    if (Wire.available()) {
        reg.value = Wire.read();
    }
    while (Wire.available()) {
        Wire.read();
    }
    return reg.value;
}

static uint8_t HIMAX_Reset()
{
    uint32_t max_timeout=100;
    do {
        HIMAX_RegWrite(SW_RESET, HIMAX_RESET);
        delay(1);
    } while (HIMAX_RegRead(MODE_SELECT) != HIMAX_Standby && ((--max_timeout)>0) );

    return max_timeout>0 ? 0: -1 ;
}

static uint8_t HIMAX_Boot()
{
    uint32_t i;

    for(i = 0; i < (sizeof(himax_default_regs) / sizeof(regval_list_t)); i++) {
        HIMAX_RegWrite(himax_default_regs[i].reg_num, himax_default_regs[i].value);
    }

    HIMAX_RegWrite(PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE));

    HIMAX_RegWrite(MODE_SELECT, HIMAX_Standby);
    return 0;
}

static void HIMAX_GrayScale(uint8_t value)
{
    HIMAX_RegWrite(BLC_CFG,  ENABLE);
    HIMAX_RegWrite(BLC_TGT,  value);
    HIMAX_RegWrite(BLI_EN,   ENABLE);
    HIMAX_RegWrite(BLC2_TGT, value);
}

void HIMAX_TestPattern(bool enable, bool walking)
{
    uint8_t reg = 0;
    HIMAX_RegWrite(PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE));
    HIMAX_RegWrite(0x2100,  0 ); //AE 
    HIMAX_RegWrite(0x1000,  0 ); //BLC 
    HIMAX_RegWrite(0x1008,  0 ); //DPC 
    HIMAX_RegWrite(0x0205,  0 ); //AGAIN 
    HIMAX_RegWrite(0x020e,  1 ); //DGAINH 
    HIMAX_RegWrite(0x020f,  0 ); //DGAINL 
    
    if (enable) {
        reg = 1 | (walking ? (1 << 4) : 0);
    }
    HIMAX_RegWrite(0x0601,  reg );
    HIMAX_RegWrite(0x0104,  1 ); //group hold 

    HAL_Delay(100);

}

static void HIMAX_FrameRate()
{
    HIMAX_RegWrite( FRAME_LEN_LINES_H, 0x02);
    HIMAX_RegWrite( FRAME_LEN_LINES_L, 0x1C);
    HIMAX_RegWrite( LINE_LEN_PCK_H,    0x01);
    HIMAX_RegWrite( LINE_LEN_PCK_L,    0x72);
}

static uint8_t HIMAX_PrintReg()
{
    unsigned int i;
    for(i=0; i<(sizeof(himax_default_regs)/sizeof(regval_list_t)); i++){
        printf("0x%04X: 0x%02X  0x%02X \n",himax_default_regs[i].reg_num, himax_default_regs[i].value, HIMAX_RegRead(himax_default_regs[i].reg_num));
    }
    return 0;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
