#include "Envie_video_coreboot.h"
#include "stm32h747i_eval_lcd.h"
#include "defs.h"
#include "image_320x240_argb8888.h"

struct edid recognized_edid;

mbed::DigitalOut video_on(PK_2);
mbed::DigitalOut video_rst(PJ_3);

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  video_on = 1;
  delay(10);
  video_rst = 1;
  delay(10);

/*
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LAYER0_ADDRESS);
  BSP_LCD_SelectLayer(0);
*/

  int ret = anx7625_init(0);
  printf("anx7625_init returned %d\n", ret);
  anx7625_dp_get_edid(0, &recognized_edid);
  edid_set_framebuffer_bits_per_pixel(&recognized_edid, 16, 0);
  set_display_mode(&recognized_edid, EDID_MODE_640x480_60Hz);
  anx7625_dp_start(0, &recognized_edid);

  while (millis() < 4000) {
    stm32_LCD_Clear(0xFFFF00);
  }

  //stm32_LCD_DrawImage((void*)image_320x240_argb8888, NULL, 320, 240, DMA2D_INPUT_ARGB8888);

  /* Display example brief   */
  //LCD_BriefDisplay();

  //BSP_LCD_DrawRect(100, 100, 100, 100);


  int i = 0;
  while (i < texture_raw_len) {
    *(uint8_t*)(LAYER0_ADDRESS + (i*2)) = texture_raw[i];
    *(uint8_t*)(LAYER0_ADDRESS + (i*2) +1) = texture_raw[i];
    i++;
  }

}

int i = 0;

void loop() {
  

  delay(1000);

  i = random(0, 0xFFFFFF);
  printf("now: %d\n", millis());
  //stm32_LCD_Clear(i);

/*
  uint8_t data[10];
  uint8_t data2[10] = {1,2,3,4,5,6,7,8,9,10};
  memcpy((void*)LAYER0_ADDRESS, data2, 10);
  memcpy(data, (void*)LAYER0_ADDRESS, 10);
  for (int i=0; i<10; i++) {
    printf("%x", data[i]);
  }
  printf("\n");

  // put your main code here, to run repeatedly:
  CopyBuffer((uint32_t *)Images[0], (uint32_t *)LAYER0_ADDRESS, (BSP_LCD_GetXSize() - 320) / 2, 160, 320, 240);
*/

  /* Wait some time before switching to next stage */
  delay(1000);
}
