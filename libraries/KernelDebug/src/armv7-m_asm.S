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
/* Implementation of Cortex-M architecture assembly language routines to be used by mri debug monitor. */
    .text
    .code 16
    .syntax unified

    #include <architectures/armv7-m/armv7-m.h>


    /* Bit in LR set to 0 when automatic stacking of floating point registers occurs during exception handling. */
    .equ LR_FLOAT_STACK,         (1 << 4)

    /* Coprocessor Access Control Register. */
    .equ CPACR,                 0xE000ED88
    /* Bits set in CPACR if FPU is enabled. */
    .equ CPACR_FPU_BITS,        (5 << 20)


    .global mriExceptionHandler
    .type mriExceptionHandler, function
    .thumb_func
    /* extern "C" void mriExceptionHandler(void);
       This is the main entry point into MRI whenever a debug event or fault occurs.
       It switches MSP to mriCortexMDebuggerStack and pushes onto it all of the non-volatile registers not auto stacked
       by the CPU on exception entry in the following order:
            ** If FPU enabled & FPU registers not auto-stacked on exception entry then following 16 registers pushed:
            S0
            S1
            S2
            S3
            S4
            S5
            S6
            S7
            S8
            S9
            S10
            S11
            S12
            S13
            S14
            S15
            ** If FPU enabled then the following 16 registers are always pushed:
            S16
            S17
            S18
            S19
            S20
            S21
            S22
            S23
            S24
            S25
            S26
            S27
            S28
            S29
            S30
            S31
            ** If FPU enabled & FPU registers not auto-stacked on exception entry then following register is pushed:
            FPSCR
            ** These special registers and integer registers are always stacked:
            MSP
            PSP
            PRIMASK
            BASEPRI
            FAULTMASK
            CONTROL
            R4
            R5
            R6
            R7
            R8
            R9
            R10
            R11
            EXC_RETURN (exception LR)

       Once the registers are stacked, mriDebugException is called and upon return, the values are popped off of the
       stack and placed back into the original registers, except for PSP, PRIMASK, FAULTMASK, and CONTROL which always
       maintain their original value.
    */
mriExceptionHandler:
    /* Check to see if we should just return because a MRI initiated memory read/write failed. */
    ldr     r0, =mriCortexMFlags
    ldr     r0, [r0]
    tst     r0, #CORTEXM_FLAGS_ACTIVE_DEBUG
    beq     1$
    /* MRI has initiated a memory read/write that has failed. Unless MRI has a critical bug, it is safe to use MSP
       for calling C code at this point to take care of setting the flag to indicate that the read/write has failed
       and advanced the PC past precise read faulting instructions. */
    mov     r0, lr
    mrs     r1, psp
    mrs     r2, msp
    push    {r1, lr}
    bl      mriCortexHandleDebuggerFault
    pop     {r1, pc}

    /* Point R0 to the top of the debugger stack, where MSP should be pointed. */
1$: ldr     r0, =mriCortexMDebuggerStack
    ldr     r1, =CORTEXM_DEBUGGER_STACK_SIZE * 8
    add     r0, r0, r1

    /* Save away copy of MSP value at the time of the fault into R0. */
3$: mrs     r1, msp
    /* Set MSP to top of debugger stack. */
    msr     msp, r0

    /* Saving integer registers to debugger stack. */
    /* R4 - R11, EXC_RETURN */
    push    {r4-r11, lr}
    /* MSP, PSP, PRIMASK, BASEPRI, FAULTMASK, CONTROL */
    mov     r4, r1      /* MSP was saved away in R0 previously. */
    mrs     r5, psp
    mrs     r6, primask
    mrs     r7, basepri
    mrs     r8, faultmask
    mrs     r9, control
    push    {r4-r9}

    /* Will pass start of integer registers into mriCortexMExceptionHandler via R0. */
    mrs     r0, msp

    /* Saving floating point registers to debugger stack. */
#if MRI_DEVICE_HAS_FPU
    /* Were S0-S15 and FPSCR auto-stacked upon exception entry? */
    tst     lr, #LR_FLOAT_STACK
    bne     4$
    /* Get here if S0-S15 and FPSCR were auto-stacked. */
    /* Push S16-S31 */
    vpush.32  {s16-s31}
    b       5$
    /* Get here is no auto-stacking of FPU registers occurred. */
    /* If FPU is enabled then manually stack S0-S31 and FPSCR. */
    /* Push S0-S31, FPSCR */
4$: ldr     r1, =CPACR
    ldr     r1, [r1]
    and     r1, #CPACR_FPU_BITS
    cmp     r1, #CPACR_FPU_BITS
    bne     5$  /* Stack no FP registers if FPU is disabled. */
    vmrs    r1, fpscr
    push    {r1}
    vpush.32 {s16-s31}
    vpush.32 {s0-s15}
    b       5$
    /* Fall through to 5$ */
#endif /* MRI_DEVICE_HAS_FPU */

    /* All registers integer and float have been pushed on stack. */
    /* Will pass start of integer registers into mriCortexMExceptionHandler via R1. */
5$: mrs     r1, msp

    /* Save pointer to integer regs in the R4 non-volatile register. */
    mov     r4, r0

    /* Let C code take care of calling mriDebugException. */
    /* mriCortexMExceptionHandler(R0=pIntegerRegs, R1=pFloatingRegs); */
    bl      mriCortexMExceptionHandler

    /* Unstack floating point registers. */
#if MRI_DEVICE_HAS_FPU
    /* R4 = MSP value where integer registers were stacked. Comparing to current MSP indicates how many floating point
       registers need to be unstacked now.
    */
    mrs     r1, msp
    sub     r1, r4, r1
    lsr     r1, #2
    /* If there are more than 16 FP registers to unstack then unstack S0-S15. */
    cmp     r1, #16
    it      hi
    vpophi.32 {s0-s15}
    /* If there are more than 0 FP registers to unstack then unstack S16-S31. */
    cbz     r1, 6$
    vpop.32   {s16-s31}
    /* If there are more than 16 FP registers to unstack then unstack FPSCR. */
    it      hi
    pophi   {r2}
    vmsr    fpscr, r2
#endif /* MRI_DEVICE_HAS_FPU */

    /* Unstack integer registers. */
6$: pop     {r4-r9}
    msr     basepri, r7
    mov     r0, r4  /* R0 = original MSP */
    /* Just drop the PSP value since updating it could really corrupt things. */

    /* R4 - R11, EXC_RETURN */
    pop     {r4-r11, lr}

    /* Restore MSP to what it was upon exception entry. */
    msr     msp, r0

    /* Return to halted code. */
    bx      lr
    .pool
    .size   mriExceptionHandler, .-mriExceptionHandler



#if 0
    .global HardFault_Handler
    .type HardFault_Handler, function
    .thumb_func
    /* extern "C" void HardFault_Handler(void);
       Override Hard Faults and send to MriExceptionHandler.
    */
HardFault_Handler:
    b       mriExceptionHandler
    .pool
    .size   HardFault_Handler, .-HardFault_Handler


    .global MemManage_Handler
    .type MemManage_Handler, function
    .thumb_func
    /* extern "C" void MemManage_Handler(void);
       Override MPU Memory Faults and send to MriExceptionHandler.
    */
MemManage_Handler:
    b       mriExceptionHandler
    .pool
    .size   MemManage_Handler, .-MemManage_Handler


    .global BusFault_Handler
    .type BusFault_Handler, function
    .thumb_func
    /* extern "C" void BusFault_Handler(void);
       Override Bus Faults and send to MriExceptionHandler.
    */
BusFault_Handler:
    b       mriExceptionHandler
    .pool
    .size   BusFault_Handler, .-BusFault_Handler


    .global UsageFault_Handler
    .type UsageFault_Handler, function
    .thumb_func
    /* extern "C" void UsageFault_Handler(void);
       Override Instruction Usage Faults and send to MriExceptionHandler.
    */
UsageFault_Handler:
    b       mriExceptionHandler
    .pool
    .size   UsageFault_Handler, .-UsageFault_Handler
#endif // 0


    .global DebugMon_Handler
    .type DebugMon_Handler, function
    .thumb_func
    /* extern "C" void DebugMon_Handler(void);
       Override Debug Monintor exceptions and send to MriExceptionHandler.
    */
DebugMon_Handler:
    b       mriExceptionHandler
    .pool
    .size   DebugMon_Handler, .-DebugMon_Handler


    .end
