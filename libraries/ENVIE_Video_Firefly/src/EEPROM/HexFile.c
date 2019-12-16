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

#include  <stdio.h>
#include  "HexFile.h"

char GetLineData(unsigned char *pLine, unsigned char *pByteCount, unsigned int *pAddress, unsigned char *pRecordType, unsigned char *pData)
{
    char  start_code;
    unsigned char checksum;
    unsigned char sum;
    unsigned char i;

    if (sscanf(pLine, "%c%2bx%4x%2bx", &start_code, pByteCount, pAddress, pRecordType) == 4)
    {
        sum = *pByteCount + (*pAddress >> 8) + (*pAddress & 0xFF) + *pRecordType;
        pLine += (1 + 2 + 4 + 2); /* Start code: 1 character, Byte count: 2 characters, Address: 4 characters, Record type: 2 characters */

        if (start_code != ':')
        {
            return -1;  // bad start code
        }

        if ( (*pRecordType != HEX_RECORD_TYPE_DATA) && (*pRecordType != HEX_RECORD_TYPE_EOF) )  // only I8HEX files are supported; refer to https://en.wikipedia.org/wiki/Intel_HEX
        {
            return -2;  // unsupported record type
        }

        if ( *pRecordType == HEX_RECORD_TYPE_DATA )  // all other record types are filtered out
        {
            for (i=0; i<*pByteCount; i++)
            {
                if (sscanf(pLine, "%2bx", pData) == 1)
                {
                    sum += *pData;
                    pData++;
                    pLine += 2;
                }
                else
                {
                    return -3;  // read data error
                }
            }

            if (sscanf(pLine, "%2bx", &checksum) == 1)
            {
                if ((char)(sum + checksum) == 0)
                {
                    return 0;
                }
                else
                {
                    return -4;  // checksum error
                }
            }
            else
            {
                return -5;   // read checksum error
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -6;    // parse record error
    }
}


void SetLineData(unsigned char *pLine, unsigned char ByteCount, unsigned int Address, unsigned char RecordType, unsigned char *pData)
{
    unsigned char checksum;

    sprintf(pLine, ":%02BX%04X%02BX", ByteCount, Address, RecordType);
    pLine += (1 + 2 + 4 + 2); /* Start code: 1 character, Byte count: 2 characters, Address: 4 characters, Record type: 2 characters */
    checksum = ByteCount + (Address >> 8) + (Address & 0xFF) + RecordType;

    for(; ByteCount;  ByteCount--)
    {
        sprintf(pLine, "%02BX", *pData);
        checksum += *pData;
        pData++;
        pLine += 2;
    }

    sprintf(pLine, "%02BX", -checksum);
    pLine += 2;
    *pLine = '\0';
}

