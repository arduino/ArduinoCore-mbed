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
extern "C"
{
    #include "core/mri.h"
    #include "core/platforms.h"
    #include "core/semihost.h"
    #include "core/scatter_gather.h"
    #include "architectures/armv7-m/armv7-m.h"
    #include "architectures/armv7-m/debug_cm3.h"
}
// UNDONE: Might not need this.
#include <signal.h>
#include <string.h>

#include <cmsis_os2.h>
#include <rtx_os.h>

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

// Thread flag used to indicate that a debug event has occured.
#define MRI_THREAD_DEBUG_EVENT_FLAG (1 << 0)

osThreadId_t            g_mriThreadId;

volatile osThreadId_t   g_haltingThreadId;

osThreadId_t            g_threads[64];
uint32_t                g_threadCount;


ThreadMRI::ThreadMRI()
{
    mriInit("");
}

// UNDONE: This is just for initial bringup.
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

        if (thread != g_mriThreadId) {
            osThreadSuspend(thread);
        } else {
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
    while (1) {
        // Wait for next debug event to occur. Set priority to highest level before blocking so that it can safely
        // put all of the other application threads to sleep upon wakeup.
        osThreadSetPriority(osThreadGetId(), osPriorityRealtime7);
            osThreadFlagsWait(MRI_THREAD_DEBUG_EVENT_FLAG, osFlagsWaitAny, osWaitForever);
            suspendAllApplicationThreads();
        osThreadSetPriority(osThreadGetId(), osPriorityNormal);

        if (g_haltingThreadId == 0) {
            continue;
        }
        readThreadContext(g_haltingThreadId);
        mriDebugException();
        resumeApplicationThreads();
        g_haltingThreadId = 0;
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Global Platform_* functions needed by MRI to initialize and communicate with MRI.
// These functions will perform most of their work through the DebugSerial singleton.
// ---------------------------------------------------------------------------------------------------------------------
// Forward Function Declarations
static void switchFaultHandlersToDebugger();

static void mriDebugMonitorHandler(void)
{
    // Record information about cause of exception/debug event.
    mriCortexMState.exceptionNumber = getCurrentlyExecutingExceptionNumber();
    mriCortexMState.dfsr = SCB->DFSR;
    mriCortexMState.hfsr = SCB->HFSR;
    mriCortexMState.cfsr = SCB->CFSR;
    mriCortexMState.mmfar = SCB->MMFAR;
    mriCortexMState.bfar = SCB->BFAR;

    // Clear the debug event bits as they are already recorded.
    if (mriCortexMState.exceptionNumber == 12) {
        SCB->DFSR = mriCortexMState.dfsr;
    }

    g_haltingThreadId = osThreadGetId();
    osThreadFlagsSet(g_mriThreadId, MRI_THREAD_DEBUG_EVENT_FLAG);
}

static void mriFaultHandler(void)
{
    // UNDONE: Differentiate.
    mriDebugMonitorHandler();
}


void Platform_Init(Token* pParameterTokens)
{
    __try
        mriCortexMInit((Token*)pParameterTokens);
    __catch
        __rethrow;

    switchFaultHandlersToDebugger();
    NVIC_SetVector(DebugMonitor_IRQn, (uint32_t)mriDebugMonitorHandler);
}

static void switchFaultHandlersToDebugger(void) {
    NVIC_SetVector(HardFault_IRQn,        (uint32_t)mriFaultHandler);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)mriFaultHandler);
    NVIC_SetVector(BusFault_IRQn,         (uint32_t)mriFaultHandler);
    NVIC_SetVector(UsageFault_IRQn,       (uint32_t)mriFaultHandler);
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
