#include "camera.h"
#include "Portenta_lvgl.h"
#include "Portenta_Video.h"

#if 0
#include "gc2145.h"
GC2145 galaxyCore;
Camera cam(galaxyCore);
#define IMAGE_MODE CAMERA_RGB565
#else
#include "himax.h"
HM01B0 himax;
Camera cam(himax);
#define IMAGE_MODE CAMERA_GRAYSCALE
#endif

/*
  Other buffer instantiation options:
  FrameBuffer fb(0x30000000);
  FrameBuffer fb(320,240,2);
*/
FrameBuffer fb;

unsigned long lastUpdate = 0;


void blinkLED(uint32_t count = 0xFFFFFFFF)
{
  pinMode(LED_BUILTIN, OUTPUT);
  while (count--) {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(50);                       // wait for a second
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED off by making the voltage LOW
    delay(50);                       // wait for a second
  }
}
void LCD_ST7701_Init();

uint32_t palette[256];

void setup() {
  pinMode(PA_1, OUTPUT);
  digitalWrite(PA_1, HIGH);

  pinMode(PD_4, OUTPUT);
  digitalWrite(PD_4, LOW);

  // Init the cam QVGA, 30FPS
  if (!cam.begin(CAMERA_R320x240, IMAGE_MODE, 30)) {
    blinkLED();
  }

  for (int i = 0; i < 256; i++) {
    palette[i] = 0xFF000000 | (i << 16) | (i << 8) | i;
  }

  giga_init_video();
  LCD_ST7701_Init();

  blinkLED(5);

  stm32_configue_CLUT((uint32_t*)palette);
  stm32_LCD_Clear(0);
  stm32_LCD_Clear(0);
  stm32_LCD_Clear(0);
  stm32_LCD_Clear(0);
}

#include "avir.h"

void loop() {

  lastUpdate = millis();

  // Grab frame and write to serial
  if (cam.grabFrame(fb, 3000) == 0) {
    //avir :: CImageResizer<> ImageResizer( 8 );
    //ImageResizer.resizeImage( (uint8_t*)fb.getBuffer(), 320, 240, 0, (uint8_t*)outfb.getBuffer(), 480, 320, 1, 0 );
    static FrameBuffer outfb(0x30000000);
    //static FrameBuffer outfb(getFramebufferEnd());
    for (int i = 0; i < 300; i++) {
      for (int j = 0; j < 240; j++) {
        //((uint8_t*)outfb.getBuffer())[j * 240 + (320 - i)] = ((uint8_t*)fb.getBuffer())[i * 320 + j];
#if 1
        ((uint8_t*)outfb.getBuffer())[j * 2 + (i * 2) * 480] = ((uint8_t*)fb.getBuffer())[i + j * 320];
        ((uint8_t*)outfb.getBuffer())[j * 2 + (i * 2) * 480 + 1] = ((uint8_t*)fb.getBuffer())[i + j * 320];
        ((uint8_t*)outfb.getBuffer())[j * 2 + (i*2+1) * 480] = ((uint8_t*)fb.getBuffer())[i + j * 320];
        ((uint8_t*)outfb.getBuffer())[j * 2 + (i*2+1) * 480 + 1] = ((uint8_t*)fb.getBuffer())[i + j * 320];
#endif
#if 0
        ((uint8_t*)outfb.getBuffer())[j + i * 240] = ((uint8_t*)fb.getBuffer())[i + j * 320];
#endif
      }
    }
    //stm32_LCD_DrawImage((void*)outfb.getBuffer(), (void*)getNextFrameBuffer(), 240, 320, DMA2D_INPUT_L8);
    stm32_LCD_DrawImage((void*)outfb.getBuffer(), (void*)getNextFrameBuffer(), 480, 640, DMA2D_INPUT_L8);
    //stm32_LCD_DrawImage((void*)fb.getBuffer(), (void*)getNextFrameBuffer(), 320, 240, DMA2D_INPUT_L8);
  } else {
    blinkLED(20);
  }
}
