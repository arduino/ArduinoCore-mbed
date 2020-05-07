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
/* Handler for gdb's vCont and vCont? commands which support multithreaded single stepping/continuing. */
#include <core/cmd_vcont.h>
#include <core/buffer.h>
#include <core/cmd_common.h>
#include <core/cmd_continue.h>
#include <core/cmd_registers.h>
#include <core/cmd_step.h>
#include <core/core.h>
#include <core/mri.h>
#include <core/platforms.h>
#include <core/try_catch.h>
#include <string.h>


typedef enum
{
    THREAD_ID_ALL = -1,
    THREAD_ID_NONE,
    THREAD_ID_SPECIFIC
} ThreadIdType;

typedef struct
{
    uint32_t     id;
    ThreadIdType type;
} ThreadId;

typedef enum
{
    ACTION_NONE,
    ACTION_CONTINUE,
    ACTION_SINGLE_STEP,
    ACTION_RANGED_SINGLE_STEP
} ActionType;

typedef struct
{
    ThreadId        threadId;
    AddressRange    range;
    ActionType      type;
} Action;


/* Store last continue/step actions here so that they can be used to replay Platform_RtosSetThreadState() calls
   during ranged single steps. */
static Action g_lastContinueAction;
static Action g_lastStepAction;


static uint32_t handleVContQueryCommand(void);
static uint32_t handleVContCommand(void);
static Action   parseAction(Buffer* pBuffer);
static Action   parseContinueAction(Buffer* pBuffer);
static Action   parseContinueWithSignalAction(Buffer* pBuffer);
static Action   parseSingleStepAction(Buffer* pBuffer);
static Action   parseSingleStepWithSignalAction(Buffer* pBuffer);
static Action   parseRangedSingleStepAction(Buffer* pBuffer);
static AddressRange parseAddressRange(Buffer* pBuffer);
static ThreadId parseOptionalThreadId(Buffer* pBuffer);
static int      doesThreadIdMatchHaltedThreadId(const Action* pAction);
static uint32_t handleSingleStepAndContinueCommands(const Action* pContinueAction, const Action* pStepAction);
static uint32_t handleAction(const Action* pAction);
static uint32_t handleSingleStepAndContinueCommandsWithSetThreadState(const Action* pContinueAction, const Action* pStepAction);
static uint32_t handleActionWithSetThreadState(const Action* pAction);
static uint32_t justAdvancedPastBreakpoint(uint32_t continueReturn);
/* Handle the extended 'v' commands used by gdb.

    Command Format: vSSS
    Where SSS is a variable length string indicating which extended command is being sent to the stub.
*/
uint32_t HandleVContCommands(void)
{
    Buffer*             pBuffer = GetBuffer();
    static const char   vContQueryCommand[] = "Cont?";
    static const char   vContCommand[] = "Cont";

    if (Buffer_MatchesString(pBuffer, vContQueryCommand, sizeof(vContQueryCommand)-1))
    {
        return handleVContQueryCommand();
    }
    else if (Buffer_MatchesString(pBuffer, vContCommand, sizeof(vContCommand)-1))
    {
        return handleVContCommand();
    }
    else
    {
        PrepareEmptyResponseForUnknownCommand();
        return 0;
    }
}

static uint32_t handleVContQueryCommand(void)
{
    Buffer* pBuffer = GetInitializedBuffer();
    Buffer_WriteString(pBuffer, "vCont;c;C;s;S;r");
    return 0;
}

/* Handle the "vCont" command used by gdb for extended single stepping/continuing of execution.

    Command Format: vCont[;action[:thread-id]]
    Where supported actions are currently:
        c - Continue execution.
        CAA - Continue execution where AA is the signal to be set. Signals ignored by MRI.
        s - Single step one instruction.
        SAA - Single step one instruction where AA is the signal to be set. Signals ignored by MRI.
        rAAAAAAAA,BBBBBBBB - Single step through instructions while in address range between AAAAAAAA (inclusive) and
                             BBBBBBBB (exclusive).
    Where thread-id indicates threads to which this action should be applied. -1 means all threads. If no thread-id
    is specified then this is the default action to be applied to threads which aren't otherwise specified for an
    action.
*/
static uint32_t handleVContCommand(void)
{
    Action            continueAction = { {0, THREAD_ID_NONE}, {0, 0}, ACTION_NONE };
    Action            stepAction = { {0, THREAD_ID_NONE}, {0, 0}, ACTION_NONE };
    Buffer*           pBuffer = GetBuffer();
    Action            action;

    while (Buffer_BytesLeft(pBuffer) > 0 && Buffer_IsNextCharEqualTo(pBuffer, ';'))
    {
        __try
        {
            action = parseAction(pBuffer);
        }
        __catch
        {
            PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
            return 0;
        }

        switch (action.type)
        {
            case ACTION_RANGED_SINGLE_STEP:
            case ACTION_SINGLE_STEP:
                if (stepAction.type != ACTION_NONE)
                {
                    /* Should only have 1 single step action in vCont command. */
                    PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
                    return 0;
                }
                stepAction = action;
                break;
            case ACTION_CONTINUE:
                if (action.threadId.type == THREAD_ID_SPECIFIC || continueAction.type != ACTION_NONE)
                {
                    /* Continue action shouldn't specify thread id & there should only be 1 of them in vCont command. */
                    PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
                    return 0;
                }
                continueAction = action;
                break;
            default:
                break;
        }
    }
    if (Buffer_BytesLeft(pBuffer) > 0)
    {
        /* After processing all arguments, there should be no bytes left in packet. */
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    if (Platform_RtosIsSetThreadStateSupported())
        return handleSingleStepAndContinueCommandsWithSetThreadState(&continueAction, &stepAction);
    else
        return handleSingleStepAndContinueCommands(&continueAction, &stepAction);
}

static Action parseAction(Buffer* pBuffer)
{
    Action action = { {0, THREAD_ID_NONE}, {0, 0}, ACTION_NONE };
    char   ch = Buffer_ReadChar(pBuffer);
    switch (ch)
    {
        case 'c':
            return parseContinueAction(pBuffer);
        case 'C':
            return parseContinueWithSignalAction(pBuffer);
        case 'r':
            return parseRangedSingleStepAction(pBuffer);
        case 's':
            return parseSingleStepAction(pBuffer);
        case 'S':
            return parseSingleStepWithSignalAction(pBuffer);
        default:
            setExceptionCode(invalidArgumentException);
            return action;
    }
}

static Action parseContinueAction(Buffer* pBuffer)
{
    Action action = { {THREAD_ID_NONE, 0}, {0, 0}, ACTION_CONTINUE };
    action.threadId  = parseOptionalThreadId(pBuffer);
    return action;
}

static Action parseContinueWithSignalAction(Buffer* pBuffer)
{
    Action action = { {THREAD_ID_NONE, 0}, {0, 0}, ACTION_CONTINUE };

    __try
    {
        /* Fetch signal value but ignore it. */
        __throwing_func( Buffer_ReadByteAsHex(pBuffer) );
        action.threadId  = parseOptionalThreadId(pBuffer);
    }
    __catch
    {
        __rethrow_and_return(action);
    }
    return action;
}

static Action parseSingleStepAction(Buffer* pBuffer)
{
    Action action = { {THREAD_ID_NONE, 0}, {0, 0}, ACTION_SINGLE_STEP };
    action.threadId  = parseOptionalThreadId(pBuffer);
    return action;
}

static Action parseSingleStepWithSignalAction(Buffer* pBuffer)
{
    Action action = { {THREAD_ID_NONE, 0}, {0, 0}, ACTION_SINGLE_STEP };
    __try
    {
        /* Fetch signal value but ignore it. */
        __throwing_func( Buffer_ReadByteAsHex(pBuffer) );
        action.threadId  = parseOptionalThreadId(pBuffer);
    }
    __catch
    {
        __rethrow_and_return(action);
    }
    return action;
}

static Action parseRangedSingleStepAction(Buffer* pBuffer)
{
    Action action = { {THREAD_ID_NONE, 0}, {0, 0}, ACTION_RANGED_SINGLE_STEP };
    AddressRange range = { 0, 0 };
    __try
    {
        __throwing_func( action.range = parseAddressRange(pBuffer) );
        __throwing_func( action.threadId  = parseOptionalThreadId(pBuffer) );
        (void)range;
    }
    __catch
    {
        __rethrow_and_return(action);
    }
    return action;
}

static AddressRange parseAddressRange(Buffer* pBuffer)
{
    AddressRange range = {0, 0};
    __try
    {
        __throwing_func( range.start = Buffer_ReadUIntegerAsHex(pBuffer) );
        __throwing_func( ThrowIfNextCharIsNotEqualTo(pBuffer, ',') );
        __throwing_func( range.end = Buffer_ReadUIntegerAsHex(pBuffer) );
    }
    __catch
    {
        __rethrow_and_return(range);
    }
    return range;
}

static ThreadId parseOptionalThreadId(Buffer* pBuffer)
{
    static const char negativeOne[] = "-1";
    ThreadId          threadId = { 0, THREAD_ID_NONE };

    if (Buffer_BytesLeft(pBuffer) == 0 || !Buffer_IsNextCharEqualTo(pBuffer, ':'))
        return threadId;

    if (Buffer_MatchesString(pBuffer, negativeOne, sizeof(negativeOne)-1))
    {
        threadId.type = THREAD_ID_ALL;
        return threadId;
    }

    __try
    {
        __throwing_func( threadId.id = Buffer_ReadUIntegerAsHex(pBuffer) );
        threadId.type = THREAD_ID_SPECIFIC;
    }
    __catch
    {
        __rethrow_and_return(threadId);
    }

    return threadId;
}

static int doesThreadIdMatchHaltedThreadId(const Action* pAction)
{
    return pAction->threadId.type == THREAD_ID_SPECIFIC && pAction->threadId.id == Platform_RtosGetHaltedThreadId();
}

static uint32_t handleSingleStepAndContinueCommands(const Action* pContinueAction, const Action* pStepAction)
{
    uint32_t returnValue = 0;

    if (pStepAction->type != ACTION_NONE)
        returnValue |= handleAction(pStepAction);
    else
        returnValue |= handleAction(pContinueAction);

    return returnValue;
}

static uint32_t handleAction(const Action* pAction)
{
    const   int       noSetPC = 0;
    const   uint32_t  newPC = 0;
    uint32_t          returnValue = 0;

    /* No matter what thread ID is specified, it will be treated as though it was the halting thread-id since that is
       all a stub running without Platform_RtosSetThreadState() can handle anyway. */
    switch (pAction->type)
    {
        case ACTION_CONTINUE:
            return ContinueExecution(noSetPC, newPC);
        case ACTION_SINGLE_STEP:
            return HandleSingleStepCommand();
        case ACTION_RANGED_SINGLE_STEP:
            returnValue = HandleSingleStepCommand();
            if (returnValue)
                SetSingleSteppingRange(&pAction->range);
            return returnValue;
        default:
            PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
            return 0;
   }
}

static uint32_t handleSingleStepAndContinueCommandsWithSetThreadState(const Action* pContinueAction, const Action* pStepAction)
{
    uint32_t returnValue = 0;
    uint32_t skippedHardcodedBreakpoint = 0;

    if (pContinueAction->type != ACTION_NONE || doesThreadIdMatchHaltedThreadId(pStepAction))
        skippedHardcodedBreakpoint = SkipHardcodedBreakpoint();

    if (pContinueAction->type != ACTION_NONE)
        returnValue |= handleActionWithSetThreadState(pContinueAction);

    if (pStepAction->type != ACTION_NONE)
    {
        if (pStepAction->threadId.type != THREAD_ID_SPECIFIC)
        {
            PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
            return 0;
        }
        returnValue |= handleActionWithSetThreadState(pStepAction);
    }

    if (pContinueAction->type == ACTION_NONE && pStepAction->type == ACTION_NONE)
    {
        PrepareStringResponse(MRI_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    g_lastContinueAction = *pContinueAction;
    g_lastStepAction = *pStepAction;

    if (doesThreadIdMatchHaltedThreadId(pStepAction) && justAdvancedPastBreakpoint(skippedHardcodedBreakpoint))
    {
        /* Treat the advance over hardcoded breakpoints as the single step and don't resume execution but send T
            stop response instead. */
        return Send_T_StopResponse();
    }

    return returnValue;
}

static uint32_t handleActionWithSetThreadState(const Action* pAction)
{
    uint32_t returnValue = 0;

    switch (pAction->type)
    {
        case ACTION_CONTINUE:
            Platform_RtosSetThreadState(MRI_PLATFORM_ALL_THREADS, MRI_PLATFORM_THREAD_THAWED);
            returnValue = HANDLER_RETURN_RESUME_PROGRAM | HANDLER_RETURN_RETURN_IMMEDIATELY;
            break;
        case ACTION_SINGLE_STEP:
            Platform_RtosSetThreadState(pAction->threadId.id, MRI_PLATFORM_THREAD_SINGLE_STEPPING);
            Platform_EnableSingleStep();
            returnValue = HANDLER_RETURN_RESUME_PROGRAM | HANDLER_RETURN_RETURN_IMMEDIATELY;
            break;
        case ACTION_RANGED_SINGLE_STEP:
            Platform_RtosSetThreadState(pAction->threadId.id, MRI_PLATFORM_THREAD_SINGLE_STEPPING);
            Platform_EnableSingleStep();
            SetSingleSteppingRange(&pAction->range);
            returnValue = HANDLER_RETURN_RESUME_PROGRAM | HANDLER_RETURN_RETURN_IMMEDIATELY;
            break;
        default:
            break;
   }
   return returnValue;
}

static uint32_t justAdvancedPastBreakpoint(uint32_t continueReturn)
{
    return continueReturn & HANDLER_RETURN_SKIPPED_OVER_BREAK;
}


void ReplaySetThreadStateCalls(void)
{
    if (!Platform_RtosIsSetThreadStateSupported())
        return;
    if (g_lastContinueAction.type != ACTION_NONE)
        Platform_RtosSetThreadState(MRI_PLATFORM_ALL_THREADS, MRI_PLATFORM_THREAD_THAWED);
    if (g_lastStepAction.type != ACTION_NONE)
        Platform_RtosSetThreadState(g_lastStepAction.threadId.id, MRI_PLATFORM_THREAD_SINGLE_STEPPING);
}


void ForgetSetThreadStateCalls(void)
{
    memset(&g_lastContinueAction, 0, sizeof(g_lastContinueAction));
    memset(&g_lastStepAction, 0, sizeof(g_lastStepAction));
}
