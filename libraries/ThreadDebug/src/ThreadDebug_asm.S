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
/* Implementation of Cortex-M architecture assembly language routines to be used by ThreadMRI debug monitor. */
    .text
    .code 16
    .syntax unified


    /* Lower nibble of EXC_RETURN in LR will have of these values if interrupted code was running in thread mode. */
    .equ EXC_RETURN_THREADMODE_PROCESSSTACK, 0xD
    .equ EXC_RETURN_THREADMODE_MAINSTACK, 0x9

    /* Hard Fault Status Register */
    .equ HFSR, 0xE000ED2C
    /* This bit is set in HFSR if a hard fault was forced by debug event occuring in handler mode. */
    .equ HFSR_DEBUG_EVENT_BIT,  (1 << 31)

    /* System register and bits for pending DebugMon interrupt. */
    .equ DEMCR, 0xE000EDFC
    .equ DEMCR_MON_PEND_BIT, (1 << 17)



    .global mriDebugMonitorHandlerStub
    .type mriDebugMonitorHandlerStub, function
    .thumb_func
    /* extern "C" void mriDebugMonitorHandlerStub(void);
       This stub is called whenever a DebugMon interrupt fires. It passes the EXC_RETURN value from LR into the real
       mriDebugMonitorHandler() so that it can be queried to determine if thread or handler mode code caused the
       interrupt.
    */
mriDebugMonitorHandlerStub:
    mov     r0, lr
    b.w     mriDebugMonitorHandler
    .pool
    .size   mriDebugMonitorHandlerStub, .-mriDebugMonitorHandlerStub


    .global mriFaultHandlerStub
    .type mriFaultHandlerStub, function
    .thumb_func
    /* extern "C" void mriFaultHandlerStub(void);
       This stub is called whenever a fault fires w/ R3 set to the original handler for the particular fault. This
       allows the original handler to be called if a crash occurs in handler mode which can't be debugged by ThreadMRI.
       It checks the EXC_RETURN value to determine if thread or handler mode caused fault:
        Thread: Pass control to mriFaultHandler() so that it can be debugged.
        Handler: If caused by debug event, pass control to mriFaultHandler() so that break/watchpoints can be disabled.
                 Otherwise pass control to the standard mbed-os HardFault_Handler so that crash can be dumped.
    */
mriFaultHandlerStub:
    mov     r0, lr
    mrs     r1, psp
    mrs     r2, msp
    /* Jump to mriFaultHandler if interrupted code was running in thread mode. */
    and     r12, r0, #0xF
    cmp     r12, #EXC_RETURN_THREADMODE_PROCESSSTACK
    beq.w   mriFaultHandler
    cmp     r12, #EXC_RETURN_THREADMODE_MAINSTACK
    beq.w   mriFaultHandler
    /* Get here if interrupted code was running in handler mode. */
    /* Check HardFaultStatusRegister to see if this is a debug event. */
    ldr     r12, =HFSR
    ldr     r12, [r12]
    tst     r12, #HFSR_DEBUG_EVENT_BIT
    /* If forced into hard fault due to debug event then breakpoint/watchpoint occurred and needs to be ignored. */
    bne.w   mriFaultHandler
    /* If not a debug event then fall through to default hard fault handler as something bad has happened. */
    mov     pc, r3
    .pool
    .size   mriFaultHandlerStub, .-mriFaultHandlerStub


    .global mriHardFaultHandlerStub
    .type mriHardFaultHandlerStub, function
    .thumb_func
    /* extern "C" void mriHardFaultHandlerStub(void);
       This stub is called whenever a Hard Fault fires.
    */
mriHardFaultHandlerStub:
    /* Load address of real fault handler into r3 & call generic fault stub. */
    ldr     r3, =mriThreadOrigHardFault
    ldr     r3, [r3]
    b       mriFaultHandlerStub
    .pool
    .size   mriHardFaultHandlerStub, .-mriHardFaultHandlerStub


    .global mriMemManagementHandlerStub
    .type mriMemManagementHandlerStub, function
    .thumb_func
    /* extern "C" void mriMemManagementHandlerStub(void);
       This stub is called whenever a MemManagement fault fires.
    */
mriMemManagementHandlerStub:
    /* Load address of real fault handler into r3 & call generic fault stub. */
    ldr     r3, =mriThreadOrigMemManagement
    ldr     r3, [r3]
    b       mriFaultHandlerStub
    .pool
    .size   mriMemManagementHandlerStub, .-mriMemManagementHandlerStub


    .global mriBusFaultHandlerStub
    .type mriBusFaultHandlerStub, function
    .thumb_func
    /* extern "C" void mriBusFaultHandlerStub(void);
       This stub is called whenever a Bus Fault fires.
    */
mriBusFaultHandlerStub:
    /* Load address of real fault handler into r3 & call generic fault stub. */
    ldr     r3, =mriThreadOrigBusFault
    ldr     r3, [r3]
    b       mriFaultHandlerStub
    .pool
    .size   mriBusFaultHandlerStub, .-mriBusFaultHandlerStub


    .global mriUsageFaultHandlerStub
    .type mriUsageFaultHandlerStub, function
    .thumb_func
    /* extern "C" void mriUsageFaultHandlerStub(void);
       This stub is called whenever a Usage Fault fires.
    */
mriUsageFaultHandlerStub:
    /* Load address of real fault handler into r3 & call generic fault stub. */
    ldr     r3, =mriThreadOrigUsageFault
    ldr     r3, [r3]
    b       mriFaultHandlerStub
    .pool
    .size   mriUsageFaultHandlerStub, .-mriUsageFaultHandlerStub




    .global mriRTXHandlerStub
    .type mriRTXHandlerStub, function
    .thumb_func
    /* extern "C" void mriRTXHandlerStub(void (*pOrigHandler)(void));
       This stub pends a DebugMon interrupt whenever a RTX interrupt occurs that could perform a context switch so
       that it has a chance to enable single stepping if switching to the thread to be single stepped.
    */
mriRTXHandlerStub:
    /* Need to queue up a DebugMon interrupt to enable single stepping when switching to thread of interest. */
    ldr     r1, =DEMCR
    ldr     r2, [r1]
    orr     r2, #DEMCR_MON_PEND_BIT
    str     r2, [r1]
    /* Run actual RTX handler. */
1$: mov     pc, r0
    .pool
    .size   mriRTXHandlerStub, .-mriRTXHandlerStub


    .global mriSVCHandlerStub
    .type mriSVCHandlerStub, function
    .thumb_func
    /* extern "C" void mriSVCHandlerStub(void);
       This stub pends a DebugMon interrupt whenever a SVCall interrupt occurs that could perform a context switch so
       that it has a chance to enable single stepping if switching to the thread to be single stepped.
    */
mriSVCHandlerStub:
    /* Load address of real RTX handler into r0 & call generic stub to pend DebugMon for single stepping. */
    ldr     r0, =mriThreadOrigSVCall
    ldr     r0, [r0]
    b       mriRTXHandlerStub
    .pool
    .size   mriSVCHandlerStub, .-mriSVCHandlerStub


    .global mriPendSVHandlerStub
    .type mriPendSVHandlerStub, function
    .thumb_func
    /* extern "C" void mriPendSVHandlerStub(void);
       This stub pends a DebugMon interrupt whenever a PendSV interrupt occurs that could perform a context switch so
       that it has a chance to enable single stepping if switching to the thread to be single stepped.
    */
mriPendSVHandlerStub:
    /* Load address of real RTX handler into r0 & call generic stub to pend DebugMon for single stepping. */
    ldr     r0, =mriThreadOrigPendSV
    ldr     r0, [r0]
    b       mriRTXHandlerStub
    .pool
    .size   mriPendSVHandlerStub, .-mriPendSVHandlerStub


    .global mriSysTickHandlerStub
    .type mriSysTickHandlerStub, function
    .thumb_func
    /* extern "C" void mriSysTickHandlerStub(void);
       This stub pends a DebugMon interrupt whenever a SysTick interrupt occurs that could perform a context switch so
       that it has a chance to enable single stepping if switching to the thread to be single stepped.
    */
mriSysTickHandlerStub:
    /* Load address of real RTX handler into r0 & call generic stub to pend DebugMon for single stepping. */
    ldr     r0, =mriThreadOrigSysTick
    ldr     r0, [r0]
    b       mriRTXHandlerStub
    .pool
    .size   mriSysTickHandlerStub, .-mriSysTickHandlerStub

    .end
