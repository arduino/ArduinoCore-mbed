/**
  ******************************************************************************
  * @file    cs42l52.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    06-May-2014
  * @brief   This file provides the CS42L52 Audio Codec driver.   
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cs42l52.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */ 

/** @addtogroup CS42L52
  * @brief     This file provides a set of functions needed to drive the 
  *            CS42L52 audio codec.
  * @{
  */

/** @defgroup CS42L52_Private_Types
  * @{
  */

/**
  * @}
  */ 
  
/** @defgroup CS42L52_Private_Defines
  * @{
  */
/* Uncomment this line to enable verifying data sent to codec after each write 
   operation (for debug purpose) */
#if !defined (VERIFY_WRITTENDATA)  
/* #define VERIFY_WRITTENDATA */
#endif /* VERIFY_WRITTENDATA */
/**
  * @}
  */ 

/** @defgroup CS42L52_Private_Macros
  * @{
  */

/**
  * @}
  */ 
  
/** @defgroup CS42L52_Private_Variables
  * @{
  */

/* Audio codec driver structure initialization */  
AUDIO_DrvTypeDef cs42l52_drv = 
{
  cs42l52_Init,
  cs42l52_ReadID,

  cs42l52_Play,
  cs42l52_Pause,
  cs42l52_Resume,
  cs42l52_Stop,  
  
  cs42l52_SetFrequency,  
  cs42l52_SetVolume,
  cs42l52_SetMute,  
  cs42l52_SetOutputMode,
  0,
};

static uint8_t Is_cs42l52_Stop = 1;

volatile uint8_t OutputDev = 0;

/**
  * @}
  */ 

/** @defgroup CS42L52_Function_Prototypes
  * @{
  */
static uint8_t CODEC_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
/**
  * @}
  */ 

/** @defgroup CS42L52_Private_Functions
  * @{
  */ 

/**
  * @brief Initializes the audio codec and the control interface.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param OutputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_Init(uint16_t DeviceAddr, uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  uint32_t counter = 0;
  
  /* Initialize the Control interface of the Audio Codec */
  AUDIO_IO_Init();     
    
  /* Keep Codec powered OFF */
  counter += CODEC_IO_Write(DeviceAddr, 0x02, 0x01);  
  
  /*Save Output device for mute ON/OFF procedure*/
  switch (OutputDevice)
  {
  case OUTPUT_DEVICE_SPEAKER:
    OutputDev = 0xFA;   /* SPK always ON & HP always OFF */
    break;
    
  case OUTPUT_DEVICE_HEADPHONE:
    OutputDev = 0xAF;   /* SPK always OFF & HP always ON */
    break;
    
  case OUTPUT_DEVICE_BOTH:
    OutputDev = 0xAA;   /* SPK always ON & HP always ON */
    break;
    
  case OUTPUT_DEVICE_AUTO:
    OutputDev = 0x05;   /* Detect the HP or the SPK automatically */
    break;    
    
  default:
    OutputDev = 0x05;   /* Detect the HP or the SPK automatically */
    break;    
  }
  
  counter += CODEC_IO_Write(DeviceAddr, 0x04, OutputDev);
  
  /* Clock configuration: Auto detection */  
  counter += CODEC_IO_Write(DeviceAddr, 0x05, 0x80);
  
  /* Set the Slave Mode and the audio Standard */  
  counter += CODEC_IO_Write(DeviceAddr, 0x06, CODEC_STANDARD);
  
  /* Interface Control 2: SCLK is Re-timed signal from MCLK*/
  counter +=CODEC_IO_Write(DeviceAddr, 0x07, 0x00); 
  
  /* ADCA and PGAA Select: no input selected*/
  counter +=CODEC_IO_Write(DeviceAddr, 0x08, 0x00);
  /* ADCB and PGAB Select: no input selected*/
  counter +=CODEC_IO_Write(DeviceAddr, 0x09, 0x00); 
  /*Play Back Control 1: headphone gain is 0.4, PCM not inverted, Master not mute*/
  counter +=CODEC_IO_Write(DeviceAddr, 0x0D, 0x10);/* CS42L52 has different config than CS42L52*/ 
  /* Miscellaneous Controls: Passthrough Analog & Passthrough Mute off, Soft Ramp on @0x0E*/
  counter +=CODEC_IO_Write(DeviceAddr, 0x0E, 0x02);  
  /* Play Back Control 2: Headphone Mute off, speaker mute off, mono enabled */
  counter +=CODEC_IO_Write(DeviceAddr, 0x0F, 0x32); 
  /* PCM A Volume: PCM Mute disabled, Volume is 0db(default) */
  counter +=CODEC_IO_Write(DeviceAddr, 0x1A, 0x00);
  /* PCM B Volume: PCM Mute disabled, Volume is 0db(default) */
  counter +=CODEC_IO_Write(DeviceAddr, 0x1B, 0x00); 
  /* Headphone A Volume: Headphone Volume is -6db */
  counter +=CODEC_IO_Write(DeviceAddr, 0x22, (uint8_t)(0-12));
  /* Headphone B Volume: Headphone Volume is -6db */
  counter +=CODEC_IO_Write(DeviceAddr, 0x23, (uint8_t)(0-12));    
  /* Speaker A Volume: Speaker Volume is 0db (default) */
  counter +=CODEC_IO_Write(DeviceAddr, 0x24, 0x00);
  /* Speaker B Volume: Speaker Volume is 0db (default) */
  counter +=CODEC_IO_Write(DeviceAddr, 0x25, 0x00);
  /* Charge Pump Frequency: 5 (default) */
  counter +=CODEC_IO_Write(DeviceAddr, 0x34, 5<<4);
  /* Power Control 1: power up */
  counter += CODEC_IO_Write(DeviceAddr, 0x02, 0x00); 

  /* Set the Master volume */
  counter += cs42l52_SetVolume(DeviceAddr, Volume);
  
  /* Return communication control value */
  return counter;  
}

/**
  * @brief  Get the CS42L52 ID.
  * @param DeviceAddr: Device address on communication Bus.   
  * @retval The CS42L52 ID 
  */
uint32_t cs42l52_ReadID(uint16_t DeviceAddr)
{
  /* Initialize the Control interface of the Audio Codec */
  AUDIO_IO_Init(); 
  
  return ((uint32_t)AUDIO_IO_Read(DeviceAddr, CS42L52_CHIPID_ADDR));
}

/**
  * @brief Start the audio Codec play feature.
  * @note For this codec no Play options are required.
  * @param DeviceAddr: Device address on communication Bus.   
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_Play(uint16_t DeviceAddr, uint16_t* pBuffer, uint16_t Size)
{
  uint32_t counter = 0;
  
  if(Is_cs42l52_Stop == 1)
  {
    /* Enable Output device */  
    counter += cs42l52_SetMute(DeviceAddr, AUDIO_MUTE_OFF);
    
    /* Power on the Codec */
    counter += CODEC_IO_Write(DeviceAddr, 0x02, 0x00); 

    Is_cs42l52_Stop = 0;
  }
  
  /* Return communication control value */
  return counter;  
}

/**
  * @brief Pauses playing on the audio codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_Pause(uint16_t DeviceAddr)
{  
  uint32_t counter = 0;

  /* Pause the audio file playing */
  /* Mute the output first */
  counter += cs42l52_SetMute(DeviceAddr, AUDIO_MUTE_ON);

  AUDIO_IO_Delay(20);
  
  /* Put the Codec in Power save mode */    
  counter += CODEC_IO_Write(DeviceAddr,0x02, 0x01);
  
  return counter;
}

/**
  * @brief Resumes playing on the audio codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_Resume(uint16_t DeviceAddr)
{
  uint32_t counter = 0;
  /* Resumes the audio file playing */  
  /* Unmute the output first */
  counter += cs42l52_SetMute(DeviceAddr, AUDIO_MUTE_OFF);

  AUDIO_IO_Delay(20);
  
  counter += CODEC_IO_Write(DeviceAddr,0x04, OutputDev);

  /* Exit the Power save mode */
  counter += CODEC_IO_Write(DeviceAddr,0x02, 0x9E); 
  
  return counter;
}

/**
  * @brief Stops audio Codec playing. It powers down the codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @param CodecPdwnMode: selects the  power down mode.
  *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from this
  *                           mode, the codec is set to default configuration 
  *                           (user should re-Initialize the codec in order to 
  *                            play again the audio stream).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_Stop(uint16_t DeviceAddr, uint32_t CodecPdwnMode)
{
  uint32_t counter = 0;
  
  /* Mute the output first */
  counter += cs42l52_SetMute(DeviceAddr, AUDIO_MUTE_ON);
  
  AUDIO_IO_Delay(20);

  /* Power down the DAC and the speaker (PMDAC and PMSPK bits)*/
  counter += CODEC_IO_Write(DeviceAddr, 0x02, 0x9F);
  
  Is_cs42l52_Stop = 1;
  return counter;    
}

/**
  * @brief Sets higher or lower the codec volume level.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param Volume: a byte value from 0 to 255 (refer to codec registers 
  *         description for more details).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_SetVolume(uint16_t DeviceAddr, uint8_t Volume)
{
  uint32_t counter = 0;
  uint8_t convertedvol = VOLUME_CONVERT(Volume);

  if(Volume > 0xE6)
  {
    /* Set the Master volume */
    counter += CODEC_IO_Write(DeviceAddr, 0x20, convertedvol - 0xE7); 
    counter += CODEC_IO_Write(DeviceAddr, 0x21, convertedvol - 0xE7);     
  }
  else
  {
    /* Set the Master volume */
    counter += CODEC_IO_Write(DeviceAddr, 0x20, convertedvol + 0x19); 
    counter += CODEC_IO_Write(DeviceAddr, 0x21, convertedvol + 0x19); 
  }

  return counter;
}

/**
  * @brief Sets new frequency.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_SetFrequency(uint16_t DeviceAddr, uint32_t AudioFreq)
{
  return 0;
}

/**
  * @brief Enables or disables the mute feature on the audio codec.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
  *             mute mode.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_SetMute(uint16_t DeviceAddr, uint32_t Cmd)
{
  uint32_t counter = 0;
  
  /* Set the Mute mode */
  if(Cmd == AUDIO_MUTE_ON)
  {
    counter += CODEC_IO_Write(DeviceAddr, 0x04, 0xFF);
    counter += CODEC_IO_Write(DeviceAddr, 0x0F, 0xF0);
  }
  else /* AUDIO_MUTE_OFF Disable the Mute */
  {
    counter += CODEC_IO_Write(DeviceAddr, 0x04, OutputDev);
    counter += CODEC_IO_Write(DeviceAddr, 0x0F, 0x02);
  }
  return counter;
}

/**
  * @brief Switch dynamically (while audio file is played) the output target 
  *         (speaker or headphone).
  * @note This function modifies a global variable of the audio codec driver: OutputDev.
  * @param DeviceAddr: Device address on communication Bus.
  * @param Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
  *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs42l52_SetOutputMode(uint16_t DeviceAddr, uint8_t Output)
{
  uint32_t counter = 0; 
  
  switch (Output)
  {
  case OUTPUT_DEVICE_SPEAKER:
    OutputDev = 0xFA;   /* SPK always ON & HP always OFF */
    break;
    
  case OUTPUT_DEVICE_HEADPHONE:
    OutputDev = 0xAF;   /* SPK always OFF & HP always ON */
    break;
    
  case OUTPUT_DEVICE_BOTH:
    OutputDev = 0xAA;   /* SPK always ON & HP always ON */
    break;
    
  case OUTPUT_DEVICE_AUTO:
    OutputDev = 0x05;   /* Detect the HP or the SPK automatically */
    break;    
    
  default:
    OutputDev = 0x05;   /* Detect the HP or the SPK automatically */
    break;    
  }
  
  counter += CODEC_IO_Write(DeviceAddr, 0x04, OutputDev);

  /* Return communication control value */
  return counter;
}

/**
  * @brief  Writes/Read a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Value: Data to be written
  * @retval None
  */
static uint8_t CODEC_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  uint32_t result = 0;
  
  AUDIO_IO_Write(Addr, Reg, Value);
  
#ifdef VERIFY_WRITTENDATA
  /* Verify that the data has been correctly written */  
  result = (AUDIO_IO_Read(Addr, Reg) == Value)? 0:1;
#endif /* VERIFY_WRITTENDATA */
  
  return result;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
