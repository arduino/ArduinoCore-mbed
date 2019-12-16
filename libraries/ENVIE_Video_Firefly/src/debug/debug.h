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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#ifdef DEBUG
// Debug mode - debug functions cause printf output

#define TRACE(format)   \
        printf(format)
#define TRACE0(format)  \
        printf(format)
#define TRACE1(format, arg1)    \
        printf(format, arg1)
#define TRACE2(format, arg1, arg2)  \
        printf(format, arg1, arg2)
#define TRACE3(format, arg1, arg2, arg3)    \
        printf(format, arg1, arg2, arg3)
#define TRACE4(format, arg1, arg2, arg3, arg4)  \
        printf(format, arg1, arg2, arg3, arg4)
#define TRACE5(format, arg1, arg2, arg3, arg4, arg5)    \
        printf(format, arg1, arg2, arg3, arg4, arg5)
#define TRACE6(format, arg1, arg2, arg3, arg4, arg5, arg6)  \
        printf(format, arg1, arg2, arg3, arg4, arg5, arg6)
#define TRACE7(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7)    \
        printf(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define TRACE8(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)  \
        printf(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#define TRACE9(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)    \
        printf(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)


void TraceArray(unsigned char *array, unsigned char len);
#define TRACE_ARRAY(array, len) TraceArray(array, len)


#define ASSERT(expr)  \
  if(expr)            \
  {                   \
  }                   \
  else                \
  {                   \
    TRACE2("Assert failed: " #expr " (file %s line %d)\n", __FILE__, (int) __LINE__ );\
    while (1);        \
  }

#else

// Release mode - debug functions expand to nothing!
#define TRACE(format) 
#define TRACE0(format)
#define TRACE1(format, arg1)
#define TRACE2(format, arg1, arg2)
#define TRACE3(format, arg1, arg2, arg3)
#define TRACE4(format, arg1, arg2, arg3, arg4)
#define TRACE5(format, arg1, arg2, arg3, arg4, arg5)
#define TRACE6(format, arg1, arg2, arg3, arg4, arg5, arg6)
#define TRACE7(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define TRACE8(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#define TRACE9(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)

#define TRACE_ARRAY(array, len)
    
#endif

#endif  /* __DEBUG_H__ */

