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
/* Command handler for gdb commands related to CPU registers. */
#include <signal.h>
#include <core/cmd_common.h>
#include <core/platforms.h>
#include <core/buffer.h>
#include <core/core.h>
#include <core/mri.h>
#include <core/cmd_registers.h>


static void writeThreadIdToBuffer(Buffer* pBuffer, uint32_t threadId);
static void writeTrapReasonToBuffer(Buffer* pBuffer);
/* Sent when an exception occurs while program is executing because of previous 'c' (Continue) or 's' (Step) commands.

    Data Format: Tssii:xxxxxxxx;ii:xxxxxxxx;...

    Where ss is the hex value of the signal which caused the exception.
          ii is the hex offset of the 32-bit register value following the ':'  The offset is relative to the register
             contents in the g response packet and the SContext structure.
          xxxxxxxx is the 32-bit value of the specified register in hex format.
          The above ii:xxxxxxxx; patterns can be repeated for whichever register values should be sent with T repsonse.
*/
uint32_t Send_T_StopResponse(void)
{
    uint8_t signalValue = GetSignalValue();
    Buffer* pBuffer = GetInitializedBuffer();
    uint32_t threadId = Platform_RtosGetHaltedThreadId();

    Buffer_WriteChar(pBuffer, 'T');
    Buffer_WriteByteAsHex(pBuffer, signalValue);
    if (threadId != 0)
        writeThreadIdToBuffer(pBuffer, threadId);
    if (signalValue == SIGTRAP)
        writeTrapReasonToBuffer(pBuffer);
    Platform_WriteTResponseRegistersToBuffer(pBuffer);

    SendPacketToGdb();
    return HANDLER_RETURN_RETURN_IMMEDIATELY;
}

static void writeThreadIdToBuffer(Buffer* pBuffer, uint32_t threadId)
{
    Buffer_WriteString(pBuffer, "thread");
    Buffer_WriteChar(pBuffer, ':');
    Buffer_WriteUIntegerAsHex(pBuffer, threadId);
    Buffer_WriteChar(pBuffer, ';');
}

static void writeTrapReasonToBuffer(Buffer* pBuffer)
{
    const char* pReason;
    int         outputAddress;

    PlatformTrapReason reason = Platform_GetTrapReason();
    switch (reason.type)
    {
    case MRI_PLATFORM_TRAP_TYPE_WATCH:
        pReason = "watch";
        outputAddress = 1;
        break;
    case MRI_PLATFORM_TRAP_TYPE_RWATCH:
        pReason = "rwatch";
        outputAddress = 1;
        break;
    case MRI_PLATFORM_TRAP_TYPE_AWATCH:
        pReason = "awatch";
        outputAddress = 1;
        break;
    default:
        /* Don't dump trap reason if it is unknown. */
        return;
    }

    Buffer_WriteString(pBuffer, pReason);
    Buffer_WriteChar(pBuffer, ':');
    if (outputAddress)
        Buffer_WriteUIntegerAsHex(pBuffer, reason.address);
    Buffer_WriteChar(pBuffer, ';');
}


/* Handle the 'g' command which is to send the contents of the registers back to gdb.

    Command Format:     g
    Response Format:    xxxxxxxxyyyyyyyy...

    Where xxxxxxxx is the hexadecimal representation of the 32-bit R0 register.
          yyyyyyyy is the hexadecimal representation of the 32-bit R1 register.
          ... and so on through the members of the SContext structure.
*/
uint32_t HandleRegisterReadCommand(void)
{
    Context_CopyToBuffer(GetContext(), GetInitializedBuffer());
    return 0;
}

/* Handle the 'G' command which is to receive the new contents of the registers from gdb for the program to use when
   it resumes execution.

   Command Format:      Gxxxxxxxxyyyyyyyy...
   Response Format:     OK

    Where xxxxxxxx is the hexadecimal representation of the 32-bit R0 register.
          yyyyyyyy is the hexadecimal representation of the 32-bit R1 register.
          ... and so on through the members of the SContext structure.
*/
uint32_t HandleRegisterWriteCommand(void)
{
    Buffer*     pBuffer = GetBuffer();

    Context_CopyFromBuffer(GetContext(), pBuffer);

    if (Buffer_OverrunDetected(pBuffer))
        PrepareStringResponse(MRI_ERROR_BUFFER_OVERRUN);
    else
        PrepareStringResponse("OK");

    return 0;
}
