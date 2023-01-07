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
#include <stdint.h>
#include <core/core.h>
#include <core/hex_convert.h>
#include <core/platforms.h>
#include <core/packet.h>


void Packet_Init(Packet* pPacket, char* pBufferStart, size_t bufferSize)
{
    Buffer_Init(&pPacket->dataBuffer, pBufferStart+1, bufferSize-4);
    Buffer_Init(&pPacket->packetBuffer, pBufferStart, bufferSize);
}


static void initPacketStructure(Packet* pPacket);
static void getMostRecentPacket(Packet* pPacket);
static void getPacketDataAndExpectedChecksum(Packet* pPacket);
static void waitForStartOfNextPacket(Packet* pPacket);
static char getNextCharFromGdb(Packet* pPacket);
static int  getPacketData(Packet* pPacket);
static void clearChecksum(Packet* pPacket);
static void updateChecksum(Packet* pPacket, char nextChar);
static int  isEscapePrefixChar(char charToCheck);
static char unescapeChar(char charToUnescape);
static void extractExpectedChecksum(Packet* pPacket);
static int  isChecksumValid(Packet* pPacket);
static void sendACKToGDB(void);
static void sendNAKToGDB(void);
static void resetBufferToEnableFutureReadingOfValidPacketData(Packet* pPacket);
void Packet_GetFromGDB(Packet* pPacket)
{
    initPacketStructure(pPacket);
    do
    {
        getMostRecentPacket(pPacket);
    } while(!isChecksumValid(pPacket));

    resetBufferToEnableFutureReadingOfValidPacketData(pPacket);
}

static void initPacketStructure(Packet* pPacket)
{
    pPacket->lastChar = '\0';
    pPacket->calculatedChecksum = 0;
    pPacket->expectedChecksum = 0;
    Buffer_Reset(&pPacket->packetBuffer);
}

static void getMostRecentPacket(Packet* pPacket)
{
    do
    {
        getPacketDataAndExpectedChecksum(pPacket);
    } while (Platform_CommHasReceiveData());

    if (!isChecksumValid(pPacket))
    {
        sendNAKToGDB();
        return;
    }

    sendACKToGDB();
}

static void getPacketDataAndExpectedChecksum(Packet* pPacket)
{
    int completePacket;

    do
    {
        waitForStartOfNextPacket(pPacket);
        completePacket = getPacketData(pPacket);
    } while (!completePacket);

    extractExpectedChecksum(pPacket);
}

static void waitForStartOfNextPacket(Packet* pPacket)
{
    char nextChar = pPacket->lastChar;

    /* Wait for the packet start character, '$', and ignore all other characters. */
    while (nextChar != '$')
        nextChar = getNextCharFromGdb(pPacket);
}

static char getNextCharFromGdb(Packet* pPacket)
{
    char nextChar = Platform_CommReceiveChar();
    pPacket->lastChar = nextChar;
    return nextChar;
}

static int getPacketData(Packet* pPacket)
{
    char nextChar;

    Buffer_Reset(&pPacket->dataBuffer);
    clearChecksum(pPacket);
    nextChar = getNextCharFromGdb(pPacket);
    while (Buffer_BytesLeft(&pPacket->dataBuffer) > 0 && nextChar != '$' && nextChar != '#')
    {
        updateChecksum(pPacket, nextChar);
        if (isEscapePrefixChar(nextChar))
        {
            nextChar = getNextCharFromGdb(pPacket);
            if (nextChar == '$' || nextChar == '#')
                break;
            updateChecksum(pPacket, nextChar);
            nextChar = unescapeChar(nextChar);
        }
        Buffer_WriteChar(&pPacket->dataBuffer, nextChar);
        nextChar = getNextCharFromGdb(pPacket);
    }

    /* Return success if the expected end of packet character, '#', was received. */
    return (nextChar == '#');
}

static void clearChecksum(Packet* pPacket)
{
    pPacket->calculatedChecksum = 0;
}

static void updateChecksum(Packet* pPacket, char nextChar)
{
    pPacket->calculatedChecksum += (unsigned char)nextChar;
}

static int isEscapePrefixChar(char charToCheck)
{
    return charToCheck == '}';
}

static char unescapeChar(char charToUnescape)
{
    return charToUnescape ^ 0x20;
}

static void extractExpectedChecksum(Packet* pPacket)
{
    __try
    {
        char char1 = getNextCharFromGdb(pPacket);
        char char2 = getNextCharFromGdb(pPacket);
        unsigned char expectedChecksumHiNibble;
        unsigned char expectedChecksumLoNibble;

        __throwing_func( expectedChecksumHiNibble = HexCharToNibble(char1) );
        __throwing_func( expectedChecksumLoNibble = HexCharToNibble(char2) );
        pPacket->expectedChecksum = (expectedChecksumHiNibble << 4) | expectedChecksumLoNibble;
    }
    __catch
    {
        /* Force the checksum to mismatch if invalid hex digit was encountered. */
        pPacket->expectedChecksum = ~pPacket->calculatedChecksum;
    }
}

static int isChecksumValid(Packet* pPacket)
{
    return (pPacket->expectedChecksum == pPacket->calculatedChecksum);
}

static void sendACKToGDB(void)
{
    Platform_CommSendChar('+');
}

static void sendNAKToGDB(void)
{
    Platform_CommSendChar('-');
}

static void resetBufferToEnableFutureReadingOfValidPacketData(Packet* pPacket)
{
    Buffer_SetEndOfBuffer(&pPacket->dataBuffer);
    Buffer_Reset(&pPacket->dataBuffer);
}


static void completePacket(Packet* pPacket);
static void storePacketHeaderByte(Packet* pPacket);
static void processPacketData(Packet* pPacket);
static void storePacketChecksum(Packet* pPacket);
static void sendPacket(Packet* pPacket);
static int  receiveCharAfterSkippingControlC(Packet* pPacket);
void Packet_SendToGDB(Packet* pPacket)
{
    char  charFromGdb;

    /* Keeps looping until GDB sends back the '+' packet acknowledge character.  If GDB sends a '$' then it is trying
       to send a packet so cancel this send attempt. */
    initPacketStructure(pPacket);
    completePacket(pPacket);
    do
    {
        sendPacket(pPacket);
        charFromGdb = receiveCharAfterSkippingControlC(pPacket);
    } while (charFromGdb != '+' && charFromGdb != '$');
}

static void completePacket(Packet* pPacket)
{
    /* Complete packet by adding '$' header and '#' checksum terminator -> "$<DataInHex>#<2HexDigitsOfChecksum>" */
    Buffer_Reset(&pPacket->dataBuffer);
    clearChecksum(pPacket);

    storePacketHeaderByte(pPacket);
    processPacketData(pPacket);
    storePacketChecksum(pPacket);

    Buffer_SetEndOfBuffer(&pPacket->packetBuffer);
}

static void storePacketHeaderByte(Packet* pPacket)
{
    Buffer_WriteChar(&pPacket->packetBuffer, '$');
}

static void processPacketData(Packet* pPacket)
{
    size_t length = 0;
    while (Buffer_BytesLeft(&pPacket->dataBuffer) > 0)
    {
        char currChar = Buffer_ReadChar(&pPacket->dataBuffer);
        updateChecksum(pPacket, currChar);
        length++;
    }
    Buffer_Advance(&pPacket->packetBuffer, length);
}

static void storePacketChecksum(Packet* pPacket)
{
    Buffer_WriteChar(&pPacket->packetBuffer, '#');
    Buffer_WriteByteAsHex(&pPacket->packetBuffer, pPacket->calculatedChecksum);
}

static void sendPacket(Packet* pPacket)
{
    Buffer_Reset(&pPacket->packetBuffer);
    Platform_CommSendBuffer(&pPacket->packetBuffer);
}

static int receiveCharAfterSkippingControlC(Packet* pPacket)
{
    static const int controlC = 0x03;
    int              nextChar;

    nextChar = getNextCharFromGdb(pPacket);
    while (nextChar == controlC)
    {
        ControlCEncountered();
        nextChar = getNextCharFromGdb(pPacket);
    }

    return nextChar;
}


__attribute__((weak)) void Platform_CommSendBuffer(Buffer* pBuffer)
{
    while (Buffer_BytesLeft(pBuffer))
        Platform_CommSendChar(Buffer_ReadChar(pBuffer));
}
