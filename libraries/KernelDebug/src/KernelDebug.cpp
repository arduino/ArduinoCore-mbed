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
// The GDB compatible debug monitor for kernel debugging of thread and handler mode code.
// Utilizes MRI (Monitor for Remote Inspection) library for core debugging functionality.
#include <MRI.h>
#include <cmsis_os2.h>
#include <rtx_os.h>
#include "Arduino.h"
#include "KernelDebug.h"

// Put armv7-m module into handler mode before including its header and source code.
#define MRI_THREAD_MRI 0

extern "C" {
    #include <core/core.h>
    #include <core/platforms.h>
    #include <core/semihost.h>
    #include <architectures/armv7-m/armv7-m.h>
    // Source code for armv7-m module which is included here, configured for supporting kernel mode debugging.
    #include <architectures/armv7-m/armv7-m.x>
    #include <architectures/armv7-m/debug_cm3.h>
    #include <variants/mri_variant.h>
}


// Run the DebugMonitor and UART interrupts at this priority.
#define DEBUG_ISR_PRIORITY 2

// Valid RAM memory ranges used for verifying thread pointers before dereferencing them.
struct RamRange
{
    uint32_t start;
    uint32_t length;
};

static RamRange g_ramRanges[] =
{
    {0x00000000, 0x10000},
    {0x10000000, 0x48000},
    {0x1ff00000, 0x20000},
    {0x20000000, 0x20000},
    {0x24000000, 0x80000},
    {0x30000000, 0x48000},
    {0x38000000, 0x10000},
    {0x38800000, 0x1000},
    {0x58020000, 0x2c00},
    {0x58024400, 0xc00},
    {0x58025400, 0x800},
    {0x58026000, 0x800},
    {0x58027000, 0x400}
};



// Bit location in PSR which indicates if the stack needed to be 8-byte aligned or not.
#define PSR_STACK_ALIGN_SHIFT   9

// Entries to track the chunks of the context in a scatter list.
#if MRI_DEVICE_HAS_FPU
    #define CONTEXT_SECTIONS    (6 + 3)
#else
    #define CONTEXT_SECTIONS    6
#endif


// Structures used for iterating over RTX threads via getFirstThreadId()/getNextThreadId().
struct ThreadIteratorState;

typedef uint32_t (*IteratorFunc)(ThreadIteratorState* pState);

struct ThreadIteratorState
{
    IteratorFunc   pFunc;
    osRtxThread_t* pThread;
    bool           done;
};

// Priorities of system handlers before mriInit() modifies them.
struct SystemHandlerPriorities {
    uint8_t svcallPriority;
    uint8_t pendsvPriority;
    uint8_t systickPriority;
};

// Globals that describe the KernelDebug singleton.
static mbed::UnbufferedSerial*  g_pSerial;
static uint32_t                 g_baudRate;
static IRQn_Type                g_irq;
static bool                     g_breakInSetup;

// Thread context for the most recent call to Platform_RtosGetThreadContext() from the MRI core.
static MriContext               g_threadContext;
static ContextSection           g_threadContextSections[CONTEXT_SECTIONS];
static uint32_t                 g_threadSP;

// Floats to be returned in context if the thread being debugged has no stored float state.
static uint32_t                 g_tempFloats[33];

// Fake out the special registers (MSP, PSP, etc) when dumping anything but the halting thead.
static uint32_t                 g_fakeSpecialRegs[6];

// Global used for iterating over RTX threads via Platform_RtosGetFirstThreadId()/Platform_RtosGetNextThreadId().
static ThreadIteratorState      g_threadIterState;


// Forward Function Declarations
static void setupStopInSetup();
static int justEnteredSetupCallback(void* pv);
static void initSerial();
static SystemHandlerPriorities getSystemHandlerPrioritiesBeforeMriModifiesThem();
static void restoreSystemHandlerPriorities(const SystemHandlerPriorities* pPriorities);
static void switchFaultHandlersToDebugger();
static uint32_t getFirstThreadId(ThreadIteratorState* pState);
static uint32_t getNextThreadId(ThreadIteratorState* pState);
static osRtxThread_t* verifiedThreadPtr(osRtxThread_t* pThread);
static bool isValidRamAddress(uint8_t* p);
static uint32_t iterateOverReadyList(ThreadIteratorState* pState);
static uint32_t iterateOverDelayList(ThreadIteratorState* pState);
static uint32_t iterateOverWaitList(ThreadIteratorState* pState);
static const char* getThreadStateName(uint8_t threadState);
static void readThreadContext(MriContext* pContext, uint32_t* pSP, osRtxThread_t* pThread);

// Forward declaration of external functions used by KernelDebug.
// Will be setting initial breakpoint on setup() routine.
extern "C" void setup();
// The debugger uses this handler to handle debug events, etc.
extern "C" void mriExceptionHandler(void);
// The debugger uses this handler to handle faults.
extern "C" void mriFaultHandler(void);



arduino::KernelDebug::KernelDebug(PinName txPin, PinName rxPin, IRQn_Type irq, uint32_t baudRate, bool breakInSetup /*=true*/) :
    _serial(txPin, rxPin, baudRate)
{
    // Just return without doing anything if the singleton has already been initialized.
    // This ends up using the first initialized KernelDebug object.
    if (g_pSerial != NULL) {
        return;
    }
    g_baudRate = baudRate;
    g_irq = irq;
    g_breakInSetup = breakInSetup;
    g_pSerial = &_serial;

    mriInit("");

    setupStopInSetup();
}

static void setupStopInSetup()
{
    mriCore_SetTempBreakpoint((uint32_t)setup, justEnteredSetupCallback, NULL);
}

static int justEnteredSetupCallback(void* pv)
{
    initSerial();

    // Return 0 to indicate that we want to halt execution at the beginning of setup() or 1 to not force a halt.
    return g_breakInSetup ? 0 : 1;
}

static void initSerial()
{
    // Hook communication port ISR to allow debug monitor to be awakened when GDB sends a command.
    g_pSerial->attach(mriExceptionHandler);
    // Override mbed's configuration to set the desired priority and replace the mbed's ISR with a jump directly
    // into the MRI debugger's mriExceptionHandler routine.
    mriCortexMSetPriority(g_irq, DEBUG_ISR_PRIORITY, 0);
    NVIC_SetVector(g_irq, (uint32_t)mriExceptionHandler);
}


arduino::KernelDebug::~KernelDebug()
{
    // IMPORTANT NOTE: You are attempting to destroy the connection to GDB which isn't allowed.
    //                 Don't allow your KernelDebug object to go out of scope like this.
    debugBreak();
    for (;;) {
        // Loop forever.
    }
}




// ---------------------------------------------------------------------------------------------------------------------
// Global Platform_* functions needed by MRI to initialize and communicate with MRI.
// These functions will perform most of their work through the KernelDebug singleton.
// ---------------------------------------------------------------------------------------------------------------------
void Platform_Init(Token* pParameterTokens)
{
    Context_Init(&g_threadContext,
                 g_threadContextSections,
                 sizeof(g_threadContextSections)/sizeof(g_threadContextSections[0]));

    SystemHandlerPriorities origPriorities = getSystemHandlerPrioritiesBeforeMriModifiesThem();

    __try
        mriCortexMInit((Token*)pParameterTokens, DEBUG_ISR_PRIORITY, WAKEUP_PIN_IRQn);
    __catch
        __rethrow;

    // USB defaults to a priority of 1, keep that as highest priority interrupt so that it can respond to PC requests
    // while kernel debugging.
    // Set interrupts used by UART serial comms and DebugMonitor at next highest priority, priority level 2.
    // Set all other external interrupts lower than both serial comms and DebugMonitor.
    restoreSystemHandlerPriorities(&origPriorities);

    switchFaultHandlersToDebugger();
}

static SystemHandlerPriorities getSystemHandlerPrioritiesBeforeMriModifiesThem()
{
    SystemHandlerPriorities priorities;

    priorities.svcallPriority = NVIC_GetPriority(SVCall_IRQn);
    priorities.pendsvPriority = NVIC_GetPriority(PendSV_IRQn);
    priorities.systickPriority = NVIC_GetPriority(SysTick_IRQn);
    return priorities;
}

static void restoreSystemHandlerPriorities(const SystemHandlerPriorities* pPriorities)
{
    // Restore the system handlers used by the RTOS to their original values which already use the 2 lowest priority
    // levels on the device.
    NVIC_SetPriority(SVCall_IRQn, pPriorities->svcallPriority);
    NVIC_SetPriority(PendSV_IRQn, pPriorities->pendsvPriority);
    NVIC_SetPriority(SysTick_IRQn, pPriorities->systickPriority);
}

static void switchFaultHandlersToDebugger(void)
{
    NVIC_SetVector(HardFault_IRQn,        (uint32_t)&mriFaultHandler);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)&mriFaultHandler);
    NVIC_SetVector(BusFault_IRQn,         (uint32_t)&mriFaultHandler);
    NVIC_SetVector(UsageFault_IRQn,       (uint32_t)&mriFaultHandler);
}




uint32_t Platform_CommHasReceiveData(void)
{
    return g_pSerial->readable();
}

uint32_t Platform_CommHasTransmitCompleted(void)
{
    // This function is called by MRI to make sure that last GDB command has been ACKed before it executes a reset
    // request. We will busy wait in here until that has happened and then always return true.
    while (!g_pSerial->writable()) {
        // Wait until transmit data register is empty.
    }

    // Might still have one byte in output shift register so wait 10 bit times to be safe.
    uint32_t microsecondsForOneByte = (10  * 1000000) / g_baudRate;
    delayMicroseconds(microsecondsForOneByte);

    return 1;
}

int Platform_CommReceiveChar(void)
{
    while (!Platform_CommHasReceiveData()) {
    }
    uint8_t byte;
    g_pSerial->read(&byte, 1);
    return byte;
}

void Platform_CommSendChar(int character)
{
    g_pSerial->write(&character, 1);
}




extern "C" uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_memoryMapXml) - 1;
}

extern "C" const char* Platform_GetDeviceMemoryMapXml(void)
{
    return g_memoryMapXml;
}




extern "C" const uint8_t* Platform_GetUid(void)
{
    return NULL;
}

extern "C" uint32_t Platform_GetUidSize(void)
{
    return 0;
}

int Semihost_HandleMbedSemihostRequest(PlatformSemihostParameters* pParameters)
{
    return 0;
}




uint32_t Platform_RtosGetHaltedThreadId(void)
{
    return (uint32_t)osThreadGetId();
}

uint32_t Platform_RtosGetFirstThreadId(void)
{
    return getFirstThreadId(&g_threadIterState);
}

uint32_t Platform_RtosGetNextThreadId(void)
{
    return getNextThreadId(&g_threadIterState);
}

static uint32_t getFirstThreadId(ThreadIteratorState* pState)
{
    osRtxThread_t* pRunningThread = verifiedThreadPtr(osRtxInfo.thread.run.curr);
    if (pRunningThread == NULL) {
        pState->done = true;
        pState->pFunc = NULL;
        pState->pThread = NULL;
        return 0;
    }

    pState->done = false;
    pState->pFunc = iterateOverReadyList;
    pState->pThread = NULL;
    return (uint32_t)pRunningThread;
}

static uint32_t getNextThreadId(ThreadIteratorState* pState)
{
    if (pState->done) {
        return 0;
    }

    uint32_t threadId;
    do {
        threadId = pState->pFunc(pState);
    } while (threadId == 0 && !pState->done);

    return threadId;
}

static osRtxThread_t* verifiedThreadPtr(osRtxThread_t* pThread)
{
    if (pThread == NULL) {
        return NULL;
    }

    uint8_t* pStart = (uint8_t*)pThread;
    uint8_t* pEnd = (uint8_t*)(pThread + 1) - 1;
    if (isValidRamAddress(pStart) && isValidRamAddress(pEnd) && pThread->id == osRtxIdThread) {
        return pThread;
    } else {
        return NULL;
    }
}

static bool isValidRamAddress(uint8_t* p)
{
    uint32_t addr = (uint32_t)p;

    for (size_t i = 0 ; i < sizeof(g_ramRanges)/sizeof(g_ramRanges[0]) ; i++) {
        if (addr >= g_ramRanges[i].start && addr < g_ramRanges[i].start + g_ramRanges[i].length) {
            return true;
        }
    }
    return false;
}

static uint32_t iterateOverReadyList(ThreadIteratorState* pState)
{
    if (pState->pThread == NULL) {
        pState->pThread = verifiedThreadPtr(osRtxInfo.thread.ready.thread_list);
    }
    else {
        pState->pThread = verifiedThreadPtr(pState->pThread->thread_next);
    }

    if (pState->pThread == NULL) {
        pState->pFunc = iterateOverDelayList;
    }
    return (uint32_t)pState->pThread;
}

static uint32_t iterateOverDelayList(ThreadIteratorState* pState)
{
    if (pState->pThread == NULL) {
        pState->pThread = verifiedThreadPtr(osRtxInfo.thread.delay_list);
    }
    else {
        pState->pThread = verifiedThreadPtr(pState->pThread->delay_next);
    }

    if (pState->pThread == NULL) {
        pState->pFunc = iterateOverWaitList;
    }
    return (uint32_t)pState->pThread;
}

static uint32_t iterateOverWaitList(ThreadIteratorState* pState)
{
    if (pState->pThread == NULL) {
        pState->pThread = verifiedThreadPtr(osRtxInfo.thread.wait_list);
    }
    else {
        pState->pThread = verifiedThreadPtr(pState->pThread->delay_next);
    }

    if (pState->pThread == NULL) {
        pState->pFunc = NULL;
        pState->done = true;
    }
    return (uint32_t)pState->pThread;
}

const char* Platform_RtosGetExtraThreadInfo(uint32_t threadId)
{
    osRtxThread_t* pThread = verifiedThreadPtr((osRtxThread_t*)threadId);
    if (pThread == NULL) {
        return "Invalid Thread Pointer";
    }

    static char threadExtraInfo[64];
    const char* pState = getThreadStateName(pThread->state);
    const char* pThreadName = pThread->name;
    snprintf(threadExtraInfo, sizeof(threadExtraInfo), "\"%s\" %s", pThreadName ? pThreadName : "", pState);
    return threadExtraInfo;
}

static const char* getThreadStateName(uint8_t threadState)
{
    switch (threadState) {
        case osRtxThreadInactive:
            return "Inactive";
        case osRtxThreadReady:
            return "Ready";
        case osRtxThreadRunning:
            return "Running";
        case osRtxThreadBlocked:
            return "Blocked";
        case osRtxThreadTerminated:
            return "Terminated";
        case osRtxThreadWaitingDelay:
            return "WaitingDelay";
        case osRtxThreadWaitingJoin:
            return "WaitingJoin";
        case osRtxThreadWaitingThreadFlags:
            return "WaitingThreadFlags";
        case osRtxThreadWaitingEventFlags:
            return "WaitingEventFlags";
        case osRtxThreadWaitingMutex:
            return "WaitingMutex";
        case osRtxThreadWaitingSemaphore:
            return "WaitingSemaphore";
        case osRtxThreadWaitingMemoryPool:
            return "WaitingMemoryPool";
        case osRtxThreadWaitingMessageGet:
            return "WaitingMessageGet";
        case osRtxThreadWaitingMessagePut:
            return "WaitingMessagePut";
        default:
            return "";
    }
}

MriContext* Platform_RtosGetThreadContext(uint32_t threadId)
{
    osRtxThread_t* pThread = verifiedThreadPtr((osRtxThread_t*)threadId);
    if (pThread == NULL) {
        return NULL;
    }
    if (pThread == osThreadGetId()) {
        return &mriCortexMState.context;
    }
    readThreadContext(&g_threadContext, &g_threadSP, pThread);
    return &g_threadContext;
}

static void readThreadContext(MriContext* pContext, uint32_t* pSP, osRtxThread_t* pThread)
{
    ContextSection* pSections = pContext->pSections;

    uint32_t offset;
    uint32_t stackedCount;
    if (MRI_DEVICE_HAS_FPU && (pThread->stack_frame & LR_FLOAT_STACK) == 0) {
        offset = 16;
        stackedCount = 16 + 34;
    } else {
        offset = 0;
        stackedCount = 16;
    }

    uint32_t* pThreadContext = (uint32_t*)pThread->sp;
    // R0 - R3
    pSections[0].pValues = pThreadContext + offset + 8;
    pSections[0].count = 4;
    // R4 - R11
    pSections[1].pValues = pThreadContext + offset + 0;
    pSections[1].count = 8;
    // R12
    pSections[2].pValues = pThreadContext + offset + 12;
    pSections[2].count = 1;
    // SP - Point scatter gather context to correct location for SP but set it to correct value once CPSR is more easily
    // fetched.
    pSections[3].pValues = pSP;
    pSections[3].count = 1;
    // LR, PC, CPSR
    pSections[4].pValues = pThreadContext + offset + 13;
    pSections[4].count = 3;
    // Set SP to correct value using alignment bit in CPSR. Memory for SP is already tracked by context.
    uint32_t cpsr = Context_Get(pContext, CPSR);
    Context_Set(pContext, SP, pThread->sp + sizeof(uint32_t) * (stackedCount + ((cpsr >> PSR_STACK_ALIGN_SHIFT) & 1)));

    memset(g_fakeSpecialRegs, 0, sizeof(g_fakeSpecialRegs));
    pSections[5].pValues = g_fakeSpecialRegs;
    pSections[5].count = 6;

    if (offset != 0) {
        // S0 - S15
        pSections[6].pValues = pThreadContext + 32;
        pSections[6].count = 16;
        // S16 - S31
        pSections[7].pValues = pThreadContext + 0;
        pSections[7].count = 16;
        // FPSCR
        pSections[8].pValues = pThreadContext + 48;
        pSections[8].count = 1;
    }
    else if (MRI_DEVICE_HAS_FPU && offset == 0) {
        memset(g_tempFloats, 0, sizeof(g_tempFloats));
        // S0-S31, FPSCR
        pSections[6].pValues = g_tempFloats;
        pSections[6].count = 33;
        pSections[7].pValues = NULL;
        pSections[7].count = 0;
        pSections[8].pValues = NULL;
        pSections[8].count = 0;
    }
}

int Platform_RtosIsThreadActive(uint32_t threadId)
{
    if (verifiedThreadPtr((osRtxThread_t*)threadId) == NULL) {
        return 0;
    }

    ThreadIteratorState state;
    uint32_t currThreadId = getFirstThreadId(&state);
    while (currThreadId != 0) {
        if (currThreadId == threadId) {
            return 1;
        }
        currThreadId = getNextThreadId(&state);
    }

    return 0;
}

int Platform_RtosIsSetThreadStateSupported(void)
{
    return 0;
}

void Platform_RtosSetThreadState(uint32_t threadId, PlatformThreadState state)
{
}

void Platform_RtosRestorePrevThreadState(void)
{
}





// This class can be used instead of Serial for sending output to the PC via GDB.
DebugSerial::DebugSerial()
{
}

// Methods that must be implemented for Print subclasses.
size_t DebugSerial::write(uint8_t byte)
{
    return write(&byte, sizeof(byte));
}

size_t DebugSerial::write(const uint8_t *pBuffer, size_t size)
{
    const int    STDOUT_FILE_NO = 1;
    const char*  pCurr = (const char*)pBuffer;
    int          bytesWritten = 0;

    while (size > 0) {
        int result = mriNewlib_SemihostWrite(STDOUT_FILE_NO, pCurr, size);
        if (result == -1) {
            break;
        }
        size -= result;
        pCurr += result;
        bytesWritten += result;
    }
    return bytesWritten;
}

void DebugSerial::flush()
{
}

void DebugSerial::begin(unsigned long baud, uint16_t mode)
{
    // Silence compiler warnings about unused parameters.
    (void)baud;
    (void)mode;
}

void DebugSerial::end()
{
}


// Instantiate the single instance of this stream.
class DebugSerial arduino::DebugSerial;
