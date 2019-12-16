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

#include "config.h"
#include "EFM8UB2_helper.h"
#include "debug.h"


// TODO: When MI-2 EVB is back, measure GPIO waveform to make sure this time is accurate.
/* delay 1 milisecond */
void  delay_ms(unsigned int  ms)
{
    int  i;
    for (i=0; i<ms; i++)
    {
        DELAY_US(1000);
    }
}


/* return the logarithm of N, base 2 */
/* if 2^b <= N < 2^(b+1), consider the logarithm of N is b */
char  log2_N(unsigned char N)
{
    unsigned char  i;
    unsigned char  j;

    if (N == 0)
    {
        TRACE("ERROR! N is 0 in log2_N()\n");
        return -1;  // indicates an error
    }

    for (i=0,j=1; ; i++,j++)
    {
        if ( (N >= (1<<i)) && (N < (1<<j)) )
        {
            return i;
        }
    }
}



