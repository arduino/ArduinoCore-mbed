#include "audio.h"

#define AUDIO_FREQUENCY            BSP_AUDIO_FREQUENCY_16K
#define AUDIO_IN_PDM_BUFFER_SIZE  (uint32_t)(128*AUDIO_FREQUENCY/16000*DEFAULT_AUDIO_IN_CHANNEL_NBR)

/* Size of the recorder buffer */
#define RECORD_BUFFER_SIZE        4096

ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D2")));

uint16_t playbackBuf[RECORD_BUFFER_SIZE*2];

/* Pointer to record_data */
uint32_t playbackPtr;

void setup() {
  // put your setup code here, to run once:
  /* Set audio input interface */
  BSP_AUDIO_IN_SelectInterface(AUDIO_IN_INTERFACE_PDM);

  /* Initialize audio IN at REC_FREQ*/
  if (BSP_AUDIO_IN_InitEx(INPUT_DEVICE_DIGITAL_MIC, AUDIO_FREQUENCY, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR) != AUDIO_OK)
  {
    /* Record Error */
    //Error_Handler();
  }

  /* Initialize audio OUT at REC_FREQ*/
  if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 80, AUDIO_FREQUENCY) != AUDIO_OK)
  {
    /* Record Error */
    //Error_Handler();
  }

  /* Set audio slot */
  BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);

  /* Start the record */
  BSP_AUDIO_IN_Record((uint16_t*)recordPDMBuf, AUDIO_IN_PDM_BUFFER_SIZE);

  /* Start audio output */
  BSP_AUDIO_OUT_Play((uint16_t*)playbackBuf, RECORD_BUFFER_SIZE * 2);
}

void loop() {
  // put your main code here, to run repeatedly:
}
