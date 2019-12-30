#include "Envie_video_coreboot.h"
#include "stm32h747i_eval_lcd.h"
#include "defs.h"
#include "image_320x240_argb8888.h"

struct edid recognized_edid;

static const uint32_t * Images[] =
{
  image_320x240_argb8888,
};
static uint32_t ImageIndex = 0;

mbed::DigitalOut video_on(PK_2);
mbed::DigitalOut video_rst(PJ_3);

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  video_on = 1;
  delay(10);
  video_rst = 1;
  delay(10);
  int ret = anx7625_init(0);
  printf("anx7625_init returned %d\n", ret);
  anx7625_dp_get_edid(0, &recognized_edid);
  set_display_mode(&recognized_edid, EDID_MODE_640x480_60Hz);
  anx7625_dp_start(0, &recognized_edid);

  if (BSP_LCD_Init() != LCD_OK)
  {
    printf("BSP_LCD_Init failed\n");
    Error_Handler();
  }

  BSP_LCD_LayerDefaultInit(0, LAYER0_ADDRESS);
  BSP_LCD_SelectLayer(0);

  /* Display example brief   */
  LCD_BriefDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:
  CopyBuffer((uint32_t *)Images[0], (uint32_t *)LAYER0_ADDRESS, (BSP_LCD_GetXSize() - 320) / 2, 160, 320, 240);

  /* Wait some time before switching to next stage */
  HAL_Delay(2000);
}
