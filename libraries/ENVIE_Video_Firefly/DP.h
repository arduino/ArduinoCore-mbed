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

#ifndef __DP_H__
#define __DP_H__

/**
  * @brief SDP Header
  */
struct tagSDPHeader
{
  unsigned char ID;
  unsigned char Type;
    #define SDP_TYPE_DP_RSVD                0
    #define SDP_TYPE_AUDIO_TIMESTAMP        1
    #define SDP_TYPE_AUDIO_STREAM           2
    #define SDP_TYPE_DP_RSVD1               3
    #define SDP_TYPE_EXTENSION              4
    #define SDP_TYPE_AUDIO_COPYMANAGEMENT   5
    #define SDP_TYPE_ISRC                   6
    #define SDP_TYPE_VSC                    7
  unsigned char RevisionNumber;
    #define _3D_STEREO_ONLY     1
    #define _3D_STEREO_PSR      2
    #define _3D_STEREO_PSR2     3
    #define PSR2_EXT            4
  unsigned char ValidDataNumber;
};

#endif  /* __DP_H__ */

