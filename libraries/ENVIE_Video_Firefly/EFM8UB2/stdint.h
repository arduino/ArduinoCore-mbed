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

#ifndef __STDINT_H__
#define __STDINT_H__

#if defined __C51__

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef uint32_t       uint64_t[2];

typedef signed char    int8_t;
typedef short          int16_t;
typedef long           int32_t;
typedef int32_t        int64_t[2];

#endif

#endif  /* __STDINT_H__ */

