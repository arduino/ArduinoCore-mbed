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
/* Handler for gdb query commands. */
#include <string.h>
#include <core/buffer.h>
#include <core/core.h>
#include <core/platforms.h>
#include <core/mri.h>
#include <core/cmd_common.h>
#include <core/cmd_query.h>


typedef struct
{
    const char* pAnnex;
    uint32_t    annexSize;
    uint32_t    offset;
    uint32_t    length;
} AnnexOffsetLength;

static uint32_t    handleQuerySupportedCommand(void);
static uint32_t    handleQueryTransferCommand(void);
static uint32_t    handleQueryTransferMemoryMapCommand(void);
static void        readQueryTransferReadArguments(Buffer* pBuffer, AnnexOffsetLength* pAnnexOffsetLength);
static const char* readQueryTransferAnnexArgument(Buffer* pBuffer);
static void        readQueryTransferOffsetLengthArguments(Buffer* pBuffer, AnnexOffsetLength* pAnnexOffsetLength);
static void        validateAnnexIsNull(const char* pAnnex);
static void        handleQueryTransferReadCommand(AnnexOffsetLength* pArguments);
static uint32_t    handleQueryTransferFeaturesCommand(void);
static void        validateAnnexIs(const char* pAnnex, const char* pExpected);
/* Handle the 'q' command used by gdb to communicate state to debug monitor and vice versa.

    Command Format: qSSS
    Where SSS is a variable length string indicating which query command is being sent to the stub.
*/
uint32_t HandleQueryCommand(void)
{
    Buffer*             pBuffer = GetBuffer();
    static const char   qSupportedCommand[] = "Supported";
    static const char   qXferCommand[] = "Xfer";

    if (Buffer_MatchesString(pBuffer, qSupportedCommand, sizeof(qSupportedCommand)-1))
    {
        return handleQuerySupportedCommand();
    }
    else if (Buffer_MatchesString(pBuffer, qXferCommand, sizeof(qXferCommand)-1))
    {
        return handleQueryTransferCommand();
    }
    else
    {
        PrepareEmptyResponseForUnknownCommand();
        return 0;
    }
}

/* Handle the "qSupported" command used by gdb to communicate state to debug monitor and vice versa.

    Reponse Format: qXfer:memory-map:read+;PacketSize==SSSSSSSS
    Where SSSSSSSS is the hexadecimal representation of the maximum packet size support by this stub.
*/
static uint32_t handleQuerySupportedCommand(void)
{
    static const char querySupportResponse[] = "qXfer:memory-map:read+;qXfer:features:read+;PacketSize=";
    uint32_t          PacketSize = Platform_GetPacketBufferSize();
    Buffer*           pBuffer = GetInitializedBuffer();

    Buffer_WriteString(pBuffer, querySupportResponse);
    Buffer_WriteUIntegerAsHex(pBuffer, PacketSize);

    return 0;
}

/* Handle the "qXfer" command used by gdb to transfer data to and from the stub for special functionality.

    Command Format: qXfer:object:read:annex:offset,length
    Where supported objects are currently:
        memory-map
*/
static uint32_t handleQueryTransferCommand(void)
{
    Buffer*             pBuffer =GetBuffer();
    static const char   memoryMapObject[] = "memory-map";
    static const char   featureObject[] = "features";

    if (!Buffer_IsNextCharEqualTo(pBuffer, ':'))
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    if (Buffer_MatchesString(pBuffer, memoryMapObject, sizeof(memoryMapObject)-1))
    {
        return handleQueryTransferMemoryMapCommand();
    }
    else if (Buffer_MatchesString(pBuffer, featureObject, sizeof(featureObject)-1))
    {
        return handleQueryTransferFeaturesCommand();
    }
    else
    {
        PrepareEmptyResponseForUnknownCommand();
        return 0;
    }
}

/* Handle the "qXfer:memory-map" command used by gdb to read the device memory map from the stub.

    Command Format: qXfer:memory-map:read::offset,length
*/
static uint32_t handleQueryTransferMemoryMapCommand(void)
{
    Buffer*             pBuffer = GetBuffer();
    AnnexOffsetLength   arguments;

    __try
    {
        __throwing_func( readQueryTransferReadArguments(pBuffer, &arguments) );
        __throwing_func( validateAnnexIsNull(arguments.pAnnex) );
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    arguments.pAnnex = Platform_GetDeviceMemoryMapXml();
    arguments.annexSize = Platform_GetDeviceMemoryMapXmlSize();
    handleQueryTransferReadCommand(&arguments);

    return 0;
}

static void readQueryTransferReadArguments(Buffer* pBuffer, AnnexOffsetLength* pAnnexOffsetLength)
{
    static const char   readCommand[] = "read";

    memset(pAnnexOffsetLength, 0, sizeof(*pAnnexOffsetLength));
    if (!Buffer_IsNextCharEqualTo(pBuffer, ':') ||
        !Buffer_MatchesString(pBuffer, readCommand, sizeof(readCommand)-1) ||
        !Buffer_IsNextCharEqualTo(pBuffer, ':') )
    {
        __throw(invalidArgumentException);
    }

    __try
    {
        __throwing_func( pAnnexOffsetLength->pAnnex = readQueryTransferAnnexArgument(pBuffer) );
        __throwing_func( readQueryTransferOffsetLengthArguments(pBuffer, pAnnexOffsetLength) );
    }
    __catch
    {
        __rethrow;
    }
}

static const char* readQueryTransferAnnexArgument(Buffer* pBuffer)
{
    static const char   targetXmlAnnex[] = "target.xml";
    const char*         pReturn = NULL;

    if (Buffer_MatchesString(pBuffer, targetXmlAnnex, sizeof(targetXmlAnnex)-1))
        pReturn = targetXmlAnnex;

    if (pReturn && !Buffer_IsNextCharEqualTo(pBuffer, ':'))
        __throw_and_return(invalidArgumentException, NULL);
    else if (!pReturn && Buffer_IsNextCharEqualTo(pBuffer, ':'))
        return NULL;
    else if (!pReturn)
        __throw_and_return(invalidArgumentException, NULL);
    else
        return pReturn;
}

static void readQueryTransferOffsetLengthArguments(Buffer* pBuffer, AnnexOffsetLength* pAnnexOffsetLength)
{
    AddressLength       offsetLength;

    ReadAddressAndLengthArguments(pBuffer, &offsetLength);
    pAnnexOffsetLength->offset = offsetLength.address;
    pAnnexOffsetLength->length = offsetLength.length;
}

static void validateAnnexIsNull(const char* pAnnex)
{
    if (pAnnex)
        __throw(invalidArgumentException);
}

static void handleQueryTransferReadCommand(AnnexOffsetLength* pArguments)
{
    Buffer*  pBuffer = GetBuffer();
    char     dataPrefixChar = 'm';
    uint32_t offset = pArguments->offset;
    uint32_t length = pArguments->length;
    uint32_t outputBufferSize;
    uint32_t validMemoryMapBytes;

    if (offset >= pArguments->annexSize)
    {
        /* Attempt to read past end of XML content so flag with a l only packet. */
        dataPrefixChar = 'l';
        length = 0;
        validMemoryMapBytes = 0;
    }
    else
    {
        validMemoryMapBytes = pArguments->annexSize - offset;
    }

    InitBuffer();
    outputBufferSize = Buffer_BytesLeft(pBuffer);

    if (length > outputBufferSize)
        length = outputBufferSize;

    if (length > validMemoryMapBytes)
    {
        dataPrefixChar = 'l';
        length = validMemoryMapBytes;
    }

    Buffer_WriteChar(pBuffer, dataPrefixChar);
    Buffer_WriteSizedString(pBuffer, pArguments->pAnnex + offset, length);
}

/* Handle the "qXfer:features" command used by gdb to read the target XML from the stub.

    Command Format: qXfer:features:read:target.xml:offset,length
*/
static uint32_t handleQueryTransferFeaturesCommand(void)
{
    Buffer*             pBuffer = GetBuffer();
    AnnexOffsetLength   arguments;

    __try
    {
        __throwing_func( readQueryTransferReadArguments(pBuffer, &arguments) );
        __throwing_func( validateAnnexIs(arguments.pAnnex, "target.xml") );
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    arguments.pAnnex = Platform_GetTargetXml();
    arguments.annexSize = Platform_GetTargetXmlSize();
    handleQueryTransferReadCommand(&arguments);

    return 0;
}

static void validateAnnexIs(const char* pAnnex, const char* pExpected)
{
    if (pAnnex == NULL || 0 != strcmp(pAnnex, pExpected))
        __throw(invalidArgumentException);
}
