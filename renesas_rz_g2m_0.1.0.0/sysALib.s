/* sysALib.s - Renesas RZ/G2 ARMv8 system entry */

/*
 * Copyright (c) 2019 Wind River Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3) Neither the name of Wind River Systems nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
modification history
--------------------
17sep19,hli created (VXWPG-394)
*/

/*
DESCRIPTION
This module contains system-dependent functions written in assembly
language.  It contains the entry code, sysInit(), for VxWorks images
that start running from RAM, such as 'vxWorks'.  These images are
loaded into memory by some external program (e.g., a boot ROM) and then
started.  The function sysInit() must come first in the text segment.
Its job is to perform the minimal setup needed to call the generic C
function usrInit().

sysInit() masks interrupts in the processor and the interrupt
controller and sets the initial stack pointer.  Other hardware and
device initialization is performed later in the sysHwInit function in
sysLib.c.
*/

#define _ASMLANGUAGE
#include <vxWorks.h>
#include <vsbConfig.h>
#include <asm.h>
#include <regs.h>
#include <prjParams.h>

/* defines */

#undef DEBUG

#define DEBUG_SCIF_BASE                 (0xE6E88000)            /* SCIF2 */

#define RZ_G2_GIC400_BASE               (0xF1000000)
#define GIC_DIST                        (RZ_G2_GIC400_BASE + 0x10000)
#define GIC_CPU                         (RZ_G2_GIC400_BASE + 0x20000)

#define ARM_GIC_CPU_CONTROL             (GIC_CPU)
#define ARM_GIC_DIST_CONTROL            (GIC_DIST)
#define ARM_GIC_DIST_TYPER              (GIC_DIST + 0x04)
#define ARM_GIC_DIST_IGROUPRn           (GIC_DIST + 0x80)
#define ARM_GIC_DIST_ISENABLERn         (GIC_DIST + 0x100)

#define ARM_GIC_CPU_ACK                 (GIC_CPU + 0xC)
#define ARM_GIC_INT_SPURIOUS            (0x3FF)
#define ARM_GIC_CPU_END_INTR            (GIC_CPU + 0x10)
#define GIC_IntPendClr                  (GIC_DIST + 0x280)
#define ALL_PPI_INT_MASK                0xFFFF0000
#define ALL_SGI_INT_MASK                0x0000FFFF
#define GIC_CPU_PriMask                 (GIC_CPU + 0x04)
#define GIC_INT_ALL_ENABLED             0xFF
#define GIC_CPU_BinPoint                (GIC_CPU + 0x08)
#define GIC_CPU_BINP_DEFAULT            0x07
#define GIC_CPU_Control                 (GIC_CPU + 0x00)
#define GIC_CONTROL_ENABLE              0x01
#define GIC_IntEnClr                    (GIC_DIST + 0x180)
#define GIC_Prio                        (GIC_DIST + 0x400)

#define CNTHCTL_EL2_EL1PCTEN            (0x1)
#define CNTHCTL_EL2_EL1PCEN             (0x1 << 1)

#define HCR_EL2_RW                      (0x1 << 31)
#define HCR_EL2_HCD                     (0x1 << 29)

#define SCR_EL3_RIQ                     (0x1)
#define SCR_EL3_RES1                    (0x3 << 4)
#define SCR_EL3_SMD                     (0x1 << 7)
#define SCR_EL3_HCE                     (0x1 << 8)
#define SCR_EL3_RW                      (0x1 << 10)

#define CPTR_EL2_RES1                   (0x33ff)
#define SCTLR_EL2_RES1                  (0x30C50830)

#define AARCH64_SPSR_MODE_EL0           (0x0)
#define AARCH64_SPSR_MODE_EL1           (0x5)
#define AARCH64_SPSR_MODE_EL2           (0x9)

#define AARCH64_SPSR_EL3_M_EL2h         (AARCH64_SPSR_MODE_EL2)
#define AARCH64_SPSR_EL3_F              (0x1 << 6)
#define AARCH64_SPSR_EL3_I              (0x1 << 7)
#define AARCH64_SPSR_EL3_A              (0x1 << 8)
#define AARCH64_SPSR_EL3_E              (0x1 << 9)

#define SCTLR_EL1_RES1                  (0x30D00800)

#define AARCH64_SPSR_EL2_M_EL1h         (AARCH64_SPSR_MODE_EL1)
#define AARCH64_SPSR_EL2_F              (0x1 << 6)
#define AARCH64_SPSR_EL2_I              (0x1 << 7)
#define AARCH64_SPSR_EL2_A              (0x1 << 8)
#define AARCH64_SPSR_EL2_E              (0x1 << 9)

#define AARCH32_SPSR_EL2_M_EL1h         (0x13) /* M-bit for AARCH32 + SUP mode */
#define AARCH32_SPSR_EL2_F              (0x1 << 6)
#define AARCH32_SPSR_EL2_I              (0x1 << 7)
#define AARCH32_SPSR_EL2_A              (0x1 << 8)
#define AARCH32_SPSR_EL2_D              (0x1 << 9)

#define CPACR_EL1_FPEN_NO_INST_TRAPPED  (0x3 << 20)

#define SCR_EL3_DEFAULT                 (SCR_EL3_RIQ  |             \
                                         SCR_EL3_RES1 |             \
                                         SCR_EL3_SMD  |             \
                                         SCR_EL3_HCE  |             \
                                         SCR_EL3_RW)

#define SPSR_EL3_DEFAULT                (AARCH64_SPSR_EL3_E |       \
                                         AARCH64_SPSR_EL3_A |       \
                                         AARCH64_SPSR_EL3_I |       \
                                         AARCH64_SPSR_EL3_F |       \
                                         AARCH64_SPSR_EL3_M_EL2h)

/* goto EL1 in AARCH64 mode */
#define SPSR_EL2_DEFAULT                (AARCH64_SPSR_EL2_E |       \
                                         AARCH64_SPSR_EL2_A |       \
                                         AARCH64_SPSR_EL2_I |       \
                                         AARCH64_SPSR_EL2_F |       \
                                         AARCH64_SPSR_EL2_M_EL1h)

/*
 * Common code for cache operations on entire data/unified caches,
 * performed by set/way to the point of coherency (PoC).
 * This code is based on 'Example code for Cleaning to Point of Coherency'
 * provided in "Programmer's Guide for ARMv8-A" (ARM DEN0024A)
 *
 * Registers used: 0-5, 7-11, 16, 17 - no need to save and restore
 *
 * This is used from sysInit, where it is too early to require stack.
 */

#define ARM_IMM #
#define ARM_LOC_MASK ARM_IMM 0x7000000	/* level of coherency mask of CLIDR */

#define _CORTEX_AR_ENTIRE_DATA_CACHE_OP(operation)                            \
    MRS X0, CLIDR_EL1                                                        ;\
    AND W3, W0, ARM_LOC_MASK    /* Get 2 x Level of Coherence */             ;\
    LSR W3, W3, ARM_IMM 23                                                   ;\
    CBZ W3, 5f                                                               ;\
    MOV W10, ARM_IMM 0          /* W10 = 2 x cache level */                  ;\
    MOV W8, ARM_IMM 1           /* W8 = constant 0b1 */                      ;\
1:  ADD W2, W10, W10, LSR ARM_IMM 1 /* Calculate 3 x cache level */          ;\
    LSR W1, W0, W2              /* extract 3-bit cache type for this level */;\
    AND W1, W1, ARM_IMM 0x7                                                  ;\
    CMP W1, ARM_IMM 2                                                        ;\
    B.LT 4f                     /* No data or unified cache at this level */ ;\
    MSR CSSELR_EL1, X10         /* Select this cache level */                ;\
    ISB                         /* Synchronize change of CSSELR */           ;\
    MRS X1, CCSIDR_EL1          /* Read CCSIDR */                            ;\
    AND W2, W1, ARM_IMM 7       /* W2 = log2(linelen)-4 */                   ;\
    ADD W2, W2, ARM_IMM 4       /* W2 = log2(linelen) */                     ;\
    UBFX W4, W1, ARM_IMM 3, ARM_IMM 10  /* W4 = max way number,*/            ;\
                                        /*    right aligned */               ;\
    CLZ W5, W4                  /* W5 = 32-log2(ways), bit position of */    ;\
                                /*    way in DC operand*/                    ;\
    LSL W9, W4, W5              /* W9 = max way number, aligned to */        ;\
                                /*    position in DC operand*/               ;\
    LSL W16, W8, W5             /* W16 = amount to decrement way */          ;\
                                /*    number per iteration */                ;\
2:  UBFX W7, W1, ARM_IMM 13, ARM_IMM 15 /* W7 = max set number, */           ;\
                                        /*    right aligned */               ;\
    LSL W7, W7, W2              /* W7 = max set number, aligned to */        ;\
                                /*    position in DC operand*/               ;\
    LSL W17, W8, W2             /* W17 = amount to decrement set number */   ;\
                                /*    per iteration */                       ;\
3:  ORR W11, W10, W9            /* W11 = combine way number and */           ;\
                                /*    cache number... */                     ;\
    ORR W11, W11, W7            /* ... and set number for DC operand */      ;\
    DC operation, X11           /* Do DC instr by set and way */             ;\
    SUBS W7, W7, W17            /* Decrement set number */                   ;\
    B.GE 3b                                                                  ;\
    SUBS X9, X9, X16            /* Decrement way number */                   ;\
    B.GE 2b                                                                  ;\
4:  ADD W10, W10, ARM_IMM 2     /* Increment 2 x cache level */              ;\
    CMP W3, W10                                                              ;\
    DSB SY                      /* Ensure completion of previous cache */    ;\
                                /* maintenance operation */                  ;\
    B.GT 1b                                                                  ;\
5:  MOV X0, #0

        /* externals */

        FUNC_IMPORT (vxCpuInit)
        FUNC_IMPORT (vxMmuEarlyInit)
        FUNC_IMPORT (usrInit)
        DATA_IMPORT (gpDtbInit)

        /* exports */

        FUNC_EXPORT (sysInit)
#ifdef _WRS_CONFIG_LP64
        FUNC_EXPORT (armWarmReboot)
#endif /* _WRS_CONFIG_LP64 */
        FUNC_EXPORT (sysInitSecondary)

#ifdef DEBUG
        FUNC_EXPORT (sysInitConsolePrint)
        FUNC_EXPORT (sysInitConsoleToHex)

        .globl sysInitConsoleInit
#endif

#ifdef INCLUDE_STANDALONE_DTB
        .data

#define TO_STRING(exp)                  #exp
#define DTS_ASM_FILE(file)              TO_STRING (file.s)

#include DTS_ASM_FILE (DTS_FILE)

#endif /* INCLUDE_STANDALONE_DTB */

#if defined (__llvm__)
        .section .text.entry, "ax"
#elif defined (__GNUC__)
        .section .text.entry, "ax"
#elif defined(__DCC__)
        .section .text.entry, 4, "rx"
#else
#error "TOOL not supported!"
#endif

#ifndef _WRS_CONFIG_LP64
        .code 32
#endif /* _WRS_CONFIG_LP64 */

/*******************************************************************************
*
* sysInit - start after boot
*
* This function is the system start-up entry point for VxWorks in RAM, the
* first code executed after booting.  It disables interrupts, sets up
* the stack, and jumps to the C function usrInit() in usrConfig.c.
*
* The initial stack is set to grow down from the address of sysInit().  This
* stack is used only by usrInit() and is never used again.  Memory for the
* stack must be accounted for when determining the system load address.
*
* NOTE: This function should not be called by the user.
*
* SYNOPSIS
* \ss
* VOID sysInit
*     (
*     void * pDtb   /@ physical pointer for device-tree in RAM  @/
*     )
* \se
*
* NOTE:
*   1. This function should not be called by the user;
*   2. <pOf> is unused because bootng from Open Firmware is unsupported now;
*   3. The initial stack is placeed in .data section, and mapped with the entire
*   image vxWorks.
*
* RETURNS: N/A
*
* sysInit ()              /@ THIS IS NOT A CALLABLE ROUTINE @/
*
*/

FUNC_LABEL (sysInit)

sysInitSecondary:

        MOV     X20, X0                 /* preserve DTB address in X20 */

#ifdef DEBUG
        BL      sysInitConsoleInit
        ADR     X0, logSysInit
        BL      sysInitConsolePrint

        /* examine MPIDR */

        MRS    X1, MPIDR_EL1
        ADR    X0, sysInitMsgNr
        BL     sysInitConsoleToHex

        ADR    X0, sysInitMsgMPIDR
        BL     sysInitConsolePrint

        ADR    X0, sysInitMsgNr
        BL     sysInitConsolePrint

        ADR    X0, sysInitMsgCRLF
        BL     sysInitConsolePrint

        /* examine L2CTRL */

        MRS    X1, S3_1_c11_c0_2
        ADR    X0, sysInitMsgNr
        BL     sysInitConsoleToHex

        ADR    X0, sysInitMsgL2CTRL
        BL     sysInitConsolePrint

        ADR    X0, sysInitMsgNr
        BL     sysInitConsolePrint

        ADR    X0, sysInitMsgCRLF
        BL     sysInitConsolePrint

#endif  /* DEBUG */

        /* initialize GIC */

        LDR     X0, L$_ARM_GIC_DIST_CONTROL
        MOV     W2, #0x3
        STR     W2, [X0]

        LDR     X0, L$_ARM_GIC_DIST_TYPER
        LDR     W2, [X0]

        LDR     X0, L$_ARM_GIC_DIST_IGROUPRn
        AND     W3, W2, #0x1f
        CBZ     W3, 1f
        ADD     X1, X0, #0x4
        MOV     W2, #~0
0:      STR     W2, [X1], #0x4
        SUB     W3, W3, #0x1
        CBNZ    W3, 0b
1:

        /* initialize Distributor Interface */

        LDR     X0, L$_ARM_GIC_DIST_IGROUPRn
        MOV     W2, #~0
        STR     W2, [X0]

        LDR     X0, L$_ARM_GIC_DIST_ISENABLERn
        MOV     W2, #0x1
        STR     W2, [X0]

        /* initialize CPU Interface */

        LDR     X1, L$_ARM_GIC_CPU_CONTROL
        MOV     W2, #0x1e7
        STR     W2, [X1]

        LDR     X1, L$_GIC_CPU_PriMask
        MOV     W2, #0x1 << 7
        STR     W2, [X1]

        /* examine EL */

        MRS     X0, CurrentEL
        CMP     X0, #3<<2               /* is this exception level EL3? */
        BNE     1f

startInEL3:
#ifdef DEBUG
        ADR     X0, log64startEL3
        BL      sysInitConsolePrint
#endif
        B       handleEL3

1:
        CMP     X0, #2<<2               /* is this exception level EL2? */
        BNE     2f

startInEL2:
#ifdef DEBUG
        ADR     X0, log64startEL2
        BL      sysInitConsolePrint
#endif
        B       handleEL2

2:
        CMP     X0, #1<<2               /* is this exception level EL1? */
        BNE     3f

startInEL1:

#ifdef DEBUG
        ADR     X0, log64startEL1
        BL      sysInitConsolePrint
#endif

        B       handleEL1

3:
#ifdef DEBUG
        ADR     X0, log64startEL0
        BL      sysInitConsolePrint
#endif

4:                                      /* Unrecognised exception level */
        B       4b                      /* Stop here */

handleEL3:
        /* EL3 */

        MOV     X1, #(SCR_EL3_DEFAULT)  /* SCR */
        MSR     SCR_EL3, X1
        MSR     CPTR_EL3, XZR           /* CPTR_EL3 */

        MOV     X1, #(CPTR_EL2_RES1)
        MSR     CPTR_EL2, X1            /* CPTR_EL2 */

        MOV     X1, #(SCTLR_EL2_RES1&0xffff)
        MOVK    X1, #((SCTLR_EL2_RES1>>16)&0xffff), lsl #16
        MSR     SCTLR_EL2, X1           /* SCTLR_EL2 */

        MOV     X1, SP
        MSR     SP_EL2, X1              /* SP_EL2 */

        MRS     X1, VBAR_EL3
        MSR     VBAR_EL2, X1            /* VBAR_EL2 */

        MOV     X1, #(SPSR_EL3_DEFAULT)
        MSR     SPSR_EL3, X1            /* SPSR_EL3 */

        ADR     X7, 0f
        MOV     LR, X7
        MSR     ELR_EL3, LR
        ERET                            /* to EL2 */
0:

handleEL2:
        /* EL2 */

        /* initialize Generic Timers */

        MRS     X1, CNTHCTL_EL2
        ORR     X1, X1, #(CNTHCTL_EL2_EL1PCTEN | CNTHCTL_EL2_EL1PCEN)
        MSR     CNTHCTL_EL2, X1         /* CNTHCTL_EL2 */
        MSR     CNTVOFF_EL2, XZR        /* CNTVOFF_EL2 */

        MRS     X1, MIDR_EL1
        MRS     X2, MPIDR_EL1

        MSR     VPIDR_EL2, X1           /* VPIDR_EL2 */
        MSR     VMPIDR_EL2, X2          /* VMPIDR_EL2 */

        MOV     X1, #(CPTR_EL2_RES1)
        MSR     CPTR_EL2, X1            /* CPTR_EL2 */

        MSR     HSTR_EL2, XZR           /* HSTR_EL2 */
        MOV     X1, #(CPACR_EL1_FPEN_NO_INST_TRAPPED)
        MSR     CPACR_EL1, X1           /* CPACR_EL1 */

        MOV     X1, #(HCR_EL2_RW)
        ORR     X1, X1, #(HCR_EL2_HCD)

        MSR     HCR_EL2, X1             /* HCR_EL2 */

        MOV     X1, #(SCTLR_EL1_RES1&0xffff)
        MOVK    X1, #((SCTLR_EL1_RES1>>16)&0xffff), lsl #16
        MSR     SCTLR_EL1, X1           /* SCTLR_EL1 */

        MOV     X1, SP
        MSR     SP_EL1, X1              /* SP_EL1 */

        MRS     X1, VBAR_EL2
        MSR     VBAR_EL1, X1            /* VBAR_EL1 */

        MOV     X1, #(SPSR_EL2_DEFAULT)
        MSR     SPSR_EL2, X1            /* SPSR_EL2 */

#ifdef DEBUG
        ADR     X0, log64startEL2_goto_EL1
        BL      sysInitConsolePrint
#endif

        ADR     X7, 1f
        MOV     LR, X7
        MSR     ELR_EL2, LR
        ERET                            /* to EL1 */

1:

#ifdef DEBUG
        ADR     X0, log64startEL2_in_EL1
        BL      sysInitConsolePrint
#endif

handleEL1:
        /* EL1 */

        MRS     X0, SCTLR_EL1
        BIC     X0, X0, #SCTLR_SA
        MSR     SCTLR_EL1, X0

        /* set initial stack pointer so stack grows down from start of code */

        ADR     X1, sysInit
        MOV     SP, X1
        MOV     X29, #0

        MRS     X0, SPSel
        CBZ     X0, notSetSpsel

        MOV     X1, SP
        MSR     SP_EL0, X1              /* set SP_EL0 the same as SP_EL1 */
        MSR     SPSel, #0               /* always use SP0 */

notSetSpsel:

#ifdef DEBUG
        ADR     X0, sysInitNotSetSPSel
        BL      sysInitConsolePrint
#endif

#ifdef _WRS_CONFIG_SMP
        MRS     X1, MPIDR_EL1
        ANDS    X1, X1, #0xffff
        BEQ     2f                      /* if boot core (A57_0) */

        /* flush all pending local interrupts */

3:
        LDR     X0, =ARM_GIC_CPU_ACK
        LDR     W1, [X0]
        LDR     X2, =ARM_GIC_CPU_END_INTR
        STR     W1, [X2]
        LDR     X3, =ARM_GIC_INT_SPURIOUS
        CMP     W1, W3
        BNE     3b
        LDR     X3, =GIC_IntPendClr
        LDR     X0, =0xffff
        STR     W0, [X3]

        /* clear all pending PPI and SGI interrupts in the distributor */

        LDR     X0, =GIC_IntPendClr
        LDR     X1, =(ALL_PPI_INT_MASK | ALL_SGI_INT_MASK)
        STR     W1, [X0]

        /* set priority */

        MOV     x2, #4
        LDR     X0, =GIC_Prio
4:
        MOV     X1, #0
        STR     W1, [X0]
        ADD     X0, X0, #4
        SUBS    X2, X2, #1
        BNE     4b

        /* enable all interrupt priorities */

        LDR     X0, =GIC_CPU_PriMask
        LDR     X1, =GIC_INT_ALL_ENABLED
        STR     W1, [X0]

        /* enable preemption of all interrupts */

        LDR     X0, =GIC_CPU_BinPoint
        LDR     X1, =GIC_CPU_BINP_DEFAULT
        STR     W1, [X0]

        /* enable this processor's CPU interface */

        LDR     X0, =GIC_CPU_Control
        LDR     X1, =GIC_CONTROL_ENABLE
        STR     W1, [X0]
#ifdef DEBUG
        ADR     X0, laterCoreMsg
        BL      sysInitConsolePrint
#endif

        B       3f

2:
#ifdef DEBUG
        ADR     X0, bootCoreMsg
        BL      sysInitConsolePrint
#endif
#endif /* _WRS_CONFIG_SMP */

        LDR     X1, =sysInit
        ADR     X2, sysInit
        SUB     X2, X2, X1              /* relocation offset to X2 */

        /* DTB address must be a virtual memory address. */

#ifdef INCLUDE_STANDALONE_DTB
        /* A native vxWorks with INCLUDE_STANDALONE_DTB */
        LDR     X5, =dt_blob_start
#else
        /* The rootOS. And a native vxWorks without INCLUDE_STANDALONE_DTB */
        MOV     X5, X20
        SUB     X5, X5, X2
#endif

        LDR     X1, =gpDtbInit
        ADD     X1, X1, X2
        STR     X5, [X1]

3:
        /* now call usrInit (startType) */

        MOV     X0, #2      /* BOOT_COLD */

armWarmReboot:

        MOV     X19, X0

#ifdef DEBUG
        ADR     X0, usrInitMsg
        BL      sysInitConsolePrint
#endif

        BL      vxCpuInit

        LDR     X0, =STATIC_MMU_TABLE_BASE
        LDR     X1, =LOCAL_MEM_LOCAL_ADRS
        LDR     X2, =IMA_SIZE
        BL      vxMmuEarlyInit

        MOV     X0, X19
        B       usrInit

#ifdef DEBUG
logSysInit:
        .asciz  "\r\n>sysInit\r\n"

log64startEL1:
        .asciz  "64-bit mode start in EL1\r\n"

log64startEL2:
        .asciz  "64-bit mode start in EL2\r\n"

log64startEL2_goto_EL1:
        .asciz  "goto EL1\r\n"

log64startEL2_in_EL1:
        .asciz  "reached EL1\r\n"

log64startEL3:
        .asciz  "64-bit mode start in EL3\r\n"

log64startEL0:
        .asciz  "64-bit mode start in EL0 - stop\r\n"

sysInitNotSetSPSel:
        .asciz  "sysInit not set SPSel\r\n"

sysInitMsgL2CTRL:
        .asciz  "sysInit L2CTRL: 0x"

sysInitMsgMPIDR:
        .asciz  "sysInit MPIDR: 0x"

sysInitMsgNr:
        .asciz  "................"

sysInitMsgCRLF:
		.asciz  "\r\n"

hexDigits:
        .ascii  "0123456789ABCDEF"

usrInitMsg:
        .asciz  "goto usrInit\r\n"

bootCoreMsg:
        .asciz  "boot core\r\n"

laterCoreMsg:
        .asciz  "later core\r\n"

        .align  4
sysInitConsoleInit:
        ADR     X1, SCFSCR_SCIF_DEBUG       /* load address of SCSCR        */
        LDR     X1, [X1]
        LDRH    W2, [X1]                    /* read SCSCR                   */
        ORR     W2, W2, #3 << 4             /* enable Tx and Rx             */
        STRH    W2, [X1]                    /* update SCSCR                 */
        RET

sysInitConsolePrint:
        ADR     X1, SCFTDR_SCIF_DEBUG       /* load address of SCFTDR       */
        LDR     X1, [X1]
        ADR     X2, SCFSR_SCIF_DEBUG        /* load address of SCFSR        */
        LDR     X2, [X2]
1:
        LDRB    W3, [X0]                    /* load next character          */
        ADD     X0, X0, #1                  /* increment buffer pointer     */
        CMP     W3, #0                      /* is this the end of string?   */
        BEQ     3f

2:      /* wait for TDFE to become 1 */

        LDRH    W4, [X2]
        AND     W4, W4, #1<<5
        CMP     W4, #1<<5
        BNE     2b

        /* put the character in transmit FIFO */

        STRB    W3, [X1]

        /* clear TDFE, TEND, PE and FE (write 0 to clear) */

        MOV     W4, #0x00cf
        STRH    W4, [X2]

        B       1b

3:      /*  wait for TEND to become 1 */

        LDRH    W4, [X2]
        AND     W4, W4, #1<<6
        CMP     W4, #1<<6
        BNE     3b

        ADR     X3, SCFDR_SCIF_DEBUG        /* load address of SCFDR        */
        LDR     X3, [X3]

4:      /* wait for FIFO to drain (TCOUNT is 0) */

        LDRH    W4, [X3]
        AND     W4, W4, #0x1f00
        CMP     W4, #0
        BNE     4b

        RET

sysInitConsoleToHex:        /* X0 result buffer (17 bytes), X1 quad value */
        ADR     X3, hexDigits
        MOV     W4, #0
        ADD     X0, X0, 16
        STRB    W4, [X0]
        MOV     X2, #16
1:
        SUB     X0, X0, #1
        AND     X4, X1, #0xF
        LSR     X1, X1, #4
        ADD     X4, X3, X4
        LDRB    W4, [X4]
        STRB    W4, [X0]
        SUB     X2, X2, #1
        CMP     X2, #0
        BGT     1b
        RET

        .align 4

SCFSCR_SCIF_DEBUG:
        .word   DEBUG_SCIF_BASE+0x8
        .word   0
SCFTDR_SCIF_DEBUG:
        .word   DEBUG_SCIF_BASE+0xc
        .word   0
SCFSR_SCIF_DEBUG:
        .word   DEBUG_SCIF_BASE+0x10
        .word   0
SCFDR_SCIF_DEBUG:
        .word   DEBUG_SCIF_BASE+0x1c
        .word   0

#endif /* DEBUG */

        .align  4
L$_ARM_GIC_DIST_CONTROL:
        .word   ARM_GIC_DIST_CONTROL
        .word   0
L$_ARM_GIC_DIST_TYPER:
        .word   ARM_GIC_DIST_TYPER
        .word   0
L$_ARM_GIC_DIST_IGROUPRn:
        .word   ARM_GIC_DIST_IGROUPRn
        .word   0
L$_ARM_GIC_DIST_ISENABLERn:
        .word   ARM_GIC_DIST_ISENABLERn
        .word   0
L$_ARM_GIC_CPU_CONTROL:
        .word   ARM_GIC_CPU_CONTROL
        .word   0
L$_GIC_CPU_PriMask:
        .word   GIC_CPU_PriMask
        .word   0

FUNC_END (sysInit)
