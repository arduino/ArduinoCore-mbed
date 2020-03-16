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
#define CORTEXM_FLAGS_CTRL_C                (1 << 5)

/* Constants related to special memory area used by the debugger for its stack so that it doesn't interfere with
   the task's stack contents. */
#if THREAD_MRI
    #define CORTEXM_DEBUGGER_STACK_SIZE            0
#else
    #define CORTEXM_DEBUGGER_STACK_SIZE            39
#endif
#define CORTEXM_DEBUGGER_STACK_SIZE_IN_BYTES   (CORTEXM_DEBUGGER_STACK_SIZE * 8)
#define CORTEXM_DEBUGGER_STACK_FILL            0xDEADBEEF

/* Offsets of fields within the CortexMState structure defined below.  These are used to access the fields of the
   structure from within assembly language code. */
#define CORTEXM_STATE_DEBUGGER_STACK_OFFSET 0
#define CORTEXM_STATE_FLAGS_OFFSET          (CORTEXM_STATE_DEBUGGER_STACK_OFFSET + CORTEXM_DEBUGGER_STACK_SIZE_IN_BYTES)
#define CORTEXM_STATE_TASK_SP_OFFSET        (CORTEXM_STATE_FLAGS_OFFSET + 4)

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
