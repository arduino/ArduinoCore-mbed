#include "audio.h"

#define AUDIO_FREQUENCY            BSP_AUDIO_FREQUENCY_16K
#define AUDIO_IN_PDM_BUFFER_SIZE  (uint32_t)(128*AUDIO_FREQUENCY/16000*DEFAULT_AUDIO_IN_CHANNEL_NBR)

/* Size of the recorder buffer */
#define RECORD_BUFFER_SIZE        4096

//ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".OPEN_AMP_SHMEM")));
// FIXME: need to add an entry for RAM_D3 to linker script
uint16_t* recordPDMBuf = (uint16_t*)0x38000000;
uint16_t playbackBuf[RECORD_BUFFER_SIZE * 2];

/* Pointer to record_data */
uint32_t playbackPtr;

void setup() {
  // put your setup code here, to run once:
  /* Set audio input interface */
  Serial.begin(115200);

  BSP_AUDIO_IN_SelectInterface(AUDIO_IN_INTERFACE_PDM);

  /* Initialize audio IN at REC_FREQ*/
  if (BSP_AUDIO_IN_InitEx(INPUT_DEVICE_DIGITAL_MIC, AUDIO_FREQUENCY, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR) != AUDIO_OK)
  {
    /* Record Error */
    Serial.println("BSP_AUDIO_IN_InitEx error");
    //Error_Handler();
  }

#if 0
  /* Initialize audio OUT at REC_FREQ*/
  if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 80, AUDIO_FREQUENCY) != AUDIO_OK)
  {
    /* Record Error */
    //Error_Handler();
  }

  /* Set audio slot */
  BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
#endif

  /* Start the record */
  BSP_AUDIO_IN_Record((uint16_t*)recordPDMBuf, AUDIO_IN_PDM_BUFFER_SIZE);

  /* Start audio output */
  //BSP_AUDIO_OUT_Play((uint16_t*)playbackBuf, RECORD_BUFFER_SIZE * 2);

  Serial.println("init done ");
}

bool done = false;

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(playbackPtr);
  if (playbackPtr == 0 && !done) {
    Serial.write((uint8_t*)playbackBuf, RECORD_BUFFER_SIZE * 2);
    done = true;
  }
  if (playbackPtr != 0) {
    done = false;
  }
}

extern "C" {
  /**
      @brief Calculates the remaining file size and new position of the pointer.
      @param  None
      @retval None
  */
  void BSP_AUDIO_IN_TransferComplete_CallBack(void)
  {
    if (BSP_AUDIO_IN_GetInterface() == AUDIO_IN_INTERFACE_PDM)
    {
      /* Invalidate Data Cache to get the updated content of the SRAM*/
      SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE / 2], AUDIO_IN_PDM_BUFFER_SIZE * 2);

      BSP_AUDIO_IN_PDMToPCM((uint16_t*)&recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE / 2], &playbackBuf[playbackPtr]);

      /* Clean Data Cache to update the content of the SRAM */
      SCB_CleanDCache_by_Addr((uint32_t*)&playbackBuf[playbackPtr], AUDIO_IN_PDM_BUFFER_SIZE / 4);

      playbackPtr += AUDIO_IN_PDM_BUFFER_SIZE / 4 / 2;
      if (playbackPtr >= RECORD_BUFFER_SIZE)
      {
        playbackPtr = 0;
      }
    }
  }

  /**
      @brief  Manages the DMA Half Transfer complete interrupt.
      @param  None
      @retval None
  */
  void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
  {
    if (BSP_AUDIO_IN_GetInterface() == AUDIO_IN_INTERFACE_PDM)
    {
      /* Invalidate Data Cache to get the updated content of the SRAM*/
      SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuf[0], AUDIO_IN_PDM_BUFFER_SIZE * 2);

      BSP_AUDIO_IN_PDMToPCM((uint16_t*)&recordPDMBuf[0], &playbackBuf[playbackPtr]);

      /* Clean Data Cache to update the content of the SRAM */
      SCB_CleanDCache_by_Addr((uint32_t*)&playbackBuf[playbackPtr], AUDIO_IN_PDM_BUFFER_SIZE / 4);

      playbackPtr += AUDIO_IN_PDM_BUFFER_SIZE / 4 / 2;
      if (playbackPtr >= RECORD_BUFFER_SIZE)
      {
        playbackPtr = 0;
      }
    }
  }

}
