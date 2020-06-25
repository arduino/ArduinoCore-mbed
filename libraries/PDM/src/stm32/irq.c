#ifdef TARGET_STM

#include "audio.h"

extern SAI_HandleTypeDef haudio_out_sai;
extern SAI_HandleTypeDef haudio_in_sai;

void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
}

void AUDIO_OUT_SAIx_DMAx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
}

/**
  * @brief  This function handles BDMA Channel 1 for SAI_PDM interrupt request.
  * @param  None
  * @retval None
  */
void AUDIO_IN_SAI_PDMx_DMAx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(haudio_in_sai.hdmarx);
}

#endif