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

#ifndef __CEA861_H__
#define __CEA861_H__


struct tagAVI_INFOFRAME
{
    unsigned char PacketType; // 0xA0
    unsigned char Version;
    unsigned char Length;
    unsigned char Checksum;
    struct tagAVI_INFOFRAME_DB1
    {
        unsigned char S:2; // Scan Information
        unsigned char B:2; // Bar Data Present
        unsigned char A:1; // Active Format Information Present
        unsigned char Y:2; // RGB or YCBCR
            #define AVI_RGB         0
            #define AVI_YCBCR_422   1
            #define AVI_YCBCR_444   2
        unsigned char  :1; // Future Use
    }DB1;
    struct tagAVI_INFOFRAME_DB2
    {
        unsigned char R:4; // Active Portion Aspect Ratio
        unsigned char M:2; // Coded Frame Aspect Ratio
        unsigned char C:2; // Colorimetry
            #define AVI_C_DEFAULT   0
            #define AVI_C_ITU_601   1
            #define AVI_C_ITU_709   2
            #define AVI_C_EC_VALID  3
    }DB2;
    struct tagAVI_INFOFRAME_DB3
    {
        unsigned char SC    :2; // Non-Uniform Picture Scaling
        unsigned char Q     :2; // RGB Quantization Range
            #define AVI_Q_LIMITED_RANGE 1
            #define AVI_Q_FULL_RANGE    2
        unsigned char EC    :3; // Extended Colorimetry
        unsigned char ITC   :1; // IT content
    }DB3;
    struct tagAVI_INFOFRAME_DB4
    {
        unsigned char VIC   :7; // Video Identification Code (VIC)
        unsigned char       :1;
    }DB4;
    struct tagAVI_INFOFRAME_DB5
    {
        unsigned char PR    :4; // Pixel Repetition Factor
        unsigned char CN    :2; // IT Content Type
        unsigned char YQ    :2; // YCC Quantization Range
            #define AVI_YQ_LIMITED_RANGE    0
            #define AVI_YQ_FULL_RANGE       1
    }DB5;
    unsigned char rsvd1[8];
};

#endif  /* __CEA861_H__ */

