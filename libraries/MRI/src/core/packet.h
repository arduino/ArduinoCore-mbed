/* Copyright 2014 Adam Green (https://github.com/adamgreen/)

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
    Buffer*        pBuffer;
    char           lastChar;
    unsigned char  calculatedChecksum;
    unsigned char  expectedChecksum;
} Packet;

/* Real name of functions are in mri namespace. */
void    mriPacket_GetFromGDB(Packet* pPacket, Buffer* pBuffer);
void    mriPacket_SendToGDB(Packet* pPacket, Buffer* pBuffer);

/* Macroes which allow code to drop the mri namespace prefix. */
#define Packet_Init         mriPacket_Init
#define Packet_GetFromGDB   mriPacket_GetFromGDB
#define Packet_SendToGDB    mriPacket_SendToGDB


#endif /* PACKET_H_ */
