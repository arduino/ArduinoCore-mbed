/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_jpeg.h"
#include "stm32h7xx_hal_dma2d.h"

#define MDMA_INSTANCE MDMA_Channel0

#define LCD_FRAME_BUFFER            0xC0000000
#define JPEG_OUTPUT_DATA_BUFFER0    0xC0400000
#define JPEG_OUTPUT_DATA_BUFFER1    0xC0600000

/*uncomment this line to regulate the decoding frame rate to the native video frame rate */
//#define USE_FRAMERATE_REGULATION

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
