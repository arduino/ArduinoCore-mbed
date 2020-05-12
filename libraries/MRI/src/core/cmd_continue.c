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
/* Handler for continue gdb command. */
#include <core/buffer.h>
#include <core/core.h>
#include <core/platforms.h>
#include <core/mri.h>
#include <core/cmd_common.h>
#include <core/cmd_continue.h>


static int shouldSkipHardcodedBreakpoint(void);
static int isCurrentInstructionHardcodedBreakpoint(void);
uint32_t ContinueExecution(int setPC, uint32_t newPC)
{
    if (Platform_RtosIsSetThreadStateSupported())
        Platform_RtosSetThreadState(MRI_PLATFORM_ALL_THREADS, MRI_PLATFORM_THREAD_THAWED);
    uint32_t returnValue = SkipHardcodedBreakpoint();
    if (setPC)
        Platform_SetProgramCounter(newPC);
    return (returnValue | HANDLER_RETURN_RESUME_PROGRAM | HANDLER_RETURN_RETURN_IMMEDIATELY);
}

uint32_t SkipHardcodedBreakpoint(void)
{
    if (shouldSkipHardcodedBreakpoint())
    {
        Platform_AdvanceProgramCounterToNextInstruction();
        return HANDLER_RETURN_SKIPPED_OVER_BREAK;
    }

    return 0;
}

static int shouldSkipHardcodedBreakpoint(void)
{
    return !Platform_WasProgramCounterModifiedByUser() && isCurrentInstructionHardcodedBreakpoint();
}

static int isCurrentInstructionHardcodedBreakpoint(void)
{
    return Platform_TypeOfCurrentInstruction() == MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT;
}


/* Handle the 'c' command which is sent from gdb to tell the debugger to continue execution of the currently halted
   program.

    Command Format:     cAAAAAAAA
    Response Format:    Blank until the next exception, at which time a 'T' stop response packet will be sent.

    Where AAAAAAAA is an optional value to be used for the Program Counter when restarting the program.
*/
uint32_t HandleContinueCommand(void)
{
    Buffer*     pBuffer = GetBuffer();
    int         setPC = 0;
    uint32_t    newPC = 0;

    /* New program counter value is optional parameter. */
    __try
    {
        __throwing_func( newPC = ReadUIntegerArgument(pBuffer) );
        setPC = 1;
    }
    __catch
    {
        clearExceptionCode();
    }
    return ContinueExecution(setPC, newPC);
}



/* Handle the 'C' command which is sent from gdb to tell the debugger to continue execution of the currently halted
   program. It is similar to the 'c' command but it also provides a signal level, which MRI ignores.

    Command Format:     cAA;BBBBBBBB
    Response Format:    Blank until the next exception, at which time a 'T' stop response packet will be sent.

    Where AA is the signal to be set, and
          BBBBBBBB is an optional value to be used for the Program Counter when restarting the program.
*/
uint32_t HandleContinueWithSignalCommand(void)
{
    Buffer*     pBuffer = GetBuffer();
    int         setPC = 0;
    uint32_t    newPC = 0;

    __try
    {
        /* Fetch signal value but ignore it. */
        __throwing_func( ReadUIntegerArgument(pBuffer) );
        if (Buffer_BytesLeft(pBuffer) && Buffer_IsNextCharEqualTo(pBuffer, ';'))
        {
            __throwing_func( newPC = ReadUIntegerArgument(pBuffer) );
            setPC = 1;
        }
    }
    __catch
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    return ContinueExecution(setPC, newPC);
}


/* Handle the 'D' command which is sent from gdb to resume execution before it detaches and exits.

    Command Format:     D
    Response Format:    OK

*/
uint32_t HandleDetachCommand(void)
{
    if (Platform_RtosIsSetThreadStateSupported())
        Platform_RtosSetThreadState(MRI_PLATFORM_ALL_THREADS, MRI_PLATFORM_THREAD_THAWED);
    SkipHardcodedBreakpoint();
    PrepareStringResponse("OK");
    return HANDLER_RETURN_RESUME_PROGRAM;
}
