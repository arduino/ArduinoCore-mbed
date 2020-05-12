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
/* In some other build systems, MRI_DEVICE_HAS_FPU won't be passed in on compiler's command line so use the
   target Cortex-M type to determine if it has a FPU or not.
*/
#ifndef MRI_DEVICE_HAS_FPU
    #ifdef __ARM_ARCH_7EM__
        #define MRI_DEVICE_HAS_FPU 1
    #else
        #define MRI_DEVICE_HAS_FPU 0
    #endif
#endif

/* Flag bits used in mriCortexMFlags global. */
#define CORTEXM_FLAGS_ACTIVE_DEBUG          (1 << 0)
#define CORTEXM_FLAGS_FAULT_DURING_DEBUG    (1 << 1)
#define CORTEXM_FLAGS_SINGLE_STEPPING       (1 << 2)
#define CORTEXM_FLAGS_RESTORE_BASEPRI       (1 << 3)
#define CORTEXM_FLAGS_SVC_STEP              (1 << 4)
#define CORTEXM_FLAGS_CTRL_C                (1 << 5)

/* Special memory area used by the debugger for its stack so that it doesn't interfere with the task's
   stack contents.

   The stack sizes below are based on actual test runs on LPC1768 (non-FPU) and LPC4330 (FPU) based boards with a
   roughly 25% reservation for future growth.
*/
#define CORTEXM_DEBUGGER_STACK_FILL            0xDEADBEEF
#if MRI_DEVICE_HAS_FPU
    #define CORTEXM_DEBUGGER_STACK_SIZE        193
#else
    #define CORTEXM_DEBUGGER_STACK_SIZE        58
#endif



/* Definitions only required from C code. */
#if !__ASSEMBLER__

#include <stdint.h>
#include <core/token.h>
#include <core/context.h>
#include <core/platforms.h>


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
#define BASEPRI 20


#if MRI_THREAD_MRI
    #define SPECIAL_REGISTER_COUNT      0
#else
    #define SPECIAL_REGISTER_COUNT      6
#endif
#if MRI_DEVICE_HAS_FPU
    #define CONTEXT_SIZE    (17 + 33 + SPECIAL_REGISTER_COUNT)
#else
    #define CONTEXT_SIZE    (17 + SPECIAL_REGISTER_COUNT)
#endif

/* NOTE: The largest buffer is required for receiving the 'G' command which receives the contents of the registers from
   the debugger as two hex digits per byte.  Also need a character for the 'G' command itself. */
#define CORTEXM_PACKET_BUFFER_SIZE  (1 + 2 * sizeof(uint32_t) * CONTEXT_SIZE)

typedef struct
{
    MriContext          context;
    PlatformTrapReason  reason;
    uint32_t            taskSP;
    uint32_t            sp;
    uint32_t            exceptionNumber;
    uint32_t            dfsr;
    uint32_t            hfsr;
    uint32_t            cfsr;
    uint32_t            mmfar;
    uint32_t            bfar;
    uint32_t            originalPC;
    uint32_t            originalBasePriority;
    uint32_t            subPriorityBitCount;
    int                 maxStackUsed;
    char                packetBuffer[CORTEXM_PACKET_BUFFER_SIZE];
} CortexMState;

extern uint64_t             mriCortexMDebuggerStack[CORTEXM_DEBUGGER_STACK_SIZE];
extern volatile uint32_t    mriCortexMFlags;
extern CortexMState         mriCortexMState;

void    mriCortexMInit(Token* pParameterTokens, uint8_t debugMonPriority, IRQn_Type highestExternalIrq);
void    mriCortexMSetPriority(IRQn_Type irq, uint8_t priority, uint8_t subPriority);
uint8_t mriCortexMGetPriority(IRQn_Type irq);


#endif /* !__ASSEMBLER__ */

#endif /* CORTEXM_H_ */
