/**
  ******************************************************************************
  * @file    st7701.cpp
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Arduino.h"
#include "st7701.h"
#include "dsi.h"
#include "video_modes.h"

/* Private define ------------------------------------------------------------*/
/* Command2 BKx selection command */
#define DSI_CMD2BKX_SEL           0xFF
#define DSI_CMD2BK1_SEL           0x11
#define DSI_CMD2BK0_SEL           0x10
#define DSI_CMD2BKX_SEL_NONE      0x00

/* Command2, BK0 commands */
#define DSI_CMD2_BK0_PVGAMCTRL    0xB0 /* Positive Voltage Gamma Control */
#define DSI_CMD2_BK0_NVGAMCTRL    0xB1 /* Negative Voltage Gamma Control */
#define DSI_CMD2_BK0_LNESET       0xC0 /* Display Line setting */
#define DSI_CMD2_BK0_PORCTRL      0xC1 /* Porch control */
#define DSI_CMD2_BK0_INVSEL       0xC2 /* Inversion selection, Frame Rate Control */

/* Command2, BK1 commands */
#define DSI_CMD2_BK1_VRHS         0xB0 /* Vop amplitude setting */
#define DSI_CMD2_BK1_VCOM         0xB1 /* VCOM amplitude setting */
#define DSI_CMD2_BK1_VGHSS        0xB2 /* VGH Voltage setting */
#define DSI_CMD2_BK1_TESTCMD      0xB3 /* TEST Command Setting */
#define DSI_CMD2_BK1_VGLS         0xB5 /* VGL Voltage setting */
#define DSI_CMD2_BK1_PWCTLR1      0xB7 /* Power Control 1 */
#define DSI_CMD2_BK1_PWCTLR2      0xB8 /* Power Control 2 */
#define DSI_CMD2_BK1_SPD1         0xC1 /* Source pre_drive timing set1 */
#define DSI_CMD2_BK1_SPD2         0xC2 /* Source EQ2 Setting */
#define DSI_CMD2_BK1_MIPISET1     0xD0 /* MIPI Setting 1 */

#define MIPI_DCS_SOFT_RESET       0x01
#define MIPI_DCS_EXIT_SLEEP_MODE  0x11

#define hdsi_eval                 dsi

#define LCD_ST7701_ID             0x00  // VC (Virtual channel, for using muliple displays)

#define Delay(x)                  delay(x)

/* Private macro -------------------------------------------------------------*/
#define SSD_MODE(a,b)
#define Set_POWER(a,b,c,d)
#define Set_STANDBY()
#define Set_BOOST(a,b,c,d)
#define Set_RESET(a,b)
#define SSD_LANE(a,b)

/* Private variables ---------------------------------------------------------*/
const uint16_t _E5[17] = {0xE5,0x0E,0x2D,0xA0,0xa0,0x10,0x2D,0xA0,0xA0,0x0A,0x2D,0xA0,0xA0,0x0C,0x2D,0xA0,0xA0};
const uint16_t _E8[17] = {0xE8,0x0D,0x2D,0xA0,0xA0,0x0F,0x2D,0xA0,0xA0,0x09,0x2D,0xA0,0xA0,0x0B,0x2D,0xA0,0xA0};
const uint16_t _ED[17] = {0xED,0xAB,0x89,0x76,0x54,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x10,0x45,0x67,0x98,0xBA};

/* Private function prototypes -----------------------------------------------*/
static void Generic_Long_Write(uint8_t* pdata, int length);
static void DCS_Short_Write_NP(uint8_t data0);
static void Generic_Short_Write_1P(uint8_t data0, uint8_t data1);
static void DCS_Short_Read_NP(uint8_t data0, int length, uint8_t* p_data);

/* Functions -----------------------------------------------------------------*/

void st7701_init(enum edid_modes mode) {
  struct edid _edid;
  struct display_timing dt;

  //DSI Configuration
  dt.pixelclock   = envie_known_modes[mode].pixel_clock;
  dt.hactive      = envie_known_modes[mode].hactive;
  dt.hsync_len    = envie_known_modes[mode].hsync_len;
  dt.hback_porch  = envie_known_modes[mode].hback_porch;
  dt.hfront_porch = envie_known_modes[mode].hfront_porch;
  dt.vactive      = envie_known_modes[mode].vactive;
  dt.vsync_len    = envie_known_modes[mode].vsync_len;
  dt.vback_porch  = envie_known_modes[mode].vback_porch;
  dt.vfront_porch = envie_known_modes[mode].vfront_porch;
  dt.hpol         = envie_known_modes[mode].hpol;
  dt.vpol         = envie_known_modes[mode].vpol;
  dsi_init(0, &_edid, &dt);

  DCS_Short_Write_NP(MIPI_DCS_SOFT_RESET);
  Delay(200);

  //ST7701S+IVO5.0
  DCS_Short_Write_NP(MIPI_DCS_EXIT_SLEEP_MODE);

  //------------------------------------------Bank0 Setting------------------------------------------------//
  //------------------------------------Display Control setting--------------------------------------------//
  Delay(800);

  const uint8_t Display_Control_0[] = {0xFF,0x77,0x01,0x00,0x00,0x10};  
  const uint8_t Display_Control_1[] = {0xC0,0x63,0x00};
  const uint8_t Display_Control_2[] = {0xC1,0x11,0x02};
  const uint8_t Display_Control_3[] = {0xC2,0x01,0x08};
  const uint8_t Display_Control_4[] = {0xCC,0x18};

  Generic_Long_Write((uint8_t*)Display_Control_0, sizeof(Display_Control_0));
  Generic_Long_Write((uint8_t*)Display_Control_1, sizeof(Display_Control_1));
  Generic_Long_Write((uint8_t*)Display_Control_2, sizeof(Display_Control_2));
  Generic_Long_Write((uint8_t*)Display_Control_3, sizeof(Display_Control_3));
  Generic_Long_Write((uint8_t*)Display_Control_4, sizeof(Display_Control_4));

  //-------------------------------------Gamma Cluster Setting---------------------------------------------//
  const uint8_t _B0[] = {0xB0, 0x40, 0xc9, 0x91, 0x0d,
                        0x12, 0x07, 0x02, 0x09, 0x09, 
                        0x1f, 0x04, 0x50, 0x0f, 0xe4, 
                        0x29, 0xdf};
  
  const uint8_t _B1[] = {0xB1, 0x40, 0xcb, 0xd0, 0x11,
                        0x92, 0x07, 0x00, 0x08, 0x07, 
                        0x1c, 0x06, 0x53, 0x12, 0x63, 
                        0xeb, 0xdf};

  Generic_Long_Write((uint8_t*)_B0, sizeof(_B0));
  Generic_Long_Write((uint8_t*)_B1, sizeof(_B1));

  //---------------------------------------End Gamma Setting-----------------------------------------------//
  //------------------------------------End Display Control setting----------------------------------------//
  //-----------------------------------------Bank0 Setting End---------------------------------------------//

  //-------------------------------------------Bank1 Setting-----------------------------------------------//
  //-------------------------------- Power Control Registers Initial --------------------------------------//
  const uint8_t _FF1[] = {DSI_CMD2BKX_SEL,0x77,0x01,0x00,0x00,DSI_CMD2BK1_SEL};
  Generic_Long_Write ((uint8_t*)_FF1, sizeof(_FF1));
  
  Generic_Short_Write_1P (DSI_CMD2_BK1_VRHS,0x65);
  //-------------------------------------------Vcom Setting------------------------------------------------//
  Generic_Short_Write_1P (DSI_CMD2_BK1_VCOM,0x34);
  //-----------------------------------------End Vcom Setting----------------------------------------------//
  Generic_Short_Write_1P (DSI_CMD2_BK1_VGHSS,0x87);
  Generic_Short_Write_1P (DSI_CMD2_BK1_TESTCMD,0x80);

  Generic_Short_Write_1P (DSI_CMD2_BK1_VGLS,0x49);
  Generic_Short_Write_1P (DSI_CMD2_BK1_PWCTLR1,0x85);

  Generic_Short_Write_1P (DSI_CMD2_BK1_PWCTLR2,0x20);
  Generic_Short_Write_1P (0xB9,0x10);
  Generic_Short_Write_1P (DSI_CMD2_BK1_SPD1,0x78);
  Generic_Short_Write_1P (DSI_CMD2_BK1_SPD2,0x78);
  Generic_Short_Write_1P (DSI_CMD2_BK1_MIPISET1,0x88);
  //---------------------------------End Power Control Registers Initial ----------------------------------//
  Delay(100);
  //---------------------------------------------GIP Setting-----------------------------------------------//
  const uint8_t _E0[] = {0xE0,0x00,0x00,0x02};
  Generic_Long_Write((uint8_t*)_E0, sizeof(_E0));
  //----------------------------------GIP------------------------------------------------------------------//
  const uint8_t _E1[] = {0xE1,0x08,0x00,0x0A,0x00,0x07,0x00,0x09,0x00,0x00,0x33,0x33};
  const uint8_t _E2[] = {0xE2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  Generic_Long_Write((uint8_t*)_E1, sizeof(_E1));
  Generic_Long_Write((uint8_t*)_E2, sizeof(_E2));

  //-------------------------------------------------------------------------------------------------------//
  const uint8_t _E3[] = {0xE3,0x00,0x00,0x33,0x33};
  const uint8_t _E4[] = {0xE4,0x44,0x44};
  Generic_Long_Write((uint8_t*)_E3, sizeof(_E3));
  Generic_Long_Write((uint8_t*)_E4, sizeof(_E4));
  
  const uint8_t _E5[] = {0xE5,0x0E,0x60,0xA0,0xa0,0x10,0x60,0xA0,0xA0,0x0A,0x60,0xA0,0xA0,0x0C,0x60,0xA0,0xA0};
  Generic_Long_Write((uint8_t*)_E5, sizeof(_E5));
  
  const uint8_t _E6[] = {0xE6,0x00,0x00,0x33,0x33};
  const uint8_t _E7[] = {0xE7,0x44,0x44};
  Generic_Long_Write((uint8_t*)_E6, sizeof(_E6));
  Generic_Long_Write((uint8_t*)_E7, sizeof(_E7));

  const uint8_t _E8[] = {0xE8,0x0D,0x60,0xA0,0xA0,0x0F,0x60,0xA0,0xA0,0x09,0x60,0xA0,0xA0,0x0B,0x60,0xA0,0xA0};
  Generic_Long_Write((uint8_t*)_E8, sizeof(_E8));

  const uint8_t _EB[] = {0xEB,0x02,0x01,0xE4,0xE4,0x44,0x00,0x40};
  const uint8_t _EC[] = {0xEC,0x02,0x01};
  Generic_Long_Write((uint8_t*)_EB, sizeof(_EB));
  Generic_Long_Write((uint8_t*)_EC, sizeof(_EC));

  const uint8_t _ED[17] = {0xED,0xAB,0x89,0x76,0x54,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x10,0x45,0x67,0x98,0xBA};
  Generic_Long_Write((uint8_t*)_ED, sizeof(_ED));

  //--------------------------------------------End GIP Setting-----------------------------------------------//
  //------------------------------ Power Control Registers Initial End----------------------------------------//
  //------------------------------------------Bank1 Setting---------------------------------------------------//
  const uint8_t _FF2[] = {DSI_CMD2BKX_SEL,0x77,0x01,0x00,0x00,DSI_CMD2BKX_SEL_NONE};
  Generic_Long_Write ((uint8_t*)_FF2, sizeof(_FF2));

  Delay(10);
  DCS_Short_Write_NP(0x29);
  Delay(200);

  SSD_MODE(0,1);
}

void st7701_DisplayOn() {
  Set_POWER(1,1,1,1);//1.8V ON, 2.8V ON, 5V ON, BL ON
}

void st7701_PowerOffSequence() {
  DCS_Short_Write_NP(0x28);
  Delay(200);
  DCS_Short_Write_NP(0x10);
  Delay(100);
  Set_STANDBY();//Video transfer stop
  Delay(50);
    
  Set_RESET(1,0);//MIPI RESET 1, LCD RESET 0
  Delay(50);
  Set_RESET(0,0);//MIPI RESET 0, LCD RESET 0
  Delay(50);

  Set_POWER(1,1,0,1);//1.8V ON, 2.8V ON, 5V OFF, BL ON
  Delay(50);

  Set_BOOST(5.00, 5.00, 0x81, 50);//VDD, VEE, OFF:VDD->VEE, 10ms
  Delay(50);

  Set_POWER(1,0,0,1);//1.8V ON, 2.8V OFF, 5V OFF, BL ON
  Delay(100);
  Set_POWER(0,0,0,0);//1.8V OFF, 2.8V OFF, 5V OFF, BL OFF
}

void st7701_ReadOperation() {
  uint8_t BUFFER[8];

  //Clean memory: BUFFER  
  memset(BUFFER, 0, sizeof(BUFFER));//BUFFER size: 8 Bytes

  //Read value to BUFFER
  DCS_Short_Read_NP(0xDA, 1, BUFFER+0);
  DCS_Short_Read_NP(0xDB, 1, BUFFER+1);
  DCS_Short_Read_NP(0xDC, 1, BUFFER+2);
}

void DCS_Short_Write_NP(uint8_t data0) {
  HAL_DSI_ShortWrite(&hdsi_eval, LCD_ST7701_ID, DSI_DCS_SHORT_PKT_WRITE_P1, data0, 0x00); // DSI_DCS_SHORT_PKT_WRITE_P0
}

void Generic_Short_Write_1P(uint8_t data0, uint8_t data1) {
  HAL_DSI_ShortWrite(&hdsi_eval, LCD_ST7701_ID, DSI_GEN_SHORT_PKT_WRITE_P1, data0, data1);
}

void Generic_Long_Write(uint8_t* pdata, int length) {
  uint8_t data_buf[19] = {0};     // data_buf size with command(*pdata) = 20 so data is alligned 4
  uint8_t data_length = length- 1;

  if (data_length > sizeof(data_buf))
    return;

  memcpy(data_buf, pdata+1, data_length);
  HAL_DSI_LongWrite(&hdsi_eval, LCD_ST7701_ID, DSI_DCS_LONG_PKT_WRITE, data_length, *pdata, data_buf);
}

void DCS_Short_Read_NP(uint8_t data0, int length, uint8_t* p_data) {
  HAL_DSI_Read(&hdsi_eval, LCD_ST7701_ID, p_data, length, DSI_DCS_SHORT_PKT_READ, data0, NULL);
}

/**** END OF FILE ****/