#include "Arduino.h"

#if defined(ARDUINO_GIGA)
#undef HSE_VALUE
#define HSE_VALUE 16000000
#endif

extern "C" uint32_t HAL_RCC_GetSysClockFreq(void)
{
  uint32_t pllp, pllsource, pllm, pllfracen, hsivalue;
  float_t fracn1, pllvco;
  uint32_t sysclockfreq;

  /* Get SYSCLK source -------------------------------------------------------*/

  switch (RCC->CFGR & RCC_CFGR_SWS)
  {
  case RCC_CFGR_SWS_HSI:  /* HSI used as system clock source */

   if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIDIV) != 0U)
      {
        sysclockfreq = (uint32_t) (HSI_VALUE >> (__HAL_RCC_GET_HSI_DIVIDER()>> 3));
      }
      else
      {
        sysclockfreq = (uint32_t) HSI_VALUE;
      }

    break;

  case RCC_CFGR_SWS_CSI:  /* CSI used as system clock  source */
    sysclockfreq = CSI_VALUE;
    break;

  case RCC_CFGR_SWS_HSE:  /* HSE used as system clock  source */
    sysclockfreq = HSE_VALUE;
    break;

  case RCC_CFGR_SWS_PLL1:  /* PLL1 used as system clock  source */

    /* PLL_VCO = (HSE_VALUE or HSI_VALUE or CSI_VALUE/ PLLM) * PLLN
    SYSCLK = PLL_VCO / PLLR
    */
    pllsource = (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC);
    pllm = ((RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1)>> 4)  ;
    pllfracen = ((RCC-> PLLCFGR & RCC_PLLCFGR_PLL1FRACEN)>>RCC_PLLCFGR_PLL1FRACEN_Pos);
    fracn1 = (float_t)(uint32_t)(pllfracen* ((RCC->PLL1FRACR & RCC_PLL1FRACR_FRACN1)>> 3));

    if (pllm != 0U)
    {
      switch (pllsource)
      {
      case RCC_PLLSOURCE_HSI:  /* HSI used as PLL clock source */

       if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIDIV) != 0U)
        {
          hsivalue= (HSI_VALUE >> (__HAL_RCC_GET_HSI_DIVIDER()>> 3));
          pllvco = ( (float_t)hsivalue / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
        }
        else
        {
          pllvco = ((float_t)HSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
        }
        break;

      case RCC_PLLSOURCE_CSI:  /* CSI used as PLL clock source */
        pllvco = ((float_t)CSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
        break;

      case RCC_PLLSOURCE_HSE:  /* HSE used as PLL clock source */
        pllvco = ((float_t)HSE_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
        break;

      default:
        pllvco = ((float_t)CSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
        break;
      }
      pllp = (((RCC->PLL1DIVR & RCC_PLL1DIVR_P1) >>9) + 1U ) ;
      sysclockfreq =  (uint32_t)(float_t)(pllvco/(float_t)pllp);
    }
    else
    {
      sysclockfreq = 0U;
    }
    break;

  default:
    sysclockfreq = CSI_VALUE;
    break;
  }

  return sysclockfreq;
}