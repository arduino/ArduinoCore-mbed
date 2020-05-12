/* Copyright 2020 Adam Green (https://github.com/adamgreen/)

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
/*  'Class' which represents a text buffer.  Has routines to both extract and inject strings of various types into the
    buffer while verifying that no overflow takes place. */
#ifndef BUFFER_H_
#define BUFFER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    char*   pStart;
    char*   pEnd;
    char*   pCurrent;
} Buffer;

/* Real name of functions are in mri namespace. */
void     mriBuffer_Init(Buffer* pBuffer, char* pBufferStart, size_t bufferSize);
void     mriBuffer_Reset(Buffer* pBuffer);
void     mriBuffer_SetEndOfBuffer(Buffer* pBuffer);
size_t   mriBuffer_BytesLeft(Buffer* pBuffer);
int      mriBuffer_OverrunDetected(Buffer* pBuffer);
size_t   mriBuffer_GetLength(Buffer* pBuffer);
char*    mriBuffer_GetArray(Buffer* pBuffer);
void     mriBuffer_WriteChar(Buffer* pBuffer, char character);
char     mriBuffer_ReadChar(Buffer* pBuffer);
void     mriBuffer_WriteByteAsHex(Buffer* pBuffer, uint8_t byte);
uint8_t  mriBuffer_ReadByteAsHex(Buffer* pBuffer);
void     mriBuffer_WriteString(Buffer* pBuffer, const char* pString);
void     mriBuffer_WriteSizedString(Buffer* pBuffer, const char* pString, size_t length);
void     mriBuffer_WriteStringAsHex(Buffer* pBuffer, const char* pString);
uint32_t mriBuffer_ReadUIntegerAsHex(Buffer* pBuffer);
void     mriBuffer_WriteUIntegerAsHex(Buffer* pBuffer, uint32_t value);
int32_t  mriBuffer_ReadIntegerAsHex(Buffer* pBuffer);
void     mriBuffer_WriteIntegerAsHex(Buffer* pBuffer, int32_t value);
int      mriBuffer_IsNextCharEqualTo(Buffer* pBuffer, char thisChar);
int      mriBuffer_MatchesString(Buffer* pBuffer, const char* pString, size_t stringLength);
int      mriBuffer_MatchesHexString(Buffer* pBuffer, const char* pString, size_t stringLength);

/* Macroes which allow code to drop the mri namespace prefix. */
#define Buffer_Init                 mriBuffer_Init
#define Buffer_Reset                mriBuffer_Reset
#define Buffer_SetEndOfBuffer       mriBuffer_SetEndOfBuffer
#define Buffer_BytesLeft            mriBuffer_BytesLeft
#define Buffer_OverrunDetected      mriBuffer_OverrunDetected
#define Buffer_GetLength            mriBuffer_GetLength
#define Buffer_GetArray             mriBuffer_GetArray
#define Buffer_WriteChar            mriBuffer_WriteChar
#define Buffer_ReadChar             mriBuffer_ReadChar
#define Buffer_WriteByteAsHex       mriBuffer_WriteByteAsHex
#define Buffer_ReadByteAsHex        mriBuffer_ReadByteAsHex
#define Buffer_WriteString          mriBuffer_WriteString
#define Buffer_WriteSizedString     mriBuffer_WriteSizedString
#define Buffer_WriteStringAsHex     mriBuffer_WriteStringAsHex
#define Buffer_ReadUIntegerAsHex    mriBuffer_ReadUIntegerAsHex
#define Buffer_WriteUIntegerAsHex   mriBuffer_WriteUIntegerAsHex
#define Buffer_ReadIntegerAsHex     mriBuffer_ReadIntegerAsHex
#define Buffer_WriteIntegerAsHex    mriBuffer_WriteIntegerAsHex
#define Buffer_IsNextCharEqualTo    mriBuffer_IsNextCharEqualTo
#define Buffer_MatchesString        mriBuffer_MatchesString
#define Buffer_MatchesHexString     mriBuffer_MatchesHexString

#endif /* BUFFER_H_ */
