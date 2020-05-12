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
/* Monitor for Remote Inspection - Provides core mri routines to initialize the debug monitor, query its state, and
   invoke it into action when a debug event occurs on the target hardware. */
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <core/mri.h>
#include <core/buffer.h>
#include <core/hex_convert.h>
#include <core/try_catch.h>
#include <core/packet.h>
#include <core/token.h>
#include <core/core.h>
#include <core/platforms.h>
#include <core/posix4win.h>
#include <core/semihost.h>
#include <core/cmd_common.h>
#include <core/cmd_file.h>
#include <core/cmd_registers.h>
#include <core/cmd_memory.h>
#include <core/cmd_continue.h>
#include <core/cmd_query.h>
#include <core/cmd_break_watch.h>
#include <core/cmd_step.h>
#include <core/cmd_thread.h>
#include <core/cmd_vcont.h>
#include <core/memory.h>


typedef struct
{
    TempBreakpointCallbackPtr   pTempBreakpointCallback;
    void*                       pvTempBreakpointContext;
    MriDebuggerHookPtr          pEnteringHook;
    MriDebuggerHookPtr          pLeavingHook;
    void*                       pvEnteringLeavingContext;
    MriContext*                 pContext;
    Packet                      packet;
    Buffer                      buffer;
    uint32_t                    tempBreakpointAddress;
    uint32_t                    flags;
    AddressRange                rangeForSingleStepping;
    int                         semihostReturnCode;
    int                         semihostErrno;
    uint8_t                     signalValue;
} MriCore;

static MriCore g_mri;

/* MriCore::flags bit definitions. */
#define MRI_FLAGS_SUCCESSFUL_INIT       (1 << 0)
#define MRI_FLAGS_FIRST_EXCEPTION       (1 << 1)
#define MRI_FLAGS_SEMIHOST_CTRL_C       (1 << 2)
#define MRI_FLAGS_TEMP_BREAKPOINT       (1 << 3)
#define MRI_FLAGS_RESET_ON_CONTINUE     (1 << 4)
#define MRI_FLAGS_RANGED_SINGLE_STEP    (1 << 5)

/* Calculates the number of items in a static array at compile time. */
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

static void clearCoreStructure(void);
static void initializePlatformSpecificModulesWithDebuggerParameters(const char* pDebuggerParameters);
static void setFirstExceptionFlag(void);
static void setSuccessfulInitFlag(void);
void mriInit(const char* pDebuggerParameters)
{
    clearCoreStructure();

    __try
        initializePlatformSpecificModulesWithDebuggerParameters(pDebuggerParameters);
    __catch
        return;

    setFirstExceptionFlag();
    setSuccessfulInitFlag();
}

static void clearCoreStructure(void)
{
    memset(&g_mri, 0, sizeof(g_mri));
}

static void initializePlatformSpecificModulesWithDebuggerParameters(const char* pDebuggerParameters)
{
    Token    tokens;

    Token_Init(&tokens);
    __try
    {
        __throwing_func( Token_SplitString(&tokens, pDebuggerParameters) );
        __throwing_func( Platform_Init(&tokens) );
    }
    __catch
        __rethrow;
}

static void setFirstExceptionFlag(void)
{
    g_mri.flags |= MRI_FLAGS_FIRST_EXCEPTION;
}

static void setSuccessfulInitFlag(void)
{
    g_mri.flags |= MRI_FLAGS_SUCCESSFUL_INIT;
}


static int isTempBreakpointSet(void);
static uint32_t clearThumbBitOfAddress(uint32_t address);
static void setTempBreakpointFlag(void);
int SetTempBreakpoint(uint32_t breakpointAddress, TempBreakpointCallbackPtr pCallback, void* pvContext)
{
    if (isTempBreakpointSet())
        return 0;

    breakpointAddress = clearThumbBitOfAddress(breakpointAddress);
    __try
        Platform_SetHardwareBreakpoint(breakpointAddress);
    __catch
    {
        clearExceptionCode();
        return 0;
    }
    g_mri.tempBreakpointAddress = breakpointAddress;
    g_mri.pTempBreakpointCallback = pCallback;
    g_mri.pvTempBreakpointContext = pvContext;
    setTempBreakpointFlag();
    return 1;
}

static int isTempBreakpointSet(void)
{
    return g_mri.flags & MRI_FLAGS_TEMP_BREAKPOINT;
}

static uint32_t clearThumbBitOfAddress(uint32_t address)
{
    return address & ~1;
}

static void setTempBreakpointFlag(void)
{
    g_mri.flags |= MRI_FLAGS_TEMP_BREAKPOINT;
}


void mriSetDebuggerHooks(MriDebuggerHookPtr pEnteringHook, MriDebuggerHookPtr pLeavingHook, void* pvContext)
{
    g_mri.pEnteringHook = pEnteringHook;
    g_mri.pLeavingHook = pLeavingHook;
    g_mri.pvEnteringLeavingContext = pvContext;
}


static int wasTempBreakpointHit(void);
static void clearTempBreakpoint(void);
static void clearTempBreakpointFlag(void);
static int areSingleSteppingInRange(void);
static void clearSingleSteppingInRange(void);
static void determineSignalValue(void);
static int  isDebugTrap(void);
static void prepareForDebuggerExit(void);
static void clearFirstExceptionFlag(void);
static int hasResetBeenRequested(void);
static void waitForAckToBeTransmitted(void);
void mriDebugException(MriContext* pContext)
{
    int justSingleStepped;

    SetContext(pContext);
    justSingleStepped = Platform_IsSingleStepping();

    if (wasTempBreakpointHit())
    {
        TempBreakpointCallbackPtr pTempBreakpointCallback = g_mri.pTempBreakpointCallback;
        void* pvTempBreakpointContext = g_mri.pvTempBreakpointContext;
        int resumeExecution;

        clearTempBreakpoint();
        if (pTempBreakpointCallback)
        {
            resumeExecution = pTempBreakpointCallback(pvTempBreakpointContext);
            if (resumeExecution)
            {
                RestoreThreadStates();
                return;
            }
        }
    }

    determineSignalValue();
    if (areSingleSteppingInRange())
    {
        uint32_t pc = Platform_GetProgramCounter();
        if (pc >= g_mri.rangeForSingleStepping.start && pc < g_mri.rangeForSingleStepping.end)
        {
            Platform_DisableSingleStep();
            Platform_EnableSingleStep();
            RestoreThreadStates();
            return;
        }
    }
    clearSingleSteppingInRange();

    if (g_mri.pEnteringHook)
        g_mri.pEnteringHook(g_mri.pvEnteringLeavingContext);
    Platform_EnteringDebugger();

    if (isDebugTrap() &&
        Semihost_IsDebuggeeMakingSemihostCall() &&
        Semihost_HandleSemihostRequest() &&
        !justSingleStepped )
    {
        RestoreThreadStates();
        prepareForDebuggerExit();
        return;
    }

    if (!IsFirstException())
        Platform_DisplayFaultCauseToGdbConsole();
    Send_T_StopResponse();

    GdbCommandHandlingLoop();

    prepareForDebuggerExit();
}

static int wasTempBreakpointHit(void)
{
    return (isTempBreakpointSet() &&
        clearThumbBitOfAddress(Platform_GetProgramCounter()) == g_mri.tempBreakpointAddress);
}

static void clearTempBreakpoint(void)
{
    __try
        Platform_ClearHardwareBreakpoint(g_mri.tempBreakpointAddress);
    __catch
        clearExceptionCode();
    g_mri.tempBreakpointAddress = 0;
    g_mri.pTempBreakpointCallback = NULL;
    g_mri.pvTempBreakpointContext = NULL;
    clearTempBreakpointFlag();
}

static void clearTempBreakpointFlag(void)
{
    g_mri.flags &= ~MRI_FLAGS_TEMP_BREAKPOINT;
}

static int areSingleSteppingInRange(void)
{
    // Ignore ranged single stepping if CTRL+C was pressed or...
    if (g_mri.signalValue == SIGINT)
        return 0;
    // if a debug breakpoint/watchpoint was hit.
    if (g_mri.signalValue == SIGTRAP)
    {
        PlatformTrapReason reason = Platform_GetTrapReason();
        if (reason.type != MRI_PLATFORM_TRAP_TYPE_UNKNOWN)
            return 0;
    }
    return g_mri.flags & MRI_FLAGS_RANGED_SINGLE_STEP;
}

static void clearSingleSteppingInRange(void)
{
    g_mri.flags &= ~MRI_FLAGS_RANGED_SINGLE_STEP;
}

static void determineSignalValue(void)
{
    g_mri.signalValue = Platform_DetermineCauseOfException();
}

static int isDebugTrap(void)
{
    return g_mri.signalValue == SIGTRAP;
}

static void prepareForDebuggerExit(void)
{
    if (hasResetBeenRequested()) {
        waitForAckToBeTransmitted();
        Platform_ResetDevice();
    }
    Platform_LeavingDebugger();
    if (g_mri.pLeavingHook)
        g_mri.pLeavingHook(g_mri.pvEnteringLeavingContext);
    clearFirstExceptionFlag();
}

static int hasResetBeenRequested(void)
{
    return (int)(g_mri.flags & MRI_FLAGS_RESET_ON_CONTINUE);
}

static void waitForAckToBeTransmitted(void)
{
    while ( !Platform_CommHasTransmitCompleted() )
    {
        // Waiting for ACK to be sent back to GDB for last command received.
    }
}

static void clearFirstExceptionFlag(void)
{
    g_mri.flags &= ~MRI_FLAGS_FIRST_EXCEPTION;
}


/*********************************************/
/* Routines to manipulate MRI state objects. */
/*********************************************/
static int handleGDBCommand(void);
static void getPacketFromGDB(void);
void GdbCommandHandlingLoop(void)
{
    int startDebuggeeUpAgain;

    do
    {
        startDebuggeeUpAgain = handleGDBCommand();
    } while (!startDebuggeeUpAgain);
}

__attribute__((weak)) uint32_t  mriPlatform_HandleGDBComand(Buffer* pBuffer);
static int handleGDBCommand(void)
{
    Buffer*         pBuffer = GetBuffer();
    uint32_t        handlerResult = 0;
    char            commandChar;
    size_t          i;
    static const struct
    {
        uint32_t     (*Handler)(void);
        char         commandChar;
    } commandTable[] =
    {
        {Send_T_StopResponse,                       '?'},
        {HandleContinueCommand,                     'c'},
        {HandleContinueWithSignalCommand,           'C'},
        {HandleDetachCommand,                       'D'},
        {HandleFileIOCommand,                       'F'},
        {HandleRegisterReadCommand,                 'g'},
        {HandleRegisterWriteCommand,                'G'},
        {HandleThreadContextCommand,                'H'},
        {HandleMemoryReadCommand,                   'm'},
        {HandleMemoryWriteCommand,                  'M'},
        {HandleQueryCommand,                        'q'},
        {HandleSingleStepCommand,                   's'},
        {HandleSingleStepWithSignalCommand,         'S'},
        {HandleIsThreadActiveCommand,               'T'},
        {HandleVContCommands,                       'v'},
        {HandleBinaryMemoryWriteCommand,            'X'},
        {HandleBreakpointWatchpointRemoveCommand,   'z'},
        {HandleBreakpointWatchpointSetCommand,      'Z'}
    };

    getPacketFromGDB();

    if (mriPlatform_HandleGDBComand)
        handlerResult = mriPlatform_HandleGDBComand(pBuffer);
    if (handlerResult == 0)
    {
        Buffer_Reset(pBuffer);
        commandChar = Buffer_ReadChar(pBuffer);
        for (i = 0 ; i < ARRAY_SIZE(commandTable) ; i++)
        {
            if (commandTable[i].commandChar == commandChar)
            {
                handlerResult = commandTable[i].Handler();
                break;
            }
        }
        if (ARRAY_SIZE(commandTable) == i)
            PrepareEmptyResponseForUnknownCommand();
    }

    if ((handlerResult & HANDLER_RETURN_RETURN_IMMEDIATELY) == 0)
        SendPacketToGdb();
    return handlerResult & HANDLER_RETURN_RESUME_PROGRAM;
}

static void getPacketFromGDB(void)
{
    InitBuffer();
    Packet_GetFromGDB(&g_mri.packet, &g_mri.buffer);
}


void InitBuffer(void)
{
    Buffer_Init(&g_mri.buffer, Platform_GetPacketBuffer(), Platform_GetPacketBufferSize());
}


void PrepareStringResponse(const char* pErrorString)
{
    InitBuffer();
    Buffer_WriteString(&g_mri.buffer, pErrorString);
}


int WasSuccessfullyInit(void)
{
    return (int)(g_mri.flags & MRI_FLAGS_SUCCESSFUL_INIT);
}


int WasControlCFlagSentFromGdb(void)
{
    return (int)(g_mri.flags & MRI_FLAGS_SEMIHOST_CTRL_C);
}

void RequestResetOnNextContinue(void)
{
    g_mri.flags |= MRI_FLAGS_RESET_ON_CONTINUE;
}

void SetSingleSteppingRange(const AddressRange* pRange)
{
    g_mri.rangeForSingleStepping = *pRange;
    g_mri.flags |= MRI_FLAGS_RANGED_SINGLE_STEP;
}


MriContext* GetContext(void)
{
    return g_mri.pContext;
}

void SetContext(MriContext* pContext)
{
    g_mri.pContext = pContext;
}


void RecordControlCFlagSentFromGdb(int controlCFlag)
{
    if (controlCFlag)
        g_mri.flags |= MRI_FLAGS_SEMIHOST_CTRL_C;
    else
        g_mri.flags &= ~MRI_FLAGS_SEMIHOST_CTRL_C;
}


int WasSemihostCallCancelledByGdb(void)
{
    return g_mri.semihostErrno == EINTR;
}


void FlagSemihostCallAsHandled(void)
{
    Platform_AdvanceProgramCounterToNextInstruction();
    Platform_SetSemihostCallReturnAndErrnoValues(g_mri.semihostReturnCode, g_mri.semihostErrno);
}


int IsFirstException(void)
{
    return (int)(g_mri.flags & MRI_FLAGS_FIRST_EXCEPTION);
}


void SetSignalValue(uint8_t signalValue)
{
    g_mri.signalValue = signalValue;
}


uint8_t GetSignalValue(void)
{
    return g_mri.signalValue;
}


void SetSemihostReturnValues(int semihostReturnCode, int semihostErrNo)
{
    g_mri.semihostReturnCode = semihostReturnCode;
    g_mri.semihostErrno = semihostErrNo;
}


int GetSemihostReturnCode(void)
{
    return g_mri.semihostReturnCode;
}


int GetSemihostErrno(void)
{
    return g_mri.semihostErrno;
}


Buffer* GetBuffer(void)
{
    return &g_mri.buffer;
}


Buffer* GetInitializedBuffer(void)
{
    InitBuffer();
    return &g_mri.buffer;
}


void SendPacketToGdb(void)
{
    if (Buffer_OverrunDetected(&g_mri.buffer))
    {
        InitBuffer();
        Buffer_WriteString(&g_mri.buffer, MRI_ERROR_BUFFER_OVERRUN);
    }

    Buffer_SetEndOfBuffer(&g_mri.buffer);
    Packet_SendToGDB(&g_mri.packet, &g_mri.buffer);
}
