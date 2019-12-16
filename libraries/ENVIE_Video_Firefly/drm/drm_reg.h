//********************************************************************************
//*文件名   : auth.h
//*作用     :
//*创建时间 : 2015.02.01
//********************************************************************************
#ifndef __DRM_REG_H
#define __DRM_REG_H
//********************************************************************************
//********************************************************************************

//----------------Mail box source--------------
#define Down_HASH1              0x50		/*_at_ 0x1100 + 0x00~0x0F*/ //84:50~5F, 16Bytes

#define DRM_COMMAND             0xEC		/*_at_ 0x1100 + 0x10~0x11*/ //84:EC~ED, 2Bytes
#define KSV_Block_Received      (0x001)
#define Hash_Ready              (0x002)
#define Get_Auth_Status         (0x004)
#define Status_Read             (0x008)
#define Try_Auth                (0x010)
#define Start_M2_Auth           (0x020)
#define Start_Checksum          (0x040)
#define Get_KSVs                (0x080)
#define M2_Auth_Okay            (0x100)
#define Auth_Status_Read        (0x200)

#define Enc_Level               0xEE		/*_at_ 0x1100 + 0x12*/ //84:EE
#define Encrypt_Level_Required  (0x03)
#define KSV_Check               (0x0C)
#define No_Checked           (0x00)
#define KSV_Okay             (0x01)
#define KSV_Fail             (0x02)
#define Type                    (0x10)
#define Try_Enc_Level           (0x60)
#define No_Encrypt           (0x00)
#define HDCP_1_X             (0x01)
#define HDCP_2_2             (0x02)

#define Down_HASH2              0xEF		/*_at_ 0x1100 + 0x13~0x23*/ //84:EF~FE, 16Bytes, 0x1100 + 0x22 cannot be used

#define Interrupt               0xFF		/*_at_ 0x1100 + 0x24*/ //84:FF
#define INTR                    (0x01)

//----------------Mail box sink----------------
#define OUI                     0x80		/*_at_ 0x1080 + 0x00~0x02*/ //84:80~82, 3Bytes

#define Chip_ID                 0x83		/*_at_ 0x1080 + 0x03~0x04*/ //84:83~84, 2Bytes

#define Hard_Rev                0x85		/*_at_ 0x1080 + 0x05*/ //84:85
#define DRM_HW_VERSION          (0xAB)
#define Soft_Rev                0x86		/*_at_ 0x1080 + 0x06*/ //84:86
#define DRM_FW_VERSION          (0x03)

#define STATUS                  0x87		/*_at_ 0x1080 + 0x07*/ //84:87
#define Auth_Status_Changed     (0x01)
#define Up_HASH_Ready           (0x02)
#define MI_2_Auth_Done          (0x04)
#define KSV_Ready               (0x08)
#define KSV_Buffer_Ready        (0x10)
#define Request_Checksum        (0x20)

#define Reserved1               0x88		/*_at_ 0x1080 + 0x08~0x09*/ //84:88~89, 2Bytes, 84:89 cannot be used;

#define Auth_Status             0x8A		/*_at_ 0x1080 + 0x0A~0x0B*/ //84:8A~8B, 2Bytes
#define Authentication_Status   (0x007)
#define Packet_Inactivity_Timer (0x008)
#define Unstable_HS_Link_Timer  (0x010)
#define MIN_ENC_LVL             (0x020)
#define Encrypton_Status        (0x040)
#define Repeater_Mode           (0x080)
#define Lock                    (0x100)
#define Enc_Level_Error         (0x200)
#define MI_2_Auth_Status        (0xC00)

#define Reserved2               0x8C		/*_at_ 0x1080 + 0x0C~0x0F*/ //84:8C~8F, 4Bytes

#define Up_HASH                 0x90		/*_at_ 0x1080 + 0x10~0x2F*/ //84:90~AF, 32Bytes

#define Num_KSVs                0xB0		/*_at_ 0x1080 + 0x30*/ //84:B0
#define Total_RXIDs             (0x3F)
#define Last_KSV_Block          (0x80)

#define KSV_Buffer              0xB1		/*_at_ 0x1080 + 0x31~0x3F*/ //84:B1~BF, 15Bytes

#define Debug                   0xC0		/*_at_ 0x1080 + 0x40~0x5F*/ //84:C0~DF, 32Bytes

#define Reserved3               0xE0		/*_at_ 0x1080 + 0x60~0x69*/ //84:E0~E9, 10Bytes

#endif /* _DRM_REG_H */
//********************************************************************************
//********************************************************************************
