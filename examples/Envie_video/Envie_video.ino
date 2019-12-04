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
  delay(2000);
  // put your setup code here, to run once:
  anx7625_i2c_probe(&anx7625);
  doStuff();
}

void loop() {
  // put your main code here, to run repeatedly:

}

extern "C" {

  void DSI_IRQHandler(void)
  {
    HAL_DSI_IRQHandler(&hdsi_eval);
  }

  void DMA2D_IRQHandler(void)
  {
    HAL_DMA2D_IRQHandler(&Dma2dHandle);
  }
}
