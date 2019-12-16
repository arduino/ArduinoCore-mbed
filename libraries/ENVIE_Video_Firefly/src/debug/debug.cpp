/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V0.1

Filename : 

Project  : ANX7625 

Created  : 20 Sept. 2016

Devices  : ANX7625

Toolchain: Keil
 
Description:

Revision History:

******************************************************************************/

#include "debug.h"

#define DEBUG

#ifdef DEBUG
/**
 * Trace an array output for debugging
 *
 * \param[in] array The array name or address to trace output
 * \param[in] len The number of bytes to trace output
 *
 * \note The array size must be no more than 256. If the size is 256 set len to 0.
 */
void TraceArray(unsigned char array[], unsigned char len)
{
  unsigned char i;
  
  i = 0;
  while(1)
  {
    TRACE1("%02BX", array[i]);
    i++;
    
    if (i != len)
    {
      if (i%16 == 0)
      {
        TRACE("\n");
      }
      else
      {
        TRACE(" ");
      }
    }
    else
    {
      TRACE("\n");
      break;
    }
  }
}

#else


#endif

