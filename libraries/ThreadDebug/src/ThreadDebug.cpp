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
// The size of the mriThread() stack in uint64_t objects.
#define MRI_THREAD_STACK_SIZE   128
// The maximum number of active threads that can be handled by the debugger.
#define MAXIMUM_ACTIVE_THREADS  64

// Threads with these names will not be suspended when a debug event is occurred.
// Typically these will be threads used by the communication stack to communicate with GDB or other important system
// threads.
static const char* g_threadNamesToIgnore[] = {
};



// Bit location in PSR which indicates if the stack needed to be 8-byte aligned or not.
#define PSR_STACK_ALIGN_SHIFT   9

// Bit in LR set to 0 when automatic stacking of floating point registers occurs during exception handling.
#define LR_FLOAT_STACK          (1 << 4)

// RTX Thread synchronization flag used to indicate that a debug event has occured.
#define MRI_THREAD_DEBUG_EVENT_FLAG (1 << 0)

// Lower nibble of EXC_RETURN in LR will have one of these values if interrupted code was running in thread mode.
//  Using PSP.
#define EXC_RETURN_THREADMODE_PROCESSSTACK  0xD
//  Using MSP.
#define EXC_RETURN_THREADMODE_MAINSTACK     0x9

// Macro to make is easier to calculate array size.
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

// Assert routine that will dump error text to USB GDB connection before entering infinite loop. If user is logging MRI
// remote communications then they will see the error text in the log before debug stub becomes unresponseive.
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




// Structure used to store a 'suspended' thread's original priority settings (current and base) so that they can be
// later restored.
struct ThreadPriority
{
    int8_t priority;
    int8_t basePriority;
};




// Globals that describe the ThreadDebug singleton.
static DebugCommInterface*      g_pComm;
static bool                     g_breakInSetup;

// The ID of the halted thread being debugged.
static volatile osThreadId_t    g_haltedThreadId;

// The list of threads which were suspended upon entry into the debugger. These are the threads that will be resumed
// upon exit from the debugger.
static osThreadId_t             g_suspendedThreads[MAXIMUM_ACTIVE_THREADS];
// The priorities (current and base) of each thread modified to suspend application threads when halting in debugger.
static ThreadPriority           g_threadPriorities[MAXIMUM_ACTIVE_THREADS];
// The number of active threads that were placed in g_suspendedThreads. Some of those entries may be NULL if they were
// important enough to not be suspended.
static uint32_t                 g_threadCount;
// The current index into the g_suspendedThreads array being returned from Platform_RtosGetFirstThread.
static uint32_t                 g_threadIndex;
// Buffer to be used for storing extra thread info.
static char                     g_threadExtraInfo[64];
// The ID of the rtx_idle thread to be skipped when providing list of threads to GDB.
static osThreadId_t             g_idleThread;

// This flag is set to a non-zero value if the DebugMon handler is to re-enable DWT watchpoints and FPB breakpoints
// after being disabled by the HardFault handler when a debug event is encounted in handler mode.
static volatile uint32_t        g_enableDWTandFPB;

// The ID of the mriMain() thread.
volatile osThreadId_t           mriThreadId;

// If non-NULL, this is the thread that we want to single step.
// If NULL, single stepping is not enabled.
// Accessed by this module and the assembly language handlers in ThreadDebug_asm.S.
volatile osThreadId_t           mriThreadSingleStepThreadId;

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
// calling the actual RTX handlers.
extern "C" void mriSVCHandlerStub(void);
extern "C" void mriPendSVHandlerStub(void);
extern "C" void mriSysTickHandlerStub(void);

// Forward Function Declarations
static __NO_RETURN void mriMain(void *pv);
static void suspendAllApplicationThreads();
static bool isThreadToIgnore(osThreadId_t thread);
static void resumeApplicationThreads();
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
static void skipNullThreadIds();
static bool isNullOrIdleThread(osThreadId_t threadId);
static const char* getThreadStateName(uint8_t threadState);
static bool isDebugThreadActive();
static void setFaultDetectedFlag();
static bool isImpreciseBusFault();
static void advancePCToNextInstruction(uint32_t excReturn, uint32_t psp, uint32_t msp);
static uint32_t* threadSP(uint32_t excReturn, uint32_t psp, uint32_t msp);
static void clearFaultStatusBits();
static void serialISRHook();
static bool isDebuggerActive();



ThreadDebug::ThreadDebug(DebugCommInterface* pCommInterface, bool breakInSetup)
{
    if (g_pComm != NULL) {
        // Only allow 1 ThreadDebug object to be initialized.
        return;
    }

    // Setup the singleton.
    g_breakInSetup = breakInSetup;
    g_pComm = pCommInterface;

    // Initialize the MRI core.
    mriInit("");

    // Start the debugger thread.
    static uint64_t             stack[MRI_THREAD_STACK_SIZE];
    static osRtxThread_t        threadTcb;
    static const osThreadAttr_t threadAttr =
    {
        .name = "mriMain",
        .attr_bits = osThreadDetached,
        .cb_mem  = &threadTcb,
        .cb_size = sizeof(threadTcb),
        .stack_mem = stack,
        .stack_size = sizeof(stack),
        .priority = osPriorityNormal
    };
    mriThreadId = osThreadNew(mriMain, NULL, &threadAttr);
    if (mriThreadId == NULL) {
        return;
    }

    callAttachFromSetup();
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
        if (Platform_IsSingleStepping()) {
            restoreRtxHandlers();
        }

        osThreadSetPriority(osThreadGetId(), osPriorityNormal);
        mriDebugException(&mriCortexMState.context);
        osThreadSetPriority(osThreadGetId(), osPriorityISR);

        if (Platform_IsSingleStepping()) {
            mriThreadSingleStepThreadId = g_haltedThreadId;
            switchRtxHandlersToDebugStubsForSingleStepping();
        }
        resumeApplicationThreads();
        g_haltedThreadId = 0;
    }
}

static void suspendAllApplicationThreads()
{
    // Suspend application threads by setting their priorities to the lowest setting, osPriorityIdle.
    // Bump rtx_idle thread up one priority level so that it will run instead of the 'suspended' application threads if
    // the debug related threads go idle.
    g_idleThread = 0;
    g_threadCount = osThreadGetCount();
    ASSERT ( g_threadCount <= ARRAY_SIZE(g_suspendedThreads) );
    osThreadEnumerate(g_suspendedThreads, ARRAY_SIZE(g_suspendedThreads));
    for (uint32_t i = 0 ; i < g_threadCount ; i++) {
        osThreadId_t thread = g_suspendedThreads[i];
        osPriority_t newPriority = osPriorityIdle;
        const char* pThreadName = osThreadGetName(thread);
        if (strcmp(pThreadName, "rtx_idle") == 0) {
            newPriority = (osPriority_t)(osPriorityIdle + 1);
            g_idleThread = thread;
        }

        if (isThreadToIgnore(thread)) {
            g_suspendedThreads[i] = 0;
        } else {
            osRtxThread_t* pThread = (osRtxThread_t*)thread;
            g_threadPriorities[i].basePriority = pThread->priority_base;
            g_threadPriorities[i].priority = pThread->priority;
            osThreadSetPriority(thread, newPriority);
        }
    }
}

static bool isThreadToIgnore(osThreadId_t thread)
{
    if (thread == mriThreadId) {
        // Don't want to suspend the debugger thread itself.
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

static void resumeApplicationThreads()
{
    for (uint32_t i = 0 ; i < g_threadCount ; i++) {
        osThreadId_t thread = g_suspendedThreads[i];
        if (thread != 0) {
            osThreadSetPriority(thread, (osPriority_t)g_threadPriorities[i].priority);
            osRtxThread_t* pThread = (osRtxThread_t*)thread;
            pThread->priority_base = g_threadPriorities[i].basePriority;
        }
    }
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

    // Return 0 to indicate that we want to halt execution at the beginning of setup() or 1 to not force a halt.
    return g_breakInSetup ? 0 : 1;
}


ThreadDebug::~ThreadDebug()
{
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




static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
                                     "<memory type=\"ram\" start=\"0x00000000\" length=\"0x10000\"> </memory>"
                                     "<memory type=\"flash\" start=\"0x08000000\" length=\"0x200000\"> <property name=\"blocksize\">0x20000</property></memory>"
                                     "<memory type=\"ram\" start=\"0x10000000\" length=\"0x48000\"> </memory>"
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
                                     "<memory type=\"ram\" start=\"0xc0000000\" length=\"0x800000\"> </memory>"
                                     "</memory-map>";

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
    g_threadIndex = 0;
    return Platform_RtosGetNextThreadId();
}

uint32_t Platform_RtosGetNextThreadId(void)
{
    skipNullThreadIds();
    if (g_threadIndex >= g_threadCount)
        return 0;
    return (uint32_t)g_suspendedThreads[g_threadIndex++];
}

static void skipNullThreadIds()
{
    while (g_threadIndex < g_threadCount && isNullOrIdleThread(g_suspendedThreads[g_threadIndex]))
        g_threadIndex++;
}

static bool isNullOrIdleThread(osThreadId_t threadId)
{
    return threadId == 0 || threadId == g_idleThread;
}

const char* Platform_RtosGetExtraThreadInfo(uint32_t threadId)
{
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
    if ((osThreadId_t)threadId == g_haltedThreadId)
        return &mriCortexMState.context;
    readThreadContext(&g_threadContext, &g_threadSP, (osThreadId_t)threadId);
    return &g_threadContext;
}

int Platform_RtosIsThreadActive(uint32_t threadId)
{
    for (uint32_t i = 0 ; i < g_threadCount ; i++) {
        if ((uint32_t)g_suspendedThreads[i] == threadId) {
            return 1;
        }
    }
    return 0;
}




// ---------------------------------------------------------------------------------------------------------------------
// Functions called from Assembly Language fault/interrupt handlers to deal with crashes and debug events.
// ---------------------------------------------------------------------------------------------------------------------
extern "C" void mriDebugMonitorHandler(uint32_t excReturn)
{
    while (!isThreadMode(excReturn)) {
        // DebugMon is running at such low priority that we should be getting ready to return to thread mode.
    }

    if (!hasEncounteredDebugEvent() && !hasControlCBeenDetected()) {
        if (g_enableDWTandFPB) {
            enableDWTandITM();
            enableFPB();
            g_enableDWTandFPB = 0;
        }
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

    // Get here when a debug event of interest has occurred in a thread.
    recordAndClearFaultStatusBits();
    stopSingleStepping();
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
    g_enableDWTandFPB = 1;
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
    disableSingleStep();
    g_debugThreadId = osThreadGetId();
    g_haltedThreadId = g_debugThreadId;
    osThreadFlagsSet(mriThreadId, MRI_THREAD_DEBUG_EVENT_FLAG);
}

static void stopSingleStepping()
{
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
    if (!isDebuggerActive() && g_pComm->readable()) {
        // Pend a halt into the debug monitor now that there is data from GDB ready to be read by it.
        setControlCFlag();
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
