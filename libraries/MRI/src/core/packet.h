/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* 'Class' to manage the sending and receiving of packets to/from gdb.  Takes care of crc and ack/nak handling too. */
#ifndef PACKET_H_
#define PACKET_H_

#include <stdio.h>
#include <core/buffer.h>

typedef struct
{
    /* This is the complete buffer with room for '$', '#', and 2-byte checksum. */
    Buffer         packetBuffer;
    /* This is a subset of pPacketBuffer, after room has been made for '$', '#', and 2-byte checksum. */
    Buffer         dataBuffer;
    char           lastChar;
    unsigned char  calculatedChecksum;
    unsigned char  expectedChecksum;
} Packet;

/* Real name of functions are in mri namespace. */
void    mriPacket_Init(Packet* pPacket, char* pBufferStart, size_t bufferSize);
void    mriPacket_GetFromGDB(Packet* pPacket);
void    mriPacket_SendToGDB(Packet* pPacket);

/* Macroes which allow code to drop the mri namespace prefix. */
#define Packet_Init         mriPacket_Init
#define Packet_GetFromGDB   mriPacket_GetFromGDB
#define Packet_SendToGDB    mriPacket_SendToGDB


#endif /* PACKET_H_ */
