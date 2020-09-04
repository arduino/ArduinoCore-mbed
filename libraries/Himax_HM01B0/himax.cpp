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
static regval_list_t himax_default_regs[] = {
    {BLC_TGT, 0x08},            //  BLC target :8  at 8 bit mode
    {BLC2_TGT, 0x08},           //  BLI target :8  at 8 bit mode
    {0x3044, 0x0A},             //  Increase CDS time for settling
    {0x3045, 0x00},             //  Make symetric for cds_tg and rst_tg
    {0x3047, 0x0A},             //  Increase CDS time for settling
    {0x3050, 0xC0},             //  Make negative offset up to 4x
    {0x3051, 0x42},
    {0x3052, 0x50},
    {0x3053, 0x00},
    {0x3054, 0x03},             //  tuning sf sig clamping as lowest
    {0x3055, 0xF7},             //  tuning dsun
    {0x3056, 0xF8},             //  increase adc nonoverlap clk
    {0x3057, 0x29},             //  increase adc pwr for missing code
    {0x3058, 0x1F},             //  turn on dsun
    {0x3059, 0x1E},
    {0x3064, 0x00},
    {0x3065, 0x04},             //  pad pull 0

    {BLC_CFG, 0x43},            //  BLC_on, IIR

    {0x1001, 0x43},             //  BLC dithering en
    {0x1002, 0x43},             //  blc_darkpixel_thd
    {0x0350, 0x00},             //  Dgain Control
    {BLI_EN, 0x01},             //  BLI enable
    {0x1003, 0x00},             //  BLI Target [Def: 0x20]

    {DPC_CTRL, 0x01},           //  DPC option 0: DPC off   1 : mono   3 : bayer1   5 : bayer2
    {0x1009, 0xA0},             //  cluster hot pixel th
    {0x100A, 0x60},             //  cluster cold pixel th
    {SINGLE_THR_HOT, 0x90},     //  single hot pixel th
    {SINGLE_THR_COLD, 0x40},    //  single cold pixel th
    {0x1012, 0x00},             //  Sync. shift disable
    {0x2000, 0x07},
    {0x2003, 0x00},
    {0x2004, 0x1C},
    {0x2007, 0x00},
    {0x2008, 0x58},
    {0x200B, 0x00},
    {0x200C, 0x7A},
    {0x200F, 0x00},
    {0x2010, 0xB8},
    {0x2013, 0x00},
    {0x2014, 0x58},
    {0x2017, 0x00},
    {0x2018, 0x9B},

    {AE_CTRL,        0x01},      //Automatic Exposure
    {AE_TARGET_MEAN, 0x3C},      //AE target mean          [Def: 0x3C]
    {AE_MIN_MEAN,    0x0A},      //AE min target mean      [Def: 0x0A]

    {INTEGRATION_H,  0x01},      //Integration H           [Def: 0x01]
    {INTEGRATION_L,  0x08},      //Integration L           [Def: 0x08]
    {ANALOG_GAIN,    0x00},      //Analog Global Gain      [Def: 0x00]
    {DAMPING_FACTOR, 0x20},      //Damping Factor          [Def: 0x20]
    {DIGITAL_GAIN_H, 0x01},      //Digital Gain High       [Def: 0x01]
    {DIGITAL_GAIN_L, 0x00},      //Digital Gain Low        [Def: 0x00]

    {CONVERGE_IN_TH,  0x03},     //Converge in threshold   [Def: 0x03]
    {CONVERGE_OUT_TH, 0x05},     //Converge out threshold  [Def: 0x05]
    {MAX_INTG_H,      0x01},     //Maximum INTG High Byte  [Def: 0x01]
    {MAX_INTG_L,      0x54},     //Maximum INTG Low Byte   [Def: 0x54]
    {MAX_AGAIN_FULL,  0x03},     //Maximum Analog gain in full frame mode [Def: 0x03]
    {MAX_AGAIN_BIN2,  0x04},     //Maximum Analog gain in bin2 mode       [Def: 0x04]

    {0x210B, 0xC0},
    {0x210E, 0x00}, //Flicker Control
    {0x210F, 0x00},
    {0x2110, 0x3C},
    {0x2111, 0x00},
    {0x2112, 0x32},

    {0x2150, 0x30},
    {0x0340, 0x02},
    {0x0341, 0x16},
    {0x0342, 0x01},
    {0x0343, 0x78},
    {0x3010, 0x01}, // 324 x 244 pixel
    {0x0383, 0x01},
    {0x0387, 0x01},
    {0x0390, 0x03},
    {0x3011, 0x70},
    {0x3059, 0x02},
    {0x3060, 0x00},
    //{0x0601, 0x01},
    {IMG_ORIENTATION, 0x00},
    {0x0104, 0x01}
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

    if (HIMAX_Reset()!=0) return -1;
    //HIMAX_Boot();
    //For debugging camera Configuration
    //HIMAX_PrintReg();
    HAL_Delay(200);

    return 0;
}

/**
 * @brief  This function selects HIMAX camera mode.
 * @retval None
 */
void HIMAX_Mode(uint8_t mode)
{
    HIMAX_RegWrite(MODE_SELECT, mode);
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
        //printf("%d\n", i);
        HIMAX_RegWrite(himax_default_regs[i].reg_num, himax_default_regs[i].value);
        //delay(1);
    }

    HIMAX_RegWrite(PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE));

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