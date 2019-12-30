/* vxbFdtRsRzg2CpgMssr.h - Renesas RZ/G2 CPG and MSSR header file */

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
17sep19,hli  created (VXWPG-394)
*/

#ifndef __INCvxbFdtRsRzg2CpgMssrh
#define __INCvxbFdtRsRzg2CpgMssrh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* defines */

/* clock Pulse Generator register offsets */

#define CPGWPCR         0x0904U     /* CPG Write Protect Control Register */
#define CPGWPR          0x0900U     /* CPG Write Protect Register */
#define FRQCRB          0x0004U     /* Frequency Control Register B */
#define FRQCRC          0x00E0U     /* Frequency Control Register C */
#define PLLECR          0x00D0U     /* PLL Enable Control Register */
#define PLL0CR          0x00D8U     /* PLL0 Control Register */
#define PLL2CR          0x002CU     /* PLL2 Control Register */
#define PLL3CR          0x00DCU     /* PLL3 Control Register. Not public. */
#define PLL4CR          0x01F4U     /* PLL4 Control Register */
#define PLL0STPCR       0x00F0U     /* PLL0 Stop Condition Register */
#define PLL2STPCR       0x00F8U     /* PLL2 Stop Condition Register */
#define PLL3STPCR       0x00FCU     /* PLL3 Stop Condition Register */
#define PLL4STPCR       0x01F8U     /* PLL4 Stop Condition Register */
#define SD0CKCR         0x0074U     /* SD-IF0 Clock Frequency Control Reg */
#define SD1CKCR         0x0078U     /* SD-IF1 Clock Frequency Control Reg */
#define SD2CKCR         0x0268U     /* SD-IF2 Clock Frequency Control Reg */
#define SD3CKCR         0x026CU     /* SD-IF3 Clock Frequency Control Reg */
#define RPCCKCR         0x0238U     /* RPC Clock Frequency Control Register */
#define CANFDCKCR       0x0244U     /* CAN-FD Clock Frequency Control Reg */
#define MSOCKCR         0x0014U     /* MSIOF Clock Frequency Control Reg */
#define HDMICKCR        0x0250U     /* HDMI-IF Clock Frequency Control Reg */
#define CSI0CKCR        0x000CU     /* CSI0 Clock Frequency Control Register */
#define CSIREFCKCR      0x0034U     /* CSIREF Clock Frequency Control Reg */
#define RCKCR           0x0240U     /* RCLK Frequency Control Register */
#define DVFSCR0         0x0058U     /* DVFS Control Register 0 */
#define DVFSCR1         0x005CU     /* DVFS Control Register 1 */

/* clock pulse generator register values */

/* write protect enable */

#define CPGWPCR_WPE           (1U << 0)

/* frequency contol register B */

#define FRQCRB_ZTRFC_SHIFT   20
#define FRQCRB_ZTRFC_MASK    (0x0FU << FRQCRB_ZTRFC_SHIFT)
#define FRQCRB_ZTRFC(bits)   (((bits) & FRQCRB_ZTRFC_MASK) >> FRQCRB_ZTRFC_SHIFT)

#define FRQCRB_ZTFC_SHIFT    16
#define FRQCRB_ZTFC_MASK     (0x0FU << FRQCRB_ZTFC_SHIFT)
#define FRQCRB_ZTFC(bits)    (((bits) & FRQCRB_ZTFC_MASK) >> FRQCRB_ZTFC_SHIFT)

#define FRQCRB_ZTRD2FC_SHIFT 0
#define FRQCRB_ZTRD2FC_MASK  (0x0FU << FRQCRB_ZTRD2FC_SHIFT)
#define FRQCRB_ZTRD2FC(bits) (((bits) & FRQCRB_ZTRD2FC_MASK) >> \
                              FRQCRB_ZTRD2FC_SHIFT)

/* frequency contol register C */

#define FRQCRC_ZRFC_SHIFT    16
#define FRQCRC_ZRFC_MASK     (0x01FU << FRQCRC_ZRFC_SHIFT)
#define FRQCRC_ZRFC(bits)    (((bits) & FRQCRC_ZRFC_MASK) >> FRQCRC_ZRFC_SHIFT)

#define FRQCRC_ZFC_SHIFT     8
#define FRQCRC_ZFC_MASK      (0x01FU << FRQCRC_ZFC_SHIFT)
#define FRQCRC_ZFC(bits)     (((bits) & FRQCRC_ZFC_MASK) >> FRQCRC_ZFC_SHIFT)

#define FRQCRC_Z2FC_SHIFT    0
#define FRQCRC_Z2FC_MASK     (0x01FU << FRQCRC_Z2FC_SHIFT)
#define FRQCRC_Z2FC(bits)    (((bits) & FRQCRC_Z2FC_MASK) >> FRQCRC_Z2FC_SHIFT)

/* PLL enable control and status */

#define PLLECR_PLL0ST        (1U << 8)
#define PLLECR_PLLnST(n)     (PLLECR_PLL0ST << (n))

#define PLLECR_PLLE0         (1U << 0)
#define PLLECR_PLLnE(n)      (PLLECR_PLLE0 << (n))

/* PLL control STC[6:0] */

#define PLLnCR_STC_SHIFT     24
#define PLLnCR_STC_MASK      (0x07FU << PLLnCR_STC_SHIFT)
#define PLLnCR_STC(bits)     (((bits) & PLLnCR_STC_MASK) >> PLLnCR_STC_SHIFT)

/* SD-IFn clock frequency control registers */

#define SDnCKCR_SDnSRCFC_SHIFT       2
#define SDnCKCR_SDnSRCFC_MASK        (0x01CU << SDnCKCR_SDnSRCFC_SHIFT)
#define SDnCKCR_SDnSRCFC(bits)       (((bits) & SDnCKCR_SDnSRCFC_MASK) >> \
                                      SDnCKCR_SDnSRCFC_SHIFT)
#define SDnCKCR_SDnSRCFC_VALID_MAX   4U

#define SDnCKCR_SDnFC(bits)          ((bits) & 0x03U)
#define SDnCKCR_SDnFC_VALID_MAX      2U

#define SDnCKCR_STPnHCK              (1U << 9)
#define SDnCKCR_STPnCK               (1U << 8)

/* RPC clock frequency control register */

#define RPCCKCR_CKSTP2               (1U << 9)
#define RPCCKCR_CKSTP                (1U << 8)

#define RPCCKCR_RPC_DIV_SHIFT        3
#define RPCCKCR_RPC_DIV_MASK         (0x03U << RPCCKCR_RPC_DIV_SHIFT)
#define RPCCKCR_RPC_DIV(bits)        (((bits) & RPCCKCR_RPC_DIV_MASK) >> \
                                      RPCCKCR_RPC_DIV_SHIFT)
#define RCPCKCR_RPC_DIV_VALID_MIN    2U

#define RPCCKCR_RPCD2_DIV_SHIFT      0
#define RPCCKCR_RPCD2_DIV_MASK       (0x07U << RPCCKCR_RPCD2_DIV_SHIFT)
#define RPCCKCR_RPCD2_DIV(bits)      (((bits) & RPCCKCR_RPCD2_DIV_MASK) >> \
                                      RPCCKCR_RPCD2_DIV_SHIFT)
#define RCPCKCR_RPCD2_DIV_VALID_MIN  2U

/* clock frequency control registers */

#define CKCR_CKSTP          (1U << 8)
#define CKCR_DIV(bits)      ((bits) & 0x03FU)

/* module stop standby reset register offsets */

#define MSTPSR0         0x0030U     /* Module Stop Status Register 0 */
#define MSTPSR1         0x0038U     /* Module Stop Status Register 1 */
#define MSTPSR2         0x0040U     /* Module Stop Status Register 2 */
#define MSTPSR3         0x0048U     /* Module Stop Status Register 3 */
#define MSTPSR4         0x004CU     /* Module Stop Status Register 4 */
#define MSTPSR5         0x003CU     /* Module Stop Status Register 5 */
#define MSTPSR6         0x01C0U     /* Module Stop Status Register 6 */
#define MSTPSR7         0x01C4U     /* Module Stop Status Register 7 */
#define MSTPSR8         0x09A0U     /* Module Stop Status Register 8 */
#define MSTPSR9         0x09A4U     /* Module Stop Status Register 9 */
#define MSTPSR10        0x09A8U     /* Module Stop Status Register 10 */
#define MSTPSR11        0x09ACU     /* Module Stop Status Register 11 */
#define SMSTPCR0        0x0130U     /* System Module Stop Control Register 0 */
#define SMSTPCR1        0x0134U     /* System Module Stop Control Register 1 */
#define SMSTPCR2        0x0138U     /* System Module Stop Control Register 2 */
#define SMSTPCR3        0x013CU     /* System Module Stop Control Register 3 */
#define SMSTPCR4        0x0140U     /* System Module Stop Control Register 4 */
#define SMSTPCR5        0x0144U     /* System Module Stop Control Register 5 */
#define SMSTPCR6        0x0148U     /* System Module Stop Control Register 6 */
#define SMSTPCR7        0x014CU     /* System Module Stop Control Register 7 */
#define SMSTPCR8        0x0990U     /* System Module Stop Control Register 8 */
#define SMSTPCR9        0x0994U     /* System Module Stop Control Register 9 */
#define SMSTPCR10       0x0998U     /* System Module Stop Control Register 10 */
#define SMSTPCR11       0x099CU     /* System Module Stop Control Register 11 */

/* TMU module stop control */

#define TMU0_MSTP125    (1U << 25)
#define TMU1_MSTP124    (1U << 24)
#define TMU2_MSTP123    (1U << 23)
#define TMU3_MSTP122    (1U << 22)
#define TMU4_MSTP121    (1U << 21)

#define TMU_ALL_MSTP    (TMU0_MSTP125 | TMU1_MSTP124 | TMU2_MSTP123 | \
                         TMU3_MSTP122 | TMU4_MSTP121)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCvxbFdtRsRzg2CpgMssrh */
