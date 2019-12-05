#include "anx7625_public.h"
#include "RPC_internal.h"
#include "lcd.h"

extern "C" {
  DMA2D_HandleTypeDef Dma2dHandle;
  extern DSI_HandleTypeDef hdsi_eval;
}

struct i2c_client anx7625;

void setup() {
  RPC1.begin();
  delay(3000);
  // put your setup code here, to run once:
  doStuff();
  anx7625_i2c_probe(&anx7625);
  anx7625_debug("dumpall");
  anx7625_debug("dpi 0");
  anx7625_debug("dsi 0");
  anx7625_start_dp();
  anx7625_debug("edid");
}

extern const uint32_t * Images[];
extern uint32_t ImageIndex;
extern const uint32_t Buffers[];

void loop() {
  // put your main code here, to run repeatedly:

  const int LCD_X_Size = 800;

  CopyBuffer((uint32_t *)Images[ImageIndex ++], (uint32_t *)LAYER0_ADDRESS, (LCD_X_Size - 320) / 2, 160, 320, 240);

  if (ImageIndex >= 2)
  {
    ImageIndex = 0;
  }

  printf("looping\n");

  /* Wait some time before switching to next stage */
  HAL_Delay(2000);
}

extern "C" {

  /*
    void DSI_IRQHandler(void)
    {
      HAL_DSI_IRQHandler(&hdsi_eval);
    }
  */
  /*
    void DMA2D_IRQHandler(void)
    {
      HAL_DMA2D_IRQHandler(&Dma2dHandle);
    }
  */
}
