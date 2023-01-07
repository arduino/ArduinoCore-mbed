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
/* Routines to expose the Cortex-M functionality to the mri debugger. */
#include <core/libc.h>
#include <core/signal.h>
#include <core/core.h>
#include <core/platforms.h>
#include <core/gdb_console.h>
#include <semihost/newlib/newlib_stubs.h>
#include "debug_cm3.h"
#include "armv7-m.h"

/* Bits in CFSR which indicate that stacking/unstacking fault has occurred during exception entry/exit. */
#define CFSR_STACK_ERROR_BITS               0x00001818


/* Fake stack used when task encounters stacking/unstacking fault. */
static const uint32_t  g_fakeStack[] = { 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD,
                                         0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD,
#if MRI_DEVICE_HAS_FPU
                                         0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD,
                                         0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD,
                                         0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD,
                                         0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD, 0xDEADDEAD,
                                         0xDEADDEAD, 0xDEADDEAD
#endif
                                        };
uint64_t            mriCortexMDebuggerStack[CORTEXM_DEBUGGER_STACK_SIZE];
volatile uint32_t   mriCortexMFlags;
CortexMState        mriCortexMState;

/* NOTE: This is the original version of the following XML which has had things stripped to reduce the amount of
         FLASH consumed by the debug monitor.  This includes the removal of the copyright comment.
<?xml version="1.0"?>
<!-- Copyright (C) 2010, 2011 Free Software Foundation, Inc.

     Copying and distribution of this file, with or without modification,
     are permitted in any medium without royalty provided the copyright
     notice and this notice are preserved.  -->

<!DOCTYPE feature SYSTEM "gdb-target.dtd">
<feature name="org.gnu.gdb.arm.m-profile">
  <reg name="r0" bitsize="32"/>
  <reg name="r1" bitsize="32"/>
  <reg name="r2" bitsize="32"/>
  <reg name="r3" bitsize="32"/>
  <reg name="r4" bitsize="32"/>
  <reg name="r5" bitsize="32"/>
  <reg name="r6" bitsize="32"/>
  <reg name="r7" bitsize="32"/>
  <reg name="r8" bitsize="32"/>
  <reg name="r9" bitsize="32"/>
  <reg name="r10" bitsize="32"/>
  <reg name="r11" bitsize="32"/>
  <reg name="r12" bitsize="32"/>
  <reg name="sp" bitsize="32" type="data_ptr"/>
  <reg name="lr" bitsize="32"/>
  <reg name="pc" bitsize="32" type="code_ptr"/>
  <reg name="xpsr" bitsize="32" regnum="25"/>
</feature>
*/
static const char g_targetXml[] =
    "<?xml version=\"1.0\"?>\n"
    "<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">\n"
    "<target>\n"
    "<feature name=\"org.gnu.gdb.arm.m-profile\">\n"
    "<reg name=\"r0\" bitsize=\"32\"/>\n"
    "<reg name=\"r1\" bitsize=\"32\"/>\n"
    "<reg name=\"r2\" bitsize=\"32\"/>\n"
    "<reg name=\"r3\" bitsize=\"32\"/>\n"
    "<reg name=\"r4\" bitsize=\"32\"/>\n"
    "<reg name=\"r5\" bitsize=\"32\"/>\n"
    "<reg name=\"r6\" bitsize=\"32\"/>\n"
    "<reg name=\"r7\" bitsize=\"32\"/>\n"
    "<reg name=\"r8\" bitsize=\"32\"/>\n"
    "<reg name=\"r9\" bitsize=\"32\"/>\n"
    "<reg name=\"r10\" bitsize=\"32\"/>\n"
    "<reg name=\"r11\" bitsize=\"32\"/>\n"
    "<reg name=\"r12\" bitsize=\"32\"/>\n"
    "<reg name=\"sp\" bitsize=\"32\" type=\"data_ptr\"/>\n"
    "<reg name=\"lr\" bitsize=\"32\" type=\"code_ptr\"/>\n"
    "<reg name=\"pc\" bitsize=\"32\" type=\"code_ptr\"/>\n"
    "<reg name=\"xpsr\" bitsize=\"32\" regnum=\"25\"/>\n"
    "</feature>\n"
#if !MRI_THREAD_MRI
    "<feature name=\"org.gnu.gdb.arm.m-system\">\n"
    "<reg name=\"msp\" bitsize=\"32\" regnum=\"26\"/>\n"
    "<reg name=\"psp\" bitsize=\"32\" regnum=\"27\"/>\n"
    "<reg name=\"primask\" bitsize=\"32\" regnum=\"28\"/>\n"
    "<reg name=\"basepri\" bitsize=\"32\" regnum=\"29\"/>\n"
    "<reg name=\"faultmask\" bitsize=\"32\" regnum=\"30\"/>\n"
    "<reg name=\"control\" bitsize=\"32\" regnum=\"31\"/>\n"
    "</feature>\n"
#endif
#if MRI_DEVICE_HAS_FPU
    "<feature name=\"org.gnu.gdb.arm.vfp\">\n"
    "<reg name=\"d0\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d1\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d2\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d3\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d4\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d5\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d6\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d7\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d8\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d9\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d10\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d11\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d12\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d13\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d14\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"d15\" bitsize=\"64\" type=\"ieee_double\"/>\n"
    "<reg name=\"fpscr\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
    "</feature>\n"
#endif
    "</target>\n";

/* Reference this handler in the ASM module to make sure that it gets linked in. */
void mriExceptionHandler(void);


static void fillDebuggerStack(void);
static void clearState(void);
static void determinePriorityBitShift(void);
static void configureDWTandFPB(void);
static void defaultSvcAndSysTickInterruptsToLowerPriority(uint8_t priority);
static void defaultExternalInterruptsToLowerPriority(uint8_t priority, IRQn_Type highestExternalIrq);
static void disableDebuggerStack(void);
static void enableDebugMonitorAtSpecifiedPriority(uint8_t priority);
void mriCortexMInit(Token* pParameterTokens, uint8_t debugMonPriority, IRQn_Type highestExternalIrq)
{
    if (MRI_THREAD_MRI)
    {
        /* Always set DebugMon interrupt priority to lowest level, 255, when using threaded MRI. */
        debugMonPriority = 255;
    }
    else
    {
        /* Reference routine in ASM module to make sure that is gets linked in. */
        void (* volatile dummyReference)(void) = mriExceptionHandler;
        (void)dummyReference;
    }
    (void)pParameterTokens;

    fillDebuggerStack();
    clearState();
    determinePriorityBitShift();
    configureDWTandFPB();
    if (debugMonPriority == 0)
    {
        defaultSvcAndSysTickInterruptsToLowerPriority(debugMonPriority+1);
        defaultExternalInterruptsToLowerPriority(debugMonPriority+1, highestExternalIrq);
    }
    else
    {
        /* When not running MRI at highest priority, the user is responsible for the priority level of all the */
        /* other interrupts in the system. Only the user knows what they want to be able to debug and what should */
        /* run in the background while the debugger is active. */
        /* Don't want to change MSP to point to special debugger stack during debug exceptions if higher priority */
        /* interrupts exist that can run while stopped in debugger. */
        disableDebuggerStack();
    }
    Platform_DisableSingleStep();
    clearMonitorPending();
    enableDebugMonitorAtSpecifiedPriority(debugMonPriority);
}

static void fillDebuggerStack(void)
{
    uint64_t fillValue = ((uint64_t)CORTEXM_DEBUGGER_STACK_FILL << 32) | (uint64_t)CORTEXM_DEBUGGER_STACK_FILL;
    size_t i;

    for (i = 0 ; i < sizeof(mriCortexMDebuggerStack)/sizeof(mriCortexMDebuggerStack[0]) ; i++)
        mriCortexMDebuggerStack[i] = fillValue;
}

static void clearState(void)
{
    mri_memset(&mriCortexMState, 0, sizeof(mriCortexMState));
}

/* Cortex-M7 microcontrollers name the SHP priority registers SHPR unlike other ARMv7-M devices. */
#if defined(__CORTEX_M) && (__CORTEX_M == 7U)
#define SHP SHPR
#endif

static void determinePriorityBitShift(void)
{
    const uint32_t debugMonExceptionNumber = 12;
    uint32_t zeroBitCount;
    uint32_t priorityBitShift;

    /* Setting DebugMon priority to 0xFF to see how many lsbits read back as zero. */
    /* DebugMon priority will later be set correctly by mriCortexMInit(). */
    SCB->SHP[debugMonExceptionNumber-4] = 0xFF;
    zeroBitCount = 32 - (uint32_t)__CLZ(~(SCB->SHP[debugMonExceptionNumber-4] | 0xFFFFFF00));
    priorityBitShift = NVIC_GetPriorityGrouping() + 1;
    if (zeroBitCount > priorityBitShift)
        priorityBitShift = zeroBitCount;
    mriCortexMState.priorityBitShift = priorityBitShift;
}

static void configureDWTandFPB(void)
{
    enableDWTandITM();
    initDWT();
    initFPB();
}

static void defaultSvcAndSysTickInterruptsToLowerPriority(uint8_t priority)
{
    mriCortexMSetPriority(SVCall_IRQn, priority, 0);
    mriCortexMSetPriority(PendSV_IRQn, priority, 0);
    mriCortexMSetPriority(SysTick_IRQn, priority, 0);
}

static void defaultExternalInterruptsToLowerPriority(uint8_t priority, IRQn_Type highestExternalIrq)
{
    int irq;

    for (irq = 0 ; irq <= highestExternalIrq ; irq++)
        mriCortexMSetPriority((IRQn_Type)irq, priority, 0);
}

static void disableDebuggerStack(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_NO_DEBUG_STACK;
}

static void enableDebugMonitorAtSpecifiedPriority(uint8_t priority)
{
    mriCortexMSetPriority(DebugMonitor_IRQn, priority, priority);
    enableDebugMonitor();
}


void mriCortexMSetPriority(IRQn_Type irq, uint8_t priority, uint8_t subPriority)
{
    uint8_t fullPriority = (priority << mriCortexMState.priorityBitShift) |
                           (subPriority & ((1 << mriCortexMState.priorityBitShift) -1));

    if ((int32_t)irq >= 0)
    {
        NVIC->IP[((uint32_t)irq)] = fullPriority;
    }
    else
    {
        SCB->SHP[(((uint32_t)irq) & 0xF)-4] = fullPriority;
    }
}


static void     cleanupIfSingleStepping(void);
static void     restorePriorityRegsIfNeeded(void);
static uint32_t shouldRestorePriorityRegs(void);
static void     clearPriorityRestoreFlag();
static void     removeHardwareBreakpointOnSvcHandlerIfNeeded(void);
static int      shouldRemoveHardwareBreakpointOnSvcHandler(void);
static void     clearSvcStepFlag(void);
static void     clearHardwareBreakpointOnSvcHandler(void);
static uint32_t getNvicVector(IRQn_Type irq);
static void     clearSingleSteppingFlag(void);
void Platform_DisableSingleStep(void)
{
    cleanupIfSingleStepping();
    disableSingleStep();
    clearSingleSteppingFlag();
}

static void cleanupIfSingleStepping(void)
{
    restorePriorityRegsIfNeeded();
    removeHardwareBreakpointOnSvcHandlerIfNeeded();
}

static void restorePriorityRegsIfNeeded(void)
{
    if (shouldRestorePriorityRegs())
    {
        clearPriorityRestoreFlag();
        Context_Set(&mriCortexMState.context, PRIMASK, mriCortexMState.primask);
        Context_Set(&mriCortexMState.context, BASEPRI, mriCortexMState.basepri);
        mriCortexMState.primask = 0;
        mriCortexMState.basepri = 0;
    }
}

static uint32_t shouldRestorePriorityRegs(void)
{
    return mriCortexMFlags & CORTEXM_FLAGS_RESTORE_PRI;
}

static void clearPriorityRestoreFlag()
{
    mriCortexMFlags &= ~CORTEXM_FLAGS_RESTORE_PRI;
}

static void removeHardwareBreakpointOnSvcHandlerIfNeeded(void)
{
    if (shouldRemoveHardwareBreakpointOnSvcHandler())
    {
        clearSvcStepFlag();
        clearHardwareBreakpointOnSvcHandler();
    }
}

static int shouldRemoveHardwareBreakpointOnSvcHandler(void)
{
    return mriCortexMFlags & CORTEXM_FLAGS_SVC_STEP;
}

static void clearSvcStepFlag(void)
{
    mriCortexMFlags &= ~CORTEXM_FLAGS_SVC_STEP;
}

static void clearHardwareBreakpointOnSvcHandler(void)
{
    Platform_ClearHardwareBreakpoint(getNvicVector(SVCall_IRQn) & ~1);
}

static uint32_t getNvicVector(IRQn_Type irq)
{
    const uint32_t           nvicBaseVectorOffset = 16;
    volatile const uint32_t* pVectors = (volatile const uint32_t*)SCB->VTOR;
    return pVectors[irq + nvicBaseVectorOffset];
}

static void clearSingleSteppingFlag(void)
{
    mriCortexMFlags &= ~CORTEXM_FLAGS_SINGLE_STEPPING;
}


static int      doesPCPointToSVCInstruction(void);
static void     setHardwareBreakpointOnSvcHandler(void);
static void     setSvcStepFlag(void);
static void     setSingleSteppingFlag(void);
static int      advancePastPriorityModifyingInstruction(void);
static int      checkCurrentInstruction(void);
static uint16_t getFirstHalfWordOfCurrentInstruction(void);
static uint16_t getSecondHalfWordOfCurrentInstruction(void);
static uint16_t throwingMemRead16(uint32_t address);
static int      isInstructionMSR(uint16_t firstWord, uint16_t secondWord);
static int      processInstructionMSR(uint16_t firstWord, uint16_t secondWord);
static int      isInstructionCPS(uint16_t firstWord);
static int      processInstructionCPS(uint16_t firstWord);
static int      isInstructionMRS(uint16_t firstWord, uint16_t secondWord);
static int      processInstructionMRS(uint16_t firstWord, uint16_t secondWord);
static void     recordCurrentBasePriorityAndRaisePriorityToDisableNonDebugInterrupts(void);
static void     recordCurrentPriorityRegs(void);
static void     setRestorePriorityRegsFlag(void);
static uint8_t  calculateBasePriorityForThisCPU(uint8_t basePriority);
void Platform_EnableSingleStep(void)
{
    int didAdvancePastPriorityInstruction;

    if (MRI_THREAD_MRI)
    {
        setSingleSteppingFlag();
        return;
    }

    didAdvancePastPriorityInstruction = advancePastPriorityModifyingInstruction();
    if (didAdvancePastPriorityInstruction)
    {
        return;
    }

    if (!doesPCPointToSVCInstruction())
    {
        setSingleSteppingFlag();
        recordCurrentBasePriorityAndRaisePriorityToDisableNonDebugInterrupts();
        enableSingleStep();
        return;
    }

    __try
    {
        __throwing_func( setHardwareBreakpointOnSvcHandler() );
        setSvcStepFlag();
    }
    __catch
    {
        /* Failed to set hardware breakpoint so single step without modifying priority since the priority
           elevation leads SVC to escalate to Hard Fault. */
        clearExceptionCode();
        setSingleSteppingFlag();
        enableSingleStep();
    }
    return;
}

static int doesPCPointToSVCInstruction(void)
{
    static const uint16_t svcMachineCodeMask = 0xff00;
    static const uint16_t svcMachineCode = 0xdf00;
    uint16_t              instructionWord;

    __try
    {
        instructionWord = getFirstHalfWordOfCurrentInstruction();
    }
    __catch
    {
        clearExceptionCode();
        return 0;
    }

    return ((instructionWord & svcMachineCodeMask) == svcMachineCode);
}

static void setHardwareBreakpointOnSvcHandler(void)
{
    Platform_SetHardwareBreakpoint(getNvicVector(SVCall_IRQn) & ~1);
}

static void setSvcStepFlag(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_SVC_STEP;
}

static void setSingleSteppingFlag(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_SINGLE_STEPPING;
}

static int advancePastPriorityModifyingInstruction(void)
{
    if (checkCurrentInstruction())
    {
        /* Current instruction is related to priority registers and was simulated so advance PC past it. */
        Platform_AdvanceProgramCounterToNextInstruction();
        return 1;
    }
    return 0;
}

static int checkCurrentInstruction(void)
{
    uint16_t firstWord = 0;
    uint16_t secondWord = 0;

    __try
    {
        __throwing_func( firstWord = getFirstHalfWordOfCurrentInstruction() );
        __throwing_func( secondWord = getSecondHalfWordOfCurrentInstruction() );
    }
    __catch
    {
        clearExceptionCode();
        return 0;
    }

    /* MSR and CPSI* instructions can modify the PRIMASK and BASEPRI registers. */
    if (isInstructionMSR(firstWord, secondWord))
        return processInstructionMSR(firstWord, secondWord);
    if (isInstructionCPS(firstWord))
        return processInstructionCPS(firstWord);
    /* MRS instructions might want to read the real values of PRIMASK and BASEPRI and not the values as modified
       by MRI. */
    if (isInstructionMRS(firstWord, secondWord))
        return processInstructionMRS(firstWord, secondWord);
    return 0;
}

static uint16_t getFirstHalfWordOfCurrentInstruction(void)
{
    return throwingMemRead16(Platform_GetProgramCounter());
}

static uint16_t getSecondHalfWordOfCurrentInstruction(void)
{
    return throwingMemRead16(Platform_GetProgramCounter() + sizeof(uint16_t));
}

static uint16_t throwingMemRead16(uint32_t address)
{
    uint16_t instructionWord = Platform_MemRead16((const uint16_t*)address);
    if (Platform_WasMemoryFaultEncountered())
        __throw_and_return(memFaultException, 0);
    return instructionWord;
}

static int isInstructionMSR(uint16_t firstWord, uint16_t secondWord)
{
    return ((firstWord & 0xFFF0) == 0xF380) && ((secondWord & 0xFF00) == 0x8800);
}

static int processInstructionMSR(uint16_t firstWord, uint16_t secondWord)
{
    uint16_t Rn = firstWord & 0x000F;
    uint16_t SYSm = secondWord & 0x00FF;

    switch (SYSm)
    {
        case 16: // PRIMASK
            Context_Set(&mriCortexMState.context, PRIMASK, Context_Get(&mriCortexMState.context, Rn));
            return 1;
        case 17: // BASEPRI
            Context_Set(&mriCortexMState.context, BASEPRI, Context_Get(&mriCortexMState.context, Rn));
            return 1;
        case 18: // BASEPRI_MAX
        {
            uint32_t basepriVal = Context_Get(&mriCortexMState.context, BASEPRI);
            uint32_t RnVal = Context_Get(&mriCortexMState.context, Rn);
            if (basepriVal == 0 || RnVal < basepriVal)
            {
                Context_Set(&mriCortexMState.context, BASEPRI, RnVal);
            }
            return 1;
        }
        default:
            return 0;
    }
}

static int isInstructionCPS(uint16_t firstWord)
{
    return (firstWord & 0xFFEC) == 0xB660;
}

static int processInstructionCPS(uint16_t firstWord)
{
    uint16_t enable = (firstWord & 0x0010) >> 4;
    uint16_t I = firstWord & 0x0002;

    if (I)
    {
        Context_Set(&mriCortexMState.context, PRIMASK, enable);
        return 1;
    }
    return 0;
}

static int isInstructionMRS(uint16_t firstWord, uint16_t secondWord)
{
    return ((firstWord & 0xFFFF) == 0xF3EF) && ((secondWord & 0xF000) == 0x8000);
}

static int processInstructionMRS(uint16_t firstWord, uint16_t secondWord)
{
    uint16_t Rn = (secondWord & 0x0F00) >> 8;
    uint16_t SYSm = secondWord & 0x00FF;

    switch (SYSm)
    {
        case 16: // PRIMASK
            Context_Set(&mriCortexMState.context, Rn, Context_Get(&mriCortexMState.context, PRIMASK));
            return 1;
        case 17: // BASEPRI
        case 18: // BASEPRI_MAX
            Context_Set(&mriCortexMState.context, Rn, Context_Get(&mriCortexMState.context, BASEPRI));
            return 1;
        default:
            return 0;
    }
}

static void recordCurrentBasePriorityAndRaisePriorityToDisableNonDebugInterrupts(void)
{
    recordCurrentPriorityRegs();
    Context_Set(&mriCortexMState.context, PRIMASK, 0);
    Context_Set(&mriCortexMState.context, BASEPRI,
                calculateBasePriorityForThisCPU(mriCortexMGetPriority(DebugMonitor_IRQn) + 1));
}

static void recordCurrentPriorityRegs(void)
{
    mriCortexMState.primask = Context_Get(&mriCortexMState.context, PRIMASK);
    mriCortexMState.basepri = Context_Get(&mriCortexMState.context, BASEPRI);
    setRestorePriorityRegsFlag();
}

static void setRestorePriorityRegsFlag(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_RESTORE_PRI;
}

static uint8_t calculateBasePriorityForThisCPU(uint8_t basePriority)
{
    /* Different Cortex-M3 chips support different number of bits in the priority register. */
    return basePriority << mriCortexMState.priorityBitShift;
}


uint8_t mriCortexMGetPriority(IRQn_Type irq)
{
    uint8_t priority;

    if ((int32_t)irq >= 0)
    {
        priority = NVIC->IP[(uint32_t)irq];
    }
    else
    {
        priority = SCB->SHP[((uint32_t)irq & 0xF)-4];
    }
    return priority >> mriCortexMState.priorityBitShift;
}


int Platform_IsSingleStepping(void)
{
    return mriCortexMFlags & CORTEXM_FLAGS_SINGLE_STEPPING;
}


char* Platform_GetPacketBuffer(void)
{
    return mriCortexMState.packetBuffer;
}


uint32_t Platform_GetPacketBufferSize(void)
{
    return sizeof(mriCortexMState.packetBuffer);
}


static PlatformTrapReason cacheTrapReason(void);
static uint32_t encounteredStackingException(void);
static PlatformTrapReason findMatchedWatchpoint(void);
static PlatformTrapReason getReasonFromMatchComparator(const DWT_COMP_Type* pComparator);
static uint32_t hasControlCBeenDetected();
static uint8_t  determineCauseOfDebugEvent(void);
uint8_t Platform_DetermineCauseOfException(void)
{
    uint32_t exceptionNumber = mriCortexMState.exceptionNumber;
    mriCortexMState.reason = cacheTrapReason();

    if (hasControlCBeenDetected())
    {
        return SIGINT;
    }

    switch(exceptionNumber)
    {
    case 2:
        /* NMI */
        return SIGINT;
    case 3:
        /* HardFault */
        return SIGSEGV;
    case 4:
        /* MemManage */
        return SIGSEGV;
    case 5:
        /* BusFault */
        return SIGBUS;
    case 6:
        /* UsageFault */
        return SIGILL;
    case 12:
        /* Debug Monitor */
        return determineCauseOfDebugEvent();
    default:
        /* NOTE: Catch all signal will be SIGINT. */
        return SIGINT;
    }
}

PlatformTrapReason cacheTrapReason(void)
{
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_UNKNOWN, 0x00000000 };
    uint32_t debugFaultStatus = mriCortexMState.dfsr;

    if (encounteredStackingException())
    {
        /* Stacking faults are more important than breakpoints and lead to an unknown PC anyway. */
        return reason;
    }
    else if (debugFaultStatus & SCB_DFSR_BKPT)
    {
        /* Was caused by hardware or software breakpoint. If PC points to BKPT then report as software breakpoint. */
        if (Platform_TypeOfCurrentInstruction() == MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT)
            reason.type = MRI_PLATFORM_TRAP_TYPE_SWBREAK;
        else
            reason.type = MRI_PLATFORM_TRAP_TYPE_HWBREAK;
    }
    else if (debugFaultStatus & SCB_DFSR_DWTTRAP)
    {
        reason = findMatchedWatchpoint();
    }
    return reason;
}

static uint32_t encounteredStackingException(void)
{
    return mriCortexMState.cfsr & CFSR_STACK_ERROR_BITS;
}

static PlatformTrapReason findMatchedWatchpoint(void)
{
    PlatformTrapReason reason = { MRI_PLATFORM_TRAP_TYPE_UNKNOWN, 0x00000000 };
    DWT_COMP_Type*     pCurrentComparator = DWT_COMP_ARRAY;
    uint32_t           comparatorCount;
    uint32_t           i;

    comparatorCount = getDWTComparatorCount();
    for (i = 0 ; i < comparatorCount ; i++)
    {
        if (pCurrentComparator->FUNCTION & DWT_COMP_FUNCTION_MATCHED)
            reason = getReasonFromMatchComparator(pCurrentComparator);
        pCurrentComparator++;
    }
    return reason;
}

static PlatformTrapReason getReasonFromMatchComparator(const DWT_COMP_Type* pComparator)
{
    PlatformTrapReason reason;
    switch (pComparator->FUNCTION & DWT_COMP_FUNCTION_FUNCTION_MASK)
    {
    case DWT_COMP_FUNCTION_FUNCTION_DATA_READ:
        reason.type = MRI_PLATFORM_TRAP_TYPE_RWATCH;
        break;
    case DWT_COMP_FUNCTION_FUNCTION_DATA_WRITE:
        reason.type = MRI_PLATFORM_TRAP_TYPE_WATCH;
        break;
    case DWT_COMP_FUNCTION_FUNCTION_DATA_READWRITE:
        reason.type = MRI_PLATFORM_TRAP_TYPE_AWATCH;
        break;
    default:
        reason.type = MRI_PLATFORM_TRAP_TYPE_UNKNOWN;
        break;
    }
    reason.address = pComparator->COMP;
    return reason;
}

static uint32_t hasControlCBeenDetected()
{
    return mriCortexMFlags & CORTEXM_FLAGS_CTRL_C;
}

static uint8_t determineCauseOfDebugEvent(void)
{
    static struct
    {
        uint32_t        statusBit;
        unsigned char   signalToReturn;
    } const debugEventToSignalMap[] =
    {
        {SCB_DFSR_EXTERNAL, SIGSTOP},
        {SCB_DFSR_DWTTRAP, SIGTRAP},
        {SCB_DFSR_BKPT, SIGTRAP},
        {SCB_DFSR_HALTED, SIGTRAP}
    };
    uint32_t debugFaultStatus = mriCortexMState.dfsr;
    size_t   i;

    for (i = 0 ; i < sizeof(debugEventToSignalMap)/sizeof(debugEventToSignalMap[0]) ; i++)
    {
        if (debugFaultStatus & debugEventToSignalMap[i].statusBit)
        {
            return debugEventToSignalMap[i].signalToReturn;
        }
    }

    /* NOTE: Default catch all signal is SIGSTOP. */
    return SIGSTOP;
}


PlatformTrapReason Platform_GetTrapReason(void)
{
    /* Return reason cached earlier by call to Platform_DetermineCauseOfException() so that findMatchedWatchpoint()
       doesn't get called multiple times as it has the side effect of clearing the watchpoint MATCH bits. */
    return mriCortexMState.reason;
}


static void displayHardFaultCauseToGdbConsole(void);
static void displayMemFaultCauseToGdbConsole(void);
static void displayBusFaultCauseToGdbConsole(void);
static void displayUsageFaultCauseToGdbConsole(void);
void Platform_DisplayFaultCauseToGdbConsole(void)
{
    switch (mriCortexMState.exceptionNumber)
    {
    case 3:
        /* HardFault */
        displayHardFaultCauseToGdbConsole();
        break;
    case 4:
        /* MemManage */
        displayMemFaultCauseToGdbConsole();
        break;
    case 5:
        /* BusFault */
        displayBusFaultCauseToGdbConsole();
        break;
    case 6:
        /* UsageFault */
        displayUsageFaultCauseToGdbConsole();
        break;
    default:
        return;
    }
    WriteStringToGdbConsole("\n");
}

static void displayHardFaultCauseToGdbConsole(void)
{
    static const uint32_t debugEventBit = 1 << 31;
    static const uint32_t forcedBit = 1 << 30;
    static const uint32_t vectorTableReadBit = 1 << 1;
    uint32_t              hardFaultStatusRegister = mriCortexMState.hfsr;

    WriteStringToGdbConsole("\n**Hard Fault**");
    WriteStringToGdbConsole("\n  Status Register: ");
    WriteHexValueToGdbConsole(hardFaultStatusRegister);

    if (hardFaultStatusRegister & debugEventBit)
        WriteStringToGdbConsole("\n    Debug Event");

    if (hardFaultStatusRegister & vectorTableReadBit)
        WriteStringToGdbConsole("\n    Vector Table Read");

    if (hardFaultStatusRegister & forcedBit)
    {
        WriteStringToGdbConsole("\n    Forced");
        displayMemFaultCauseToGdbConsole();
        displayBusFaultCauseToGdbConsole();
        displayUsageFaultCauseToGdbConsole();
    }
}

static void displayMemFaultCauseToGdbConsole(void)
{
    static const uint32_t MMARValidBit = 1 << 7;
    static const uint32_t FPLazyStatePreservationBit = 1 << 5;
    static const uint32_t stackingErrorBit = 1 << 4;
    static const uint32_t unstackingErrorBit = 1 << 3;
    static const uint32_t dataAccess = 1 << 1;
    static const uint32_t instructionFetch = 1;
    uint32_t              memManageFaultStatusRegister = mriCortexMState.cfsr & 0xFF;

    /* Check to make sure that there is a memory fault to display. */
    if (memManageFaultStatusRegister == 0)
        return;

    WriteStringToGdbConsole("\n**MPU Fault**");
    WriteStringToGdbConsole("\n  Status Register: ");
    WriteHexValueToGdbConsole(memManageFaultStatusRegister);

    if (memManageFaultStatusRegister & MMARValidBit)
    {
        WriteStringToGdbConsole("\n    Fault Address: ");
        WriteHexValueToGdbConsole(mriCortexMState.mmfar);
    }
    if (memManageFaultStatusRegister & FPLazyStatePreservationBit)
        WriteStringToGdbConsole("\n    FP Lazy Preservation");

    if (memManageFaultStatusRegister & stackingErrorBit)
    {
        WriteStringToGdbConsole("\n    Stacking Error w/ SP = ");
        WriteHexValueToGdbConsole(mriCortexMState.taskSP);
    }
    if (memManageFaultStatusRegister & unstackingErrorBit)
    {
        WriteStringToGdbConsole("\n    Unstacking Error w/ SP = ");
        WriteHexValueToGdbConsole(mriCortexMState.taskSP);
    }
    if (memManageFaultStatusRegister & dataAccess)
        WriteStringToGdbConsole("\n    Data Access");

    if (memManageFaultStatusRegister & instructionFetch)
        WriteStringToGdbConsole("\n    Instruction Fetch");
}

static void displayBusFaultCauseToGdbConsole(void)
{
    static const uint32_t BFARValidBit = 1 << 7;
    static const uint32_t FPLazyStatePreservationBit = 1 << 5;
    static const uint32_t stackingErrorBit = 1 << 4;
    static const uint32_t unstackingErrorBit = 1 << 3;
    static const uint32_t impreciseDataAccessBit = 1 << 2;
    static const uint32_t preciseDataAccessBit = 1 << 1;
    static const uint32_t instructionPrefetch = 1;
    uint32_t              busFaultStatusRegister = (mriCortexMState.cfsr >> 8) & 0xFF;

    /* Check to make sure that there is a bus fault to display. */
    if (busFaultStatusRegister == 0)
        return;

    WriteStringToGdbConsole("\n**Bus Fault**");
    WriteStringToGdbConsole("\n  Status Register: ");
    WriteHexValueToGdbConsole(busFaultStatusRegister);

    if (busFaultStatusRegister & BFARValidBit)
    {
        WriteStringToGdbConsole("\n    Fault Address: ");
        WriteHexValueToGdbConsole(mriCortexMState.bfar);
    }
    if (busFaultStatusRegister & FPLazyStatePreservationBit)
        WriteStringToGdbConsole("\n    FP Lazy Preservation");

    if (busFaultStatusRegister & stackingErrorBit)
    {
        WriteStringToGdbConsole("\n    Stacking Error w/ SP = ");
        WriteHexValueToGdbConsole(mriCortexMState.taskSP);
    }
    if (busFaultStatusRegister & unstackingErrorBit)
    {
        WriteStringToGdbConsole("\n    Unstacking Error w/ SP = ");
        WriteHexValueToGdbConsole(mriCortexMState.taskSP);
    }
    if (busFaultStatusRegister & impreciseDataAccessBit)
        WriteStringToGdbConsole("\n    Imprecise Data Access");

    if (busFaultStatusRegister & preciseDataAccessBit)
        WriteStringToGdbConsole("\n    Precise Data Access");

    if (busFaultStatusRegister & instructionPrefetch)
        WriteStringToGdbConsole("\n    Instruction Prefetch");
}

static void displayUsageFaultCauseToGdbConsole(void)
{
    static const uint32_t divideByZeroBit = 1 << 9;
    static const uint32_t unalignedBit = 1 << 8;
    static const uint32_t coProcessorAccessBit = 1 << 3;
    static const uint32_t invalidPCBit = 1 << 2;
    static const uint32_t invalidStateBit = 1 << 1;
    static const uint32_t undefinedInstructionBit = 1;
    uint32_t              usageFaultStatusRegister = mriCortexMState.cfsr >> 16;

    /* Make sure that there is a usage fault to display. */
    if (usageFaultStatusRegister == 0)
        return;

    WriteStringToGdbConsole("\n**Usage Fault**");
    WriteStringToGdbConsole("\n  Status Register: ");
    WriteHexValueToGdbConsole(usageFaultStatusRegister);

    if (usageFaultStatusRegister & divideByZeroBit)
        WriteStringToGdbConsole("\n    Divide by Zero");

    if (usageFaultStatusRegister & unalignedBit)
        WriteStringToGdbConsole("\n    Unaligned Access");

    if (usageFaultStatusRegister & coProcessorAccessBit)
        WriteStringToGdbConsole("\n    Coprocessor Access");

    if (usageFaultStatusRegister & invalidPCBit)
        WriteStringToGdbConsole("\n    Invalid Exception Return State");

    if (usageFaultStatusRegister & invalidStateBit)
        WriteStringToGdbConsole("\n    Invalid State");

    if (usageFaultStatusRegister & undefinedInstructionBit)
        WriteStringToGdbConsole("\n    Undefined Instruction");
}


static void     clearMemoryFaultFlag(void);
static int      isExternalInterrupt(uint32_t exceptionNumber);
static void     setControlCFlag(void);
static void     setActiveDebugFlag(void);
void Platform_EnteringDebugger(void)
{
    clearMemoryFaultFlag();
    mriCortexMState.originalPC = Platform_GetProgramCounter();
    Platform_DisableSingleStep();
    if (isExternalInterrupt(mriCortexMState.exceptionNumber))
        setControlCFlag();
    setActiveDebugFlag();
}

static void clearMemoryFaultFlag(void)
{
    mriCortexMFlags &= ~CORTEXM_FLAGS_FAULT_DURING_DEBUG;
}

static int isExternalInterrupt(uint32_t exceptionNumber)
{
    /* Exception numbers below 16 are reserved for system faults. */
    return exceptionNumber >= 16;
}

static void setControlCFlag(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_CTRL_C;
}

static void setActiveDebugFlag(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_ACTIVE_DEBUG;
}


static void checkStack(void);
static void clearControlCFlag(void);
static void clearActiveDebugFlag(void);
static void clearPendedFromFaultFlag(void);
void Platform_LeavingDebugger(void)
{
    checkStack();
    clearControlCFlag();
    clearActiveDebugFlag();
    clearPendedFromFaultFlag();
    clearMonitorPending();
}

static void clearControlCFlag(void)
{
    mriCortexMFlags &= ~CORTEXM_FLAGS_CTRL_C;
}

static void clearActiveDebugFlag(void)
{
    mriCortexMFlags &= ~CORTEXM_FLAGS_ACTIVE_DEBUG;
}

static void clearPendedFromFaultFlag(void)
{
    mriCortexMFlags &= ~CORTEXM_FLAGS_PEND_FROM_FAULT;
}

static void checkStack(void)
{
    uint32_t* pCurr = (uint32_t*)mriCortexMDebuggerStack;
    uint8_t*  pEnd = (uint8_t*)mriCortexMDebuggerStack + sizeof(mriCortexMDebuggerStack);
    int       spaceUsed;

    while ((uint8_t*)pCurr < pEnd && *pCurr == CORTEXM_DEBUGGER_STACK_FILL)
        pCurr++;

    spaceUsed = pEnd - (uint8_t*)pCurr;
    if (spaceUsed > mriCortexMState.maxStackUsed)
        mriCortexMState.maxStackUsed = spaceUsed;
}


uint32_t Platform_GetProgramCounter(void)
{
    return Context_Get(&mriCortexMState.context, PC);
}


void Platform_SetProgramCounter(uint32_t newPC)
{
    Context_Set(&mriCortexMState.context, PC, newPC);
}


static int isInstruction32Bit(uint16_t firstWordOfInstruction);
void Platform_AdvanceProgramCounterToNextInstruction(void)
{
    uint16_t  firstWordOfCurrentInstruction;

    __try
    {
        firstWordOfCurrentInstruction = getFirstHalfWordOfCurrentInstruction();
    }
    __catch
    {
        /* Will get here if PC isn't pointing to valid memory so don't bother to advance. */
        clearExceptionCode();
        return;
    }

    if (isInstruction32Bit(firstWordOfCurrentInstruction))
    {
        /* 32-bit Instruction. */
        Platform_SetProgramCounter(Platform_GetProgramCounter() + sizeof(uint32_t));
    }
    else
    {
        /* 16-bit Instruction. */
        Platform_SetProgramCounter(Platform_GetProgramCounter() + sizeof(uint16_t));
    }
}

static int isInstruction32Bit(uint16_t firstWordOfInstruction)
{
    uint16_t maskedOffUpper5BitsOfWord = firstWordOfInstruction & 0xF800;

    /* 32-bit instructions start with 0b11101, 0b11110, 0b11111 according to page A5-152 of the
       ARMv7-M Architecture Manual. */
    return  (maskedOffUpper5BitsOfWord == 0xE800 ||
             maskedOffUpper5BitsOfWord == 0xF000 ||
             maskedOffUpper5BitsOfWord == 0xF800);
}


int Platform_WasProgramCounterModifiedByUser(void)
{
    return Platform_GetProgramCounter() != mriCortexMState.originalPC;
}


static int isInstructionMbedSemihostBreakpoint(uint16_t instruction);
static int isInstructionNewlibSemihostBreakpoint(uint16_t instruction);
static int isInstructionHardcodedBreakpoint(uint16_t instruction);
PlatformInstructionType Platform_TypeOfCurrentInstruction(void)
{
    uint16_t currentInstruction;

    __try
    {
        currentInstruction = getFirstHalfWordOfCurrentInstruction();
    }
    __catch
    {
        /* Will get here if PC isn't pointing to valid memory so treat as other. */
        clearExceptionCode();
        return MRI_PLATFORM_INSTRUCTION_OTHER;
    }

    if (isInstructionMbedSemihostBreakpoint(currentInstruction))
        return MRI_PLATFORM_INSTRUCTION_MBED_SEMIHOST_CALL;
    else if (isInstructionNewlibSemihostBreakpoint(currentInstruction))
        return MRI_PLATFORM_INSTRUCTION_NEWLIB_SEMIHOST_CALL;
    else if (isInstructionHardcodedBreakpoint(currentInstruction))
        return MRI_PLATFORM_INSTRUCTION_HARDCODED_BREAKPOINT;
    else
        return MRI_PLATFORM_INSTRUCTION_OTHER;
}

static int isInstructionMbedSemihostBreakpoint(uint16_t instruction)
{
    static const uint16_t mbedSemihostBreakpointMachineCode = 0xbeab;

    return mbedSemihostBreakpointMachineCode == instruction;
}

static int isInstructionNewlibSemihostBreakpoint(uint16_t instruction)
{
    static const uint16_t newlibSemihostBreakpointMinMachineCode = 0xbe00 | MRI_NEWLIB_SEMIHOST_MIN;
    static const uint16_t newlibSemihostBreakpointMaxMachineCode = 0xbe00 | MRI_NEWLIB_SEMIHOST_MAX;

    return (instruction >= newlibSemihostBreakpointMinMachineCode &&
            instruction <=  newlibSemihostBreakpointMaxMachineCode);
}

static int isInstructionHardcodedBreakpoint(uint16_t instruction)
{
    static const uint16_t hardCodedBreakpointMachineCode = 0xbe00;

    return (hardCodedBreakpointMachineCode == instruction);
}


PlatformSemihostParameters Platform_GetSemihostCallParameters(void)
{
    PlatformSemihostParameters parameters;

    parameters.parameter1 = Context_Get(&mriCortexMState.context, R0);
    parameters.parameter2 = Context_Get(&mriCortexMState.context, R1);
    parameters.parameter3 = Context_Get(&mriCortexMState.context, R2);
    parameters.parameter4 = Context_Get(&mriCortexMState.context, R3);

    return parameters;
}


void Platform_SetSemihostCallReturnAndErrnoValues(int returnValue, int errNo)
{
    Context_Set(&mriCortexMState.context, R0, returnValue);
}


int Platform_WasMemoryFaultEncountered(void)
{
    int wasFaultEncountered;

    __DSB();
    wasFaultEncountered = mriCortexMFlags & CORTEXM_FLAGS_FAULT_DURING_DEBUG;
    clearMemoryFaultFlag();

    return wasFaultEncountered;
}


static void sendRegisterForTResponse(Buffer* pBuffer, uint8_t registerOffset, uint32_t registerValue);
static void writeBytesToBufferAsHex(Buffer* pBuffer, void* pBytes, size_t byteCount);
void Platform_WriteTResponseRegistersToBuffer(Buffer* pBuffer)
{
    sendRegisterForTResponse(pBuffer, R7, Context_Get(&mriCortexMState.context, R7));
    sendRegisterForTResponse(pBuffer, SP, Context_Get(&mriCortexMState.context, SP));
    sendRegisterForTResponse(pBuffer, LR, Context_Get(&mriCortexMState.context, LR));
    sendRegisterForTResponse(pBuffer, PC, Context_Get(&mriCortexMState.context, PC));
}

static void sendRegisterForTResponse(Buffer* pBuffer, uint8_t registerOffset, uint32_t registerValue)
{
    Buffer_WriteByteAsHex(pBuffer, registerOffset);
    Buffer_WriteChar(pBuffer, ':');
    writeBytesToBufferAsHex(pBuffer, &registerValue, sizeof(registerValue));
    Buffer_WriteChar(pBuffer, ';');
}

static void writeBytesToBufferAsHex(Buffer* pBuffer, void* pBytes, size_t byteCount)
{
    uint8_t* pByte = (uint8_t*)pBytes;
    size_t   i;

    for (i = 0 ; i < byteCount ; i++)
        Buffer_WriteByteAsHex(pBuffer, *pByte++);
}


static int doesKindIndicate32BitInstruction(uint32_t kind);
void Platform_SetHardwareBreakpointOfGdbKind(uint32_t address, uint32_t kind)
{
    uint32_t* pFPBBreakpointComparator;
    int       is32BitInstruction;

    __try
        is32BitInstruction = doesKindIndicate32BitInstruction(kind);
    __catch
        __rethrow;

    pFPBBreakpointComparator = enableFPBBreakpoint(address, is32BitInstruction);
    if (!pFPBBreakpointComparator)
        __throw(exceededHardwareResourcesException);
}

static int doesKindIndicate32BitInstruction(uint32_t kind)
{
    switch (kind)
    {
    case 2:
        return 0;
    case 3:
    case 4:
        return 1;
    default:
        __throw_and_return(invalidArgumentException, -1);
    }
}


void Platform_SetHardwareBreakpoint(uint32_t address)
{
    uint32_t* pFPBBreakpointComparator;
    uint16_t  firstInstructionWord;

     __try
    {
        firstInstructionWord = throwingMemRead16(address);
    }
    __catch
        __rethrow;

    pFPBBreakpointComparator = enableFPBBreakpoint(address, isInstruction32Bit(firstInstructionWord));
    if (!pFPBBreakpointComparator)
        __throw(exceededHardwareResourcesException);
}


void Platform_ClearHardwareBreakpointOfGdbKind(uint32_t address, uint32_t kind)
{
    int       is32BitInstruction;

    __try
        is32BitInstruction = doesKindIndicate32BitInstruction(kind);
    __catch
        __rethrow;

    disableFPBBreakpointComparator(address, is32BitInstruction);
}


void Platform_ClearHardwareBreakpoint(uint32_t address)
{
    uint16_t  firstInstructionWord;

     __try
    {
        firstInstructionWord = throwingMemRead16(address);
    }
    __catch
        __rethrow;

    disableFPBBreakpointComparator(address, isInstruction32Bit(firstInstructionWord));
}


static uint32_t convertWatchpointTypeToCortexMType(PlatformWatchpointType type);
void Platform_SetHardwareWatchpoint(uint32_t address, uint32_t size, PlatformWatchpointType type)
{
    uint32_t       nativeType = convertWatchpointTypeToCortexMType(type);
    DWT_COMP_Type* pComparator;

    if (!isValidDWTComparatorSetting(address, size, nativeType))
        __throw(invalidArgumentException);

    pComparator = enableDWTWatchpoint(address, size, nativeType);
    if (!pComparator)
        __throw(exceededHardwareResourcesException);
}

static uint32_t convertWatchpointTypeToCortexMType(PlatformWatchpointType type)
{
    switch (type)
    {
    case MRI_PLATFORM_WRITE_WATCHPOINT:
        return DWT_COMP_FUNCTION_FUNCTION_DATA_WRITE;
    case MRI_PLATFORM_READ_WATCHPOINT:
        return DWT_COMP_FUNCTION_FUNCTION_DATA_READ;
    case MRI_PLATFORM_READWRITE_WATCHPOINT:
        return DWT_COMP_FUNCTION_FUNCTION_DATA_READWRITE;
    default:
        return 0;
    }
}


void Platform_ClearHardwareWatchpoint(uint32_t address, uint32_t size, PlatformWatchpointType type)
{
    uint32_t nativeType = convertWatchpointTypeToCortexMType(type);

    if (!isValidDWTComparatorSetting(address, size, nativeType))
        __throw(invalidArgumentException);

    disableDWTWatchpoint(address, size, nativeType);
}

uint32_t Platform_GetTargetXmlSize(void)
{
    return sizeof(g_targetXml) - 1;
}


const char* Platform_GetTargetXml(void)
{
    return g_targetXml;
}


void Platform_ResetDevice(void)
{
    NVIC_SystemReset();
}


/* Cache related registers as defined in ARMv7-M Architecture Reference Manual but only partially in CMSIS headers. */
#define SCB_CLIDR   (*(__IM uint32_t*)0xE000ED78)   /* Cache Level ID register */
#define SCB_CTR     (*(__IM uint32_t*)0xE000ED7C)   /* Cache Type register */
#define SCB_ICIMVAU (*(__OM uint32_t*)0xE000EF58)   /* I-Cache Invalidate by MVA to PoU */
#define SCB_DCCMVAU (*(__OM uint32_t*)0xE000EF64)   /* D-Cache Clean by MVA to PoU */
#define SCB_BPIALL  (*(__OM uint32_t*)0xE000EF78)   /* Branch Predictor Invalidate All */

static int levelOfCacheUnification(void);
static void cleanDataCacheToPointOfUnification(uint32_t address, uint32_t size);
static uint32_t getDCacheLineSize(void);
static void invalidateInstructionCacheToPointOfUnification(uint32_t address, uint32_t size);
static uint32_t getICacheLineSize(void);
static void invalidateAllBranchPredictions(void);
void Platform_SyncICacheToDCache(void *pv, uint32_t size)
{
    uint32_t address = (uint32_t)pv;

    if (size == 0 || levelOfCacheUnification() == 0)
        return;

    __DSB();
    cleanDataCacheToPointOfUnification(address, size);
    __DSB();
    invalidateInstructionCacheToPointOfUnification(address, size);
    invalidateAllBranchPredictions();
    __DSB();
    __ISB();
}

static int levelOfCacheUnification(void)
{
    const uint32_t LoUU_BitShift = 27;
    const uint32_t LoUU_BitMask = 0x7 << LoUU_BitShift;

    return (SCB_CLIDR & LoUU_BitMask) >> LoUU_BitShift;
}

static void cleanDataCacheToPointOfUnification(uint32_t address, uint32_t size)
{
    uint32_t lineSize = getDCacheLineSize();
    int32_t  op_size = size + (address & (lineSize - 1U));
    uint32_t op_addr = address; /* & ~(lineSize - 1U) */

    do
    {
        /* Register forces lineSize alignment by setting lsb to 0, so don't need to do in op_addr calc above too. */
        SCB_DCCMVAU = op_addr;
        op_addr += lineSize;
        op_size -= lineSize;
    } while ( op_size > 0 );
}

static uint32_t getDCacheLineSize(void)
{
    const uint32_t DminLine_BitShift = 16;
    const uint32_t DminLine_BitMask = 0xF << DminLine_BitShift;

    uint32_t DminLineLog2 = (SCB_CTR & DminLine_BitMask) >> DminLine_BitShift;
    uint32_t DminLineWords = 1 << DminLineLog2;
    uint32_t DminLineBytes = DminLineWords * sizeof(uint32_t);

    return DminLineBytes;
}

static void invalidateInstructionCacheToPointOfUnification(uint32_t address, uint32_t size)
{
    uint32_t lineSize = getICacheLineSize();
    int32_t  op_size = size + (address & (lineSize - 1U));
    uint32_t op_addr = address; /* & ~(lineSize - 1U) */

    do
    {
        /* Register forces lineSize alignment by setting lsb to 0, so don't need to do in op_addr calc above too. */
        SCB_ICIMVAU = op_addr;
        op_addr += lineSize;
        op_size -= lineSize;
    } while ( op_size > 0 );
}

static uint32_t getICacheLineSize(void)
{
    const uint32_t IminLine_BitMask = 0xF;

    uint32_t IminLineLog2 = SCB_CTR & IminLine_BitMask;
    uint32_t IminLineWords = 1 << IminLineLog2;
    uint32_t IminLineBytes = IminLineWords * sizeof(uint32_t);

    return IminLineBytes;
}

static void invalidateAllBranchPredictions(void)
{
    SCB_BPIALL = 1;
}



#if !MRI_THREAD_MRI
/****************************************************************************************************/
/* Handler/Kernel Mode MRI C code handlers to prepare environment before calling mriDebugException. */
/****************************************************************************************************/
/* Entries to track the chunks of the context in a scatter list. */
#if MRI_DEVICE_HAS_FPU
    #define CONTEXT_ENTRIES     (6 + 3)
#else
    #define CONTEXT_ENTRIES     6
#endif

static ContextSection   g_contextEntries[CONTEXT_ENTRIES];


/* Lower nibble of EXC_RETURN in LR will have one of these values if interrupted code was running in thread mode.
   Using PSP. */
#define EXC_RETURN_THREADMODE_PROCESSSTACK  0xD
/*  Using MSP. */
#define EXC_RETURN_THREADMODE_MAINSTACK     0x9

/* Bit location in PSR which indicates if the stack needed to be 8-byte aligned or not. */
#define PSR_STACK_ALIGN_BIT_POS             9

/* Bit in LR set to 0 when automatic stacking of floating point registers occurs during exception handling. */
#define LR_FLOAT_STACK                      (1 << 4)



typedef struct IntegerRegisters
{
    uint32_t    msp;
    uint32_t    psp;
    uint32_t    primask;
    uint32_t    basepri;
    uint32_t    faultmask;
    uint32_t    control;
    uint32_t    r4;
    uint32_t    r5;
    uint32_t    r6;
    uint32_t    r7;
    uint32_t    r8;
    uint32_t    r9;
    uint32_t    r10;
    uint32_t    r11;
    uint32_t    excReturn;
} IntegerRegisters;

typedef struct ExceptionStack
{
    uint32_t    r0;
    uint32_t    r1;
    uint32_t    r2;
    uint32_t    r3;
    uint32_t    r12;
    uint32_t    lr;
    uint32_t    pc;
    uint32_t    xpsr;
    /* Need to check EXC_RETURN value in exception LR to see if these floating point registers have been stacked. */
    uint32_t    s0;
    uint32_t    s1;
    uint32_t    s2;
    uint32_t    s3;
    uint32_t    s4;
    uint32_t    s5;
    uint32_t    s6;
    uint32_t    s7;
    uint32_t    s8;
    uint32_t    s9;
    uint32_t    s10;
    uint32_t    s11;
    uint32_t    s12;
    uint32_t    s13;
    uint32_t    s14;
    uint32_t    s15;
    uint32_t    fpscr;
} ExceptionStack;



static void setFaultDetectedFlag(void);
static uint32_t isImpreciseBusFaultRaw(void);
static ExceptionStack* getExceptionStack(uint32_t excReturn, uint32_t psp, uint32_t msp);
static void advancePCToNextInstruction(ExceptionStack* pExceptionStack);
static void clearFaultStatusBits(void);
void mriCortexHandleDebuggerFault(uint32_t psp, uint32_t msp, uint32_t excReturn)
{
    /* Encountered memory fault when GDB attempted to access an invalid address.
        Set flag to let MRI know that its access failed and advance past the faulting instruction
        if it was a precise bus fault so that it doesn't just occur again on return.
    */
    setFaultDetectedFlag();
    if (!isImpreciseBusFaultRaw())
    {
        ExceptionStack* pExceptionStack = getExceptionStack(excReturn, psp, msp);
        advancePCToNextInstruction(pExceptionStack);
    }
    clearFaultStatusBits();
}

static void setFaultDetectedFlag(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_FAULT_DURING_DEBUG;
}

static uint32_t isImpreciseBusFaultRaw(void)
{
    /* Uses the raw SCB->CFSR register since it is called before recordAndClearFaultStatusBits(). */
    return SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk;
}

static ExceptionStack* getExceptionStack(uint32_t excReturn, uint32_t psp, uint32_t msp)
{
    uint32_t sp;
    if ((excReturn & 0xF) == EXC_RETURN_THREADMODE_PROCESSSTACK)
        sp = psp;
    else
        sp = msp;
    return (ExceptionStack*)sp;
}

static void advancePCToNextInstruction(ExceptionStack* pExceptionStack)
{
    uint32_t* pPC = &pExceptionStack->pc;
    uint16_t  currentInstruction = *(uint16_t*)*pPC;
    if (isInstruction32Bit(currentInstruction)) {
        *pPC += sizeof(uint32_t);
    } else {
        *pPC += sizeof(uint16_t);
    }
}

static void clearFaultStatusBits(void)
{
    /* Clear fault status bits by writing 1s to bits that are already set. */
    SCB->DFSR = SCB->DFSR;
    SCB->HFSR = SCB->HFSR;
    SCB->CFSR = SCB->CFSR;
}

static int wasFaultCausedByStackingException(void);
static int isExceptionPriorityLowEnoughToDebug(uint32_t exceptionNumber);
static int hasDebugMonInterruptBeenDisabled();
static void recordAndClearFaultStatusBits(uint32_t exceptionNumber);
static void disableInterruptMaskingIfNecessary(void);
static void treatDebugEventHardFaultAsDebugMonInterrupt(void);
static void setPendedFromFaultBit(void);
int mriPendFaultToDebugMon(uint32_t psp, uint32_t msp, uint32_t excReturn)
{
    /* This handler will be called from the fault handlers (Hard Fault, etc.)
       What needs to be done depends on CPU state when the fault occurs.
    */
    if (wasFaultCausedByStackingException())
    {
        /* Return -1 to let asm caller know that it wasn't safe to pend DebugMon interrupt because of stacking error. */
        return -1;
    }

    ExceptionStack* pExceptionStack = getExceptionStack(excReturn, psp, msp);
    uint32_t exceptionNumber = pExceptionStack->xpsr & 0xFF;
    if (isExceptionPriorityLowEnoughToDebug(exceptionNumber))
    {
        /* Pend DebugMon interrupt to debug the fault.

           Returns 0 to let asm routine know that it can now just return to let the pended DebugMon run.
        */
        recordAndClearFaultStatusBits(getCurrentlyExecutingExceptionNumber());
        disableInterruptMaskingIfNecessary();
        treatDebugEventHardFaultAsDebugMonInterrupt();
        setPendedFromFaultBit();
        setMonitorPending();
        return 0;
    }
    else
    {
        /* Exception occurred in code too high priority to debug so start a crash dump.

           Returns -1 to let asm routine know that it should call Platform_HandleFaultFromHighPriorityCode() to handle
           this special case by doing something like dumping a crash dump since MRI can't debug it.
        */
        return -1;
    }
}

static int wasFaultCausedByStackingException(void)
{
    return SCB->CFSR & CFSR_STACK_ERROR_BITS;
}

static int isExceptionPriorityLowEnoughToDebug(uint32_t exceptionNumber)
{
    if (hasDebugMonInterruptBeenDisabled())
    {
        /* User code has entered critical section using PRIMASK or BASEPRI which disables debug monitor. */
        return 0;
    }
    else if (exceptionNumber == 0)
    {
        /* Can always debug main thread as it has lowest priority. */
        return 1;
    }
    else if (exceptionNumber >= 1 && exceptionNumber <= 3)
    {
        /* NMI & HardFault are always higher priority than DebugMon. */
        return 0;
    }
    else
    {
        return mriCortexMGetPriority((IRQn_Type)(-16+exceptionNumber)) > mriCortexMGetPriority(DebugMonitor_IRQn);
    }
}

static int hasDebugMonInterruptBeenDisabled()
{
    /* Was user code in a critical section that disabled DebugMon interrupt when debug event occurred? */
    uint32_t primask = __get_PRIMASK();
    uint32_t basepri = __get_BASEPRI();
    uint32_t debugMonPriority = mriCortexMGetPriority(DebugMonitor_IRQn);

    if (primask != 0 && debugMonPriority > 0)
    {
        /* All interrupts have been masked and if DebugMon is running at priority lower than 0 then there is no way
           to safely pend a transition to the DebugMon handler. */
        return 1;
    }
    else if (basepri != 0x00 && (basepri >> mriCortexMState.priorityBitShift) <= debugMonPriority)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void recordAndClearFaultStatusBits(uint32_t exceptionNumber)
{
    mriCortexMState.exceptionNumber = exceptionNumber;
    mriCortexMState.dfsr = SCB->DFSR;
    mriCortexMState.hfsr = SCB->HFSR;
    mriCortexMState.cfsr = SCB->CFSR;
    mriCortexMState.mmfar = SCB->MMFAR;
    mriCortexMState.bfar = SCB->BFAR;

    /* Clear fault status bits by writing 1s to bits that are already set. */
    SCB->DFSR = mriCortexMState.dfsr;
    SCB->HFSR = mriCortexMState.hfsr;
    SCB->CFSR = mriCortexMState.cfsr;
}

static void disableInterruptMaskingIfNecessary(void)
{
    /* When DebugMon is running at priority level 0 then can re-enable interrupts and pend DebugMon but must record
       the fact that PRIMASK was set so that it can be restored when leaving debugger. */
    uint32_t primask = __get_PRIMASK();
    uint32_t basepri = __get_BASEPRI();

    if (primask != 0)
    {
        mriCortexMState.primask = primask;
        mriCortexMState.basepri = basepri;
        setRestorePriorityRegsFlag();
        __set_PRIMASK(0);
    }
}

static void treatDebugEventHardFaultAsDebugMonInterrupt(void)
{
    static const uint32_t debugEventBit = 1 << 31;
    static const uint32_t debugMonExceptionNumber = 12;

    /* Treat as DebugMon interrupt if only the DEBUGEVT bit is set in the HFSR. */
    if (mriCortexMState.hfsr == debugEventBit)
    {
        mriCortexMState.exceptionNumber = debugMonExceptionNumber;
    }
}


static void setPendedFromFaultBit(void)
{
    mriCortexMFlags |= CORTEXM_FLAGS_PEND_FROM_FAULT;
}


__attribute__((weak)) void Platform_HandleFaultFromHighPriorityCode(void)
{
    /* This weak implemention does nothing and just returns to allow a jump directly to mriExceptionHandler to occur.
       A platform may want to provide a strong implementation to generate a crash dump in this scenario if MRI can't
       safely communicate with GDB from the HardFault priority level.
    */
}



static ExceptionStack* getExceptionStack(uint32_t excReturn, uint32_t psp, uint32_t msp);
static int wasPendedFromFault(void);
static int prepareThreadContext(ExceptionStack* pExceptionStack, IntegerRegisters* pIntegerRegs, uint32_t* pFloatingRegs);
static void allocateFakeFloatRegAndCallMriDebugException(void);
void mriCortexMExceptionHandler(IntegerRegisters* pIntegerRegs, uint32_t* pFloatingRegs)
{
    uint32_t excReturn = pIntegerRegs->excReturn;
    uint32_t msp = pIntegerRegs->msp;
    uint32_t psp = pIntegerRegs->psp;
    ExceptionStack* pExceptionStack = getExceptionStack(excReturn, psp, msp);
    int needToFakeFloatRegs = 0;

    /* If we know that DebugMon was pended from a fault handler then it wasn't a comm channel interrupt and there */
    /* is no need to record/clear the fault status bits as this was done before pending DebugMon. */
    if (!wasPendedFromFault())
    {
        uint32_t exceptionNumber = getCurrentlyExecutingExceptionNumber();
        if (isExternalInterrupt(exceptionNumber) && !Platform_CommHasReceiveData())
        {
            /* Just return if communication channel had a pending interrupt when last debug session completed. */
            return;
        }

        recordAndClearFaultStatusBits(exceptionNumber);
    }

    mriCortexMState.taskSP = (uint32_t)pExceptionStack;
    if (encounteredStackingException())
        pExceptionStack = (ExceptionStack*)g_fakeStack;

    /* Setup scatter gather list for context. */
    needToFakeFloatRegs = prepareThreadContext(pExceptionStack, pIntegerRegs, pFloatingRegs);

    /* Record some stats about the exception stack to help move it later if the user changes the SP. */
    uint32_t exceptionStackSize = mriCortexMState.sp - mriCortexMState.taskSP;
    int isExceptionStackMSP = msp == (uint32_t)pExceptionStack;
    int isExceptionStackPSP = psp == (uint32_t)pExceptionStack;
    uint32_t origSP = mriCortexMState.sp;

    if (needToFakeFloatRegs)
        allocateFakeFloatRegAndCallMriDebugException();
    else
        mriDebugException(&mriCortexMState.context);

    /* If exception stack has been modified by user then will need to move exception record. */
    if (mriCortexMState.sp != origSP)
    {
        if (isExceptionStackMSP)
        {
            Context_Set(&mriCortexMState.context, MSP, mriCortexMState.sp - exceptionStackSize);
        }
        if (isExceptionStackPSP)
        {
            Context_Set(&mriCortexMState.context, PSP, mriCortexMState.sp - exceptionStackSize);
        }
    }

    uint32_t newMSP = Context_Get(&mriCortexMState.context, MSP);
    uint32_t newPSP = Context_Get(&mriCortexMState.context, PSP);
    uint32_t oldSP = 0x00000000;
    uint32_t newSP = 0x00000000;
    if (isExceptionStackMSP && msp != newMSP)
    {
        oldSP = msp;
        newSP = newMSP;
    }
    else if (isExceptionStackPSP && psp != newPSP)
    {
        oldSP = psp;
        newSP = newPSP;
    }
    if (oldSP != newSP)
    {
        mri_memmove((void*)newSP, (void*)oldSP, exceptionStackSize);
    }
}

static int wasPendedFromFault(void)
{
    return mriCortexMFlags & CORTEXM_FLAGS_PEND_FROM_FAULT;
}

static int prepareThreadContext(ExceptionStack* pExceptionStack, IntegerRegisters* pIntegerRegs, uint32_t* pFloatingRegs)
{
    uint32_t excReturn = pIntegerRegs->excReturn;
    uint32_t entryCount = 0;
    size_t   fpuRegCount = (uint32_t*)pIntegerRegs - pFloatingRegs;

    uint32_t autoStackedFloats = 0;
    if (MRI_DEVICE_HAS_FPU && (excReturn & LR_FLOAT_STACK) == 0)
    {
        /* Auto stacked S0-S15, FPSCR, +1 extra word for alignment. */
        autoStackedFloats = 18;
    }
    uint32_t autoStackedRegs = 8 + autoStackedFloats + ((pExceptionStack->xpsr >> PSR_STACK_ALIGN_BIT_POS) & 1);

    /* R0 - R3 */
    g_contextEntries[0].pValues = &pExceptionStack->r0;
    g_contextEntries[0].count = 4;
    /* R4 - R11 */
    g_contextEntries[1].pValues = &pIntegerRegs->r4;
    g_contextEntries[1].count = 8;
    /* R12 */
    g_contextEntries[2].pValues = &pExceptionStack->r12;
    g_contextEntries[2].count = 1;
    /* SP - Point scatter gather context to correct location for SP but set it to correct value once CPSR is more easily
       fetched. */
    g_contextEntries[3].pValues = &mriCortexMState.sp;
    g_contextEntries[3].count = 1;
    /* LR, PC, CPSR */
    g_contextEntries[4].pValues = &pExceptionStack->lr;
    g_contextEntries[4].count = 3;
    /* MSP, PSP, PRIMASK, BASEPRI, FAULTMASK, CONTROL */
    g_contextEntries[5].pValues = &pIntegerRegs->msp;
    g_contextEntries[5].count = 6;
    /* Set SP to correct value using alignment bit in CPSR. Memory for SP is already tracked by context. */
    mriCortexMState.sp = (uint32_t)((uint32_t*)pExceptionStack + autoStackedRegs);
    entryCount = 6;

    if (MRI_DEVICE_HAS_FPU && fpuRegCount == 16)
    {
        /* S0 - S15 */
        g_contextEntries[6].pValues = &pExceptionStack->s0;
        g_contextEntries[6].count = 16;
        /* S16 - S31 */
        g_contextEntries[7].pValues = pFloatingRegs;
        g_contextEntries[7].count = 16;
        /* FPSCR */
        g_contextEntries[8].pValues = &pExceptionStack->fpscr;
        g_contextEntries[8].count = 1;

        entryCount += 3;
    }
    else if (MRI_DEVICE_HAS_FPU && fpuRegCount == 33)
    {
        /* S0 - S31 & FPSCR */
        g_contextEntries[6].pValues = pFloatingRegs;
        g_contextEntries[6].count = 33;

        entryCount += 1;
    }
    else if (MRI_DEVICE_HAS_FPU && fpuRegCount == 0)
    {
        /* Reserve an entry for zeroed out floating point registers that will be filled in later from stack. */
        entryCount += 1;
    }

    Context_Init(&mriCortexMState.context, g_contextEntries, entryCount);

    /* Return true if we need to allocate space for floating point registers on stack. */
    return (MRI_DEVICE_HAS_FPU && fpuRegCount == 0);
}

static void allocateFakeFloatRegAndCallMriDebugException(void)
{
    uint32_t fakeFloats[33];

    mri_memset(&fakeFloats, 0, sizeof(fakeFloats));
    g_contextEntries[6].pValues = fakeFloats;
    g_contextEntries[6].count = sizeof(fakeFloats)/sizeof(fakeFloats[0]);

    mriDebugException(&mriCortexMState.context);
}

#endif /* !MRI_THREAD_MRI */
