/* vxbFdtRsRzg2PfcPinMux.h - Renesas Rz/G2 PFC driver header file */

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

#ifndef __INCvxbFdtRsRzg2PfcPinMuxh
#define __INCvxbFdtRsRzg2PfcPinMuxh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* defines */

/* Pin function controller (PFC) registers offsets */

#define PFC_PMMR        0x0000U /* LSI Multiplexed Pin Setting Mask Register */
#define PFC_GPSR0       0x0100U /* GPIO/Peripheral Function Select Register 0 */
#define PFC_IPSR0       0x0200U /* Peripheral Function Select Register 0 */
#define PFC_DRVCTRL0    0x0300U /* DRV Control Register 0 */
#define PFC_POCCTRL     0x0380U /* POC Control Register */
#define PFC_TDSELCTRL   0x03C0U /* TDSEL Control Register */
#define PFC_IOCTRL      0x03E0U /* IO Cell Control for IICDVFS */
#define PFC_FUSEMON     0x03E4U /* Fuse Monitor Register */
#define PFC_PUEN0       0x0400U /* LSI Pin Pull-Enable Register 0 */
#define PFC_PUD0        0x0440U /* LSI Pin Pull-Up/Down Control Register 0 */
#define PFC_MOD_SEL0    0x0500U /* Module Select Register 0 */

/* GPIO/peripheral function select registers 0 to 7 offsets */

#define NUM_GPSR_REGS       8
#define PFC_GPSR(n)         (PFC_GPSR0 + ((n) * 4U))

/* peripheral function select registers 0 to 17 offsets */

#define NUM_IPSR_REGS       18
#define PFC_IPSR(n)         (PFC_IPSR0 + ((n) * 4U))

/* DRV Control Registers 0 to 24 offsets */

#define NUM_DRVTRL_REGS     25
#define PFC_DRVCTRL(n)      (PFC_DRVCTRL0 + ((n) * 4U))

/* LSI Pin Pull-Enable registers 0 to 6 offsets */

#define NUM_PUEN_REGS       7
#define PFC_PUEN(n)         (PFC_PUEN0 + ((n) * 4U))

/* LSI Pin Pull-Up/Down control registers 0 to 6 offsets */

#define NUM_PUD_REGS        7
#define PFC_PUD(n)          (PFC_PUD0 + ((n) * 4U))

/* module select registers 0 to 2 offsets */

#define NUM_MOD_SEL_REGS    3
#define PFC_MOD_SEL(n)      (PFC_MOD_SEL0 + ((n) * 4U))

/* GPSR registers */

#define GPSR_GPIO               0U          /* select GPIO function */
#define GPSR_PERIPHERAL         1U          /* select peripheral function */

#define GPSR_REG_PIN(reg, pin)  (((reg) << 8) | (pin))
#define GPSR_REG(regPin)        ((regPin) >> 8)
#define GPSR_MASK(regPin)       (1U << ((regPin) & 0x01FU))
#define GPSR_NONE               0xFFFFFFFFU

/* peripheral function select registers */

#define IPSR_REG_POS(reg, pos)  (((reg) << 8) | (pos))
#define IPSR_REG(regPos)        ((regPos) >> 8)
#define IPSR_SHIFT(regPos)      ((regPos) & 0x01FU)
#define IPSR_MASK(regPos)       (0x0FU << IPSR_SHIFT (regPos))

/* peripheral function select register 0 */

#define IPSR_AVB_MDC                    IPSR_REG_POS (0U, 0U)
#define IPSR_MSIOF2_SS2_C               IPSR_REG_POS (0U, 0U)

#define IPSR_AVB_MAGIC                  IPSR_REG_POS (0U, 4U)
#define IPSR_MSIOF2_SS1_C               IPSR_REG_POS (0U, 4U)
#define IPSR_SCK4_A                     IPSR_REG_POS (0U, 4U)

#define IPSR_AVB_PHY_INT                IPSR_REG_POS (0U, 8U)
#define IPSR_MSIOF2_MSIOF2_SYNC_C       IPSR_REG_POS (0U, 8U)
#define IPSR_RX4_A                      IPSR_REG_POS (0U, 8U)

#define IPSR_AVB_LINK                   IPSR_REG_POS (0U, 12U)
#define IPSR_MSIOF2_MSIOF2_SCK_C        IPSR_REG_POS (0U, 12U)
#define IPSR_TX4_A                      IPSR_REG_POS (0U, 12U)

#define IPSR_AVB_AVTP_MATCH_A           IPSR_REG_POS (0U, 16U)
#define IPSR_MSIOF2_MSIOF2_RXD_C        IPSR_REG_POS (0U, 16U)
#define IPSR_CTS4_A                     IPSR_REG_POS (0U, 16U)

#define IPSR_AVB_AVTP_CAPTURE_A         IPSR_REG_POS (0U, 20U)
#define IPSR_MSIOF2_MSIOF2_TXD_C        IPSR_REG_POS (0U, 20U)
#define IPSR_RTS4_A                     IPSR_REG_POS (0U, 20U)

#define IPSR_IRQ0                       IPSR_REG_POS (0U, 24U)
#define IPSR_DU_CDE                     IPSR_REG_POS (0U, 24U)
#define IPSR_DU_CDE                     IPSR_REG_POS (0U, 24U)
#define IPSR_VI4_DATA0_B                IPSR_REG_POS (0U, 24U)
#define IPSR_CAN0_TX_B                  IPSR_REG_POS (0U, 24U)
#define IPSR_CANFD0_TX_B                IPSR_REG_POS (0U, 24U)
#define IPSR_MSIOF3_SS2_E               IPSR_REG_POS (0U, 24U)

#define IPSR_IRQ1                       IPSR_REG_POS (0U, 28U)
#define IPSR_DU_DISP                    IPSR_REG_POS (0U, 28U)
#define IPSR_VI4_DATA1_B                IPSR_REG_POS (0U, 28U)
#define IPSR_CAN0_RX_B                  IPSR_REG_POS (0U, 28U)
#define IPSR_CANFD0_RX_B                IPSR_REG_POS (0U, 28U)
#define IPSR_MSIOF3_SS1_E               IPSR_REG_POS (0U, 28U)

/* peripheral function select register 1 */

#define IPSR_IRQ2                       IPSR_REG_POS (1U, 0U)
#define IPSR_IRQ3                       IPSR_REG_POS (1U, 4U)
#define IPSR_IRQ4                       IPSR_REG_POS (1U, 8U)
#define IPSR_IRQ5                       IPSR_REG_POS (1U, 12U)

#define IPSR_PWM0                       IPSR_REG_POS (1U, 16U)
#define IPSR_AVB_AVTP_PPS               IPSR_REG_POS (1U, 16U)

#define IPSR_PWM1_A                     IPSR_REG_POS (1U, 20U)
#define IPSR_PWM2_A                     IPSR_REG_POS (1U, 24U)
#define IPSR_A0                         IPSR_REG_POS (1U, 28U)

/* peripheral function select register 2 */
#define IPSR_A1                         IPSR_REG_POS (2U, 0U)
#define IPSR_A2                         IPSR_REG_POS (2U, 4U)
#define IPSR_A3                         IPSR_REG_POS (2U, 8U)
#define IPSR_A4                         IPSR_REG_POS (2U, 12U)
#define IPSR_A5                         IPSR_REG_POS (2U, 16U)

#define IPSR_A6                         IPSR_REG_POS (2U, 20U)
#define IPSR_RX4_B                      IPSR_REG_POS (2U, 20U)

#define IPSR_A7                         IPSR_REG_POS (2U, 24U)
#define IPSR_TX4_B                      IPSR_REG_POS (2U, 24U)

#define IPSR_A8                         IPSR_REG_POS (2U, 28U)
#define IPSR_SDA6_A                     IPSR_REG_POS (2U, 28U)
#define IPSR_RX3_B                      IPSR_REG_POS (2U, 28U)
#define IPSR_AVB_AVTP_MATCH_B           IPSR_REG_POS (2U, 28U)

/* peripheral function select register 3 */

#define IPSR_A9                         IPSR_REG_POS (3U, 0U)
#define IPSR_A10                        IPSR_REG_POS (3U, 4U)

#define IPSR_A11                        IPSR_REG_POS (3U, 8U)
#define IPSR_SCL6_A                     IPSR_REG_POS (3U, 8U)
#define IPSR_TX3_B                      IPSR_REG_POS (3U, 8U)

#define IPSR_A12                        IPSR_REG_POS (3U, 12U)
#define IPSR_A13                        IPSR_REG_POS (3U, 16U)
#define IPSR_A14                        IPSR_REG_POS (3U, 20U)
#define IPSR_A15                        IPSR_REG_POS (3U, 24U)
#define IPSR_A16                        IPSR_REG_POS (3U, 28U)

/* peripheral function select register 4 */

#define IPSR_A17                        IPSR_REG_POS (4U, 0U)
#define IPSR_A18                        IPSR_REG_POS (4U, 4U)
#define IPSR_A19                        IPSR_REG_POS (4U, 8U)

#define IPSR_CS0                        IPSR_REG_POS (4U, 12U)
#define IPSR_CS1                        IPSR_REG_POS (4U, 16U)

#define IPSR_BS                         IPSR_REG_POS (4U, 20U)
#define IPSR_SCK3                       IPSR_REG_POS (4U, 20U)

#define IPSR_RD                         IPSR_REG_POS (4U, 24U)
#define IPSR_RX3_A                      IPSR_REG_POS (4U, 24U)

#define IPSR_RD_WR                      IPSR_REG_POS (4U, 28U)
#define IPSR_TX3_A                      IPSR_REG_POS (4U, 28U)

/* peripheral function select register 5 */

#define IPSR_WE0                        IPSR_REG_POS (5U, 0U)
#define IPSR_CTS3                       IPSR_REG_POS (5U, 0U)
#define IPSR_SCL6_B                     IPSR_REG_POS (5U, 0U)

#define IPSR_WE1                        IPSR_REG_POS (5U, 4U)
#define IPSR_SDA6_B                     IPSR_REG_POS (5U, 4U)

#define IPSR_EX_WAIT0_A                 IPSR_REG_POS (5U, 8U)

#define IPSR_D0                         IPSR_REG_POS (5U, 12U)
#define IPSR_D1                         IPSR_REG_POS (5U, 16U)

#define IPSR_D2                         IPSR_REG_POS (5U, 20U)

#define IPSR_D3                         IPSR_REG_POS (5U, 24U)
#define IPSR_D4                         IPSR_REG_POS (5U, 28U)

/* peripheral function select register 6 */

#define IPSR_D5                         IPSR_REG_POS (6U, 0U)
#define IPSR_D6                         IPSR_REG_POS (6U, 4U)
#define IPSR_D7                         IPSR_REG_POS (6U, 8U)
#define IPSR_D8                         IPSR_REG_POS (6U, 12U)
#define IPSR_D9                         IPSR_REG_POS (6U, 16U)
#define IPSR_D10                        IPSR_REG_POS (6U, 20U)
#define IPSR_D11                        IPSR_REG_POS (6U, 24U)

#define IPSR_D12                        IPSR_REG_POS (6U, 28U)
#define IPSR_RX4_C                      IPSR_REG_POS (6U, 28U)

/* peripheral function select register 7 */

#define IPSR_D13                        IPSR_REG_POS (7U, 0U)
#define IPSR_TX4_C                      IPSR_REG_POS (7U, 0U)

#define IPSR_D14                        IPSR_REG_POS (7U, 4U)
#define IPSR_SCL6_C                     IPSR_REG_POS (7U, 4U)

#define IPSR_D15                        IPSR_REG_POS (7U, 8U)
#define IPSR_SDA6_C                     IPSR_REG_POS (7U, 8U)

#define IPSR_SD0_CLK                    IPSR_REG_POS (7U, 16U)
#define IPSR_SD0_CMD                    IPSR_REG_POS (7U, 20U)
#define IPSR_SD0_DAT0                   IPSR_REG_POS (7U, 24U)
#define IPSR_SD0_DAT1                   IPSR_REG_POS (7U, 28U)

/* peripheral function select register 8 */

#define IPSR_SD0_DAT2                   IPSR_REG_POS (8U, 0U)
#define IPSR_SD0_DAT3                   IPSR_REG_POS (8U, 4U)
#define IPSR_SD1_CLK                    IPSR_REG_POS (8U, 8U)
#define IPSR_SD1_CMD                    IPSR_REG_POS (8U, 12U)
#define IPSR_SD1_DAT0                   IPSR_REG_POS (8U, 16U)
#define IPSR_SD1_DAT1                   IPSR_REG_POS (8U, 20U)
#define IPSR_SD1_DAT2                   IPSR_REG_POS (8U, 24U)
#define IPSR_SD1_DAT3                   IPSR_REG_POS (8U, 28U)

/* peripheral function select register 9 */

#define IPSR_SD2_CLK                    IPSR_REG_POS (9U, 0U)
#define IPSR_SD2_CMD                    IPSR_REG_POS (9U, 4U)
#define IPSR_SD2_DAT0                   IPSR_REG_POS (9U, 8U)
#define IPSR_SD2_DAT1                   IPSR_REG_POS (9U, 12U)
#define IPSR_SD2_DAT2                   IPSR_REG_POS (9U, 16U)
#define IPSR_SD2_DAT3                   IPSR_REG_POS (9U, 20U)
#define IPSR_SD2_DS                     IPSR_REG_POS (9U, 24U)
#define IPSR_SD3_CLK                    IPSR_REG_POS (9U, 28U)

/* peripheral function select register 10 */

#define IPSR_SD3_CMD                    IPSR_REG_POS (10U, 0U)
#define IPSR_SD3_DAT0                   IPSR_REG_POS (10U, 4U)
#define IPSR_SD3_DAT1                   IPSR_REG_POS (10U, 8U)
#define IPSR_SD3_DAT2                   IPSR_REG_POS (10U, 12U)
#define IPSR_SD3_DAT3                   IPSR_REG_POS (10U, 16U)
#define IPSR_SD3_DAT4                   IPSR_REG_POS (10U, 20U)
#define IPSR_SD3_DAT5                   IPSR_REG_POS (10U, 24U)
#define IPSR_SD3_DAT6                   IPSR_REG_POS (10U, 28U)

#define IPSR_SD3_CD                     IPSR_SD3_DAT6
#define IPSR_SD3_WP                     IPSR_SD3_DAT7

/* peripheral function select register 11 */

#define IPSR_SD3_DAT7                   IPSR_REG_POS (11U, 0U)
#define IPSR_SD3_DS                     IPSR_REG_POS (11U, 4U)

#define IPSR_SD0_CD                     IPSR_REG_POS (11U, 8U)
#define IPSR_SCL2_B                     IPSR_REG_POS (11U, 8U)

#define IPSR_SD0_WP                     IPSR_REG_POS (11U, 12U)
#define IPSR_SDA2_B                     IPSR_REG_POS (11U, 12U)

#define IPSR_SD1_CD                     IPSR_REG_POS (11U, 16U)
#define IPSR_SD1_WP                     IPSR_REG_POS (11U, 20U)

#define IPSR_SCK0                       IPSR_REG_POS (11U, 24U)
#define IPSR_SDA2_A                     IPSR_REG_POS (11U, 24U)
#define IPSR_SCK5_B                     IPSR_REG_POS (11U, 24U)

#define IPSR_RX0                        IPSR_REG_POS (11U, 28U)

/* peripheral function select register 12 */

#define IPSR_TX0                        IPSR_REG_POS (12U, 0U)
#define IPSR_CTS0                       IPSR_REG_POS (12U, 4U)

#define IPSR_RTS0                       IPSR_REG_POS (12U, 8U)
#define IPSR_SCL2_A                     IPSR_REG_POS (12U, 8U)

#define IPSR_RX1_A                      IPSR_REG_POS (12U, 12U)
#define IPSR_TX1_A                      IPSR_REG_POS (12U, 16U)
#define IPSR_CTS1                       IPSR_REG_POS (12U, 20U)
#define IPSR_RTS1                       IPSR_REG_POS (12U, 24U)

#define IPSR_SCK2                       IPSR_REG_POS (12U, 28U)
#define IPSR_SCIF_CLK_B                 IPSR_REG_POS (12U, 28U)

/* peripheral function select register 13 */

#define IPSR_TX2_A                      IPSR_REG_POS (13U, 0U)
#define IPSR_SD2_CD_B                   IPSR_REG_POS (13U, 0U)
#define IPSR_SCL1_A                     IPSR_REG_POS (13U, 0U)

#define IPSR_RX2_A                      IPSR_REG_POS (12U, 4U)
#define IPSR_SD2_WP_B                   IPSR_REG_POS (13U, 4U)
#define IPSR_SDA1_A                     IPSR_REG_POS (13U, 4U)

#define IPSR_HSCK0                      IPSR_REG_POS (13U, 8U)
#define IPSR_RX5_B                      IPSR_REG_POS (13U, 8U)

#define IPSR_HRX0                       IPSR_REG_POS (13U, 12U)
#define IPSR_HTX0                       IPSR_REG_POS (13U, 16U)

#define IPSR_HCTS0                      IPSR_REG_POS (13U, 20U)
#define IPSR_RX2_B                      IPSR_REG_POS (13U, 20U)

#define IPSR_HRTS0                      IPSR_REG_POS (13U, 24U)
#define IPSR_TX2_B                      IPSR_REG_POS (13U, 24U)

#define IPSR_MSIOF0_SYNC                IPSR_REG_POS (13U, 28U)
#define IPSR_TX5_B                      IPSR_REG_POS (13U, 28U)

/* peripheral function select register 14 */

#define IPSR_MSIOF0_SS1                 IPSR_REG_POS (14U, 0U)
#define IPSR_RX5_A                      IPSR_REG_POS (14U, 0U)

#define IPSR_MSIOF0_SS2                 IPSR_REG_POS (14U, 4U)
#define IPSR_TX5_A                      IPSR_REG_POS (14U, 4U)

#define IPSR_MLB_CLK                    IPSR_REG_POS (14U, 8U)
#define IPSR_SCL1_B                     IPSR_REG_POS (14U, 8U)


#define IPSR_MLB_SIG                    IPSR_REG_POS (14U, 12U)
#define IPSR_RX1_B                      IPSR_REG_POS (14U, 12U)
#define IPSR_SDA1_B                     IPSR_REG_POS (14U, 12U)

#define IPSR_MLB_DAT                    IPSR_REG_POS (14U, 16U)
#define IPSR_TX1_B                      IPSR_REG_POS (14U, 16U)

#define IPSR_SSI_SCK01239               IPSR_REG_POS (14U, 20U)
#define IPSR_SSI_WS01239                IPSR_REG_POS (14U, 24U)
#define IPSR_SSI_SDATA0                 IPSR_REG_POS (14U, 28U)

/* peripheral function select register 15 */

#define IPSR_SSI_SDATA1_A               IPSR_REG_POS (15U, 0U)
#define IPSR_SSI_SDATA2_A               IPSR_REG_POS (15U, 4U)
#define IPSR_SSI_SCK349                 IPSR_REG_POS (15U, 8U)
#define IPSR_SSI_WS349                  IPSR_REG_POS (15U, 12U)
#define IPSR_SSI_SDATA3                 IPSR_REG_POS (15U, 16U)
#define IPSR_SSI_SSI_SCK4               IPSR_REG_POS (15U, 20U)
#define IPSR_SSI_SSI_WS4                IPSR_REG_POS (15U, 24U)
#define IPSR_SSI_SSI_SDATA4             IPSR_REG_POS (15U, 28U)

/* peripheral function select register 16 */

#define IPSR_SSI_SCK6                   IPSR_REG_POS (16U, 0U)
#define IPSR_SSI_WS6                    IPSR_REG_POS (16U, 4U)
#define IPSR_SSI_SDATA6                 IPSR_REG_POS (16U, 8U)
#define IPSR_SSI_SCK78                  IPSR_REG_POS (16U, 12U)
#define IPSR_SSI_WS78                   IPSR_REG_POS (16U, 16U)
#define IPSR_SSI_SDATA7                 IPSR_REG_POS (16U, 20U)
#define IPSR_SSI_SDATA8                 IPSR_REG_POS (16U, 24U)
#define IPSR_SSI_SDATA9_A               IPSR_REG_POS (16U, 28U)

/* peripheral function select register 17 */

#define IPSR_AUDIO_CLKA_A               IPSR_REG_POS (17U, 0U)
#define IPSR_AUDIO_CLKB_B               IPSR_REG_POS (17U, 4U)
#define IPSR_USB0_PWEN                  IPSR_REG_POS (17U, 8U)
#define IPSR_USB0_OVC                   IPSR_REG_POS (17U, 12U)
#define IPSR_USB1_PWEN                  IPSR_REG_POS (17U, 16U)
#define IPSR_USB1_OVC                   IPSR_REG_POS (17U, 20U)
#define IPSR_USB30_PWEN                 IPSR_REG_POS (17U, 24U)
#define IPSR_USB30_OVC                  IPSR_REG_POS (17U, 28U)

/* peripheral function select register 18 */

#define IPSR_GP6_30                     IPSR_REG_POS (18U, 0U)
#define IPSR_GP6_31                     IPSR_REG_POS (18U, 4U)

/* DRV Control (DRVCTRL) registers */

#define DRVCTRL_H_MASK          7U      /* 3 bits for H type buffers */
#define DRVCTRL_F_MASK          3U      /* 2 bits for F type buffers */

#define DRVCTRL_REG_POS_MASK(reg, pos, mask) \
                                (((mask) << 12) | ((reg) << 8) | (pos))

#define DRVCTRL_BITMASK(rpm)    (((rpm) >> 12) & 0x0FU)
#define DRVCTRL_REG(rpm)        (((rpm) >> 8)  & 0x0FU)
#define DRVCTRL_SHIFT(rpm)      ((rpm) & 0x01FU)
#define DRVCTRL_MASK(rpm)       (DRVCTRL_BITMASK (rpm) << DRVCTRL_SHIFT (rpm))

#define DRVCTRL_AVB_RX_CTL      DRVCTRL_REG_POS_MASK (1U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_AVB_RXC         DRVCTRL_REG_POS_MASK (2U, 28U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_RD0         DRVCTRL_REG_POS_MASK (2U, 24U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_RD1         DRVCTRL_REG_POS_MASK (2U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_RD2         DRVCTRL_REG_POS_MASK (2U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_RD3         DRVCTRL_REG_POS_MASK (2U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_TX_CTL      DRVCTRL_REG_POS_MASK (2U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_AVB_TXC         DRVCTRL_REG_POS_MASK (2U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_AVB_TD0         DRVCTRL_REG_POS_MASK (2U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_AVB_TD1         DRVCTRL_REG_POS_MASK (3U, 28U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_TD2         DRVCTRL_REG_POS_MASK (3U, 24U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_TD3         DRVCTRL_REG_POS_MASK (3U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_TXCREFCLK   DRVCTRL_REG_POS_MASK (3U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_MDIO        DRVCTRL_REG_POS_MASK (3U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_MDC         DRVCTRL_REG_POS_MASK (3U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_AVB_MAGIC       DRVCTRL_REG_POS_MASK (3U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_AVB_PHY_INT     DRVCTRL_REG_POS_MASK (3U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_AVB_LINK        DRVCTRL_REG_POS_MASK (4U, 28U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_MATCH       DRVCTRL_REG_POS_MASK (4U, 24U, DRVCTRL_H_MASK)
#define DRVCTRL_AVB_CAPTURE     DRVCTRL_REG_POS_MASK (4U, 20U, DRVCTRL_H_MASK)

#define DRVCTRL_PWM1            DRVCTRL_REG_POS_MASK (5U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_PWM2            DRVCTRL_REG_POS_MASK (5U, 16U, DRVCTRL_H_MASK)

#define DRVCTRL_A8              DRVCTRL_REG_POS_MASK (6U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_A11             DRVCTRL_REG_POS_MASK (6U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_WE0             DRVCTRL_REG_POS_MASK (8U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_WE1             DRVCTRL_REG_POS_MASK (8U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_D14             DRVCTRL_REG_POS_MASK (11U, 28U, DRVCTRL_H_MASK)
#define DRVCTRL_D15             DRVCTRL_REG_POS_MASK (11U, 24U, DRVCTRL_H_MASK)

#define DRVCTRL_SD0_CLK         DRVCTRL_REG_POS_MASK (13U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_SD0_CMD         DRVCTRL_REG_POS_MASK (13U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_SD0_DAT0        DRVCTRL_REG_POS_MASK (13U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_SD0_DAT1        DRVCTRL_REG_POS_MASK (13U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD0_DAT2        DRVCTRL_REG_POS_MASK (13U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD0_DAT3        DRVCTRL_REG_POS_MASK (13U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_SD1_DAT0        DRVCTRL_REG_POS_MASK (14U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_SD1_DAT1        DRVCTRL_REG_POS_MASK (14U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_SD1_DAT2        DRVCTRL_REG_POS_MASK (14U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_SD1_DAT3        DRVCTRL_REG_POS_MASK (14U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD2_CLK         DRVCTRL_REG_POS_MASK (14U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD2_CMD         DRVCTRL_REG_POS_MASK (14U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_SD2_DAT0        DRVCTRL_REG_POS_MASK (15U, 28U, DRVCTRL_H_MASK)
#define DRVCTRL_SD2_DAT1        DRVCTRL_REG_POS_MASK (15U, 24U, DRVCTRL_H_MASK)
#define DRVCTRL_SD2_DAT2        DRVCTRL_REG_POS_MASK (15U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_SD2_DAT3        DRVCTRL_REG_POS_MASK (15U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_SD2_DS          DRVCTRL_REG_POS_MASK (15U, 12U, DRVCTRL_H_MASK)

#define DRVCTRL_SD3_CLK         DRVCTRL_REG_POS_MASK (15U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD3_CMD         DRVCTRL_REG_POS_MASK (15U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD3_DAT0        DRVCTRL_REG_POS_MASK (15U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_SD3_DAT1        DRVCTRL_REG_POS_MASK (16U, 28U, DRVCTRL_H_MASK)
#define DRVCTRL_SD3_DAT2        DRVCTRL_REG_POS_MASK (16U, 24U, DRVCTRL_H_MASK)
#define DRVCTRL_SD3_DAT3        DRVCTRL_REG_POS_MASK (16U, 20U, DRVCTRL_H_MASK)

#define DRVCTRL_SD3_DAT4        DRVCTRL_REG_POS_MASK (16U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_SD3_DAT5        DRVCTRL_REG_POS_MASK (16U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_SD3_DAT6        DRVCTRL_REG_POS_MASK (16U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD3_DAT7        DRVCTRL_REG_POS_MASK (16U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_SD3_DS          DRVCTRL_REG_POS_MASK (16U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_SD0_CD          DRVCTRL_REG_POS_MASK (17U, 30U, DRVCTRL_H_MASK)
#define DRVCTRL_SD0_WP          DRVCTRL_REG_POS_MASK (17U, 26U, DRVCTRL_H_MASK)
#define DRVCTRL_SD1_CD          DRVCTRL_REG_POS_MASK (17U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_SD1_WP          DRVCTRL_REG_POS_MASK (17U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_SCK0            DRVCTRL_REG_POS_MASK (17U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_CTS0            DRVCTRL_REG_POS_MASK (17U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_RTS0            DRVCTRL_REG_POS_MASK (18U, 28U, DRVCTRL_H_MASK)
#define DRVCTRL_RX1             DRVCTRL_REG_POS_MASK (18U, 24U, DRVCTRL_H_MASK)
#define DRVCTRL_TX1             DRVCTRL_REG_POS_MASK (18U, 20U, DRVCTRL_H_MASK)
#define DRVCTRL_CTS1            DRVCTRL_REG_POS_MASK (18U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_RTS1            DRVCTRL_REG_POS_MASK (18U, 12U, DRVCTRL_H_MASK)
#define DRVCTRL_SCK2            DRVCTRL_REG_POS_MASK (18U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_TX2             DRVCTRL_REG_POS_MASK (18U, 4U,  DRVCTRL_H_MASK)
#define DRVCTRL_RX2             DRVCTRL_REG_POS_MASK (18U, 0U,  DRVCTRL_H_MASK)

#define DRVCTRL_MLB_CLK         DRVCTRL_REG_POS_MASK (20U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_MLB_SIG         DRVCTRL_REG_POS_MASK (20U, 12U, DRVCTRL_H_MASK)

#define DRVCTRL_SSI_SCK6        DRVCTRL_REG_POS_MASK (22U, 8U,  DRVCTRL_H_MASK)
#define DRVCTRL_SSI_WS6         DRVCTRL_REG_POS_MASK (22U, 4U,  DRVCTRL_H_MASK)

#define DRVCTRL_USB3_PWEN       DRVCTRL_REG_POS_MASK (24U, 16U, DRVCTRL_H_MASK)
#define DRVCTRL_USB3_OVC        DRVCTRL_REG_POS_MASK (24U, 12U, DRVCTRL_H_MASK)

#define DRVCTRL_USB1_PWEN       DRVCTRL_REG_POS_MASK (24U, 24U, DRVCTRL_H_MASK)
#define DRVCTRL_USB1_OVC        DRVCTRL_REG_POS_MASK (24U, 20U, DRVCTRL_H_MASK)

#define DRVCTRL_USB0_PWEN       DRVCTRL_REG_POS_MASK (23U, 0U,  DRVCTRL_H_MASK)
#define DRVCTRL_USB0_OVC        DRVCTRL_REG_POS_MASK (24U, 28U, DRVCTRL_H_MASK)

/* Pull enable (PUEN) and control (PUD) registers */

#define PUEN_DIS                0U          /* disable pull-up/down function */
#define PUEN_EN                 1U          /* enable pull-up/down function */

#define PUD_DOWN                0U          /* pull-down enable */
#define PUD_UP                  1U          /* pull-up enable */

/* PUEN and PUD bit positions are the same */

#define PU_REG_PIN(reg, pin)    (((reg) << 8) | (pin))
#define PU_REG(regPin)          ((regPin) >> 8)
#define PU_MASK(regPin)         (1U << ((regPin) & 0x01FU))

#define PU_AVB_RX_CTL           PU_REG_PIN (0U, 15U)
#define PU_AVB_RXC              PU_REG_PIN (0U, 16U)
#define PU_AVB_RD0              PU_REG_PIN (0U, 17U)
#define PU_AVB_RD1              PU_REG_PIN (0U, 18U)
#define PU_AVB_RD2              PU_REG_PIN (0U, 19U)
#define PU_AVB_RD3              PU_REG_PIN (0U, 20U)
#define PU_AVB_TX_CTL           PU_REG_PIN (0U, 21U)
#define PU_AVB_TXC              PU_REG_PIN (0U, 22U)
#define PU_AVB_TD0              PU_REG_PIN (0U, 23U)
#define PU_AVB_TD1              PU_REG_PIN (0U, 24U)
#define PU_AVB_TD2              PU_REG_PIN (0U, 25U)
#define PU_AVB_TD3              PU_REG_PIN (0U, 26U)
#define PU_AVB_TXCREFCLK        PU_REG_PIN (0U, 27U)
#define PU_AVB_MDIO             PU_REG_PIN (0U, 28U)
#define PU_AVB_MDC              PU_REG_PIN (0U, 29U)
#define PU_AVB_MAGIC            PU_REG_PIN (0U, 30U)
#define PU_AVB_PHY_INT          PU_REG_PIN (0U, 31U)

#define PU_AVB_LINK             PU_REG_PIN (1U, 0U)
#define PU_AVB_MATCH            PU_REG_PIN (1U, 1U)
#define PU_AVB_CAPTURE          PU_REG_PIN (1U, 2U)
#define PU_PWM1                 PU_REG_PIN (1U, 10U)
#define PU_PWM2                 PU_REG_PIN (1U, 11U)
#define PU_A8                   PU_REG_PIN (1U, 20U)
#define PU_A11                  PU_REG_PIN (1U, 23U)

#define PU_WE0                  PU_REG_PIN (2U, 6U)
#define PU_WE1                  PU_REG_PIN (2U, 7U)
#define PU_D14                  PU_REG_PIN (2U, 24U)
#define PU_D15                  PU_REG_PIN (2U, 25U)

#define PU_SD0_CLK              PU_REG_PIN (3U, 10U)
#define PU_SD0_CMD              PU_REG_PIN (3U, 11U)
#define PU_SD0_DAT0             PU_REG_PIN (3U, 12U)
#define PU_SD0_DAT1             PU_REG_PIN (3U, 13U)
#define PU_SD0_DAT2             PU_REG_PIN (3U, 14U)
#define PU_SD0_DAT3             PU_REG_PIN (3U, 15U)

#define PU_SD1_DAT0             PU_REG_PIN (3U, 18U)
#define PU_SD1_DAT1             PU_REG_PIN (3U, 19U)
#define PU_SD1_DAT2             PU_REG_PIN (3U, 20U)
#define PU_SD1_DAT3             PU_REG_PIN (3U, 21U)
#define PU_SD2_CLK              PU_REG_PIN (3U, 22U)
#define PU_SD2_CMD              PU_REG_PIN (3U, 23U)
#define PU_SD2_DAT0             PU_REG_PIN (3U, 24U)
#define PU_SD2_DAT1             PU_REG_PIN (3U, 25U)
#define PU_SD2_DAT2             PU_REG_PIN (3U, 26U)
#define PU_SD2_DAT3             PU_REG_PIN (3U, 27U)
#define PU_SD2_DS               PU_REG_PIN (3U, 28U)

#define PU_SD3_CLK              PU_REG_PIN (3U, 29U)
#define PU_SD3_CMD              PU_REG_PIN (3U, 30U)
#define PU_SD3_DAT0             PU_REG_PIN (3U, 31U)
#define PU_SD3_DAT1             PU_REG_PIN (4U, 0U)
#define PU_SD3_DAT2             PU_REG_PIN (4U, 1U)
#define PU_SD3_DAT3             PU_REG_PIN (4U, 2U)
#define PU_SD3_DAT4             PU_REG_PIN (4U, 3U)
#define PU_SD3_DAT5             PU_REG_PIN (4U, 4U)
#define PU_SD3_DAT6             PU_REG_PIN (4U, 5U)
#define PU_SD3_DAT7             PU_REG_PIN (4U, 6U)
#define PU_SD3_DS               PU_REG_PIN (4U, 7U)
#define PU_SD0_CD               PU_REG_PIN (4U, 8U)
#define PU_SD0_WP               PU_REG_PIN (4U, 9U)
#define PU_SD1_CD               PU_REG_PIN (4U, 10U)
#define PU_SD1_WP               PU_REG_PIN (4U, 11U)
#define PU_SCK0                 PU_REG_PIN (4U, 12U)
#define PU_CTS0                 PU_REG_PIN (4U, 15U)
#define PU_RTS0                 PU_REG_PIN (4U, 16U)
#define PU_RX1                  PU_REG_PIN (4U, 17U)
#define PU_TX1                  PU_REG_PIN (4U, 18U)
#define PU_CTS1                 PU_REG_PIN (4U, 19U)
#define PU_RTS1                 PU_REG_PIN (4U, 20U)
#define PU_SCK2                 PU_REG_PIN (4U, 21U)
#define PU_TX2                  PU_REG_PIN (4U, 22U)
#define PU_RX2                  PU_REG_PIN (4U, 23U)

#define PU_SSI_SCK6             PU_REG_PIN (5U, 21U)
#define PU_USB2_PWEN            PU_SSI_SCK6
#define PU_SSI_WS6              PU_REG_PIN (5U, 22U)
#define PU_USB2_OVC             PU_SSI_WS6

#define PU_MLB_CLK              PU_REG_PIN (6U, 3U)
#define PU_MLB_SIG              PU_REG_PIN (6U, 4U)

#define PU_USB0_PWEN            PU_REG_PIN (5U, 31U)
#define PU_USB0_OVC             PU_REG_PIN (6U, 0U)

#define PU_USB1_PWEN            PU_REG_PIN (6U, 1U)
#define PU_USB1_OVC             PU_REG_PIN (6U, 2U)

#define PU_USB3_PWEN            PU_MLB_CLK
#define PU_USB3_OVC             PU_MLB_SIG

/* module Select Registers */

#define MOD_SEL_REG_POS_MASK(reg, pos, mask) \
                                (((mask) << 12) | ((reg) << 8) | (pos))

#define MOD_SEL_BITMASK(rpm)    (((rpm) >> 12) & 0x0FU)
#define MOD_SEL_REG(rpm)        (((rpm) >> 8)  & 0x0FU)
#define MOD_SEL_SHIFT(rpm)      ((rpm) & 0x01FU)
#define MOD_SEL_MASK(rpm)       (MOD_SEL_BITMASK (rpm) << MOD_SEL_SHIFT (rpm))

/* module Select Register 0 */

#define MOD_SEL_ADGA            MOD_SEL_REG_POS_MASK (0U, 3U,  0x03U)
#define MOD_SEL_CANFD0          MOD_SEL_REG_POS_MASK (0U, 5U,  0x01U)
#define MOD_SEL_ETHERAVB        MOD_SEL_REG_POS_MASK (0U, 12U, 0x01U)

#define MOD_SEL_HSCIF2          MOD_SEL_REG_POS_MASK (0U, 13U, 0x03U)
#define MOD_SEL_HSCIF1          MOD_SEL_REG_POS_MASK (0U, 16U, 0x01U)
#define MOD_SEL_HSCIF3          MOD_SEL_REG_POS_MASK (0U, 17U, 0x03U)
#define MOD_SEL_HSCIF4          MOD_SEL_REG_POS_MASK (0U, 19U, 0x01U)

#define MOD_SEL_I2C1            MOD_SEL_REG_POS_MASK (0U, 20U, 0x01U)
#define MOD_SEL_I2C2            MOD_SEL_REG_POS_MASK (0U, 21U, 0x01U)

#define MOD_SEL_LBSC            MOD_SEL_REG_POS_MASK (0U, 23U, 0x01U)

#define MOD_SEL_MSIOF1          MOD_SEL_REG_POS_MASK (0U, 24U, 0x07U)
#define MOD_SEL_MSIOF2          MOD_SEL_REG_POS_MASK (0U, 27U, 0x03U)
#define MOD_SEL_MSIOF3          MOD_SEL_REG_POS_MASK (0U, 29U, 0x07U)

/* module Select Register 1 */

#define MOD_SEL_PWM1            MOD_SEL_REG_POS_MASK (1U, 0U, 0x01U)
#define MOD_SEL_PWM2            MOD_SEL_REG_POS_MASK (1U, 1U, 0x01U)
#define MOD_SEL_PWM3            MOD_SEL_REG_POS_MASK (1U, 2U, 0x01U)
#define MOD_SEL_PWM4            MOD_SEL_REG_POS_MASK (1U, 3U, 0x01U)
#define MOD_SEL_PWM5            MOD_SEL_REG_POS_MASK (1U, 4U, 0x01U)
#define MOD_SEL_PWM6            MOD_SEL_REG_POS_MASK (1U, 5U, 0x01U)

#define MOD_SEL_RCAN0           MOD_SEL_REG_POS_MASK (1U, 6U, 0x01U)

#define MOD_SEL_SCIF            MOD_SEL_REG_POS_MASK (1U, 10U, 0x01U)
#define MOD_SEL_SCIF1           MOD_SEL_REG_POS_MASK (1U, 11U, 0x01U)
#define MOD_SEL_SCIF2           MOD_SEL_REG_POS_MASK (1U, 12U, 0x01U)
#define MOD_SEL_SCIF3           MOD_SEL_REG_POS_MASK (1U, 13U, 0x01U)
#define MOD_SEL_SCIF4           MOD_SEL_REG_POS_MASK (1U, 14U, 0x03U)

#define MOD_SEL_SDHI2           MOD_SEL_REG_POS_MASK (1U, 16U, 0x01U)
#define MOD_SEL_SSI1            MOD_SEL_REG_POS_MASK (1U, 20U, 0x01U)
#define MOD_SEL_TIMER_TMU1      MOD_SEL_REG_POS_MASK (1U, 26U, 0x01U)

/* module Select Register 2 */

#define MOD_SEL_ADG_C           MOD_SEL_REG_POS_MASK (2U, 17U, 0x01U)
#define MOD_SEL_ADG_B           MOD_SEL_REG_POS_MASK (2U, 18U, 0x01U)

#define MOD_SEL_TIMER_TMU2      MOD_SEL_REG_POS_MASK (2U, 19U, 0x01U)

#define MOD_SEL_SSI9            MOD_SEL_REG_POS_MASK (2U, 20U, 0x01U)
#define MOD_SEL_SSI2            MOD_SEL_REG_POS_MASK (2U, 21U, 0x01U)

#define MOD_SEL_NDFC            MOD_SEL_REG_POS_MASK (2U, 22U, 0x01U)

#define MOD_SEL_I2C6            MOD_SEL_REG_POS_MASK (2U, 23U, 0x07U)

#define MOD_SEL_SCIF5           MOD_SEL_REG_POS_MASK (2U, 26U, 0x03U)

#define MOD_SEL_I2C0            MOD_SEL_REG_POS_MASK (2U, 29U, 0x01U)
#define MOD_SEL_I2C3            MOD_SEL_REG_POS_MASK (2U, 30U, 0x01U)
#define MOD_SEL_I2C5            MOD_SEL_REG_POS_MASK (2U, 31U, 0x01U)

/* POC Control Reigster */

#define POC_REG_POS_MASK(reg, pos, mask) (((mask) << 12) | ((reg) << 8) | (pos))

#define POC_BITMASK(rpm)    (((rpm) >> 12) & 0xfffffU)
#define POC_REG(rpm)        (((rpm) >> 8)  & 0x0FU)
#define POC_SHIFT(rpm)      ((rpm) & 0x01FU)
#define POC_MASK(rpm)       (POC_BITMASK (rpm) << POC_SHIFT (rpm))

#define POC_SD0             POC_REG_POS_MASK (0U, 0U,  0x3fU)   /* bit0-5 */
#define POC_SD3             POC_REG_POS_MASK (0U, 19U, 0x7ffU)  /* bit19-29 */

/* typedefs */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCvxbFdtRsRzg2PfcPinMuxh */
