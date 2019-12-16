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

#ifndef __DEFS_H__
#define __DEFS_H__

#define _BIT0	0x01
#define _BIT1	0x02
#define _BIT2	0x04
#define _BIT3	0x08
#define _BIT4	0x10
#define _BIT5	0x20
#define _BIT6	0x40
#define _BIT7	0x80

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ABS(x, y) ((x) > (y) ? ((x)- (y)) : ((y)-(x)))

////////////////////////////////////////////////////////////////////////////////
// byte extraction macros
#define LOBYTE(x)   ((unsigned char)(x))
#define HIBYTE(x)   ((unsigned char)((x) >> 8))

// byte access macros
// generic pointers
#define BYTE0(x)     ((unsigned char *)&x)[0]
#define BYTE1(x)     ((unsigned char *)&x)[1]
#define BYTE2(x)     ((unsigned char *)&x)[2]
#define BYTE3(x)     ((unsigned char *)&x)[3]
// data pointer
#define DBYTE0(x)    ((unsigned char data *)&x)[0]
#define DBYTE1(x)    ((unsigned char data *)&x)[1]
#define DBYTE2(x)    ((unsigned char data *)&x)[2]
#define DBYTE3(x)    ((unsigned char data *)&x)[3]
// idata pointer
#define IBYTE0(x)    ((unsigned char idata *)&x)[0]
#define IBYTE1(x)    ((unsigned char idata *)&x)[1]
#define IBYTE2(x)    ((unsigned char idata *)&x)[2]
#define IBYTE3(x)    ((unsigned char idata *)&x)[3]
// xdata pointer
#define XBYTE0(x)    ((unsigned char xdata *)&x)[0]
#define XBYTE1(x)    ((unsigned char xdata *)&x)[1]
#define XBYTE2(x)    ((unsigned char xdata *)&x)[2]
#define XBYTE3(x)    ((unsigned char xdata *)&x)[3]

////////////////////////////////////////////////////////////////////////////////
// bitwise operation macros
#define BIT_IS_0(x, bitindex)       (((x) & 1<<bitindex) == 0)
#define BIT_IS_1(x, bitindex)       (((x) & 1<<bitindex) != 0)
#define BIT(x, bitindex)            ((bit)((x) & 1<<bitindex))
#define BITSET(x, bitindex)         ((x) |= 1<<bitindex)
#define BITCLEAR(x, bitindex)       ((x) &= ~(1<<bitindex))
// Extracts bits: from x, starting at bit bitindex, extracts bitlen bits; bit bitindex is the extracted LSB.
#define BITS_EXTRACT(x, bitindex, bitlen) ((x)>>(bitindex) & ((1<<bitlen) - 1))
#define BITMASK(bitindex)               (1 << bitindex)
#define BITSMASK(bitindex, bitlen)      (((1<<bitlen) - 1) << bitindex)
// in x, set bits [bitindex+bitlen-1 : bitindex] to value y; y <= ((1<<bitlen) - 1), otherwise data corruption!!
#define BITS_SET(x, bitindex, bitlen, y) ((x) = (x) & ~BITSMASK(bitindex, bitlen) | (y)<<bitindex)

#define UNUSED_VAR(x) ((x) = (x))

#endif  /* __DEFS_H__ */

