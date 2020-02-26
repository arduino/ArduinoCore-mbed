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
// GDB debugging of the Arduino Portenta-H7 over a serial connection.
#include "DebugSerial.h"
extern "C" {
    #include <core/mri.h>
    #include <core/platforms.h>
    #include <architectures/armv7-m/armv7-m.h>
    #include <architectures/armv7-m/debug_cm3.h>
}

// UNDONE: Update for STM32H747XI.
static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
                                     "<memory type=\"flash\" start=\"0x08000000\" length=\"0x10000\"> <property name=\"blocksize\">0x4000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08010000\" length=\"0x10000\"> <property name=\"blocksize\">0x10000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08020000\" length=\"0xE0000\"> <property name=\"blocksize\">0x20000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08100000\" length=\"0x10000\"> <property name=\"blocksize\">0x4000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08110000\" length=\"0x10000\"> <property name=\"blocksize\">0x10000</property></memory>"
                                     "<memory type=\"flash\" start=\"0x08120000\" length=\"0xE0000\"> <property name=\"blocksize\">0x20000</property></memory>"
                                     "<memory type=\"ram\" start=\"0x20000000\" length=\"0x1C000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x2001C000\" length=\"0x4000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x20020000\" length=\"0x10000\"> </memory>"
                                     "</memory-map>";


// The singleton through which all of the Platform* APIs redirect their calls.
static DebugSerial* g_pDebugSerial = NULL;



DebugSerial::DebugSerial(HardwareSerial& serial, IRQn_Type irq) : _serial(serial), _irq(irq), _commIsr(NULL) {
    // Just return without doing anything if the singleton has already been initialized.
    // This ends up using the first initialized DebugSerial object.
    if (g_pDebugSerial != NULL) {
        return;
    }
    g_pDebugSerial = this;
    __mriInit("");
}

DebugSerial::~DebugSerial() {
    // IMPORTANT NOTE: You are attempting to destroy the connection to GDB which isn't allowed.
    //                 Don't allow your DebugSerial object to go out of scope like this.
    __debugbreak();
    for (;;) {
        // Loop forever.
    }
}
void DebugSerial::setSerialPriority(uint32_t priority) {
    NVIC_SetPriority(_irq, priority);
}

void DebugSerial::initSerial() {
    // Hook communication port ISR to allow debug monitor to be awakened when GDB sends a command.
    _commIsr = (IsrFunctionPtr) NVIC_GetVector(_irq);
    NVIC_SetVector(_irq, (uint32_t)commInterruptHook);
}

void DebugSerial::commInterruptHook(void) {
    g_pDebugSerial->pendDebugMonAndRunCommIsr();
}

void DebugSerial::pendDebugMonAndRunCommIsr(void) {
    _commIsr();
    if (_serial.available()) {
        // Pend a halt into the debug monitor if there is now data from GDB ready to be read by it.
        setMonitorPending();
    }
}

uint32_t DebugSerial::hasReceiveData(void) {
    handleAnyPendingCommInterrupts();
    return _serial.available();
}

void DebugSerial::handleAnyPendingCommInterrupts() {
    if (NVIC_GetPendingIRQ(_irq)) {
        _commIsr();
        NVIC_ClearPendingIRQ(_irq);
    }
}

int DebugSerial::receiveChar(void) {
    while (!hasReceiveData()) {
    }
    return _serial.read();
}

void DebugSerial::sendChar(int character) {
    _serial.write(character);
}




// ---------------------------------------------------------------------------------------------------------------------
// Global Platform_* functions needed by MRI to initialize and communicate with MRI.
// These functions will perform most of their work through the DebugSerial singleton.
// ---------------------------------------------------------------------------------------------------------------------
// The debugger uses these handlers to catch faults, debug events, etc.
extern "C" void __mriExceptionHandler(void);
extern "C" void __mriFaultHandler(void);

struct SystemHandlerPriorities {
    uint32_t svcallPriority;
    uint32_t pendsvPriority;
    uint32_t systickPriority;
};

// Forward Function Declarations
static SystemHandlerPriorities getSystemHandlerPrioritiesBeforeMriModifiesThem();
static void lowerPriorityOfNonDebugHandlers(const SystemHandlerPriorities* pPriorities);
static void setHandlerPriorityLowerThanDebugger(IRQn_Type irq, uint32_t origPriority);
static void switchFaultHandlersToDebugger();


extern "C" void Platform_Init(Token* pParameterTokens) {
    SystemHandlerPriorities origPriorities = getSystemHandlerPrioritiesBeforeMriModifiesThem();

    __try
        __mriCortexMInit(pParameterTokens);
    __catch
        __rethrow;

    // UNDONE: Might want to always keep the USB handler at elevated priority.
    // Set interrupt used by serial comms (UART or USB) at highest priority.
    // Set DebugMonitor interrupt at next highest priority.
    // Set all other external interrupts lower than both serial comms and DebugMonitor.
    lowerPriorityOfNonDebugHandlers(&origPriorities);
    g_pDebugSerial->setSerialPriority(0);
    NVIC_SetPriority(DebugMonitor_IRQn, 1);

    switchFaultHandlersToDebugger();

    // UNDONE: I want to finish initializing the serial port later, at a breakpoint in setup().
    g_pDebugSerial->initSerial();
}

static SystemHandlerPriorities getSystemHandlerPrioritiesBeforeMriModifiesThem() {
    SystemHandlerPriorities priorities;

    priorities.svcallPriority = NVIC_GetPriority(SVCall_IRQn);
    priorities.pendsvPriority = NVIC_GetPriority(PendSV_IRQn);
    priorities.systickPriority = NVIC_GetPriority(SysTick_IRQn);
    return priorities;
}

static void lowerPriorityOfNonDebugHandlers(const SystemHandlerPriorities* pPriorities) {
    // Set priority of system handlers that aren't directly related to debugging lower than those that are.
    setHandlerPriorityLowerThanDebugger(SVCall_IRQn, pPriorities->svcallPriority);
    setHandlerPriorityLowerThanDebugger(PendSV_IRQn, pPriorities->pendsvPriority);
    setHandlerPriorityLowerThanDebugger(SysTick_IRQn, pPriorities->systickPriority);

    // Do the same for external interrupts.
    for (int irq = WWDG_IRQn ; irq <= WAKEUP_PIN_IRQn ; irq++) {
        setHandlerPriorityLowerThanDebugger((IRQn_Type)irq, NVIC_GetPriority((IRQn_Type)irq));
    }
}

static void setHandlerPriorityLowerThanDebugger(IRQn_Type irq, uint32_t priority)
{
    // There are a total of 16 priority levels on the STM32H747XI,
    // 4 of them reserved:
    // 0 - Highest - Communication Peripheral ISR
    // 1           - DebugMon
    // 14          - SVCall
    // 15 - Lowest - PendSV & SysTick for switching context
    //
    // Everything not listed above will be lowered in priority by 2 to make room for the debugger priorities
    // except that ISRs that are already at priorities 12 & 13 will not be altered or they would conflict
    // with the 2 lowest reserved priorities.
    uint32_t highestPriority = (1 << __NVIC_PRIO_BITS) - 1;
    if (priority <= highestPriority-4) {
        priority += 2;
    }
    NVIC_SetPriority(irq, priority);
}

static void switchFaultHandlersToDebugger(void) {
    NVIC_SetVector(HardFault_IRQn,        (uint32_t)&__mriFaultHandler);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)&__mriFaultHandler);
    NVIC_SetVector(BusFault_IRQn,         (uint32_t)&__mriFaultHandler);
    NVIC_SetVector(UsageFault_IRQn,       (uint32_t)&__mriExceptionHandler);
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

extern "C" int Platform_CommCausedInterrupt(void) {
    return 0;
}

extern "C" void Platform_CommClearInterrupt(void) {
}

extern "C" int Platform_CommSharingWithApplication(void) {
    return 0;
}

extern "C" int Platform_CommShouldWaitForGdbConnect(void) {
    return 0;
}

extern "C" int Platform_CommIsWaitingForGdbToConnect(void) {
    return 0;
}

extern "C" void Platform_CommPrepareToWaitForGdbConnection(void) {
    return;
}

extern "C" void Platform_CommWaitForReceiveDataToStop(void) {
    return;
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
