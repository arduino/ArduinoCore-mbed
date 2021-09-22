#ifndef RGBled_h
#define RGBled_h

// Regsiter Map
// http://www.issi.com/WW/pdf/IS31FL3194.pdf
#define IS31FL3194_PRODUCT_ID           0x00  // should return 0xCE
#define IS31FL3194_OP_CONFIG            0x01
#define IS31FL3194_OUT_CONFIG           0x02
#define IS31FL3194_CURRENT_BAND         0x03
#define IS31FL3194_HOLD_FUNCTION        0x04

#define IS31FL3194_P1_STATE             0x0D
#define IS31FL3194_P2_STATE             0x0E
#define IS31FL3194_P3_STATE             0x0F

// Current Mode
#define IS31FL3194_OUT1                 0x10
#define IS31FL3194_OUT2                 0x21
#define IS31FL3194_OUT3                 0x32

//Pattern mode
// Colors 1, 2 and 3 of pattern 1
#define IS31FL3194_COL1_PATT1_R         0x10
#define IS31FL3194_COL1_PATT1_G         0x11
#define IS31FL3194_COL1_PATT1_B         0x12
#define IS31FL3194_COL2_PATT1_R         0x13
#define IS31FL3194_COL2_PATT1_G         0x14
#define IS31FL3194_COL2_PATT1_B         0x15
#define IS31FL3194_COL3_PATT1_R         0x16
#define IS31FL3194_COL3_PATT1_G         0x17
#define IS31FL3194_COL3_PATT1_B         0x18

// Colors 1, 2 and 3 of pattern 2
#define IS31FL3194_COL1_PATT2_R         0x20
#define IS31FL3194_COL1_PATT2_G         0x21
#define IS31FL3194_COL1_PATT2_B         0x22
#define IS31FL3194_COL2_PATT2_R         0x23
#define IS31FL3194_COL2_PATT2_G         0x24
#define IS31FL3194_COL2_PATT2_B         0x25
#define IS31FL3194_COL3_PATT2_R         0x26
#define IS31FL3194_COL3_PATT2_G         0x27
#define IS31FL3194_COL3_PATT2_B         0x28

// Colors 1, 2 and 3 of pattern 3
#define IS31FL3194_COL1_PATT3_R         0x30
#define IS31FL3194_COL1_PATT3_G         0x31
#define IS31FL3194_COL1_PATT3_B         0x32
#define IS31FL3194_COL2_PATT3_R         0x33
#define IS31FL3194_COL2_PATT3_G         0x34
#define IS31FL3194_COL2_PATT3_B         0x35
#define IS31FL3194_COL3_PATT3_R         0x36
#define IS31FL3194_COL3_PATT3_G         0x37
#define IS31FL3194_COL3_PATT3_B         0x38

#define IS31FL3194_P1_TS_T1_Time_SET    0x19
#define IS31FL3194_P1_T2_T3_Time_SET    0x1A
#define IS31FL3194_P1_TP_T4_Time_SET    0x1B
#define IS31FL3194_P2_TS_T1_Time_SET    0x29
#define IS31FL3194_P2_T2_T3_Time_SET    0x2A
#define IS31FL3194_P2_TP_T4_Time_SET    0x2B
#define IS31FL3194_P3_TS_T1_Time_SET    0x39
#define IS31FL3194_P3_T2_T3_Time_SET    0x3A
#define IS31FL3194_P3_TP_T4_Time_SET    0x3B

#define IS31FL3194_P1_COLOR_EN          0x1C
#define IS31FL3194_P2_COLOR_EN          0x2C
#define IS31FL3194_P3_COLOR_EN          0x3C

#define IS31FL3194_P1_COLOR_CYC_TIME    0x1D
#define IS31FL3194_P2_COLOR_CYC_TIME    0x2D
#define IS31FL3194_P3_COLOR_CYC_TIME    0x3D

#define IS31FL3194_P1_NXT               0x1E
#define IS31FL3194_P2_NXT               0x2E
#define IS31FL3194_P3_NXT               0x3E

#define IS31FL3194_P1_LOOP_TIMES        0x1F
#define IS31FL3194_P2_LOOP_TIMES        0x2F
#define IS31FL3194_P3_LOOP_TIMES        0x3F

#define IS31FL3194_COLOR_UPDATE         0x40

#define IS31FL3194_P1_UPDATE            0x41
#define IS31FL3194_P2_UPDATE            0x42
#define IS31FL3194_P3_UPDATE            0x43

#define IS31FL3194_RESET                0x4F

#define IS31FL3194_ADDRESS 0x53

#define Mode 1

// define times
#define t_0_03s 0x00
#define t_0_13s 0x01
#define t_0_26s 0x02
#define t_0_38s 0x03
#define t_0_51s 0x04
#define t_0_77s 0x05
#define t_1_04s 0x06
#define t_1_60s 0x07
#define t_2_10s 0x08
#define t_2_60s 0x09
#define t_3_10s 0x0A
#define t_4_20s 0x0B
#define t_5_20s 0x0C
#define t_6_20s 0x0D
#define t_7_30s 0x0E
#define t_8_30s 0x0F

// define pattern times
#define TS t_2_10s  // Start time
#define T1 t_2_10s  // Rise time
#define T2 t_1_04s  // Hold time
#define T3 t_2_10s  // Fall time
#define T4 t_2_10s  // Off time
#define TP t_2_10s  // Time between pulses

// define cycle times
#define endless 0x00
#define once    0x15
#define twice   0x2A
#define thrice  0x3F

// light intensity (fraction of current max)
#define Imax_frac  0x80 // Imax_frac/256 * Imax = current

enum RGBColors {
	off = 0,
	red = 1,
  green = 2,
  blue = 3,
  yellow = 4,
  magenta = 5,
  cyan = 6,
  white = 7
};

/*
// allowed colors
#define off     0
#define red     1
#define green   2
#define blue    3
#define yellow  4
#define magenta 5
#define cyan    6
*/

#include "Wire.h"

class RGBled
{
public: 
  RGBled() {};

  void begin();
  void end();
  void setColor(RGBColors color);
  void setColor(uint8_t red, uint8_t green, uint8_t blue);

  void setColorBlue(uint8_t blue = 0xFF);
  void setColorRed(uint8_t red = 0xFF);
  void setColorGreen(uint8_t green = 0xFF);

  /* intensity from 1 (lowest) to 8 (full) */
  void setIntensity(uint8_t power) {
    scale_factor = 8 - power;
    if (scale_factor < 0) {
      scale_factor = 0;
    }
    /*
    if (power > 8) {
      writeByte(IS31FL3194_ADDRESS, IS31FL3194_CURRENT_BAND, 0x15);
    }
    if (power > 16) {
      writeByte(IS31FL3194_ADDRESS, IS31FL3194_CURRENT_BAND, 0x2A);
    }
    if (power > 32) {
      writeByte(IS31FL3194_ADDRESS, IS31FL3194_CURRENT_BAND, 0x3F);
    }
    */
  }

private:
  void init();
  uint8_t getChipID();
  void reset();
  void powerDown();
  void powerUp();
  void ledBlink(RGBColors color, uint32_t duration);
  void I2Cscan();
  void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);
  uint8_t readByte(uint8_t address, uint8_t subAddress);

  uint8_t _blue;
  uint8_t _green;
  uint8_t _red;

  int8_t scale_factor = 4;
};

#endif
