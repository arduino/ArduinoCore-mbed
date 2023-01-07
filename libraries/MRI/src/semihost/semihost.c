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
/* Semihost functionality for redirecting operations such as file I/O to the GNU debugger. */
#include <core/core.h>
#include <core/gdb_console.h>
#include <core/platforms.h>
#include <core/semihost.h>
#include <core/signal.h>


int Semihost_IsDebuggeeMakingSemihostCall(void)
{
    PlatformInstructionType instructionType = Platform_TypeOfCurrentInstruction();

    return (instructionType == MRI_PLATFORM_INSTRUCTION_MBED_SEMIHOST_CALL ||
            instructionType == MRI_PLATFORM_INSTRUCTION_NEWLIB_SEMIHOST_CALL);
}

int Semihost_HandleSemihostRequest(void)
{
    PlatformInstructionType    instructionType = Platform_TypeOfCurrentInstruction();
    PlatformSemihostParameters parameters = Platform_GetSemihostCallParameters();

    if (instructionType == MRI_PLATFORM_INSTRUCTION_MBED_SEMIHOST_CALL)
        return Semihost_HandleMbedSemihostRequest(&parameters);
    else if (instructionType == MRI_PLATFORM_INSTRUCTION_NEWLIB_SEMIHOST_CALL)
        return Semihost_HandleNewlibSemihostRequest(&parameters);
    else
        return 0;
}


static int writeToGdbConsole(const TransferParameters* pParameters);
int Semihost_WriteToFileOrConsole(const TransferParameters* pParameters)
{
    const uint32_t STDOUT_FILE_NO = 1;
    if (pParameters->fileDescriptor == STDOUT_FILE_NO)
    {
        return writeToGdbConsole(pParameters);
    }
    return IssueGdbFileWriteRequest(pParameters);
}

static int writeToGdbConsole(const TransferParameters* pParameters)
{
    const char* pBuffer = (const char*)pParameters->bufferAddress;
    size_t length = pParameters->bufferSize;

    size_t charsWritten = WriteSizedStringToGdbConsole(pBuffer, length);

    SetSemihostReturnValues(charsWritten, 0);
    FlagSemihostCallAsHandled();

    if (WasControlCEncountered())
    {
        SetSignalValue(SIGINT);
        return 0;
    }
    return 1;
}
