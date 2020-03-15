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
// Library to enable GDB debugging of Thread Mode code running on the Adruino Portenta H7 over
// the USB serial connection.
#include <Arduino.h>
#include "ThreadMRI.h"
#include <cmsis_os2.h>
#include <rtx_os.h>
extern "C"
{
    #include "core/mri.h"
    #include "core/core.h"
    #include "core/platforms.h"
    #include "core/semihost.h"
    #include "core/scatter_gather.h"
    #include "architectures/armv7-m/armv7-m.h"
    #include "architectures/armv7-m/debug_cm3.h"
}

// UNDONE: Switch back to the USB/CDC based serial port.
#undef Serial
#define Serial Serial1


// Configuration Parameters
// The size of the mriThread() stack in uint64_t objects.
#define MRI_THREAD_STACK_SIZE   64
// The maximum number of active threads that can be handled by the debugger.
#define MAXIMUM_ACTIVE_THREADS  64




// Bit location in PSR which indicates if the stack needed to be 8-byte aligned or not.
#define PSR_STACK_ALIGN_SHIFT   9

// Bit in LR set to 0 when automatic stacking of floating point registers occurs during exception handling.
#define LR_FLOAT_STACK          (1 << 4)

// Thread syncronization flag used to indicate that a debug event has occured.
#define MRI_THREAD_DEBUG_EVENT_FLAG (1 << 0)

// Lower nibble of EXC_RETURN in LR will have one of these values if interrupted code was running in thread mode.
//  Using PSP.
#define EXC_RETURN_THREADMODE_PROCESSSTACK  0xD
//  Using MSP.
#define EXC_RETURN_THREADMODE_MAINSTACK     0x9


// Assert routine that will dump error text to GDB connection before entering infinite loop. If user is logging MRI
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




// Flag to verify that only one ThreadMRI object has been initialized.
static bool                     g_alreadyInitialized;

// The ID of the halted thread being debugged.
static volatile osThreadId_t    g_haltedThreadId;

// The list of threads which were suspended upon entry into the debugger. These are the threads that will be resumed
// upon exit from the debugger.
static osThreadId_t             g_suspendedThreads[MAXIMUM_ACTIVE_THREADS];
// The number of active threads that were placed in g_suspendedThreads. Some of those entries may be NULL if they were important
// enough to not be suspended.
static uint32_t                 g_threadCount;

// This flag is set to a non-zero value if the DebugMon handler is to re-enable DWT watchpoints and FPB breakpoints
// after being disabled by the HardFault handler when a debug event is encounted in handler mode.
static volatile uint32_t        g_enableDWTandFPB;

// The ID of the mriMain() thread.
volatile osThreadId_t           mriThreadId;

// If non-NULL, this is the thread that we want to single step.
// If NULL, single stepping is not enabled.
// Accessed by this module and the assembly language handlers in ThreadMRI_asm.S.
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
static void resumeApplicationThreads();
static void readThreadContext(osThreadId_t thread);
static void switchRtxHandlersToDebugStubsForSingleStepping();
static void restoreRtxHandlers();
static void setDebugActiveFlag();
static void clearDebugActiveFlag();
static bool isThreadMode(uint32_t excReturn);
static bool hasEncounteredDebugEvent();
static void recordAndClearFaultStatusBits();
static void wakeMriMainToDebugCurrentThread();
static void stopSingleStepping();
static void recordAndSwitchFaultHandlersToDebugger();
static bool isDebugThreadActive();
static void setFaultDetectedFlag();
static bool isImpreciseBusFault();
static void advancePCToNextInstruction(uint32_t excReturn, uint32_t psp, uint32_t msp);
static uint32_t* threadSP(uint32_t excReturn, uint32_t psp, uint32_t msp);
static bool isInstruction32Bit(uint16_t firstWordOfInstruction);




ThreadMRI::ThreadMRI()
{
    mriInit("");
}


bool ThreadMRI::begin()
{
    if (g_alreadyInitialized) {
        // Only allow 1 ThreadMRI object to be initialized.
        return false;
    }

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
        return false;
    }
    g_alreadyInitialized = true;
    return true;
}

static __NO_RETURN void mriMain(void *pv)
{
    // Run the code which suspends, resumes, etc the other threads at highest priority so that it doesn't context
    // switch to one of the other threads. Switch to normal priority when running mriDebugException() though.
    osThreadSetPriority(osThreadGetId(), osPriorityRealtime7);

    while (1) {
        int waitResult = osThreadFlagsWait(MRI_THREAD_DEBUG_EVENT_FLAG, osFlagsWaitAny, osWaitForever);
        ASSERT ( waitResult > 0 );
        ASSERT ( g_haltedThreadId != 0 );
        suspendAllApplicationThreads();
        readThreadContext(g_haltedThreadId);
        if (Platform_IsSingleStepping()) {
            restoreRtxHandlers();
        }
        setDebugActiveFlag();

        osThreadSetPriority(osThreadGetId(), osPriorityNormal);
        mriDebugException();
        osThreadSetPriority(osThreadGetId(), osPriorityRealtime7);

        clearDebugActiveFlag();
        if (Platform_IsSingleStepping()) {
            mriThreadSingleStepThreadId = g_haltedThreadId;
            switchRtxHandlersToDebugStubsForSingleStepping();
            osThreadResume(mriThreadSingleStepThreadId);
        } else {
            resumeApplicationThreads();
        }
        g_haltedThreadId = 0;
    }
}

static void suspendAllApplicationThreads()
{
    g_threadCount = osThreadGetCount();
    ASSERT ( g_threadCount <= sizeof(g_suspendedThreads)/sizeof(g_suspendedThreads[0]) );
    osThreadEnumerate(g_suspendedThreads, sizeof(g_suspendedThreads)/sizeof(g_suspendedThreads[0]));
    for (uint32_t i = 0 ; i < g_threadCount ; i++) {
        osThreadId_t thread = g_suspendedThreads[i];
        const char*  pThreadName = osThreadGetName(thread);

        if (thread != mriThreadId && strcmp(pThreadName, "rtx_idle") != 0) {
            osThreadSuspend(thread);
        }
        else {
            g_suspendedThreads[i] = 0;
        }
    }
}

static void resumeApplicationThreads()
{
    for (uint32_t i = 0 ; i < g_threadCount ; i++) {
        osThreadId_t thread = g_suspendedThreads[i];
        if (thread != 0) {
            osThreadResume(thread);
        }
    }
}

static void readThreadContext(osThreadId_t thread)
{
    osRtxThread_t* pThread = (osRtxThread_t*)thread;

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
    mriCortexMState.contextEntries[0].pValues = pThreadContext + offset + 8;
    mriCortexMState.contextEntries[0].count = 4;
    // R4 - R11
    mriCortexMState.contextEntries[1].pValues = pThreadContext + offset + 0;
    mriCortexMState.contextEntries[1].count = 8;
    // R12
    mriCortexMState.contextEntries[2].pValues = pThreadContext + offset + 12;
    mriCortexMState.contextEntries[2].count = 1;
    // SP - Point scatter gather context to correct location for SP but set it to correct value once CPSR is more easily
    // fetched.
    mriCortexMState.contextEntries[3].pValues = &mriCortexMState.sp;
    mriCortexMState.contextEntries[3].count = 1;
    // LR, PC, CPSR
    mriCortexMState.contextEntries[4].pValues = pThreadContext + offset + 13;
    mriCortexMState.contextEntries[4].count = 3;
    // Set SP to correct value using alignment bit in CPSR. Memory for SP is already tracked by context.
    uint32_t cpsr = ScatterGather_Get(&mriCortexMState.context, CPSR);
    mriCortexMState.sp = pThread->sp + sizeof(uint32_t) * (stackedCount + ((cpsr >> PSR_STACK_ALIGN_SHIFT) & 1));

    if (offset != 0) {
        // S0 - S15
        mriCortexMState.contextEntries[5].pValues = pThreadContext + 32;
        mriCortexMState.contextEntries[5].count = 16;
        // S16 - S31
        mriCortexMState.contextEntries[6].pValues = pThreadContext + 0;
        mriCortexMState.contextEntries[6].count = 16;
        // FPSCR
        mriCortexMState.contextEntries[7].pValues = pThreadContext + 48;
        mriCortexMState.contextEntries[7].count = 1;
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

static void setDebugActiveFlag()
{
    mriCortexMState.flags |= CORTEXM_FLAGS_ACTIVE_DEBUG;
}

static void clearDebugActiveFlag()
{
    mriCortexMState.flags &= ~CORTEXM_FLAGS_ACTIVE_DEBUG;
}



// ---------------------------------------------------------------------------------------------------------------------
// Global Platform_* functions needed by MRI to initialize and communicate with MRI.
// These functions will perform most of their work through the DebugSerial singleton.
// ---------------------------------------------------------------------------------------------------------------------
void Platform_Init(Token* pParameterTokens)
{
    __try
        mriCortexMInit((Token*)pParameterTokens);
    __catch
        __rethrow;

    g_enableDWTandFPB = 0;
    mriThreadSingleStepThreadId = NULL;
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
    return Serial.available();
}

int Platform_CommReceiveChar(void)
{
    while (!Serial.available()) {
        // Busy wait.
    }
    return Serial.read();
}

void Platform_CommSendChar(int character)
{
    Serial.write(character);
}

int Platform_CommCausedInterrupt(void)
{
    return 0;
}

void Platform_CommClearInterrupt(void)
{
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





// ---------------------------------------------------------------------------------------------------------------------
// Functions called from Assembly Language fault/interrupt handlers to deal with crashes and debug events.
// ---------------------------------------------------------------------------------------------------------------------
extern "C" void mriDebugMonitorHandler(uint32_t excReturn)
{
    while (!isThreadMode(excReturn)) {
        // DebugMon is running at such low priority that we should be getting ready to return to thread mode.
    }

    if (!hasEncounteredDebugEvent()) {
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
    g_haltedThreadId = g_debugThreadId; // UNDONE: osThreadGetId();
    osThreadFlagsSet(mriThreadId, MRI_THREAD_DEBUG_EVENT_FLAG);
}

static void stopSingleStepping()
{
    disableSingleStep();
    mriThreadSingleStepThreadId = NULL;
}

static bool isDebugThreadActive()
{
    return (mriCortexMState.flags & CORTEXM_FLAGS_ACTIVE_DEBUG) && mriThreadId == osThreadGetId();
}

static void setFaultDetectedFlag()
{
    mriCortexMState.flags |= CORTEXM_FLAGS_FAULT_DURING_DEBUG;
}

static bool isImpreciseBusFault()
{
    return SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk;
}

static void advancePCToNextInstruction(uint32_t excReturn, uint32_t psp, uint32_t msp)
{
    uint32_t* pSP = threadSP(excReturn, psp, msp);
    uint32_t* pPC = pSP + 7;
    uint16_t  currentInstruction = *(uint16_t*)pPC;
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


static bool isInstruction32Bit(uint16_t firstWordOfInstruction)
{
    uint16_t maskedOffUpper5BitsOfWord = firstWordOfInstruction & 0xF800;

    // 32-bit instructions start with 0b11101, 0b11110, 0b11111 according to page A5-152 of the
    // ARMv7-M Architecture Manual.
    return  (maskedOffUpper5BitsOfWord == 0xE800 ||
             maskedOffUpper5BitsOfWord == 0xF000 ||
             maskedOffUpper5BitsOfWord == 0xF800);
}
