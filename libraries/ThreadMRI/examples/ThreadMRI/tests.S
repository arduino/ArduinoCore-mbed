/* Copyright (C) 2020  Adam Green (https://github.com/adamgreen)

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
/* Implementation of code to place known values in registers to verify context reading code. */
    .text
    .syntax unified

    .global testContext
    .type testContext, %function
    .thumb_func
    /* extern "C" void testContext(void);
       Sets all of the registers to a known value to make sure that context is being read correctly.
    */
testContext:
    // Load known values into floating point registers.
    vmov.f32    s0, #-1.0
    vmov.f32    s1, #1.0
    vmov.f32    s2, #2.0
    vmov.f32    s3, #3.0
    vmov.f32    s4, #4.0
    vmov.f32    s5, #5.0
    vmov.f32    s6, #6.0
    vmov.f32    s7, #7.0
    vmov.f32    s8, #8.0
    vmov.f32    s9, #9.0
    vmov.f32    s10, #10.0
    vmov.f32    s11, #11.0
    vmov.f32    s12, #12.0
    vmov.f32    s13, #13.0
    vmov.f32    s14, #14.0
    vmov.f32    s15, #15.0
    vmov.f32    s16, #16.0
    vmov.f32    s17, #17.0
    vmov.f32    s18, #18.0
    vmov.f32    s19, #19.0
    vmov.f32    s20, #20.0
    vmov.f32    s21, #21.0
    vmov.f32    s22, #22.0
    vmov.f32    s23, #23.0
    vmov.f32    s24, #24.0
    vmov.f32    s25, #25.0
    vmov.f32    s26, #26.0
    vmov.f32    s27, #27.0
    vmov.f32    s28, #28.0
    vmov.f32    s29, #29.0
    vmov.f32    s30, #30.0
    vmov.f32    s31, #31.0
    ldr         r0, =0xBAADFEED
    vmsr        fpscr, r0
    // Load known values into R0-R12
    mov     r0, #0
    mov     r1, #1
    mov     r2, #2
    mov     r3, #3
    mov     r4, #4
    mov     r5, #5
    mov     r6, #6
    mov     r7, #7
    mov     r8, #8
    mov     r9, #9
    mov     r10, #10
    mov     r11, #11
    mov     r12, #12
    // Crash on read from invalid memory.
    ldr     r0, =0xFFFFFFF0
    ldr     r0, [r0]
    // Hardcoded breakpoint.
    bkpt    #0
    // Infinite loop.
    b       .
    // Let assembler know that we have hit the end of the function.
    .pool
    .size   testContext, .-testContext


    .end