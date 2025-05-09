/*
 * FreeRTOS Kernel V10.6.2
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef PORTCONTEXT_H
#define PORTCONTEXT_H

#if __riscv_xlen == 64
    #define portWORD_SIZE 8
    #define store_x sd
    #define load_x ld
#elif __riscv_xlen == 32
    #define store_x sw
    #define load_x lw
    #define portWORD_SIZE 4
#else
    #error Assembler did not define __riscv_xlen
#endif

#include "freertos_risc_v_chip_specific_extensions.h"

/* Only the standard core registers are stored by default.  Any additional
 * registers must be saved by the portasmSAVE_ADDITIONAL_REGISTERS and
 * portasmRESTORE_ADDITIONAL_REGISTERS macros - which can be defined in a chip
 * specific version of freertos_risc_v_chip_specific_extensions.h.  See the
 * notes at the top of portASM.S file. */
#ifdef __riscv_32e
    #define portCONTEXT_SIZE ( 15 * portWORD_SIZE )
    #define portCRITICAL_NESTING_OFFSET 13
    #define portMSTATUS_OFFSET  14
#else
    #define portCONTEXT_SIZE ( 30 * portWORD_SIZE )
    //#define portCRITICAL_NESTING_OFFSET 29
    #define portMSTATUS_OFFSET  29
#endif

/*-----------------------------------------------------------*/

.extern pxCurrentTCB
.extern xISRStackTop
//.extern xCriticalNesting
//.extern pxCriticalNesting

/* stat trap variable */
.extern vCountLastTrapCost
.extern ulLastTrapCount
.extern lLastTrapNum
.extern ulSwitchOutExtra
.extern ulSwitchInExtra

/*-----------------------------------------------------------*/

.macro portcontextSAVE_CONTEXT_INTERNAL
    addi sp, sp, -portCONTEXT_SIZE
    store_x x1, 1 * portWORD_SIZE( sp )

#ifdef CONFIG_PS_EXTEND
    /* save trapin count, use 1 register at least */
    rdcycle x1
    csrw mscratch, x1
#endif

    store_x x5, 2 * portWORD_SIZE( sp )
    store_x x6, 3 * portWORD_SIZE( sp )
    store_x x7, 4 * portWORD_SIZE( sp )
    store_x x8, 5 * portWORD_SIZE( sp )
    store_x x9, 6 * portWORD_SIZE( sp )
    store_x x10, 7 * portWORD_SIZE( sp )
    store_x x11, 8 * portWORD_SIZE( sp )
    store_x x12, 9 * portWORD_SIZE( sp )
    store_x x13, 10 * portWORD_SIZE( sp )
    store_x x14, 11 * portWORD_SIZE( sp )
    store_x x15, 12 * portWORD_SIZE( sp )
#ifndef __riscv_32e
    store_x x16, 13 * portWORD_SIZE( sp )
    store_x x17, 14 * portWORD_SIZE( sp )
    store_x x18, 15 * portWORD_SIZE( sp )
    store_x x19, 16 * portWORD_SIZE( sp )
    store_x x20, 17 * portWORD_SIZE( sp )
    store_x x21, 18 * portWORD_SIZE( sp )
    store_x x22, 19 * portWORD_SIZE( sp )
    store_x x23, 20 * portWORD_SIZE( sp )
    store_x x24, 21 * portWORD_SIZE( sp )
    store_x x25, 22 * portWORD_SIZE( sp )
    store_x x26, 23 * portWORD_SIZE( sp )
    store_x x27, 24 * portWORD_SIZE( sp )
    store_x x28, 25 * portWORD_SIZE( sp )
    store_x x29, 26 * portWORD_SIZE( sp )
    store_x x30, 27 * portWORD_SIZE( sp )
    store_x x31, 28 * portWORD_SIZE( sp )
#endif

    //load_x  t0, xCriticalNesting         /* Load the value of xCriticalNesting into t0. */
    //store_x t0, portCRITICAL_NESTING_OFFSET * portWORD_SIZE( sp ) /* Store the critical nesting value to the stack. */

#ifdef CONFIG_PS_EXTEND
    /* stat last trap cost */
    la t0, vCountLastTrapCost
    jalr t0
    /* save uLastTrapCount cause to lLastTrapNum */
    csrr  a0, mcause
    li t0, 0x80000FFF
    and a0, a0, t0
    la t0, lLastTrapNum
    store_x a0, (t0)
#endif

    csrr t0, mstatus                     /* Required for MPIE bit. */
    store_x t0, portMSTATUS_OFFSET * portWORD_SIZE( sp )


    portasmSAVE_ADDITIONAL_REGISTERS     /* Defined in freertos_risc_v_chip_specific_extensions.h to save any registers unique to the RISC-V implementation. */

    load_x  t0, pTrapNetCounter
    lw   t1, 0( t0 )
    addi   t1, t1, 1
    store_x t1, 0( t0 )
#ifdef CONFIG_HIGH_ISR_STACK
    li t0, 2
    bgeu t1, t0, 1f
    load_x  t0, pxCurrentTCB
    j 2f
1:
    load_x  t0, pLowIrqContext
2:
    store_x  sp, 0( t0 )
#else
    load_x  t0, pxCurrentTCB
    store_x  sp, 0( t0 )
#endif

    .endm
/*-----------------------------------------------------------*/

.macro portcontextSAVE_EXCEPTION_CONTEXT
    portcontextSAVE_CONTEXT_INTERNAL
    csrr a0, mcause
    csrr a1, mepc
    addi a1, a1, 4                      /* Synchronous so update exception return address to the instruction after the instruction that generated the exception. */
    store_x a1, 0( sp )                 /* Save updated exception return address. */
    load_x sp, xISRStackTop             /* Switch to ISR stack. */
    .endm
/*-----------------------------------------------------------*/

.macro portcontextSAVE_INTERRUPT_CONTEXT
    portcontextSAVE_CONTEXT_INTERNAL
    csrr a0, mcause
    csrr a1, mepc
    store_x a1, 0( sp )                 /* Asynchronous interrupt so save unmodified exception return address. */
    load_x sp, xISRStackTop             /* Switch to ISR stack. */
    .endm
/*-----------------------------------------------------------*/

.macro portcontextRESTORE_CONTEXT
    load_x  t0, pTrapNetCounter
    lw   t1, 0 ( t0 )
    addi   t1, t1, -1
    store_x t1, 0( t0 )
#ifdef CONFIG_HIGH_ISR_STACK
    bnez    t1, 1f
    load_x  t0, pxCurrentTCB
    j 2f
1:
    load_x  t0, pLowIrqContext
2:
    load_x  sp, 0( t0 )
#else
    load_x  t1, pxCurrentTCB
    load_x  sp, 0( t1 )
#endif

    /* Load mepc with the address of the instruction in the task to run next. */
    load_x t0, 0( sp )
    csrw mepc, t0

    /* Defined in freertos_risc_v_chip_specific_extensions.h to restore any registers unique to the RISC-V implementation. */
    portasmRESTORE_ADDITIONAL_REGISTERS

    /* Load mstatus with the interrupt enable bits used by the task. */
    load_x  t0, portMSTATUS_OFFSET * portWORD_SIZE( sp )
    csrw mstatus, t0                        /* Required for MPIE bit. */

    //load_x  t0, portCRITICAL_NESTING_OFFSET * portWORD_SIZE( sp )    /* Obtain xCriticalNesting value for this task from task's stack. */
    //load_x  t1, pxCriticalNesting           /* Load the address of xCriticalNesting into t1. */
    //store_x t0, 0( t1 )                     /* Restore the critical nesting value for this task. */

    load_x  x1, 1 * portWORD_SIZE( sp )
    load_x  x5, 2 * portWORD_SIZE( sp )
    load_x  x6, 3 * portWORD_SIZE( sp )
    load_x  x7, 4 * portWORD_SIZE( sp )
    load_x  x8, 5 * portWORD_SIZE( sp )
    load_x  x9, 6 * portWORD_SIZE( sp )
    load_x  x10, 7 * portWORD_SIZE( sp )
    load_x  x11, 8 * portWORD_SIZE( sp )
    load_x  x12, 9 * portWORD_SIZE( sp )
    load_x  x13, 10 * portWORD_SIZE( sp )
    load_x  x14, 11 * portWORD_SIZE( sp )
    load_x  x15, 12 * portWORD_SIZE( sp )
#ifndef __riscv_32e
    load_x  x16, 13 * portWORD_SIZE( sp )
    load_x  x17, 14 * portWORD_SIZE( sp )
    load_x  x18, 15 * portWORD_SIZE( sp )
    load_x  x19, 16 * portWORD_SIZE( sp )
    load_x  x20, 17 * portWORD_SIZE( sp )
    load_x  x21, 18 * portWORD_SIZE( sp )
    load_x  x22, 19 * portWORD_SIZE( sp )
    load_x  x23, 20 * portWORD_SIZE( sp )
    load_x  x24, 21 * portWORD_SIZE( sp )
    load_x  x25, 22 * portWORD_SIZE( sp )
    load_x  x26, 23 * portWORD_SIZE( sp )
    load_x  x27, 24 * portWORD_SIZE( sp )
    load_x  x28, 25 * portWORD_SIZE( sp )

#ifdef CONFIG_PS_EXTEND
    /* calc cost, use 2 registers at least */
    rdcycle x31
    csrr x30, mscratch
    sub x31, x31, x30
    la x30, ulLastTrapCount
    store_x x31, (x30)

    /* test if switch, update ulSwitchInExtra, use 3 resgister at least */
    la x30, ulSwitchOutExtra
    load_x x29, (x30)
    beq x29, x0, 1f
    sub x31, x31, x29
    la x30, ulSwitchInExtra
    store_x x31, (x30)

1:
#endif

    load_x  x29, 26 * portWORD_SIZE( sp )
    load_x  x30, 27 * portWORD_SIZE( sp )
    load_x  x31, 28 * portWORD_SIZE( sp )
#endif
    addi sp, sp, portCONTEXT_SIZE

    mret
    .endm
/*-----------------------------------------------------------*/

#endif /* PORTCONTEXT_H */
