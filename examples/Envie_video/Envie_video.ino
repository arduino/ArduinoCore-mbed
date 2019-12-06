#include "anx7625_public.h"
#include "stm32h747i_eval_lcd.h"
#include "defs.h"
#include "image_320x240_argb8888.h"
#include "life_augmented_argb8888.h"

static const uint32_t * Images[] = 
{
  image_320x240_argb8888,
  life_augmented_argb8888,  
};
static uint32_t ImageIndex = 0;

struct i2c_client anx7625;

void setup() {
  // put your setup code here, to run once:
  printf("starting\n");

  /* Initialize the LCD   */
  if (BSP_LCD_Init() != LCD_OK)
  {
    Error_Handler();
  }

  BSP_LCD_LayerDefaultInit(0, LAYER0_ADDRESS);
  BSP_LCD_SelectLayer(0);

  /* Display example brief   */
  LCD_BriefDisplay();

  anx7625_i2c_probe(&anx7625);
/*
  anx7625_debug("dumpall");
  anx7625_debug("dpi 0");
  anx7625_debug("dsi 0");
  anx7625_start_dp();
  anx7625_debug("edid");
*/

  /* Infinite loop */
  while (1)
  {
    CopyBuffer((uint32_t *)Images[ImageIndex ++], (uint32_t *)LAYER0_ADDRESS, (BSP_LCD_GetXSize() - 320) / 2, 160, 320, 240);

    if (ImageIndex >= 2)
    {
      ImageIndex = 0;
    }

    printf("LOOPING\n");
    /* Wait some time before switching to next stage */
    HAL_Delay(2000);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
