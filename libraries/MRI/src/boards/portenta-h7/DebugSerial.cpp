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
// GDB Kernel debugging of the Arduino Portenta-H7 over a serial UART connection.
#include "DebugSerial.h"
extern "C" {
    #include <core/mri.h>
    #include <core/core.h>
    #include <core/platforms.h>
    #include <core/semihost.h>
    #include <architectures/armv7-m/armv7-m.h>
    #include <architectures/armv7-m/debug_cm3.h>
}


// The number of milliseconds to pause at the beginning of setup() to give time for host to enumerate USB device.
#define STARTUP_DELAY_MSEC 250

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


// The singleton through which all of the Platform* APIs redirect their calls.
static DebugSerial* g_pDebugSerial = NULL;

// Will be setting initial breakpoint on setup() routine.
void setup();

// The debugger uses this handler to catch faults, debug events, etc.
extern "C" void mriExceptionHandler(void);


DebugSerial::DebugSerial(PinName txPin, PinName rxPin, IRQn_Type irq, uint32_t baudRate, bool breakInSetup /*=true*/) :
    _serial(txPin, rxPin, baudRate), _irq(irq), _breakInSetup(breakInSetup)
{
    // Just return without doing anything if the singleton has already been initialized.
    // This ends up using the first initialized DebugSerial object.
    if (g_pDebugSerial != NULL) {
        return;
    }
    g_pDebugSerial = this;

    mriInit("");

    setupStopInSetup();
}

void DebugSerial::setupStopInSetup() {
    mriCore_SetTempBreakpoint((uint32_t)setup, justEnteredSetupCallback, NULL);
}

int DebugSerial::justEnteredSetupCallback(void* pv){
    return g_pDebugSerial->justEnteredSetup();
}

int DebugSerial::justEnteredSetup() {
    initSerial();

    // Return 0 to indicate that we want to halt execution at the beginning of setup() or 1 to not force a halt.
    return _breakInSetup ? 0 : 1;
}


DebugSerial::~DebugSerial() {
    // IMPORTANT NOTE: You are attempting to destroy the connection to GDB which isn't allowed.
    //                 Don't allow your DebugSerial object to go out of scope like this.
    __debugbreak();
    for (;;) {
        // Loop forever.
    }
}

void DebugSerial::setSerialPriority(uint8_t priority) {
    mriCortexMSetPriority(_irq, priority, 0);
}

void DebugSerial::initSerial() {

    // Hook communication port ISR to allow debug monitor to be awakened when GDB sends a command.
    _serial.attach(mriExceptionHandler);
    setSerialPriority(2);
    NVIC_SetVector(_irq, (uint32_t)mriExceptionHandler);
}

uint32_t DebugSerial::hasReceiveData(void) {
    return _serial.readable();
}

int DebugSerial::receiveChar(void) {
    while (!hasReceiveData()) {
    }
    uint8_t byte;
    _serial.read(&byte, 1);
    return byte;
}

void DebugSerial::sendChar(int character) {
    _serial.write(&character, 1);
}




// ---------------------------------------------------------------------------------------------------------------------
// Global Platform_* functions needed by MRI to initialize and communicate with MRI.
// These functions will perform most of their work through the DebugSerial singleton.
// ---------------------------------------------------------------------------------------------------------------------
struct SystemHandlerPriorities {
    uint8_t svcallPriority;
    uint8_t pendsvPriority;
    uint8_t systickPriority;
};

// Forward Function Declarations
static SystemHandlerPriorities getSystemHandlerPrioritiesBeforeMriModifiesThem();
static void restoreSystemHandlerPriorities(const SystemHandlerPriorities* pPriorities);
static void switchFaultHandlersToDebugger();


extern "C" void Platform_Init(Token* pParameterTokens) {
    SystemHandlerPriorities origPriorities = getSystemHandlerPrioritiesBeforeMriModifiesThem();
    uint8_t                 debugMonPriority = 2;

    __try
        mriCortexMInit((Token*)pParameterTokens, debugMonPriority, WAKEUP_PIN_IRQn);
    __catch
        __rethrow;

    // USB defaults to a priority of 1, keep that as highest priority interrupt so that it can respond to PC requests
    // while kernel debugging.
    // Set interrupts used by UART serial comms and DebugMonitor at next highest priority.
    // Set all other external interrupts lower than both serial comms and DebugMonitor.
    restoreSystemHandlerPriorities(&origPriorities);

    switchFaultHandlersToDebugger();
}

static SystemHandlerPriorities getSystemHandlerPrioritiesBeforeMriModifiesThem() {
    SystemHandlerPriorities priorities;

    priorities.svcallPriority = NVIC_GetPriority(SVCall_IRQn);
    priorities.pendsvPriority = NVIC_GetPriority(PendSV_IRQn);
    priorities.systickPriority = NVIC_GetPriority(SysTick_IRQn);
    return priorities;
}

static void restoreSystemHandlerPriorities(const SystemHandlerPriorities* pPriorities) {
    // Set priority of system handlers that aren't directly related to debugging lower than those that are.
    NVIC_SetPriority(SVCall_IRQn, pPriorities->svcallPriority);
    NVIC_SetPriority(PendSV_IRQn, pPriorities->pendsvPriority);
    NVIC_SetPriority(SysTick_IRQn, pPriorities->systickPriority);
}

static void switchFaultHandlersToDebugger(void) {
    NVIC_SetVector(HardFault_IRQn,        (uint32_t)&mriExceptionHandler);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)&mriExceptionHandler);
    NVIC_SetVector(BusFault_IRQn,         (uint32_t)&mriExceptionHandler);
    NVIC_SetVector(UsageFault_IRQn,       (uint32_t)&mriExceptionHandler);
}


extern "C" uint32_t Platform_CommHasReceiveData(void) {
    return g_pDebugSerial->hasReceiveData();
}

extern "C" int Platform_CommReceiveChar(void) {
    return g_pDebugSerial->receiveChar();
}

extern "C" void Platform_CommSendChar(int character) {
    g_pDebugSerial->sendChar(character);
}


extern "C" uint32_t Platform_GetDeviceMemoryMapXmlSize(void) {
    return sizeof(g_memoryMapXml) - 1;
}

extern "C" const char* Platform_GetDeviceMemoryMapXml(void) {
    return g_memoryMapXml;
}


extern "C" const uint8_t* Platform_GetUid(void) {
    return NULL;
}

extern "C" uint32_t Platform_GetUidSize(void) {
    return 0;
}

extern "C" int Semihost_IsDebuggeeMakingSemihostCall(void)
{
    return 0;
}

int Semihost_HandleSemihostRequest(void)
{
    return 0;
}
