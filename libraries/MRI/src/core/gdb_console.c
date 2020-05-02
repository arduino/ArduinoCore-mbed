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
/* Routines to output text to stdout on the gdb console. */
#include <string.h>
#include <core/buffer.h>
#include <core/platforms.h>
#include <core/core.h>
#include <core/memory.h>
#include <core/gdb_console.h>


static void writeStringToExclusiveGdbCommChannel(const char* pString);
void WriteStringToGdbConsole(const char* pString)
{
    writeStringToExclusiveGdbCommChannel(pString);
}

/* Send the 'O' command to gdb to output text to its console.

    Command Format: OXX...
    Where XX is the hexadecimal representation of each character in the string to be sent to the gdb console.
*/
static void writeStringToExclusiveGdbCommChannel(const char* pString)
{
    Buffer* pBuffer = GetInitializedBuffer();

    Buffer_WriteChar(pBuffer, 'O');
    Buffer_WriteStringAsHex(pBuffer, pString);
    if (!Buffer_OverrunDetected(pBuffer))
        SendPacketToGdb();
}


void WriteHexValueToGdbConsole(uint32_t Value)
{
    Buffer BufferObject;
    char   StringBuffer[11];

    Buffer_Init(&BufferObject, StringBuffer, sizeof(StringBuffer));
    Buffer_WriteString(&BufferObject, "0x");
    Buffer_WriteUIntegerAsHex(&BufferObject, Value);
    Buffer_WriteChar(&BufferObject, '\0');

    WriteStringToGdbConsole(StringBuffer);
}
