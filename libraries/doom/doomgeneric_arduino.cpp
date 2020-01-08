//doomgeneric for arduino

#include "Arduino.h"
#include "Envie_video_coreboot.h"

#define sleep _sleep
#include "malloc.h"

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>

static int FrameBufferFd = -1;
static int* FrameBuffer = 0;

static int KeyboardFd = -1;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned int s_PositionX = 0;
static unsigned int s_PositionY = 0;

static unsigned char convertToDoomKey(unsigned char scancode)
{
    unsigned char key = 0;

    switch (scancode)
    {
    case 0x9C:
    case 0x1C:
        key = KEY_ENTER;
        break;
    case 0x01:
        key = KEY_ESCAPE;
        break;
    case 0xCB:
    case 0x4B:
        key = KEY_LEFTARROW;
        break;
    case 0xCD:
    case 0x4D:
        key = KEY_RIGHTARROW;
        break;
    case 0xC8:
    case 0x48:
        key = KEY_UPARROW;
        break;
    case 0xD0:
    case 0x50:
        key = KEY_DOWNARROW;
        break;
    case 0x1D:
        key = KEY_FIRE;
        break;
    case 0x39:
        key = KEY_USE;
        break;
    case 0x2A:
    case 0x36:
        key = KEY_RSHIFT;
        break;
    case 0x15:
        key = 'y';
        break;
    default:
        break;
    }

    return key;
}

static void addKeyToQueue(int pressed, unsigned char keyCode)
{
	//printf("key hex %x decimal %d\n", keyCode, keyCode);

  unsigned char key = convertToDoomKey(keyCode);

  unsigned short keyData = (pressed << 8) | key;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

struct edid recognized_edid;

mbed::DigitalOut video_on(PK_2);
mbed::DigitalOut video_rst(PJ_3);

uint32_t LCD_X_Size = 0, LCD_Y_Size = 0;
DMA2D_HandleTypeDef    DMA2D_Handle;

static void DMA2D_Init(uint16_t xsize, uint16_t ysize)
{
  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
  DMA2D_Handle.Init.Mode         = DMA2D_M2M_PFC;
  DMA2D_Handle.Init.ColorMode    = DMA2D_OUTPUT_RGB565;
  DMA2D_Handle.Init.OutputOffset = LCD_X_Size - xsize;
  DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
  DMA2D_Handle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */

  /*##-2- DMA2D Callbacks Configuration ######################################*/
  DMA2D_Handle.XferCpltCallback  = NULL;

  /*##-3- Foreground Configuration ###########################################*/
  DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_OUTPUT_RGB565;
  //DMA2D_Handle.LayerCfg[1].ChromaSubSampling = cssMode;
  DMA2D_Handle.LayerCfg[1].InputOffset = 0;
  DMA2D_Handle.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
  DMA2D_Handle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

  DMA2D_Handle.Instance          = DMA2D;

  /*##-4- DMA2D Initialization     ###########################################*/
  HAL_DMA2D_Init(&DMA2D_Handle);
  HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1);
}

void DG_Init()
{

  malloc_addblock((void*)0xC0200000, 0x600000);

  int ret = -1;

  while (ret < 0) {
    video_on = 0;
    delay(10);
    video_rst = 0;
    delay(100);

    video_on = 1;
    delay(100);
    video_rst = 1;

    ret = anx7625_init(0);
    printf("after init\n");

    if (ret < 0) {
      printf("anx7625_init returned %d\n", ret);
    }
  }
  anx7625_dp_get_edid(0, &recognized_edid);
  //edid_set_framebuffer_bits_per_pixel(&recognized_edid, 16, 0);
  //set_display_mode(&recognized_edid, EDID_MODE_720x480_60Hz);
  //anx7625_dp_start(0, &recognized_edid, EDID_MODE_1280x720_60Hz);
  anx7625_dp_start(0, &recognized_edid, EDID_MODE_640x480_60Hz);

  LCD_X_Size = stm32_getXSize();
  LCD_Y_Size = stm32_getYSize();

  DMA2D_Init(DOOMGENERIC_RESX, DOOMGENERIC_RESY);
}

static void handleKeyInput()
{
    unsigned char scancode = 0;

#if 0
    if (read(KeyboardFd, &scancode, 1) > 0)
    {
        unsigned char keyRelease = (0x80 & scancode);

        scancode = (0x7F & scancode);

        //printf("scancode:%x pressed:%d\n", scancode, 0 == keyRelease);

        if (0 == keyRelease)
        {
            addKeyToQueue(1, scancode);
        }
        else
        {
            addKeyToQueue(0, scancode);
        }
    }
#endif
}

static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst)
{
  uint32_t xPos, yPos, destination;

  /*##-1- calculate the destination transfer address  ############*/
  xPos = (stm32_getXSize() - DOOMGENERIC_RESX) / 2;
  yPos = (stm32_getYSize() - DOOMGENERIC_RESY) / 2;

  destination = (uint32_t)pDst + ((yPos * stm32_getXSize()) + xPos) * 4;

  HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 25);  /* wait for the previous DMA2D transfer to ends */
  /* copy the new decoded frame to the LCD Frame buffer*/
  HAL_DMA2D_Start(&DMA2D_Handle, (uint32_t)pSrc, destination, DOOMGENERIC_RESX, DOOMGENERIC_RESY);

}

void DG_DrawFrame()
{
  printf("calling DG_DrawFrame\n");
  DMA2D_CopyBuffer((uint32_t *)DG_ScreenBuffer, (uint32_t *)getNextFrameBuffer());
  //handleKeyInput();
}

void DG_SleepMs(uint32_t ms)
{
    delay(ms);
}

uint32_t DG_GetTicksMs()
{
  return millis();
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
   return 0;
}

void DG_SetWindowTitle(const char * title)
{
}
