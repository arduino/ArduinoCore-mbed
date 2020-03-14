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
    #include "core/platforms.h"
    #include "core/semihost.h"
    #include "core/scatter_gather.h"
    #include "architectures/armv7-m/armv7-m.h"
    #include "architectures/armv7-m/debug_cm3.h"
}

// UNDONE: Switch back to the USB/CDC based serial port.
#undef Serial
#define Serial Serial1

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

// Bit location in PSR which indicates if the stack needed to be 8-byte aligned or not.
#define PSR_STACK_ALIGN_SHIFT   9

// Bit in LR set to 0 when automatic stacking of floating point registers occurs during exception handling.
#define LR_FLOAT_STACK          (1 << 4)

// Thread syncronization flag used to indicate that a debug event has occured.
#define MRI_THREAD_DEBUG_EVENT_FLAG (1 << 0)

osThreadId_t            g_mriThreadId;

volatile osThreadId_t   g_haltingThreadId;

// UNDONE: Get rid of.
volatile osThreadId_t   g_test;

osThreadId_t            g_threads[64];
uint32_t                g_threadCount;

volatile osThreadId_t   mriThreadSingleStepThreadId;

volatile uint32_t       mriThreadEnableDWTandFPB;


ThreadMRI::ThreadMRI()
{
    mriInit("");
}

// Main entry point into MRI debugger core.
extern "C" void mriDebugException(void);

// UNDONE: Could push into debugException() API.
static __NO_RETURN void mriMain(void *pv);

static uint64_t      g_stack[64];
static osRtxThread_t g_threadTcb;
static const osThreadAttr_t g_threadAttr =
{
    .name = "mriMain",
    .attr_bits = osThreadDetached,
    .cb_mem  = &g_threadTcb,
    .cb_size = sizeof(g_threadTcb),
    .stack_mem = g_stack,
    .stack_size = sizeof(g_stack),
    .priority = osPriorityNormal
};


void ThreadMRI::debugException()
{
    g_mriThreadId = osThreadNew(mriMain, NULL, &g_threadAttr);
}

static void suspendAllApplicationThreads() {
    // UNDONE: Handle the case where threadCount is larger than threads[] array.
    g_threadCount = osThreadEnumerate(g_threads, sizeof(g_threads)/sizeof(g_threads[0]));
    for (uint32_t i = 0 ; i < g_threadCount ; i++) {
        osThreadId_t thread = g_threads[i];
        const char*  pThreadName = osThreadGetName(thread);

        if (thread != g_mriThreadId && strcmp(pThreadName, "rtx_idle") != 0) {
            osThreadSuspend(thread);
        }
        else {
            g_threads[i] = 0;
        }
    }
}

static void resumeApplicationThreads() {
    for (uint32_t i = 0 ; i < g_threadCount ; i++) {
        osThreadId_t thread = g_threads[i];
        if (thread != 0) {
            osThreadResume(thread);
        }
    }
}

static void readThreadContext(osThreadId_t thread) {
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

static __NO_RETURN void mriMain(void *pv)
{
    // Run the code which suspends, resumes, etc the other threads at highest priority so that it doesn't context
    // switch to one of the other threads. Switch to normal priority when running mriDebugException() though.
    osThreadSetPriority(osThreadGetId(), osPriorityRealtime7);

    while (1) {
        int waitResult = osThreadFlagsWait(MRI_THREAD_DEBUG_EVENT_FLAG, osFlagsWaitAny, osWaitForever);
        while (waitResult < 0) {
            // Something bad has happened if we hang here.
        }
        suspendAllApplicationThreads();

        while (g_haltingThreadId == 0) {
            // Something bad has happened if we hang here.
        }
        readThreadContext(g_haltingThreadId);

        osThreadSetPriority(osThreadGetId(), osPriorityNormal);
        mriDebugException();
        osThreadSetPriority(osThreadGetId(), osPriorityRealtime7);

        if (Platform_IsSingleStepping()) {
            mriThreadSingleStepThreadId = g_haltingThreadId;
            osThreadResume(mriThreadSingleStepThreadId);
        } else {
            resumeApplicationThreads();
        }
        g_haltingThreadId = 0;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Global Platform_* functions needed by MRI to initialize and communicate with MRI.
// These functions will perform most of their work through the DebugSerial singleton.
// ---------------------------------------------------------------------------------------------------------------------
// Assembly Language fault handling stubs. They do some preprocessing and then call the C handler below if appropriate.
extern "C" void mriDebugMonitorHandlerStub(void);
extern "C" void mriFaultHandlerStub(void);
// Assembly Language stubs for RTX context switching routines. They check to see if DebugMon should be pended before
// calling the actual RTX handlers.
extern "C" void mriSVCHandlerStub(void);
extern "C" void mriPendSVHandlerStub(void);
extern "C" void mriSysTickHandlerStub(void);



// Forward Function Declarations
static bool hasEncounteredDebugEvent();
static void recordAndClearFaultStatusBits();
static void wakeMriMainToDebugCurrentThread();
static void stopSingleStepping();
static void switchFaultHandlersToDebugger();
static void switchRtxHandlersToDebugStubs();


extern "C" void mriDebugMonitorHandler(uint32_t excReturn)
{
    excReturn &= 0xF;
    bool isThreadMode = (excReturn == 0xD || excReturn == 0x9);
    while (!isThreadMode) {
        // DebugMon is running at such low priority that we should be getting ready to return to thread mode.
    }

    if (!hasEncounteredDebugEvent()) {
        if (mriThreadEnableDWTandFPB) {
            enableDWTandITM();
            enableFPB();
            mriThreadEnableDWTandFPB = 0;
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
extern "C" void mriFaultHandler(uint32_t excReturn)
{
    excReturn &= 0xF;
    bool isThreadMode = (excReturn == 0xD || excReturn == 0x9);
    if (isThreadMode) {
        recordAndClearFaultStatusBits();
        stopSingleStepping();
        wakeMriMainToDebugCurrentThread();
        return;
    }

    // The asm stub calling this function has already verified that a debug event during handler mode caused
    // this fault.
    mriThreadEnableDWTandFPB = 1;
    SCB->DFSR = SCB->DFSR;
    SCB->HFSR = SCB_HFSR_DEBUGEVT_Msk;
    disableDWTandITM();
    disableFPB();
    setMonitorPending();
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
    g_test = osThreadGetId();
    g_haltingThreadId = g_test; // UNDONE: osThreadGetId();
    osThreadFlagsSet(g_mriThreadId, MRI_THREAD_DEBUG_EVENT_FLAG);
}

static void stopSingleStepping()
{
    disableSingleStep();
    mriThreadSingleStepThreadId = NULL;
}

void Platform_Init(Token* pParameterTokens)
{
    __try
        mriCortexMInit((Token*)pParameterTokens);
    __catch
        __rethrow;

    mriThreadEnableDWTandFPB = 0;
    mriThreadSingleStepThreadId = NULL;
    switchFaultHandlersToDebugger();
    switchRtxHandlersToDebugStubs();
}

static void switchFaultHandlersToDebugger(void) {
    NVIC_SetVector(HardFault_IRQn,        (uint32_t)mriFaultHandlerStub);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)mriFaultHandlerStub);
    NVIC_SetVector(BusFault_IRQn,         (uint32_t)mriFaultHandlerStub);
    NVIC_SetVector(UsageFault_IRQn,       (uint32_t)mriFaultHandlerStub);
    NVIC_SetVector(DebugMonitor_IRQn,     (uint32_t)mriDebugMonitorHandlerStub);
}

static void switchRtxHandlersToDebugStubs(void) {
    NVIC_SetVector(SVCall_IRQn,        (uint32_t)mriSVCHandlerStub);
    NVIC_SetVector(PendSV_IRQn, (uint32_t)mriPendSVHandlerStub);
    NVIC_SetVector(SysTick_IRQn,         (uint32_t)mriSysTickHandlerStub);
}


uint32_t Platform_CommHasReceiveData(void)
{
    return Serial.available();
}

int Platform_CommReceiveChar(void) {
    while (!Serial.available())
    {
        // Busy wait.
    }
    return Serial.read();
}

void Platform_CommSendChar(int character) {
    Serial.write(character);
}

int Platform_CommCausedInterrupt(void) {
    return 0;
}

void Platform_CommClearInterrupt(void) {
}

uint32_t Platform_GetDeviceMemoryMapXmlSize(void) {
    return sizeof(g_memoryMapXml) - 1;
}

const char* Platform_GetDeviceMemoryMapXml(void) {
    return g_memoryMapXml;
}


const uint8_t* Platform_GetUid(void) {
    return NULL;
}

uint32_t Platform_GetUidSize(void) {
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
