/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HIMAX_H
#define __HIMAX_H

#include "Arduino.h"
#include "Wire.h"
#include "mbed.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define HIMAX_I2C_ADDR                     0x24

#define QVGA_W                             (320)
#define QVGA_H                             (240)
#define IMG_QVGASIZE                       (QVGA_H*QVGA_W)    //320*240
#define PPM_HEADER                          40

#define MASK_1BIT                           0x1
#define MASK_2BITS                          0x3
#define MASK_3BITS                          0x7
#define MASK_4BITS                          0xF
#define MASK_5BITS                          0x1F
#define MASK_6BITS                          0x3F
#define MASK_7BITS                          0x7F
#define MASK_8BITS                          0xFF
#define MASK_16BITS                         0xFFFF

#define ENABLE            0x01
#define DISABLE           0x00

/*
 *  HIMAX camera macros
 */
// Register address
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

enum{
    HIMAX_Standby    = 0x0,
    HIMAX_Streaming  = 0x1,       // I2C triggered streaming enable
    HIMAX_Streaming2 = 0x3,       // Output N frames
    HIMAX_Streaming3 = 0x5        // Hardware Trigger
};

typedef struct regval_list_ {
    uint16_t reg_num;
    uint8_t  value;
} regval_list_t;

uint8_t HIMAX_Open(void);
int HIMAX_Mode(uint8_t mode);
int HIMAX_SetResolution(uint32_t resolution);
int HIMAX_SetFramerate(uint32_t framerate);
int HIMAX_EnableMD(bool enable);
int HIMAX_SetMDThreshold(uint32_t low, uint32_t high);
int HIMAX_SetLROI(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
int HIMAX_PollMD();
int HIMAX_ClearMD();
void HIMAX_TestPattern(bool enable = true, bool walking = true);


#ifdef __cplusplus
}
#endif

#endif /* __HIMAX_H */
