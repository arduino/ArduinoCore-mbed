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
/* Routines and globals which expose the Cortex-M functionality to the mri debugger.  Also describes global architecture
  state which is shared between cortex-m.c and cortex-m_asm.S */
#ifndef CORTEXM_H_
#define CORTEXM_H_

/* Definitions used by C and Assembler code. */
/* Flag bits used in CortexMState::flags field. */
#define CORTEXM_FLAGS_ACTIVE_DEBUG          (1 << 0)
#define CORTEXM_FLAGS_FAULT_DURING_DEBUG    (1 << 1)
#define CORTEXM_FLAGS_SINGLE_STEPPING       (1 << 2)
#define CORTEXM_FLAGS_RESTORE_BASEPRI       (1 << 3)
#define CORTEXM_FLAGS_SVC_STEP              (1 << 4)

// UNDONE: Stack size can be 0 for ThreadMRI.
/* Constants related to special memory area used by the debugger for its stack so that it doesn't interfere with
   the task's stack contents. */
#define CORTEXM_DEBUGGER_STACK_SIZE            39
#define CORTEXM_DEBUGGER_STACK_SIZE_IN_BYTES   (CORTEXM_DEBUGGER_STACK_SIZE * 8)
#define CORTEXM_DEBUGGER_STACK_FILL            0xDEADBEEF

/* Offsets of fields within the CortexMState structure defined below.  These are used to access the fields of the
   structure from within assembly language code. */
#define CORTEXM_STATE_DEBUGGER_STACK_OFFSET 0
#define CORTEXM_STATE_FLAGS_OFFSET          (CORTEXM_STATE_DEBUGGER_STACK_OFFSET + CORTEXM_DEBUGGER_STACK_SIZE_IN_BYTES)
#define CORTEXM_STATE_TASK_SP_OFFSET        (CORTEXM_STATE_FLAGS_OFFSET + 4)
// UNDONE: These were moved.
//#define CORTEXM_STATE_CONTEXT_OFFSET        (CORTEXM_STATE_TASK_SP_OFFSET + 4)
//#define CORTEXM_STATE_SAVED_MSP_OFFSET      (CORTEXM_STATE_CONTEXT_OFFSET + 17 * 4)

// In some other build systems, MRI_DEVICE_HAS_FPU won't be passed in on compiler's command line so use the
// target Cortex-M type to determine if it has a FPU or not.
#ifndef MRI_DEVICE_HAS_FPU
    #ifdef __ARM_ARCH_7EM__
        #define MRI_DEVICE_HAS_FPU 1
    #else
        #define MRI_DEVICE_HAS_FPU 0
    #endif
#endif

// UNDONE:
#define MRI_THREAD_MRI 1

/* Give friendly names to the indices of important registers in the context scatter gather list. */
#define R0      0
#define R1      1
#define R2      2
#define R3      3
#define R7      7
#define SP      13
#define LR      14
#define PC      15
#define CPSR    16


/* Definitions only required from C code. */
#if !__ASSEMBLER__

#include <stdint.h>
#include <core/token.h>
#include <core/scatter_gather.h>

// UNDONE: Need to get ASM code working with new scatter gather context format.
#ifdef UNDONE
/* NOTE: The MriExceptionHandler function definition in mriasm.S is dependent on the layout of this structure.  It
         is also dictated by the version of gdb which supports the ARM processors.  It should only be changed if the
         gdb ARM support code is modified and then the context saving and restoring code will need to be modified to
         use the correct offsets as well.
*/
typedef struct
{
    uint32_t    R0;
    uint32_t    R1;
    uint32_t    R2;
    uint32_t    R3;
    uint32_t    R4;
    uint32_t    R5;
    uint32_t    R6;
    uint32_t    R7;
    uint32_t    R8;
    uint32_t    R9;
    uint32_t    R10;
    uint32_t    R11;
    uint32_t    R12;
    uint32_t    SP;
    uint32_t    LR;
    uint32_t    PC;
    uint32_t    CPSR;
    uint32_t    MSP;
    uint32_t    PSP;
    uint32_t    PRIMASK;
    uint32_t    BASEPRI;
    uint32_t    FAULTMASK;
    uint32_t    CONTROL;
#if MRI_DEVICE_HAS_FPU
    uint32_t    S0;
    uint32_t    S1;
    uint32_t    S2;
    uint32_t    S3;
    uint32_t    S4;
    uint32_t    S5;
    uint32_t    S6;
    uint32_t    S7;
    uint32_t    S8;
    uint32_t    S9;
    uint32_t    S10;
    uint32_t    S11;
    uint32_t    S12;
    uint32_t    S13;
    uint32_t    S14;
    uint32_t    S15;
    uint32_t    S16;
    uint32_t    S17;
    uint32_t    S18;
    uint32_t    S19;
    uint32_t    S20;
    uint32_t    S21;
    uint32_t    S22;
    uint32_t    S23;
    uint32_t    S24;
    uint32_t    S25;
    uint32_t    S26;
    uint32_t    S27;
    uint32_t    S28;
    uint32_t    S29;
    uint32_t    S30;
    uint32_t    S31;
    uint32_t    FPSCR;
#endif
} Context;
#endif // UNDONE

// UNDONE: In ThreadMRI, the context entry count would be dependent on the RTOS.
// UNDONE: Might make more sense to pass the scatter gather context into mriDebugException.
#if MRI_THREAD_MRI
    #define SPECIAL_REGISTER_ENTRIES    0
    #define SPECIAL_REGISTER_COUNT      0
#else
    #define SPECIAL_REGISTER_ENTRIES    1
    #define SPECIAL_REGISTER_COUNT      6
#endif
#if MRI_DEVICE_HAS_FPU
    #define CONTEXT_ENTRIES (5 + 3 + SPECIAL_REGISTER_ENTRIES)
    #define CONTEXT_SIZE    (17 + 33 + SPECIAL_REGISTER_COUNT)
#else
    #define CONTEXT_ENTRIES (5 + SPECIAL_REGISTER_ENTRIES)
    #define CONTEXT_SIZE    (17 + SPECIAL_REGISTER_COUNT)
#endif

/* NOTE: The largest buffer is required for receiving the 'G' command which receives the contents of the registers from
   the debugger as two hex digits per byte.  Also need a character for the 'G' command itself. */
#define CORTEXM_PACKET_BUFFER_SIZE  (1 + 2 * sizeof(uint32_t) * CONTEXT_SIZE)

typedef struct
{
    uint64_t            debuggerStack[CORTEXM_DEBUGGER_STACK_SIZE];
    volatile uint32_t   flags;
    volatile uint32_t   taskSP;
    ScatterGatherEntry  contextEntries[CONTEXT_ENTRIES];
    ScatterGather       context;
    uint32_t            sp;
    uint32_t            exceptionNumber;
    uint32_t            dfsr;
    uint32_t            hfsr;
    uint32_t            cfsr;
    uint32_t            mmfar;
    uint32_t            bfar;
    uint32_t            originalPC;
    uint32_t            originalBasePriority;
    int                 maxStackUsed;
    char                packetBuffer[CORTEXM_PACKET_BUFFER_SIZE];
} CortexMState;

extern CortexMState     mriCortexMState;
extern const uint32_t   mriCortexMFakeStack[8];

void     mriCortexMInit(Token* pParameterTokens);

#endif /* !__ASSEMBLER__ */


#endif /* CORTEXM_H_ */
