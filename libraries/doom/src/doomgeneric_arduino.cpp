//doomgeneric for arduino

#include "Arduino.h"
#include "mbed.h"
#include "Arduino_H7_Video.h"
#include "dsi.h"

#define sleep _sleep

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

uint32_t LCD_X_Size = 0, LCD_Y_Size = 0;
DMA2D_HandleTypeDef    DMA2D_Handle;

extern struct color colors[];

static void InitCLUT(uint32_t * clut)
{
  uint32_t  red = 0, green = 0, blue = 0;
  uint32_t  i = 0x00;

  /* Color map generation */
  for (i = 0; i < 256; i++)
  {
    /* Generate red, green and blue values */
    red = (i * 8) % 256;
    green = (i * 6) % 256;
    blue = (i * 4) % 256;

    red = red << 16;
    green = green << 8;

    /* Store the 32-bit value */
    clut[i] = 0xFF000000 | (red + green + blue);
  }
}

uint32_t __ALIGNED(32) L8_CLUT[256];
static DMA2D_CLUTCfgTypeDef clut;

static void DMA2D_Init(uint16_t xsize, uint16_t ysize)
{
  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
  DMA2D_Handle.Init.Mode         = DMA2D_M2M_PFC;
  DMA2D_Handle.Init.ColorMode    = DMA2D_OUTPUT_RGB565;
  DMA2D_Handle.Init.OutputOffset = 0; //LCD_X_Size - xsize;
  DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
  DMA2D_Handle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */

  /*##-2- DMA2D Callbacks Configuration ######################################*/
  DMA2D_Handle.XferCpltCallback  = NULL;

  /*##-3- Foreground Configuration ###########################################*/
  DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA; //DMA2D_NO_MODIF_ALPHA;
  DMA2D_Handle.LayerCfg[1].InputAlpha = 0x00;
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_L8; //DMA2D_OUTPUT_RGB565;
  //DMA2D_Handle.LayerCfg[1].ChromaSubSampling = cssMode;
  DMA2D_Handle.LayerCfg[1].InputOffset = 0; //LCD_Y_Size - ysize;
  DMA2D_Handle.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
  DMA2D_Handle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

  DMA2D_Handle.Instance          = DMA2D;

  /*##-4- DMA2D Initialization     ###########################################*/
  HAL_DMA2D_Init(&DMA2D_Handle);
  HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1);

  memcpy(L8_CLUT, colors, 256 * 4);
  clut.pCLUT = (uint32_t *)L8_CLUT; //(uint32_t *)colors;
  clut.CLUTColorMode = DMA2D_CCM_ARGB8888;
  clut.Size = 0xFF;

#ifdef CORE_CM7
  SCB_CleanInvalidateDCache();
  SCB_InvalidateICache();
  //SCB_InvalidateDCache_by_Addr(clut.pCLUT, clut.Size);
#endif

  HAL_DMA2D_CLUTLoad(&DMA2D_Handle, clut, 1);
  HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 100);
}

Arduino_H7_Video display(640, 480, USBCVideo);

uint32_t fbs[2];

void DG_Init()
{
  display.begin();
}

void DG_OnPaletteReload() {
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

//#define DEBUG_CM7_VIDEO

static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst)
{
  uint32_t xPos, yPos, destination;

  /*##-1- calculate the destination transfer address  ############*/
  xPos = (display.width() - DOOMGENERIC_RESX) / 2;
  yPos = (display.height() - DOOMGENERIC_RESY) / 2;

  destination = (uint32_t)pDst; // + ((yPos * stm32_getXSize()) + xPos) * 4;

  HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 100);  /* wait for the previous DMA2D transfer to ends */
  /* copy the new decoded frame to the LCD Frame buffer*/
  HAL_DMA2D_Start(&DMA2D_Handle, (uint32_t)pSrc, destination, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
#if defined(CORE_CM7) && !defined(DEBUG_CM7_VIDEO) 
  HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 100);  /* wait for the previous DMA2D transfer to ends */
#endif
}

void DG_DrawFrame()
{
  uint32_t fb = dsi_getCurrentFrameBuffer();
#ifdef CORE_CM7
  //SCB_CleanInvalidateDCache();
  //SCB_InvalidateICache();
  SCB_InvalidateDCache_by_Addr((uint32_t *)fb, DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
#endif

  DMA2D_CopyBuffer((uint32_t *)DG_ScreenBuffer, (uint32_t *)fb);
  dsi_drawCurrentFrameBuffer();
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
