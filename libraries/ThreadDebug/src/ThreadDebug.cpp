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
// The GDB compatible debug monitor for debugging application threads.
// Utilizes MRI (Monitor for Remote Inspection) library for core debugging functionality.
#include <Arduino.h>
#include <MRI.h>
#include "ThreadDebug.h"
#include <cmsis_os2.h>
#include <rtx_os.h>
#include <rtx_core_c.h>

// Put armv7-m module into thread mode before including its header and source code.
#define MRI_THREAD_MRI 1

extern "C"
{
    #include <core/core.h>
    #include <core/platforms.h>
    #include <core/semihost.h>
    #include <core/context.h>
    #include <architectures/armv7-m/armv7-m.h>
    // Source code for armv7-m module which is included here, configured for supporting thread mode debugging.
    #include <architectures/armv7-m/armv7-m.x>
    #include <architectures/armv7-m/debug_cm3.h>
}


// ---------------------------------------------------------------------------------------------------------------------
// Configuration Parameters
// ---------------------------------------------------------------------------------------------------------------------
// The size of the mriMain() stack in uint64_t objects.
#define MRI_THREAD_STACK_SIZE   128
// The size of the mriIdle() stack in uint64_t objects.
#define IDLE_THREAD_STACK_SIZE  10
// The minimum number of threads that can be specified maxThreadCount parameter of the ThreadDebug constructor.
#define MINIMUM_THREAD_COUNT    6

// Threads with these names will not be suspended when a debug event has occurred.
// Typically these will be threads used by the communication stack to communicate with GDB or other important system
// threads.
static const char* g_threadNamesToIgnore[] = {
};



// Bit location in PSR which indicates if the stack needed to be 8-byte aligned or not.
#define PSR_STACK_ALIGN_SHIFT               9

// Bit in LR set to 0 when automatic stacking of floating point registers occurs during exception handling.
#define LR_FLOAT_STACK                      (1 << 4)

// RTX Thread synchronization flag used to indicate that a debug event has occured.
#define MRI_THREAD_DEBUG_EVENT_FLAG         (1 << 0)

// Lower nibble of EXC_RETURN in LR will have one of these values if interrupted code was running in thread mode.
//  Using PSP.
#define EXC_RETURN_THREADMODE_PROCESSSTACK  0xD
//  Using MSP.
#define EXC_RETURN_THREADMODE_MAINSTACK     0x9

// Macro to make is easier to calculate array size.
#undef ARRAY_SIZE
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

// Assert routine that will dump error text to USB GDB connection before entering infinite loop. If user is logging MRI
// remote communications then they will see the error text in the log before debug stub becomes unresponseive.
#undef ASSERT
#define ASSERT(X) \
    if (!(X)) { \
        Serial.print("Assertion Failed: "); \
        Serial.print(__FILE__); \
        Serial.print(":"); \
        Serial.print(__LINE__); \
        Serial.print("  \""); \
        Serial.print(#X); \
        Serial.println("\""); \
        for (;;); \
    }




// If non-NULL, this is the thread that we want to single step.
// If NULL, single stepping is not enabled.
// Accessed by this module and the assembly language handlers in ThreadDebug_asm.S.
volatile osThreadId_t           mriThreadSingleStepThreadId;

// Structures used to track information about all of the application's threads.  It tracks a thread's original priority
// settings (current and base) so that they can be later restored. It also tracks the thread-id and current
// frozen/thawed/single-stepping state as well.
struct ThreadInfo
{
    osThreadId_t threadId;
    int8_t       priority;
    int8_t       basePriority;
    uint8_t      state;
};

struct ThreadList
{
    ThreadInfo* pThreadList;
    uint32_t    threadCount;
    uint32_t    maxThreadCount;
    uint32_t    index;

    void allocate(uint32_t count)
    {
        pThreadList = new ThreadInfo[count];
        threadCount = 0;
        index = 0;
        maxThreadCount = count;
    }

    void free()
    {
        delete []pThreadList;
        pThreadList = NULL;
        maxThreadCount = 0;
        threadCount = 0;
    }

    void reset()
    {
        threadCount = 0;
        index = 0;
    }

    void add(osThreadId_t threadId, int8_t priority, int8_t basePriority)
    {
        if (threadCount < maxThreadCount) {
            ThreadInfo* pInfo = pThreadList + threadCount;
            pInfo->threadId = threadId;
            pInfo->priority = priority;
            pInfo->basePriority = basePriority;
            pInfo->state = (int8_t)MRI_PLATFORM_THREAD_FROZEN;
            threadCount++;
        }
    }

    void resumeThreads()
    {
        for (uint32_t i = 0 ; i < threadCount ; i++) {
            ThreadInfo*  pInfo = &pThreadList[i];
            if (pInfo->state != MRI_PLATFORM_THREAD_FROZEN) {
                thawThread(pInfo, pInfo->priority, pInfo->basePriority);
            }
            if (shouldThawIdleThreadDuringSingleStep(pInfo)) {
                // Gives RTX a thread to run if single stepping thread is waiting on sync object.
                thawThread(pInfo, osPriorityIdle+1, osPriorityIdle+1);
            }
        }
    }

    void thawThread(ThreadInfo* pInfo, int8_t priority, int8_t basePriority)
    {
        osThreadId_t threadId = pInfo->threadId;
        osThreadSetPriority(threadId, (osPriority_t)priority);
        osRtxThread_t* pThread = (osRtxThread_t*)threadId;
        pThread->priority_base = basePriority;
    }

    bool shouldThawIdleThreadDuringSingleStep(ThreadInfo* pInfo)
    {
        return pInfo->threadId == osRtxInfo.thread.idle &&
               pInfo->state == (uint8_t)MRI_PLATFORM_THREAD_FROZEN &&
               mriThreadSingleStepThreadId != 0;
    }

    void sort()
    {
        qsort(pThreadList, threadCount, sizeof(*pThreadList), compareThreadInfo);
    }

    static int compareThreadInfo(const void* pvThread1, const void* pvThread2)
    {
        ThreadInfo* pThread1 = (ThreadInfo*)pvThread1;
        ThreadInfo* pThread2 = (ThreadInfo*)pvThread2;
        osThreadId_t threadId1 = pThread1->threadId;
        osThreadId_t threadId2 = pThread2->threadId;

        if (threadId1 < threadId2) {
            return -1;
        }
        else if (threadId1 > threadId2) {
            return 1;
        }
        else {
            return 0;
        }
    }

    osThreadId_t firstThreadId()
    {
        index = 0;
        return nextThreadId();
    }

    osThreadId_t nextThreadId()
    {
        if (index >= threadCount) {
            return 0;
        }
        return pThreadList[index++].threadId;
    }

    ThreadInfo* findThreadId(osThreadId_t threadId)
    {
        ThreadInfo key = {
            .threadId = threadId,
            .priority = 0,
            .basePriority = 0,
            .state = MRI_PLATFORM_THREAD_FROZEN
        };
        return (ThreadInfo*)bsearch(&key, pThreadList, threadCount, sizeof(*pThreadList), compareThreadInfo);
    }

    void setStateOnAllThreads(PlatformThreadState state)
    {
        for (uint32_t i = 0 ; i < threadCount ; i++) {
            pThreadList[i].state = (uint8_t)state;
        }
    }

    void setStateOnFrozenThreads(PlatformThreadState state)
    {
        for (uint32_t i = 0 ; i < threadCount ; i++) {
            if (pThreadList[i].state == MRI_PLATFORM_THREAD_FROZEN)
                pThreadList[i].state = (uint8_t)state;
        }
    }

    void copyFrom(ThreadList* pFrom)
    {
        for (uint32_t i = 0 ; i < threadCount ; i++) {
            ThreadInfo* pThread = pFrom->findThreadId(pThreadList[i].threadId);
            if (pThread) {
                pThreadList[i].state = pThread->state;
                if (pThread->state == MRI_PLATFORM_THREAD_SINGLE_STEPPING) {
                    mriThreadSingleStepThreadId = pThread->threadId;
                }
            }
        }
    }
};




// Globals that describe the ThreadDebug singleton.
static DebugCommInterface*      g_pComm;
static bool                     g_breakInSetup;

// The ID of the halted thread being debugged.
static volatile osThreadId_t    g_haltedThreadId;

// The list of all RTX threads which were active upon entry into the debugger. The application's threads will be pulled
// from this list and used to populate g_pCurrThreadList.
static osThreadId_t*            g_pAllActiveThreads;
// The priorities (current and base) & state (frozen, thawed, etc) of each thread modified to suspend application
// threads when halting in debugger. g_pPrevThreadList and g_pCurrThreadList will ping pong between these 2 instances.
static ThreadList               g_threadLists[2];
// Previous thread info list.
static ThreadList*              g_pPrevThreadList;
// Current thread info list.
static ThreadList*              g_pCurrThreadList;
// The maximum number of threads which can be stored in the above arrays, set in ThreadDebug constructor.
static uint32_t                 g_maxThreadCount;
// Buffer to be used for storing extra thread info.
static char                     g_threadExtraInfo[64];

// This flag is set to a non-zero value if the DebugMon handler is to re-enable DWT watchpoints and FPB breakpoints
// after being disabled by the HardFault handler when a debug event is encounted in handler mode.
static volatile uint32_t        g_enableDWTandFPB;

// This flag is set to a non-zero value if a DebugMon interrupt was pended because GDB sent CTRL+C.
static volatile uint32_t        g_controlC;

// The ID of the mriMain() thread.
volatile osThreadId_t           mriThreadId;
// The ID of the mriIdle() thread.
static   osThreadId_t           g_mriIdleThreadId;

// Addresses of the original RTX handlers for SVCall, SysTick, and PendSV when hooks to them have been inserted for
// enabling single step.
volatile uint32_t               mriThreadOrigSVCall;
volatile uint32_t               mriThreadOrigSysTick;
volatile uint32_t               mriThreadOrigPendSV;
// Addresses of the original fault handlers before being replaced with debugger specific ones.
volatile uint32_t               mriThreadOrigHardFault;
volatile uint32_t               mriThreadOrigMemManagement;
volatile uint32_t               mriThreadOrigBusFault;
volatile uint32_t               mriThreadOrigUsageFault;

// Entries to track the chunks of the context in a scatter list.
#if MRI_DEVICE_HAS_FPU
    #define CONTEXT_SECTIONS    (5 + 3)
#else
    #define CONTEXT_SECTIONS    5
#endif

// The halting thread's context sections will be placed in these entries and hooked into mriCortexMState.context.
static ContextSection           g_contextSections[CONTEXT_SECTIONS];

// Thread context for the most recent call to Platform_RtosGetThreadContext() from the MRI core.
static MriContext               g_threadContext;
static ContextSection           g_threadContextSections[CONTEXT_SECTIONS];
static uint32_t                 g_threadSP;

// Floats to be returned in context if the thread being debugged has no stored float state.
static uint32_t                 g_tempFloats[33];

// UNDONE: For debugging. If different than g_haltedThreadId then the mriMain() thread was signalled when the previous
//         instance was still running.
static volatile osThreadId_t    g_debugThreadId;




// Assembly Language fault handling stubs. They do some preprocessing and then call the C handlers if appropriate.
extern "C" void mriDebugMonitorHandlerStub(void);
extern "C" void mriHardFaultHandlerStub(void);
extern "C" void mriMemManagementHandlerStub(void);
extern "C" void mriBusFaultHandlerStub(void);
extern "C" void mriUsageFaultHandlerStub(void);
// Assembly Language stubs for RTX context switching routines. They check to see if DebugMon should be pended before
// calling the actual RTX handlers to enable single stepping on mriThreadSingleStepThreadId.
extern "C" void mriSVCHandlerStub(void);
extern "C" void mriPendSVHandlerStub(void);
extern "C" void mriSysTickHandlerStub(void);

// Forward Function Declarations
static __NO_RETURN void mriIdle(void *pv);
static __NO_RETURN void mriMain(void *pv);
static void suspendAllApplicationThreads();
static void swapThreadLists();
static bool isThreadToIgnore(osThreadId_t thread);
static void readThreadContext(MriContext* pContext, uint32_t* pSP, osThreadId_t thread);
static void switchRtxHandlersToDebugStubsForSingleStepping();
static void restoreRtxHandlers();
static void callAttachFromSetup();
static int justEnteredSetupCallback(void* pv);
static bool isThreadMode(uint32_t excReturn);
static bool hasEncounteredDebugEvent();
static void recordAndClearFaultStatusBits();
static void wakeMriMainToDebugCurrentThread();
static void stopSingleStepping();
static void recordAndSwitchFaultHandlersToDebugger();
static const char* getThreadStateName(uint8_t threadState);
static bool isDebugThreadActive();
static void setFaultDetectedFlag();
static bool isImpreciseBusFault();
static void advancePCToNextInstruction(uint32_t excReturn, uint32_t psp, uint32_t msp);
static uint32_t* threadSP(uint32_t excReturn, uint32_t psp, uint32_t msp);
static void clearFaultStatusBits();
static void serialISRHook();
static bool isDebuggerActive();



ThreadDebug::ThreadDebug(DebugCommInterface* pCommInterface, bool breakInSetup, uint32_t maxThreadCount)
{
    if (g_pComm != NULL) {
        // Only allow 1 ThreadDebug object to be initialized.
        return;
    }

    // Setup the singleton.
    g_breakInSetup = breakInSetup;
    g_pComm = pCommInterface;

    // Allocate the arrays to store thread information.
    if (maxThreadCount < MINIMUM_THREAD_COUNT) {
        maxThreadCount = MINIMUM_THREAD_COUNT;
    }
    g_pAllActiveThreads = new osThreadId_t[maxThreadCount];
    g_threadLists[0].allocate(maxThreadCount);
    g_threadLists[1].allocate(maxThreadCount);
    g_pCurrThreadList = &g_threadLists[0];
    g_pPrevThreadList = &g_threadLists[1];
    g_maxThreadCount = maxThreadCount;

    // Initialize the MRI core.
    mriInit("");

    // Start the debugger's idle thread and suspend it for now.
    static uint64_t             idleStack[IDLE_THREAD_STACK_SIZE];
    static osRtxThread_t        idleThreadTcb;
    static const osThreadAttr_t idleThreadAttr =
    {
        .name = "mriIdle",
        .attr_bits = osThreadDetached,
        .cb_mem  = &idleThreadTcb,
        .cb_size = sizeof(idleThreadTcb),
        .stack_mem = idleStack,
        .stack_size = sizeof(idleStack),
        .priority = (osPriority_t)(osPriorityIdle + 1)
    };
    g_mriIdleThreadId = osThreadNew(mriIdle, NULL, &idleThreadAttr);
    if (g_mriIdleThreadId == NULL) {
        return;
    }
    osThreadSuspend(g_mriIdleThreadId);

    // Start the debugger thread.
    static uint64_t             mainStack[MRI_THREAD_STACK_SIZE];
    static osRtxThread_t        mainThreadTcb;
    static const osThreadAttr_t mainThreadAttr =
    {
        .name = "mriMain",
        .attr_bits = osThreadDetached,
        .cb_mem  = &mainThreadTcb,
        .cb_size = sizeof(mainThreadTcb),
        .stack_mem = mainStack,
        .stack_size = sizeof(mainStack),
        .priority = osPriorityNormal
    };
    mriThreadId = osThreadNew(mriMain, NULL, &mainThreadAttr);
    if (mriThreadId == NULL) {
        return;
    }

    callAttachFromSetup();
}

static __NO_RETURN void mriIdle(void *pv)
{
    while (true) {
        // Infinite loop at low priority if nothing else to do while debugging.
    }
}

static __NO_RETURN void mriMain(void *pv)
{
    // Run the code which suspends, resumes, etc the other threads at highest priority so that it doesn't context
    // switch to one of the other threads. Switch to normal priority when running mriDebugException() though.
    osThreadSetPriority(osThreadGetId(), osPriorityISR);

    while (1) {
        int waitResult = osThreadFlagsWait(MRI_THREAD_DEBUG_EVENT_FLAG, osFlagsWaitAny, osWaitForever);
        ASSERT ( waitResult > 0 );
        ASSERT ( g_haltedThreadId != 0 );
        suspendAllApplicationThreads();
        readThreadContext(&mriCortexMState.context, &mriCortexMState.sp, g_haltedThreadId);

        osThreadResume(g_mriIdleThreadId);
        osThreadSetPriority(osThreadGetId(), osPriorityNormal);
            mriDebugException(&mriCortexMState.context);
        osThreadSetPriority(osThreadGetId(), osPriorityISR);
        osThreadSuspend(g_mriIdleThreadId);

        if (Platform_IsSingleStepping()) {
            switchRtxHandlersToDebugStubsForSingleStepping();
        }
        g_pCurrThreadList->resumeThreads();
        g_haltedThreadId = 0;
    }
}

static void suspendAllApplicationThreads()
{
    // Suspend application threads by setting their priorities to the lowest setting, osPriorityIdle.
    swapThreadLists();
    g_pCurrThreadList->reset();
    uint32_t threadCount = osThreadGetCount();
    ASSERT ( threadCount <= g_maxThreadCount );
    osThreadEnumerate(g_pAllActiveThreads, g_maxThreadCount);
    for (uint32_t i = 0 ; i < threadCount ; i++) {
        osThreadId_t thread = g_pAllActiveThreads[i];

        if (!isThreadToIgnore(thread)) {
            osRtxThread_t* pThread = (osRtxThread_t*)thread;
            int8_t priority = pThread->priority;
            int8_t basePriority = pThread->priority_base;
            ThreadInfo* pPrevThreadInfo = g_pPrevThreadList->findThreadId(thread);
            if (pPrevThreadInfo && pPrevThreadInfo->state == (uint8_t)MRI_PLATFORM_THREAD_FROZEN) {
                // If thread was already frozen then the thread's current priorities will be osPriorityIdle and the
                // correct priorities will be stored in the previous thread list.
                priority = pPrevThreadInfo->priority;
                basePriority = pPrevThreadInfo->basePriority;
            }
            g_pCurrThreadList->add(thread, priority, basePriority);
            osThreadSetPriority(thread, osPriorityIdle);
        }
    }
    g_pCurrThreadList->sort();
}

static void swapThreadLists()
{
    ThreadList* pTemp = g_pPrevThreadList;
    g_pPrevThreadList = g_pCurrThreadList;
    g_pCurrThreadList = pTemp;
}

static bool isThreadToIgnore(osThreadId_t thread)
{
    if (thread == mriThreadId || thread == g_mriIdleThreadId) {
        // Don't want to suspend the debugger threads themselves.
        return true;
    }

    const char*  pThreadName = osThreadGetName(thread);
    for (size_t i = 0 ; i < ARRAY_SIZE(g_threadNamesToIgnore) ; i++) {
        if (strcmp(pThreadName, g_threadNamesToIgnore[i]) == 0) {
            return true;
        }
    }
    return false;
}

static void readThreadContext(MriContext* pContext, uint32_t* pSP, osThreadId_t thread)
{
    osRtxThread_t* pThread = (osRtxThread_t*)thread;
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

    if (offset != 0) {
        // S0 - S15
        pSections[5].pValues = pThreadContext + 32;
        pSections[5].count = 16;
        // S16 - S31
        pSections[6].pValues = pThreadContext + 0;
        pSections[6].count = 16;
        // FPSCR
        pSections[7].pValues = pThreadContext + 48;
        pSections[7].count = 1;
    } else if (MRI_DEVICE_HAS_FPU && offset == 0) {
        memset(g_tempFloats, 0, sizeof(g_tempFloats));
        // S0-S31, FPSCR
        pSections[5].pValues = g_tempFloats;
        pSections[5].count = 33;
        pSections[6].pValues = NULL;
        pSections[6].count = 0;
        pSections[7].pValues = NULL;
        pSections[7].count = 0;
    }
}

static void switchRtxHandlersToDebugStubsForSingleStepping()
{
    mriThreadOrigSVCall = NVIC_GetVector(SVCall_IRQn);
    mriThreadOrigPendSV = NVIC_GetVector(PendSV_IRQn);
    mriThreadOrigSysTick = NVIC_GetVector(SysTick_IRQn);
    NVIC_SetVector(SVCall_IRQn, (uint32_t)mriSVCHandlerStub);
    NVIC_SetVector(PendSV_IRQn, (uint32_t)mriPendSVHandlerStub);
    NVIC_SetVector(SysTick_IRQn, (uint32_t)mriSysTickHandlerStub);
}

static void restoreRtxHandlers()
{
    NVIC_SetVector(SVCall_IRQn, mriThreadOrigSVCall);
    NVIC_SetVector(PendSV_IRQn, mriThreadOrigPendSV);
    NVIC_SetVector(SysTick_IRQn, mriThreadOrigSysTick);
}

static void callAttachFromSetup()
{
    mriCore_SetTempBreakpoint((uint32_t)setup, justEnteredSetupCallback, NULL);
}

static int justEnteredSetupCallback(void* pv)
{
    g_pComm->attach(serialISRHook);

    if (!g_breakInSetup) {
        // Unfreeze all of the threads and return 1 to not force a halt in setup().
        g_pCurrThreadList->setStateOnAllThreads(MRI_PLATFORM_THREAD_THAWED);
        return 1;
    }

    // Return 0 to indicate that we want to halt execution at the beginning of setup().
    return 0;
}


ThreadDebug::~ThreadDebug()
{
    g_threadLists[0].free();
    g_threadLists[1].free();
    delete []g_pAllActiveThreads;
    g_pAllActiveThreads = NULL;

    // IMPORTANT NOTE: You are attempting to destroy the connection to GDB which isn't allowed.
    //                 Don't allow your ThreadDebug object to go out of scope like this.
    debugBreak();
    for (;;) {
        // Loop forever.
    }
}




// ---------------------------------------------------------------------------------------------------------------------
// Global Platform_* functions needed by MRI to initialize and communicate with MRI.
// These functions will perform most of their work through the DebugSerial singleton.
// ---------------------------------------------------------------------------------------------------------------------
void Platform_Init(Token* pParameterTokens)
{
    uint32_t debugMonPriority = 255;

    __try
        mriCortexMInit((Token*)pParameterTokens, debugMonPriority, (IRQn_Type)0);
    __catch
        __rethrow;

    g_enableDWTandFPB = 0;
    mriThreadSingleStepThreadId = NULL;
    Context_Init(&mriCortexMState.context, g_contextSections, ARRAY_SIZE(g_contextSections));
    Context_Init(&g_threadContext, g_threadContextSections, ARRAY_SIZE(g_threadContextSections));
    recordAndSwitchFaultHandlersToDebugger();
}

static void recordAndSwitchFaultHandlersToDebugger()
{
    mriThreadOrigHardFault = NVIC_GetVector(HardFault_IRQn);
    mriThreadOrigMemManagement = NVIC_GetVector(MemoryManagement_IRQn);
    mriThreadOrigBusFault = NVIC_GetVector(BusFault_IRQn);
    mriThreadOrigUsageFault = NVIC_GetVector(UsageFault_IRQn);

    NVIC_SetVector(HardFault_IRQn,        (uint32_t)mriHardFaultHandlerStub);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)mriMemManagementHandlerStub);
    NVIC_SetVector(BusFault_IRQn,         (uint32_t)mriBusFaultHandlerStub);
    NVIC_SetVector(UsageFault_IRQn,       (uint32_t)mriUsageFaultHandlerStub);
    NVIC_SetVector(DebugMonitor_IRQn,     (uint32_t)mriDebugMonitorHandlerStub);
}




uint32_t Platform_CommHasReceiveData(void)
{
    return g_pComm->readable();
}

uint32_t Platform_CommHasTransmitCompleted(void)
{
    return g_pComm->writeable();
}

int Platform_CommReceiveChar(void)
{
    while (!g_pComm->readable()) {
        // Busy wait.
    }
    return g_pComm->read();
}

void Platform_CommSendChar(int character)
{
    g_pComm->write(character);
}



#ifdef STM32H747xx
static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
#ifndef CORE_CM4
                                     "<memory type=\"ram\" start=\"0x00000000\" length=\"0x10000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x10000000\" length=\"0x48000\"> </memory>"
#endif
                                     "<memory type=\"flash\" start=\"0x08000000\" length=\"0x200000\"> <property name=\"blocksize\">0x20000</property></memory>"
                                     "<memory type=\"ram\" start=\"0x1ff00000\" length=\"0x20000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x20000000\" length=\"0x20000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x24000000\" length=\"0x80000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x30000000\" length=\"0x48000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x38000000\" length=\"0x10000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x38800000\" length=\"0x1000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x58020000\" length=\"0x2c00\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x58024400\" length=\"0xc00\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x58025400\" length=\"0x800\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x58026000\" length=\"0x800\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x58027000\" length=\"0x400\"> </memory>"
                                     "<memory type=\"flash\" start=\"0x90000000\" length=\"0x10000000\"> <property name=\"blocksize\">0x200</property></memory>"
#ifndef CORE_CM4
                                     "<memory type=\"ram\" start=\"0x60000000\" length=\"0x800000\"> </memory>"
#endif
                                     "</memory-map>";
#endif

#ifdef NRF52840_XXAA
static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
                                     "<memory type=\"flash\" start=\"0x00000000\" length=\"0x100000\"> <property name=\"blocksize\">0x1000</property></memory>"
                                     "<memory type=\"ram\" start=\"0x20000000\" length=\"0x40000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x00800000\" length=\"0x40000\"> </memory>"
                                     "</memory-map>";
#endif

uint32_t Platform_GetDeviceMemoryMapXmlSize(void)
{
    return sizeof(g_memoryMapXml) - 1;
}

const char* Platform_GetDeviceMemoryMapXml(void)
{
    return g_memoryMapXml;
}




const uint8_t* Platform_GetUid(void)
{
    return NULL;
}

uint32_t Platform_GetUidSize(void)
{
    return 0;
}

int Semihost_IsDebuggeeMakingSemihostCall(void)
{
    return 0;
}

int Semihost_HandleSemihostRequest(void)
{
    return 0;
}




uint32_t Platform_RtosGetHaltedThreadId(void)
{
    return (uint32_t)g_haltedThreadId;
}

uint32_t Platform_RtosGetFirstThreadId(void)
{
    return (uint32_t)g_pCurrThreadList->firstThreadId();
}

uint32_t Platform_RtosGetNextThreadId(void)
{
    return (uint32_t)g_pCurrThreadList->nextThreadId();
}

const char* Platform_RtosGetExtraThreadInfo(uint32_t threadId)
{
    if (!Platform_RtosIsThreadActive(threadId)) {
        return NULL;
    }

    const char*    pThreadName = osThreadGetName((osThreadId)threadId);
    osRtxThread_t* pThread = (osRtxThread_t*)threadId;
    const char*    pState = getThreadStateName(pThread->state);
    snprintf(g_threadExtraInfo, sizeof(g_threadExtraInfo), "\"%s\" %s", pThreadName ? pThreadName : "", pState);
    return g_threadExtraInfo;
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
    if (!Platform_RtosIsThreadActive(threadId)) {
        return NULL;
    }

    if ((osThreadId_t)threadId == g_haltedThreadId) {
        return &mriCortexMState.context;
    } else {
        readThreadContext(&g_threadContext, &g_threadSP, (osThreadId_t)threadId);
        return &g_threadContext;
    }
}

int Platform_RtosIsThreadActive(uint32_t threadId)
{
    return g_pCurrThreadList->findThreadId((osThreadId_t)threadId) != NULL;
}

int Platform_RtosIsSetThreadStateSupported(void)
{
    return 1;
}

void Platform_RtosSetThreadState(uint32_t threadId, PlatformThreadState state)
{
    if (threadId == MRI_PLATFORM_ALL_THREADS) {
        g_pCurrThreadList->setStateOnAllThreads(state);
        return;
    }

    if (threadId == MRI_PLATFORM_ALL_FROZEN_THREADS) {
        g_pCurrThreadList->setStateOnFrozenThreads(state);
        return;
    }

    ThreadInfo* pThreadInfo = g_pCurrThreadList->findThreadId((osThreadId_t)threadId);
    if (pThreadInfo == NULL) {
        return;
    }
    pThreadInfo->state = (uint8_t)state;

    if (state == MRI_PLATFORM_THREAD_SINGLE_STEPPING) {
        mriThreadSingleStepThreadId = (osThreadId_t)threadId;
    }
}


void Platform_RtosRestorePrevThreadState(void)
{
    g_pCurrThreadList->copyFrom(g_pPrevThreadList);
}





// ---------------------------------------------------------------------------------------------------------------------
// Functions called from Assembly Language fault/interrupt handlers to deal with crashes and debug events.
// ---------------------------------------------------------------------------------------------------------------------
extern "C" void mriDebugMonitorHandler(uint32_t excReturn)
{
    while (!isThreadMode(excReturn)) {
        // DebugMon is running at such low priority that we should be getting ready to return to thread mode.
    }

    if (g_enableDWTandFPB > 0) {
        enableDWTandITM();
        enableFPB();
        atomic_dec32((uint32_t*)&g_enableDWTandFPB);
    }

    if (!hasEncounteredDebugEvent() && !g_controlC) {
        if (mriThreadSingleStepThreadId) {
            // Code is written to handle case where single stepping gets enabled because current thread was the one
            // to be single stepped but then a higher priority interrupt comes in and makes another thread the
            // current thread so single stepping should be disabled again.
            if (mriThreadSingleStepThreadId == osThreadGetId()) {
                enableSingleStep();
            } else {
                disableSingleStep();
            }
        }

        return;
    }

    // Just return if debug event was created by MRI handling GDB commands, such as reading from an address of
    // interest before clearing the rwatch.
    if (isDebuggerActive()) {
        SCB->DFSR = SCB->DFSR;
        return;
    }

    // Get here when a debug event of interest has occurred in a thread.
    recordAndClearFaultStatusBits();
    stopSingleStepping();
    g_controlC = 0;
    wakeMriMainToDebugCurrentThread();
}

// This function is called if a fault (hard, mem, bus, usage) has occurred while running code in thread mode.
// It is also called when a debug event has forced a hard fault while running code in handler mode.
extern "C" void mriFaultHandler(uint32_t excReturn, uint32_t psp, uint32_t msp)
{
    if (isThreadMode(excReturn)) {
        if (isDebugThreadActive()) {
            // Encountered memory fault when GDB attempted to access an invalid address.
            // Set flag to let debugger thread know that its access failed and advance past the faulting instruction
            // if it was a precise bus fault so that it doesn't just occur again on return.
            setFaultDetectedFlag();
            if (!isImpreciseBusFault()) {
                advancePCToNextInstruction(excReturn, psp, msp);
            }
            clearFaultStatusBits();
            return;
        }

        // A crash has been detected in thread mode. Wake debugger thread to debug it.
        recordAndClearFaultStatusBits();
        stopSingleStepping();
        wakeMriMainToDebugCurrentThread();
        return;
    }

    // The asm stub calling this function has already verified that a debug event during handler mode caused
    // this fault. Disable DWT watchpoints and FPB breakpoints until re-entering thread mode.
    atomic_inc32((uint32_t*)&g_enableDWTandFPB);
    SCB->DFSR = SCB->DFSR;
    SCB->HFSR = SCB_HFSR_DEBUGEVT_Msk;
    disableDWTandITM();
    disableFPB();
    setMonitorPending();
}

static bool isThreadMode(uint32_t excReturn)
{
    excReturn &= 0xF;
    return excReturn == EXC_RETURN_THREADMODE_PROCESSSTACK || excReturn == EXC_RETURN_THREADMODE_MAINSTACK;
}

static bool hasEncounteredDebugEvent()
{
    return SCB->DFSR != 0;

}

static void recordAndClearFaultStatusBits()
{
    mriCortexMState.exceptionNumber = getCurrentlyExecutingExceptionNumber();
    mriCortexMState.dfsr = SCB->DFSR;
    mriCortexMState.hfsr = SCB->HFSR;
    mriCortexMState.cfsr = SCB->CFSR;
    mriCortexMState.mmfar = SCB->MMFAR;
    mriCortexMState.bfar = SCB->BFAR;

    // Clear fault status bits by writing 1s to bits that are already set.
    SCB->DFSR = mriCortexMState.dfsr;
    SCB->HFSR = mriCortexMState.hfsr;
    SCB->CFSR = mriCortexMState.cfsr;
}

static void wakeMriMainToDebugCurrentThread()
{
    g_debugThreadId = osThreadGetId();
    g_haltedThreadId = g_debugThreadId;
    osThreadFlagsSet(mriThreadId, MRI_THREAD_DEBUG_EVENT_FLAG);
}

static void stopSingleStepping()
{
    if (Platform_IsSingleStepping()) {
        restoreRtxHandlers();
    }
    disableSingleStep();
    mriThreadSingleStepThreadId = NULL;
}

static bool isDebugThreadActive()
{
    return (mriCortexMFlags & CORTEXM_FLAGS_ACTIVE_DEBUG) && mriThreadId == osThreadGetId();
}

static void setFaultDetectedFlag()
{
    mriCortexMFlags |= CORTEXM_FLAGS_FAULT_DURING_DEBUG;
}

static bool isImpreciseBusFault()
{
    return SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk;
}

static void advancePCToNextInstruction(uint32_t excReturn, uint32_t psp, uint32_t msp)
{
    uint32_t* pSP = threadSP(excReturn, psp, msp);
    uint32_t* pPC = pSP + 6;
    uint16_t  currentInstruction = *(uint16_t*)*pPC;
    if (isInstruction32Bit(currentInstruction)) {
        *pPC += sizeof(uint32_t);
    } else {
        *pPC += sizeof(uint16_t);
    }
}

static uint32_t* threadSP(uint32_t excReturn, uint32_t psp, uint32_t msp)
{
    uint32_t sp;
    if ((excReturn & 0xF) == EXC_RETURN_THREADMODE_PROCESSSTACK) {
        sp = psp;
    } else {
        sp = msp;
    }
    return (uint32_t*)sp;
}

static void clearFaultStatusBits()
{
    /* Clear fault status bits by writing 1s to bits that are already set. */
    SCB->DFSR = SCB->DFSR;
    SCB->HFSR = SCB->HFSR;
    SCB->CFSR = SCB->CFSR;
}


static void serialISRHook()
{
    if (!isDebuggerActive() && !isThreadToIgnore(osThreadGetId()) && g_pComm->readable()) {
        // Pend a halt into the debug monitor now that there is data from GDB ready to be read by it.
        setControlCFlag();
        g_controlC = 1;
        setMonitorPending();
    }
}

static bool isDebuggerActive()
{
    return mriCortexMFlags & CORTEXM_FLAGS_ACTIVE_DEBUG;
}




DebugCommInterface::~DebugCommInterface()
{
}

UartDebugCommInterface::UartDebugCommInterface(PinName txPin, PinName rxPin, uint32_t baudRate) :
    _pCallback(NULL), _serial(txPin, rxPin, baudRate), _baudRate(baudRate), _read(0), _write(0)
{
    _serial.attach(mbed::callback(this, &UartDebugCommInterface::onReceivedData));
}

UartDebugCommInterface::~UartDebugCommInterface()
{
}

bool UartDebugCommInterface::readable()
{
    return _read != _write;
}

bool UartDebugCommInterface::writeable()
{
    // This function is called by MRI to make sure that last GDB command has been ACKed before it executes a reset
    // request. We will busy wait in here until that has happened and then always return true.
    while (!_serial.writable()) {
        // Wait until transmit data register is empty.
    }

    // Might still have one byte in output shift register so wait 10 bit times to be safe.
    uint32_t microsecondsForOneByte = (10  * 1000000) / _baudRate;
    delayMicroseconds(microsecondsForOneByte);

    return 1;
}

uint8_t UartDebugCommInterface::read()
{
    // This read should never block since Platform_CommReceiveChar() always checks readable() first.
    ASSERT ( readable() );

    uint8_t byte = _queue[_read];
    _read = wrappingIncrement(_read);
    return byte;
}

uint32_t UartDebugCommInterface::wrappingIncrement(uint32_t val)
{
    return (val + 1) & (sizeof(_queue) - 1);
}

void UartDebugCommInterface::write(uint8_t c)
{
    _serial.write(&c, 1);
}

void UartDebugCommInterface::attach(void (*pCallback)())
{
    _pCallback = pCallback;
}

void UartDebugCommInterface::onReceivedData()
{
    while (_serial.readable()) {
        uint8_t byte;
        _serial.read(&byte, 1);
        if (wrappingIncrement(_write) != _read) {
            // _queue isn't full so we can add this byte to it.
            _queue[_write] = byte;
            _write = wrappingIncrement(_write);
        }
    }

    if (_pCallback) {
        _pCallback();
    }
}

#ifdef SERIAL_CDC
UsbDebugCommInterface::UsbDebugCommInterface(arduino::USBSerial* pSerial) :
    _pSerial(pSerial)
{
}

UsbDebugCommInterface::~UsbDebugCommInterface()
{
}

bool UsbDebugCommInterface::readable()
{
    return _pSerial->available() > 0;
}

bool UsbDebugCommInterface::writeable()
{
    // The USBSerial::write() method blocks until data is actually sent to the PC.
    return true;
}

uint8_t UsbDebugCommInterface::read()
{
    return _pSerial->read();
}

void UsbDebugCommInterface::write(uint8_t c)
{
    _pSerial->write(c);
}

void UsbDebugCommInterface::attach(void (*pCallback)())
{
    _pSerial->attach(pCallback);
}
#endif

#if defined(STM32H747xx) && defined(CORE_CM4)

RPCDebugCommInterface::RPCDebugCommInterface(arduino::SerialRPCClass* pSerial) :
    _pSerial(pSerial)
{
    //_pSerial->begin();
}

RPCDebugCommInterface::~RPCDebugCommInterface()
{
}

bool RPCDebugCommInterface::readable()
{
    return _pSerial->available() > 0;
}

bool RPCDebugCommInterface::writeable()
{
    // The USBSerial::write() method blocks until data is actually sent to the PC.
    return true;
}

uint8_t RPCDebugCommInterface::read()
{
    return _pSerial->read();
}

void RPCDebugCommInterface::write(uint8_t c)
{
    _pSerial->write(c);
}

void RPCDebugCommInterface::attach(void (*pCallback)())
{
    _pSerial->attach(pCallback);
}
#endif