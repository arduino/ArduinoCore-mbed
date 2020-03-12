#include "Portenta_Video.h"
#include "video.h"
#include "QSPIFBlockDevice.h"
#include "config.h"
#include "AVI_parser.h"
#include "image.h"
#include "SDRAM.h"

struct edid recognized_edid;

mbed::DigitalOut video_on(PK_2);
mbed::DigitalOut video_rst(PJ_3);

extern "C" uint32_t JPEG_Decode_DMA(JPEG_HandleTypeDef *hjpeg, uint32_t FrameSourceAddress , uint32_t FrameSize, uint32_t DestAddress);

static uint32_t AVI_FILE_ADDRESS = ((uint32_t)(0x90000000) + (2048 * 4096)); // start of qspi flash, correct me later

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
JPEG_HandleTypeDef    JPEG_Handle;
DMA2D_HandleTypeDef    DMA2D_Handle;
static JPEG_ConfTypeDef       JPEG_Info;

uint32_t LCD_X_Size = 0, LCD_Y_Size = 0;
static uint32_t FrameRate;

extern __IO uint32_t Jpeg_HWDecodingEnd;

AVI_CONTEXT AVI_Handel;  /* AVI Parser Handle*/

int KMP(char *x, int m, char *y, int n, int start = 0);

void setup() {
  // put your setup code here, to run once:

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
  anx7625_dp_start(0, &recognized_edid, EDID_MODE_800x600_59Hz);

  SDRAM.begin(getFramebufferEnd());

  delay(1000);

  /*
    QSPIFBlockDevice* block_device = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13,
        PF_10, PG_6, QSPIF_POLARITY_MODE_0, MBED_CONF_QSPIF_QSPI_FREQ);
    block_device->init();

    mbed::bd_size_t sector_size_at_address_0 = block_device->get_erase_size(0);

    printf("QSPIF BD size: %llu\n",         block_device->size());
    printf("QSPIF BD read size: %llu\n",    block_device->get_read_size());
    printf("QSPIF BD program size: %llu\n", block_device->get_program_size());

    printf("QSPIF BD erase size (at address 0): %llu\n", sector_size_at_address_0);

      char avi_header[] = {0x52, 0x49, 0x46, 0x46};
      char *buffer = (char *) malloc(sector_size_at_address_0);
      int i = 0;
      int res = 0;
      while (sector_size_at_address_0 * i < block_device->size()) {
        block_device->read(buffer, i, sector_size_at_address_0);
        res = KMP(avi_header, 4, buffer, sector_size_at_address_0);
        if (res != -1) {
          break;
        }
        i++;
      }

    char *buffer = (char *) malloc(sector_size_at_address_0);
    block_device->read(buffer, 0, sector_size_at_address_0);
  */

  //AVI_FILE_ADDRESS = (((uint32_t)(ardulogo_avi)));
  //  DumpHex((void*)buffer, 40);

  //AVI_FILE_ADDRESS = ((uint32_t)(0x90000000));
  AVI_FILE_ADDRESS = (((uint32_t)(video_envie_avi)));

  //DumpHex((void*)AVI_FILE_ADDRESS + 0x3210, 40);

  printf("Address: %x\n",  AVI_FILE_ADDRESS);

  // Deinitialize the device
  //block_device->deinit();

  LCD_X_Size = stm32_getXSize();
  LCD_Y_Size = stm32_getYSize();

  JPEG_Handle.Instance = JPEG;
  HAL_JPEG_Init(&JPEG_Handle);
}

int i = 0;

static void DMA2D_Init(uint16_t xsize, uint16_t ysize, uint32_t ChromaSampling)
{
  uint32_t cssMode = JPEG_420_SUBSAMPLING, inputLineOffset = 0;

  if (ChromaSampling == JPEG_420_SUBSAMPLING)
  {
    cssMode = DMA2D_CSS_420;

    inputLineOffset = xsize % 16;
    if (inputLineOffset != 0)
    {
      inputLineOffset = 16 - inputLineOffset;
    }
  }
  else if (ChromaSampling == JPEG_444_SUBSAMPLING)
  {
    cssMode = DMA2D_NO_CSS;

    inputLineOffset = xsize % 8;
    if (inputLineOffset != 0)
    {
      inputLineOffset = 8 - inputLineOffset;
    }
  }
  else if (ChromaSampling == JPEG_422_SUBSAMPLING)
  {
    cssMode = DMA2D_CSS_422;

    inputLineOffset = xsize % 16;
    if (inputLineOffset != 0)
    {
      inputLineOffset = 16 - inputLineOffset;
    }
  }

  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
  DMA2D_Handle.Init.Mode         = DMA2D_M2M_PFC;
  DMA2D_Handle.Init.ColorMode    = DMA2D_OUTPUT_RGB565;
  DMA2D_Handle.Init.OutputOffset = LCD_X_Size - xsize;
  DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
  DMA2D_Handle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */

  /*##-2- DMA2D Callbacks Configuration ######################################*/
  DMA2D_Handle.XferCpltCallback  = NULL;

  /*##-3- Foreground Configuration ###########################################*/
  DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_YCBCR;
  DMA2D_Handle.LayerCfg[1].ChromaSubSampling = cssMode;
  DMA2D_Handle.LayerCfg[1].InputOffset = inputLineOffset;
  DMA2D_Handle.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
  DMA2D_Handle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

  DMA2D_Handle.Instance          = DMA2D;

  /*##-4- DMA2D Initialization     ###########################################*/
  HAL_DMA2D_Init(&DMA2D_Handle);
  HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1);
}

static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t ImageWidth, uint16_t ImageHeight)
{

  uint32_t xPos, yPos, destination;

  /*##-1- calculate the destination transfer address  ############*/
  xPos = (stm32_getXSize() - JPEG_Info.ImageWidth) / 2;
  yPos = (stm32_getYSize() - JPEG_Info.ImageHeight) / 2;

  destination = (uint32_t)pDst + ((yPos * stm32_getXSize()) + xPos) * 4;

  HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 25);  /* wait for the previous DMA2D transfer to ends */
  /* copy the new decoded frame to the LCD Frame buffer*/
  HAL_DMA2D_Start(&DMA2D_Handle, (uint32_t)pSrc, destination, ImageWidth, ImageHeight);

}

void loop() {

  uint32_t isfirstFrame = 0 , startTime = 0;
  uint32_t jpegOutDataAdreess = JPEG_OUTPUT_DATA_BUFFER0;
  uint32_t FrameType = 0;

  /*##-3- Initialize the AVI Parser ##########################################*/
  if (AVI_ParserInit(&AVI_Handel, AVI_FILE_ADDRESS) == 0)
  {
    isfirstFrame = 1;
    FrameRate = 0;

    startTime = millis();

    do
    {
      /*##-4- Get a Frame from the AVI file ##################################*/
      FrameType = AVI_GetFrame(&AVI_Handel);

      if (FrameType == AVI_VIDEO_FRAME)
      {
        AVI_Handel.CurrentImage ++;
        printf("frame %d/%d\n", AVI_Handel.CurrentImage, AVI_Handel.aviInfo.TotalFrame);

        /*##-5- Start decoding the current JPEG frame with DMA (Not Blocking ) Method ################*/
        JPEG_Decode_DMA(&JPEG_Handle, (uint32_t) AVI_Handel.pVideoBuffer , AVI_Handel.FrameSize, jpegOutDataAdreess );

        /*##-6- Wait till end of JPEG decoding ###############################*/
        while (Jpeg_HWDecodingEnd == 0)
        {
        }

        if (isfirstFrame == 1)
        {
          isfirstFrame = 0;
          /*##-7- Get JPEG Info  #############################################*/
          HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);

          printf("ChromaSubsampling: %x\n", JPEG_Info.ChromaSubsampling);
          printf("w/h: %d : %d\n", JPEG_Info.ImageWidth, JPEG_Info.ImageHeight);
          printf("ImageQuality: %d\n", JPEG_Info.ImageQuality);
          printf("Colorspace: %d\n", JPEG_Info.ColorSpace);

          /*##-8- Initialize the DMA2D #######################################*/
          DMA2D_Init(JPEG_Info.ImageWidth, JPEG_Info.ImageHeight, JPEG_Info.ChromaSubsampling);
        }
        /*##-9- Copy the Decoded frame to the display frame buffer using the DMA2D #*/
        DMA2D_CopyBuffer((uint32_t *)jpegOutDataAdreess, (uint32_t *)getNextFrameBuffer(), JPEG_Info.ImageWidth, JPEG_Info.ImageHeight);

        jpegOutDataAdreess = (jpegOutDataAdreess == JPEG_OUTPUT_DATA_BUFFER0) ? JPEG_OUTPUT_DATA_BUFFER1 : JPEG_OUTPUT_DATA_BUFFER0;

#ifdef USE_FRAMERATE_REGULATION
        /* Regulate the frame rate to the video native frame rate by inserting delays */
        FrameRate =  (millis() - startTime) + 1;
        if (FrameRate < ((AVI_Handel.aviInfo.SecPerFrame / 1000) * AVI_Handel.CurrentImage))
        {
          delay(((AVI_Handel.aviInfo.SecPerFrame / 1000) * AVI_Handel.CurrentImage) - FrameRate);
        }
#endif /* USE_FRAMERATE_REGULATION */
      }
    } while (AVI_Handel.CurrentImage  <  AVI_Handel.aviInfo.TotalFrame);

    HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 50);  /* wait for the Last DMA2D transfer to ends */
  } else {
    printf("couldn't parse AVI header\n");
  }
}
