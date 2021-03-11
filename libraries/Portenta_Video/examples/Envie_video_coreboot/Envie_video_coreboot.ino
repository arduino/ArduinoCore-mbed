#include "Portenta_Video.h"
#include "image_320x240_argb8888.h"
#include "SDRAM.h"
#include "mbed.h"

struct edid recognized_edid;

void setup() {
  // put your setup code here, to run once:
  int ret = -1;

  ret = anx7625_init(0);
  if(ret < 0) {
    printf("Cannot continue, anx7625 init failed.\n");
    while(1);
  }

  anx7625_wait_hpd_event(0);
  anx7625_dp_get_edid(0, &recognized_edid);
  anx7625_dp_start(0, &recognized_edid, EDID_MODE_640x480_60Hz);

  SDRAM.begin(getFramebufferEnd());
  while (1) {
    stm32_LCD_DrawImage((void*)texture_raw, (void *)getNextFrameBuffer(), 300, 300, DMA2D_INPUT_RGB565);
    stm32_LCD_DrawImage((void*)texture_raw, (void *)getNextFrameBuffer(), 300, 300, DMA2D_INPUT_RGB565);
  }
}

int i = 0;

void loop() {
  printf("Now: %d\n", millis());

  delay(1000);
}
