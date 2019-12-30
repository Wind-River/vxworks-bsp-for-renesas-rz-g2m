/* vxbFdtRsRzg2CpgMssr.c - Renesas RZ/G2 CPG and MSSR driver for VxBus */

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

/*
DESCRIPTION

This module implements a VxBus compliant clock controller driver for the
Renesas RZ/G2 Clock Pulse Generator and Module Standby / Software Reset
(CPG/MSSR) modules. The CPG generates clocks for CPU cores and peripherals. The
MSSR clocks control peripheral module operation. For more information, see the
\tb R-Car Series, 3rd Generation User's Manual: Hardware.

This driver must be present in order to support other drivers that require a
CPG or MSSR clock source. To add this driver to the vxWorks image, add the
DRV_CLK_FDT_CPG_MSSR component to the kernel configuration.

The CPG device should be bound to a device tree node with the following
properties:

\cs
compatible:     Specify the programming model for the device.
                It should be set to "renesas,r8a774a1-cpg-mssr" and is used
                by VxBus GEN2 for device driver selection.

reg:            Specify the address and length of the device register block.
\ce

Below is an example:

\cs
    cpg: clock-controller@e6150000
        {
        compatible = "renesas,r8a774a1-cpg-mssr";
        reg = <0x00000000 0xe6150000 0x0 0x1000>;
        clocks = <&extal_clk>, <&extalr_clk>;
        clock-names = "extal", "extalr";
        #clock-cells = <1>;
        };
\ce

In order to configure the CPG clock domain, the EXTAL and EXTALR external clock
inputs should be bound to a device tree node with the following properties:

\cs
compatible:     Specify the programming model for the device.
                It should include "fixed-clock".

clock-frequency:Specify the external clock frequency.
\ce

Below is an example:

\cs
    extal_clk: extal
        {
        compatible = "fixed-clock";
        #clock-cells = <0>;
        clock-frequency = <16666600>;
        };
\ce

When binding client devices (clock users) in the device tree the <clocks>
property is required, listing the required clock source and parameter pairs. The
parameter is the clock index given in the tables below. If clocks are to be
referenced by name, then the <clock-names> property should be provided and
should list the clocks in the same order as the <clocks> property. Client device
drivers may require specific <clock-names> which should be defined in the driver
binding. An example of a device tree clock user node is shown below:

\cs
    tmu0: timer@e61e0000
        {
        compatible = "renesas,tmu-r8a774a1";
        reg = <0x00000000 0xe61e0000 0x0 0x30>;
        interrupts = <168 0 4>, /@ ch0 @/
                     <169 0 4>, /@ ch1 @/
                     <170 0 4>; /@ ch2 @/
        interrupt-parent = <&gic>;
        clocks = <&cpg 55>,     /@ CP input clock @/
                 <&cpg 63>;     /@ TMU0 module clock @/
        clock-names = "inputClk", "moduleClk";
        status = "okay";
        };
\ce

The clock names and associated parameters for clocks in the CPG domain:

\ts
  0  extal       | 15 z2         | 30 s1d4       | 45 sd3        | 60 oscclk
  1  extalr      | 16 zg         | 31 s2d1       | 46 lb         | 61 rclk
  2  pll0        | 17 zt         | 32 s2d2       | 47 rpc
  3  pll1        | 18 ztr        | 33 s2d4       | 48 rpcd2
  4  pll2        | 19 ztrd2      | 34 s3d1       | 49 rc
  5  pll3        | 20 zx         | 35 s3d2       | 50 rcd2
  6  pll4        | 21 s0d1       | 36 s3d4       | 51 hdmi
  7  pll1div2    | 22 s0d2       | 37 cl         | 52 mso
  8  pll1div4    | 23 s0d3       | 38 sd0h       | 53 canfd
  9  s0          | 24 s0d4       | 39 sd0        | 54 ddr
 10  s1          | 25 s0d6       | 40 sd1h       | 55 zb3
 11  s2          | 26 s0d8       | 41 sd1        | 56 zb3d2
 12  s3          | 27 s0d12      | 42 sd2h       | 57 zb3d4
 13  sdsrc       | 28 s1d1       | 43 sd2        | 58 cpex
 14  z           | 29 s1d2       | 44 sd3h       | 59 cp

\te

The clock names and associated parameters for clocks in the MSSR domain:

\ts
62   RT-DMAC        |  109  TPU0        |  156  DU0          |  203  I2C-IF5
63   VDPB           |  110  CMT0        |  157  DU1          |  204  I2C-IF6
64   VCPLF          |  111  CMT1        |  158  DU2          |  205  RPC
65   VCPL4          |  112  CMT2        |  159  DU3          |  206  CAN-IF0
66   iVDP1C         |  113  CMT3        |  160  TCON0        |  207  CAN-IF1
67   TMU0           |  114  SUCMT       |  161  TCON1        |  208  CAN-FD
68   TMU1           |  115  INTC-AP     |  162  DOC0         |  209  GPIO0
69   TMU2           |  116  INTC-EX     |  163  DOC1         |  210  GPIO1
70   TMU3           |  117  RWDT        |  164  CSI40        |  211  GPIO2
71   TMU4           |  118  SBROM       |  165  CSI41        |  212  GPIO3
72   FDP1-0         |  119  PWM         |  166  CSI20        |  213  GPIO4
73   FDP1-1         |  120  THS/TSC     |  167  CSI21        |  214  GPIO5
74   FDP1-2         |  121  HSCIF0      |  168  DCU          |  215  GPIO6
75   3DGE           |  122  HSCIF1      |  169  CMM0         |  216  GPIO7
76   SSP1           |  123  HSCIF2      |  170  CMM1         |  217  FM
77   TSIF0          |  124  HSCIF3      |  171  CMM2         |  218  IR
78   TSIF1          |  125  HSCIF4      |  172  CMM3         |  219  SPIF
79   STBE           |  126  ADSP        |  173  HS-USB-IF    |  220  GYROIF
80   STB            |  127  A-DMAC0     |  174  EHCI/OHCI0   |  221  SCU-SRC0
81   SCEG-PUB       |  128  A-DMAC1     |  175  EHCI/OHCI1   |  222  SCU-SRC1
82   SCEG-SEC       |  129  VSPI0       |  176  EHCI/OHCI2   |  223  SCU-SRC2
83   SCEG-PKA       |  130  VSPI1       |  177  IMP          |  224  SCU-SRC3
84   SYS-DMAC0      |  131  VSPI2       |  178  IMR0         |  225  SCU-SRC4
85   SYS-DMAC1      |  132  VSPBD       |  179  IMR1         |  226  SCU-SRC5
86   SYS-DMAC2      |  133  VSPBC       |  180  IMR2         |  227  SCU-SRC6
87   MFIS           |  134  VSPD0       |  181  IMR3         |  228  SCU-SRC7
88   MSIOF0         |  135  VSPD1       |  182  SATA-IF      |  229  SCU-SRC8
89   MSIOF1         |  136  VSPD2       |  183  EAVB-IF      |  230  SCU-SRC9
90   MSIOF2         |  137  VSPD3       |  184  VIN0         |  231  SCU-MIX0
91   MSIOF3         |  138  FCPCS       |  185  VIN1         |  232  SCU-MIX1
92   SCIF0          |  139  FCPCI0      |  186  VIN2         |  233  SCU-DVC0
93   SCIF1          |  140  FCPCI1      |  187  VIN3         |  236  SCU-DVC1
94   SCIF3          |  141  FCPF0       |  188  VIN4         |  235  SCU-ALL
95   SCIF4          |  142  FCPF1       |  189  VIN5         |  236  SSI0
96   SCIF5          |  143  FCPF2       |  190  VIN6         |  237  SSI1
97   USB-DMAC1      |  144  FCPVI0      |  191  VIN7         |  238  SSI2
98   USB-DMAC0      |  145  FCPVI1      |  192  MLP          |  239  SSI3
99   USB3.0-IF0     |  146  FCPVI2      |  193  I2C-IF0      |  240  SSI4
100  USB3.0-IF1     |  147  FCPVB0      |  194  I2C-IF1      |  241  SSI5
101  PCIEC0         |  148  FCPVB1      |  195  I2C-IF2      |  242  SSI6
102  PCIEC1         |  149  FCPVD0      |  196  I2C-IF3      |  243  SSI7
103  SD-IF0         |  150  FCPVD1      |  197  I2C-IF4      |  244  SSI8
104  SD-IF1         |  151  FCPVD2      |  198  DVFS         |  245  SSI9
105  SD-IF2         |  152  FCPVD3      |  199  MLM          |  246  SSI-ALL
106  SD-IF3         |  153  HDMI-IF0    |  200  DTCP
107  CRC0           |  154  HDMI-IF1    |  201  ADG
108  CRC1           |  155  LVDS-IF     |  202  SIM

\te

The clkRateSet, clkParentSet and clkExtCtrl methods are not supported.

INCLUDE FILES: vxBus.h vxbClkLib.h string.h vxbFdtLib.h

SEE ALSO: vxbClkLib,
\tb RZ/G Series, 2nd Generation User’s Manual: Hardware
*/

/* includes */

#include <vxWorks.h>
#include <string.h>
#include <stdio.h>
#include <intLib.h>
#include <semLib.h>
#include <vxCpuLib.h>
#include <vsbConfig.h>

#include <subsys/clk/vxbClkLib.h>
#include <hwif/vxBus.h>
#include <hwif/buslib/vxbFdtLib.h>

#include <vxbFdtRsRzg2CpgMssr.h>
#include <vxbFdtRsRzg2Rst.h>

/* defines */

/* debug macro */

#undef  RZG2_CPG_DBG
#ifdef  RZG2_CPG_DBG

/* turning local symbols into global symbols */

#ifdef  LOCAL
#undef  LOCAL
#define LOCAL
#endif

#include <private/kwriteLibP.h>    /* _func_kprintf */
#define CPG_DBG_OFF          0x00000000U
#define CPG_DBG_ISR          0x00000001U
#define CPG_DBG_ERR          0x00000002U
#define CPG_DBG_INFO         0x00000004U
#define CPG_DBG_ALL          0xffffffffU

LOCAL UINT32 cpgDbgMask = CPG_DBG_ALL;

#define RZG2_DBG_MSG(mask, ...)                                  \
    do                                                           \
        {                                                        \
        if ((cpgDbgMask & (mask)) || ((mask) == CPG_DBG_ALL))   \
            {                                                    \
            if (_func_kprintf != NULL)                           \
                {                                                \
                (* _func_kprintf)(__VA_ARGS__);                  \
                }                                                \
            }                                                    \
        }                                                        \
    while ((FALSE))
#else
#define RZG2_DBG_MSG(...)
#endif  /* RZG2_CPG_DBG */

#define CPG_DOMAIN_NAME     "CPG"
#define MSSR_DOMAIN_NAME    "MSSR"

/* typedefs */

/* structure to store the CPG information */

typedef struct vxbFdtCpgMssrInstance
    {
    VXB_DEV_ID      pDev;           /* VxBus driver ID */
    VXB_RESOURCE *  pCpgClkList;    /* static clock list */
    VXB_RESOURCE *  memRes;         /* mapped memory resource */
    VIRT_ADDR       regBase;        /* mapped CPG register base */
    void *          handle;         /* register memory access handle */
    SEM_ID          semMutex;       /* register access protection */
    } VXB_FDT_CPG_INSTANCE;

/* CPG specific dividers */

typedef enum cpgDivType
    {
    DIV_NONE,
    DIV_PLL,
    DIV_SYSCPU,
    DIV_RTCPU,
    DIV_COMMON,
    DIV_EXTAL,
    DIV_SDH,
    DIV_SD,
    DIV_LB,
    DIV_RPCSRC,
    DIV_HDMIIF,
    DIV_CSI0,
    DIV_CSIREF,
    DIV_MSIOF,
    DIV_CANFD
    } CPG_DIV_TYPE;

/* structures to define clock tree */

typedef struct cpgClkDesc
    {
    char *          name;           /* name for reference */
    int             parentIndex;    /* index to parent clock in table */
    VXB_CLK_TYPE    type;           /* clock type */
    UINT32          mult;           /* multiplier for VXB_CLK_FIX_FACTOR */
    UINT32          div;            /* divisor for VXB_CLK_FIX_FACTOR */
    UINT            index;          /* index for clocks of same type eg PLL */
    CPG_DIV_TYPE    divider;        /* CPG specific divider type */
    } CPG_CLK_DESC;

typedef struct cpgClkFactor
    {
    UINT32 mult;                    /* fixed factor multiplier */
    UINT32 div;                     /* fixed factor divisor */
    } CPG_CLK_FACTOR;

typedef struct cpgClkVar
    {
    CPG_DIV_TYPE    type;           /* specific type of divider/multiplier */
    UINT            index;          /* index for multiple instances */
    } CPG_CLK_VAR;

typedef union cpgClkData
    {
    UINT64          rate;           /* fixed rate */
    CPG_CLK_FACTOR  factor;         /* fixed factor */
    CPG_CLK_VAR     variable;       /* variable type */
    } CPG_CLK_DATA;

typedef struct mssrClkData
    {
    UINT32 moduleReg;
    UINT32 regBit;
    } MSSR_CLK_DATA;

typedef struct mssrClkDesc
    {
    char *          name;
    MSSR_CLK_DATA   moduleBit;
    int             parentIndex;    /* index to parent clock in table */
    } MSSR_CLK_DESC;

/* forward declarations */

LOCAL STATUS vxbFdtRzg2CpgMssrProbe (VXB_DEV_ID pDev);
LOCAL STATUS vxbFdtRzg2CpgMssrAttach (VXB_DEV_ID pDev);
LOCAL STATUS rzg2CpgMssrClkEnable (struct vxb_clk * pClk);
LOCAL STATUS rzg2CpgMssrClkDisable (struct vxb_clk * pClk);
LOCAL UINT64 rzg2CpgMssrClkRateGet (struct vxb_clk * pClk, UINT64 parentRate);
LOCAL UINT32 rzg2CpgMssrClkStatusGet (struct vxb_clk * pClk);
LOCAL STATUS rzg2CpgMssrClkInit (struct vxb_clk * pClk);

/* locals */

LOCAL VXB_DRV_METHOD vxbFdtRzg2CpgMssrMethodList[] =
    {
        { VXB_DEVMETHOD_CALL(vxbDevProbe),    vxbFdtRzg2CpgMssrProbe },
        { VXB_DEVMETHOD_CALL(vxbDevAttach),   vxbFdtRzg2CpgMssrAttach },
        VXB_DEVMETHOD_END
    };

LOCAL const VXB_FDT_DEV_MATCH_ENTRY vxbFdtRzg2CpgMssrMatch [] =
    {
        {
        "renesas,r8a774a1-cpg-mssr",
        NULL
        },
        {}                                      /* Empty terminated list */
    };

/* CPG and MSSR domains */

LOCAL VXB_CLK_DOMAIN_REG rzg2CpgDomainReg =
    {
    CPG_DOMAIN_NAME,
    NULL,
    NULL
    };

LOCAL VXB_CLK_DOMAIN_REG rzg2MssrDomainReg =
    {
    MSSR_DOMAIN_NAME,
    NULL,
    NULL
    };

/* CPG and MSSR clocks */

LOCAL VXB_CLK_FUNC rzg2CpgMssrMethods =
    {
    rzg2CpgMssrClkEnable,       /* clkEnable */
    rzg2CpgMssrClkDisable,      /* clkDisable */
    NULL,                       /* No clkRateSet */
    rzg2CpgMssrClkRateGet,      /* clkRateGet */
    rzg2CpgMssrClkStatusGet,    /* clkStatusGet */
    NULL,                       /* No clkParentSet */
    rzg2CpgMssrClkInit,         /* clkInit */
    NULL                        /* No clkExtCtrl */
    };

/*
 * Map the CPG clock domain
 *
 * The rzg2CpgClocks table below describes the CPG clock tree structure defined by
 * the CPG block diagram in the RZ/G2 hardware manual. Clock names are as
 * in the manual, but lower case.
 *
 * It starts with the EXTAL root, then internal clocks and clock outputs. Parent
 * clock sources precede children, since the children reference their parent's
 * index to build the tree. This table is used to initialise the list of clocks
 * registered in the CPG clock domain with vxbClkLib.
 *
 * The clock parameter published for reference by CPG clock users is the clock's
 * index in this table.
 */

LOCAL const CPG_CLK_DESC rzg2CpgClocks [] =
    {
    /* name     :  parent  :  VxBus type  : mult : div : index : divider */

    /* external clock input */

    { "extal",      (-1), VXB_CLK_FIX_RATE, 0U, 0U, 0U, DIV_NONE },     /* 0 */
    { "extalr",     (-1), VXB_CLK_FIX_RATE, 0U, 0U, 0U, DIV_NONE },     /* 1 */

    /* internal clocks */

    { "pll0",       0,  VXB_CLK_PLL,        0U, 0U, 0U, DIV_PLL },      /* 2 */
    { "pll1",       0,  VXB_CLK_PLL,        1U, 2U, 1U, DIV_PLL },      /* 3 */
    { "pll2",       0,  VXB_CLK_PLL,        0U, 0U, 2U, DIV_PLL },      /* 4 */
    { "pll3",       0,  VXB_CLK_PLL,        0U, 0U, 3U, DIV_PLL },      /* 5 */
    { "pll4",       0,  VXB_CLK_PLL,        0U, 0U, 4U, DIV_PLL },      /* 6 */
    { "pll1div2",   3,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 7 */
    { "pll1div4",   3,  VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 8 */

    { "s0",         7,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 9 */
    { "s1",         7,  VXB_CLK_FIX_FACTOR, 1U, 3U, 0U, DIV_NONE },     /* 10 */
    { "s2",         7,  VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 11 */
    { "s3",         7,  VXB_CLK_FIX_FACTOR, 1U, 6U, 0U, DIV_NONE },     /* 12 */
    { "sdsrc",      7,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 13 */

    /* clock outputs */

    { "z",          2,  VXB_CLK_DIVIDER,    0U, 0U, 1U, DIV_SYSCPU },   /* 14 */
    { "z2",         4,  VXB_CLK_DIVIDER,    0U, 0U, 2U, DIV_SYSCPU },   /* 15 */

    { "zg",         6,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 16 */

    { "zt",         7,  VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_COMMON },   /* 17 */
    { "ztr",        7,  VXB_CLK_DIVIDER,    0U, 0U, 1U, DIV_COMMON },   /* 18 */
    { "ztrd2",      7,  VXB_CLK_DIVIDER,    0U, 0U, 2U, DIV_COMMON },   /* 19 */
    { "zx",         7,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 20 */

    { "s0d1",       9,  VXB_CLK_FIX_FACTOR, 1U, 1U, 0U, DIV_NONE },     /* 21 */
    { "s0d2",       9,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 22 */
    { "s0d3",       9,  VXB_CLK_FIX_FACTOR, 1U, 3U, 0U, DIV_NONE },     /* 23 */
    { "s0d4",       9,  VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 24 */
    { "s0d6",       9,  VXB_CLK_FIX_FACTOR, 1U, 6U, 0U, DIV_NONE },     /* 25 */
    { "s0d8",       9,  VXB_CLK_FIX_FACTOR, 1U, 8U, 0U, DIV_NONE },     /* 26 */
    { "s0d12",      9,  VXB_CLK_FIX_FACTOR, 1U, 12U, 0U, DIV_NONE },    /* 27 */

    { "s1d1",       10, VXB_CLK_FIX_FACTOR, 1U, 1U, 0U, DIV_NONE },     /* 28 */
    { "s1d2",       10, VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 29 */
    { "s1d4",       10, VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 30 */

    { "s2d1",       11, VXB_CLK_FIX_FACTOR, 1U, 1U, 0U, DIV_NONE },     /* 31 */
    { "s2d2",       11, VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 32 */
    { "s2d4",       11, VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 33 */

    { "s3d1",       12, VXB_CLK_FIX_FACTOR, 1U, 1U, 0U, DIV_NONE },     /* 34 */
    { "s3d2",       12, VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 35 */
    { "s3d4",       12, VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 36 */

    { "cl",         7,  VXB_CLK_FIX_FACTOR, 1U, 48U, 0U, DIV_NONE },    /* 37 */

    { "sd0h",       13, VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_SDH },      /* 38 */
    { "sd0",        38, VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_SD },       /* 39 */

    { "sd1h",       13, VXB_CLK_DIVIDER,    0U, 0U, 1U, DIV_SDH },      /* 40 */
    { "sd1",        40, VXB_CLK_DIVIDER,    0U, 0U, 1U, DIV_SD },       /* 41 */

    { "sd2h",       13, VXB_CLK_DIVIDER,    0U, 0U, 2U, DIV_SDH },      /* 42 */
    { "sd2",        42, VXB_CLK_DIVIDER,    0U, 0U, 2U, DIV_SD },       /* 43 */

    { "sd3h",       13, VXB_CLK_DIVIDER,    0U, 0U, 3U, DIV_SDH },      /* 44 */
    { "sd3",        40, VXB_CLK_DIVIDER,    0U, 0U, 3U, DIV_SD },       /* 45 */

    { "lb",         9, VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_LB },        /* 46 */

    { "rpc",        3,  VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_RPCSRC },   /* 47 */
    { "rpcd2",      3,  VXB_CLK_DIVIDER,    0U, 0U, 1U, DIV_RPCSRC },   /* 48 */

    { "cr",         8,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 49 */
    { "crd2",       8,  VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 50 */
    
    { "hdmi",       8,  VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_HDMIIF },   /* 51 */

    { "mso",        8,  VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_MSIOF },    /* 52 */
    { "canfd",      8,  VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_CANFD },    /* 53 */

    { "ddr",        5,  VXB_CLK_FIX_FACTOR,  1U, 2U, 0U, DIV_NONE },    /* 54 */
    { "zb3",        54,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },    /* 55 */
    { "zb3d2",      54,  VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },    /* 56 */
    { "zb3d4",      54,  VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },    /* 57 */

    { "cpex",       0,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 58 */
    { "cp",         0,  VXB_CLK_FIX_FACTOR, 1U, 2U, 0U, DIV_NONE },     /* 59 */

    { "oscclk",     0,  VXB_CLK_DIVIDER,    0U, 0U, 0U, DIV_EXTAL },    /* 60 */
    { "rclk",       60, VXB_CLK_FIX_FACTOR, 1U, 4U, 0U, DIV_NONE },     /* 61 */
   };
#define RZG2_CPG_TOTAL_CLOCKS    (NELEMENTS (rzg2CpgClocks))

/*
 * Map the MSSR clock domain
 *
 * The rzg2MssrClocks table below describes the MSR clocks defined by the MSSR
 * register descriptions in the RZ/G2M hardware manual. Clock names are
 * upper case as in the manual.
 *
 * Each module clock is identified by a single bit in a register, so for
 * convenience the list is ordered by register number and bit position. This
 * table is used to initialise the list of clocks registered in the MSSR clock
 * domain with vxbClkLib.
 *
 * The clock parameter published for reference by CPG module clock users is the
 * clock's index in this table, offset by the number of CPG clocks, as indicated
 * by the comment at the start of each register group.
 */

LOCAL const MSSR_CLK_DESC rzg2MssrClocks [] =
    {
    /*  name         reg  bit    parentClk */
    { "RT-DMAC",    { 0U,  21U } },         /* 62 */

    { "VDPB",       { 1U,  31U } },         /* 63 */
    { "VCPLF",      { 1U,  30U } },
    { "VCPL4",      { 1U,  29U } },

    { "iVDP1C",     { 1U,  28U } },         /* 66 */

    { "TMU0",       { 1U,  25U } },         /* 67 */
    { "TMU1",       { 1U,  24U } },
    { "TMU2",       { 1U,  23U } },
    { "TMU3",       { 1U,  22U } },
    { "TMU4",       { 1U,  21U } },

    { "FDP1-0",     { 1U,  19U } },         /* 72 */
    { "FDP1-1",     { 1U,  18U } },
    { "FDP1-2",     { 1U,  17U } },

    { "3DGE",       { 1U,  12U } },         /* 75 */
    { "SSP1",       { 1U,  9U  } },

    { "TSIF0",      { 1U,  8U  } },         /* 77 */
    { "TSIF1",      { 1U,  7U  } },

    { "STBE",       { 1U,  5U  } },         /* 79 */
    { "STB",        { 1U,  4U  } },

    { "SCEG_PUB",   { 2U,  29U } },         /* 81 */
    { "SCEG_SEC",   { 2U,  28U } },
    { "SCEG_PKA",   { 2U,  26U } },

    { "SYS-DMAC0",  { 2U,  19U } },         /* 84 */
    { "SYS-DMAC1",  { 2U,  18U } },
    { "SYS-DMAC2",  { 2U,  17U } },

    { "MFIS",       { 2U,  13U } },         /* 87 */

    { "MSIOF0",     { 2U,  11U } },         /* 88 */
    { "MSIOF1",     { 2U,  10U } },
    { "MSIOF2",     { 2U,  9U  } },
    { "MSIOF3",     { 2U,  8U  } },

    { "SCIF0",      { 2U,  7U  } },         /* 92 */
    { "SCIF1",      { 2U,  6U  } },
    { "SCIF3",      { 2U,  4U  } },
    { "SCIF4",      { 2U,  3U  } },
    { "SCIF5",      { 2U,  2U  } },

    { "USB-DMAC1",  { 3U,  31U } },         /* 97 */
    { "USB-DMAC0",  { 3U,  30U } },
    { "USB3.0-IF0", { 3U,  28U } },
    { "USB3.0-IF1", { 3U,  27U } },

    { "PCIEC0",     { 3U,  19U } },         /* 101 */
    { "PCIEC1",     { 3U,  18U } },

    { "SD-IF0",     { 3U,  14U } },         /* 103 */
    { "SD-IF1",     { 3U,  13U } },
    { "SD-IF2",     { 3U,  12U } },
    { "SD-IF3",     { 3U,  11U } },

    { "CRC0",       { 3U,  7U  } },         /* 107 */
    { "CRC1",       { 3U,  6U  } },

    { "TPU0",       { 3U,  4U  } },         /* 109 */

    { "CMT0",       { 3U,  3U  } },         /* 110 */
    { "CMT1",       { 3U,  2U  } },
    { "CMT2",       { 3U,  1U  } },
    { "CMT3",       { 3U,  0U  } },

    { "SUCMT",      { 4U,  31U } },         /* 114 */
    { "INTC-AP",    { 4U,  8U  } },
    { "INTC-EX",    { 4U,  7U  } },

    { "RWDT",       { 4U,  2U  } },         /* 117 */
    { "SBROM",      { 5U,  30U } },
    { "PWM",        { 5U,  23U } },
    { "THS/TSC",    { 5U,  22U } },

    { "HSCIF0",     { 5U,  20U } },         /* 121 */
    { "HSCIF1",     { 5U,  19U } },
    { "HSCIF2",     { 5U,  18U } },
    { "HSCIF3",     { 5U,  17U } },
    { "HSCIF4",     { 5U,  16U } },

    { "ADSP",       { 5U,  6U  } },         /* 126 */
    { "A-DMAC0",    { 5U,  2U  } },
    { "A-DMAC1",    { 5U,  1U  } },

    { "VSPI0",      { 6U,  31U } },         /* 129 */
    { "VSPI1",      { 6U,  30U } },
    { "VSPI2",      { 6U,  29U } },

    { "VSPBD",      { 6U,  26U } },         /* 132 */
    { "VSPBC",      { 6U,  24U } },

    { "VSPD0",      { 6U,  23U } },         /* 134 */
    { "VSPD1",      { 6U,  22U } },
    { "VSPD2",      { 6U,  21U } },
    { "VSPD3",      { 6U,  20U } },

    { "FCPCS",      { 6U,  19U } },         /* 138 */
    { "FCPCI0",     { 6U,  17U } },
    { "FCPCI1",     { 6U,  16U } },

    { "FCPF0",      { 6U,  15U } },         /* 141 */
    { "FCPF1",      { 6U,  14U } },
    { "FCPF2",      { 6U,  13U } },

    { "FCPVI0",     { 6U,  11U } },         /* 144 */
    { "FCPVI1",     { 6U,  10U } },
    { "FCPVI2",     { 6U,  9U  } },

    { "FCPVB0",     { 6U,  7U  } },         /* 147 */
    { "FCPVB1",     { 6U,  6U  } },

    { "FCPVD0",     { 6U,  3U  } },         /* 149 */
    { "FCPVD1",     { 6U,  2U  } },
    { "FCPVD2",     { 6U,  1U  } },
    { "FCPVD3",     { 6U,  0U  } },

    { "HDMI-IF0",   { 7U,  29U }, 47 },     /* 153 */
    { "HDMI-IF1",   { 7U,  28U }, 47 },

    { "LVDS-IF",    { 7U,  27U } },         /* 155 */

    { "DU0",        { 7U,  24U } },         /* 156 */
    { "DU1",        { 7U,  23U } },
    { "DU2",        { 7U,  22U } },
    { "DU3",        { 7U,  21U } },

    { "TCON0",      { 7U,  20U } },         /* 160 */
    { "TCON1",      { 7U,  19U } },

    { "DOC0",       { 7U,  18U } },         /* 162 */
    { "DOC1",       { 7U,  17U } },

    { "CSI40",      { 7U,  16U }, 48 },     /* 164 */
    { "CSI41",      { 7U,  15U }, 48 },
    { "CSI20",      { 7U,  14U }, 48 },
    { "CSI21",      { 7U,  13U }, 48 },

    { "DCU",        { 7U,  12U } },         /* 168 */

    { "CMM0",       { 7U,  11U } },         /* 169 */
    { "CMM1",       { 7U,  10U } },
    { "CMM2",       { 7U,  9U  } },
    { "CMM3",       { 7U,  8U  } },

    { "HS-USB-IF",  { 7U,  4U  } },         /* 173 */
    { "EHCI/OHCI0", { 7U,  3U  } },
    { "EHCI/OHCI1", { 7U,  2U  } },
    { "EHCI/OHCI2", { 7U,  1U  } },

    { "IMP",        { 8U,  24U } },         /* 177 */
    { "IMR0",       { 8U,  23U } },
    { "IMR1",       { 8U,  22U } },
    { "IMR2",       { 8U,  21U } },
    { "IMR3",       { 8U,  20U } },

    { "SATA-IF",    { 8U,  15U } },         /* 182 */
    { "EAVB-IF",    { 8U,  12U } },

    { "VIN0",       { 8U,  11U } },         /* 184 */
    { "VIN1",       { 8U,  10U } },
    { "VIN2",       { 8U,  9U  } },
    { "VIN3",       { 8U,  8U  } },
    { "VIN4",       { 8U,  7U  } },
    { "VIN5",       { 8U,  6U  } },
    { "VIN6",       { 8U,  5U  } },
    { "VIN7",       { 8U,  4U  } },

    { "MLP",        { 8U,  2U  } },         /* 192 */

    { "I2C-IF0",    { 9U,  31U } },         /* 193 */
    { "I2C-IF1",    { 9U,  30U } },
    { "I2C-IF2",    { 9U,  29U } },
    { "I2C-IF3",    { 9U,  28U } },
    { "I2C-IF4",    { 9U,  27U } },

    { "DVFS",       { 9U,  26U } },         /* 198 */
    { "MLM",        { 9U,  24U } },
    { "DTCP",       { 9U,  23U } },
    { "ADG",        { 9U,  22U } },
    { "SIM",        { 9U,  20U } },

    { "I2C-IF5",    { 9U,  19U } },         /* 203 */
    { "I2C-IF6",    { 9U,  18U } },

    { "RPC",        { 9U,  17U } },         /* 205 */

    { "CAN-IF0",    { 9U,  16U } },         /* 206 */
    { "CAN-IF1",    { 9U,  15U } },
    { "CAN-FD",     { 9U,  14U } },

    { "GPIO0",      { 9U,  12U } },         /* 209 */
    { "GPIO1",      { 9U,  11U } },
    { "GPIO2",      { 9U,  10U } },
    { "GPIO3",      { 9U,  9U  } },
    { "GPIO4",      { 9U,  8U  } },
    { "GPIO5",      { 9U,  7U  } },
    { "GPIO6",      { 9U,  6U  } },
    { "GPIO7",      { 9U,  5U  } },

    { "FM",         { 9U,  4U  } },         /* 217 */
    { "IR",         { 9U,  3U  } },
    { "SPIF",       { 9U,  2U  } },
    { "GYROIF",     { 9U,  1U  } },

    { "SCU_SRC0",   { 10U, 31U } },         /* 221 */
    { "SCU_SRC1",   { 10U, 30U } },
    { "SCU_SRC2",   { 10U, 29U } },
    { "SCU_SRC3",   { 10U, 28U } },
    { "SCU_SRC4",   { 10U, 27U } },
    { "SCU_SRC5",   { 10U, 26U } },
    { "SCU_SRC6",   { 10U, 25U } },
    { "SCU_SRC7",   { 10U, 24U } },
    { "SCU_SRC8",   { 10U, 23U } },
    { "SCU_SRC9",   { 10U, 22U } },
    { "SCU_MIX0",   { 10U, 21U } },
    { "SCU_MIX1",   { 10U, 20U } },
    { "SCU_DVC0",   { 10U, 19U } },
    { "SCU_DVC1",   { 10U, 18U } },
    { "SCU_ALL",    { 10U, 17U } },

    { "SSI0",       { 10U, 15U } },         /* 236 */
    { "SSI1",       { 10U, 14U } },
    { "SSI2",       { 10U, 13U } },
    { "SSI3",       { 10U, 12U } },
    { "SSI4",       { 10U, 11U } },
    { "SSI5",       { 10U, 10U } },
    { "SSI6",       { 10U, 9U  } },
    { "SSI7",       { 10U, 8U  } },
    { "SSI8",       { 10U, 7U  } },
    { "SSI9",       { 10U, 6U  } },
    { "SSI_ALL",    { 10U, 5U  } },         /* 246 */
    };
#define RZG2_MSSR_TOTAL_CLOCKS    (NELEMENTS (rzg2MssrClocks))

/* external mode pins from RST module */

LOCAL UINT32 rstModePins;

/* PLL1 Initial Multiplication Ratio indexed by mode pins MD14:MD13. */

LOCAL UINT32 pll1MultRatio [] = { 192U, 160U, 128U, 192U };

/* PLL3 Initial Multiplication Ratio indexed by mode pins MD14:MD13:MD19:MD17 */

LOCAL UINT32 pll3MultRatio [] = { 192U, 128U, 0U, 192U, 160U, 106U, 0U, 160U,
                                  128U,  84U, 0U, 128U, 192U, 128U, 0U, 192U };

/* EXTAL divider division ratio indexed by mode pins MD14:MD13. */
LOCAL UINT32 extalRatio [] = { 16U, 19U, 24U, 32U };

/*
 * Frequency Control Register B (FRQCRB) division ratios
 * The index into this table, to select the divider, comes from the
 * CPG_CLK_DESC variable.index entry in rzg2CpgClocks.
 */

LOCAL UINT32 frqcrbRatios [3][16] =
    {
    /* ZT */
    { 0U, 0U, 4U, 6U, 8U, 12U, 16U, 18U, 24U, 0U, 0U, 0U, 5U, 0U, 0U, 0U },
    /* ZTR */
    { 0U, 0U, 4U, 6U, 8U, 12U, 16U, 18U, 24U, 0U, 0U, 0U, 5U, 0U, 0U, 0U },
    /* ZTRD2 */
    { 0U, 0U, 0U, 0U, 0U, 12U, 16U, 18U, 24U, 0U, 0U, 0U, 0U, 0U, 0U, 0U },
    };

/* SD-IFn registers */

LOCAL UINT32 sdifRegisters [] = { SD0CKCR, SD1CKCR, SD2CKCR, SD3CKCR };

/* MSTPSR registers */

LOCAL UINT32 mstpsrOffset [] =
    {
    MSTPSR0,
    MSTPSR1,
    MSTPSR2,
    MSTPSR3,
    MSTPSR4,
    MSTPSR5,
    MSTPSR6,
    MSTPSR7,
    MSTPSR8,
    MSTPSR9,
    MSTPSR10,
    MSTPSR11
    };

/* SMSTPCR registers */

LOCAL UINT32 mstpcrOffset [] =
    {
    SMSTPCR0,
    SMSTPCR1,
    SMSTPCR2,
    SMSTPCR3,
    SMSTPCR4,
    SMSTPCR5,
    SMSTPCR6,
    SMSTPCR7,
    SMSTPCR8,
    SMSTPCR9,
    SMSTPCR10,
    SMSTPCR11
    };

/* globals */

VXB_DRV vxbFdtRsRzg2CpgMssrDrv =
    {
    { NULL } ,
    "cpg-mssr",                                 /* Name */
    "Renesas RZG2 CPG MSSR",                    /* Description */
    VXB_BUSID_FDT,                              /* Class */
    0U,                                         /* Flags */
    0,                                          /* Reference count */
    vxbFdtRzg2CpgMssrMethodList,                    /* Method table */
    };

VXB_DRV_DEF (vxbFdtRsRzg2CpgMssrDrv)

/*******************************************************************************
*
* rzg2CpgRead32 - read a 32-bit CPG register
*
* This function reads a 32 bit CPG register at <offset> and returns the value.
*
* RETURNS: the value read from the register.
*
* ERRNO: N/A.
*/

LOCAL UINT32 rzg2CpgRead32
    (
    VXB_FDT_CPG_INSTANCE *  pCpg,
    UINT32                  offset
    )
    {
    return vxbRead32 (pCpg->handle, (UINT32 *)(pCpg->regBase + offset));
    }

/*******************************************************************************
*
* rzg2CpgSetBit32 - set individual bits in a 32-bit CPG register
*
* This function sets just the individual <bits> in a 32-bit CPG register at
* <offset>.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2CpgSetBit32
    (
    VXB_FDT_CPG_INSTANCE *  pCpg,
    UINT32                  offset,
    UINT32                  bits
    )
    {
    UINT32 val;

    (void)semTake (pCpg->semMutex, WAIT_FOREVER);

    val = vxbRead32 (pCpg->handle, (UINT32 *)(pCpg->regBase + offset));
    val |= bits;

    /*
     * If write protection is enabled, a target register can only be written
     * after writing the inverted data value to CPGWPR.
     */

    if ((rzg2CpgRead32 (pCpg, CPGWPCR) & CPGWPCR_WPE) == CPGWPCR_WPE)
        {
        vxbWrite32 (pCpg->handle, (UINT32 *)(pCpg->regBase + CPGWPR), ~val);
        }

    vxbWrite32 (pCpg->handle, (UINT32 *)(pCpg->regBase + offset), val);

    (void)semGive (pCpg->semMutex);
    }

/*******************************************************************************
*
* rzg2CpgClrBit32 - clear individual bits in a 32-bit CPG register
*
* This function clears just the individual <bits> in a 32-bit CPG register at
* <offset>.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2CpgClrBit32
    (
    VXB_FDT_CPG_INSTANCE *  pCpg,
    UINT32                  offset,
    UINT32                  bits
    )
    {
    UINT32 val;

    (void)semTake (pCpg->semMutex, WAIT_FOREVER);

    val = vxbRead32 (pCpg->handle, (UINT32 *)(pCpg->regBase + offset));
    val &= ~bits;

    /*
     * If write protection is enabled, a target register can only be written
     * after writing the inverted data value to CPGWPR.
     */

    if ((rzg2CpgRead32 (pCpg, CPGWPCR) & CPGWPCR_WPE) == CPGWPCR_WPE)
        {
        vxbWrite32 (pCpg->handle, (UINT32 *)(pCpg->regBase + CPGWPR), ~val);
        }

    vxbWrite32 (pCpg->handle, (UINT32 *)(pCpg->regBase + offset), val);

    (void)semGive (pCpg->semMutex);
    }

/*******************************************************************************
*
* rzg2RstModePinsRead - read the status of RST mode pins
*
* This function reads the contents of the Reset (RST) module's Mode Monitor
* Register. The contents are copied to <pModePins>.
*
* The RST register space address is retrieved via the device tree in order to
* read the mode pin status. The register space is mapped and unmapped here. Due
* to the overhead, it is intended that this is only called once at
* initialisation.
*
* RETURNS: OK if read successful, ERROR otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2RstModePinsRead
    (
    UINT32 * pModePins
    )
    {
    int                 offset;     /* FDT node offset */
    PHYS_ADDR           regBarAddr; /* RST register address */
    size_t              regBarSize; /* RST register size, in bytes */
    VXB_RESOURCE        memRes;     /* RST resource */
    VXB_RESOURCE_ADR    resAdr;     /* RST mapped address */

    /* find the RST node in the device tree */

    offset = vxFdtNodeOffsetByCompatible (-1, FDT_RST_COMPATIBLE);
    if (offset <= 0)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "No RST device\n");
        return ERROR;
        }

    if (vxFdtDefRegGet (offset, 0U, &regBarAddr, &regBarSize) == ERROR)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "No RST registers\n");
        return ERROR;
        }

    /* create the memory resource to map */

    resAdr.start = regBarAddr;
    resAdr.size = regBarSize;

    memRes.id = VXB_RES_ID_CREATE (VXB_RES_MEMORY, 0U);
    memRes.pRes = &resAdr;

    if (vxbRegMap (&memRes) == ERROR)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "RST mapping failed\n");
        return ERROR;
        }

    /* read the RST Mode Monitor Register */

    *pModePins = vxbRead32 (resAdr.pHandle, (UINT32 *)(resAdr.virtAddr + MODEMR));

    RZG2_DBG_MSG (CPG_DBG_INFO, "RST MODEMR: %08x\n", *pModePins);

    if (vxbRegUnmap (&memRes) == ERROR)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "RST unmapping failed\n");
        return ERROR;
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2CpgDividerStatusRegMaskGet - get the register and mask for CPG divider status
*
* This function provides the appropriate register and bit mask for the requested
* CPG divider status bits. For valid dividers that don't have status bits, zero
* is returned in both values so appropriate action can be taken by the calling
* function.
*
* RETURNS: OK if valid divider, ERROR otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2CpgDividerStatusRegMaskGet
    (
    CPG_CLK_DATA *      pCpgClk,        /* CPG divider clock */
    UINT32 *            pCpgReg,        /* pointer to return register */
    UINT32 *            pBitMask        /* pointer to return bit mask */
    )
    {

    /* sanity check pointer parameters */

    if ((pCpgClk == NULL) || (pCpgReg == NULL) || (pBitMask == NULL))
        {
        return ERROR;
        }

    switch (pCpgClk->variable.type)
        {
        case DIV_SDH:
            *pCpgReg  = sdifRegisters [pCpgClk->variable.index];
            *pBitMask = SDnCKCR_STPnHCK;
            break;

        case DIV_SD:
            *pCpgReg  = sdifRegisters [pCpgClk->variable.index];
            *pBitMask = SDnCKCR_STPnCK;
            break;

        case DIV_RPCSRC:
            *pCpgReg = RPCCKCR;
            if (pCpgClk->variable.index == 0U)
                {
                *pBitMask = RPCCKCR_CKSTP;
                }
            else
                {
                *pBitMask = RPCCKCR_CKSTP2;
                }
            break;

        case DIV_HDMIIF:
            *pCpgReg  = HDMICKCR;
            *pBitMask = CKCR_CKSTP;
            break;

        case DIV_CSI0:
            *pCpgReg  = CSI0CKCR;
            *pBitMask = CKCR_CKSTP;
            break;

        case DIV_CSIREF:
            *pCpgReg  = CSIREFCKCR;
            *pBitMask = CKCR_CKSTP;
            break;

        case DIV_MSIOF:
            *pCpgReg = MSOCKCR;
            *pBitMask = CKCR_CKSTP;
           break;

        case DIV_CANFD:
            *pCpgReg = CANFDCKCR;
            *pBitMask = CKCR_CKSTP;
            break;

        case DIV_SYSCPU:
        case DIV_RTCPU:
        case DIV_COMMON:
        case DIV_LB:
        case DIV_EXTAL:
            *pCpgReg = 0U;
            *pBitMask = 0U;
            break;

        default:
            RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected CPG divider type\n");
            return ERROR;
            break;
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2CpgMssrClkStatusSet - enable or disable CPG or MSSR clock
*
* This function enables or disables the specified clock according to the <enable>
* parameter.
*
* RETURNS: OK if clock status set successfully, ERROR otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2CpgMssrClkStatusSet
    (
    struct vxb_clk *    pClk,           /* VxBus clock */
    BOOL                enableClock     /* Enable flag */
    )
    {
    VXB_FDT_CPG_INSTANCE *  pCpg;       /* CPG driver data */
    MSSR_CLK_DATA *         pMssrClk;   /* MSSR clock data */
    CPG_CLK_DATA *          pCpgClk;    /* CPG clock data */
    UINT32                  cpgReg = 0U;/* CPG/MSSR register offset */
    UINT32                  bitMask;    /* Register bit mask */
    STATUS                  retStatus = ERROR;

    if (pClk == NULL)
        {
        return ERROR;
        }

    pCpg = (VXB_FDT_CPG_INSTANCE *) vxbDevSoftcGet (pClk->pDev);

    switch (pClk->clkType)
        {
        case VXB_CLK_FIX_RATE:
            /*
             * Fixed rate root clocks are always on in CPG, so success depends
             * only on vxbCLkLib handling.
             */

            retStatus = OK;
            break;

        case VXB_CLK_FIX_FACTOR:
            /*
             * Fixed factor dividers are always on in CPG, so success depends
             * only on vxbCLkLib handling.
             */

            retStatus = OK;
            break;

        case VXB_CLK_PLL:
            pCpgClk = (CPG_CLK_DATA *)pClk->clkContext;

            /*
             * No software PLL1 control. This will only fail when trying to
             * enable a PLL that can't be enabled. Otherwise, success depends
             * on vxbClkLib reference counting.
             */

            if (pCpgClk->variable.index == 1U)
                {
                bitMask = rzg2CpgRead32 (pCpg, PLLECR) &
                          PLLECR_PLLnST (pCpgClk->variable.index);
                if ((enableClock && (bitMask != 0U)) || !enableClock)
                    {
                    retStatus  = OK;
                    }
                }
            else
                {
                bitMask = PLLECR_PLLnE (pCpgClk->variable.index);

                /* PLLECR enable bits are 1 to enable */

                if (enableClock)
                    {
                    rzg2CpgSetBit32 (pCpg, PLLECR, bitMask);
                    }
                else
                    {
                    rzg2CpgClrBit32 (pCpg, PLLECR, bitMask);
                    }
                retStatus = OK;
                }
            break;

        case VXB_CLK_DIVIDER:
            pCpgClk = (CPG_CLK_DATA *)pClk->clkContext;

            /*
             * A valid divider is either handled by reading the appropriate
             * status register bits, or if there are no status bits, the
             * divider is always on so success depends only on vxbClkLib
             * handling.
             */

            if (rzg2CpgDividerStatusRegMaskGet (pCpgClk, &cpgReg, &bitMask) == OK)
                {
                if (cpgReg != 0U)
                    {

                    /* divider clocks are 0 to enable */

                    if (enableClock)
                        {
                        rzg2CpgClrBit32 (pCpg, cpgReg, bitMask);
                        }
                    else
                        {
                        rzg2CpgSetBit32 (pCpg, cpgReg, bitMask);
                        }
                    }
                retStatus = OK;
                }

            break;

        case VXB_CLK_GATE:
            pMssrClk = (MSSR_CLK_DATA *)pClk->clkContext;
            cpgReg = mstpcrOffset [pMssrClk->moduleReg];
            bitMask = 1U << pMssrClk->regBit;

            /* module clocks are 0 to enable */

            if (enableClock)
                {
                rzg2CpgClrBit32 (pCpg, cpgReg, bitMask);
                }
            else
                {
                rzg2CpgSetBit32 (pCpg, cpgReg, bitMask);
                }

            retStatus = OK;

            break;

        default:
            RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected clock type\n");
            retStatus = ERROR;
            break;
        }

    return retStatus;
    }

/*******************************************************************************
*
* rzg2CpgMssrClkEnable - enable CPG or MSSR clock
*
* This function enables the specified clock.
*
* RETURNS: OK if clock enabled, ERROR otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2CpgMssrClkEnable
    (
    struct vxb_clk * pClk           /* VxBus clock */
    )
    {
    return rzg2CpgMssrClkStatusSet (pClk, TRUE);
    }

/*******************************************************************************
*
* rzg2CpgMssrClkDisable - disable CPG or MSSR clock
*
* This function disables the specified clock.
*
* RETURNS: OK if clock disabled, ERROR otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2CpgMssrClkDisable
    (
    struct vxb_clk * pClk           /* VxBus clock */
    )
    {
    return rzg2CpgMssrClkStatusSet (pClk, FALSE);
    }

/*******************************************************************************
*
* rzg2FdtFixedClockGet - get the fixed clocks from the device tree
*
* This function scans the flat device tree for fixed-clock compatible devices of
* the requested <clockName>. The clock-frequency property value is returned.
*
* RETURNS: clock frequency if valid clock found in device tree.
*          CLOCK_RATE_INVALID otherwise.
*
* ERRNO: N/A.
*/

LOCAL UINT64 rzg2FdtFixedClockGet
    (
    char * clockName
    )
    {
    UINT64    clockRate = CLOCK_RATE_INVALID; /* clock rate to return */
    int       offset = -1;                    /* FDT node offset */
    char    * pName;                          /* FDT node name */
    UINT32  * pProp;                          /* FDT node property value */

    do
        {
        offset = vxFdtNodeOffsetByCompatible (offset, "fixed-clock");
        if (offset > 0)
            {
            pName = (char *) vxFdtGetName (offset, NULL);
            if ((pName != NULL) && (strcmp (pName, clockName) == 0))
                {
                pProp = (UINT32 *)vxFdtPropGet (offset, "clock-frequency", NULL);
                if (pProp != NULL)
                    {
                    clockRate = (UINT64)vxFdt32ToCpu (*pProp);
                    RZG2_DBG_MSG (CPG_DBG_INFO, "%s rate: %lld\n", pName, clockRate );

                    break;
                    }
                }
            }
        }
    while (offset > 0);

    return clockRate;
    }

/*******************************************************************************
*
* rzg2CpgPllCrRatioGet - get CPG PLL multiplication ratio from control register
*
* Get the PLL multiplication ratio from the PLL Control Register. For PLL0,
* PLL2, and PLL4, a control register is available which specifies the
* multiplication ratio of the PLL.
*
* RETURNS: multiplication ratio if valid, 0 otherwise.
*
* ERRNO: N/A.
*/

LOCAL UINT32 rzg2CpgPllCrRatioGet
    (
    VXB_FDT_CPG_INSTANCE *  pCpg,
    UINT32                  pllIndex
    )
    {
    UINT32 pllCr = 0U;  /* PLL Control Register offset */
    UINT32 ratio = 0U;  /* multiplication ratio */
    UINT32 regVal;

    switch (pllIndex)
        {
        case 0U: pllCr = PLL0CR; break;
        case 2U: pllCr = PLL2CR; break;
        case 3U: pllCr = PLL3CR; break;
        case 4U: pllCr = PLL4CR; break;
        default:
            RZG2_DBG_MSG (CPG_DBG_ERR, "No control register for PLL%d\n", pllIndex);
            break;
        }

    if (pllCr != 0U)
        {
        /*
         * STC[6:0] is from PLLnCR[30:24]
         * PLL multiplication ratio is (STC[6:0] + 1) * 2
         */

        regVal = rzg2CpgRead32 (pCpg, pllCr);
        ratio = (PLLnCR_STC (regVal) + 1U) * 2U;
        }

    return ratio;
    }

/*******************************************************************************
*
* rzg2CpgPllMdRatioGet - get CPG PLL multiplication ratio from mode pins
*
* Get the PLL multiplication ratio from the mode pin settings. For PLL1,
* and PLL3, with no control register, the ratio is determined by defined mode
* pins.
*
* RETURNS: multiplication ratio if valid, 0 otherwise.
*
* ERRNO: N/A.
*/

LOCAL UINT32 rzg2CpgPllMdRatioGet
    (
    UINT32 pllIndex
    )
    {
    int     modeIndex;      /* masked mode pin value for PLL */
    UINT32  ratio = 0U;     /* multiplication ratio */

    switch (pllIndex)
        {
        case 1U:
            /* PLL1 multiplier from mode pins MD14:13 */

            modeIndex = (rstModePins & (MODEMR_MD (14) | MODEMR_MD (13))) >> 13;
            ratio = pll1MultRatio [modeIndex];
            break;

        case 3U:
            /* PLL3 multiplier from mode pins MD14:MD13:MD19:MD17 */

            modeIndex = ((rstModePins & (MODEMR_MD (17))) >> 17) |
                        ((rstModePins & (MODEMR_MD (19))) >> 18) |
                        ((rstModePins & (MODEMR_MD (13))) >> 11) |
                        ((rstModePins & (MODEMR_MD (14))) >> 11);
            ratio = pll3MultRatio [modeIndex];
            break;

        default:
            RZG2_DBG_MSG (CPG_DBG_ERR, "No mode pins for PLL%d\n", pllIndex);
            break;
        }

    return ratio;
    }

/*******************************************************************************
*
* rzg2CpgPllRateGet - get CPG PLL clock rate
*
* This function returns the clock frequency for the specified PLL. If the
* specified PLL provides a control register, this is used to calculate the
* multiplication ratio. Where there is no control register, the initial mode
* pin settings are used.
*
* RETURNS: clock frequency if valid, CLOCK_RATE_INVALID otherwise.
*
* ERRNO: N/A.
*/

LOCAL UINT64 rzg2CpgPllRateGet
    (
    VXB_FDT_CPG_INSTANCE *  pCpg,           /* CPG driver data */
    CPG_CLK_DATA *          pCpgClk,        /* CPG PLL clock */
    UINT64                  parentRate      /* frequency of parent clock */
    )
    {
    UINT64 clockRate  = CLOCK_RATE_INVALID; /* returned clock rate */
    UINT32 multiplier = 0U;                 /* PLL multiplier */

    /* validate for PLL0 to PLL4 */

    if ((pCpgClk->variable.type != DIV_PLL) && (pCpgClk->variable.index > 4U))
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "PLL0 to PLL4 expected\n");
        return CLOCK_RATE_INVALID;
        }

    /* Main clock is output if PLL circuit is off */

    if ((rzg2CpgRead32 (pCpg, PLLECR) &
        PLLECR_PLLnST (pCpgClk->variable.index)) == 0U)
        {
        clockRate = parentRate;
        }
    else
        {
        /* PLLs multiply EXTAL/2 if MD14 and MD13 are set */

        if ((rstModePins & (MODEMR_MD (14) | MODEMR_MD (13))) ==
            (MODEMR_MD (14) | MODEMR_MD (13)))
            {
            parentRate /= 2U;
            }

        RZG2_DBG_MSG (CPG_DBG_INFO, "clk variable index %d\n",
                      pCpgClk->variable.index);

        switch (pCpgClk->variable.index)
            {
            case 0U:
            case 2U:
            case 4U:
                /* PLL0, PLL2 and PLL4 use control register */

                multiplier = rzg2CpgPllCrRatioGet (pCpg, pCpgClk->variable.index);
                break;

            case 1U:
            case 3U:
                /* PLL1 and PLL3 use mode pins */

                multiplier = rzg2CpgPllMdRatioGet (pCpgClk->variable.index);
                break;

            default:
                RZG2_DBG_MSG (CPG_DBG_ERR, "PLL%d not supported\n",
                              pCpgClk->variable.index);
                break;
            }

        if (multiplier != 0U)
            {
            clockRate = parentRate * multiplier;
            }
        }

    return clockRate;
    }

/*******************************************************************************
*
* rzg2CpgDividerRateGet - get CPG divider clock rate
*
* This function returns the clock frequency output for the specified divider.
* The variable division ratios are calculated with reference to clock frequency
* control registers and/or mode pins as described in the R-Car Gen3 hardware
* user's manual rev 0.53.
*
* RETURNS: clock frequency if valid, CLOCK_RATE_INVALID otherwise.
*
* ERRNO: N/A.
*/

LOCAL UINT64 rzg2CpgDividerRateGet
    (
    VXB_FDT_CPG_INSTANCE *  pCpg,       /* CPG driver data */
    CPG_CLK_DATA *          pCpgClk,    /* CPG clock */
    UINT64                  parentRate  /* frequency of parent clock */
    )
    {
    UINT64 clockRate = CLOCK_RATE_INVALID;
    UINT64 mult = 0U;
    UINT64 div = 0U;
    UINT32 bits;
    UINT32 cpgReg;

    switch (pCpgClk->variable.type)
        {
        case DIV_SYSCPU:
            div = 32U;
            switch (pCpgClk->variable.index)
                {
                case 1U:
                    mult = 32U - FRQCRC_ZFC (rzg2CpgRead32 (pCpg, FRQCRC));
                    break;

                case 2U:
                    mult = 32U - FRQCRC_Z2FC (rzg2CpgRead32 (pCpg, FRQCRC));
                    break;

                default:
                    RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected SYS-CPU divider\n");
                    break;
                }
            break;

        case DIV_RTCPU:
            div = 32U;
            mult = 32U - FRQCRC_ZRFC (rzg2CpgRead32 (pCpg, FRQCRC));
            break;

        case DIV_COMMON:
            mult = 1U;
            bits = rzg2CpgRead32 (pCpg, FRQCRB);
            switch (pCpgClk->variable.index)
                {
                case 0U:
                    div = frqcrbRatios[0][FRQCRB_ZTFC (bits)];
                    break;

                case 1U:
                    div = frqcrbRatios[1][FRQCRB_ZTRFC (bits)];
                    break;

                case 2U:
                    /*
                     * The clock name ZTRD2 suggests it is ZTR divided by 2,
                     * but the register allows other divisors.
                     */

                    div = frqcrbRatios[2][FRQCRB_ZTRD2FC (bits)];
                    break;

                default:
                    RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected common divider\n");
                    break;
                }
            break;

        case DIV_EXTAL:
            mult = 1U;
            bits = (rstModePins & (MODEMR_MD (14) | MODEMR_MD (13))) >> 13;
            div = extalRatio[bits];

            /* EXTAL divider outputs to further fixed divider */

            div *= 8U;
            break;

        case DIV_SDH:
            mult = 1U;
            if (pCpgClk->variable.index < NELEMENTS (sdifRegisters))
                {
                cpgReg = sdifRegisters [pCpgClk->variable.index];
                bits = SDnCKCR_SDnSRCFC (rzg2CpgRead32 (pCpg, cpgReg));
                if (bits <= SDnCKCR_SDnSRCFC_VALID_MAX)
                    {
                    div = 1U << bits;
                    }
                }
            break;

        case DIV_SD:
            mult = 1U;
            if (pCpgClk->variable.index < NELEMENTS (sdifRegisters))
                {
                cpgReg = sdifRegisters [pCpgClk->variable.index];
                bits = SDnCKCR_SDnFC (rzg2CpgRead32 (pCpg, cpgReg));
                if (bits <= SDnCKCR_SDnFC_VALID_MAX)
                    {
                    div = 2U << bits;
                    }
                }
            break;

        case DIV_LB:
            /* LB fed by S3, divider is /4 or /6 by mode pin MD18 */

            mult = 1U;
            if ((rstModePins & MODEMR_MD (18)) == 0U)
                {
                div = 4U;
                }
            else
                {
                div = 6U;
                }
            break;

        case DIV_RPCSRC:
            /* valid RPCSRC DIV field either 2 (for /5) or 3 (for /6) */

            mult = 1U;
            bits = RPCCKCR_RPC_DIV (rzg2CpgRead32 (pCpg, RPCCKCR));
            if (bits >= RCPCKCR_RPC_DIV_VALID_MIN)
                {
                div = bits + 3U;

                /* RPC divides further, but bit 0 clear is prohibited */

                bits = RPCCKCR_RPCD2_DIV (rzg2CpgRead32 (pCpg, RPCCKCR));
                if ((bits & 1U) != 0U)
                    {
                    div *= (bits + 1U);
                    }

                if (pCpgClk->variable.index != 0U)
                    {
                    div *= 2U;
                    }
                }
            break;

        case DIV_HDMIIF:
            mult = 1U;
            div = CKCR_DIV (rzg2CpgRead32 (pCpg, HDMICKCR)) + 1U;
            break;

        case DIV_CSI0:
            mult = 1U;
            div = CKCR_DIV (rzg2CpgRead32 (pCpg, CSI0CKCR)) + 1U;
            break;

        case DIV_CSIREF:
            mult = 1U;
            div = CKCR_DIV (rzg2CpgRead32 (pCpg, CSIREFCKCR)) + 1U;
            break;

        case DIV_MSIOF:
            mult = 1U;
            div = CKCR_DIV (rzg2CpgRead32 (pCpg, MSOCKCR)) + 1U;
            break;

        case DIV_CANFD:
            mult = 1U;
            div = CKCR_DIV (rzg2CpgRead32 (pCpg, CANFDCKCR)) + 1U;
            break;

        default:
            RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected CPG divider type\n");
            break;
        }

    if ((mult != 0U ) && (div != 0U))
        {
        clockRate = parentRate * mult / div;
        }

    return clockRate;
    }

/*******************************************************************************
*
* rzg2CpgMssrClkRateGet - get CPG clock rate
*
* This routine get the clock frequency for the specified CPG clock.
*
* RETURNS: clock frequency if valid, CLOCK_RATE_INVALID otherwise.
*
* ERRNO: N/A.
*/

LOCAL UINT64 rzg2CpgMssrClkRateGet
    (
    struct vxb_clk *    pClk,       /* VxBus clock */
    UINT64              parentRate  /* frequency of parent clock */
    )
    {
    UINT64                  clockRate = CLOCK_RATE_INVALID;
    VXB_FDT_CPG_INSTANCE *  pCpg;       /* CPG driver data */
    CPG_CLK_DATA *          pCpgClk;    /* CPG clock data */

    if (pClk == NULL)
        {
        return CLOCK_RATE_INVALID;
        }

    /* MSSR clocks don't return a clock frequency */

    if (pClk->clkType == VXB_CLK_GATE)
        {
        return CLOCK_RATE_INVALID;
        }

    /* CPG clocks should return a clock frequency */

    pCpgClk = (CPG_CLK_DATA *) pClk->clkContext;
    pCpg = (VXB_FDT_CPG_INSTANCE *) vxbDevSoftcGet (pClk->pDev);
    if (pCpgClk == NULL)
        {
        return CLOCK_RATE_INVALID;
        }

    switch (pClk->clkType)
        {
        case VXB_CLK_FIX_RATE:
            clockRate = pCpgClk->rate;
            break;

        case VXB_CLK_FIX_FACTOR:
            if (pCpgClk->factor.div != 0U)
                {
                clockRate = parentRate * pCpgClk->factor.mult /
                            pCpgClk->factor.div;
                }
            break;

        case VXB_CLK_PLL:
            clockRate = rzg2CpgPllRateGet (pCpg, pCpgClk, parentRate);
            break;

        case VXB_CLK_DIVIDER:
            clockRate = rzg2CpgDividerRateGet (pCpg, pCpgClk, parentRate);
            break;

        default:
            RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected clock type\n");
            break;
        }

    RZG2_DBG_MSG (CPG_DBG_INFO, "clk type %d, parentRate %lld, clock rate %lld\n",
                  pClk->clkType, parentRate, clockRate);

    return clockRate;
    }

/*******************************************************************************
*
* rzg2CpgMssrClkStatusGet - get CPG/MMSR clock status
*
* This routine get the clock status for the specified CPG/MSSR clock.
*
* RETURNS: CLOCK_STATUS_ENABLED if the clock is on, CLOCK_STATUS_GATED if the
* clock is off, and CLOCK_STATUS_UNKNOWN for indeterminate status, or error.
*
* ERRNO: N/A.
*/

LOCAL UINT32 rzg2CpgMssrClkStatusGet
    (
    struct vxb_clk * pClk
    )
    {
    VXB_FDT_CPG_INSTANCE *  pCpg;           /* CPG driver data */
    MSSR_CLK_DATA *         pMssrClk;       /* MSSR clock data */
    CPG_CLK_DATA *          pCpgClk;        /* CPG clock data */
    UINT32                  cpgReg = 0U;    /* CPG/MSSR register offset */
    UINT32                  bitMask;        /* Register bit mask */
    UINT32                  clockStatus = CLOCK_STATUS_UNKNOWN;

    if (pClk == NULL)
        {
        return CLOCK_STATUS_UNKNOWN;
        }

    pCpg = (VXB_FDT_CPG_INSTANCE *) vxbDevSoftcGet (pClk->pDev);

    switch (pClk->clkType)
        {
        case VXB_CLK_FIX_RATE:
            /* fixed rate root clocks always on in CPG */

            clockStatus = CLOCK_STATUS_ENABLED;
            break;

        case VXB_CLK_FIX_FACTOR:
            /* depends on parent, from stored VXB_CLK setting */

            clockStatus = pClk->parentClock->clkStatus;
            break;

        case VXB_CLK_PLL:
            pCpgClk = (CPG_CLK_DATA *)pClk->clkContext;
            bitMask = rzg2CpgRead32 (pCpg, PLLECR) &
                      PLLECR_PLLnST (pCpgClk->variable.index);
            if (bitMask == 0U)
                {
                clockStatus = CLOCK_STATUS_GATED;
                }
            else
                {
                clockStatus = CLOCK_STATUS_ENABLED;
                }
            break;

        case VXB_CLK_DIVIDER:
            pCpgClk = (CPG_CLK_DATA *)pClk->clkContext;

            if (rzg2CpgDividerStatusRegMaskGet (pCpgClk, &cpgReg, &bitMask) == OK)
                {
                if (cpgReg != 0U)
                    {
                    if ((rzg2CpgRead32 (pCpg, cpgReg) & bitMask) == 0U)
                        {
                        clockStatus = CLOCK_STATUS_ENABLED;
                        }
                    else
                        {
                        clockStatus = CLOCK_STATUS_GATED;
                        }
                    }
                else
                    {
                    /* depends on parent, from stored VXB_CLK setting */

                    clockStatus = pClk->parentClock->clkStatus;
                    }
                }
            break;

        case VXB_CLK_GATE:
            /* MSSR module clock */

            pMssrClk = (MSSR_CLK_DATA *)pClk->clkContext;
            cpgReg = mstpsrOffset [pMssrClk->moduleReg];
            bitMask = 1U << pMssrClk->regBit;
            if ((rzg2CpgRead32 (pCpg, cpgReg) & bitMask) == 0U)
                {
                clockStatus = CLOCK_STATUS_ENABLED;
                }
            else
                {
                clockStatus = CLOCK_STATUS_GATED;
                }
            break;

        default:
            RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected clock type\n");
            break;
        }

    return clockStatus;
    }

/*******************************************************************************
*
* rzg2CpgMssrClkInit - CPG/MMSR clock initialisation
*
* This routine initialise the clock status for the specified CPG/MSSR clock.
*
* This function is required to set an initial reference count for all software
* controllable clocks whose initial hardware status is enabled.
*
* RETURNS: OK if clock initialised, ERROR otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2CpgMssrClkInit
    (
    struct vxb_clk * pClk
    )
    {
    VXB_FDT_CPG_INSTANCE *  pCpg;       /* CPG driver data */
    CPG_CLK_DATA *          pCpgClk;    /* CPG clock data */
    STATUS                  retStatus = OK;

    if (pClk == NULL)
        {
        return ERROR;
        }

    pCpg = (VXB_FDT_CPG_INSTANCE *) vxbDevSoftcGet (pClk->pDev);
    pCpgClk = (CPG_CLK_DATA *)pClk->clkContext;

    if (vxbClkStatusGet (pClk) == CLOCK_STATUS_ENABLED)
        {
        switch (pClk->clkType)
            {
            case VXB_CLK_PLL:

                /* no software control for PLL1 */

                if (pCpgClk->variable.index != 1U)
                    {
                    pClk->clkRefs = 1U;
                    }
                break;

            case VXB_CLK_DIVIDER:
                switch (pCpgClk->variable.type)
                    {
                    case DIV_SDH:
                    case DIV_SD:
                    case DIV_RPCSRC:
                    case DIV_HDMIIF:
                    case DIV_CSI0:
                    case DIV_CSIREF:
                    case DIV_MSIOF:
                    case DIV_CANFD:

                        /* controllable dividers */

                        pClk->clkRefs = 1U;
                        break;

                    case DIV_SYSCPU:
                    case DIV_RTCPU:
                    case DIV_COMMON:
                    case DIV_LB:
                    case DIV_EXTAL:

                        /* always on dividers */

                        break;

                    default:
                        RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected CPG divider type\n");
                        retStatus = ERROR;
                        break;
                    }
                break;

            case VXB_CLK_GATE:

                /* all module clocks */

                pClk->clkRefs = 1U;
                break;

            case VXB_CLK_FIX_RATE:
            case VXB_CLK_FIX_FACTOR:

                /* no software control for these fixed clocks */

                break;

            default:
                RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected clock type\n");
                retStatus = ERROR;
                break;

            }
        }

    return retStatus;
    }

/******************************************************************************
*
* rzg2CpgClocksCreate - create CPG clocks list
*
* This function fills the previously allocated space pointed to by <pClkList>
* with a NULL terminated array of clocks required for static clock
* initialisation. The <numClks> parameter specifies the number of clocks
* to initialise, and the resource pointed to by <pClkList> must be sufficient
* for this number of clocks and an additional NULL entry.
*
* Additional space for VxBus clocks is allocated by this function and it is the
* responsibility of the calling function to free this resource.
*
* The main structure of the clock tree is predefined by the <rzg2CpgClocks> table.
*
* A static clock list is used (rather than dynamic clock registration) because
* this allows all the clocks to be associated with a single clock controller in
* the device tree.
*
* RETURNS: OK, or ERROR if initialisation failed.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2CpgClocksCreate
    (
    VXB_DEV_ID          pDev,       /* our device */
    VXB_CLK_DOMAIN *    pDomain,    /* domain for these clocks */
    VXB_CLK_ID  *       pClkList,   /* array of static clocks created */
    UINT                numClks     /* number of clocks to initialise */
    )
    {
    CPG_CLK_DESC *      pClkDesc;       /* instance of our clock descriptor */
    size_t              i;              /* index to clocks */
    VXB_CLK_ID          pVxbClk;        /* instance of VxBus clock */
    CPG_CLK_DATA *      pCpgClkData;    /* CPG clock specific context */

    /* sanity check parameters */

    if ((pDev == NULL) || (pDomain == NULL) || (pClkList == NULL))
        {
        return ERROR;
        }

    /* NULL terminate empty list first, in case of error */

    pClkList[0] = NULL;

    /* validate numClks */

    if (numClks != RZG2_CPG_TOTAL_CLOCKS)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected number of CPG clocks\n");
        return ERROR;
        }

    for (i = 0; i < numClks; i++)
        {
        /* allocate space for clock and add to array */

        pVxbClk = (VXB_CLK_ID) vxbMemAlloc(sizeof (VXB_CLK));
        if (pVxbClk == NULL)
            {
            RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
            return ERROR;
            }

        pClkList[i]   = pVxbClk;
        pClkList[i+1] = NULL;

        /* allocate clock specific context area and attach to clock */

        pVxbClk->clkContext = vxbMemAlloc (sizeof (CPG_CLK_DATA));
        if (pVxbClk->clkContext == NULL)
            {
            RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
            return ERROR;
            }
        pCpgClkData = (CPG_CLK_DATA *) pVxbClk->clkContext;

        /* reference our table of clock configuration */

        pClkDesc = (CPG_CLK_DESC *) &rzg2CpgClocks[i];

        /* copy common elements */

        pVxbClk->pDev = pDev;
        pVxbClk->clkDomain = pDomain;
        pVxbClk->clkFuncs = &rzg2CpgMssrMethods;

        pVxbClk->clkName = vxbMemAlloc (strlen(pClkDesc->name) + 1);
        if (pVxbClk->clkName == NULL)
            {
            RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
            return ERROR;
            }
        strncpy (pVxbClk->clkName, pClkDesc->name, strlen(pClkDesc->name) + 1);

        pVxbClk->clkType = pClkDesc->type;
        pCpgClkData->rate = CLOCK_RATE_INVALID;

        /* root clock has no parent */

        if (pClkDesc->parentIndex < 0)
            {
            pVxbClk->parentClock = NULL;
            }
        else
            {
            pVxbClk->parentClock = pClkList[pClkDesc->parentIndex];
            }

        /* initialise context according to clock type */

        switch (pVxbClk->clkType)
            {
            case VXB_CLK_FIX_RATE:
                pCpgClkData->rate = rzg2FdtFixedClockGet (pClkDesc->name);
                pVxbClk->clkRate  = pCpgClkData->rate;
                break;

            case VXB_CLK_FIX_FACTOR:
                pCpgClkData->factor.mult = pClkDesc->mult;
                pCpgClkData->factor.div  = pClkDesc->div;
                break;

            case VXB_CLK_PLL:
            case VXB_CLK_DIVIDER:
                pCpgClkData->variable.index = pClkDesc->index;
                pCpgClkData->variable.type  = pClkDesc->divider;
                break;

            default:
                RZG2_DBG_MSG (CPG_DBG_ERR, "clock type error\n");
                return ERROR;
                break;
            }

        /*
         * A static clock driver should use DEFINE_VXB_CLOCK to initialise the
         * static clocks. We're dynamically creating a list of static clocks so
         * need to initialise the clocks in the same way.
         */

        DLL_INIT (&pVxbClk->clkNode);
        DLL_INIT (&pVxbClk->rootClkNode);
        pVxbClk->clkStatus = 0U;
        pVxbClk->clkRefs   = 0U;
        pVxbClk->parentName   = NULL;
        pVxbClk->parentNames  = NULL;
        pVxbClk->parentClocks = NULL;
        pVxbClk->parentNum = 1U;
        pVxbClk->parentIdx = 0U;
        DLL_INIT (&pVxbClk->parentNode);
        DLL_INIT (&pVxbClk->childClkList);
        }

    return OK;
    }

/******************************************************************************
*
* rzg2MssrClocksCreate - create MSSR clocks list
*
* This function fills the previously allocated space pointed to by <pClkList>
* with a NULL terminated array of clocks required for static clock
* initialisation. The <numClks> parameter specifies the number of clocks
* to initialise, and the resource pointed to by <pClkList> must be sufficient
* for this number of clocks and an additional NULL entry.
*
* Additional space for VxBus clocks is allocated by this function and it is the
* responsibility of the calling function to free this resource.
*
* The main structure of the clock tree is predefined by the <rzg2MssrClocks> table.
*
* A static clock list is used (rather than dynamic clock registration) because
* this allows all the clocks to be associated with a single clock controller in
* the device tree.
*
* RETURNS: OK, or ERROR if initialisation failed.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2MssrClocksCreate
    (
    VXB_DEV_ID          pDev,       /* our device */
    VXB_CLK_DOMAIN *    pDomain,    /* domain for these clocks */
    VXB_CLK_ID  *       pClkList,   /* array of static clocks created */
    UINT                numClks     /* number of clocks to initialise */
    )
    {
    MSSR_CLK_DESC *     pClkDesc;       /* instance of our clock descriptor */
    size_t              i;              /* index to clocks */
    VXB_CLK_ID          pVxbClk;        /* instance of VxBus clock */
    MSSR_CLK_DATA *     pMssrClkData;   /* MSSR clock specific context */

    /* sanity check parameters */

    if ((pDev == NULL) || (pDomain == NULL) || (pClkList == NULL))
        {
        return ERROR;
        }

    /* NULL terminate empty list first, in case of error */

    pClkList[0] = NULL;

    /* validate numClks */

    if (numClks != RZG2_MSSR_TOTAL_CLOCKS)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "Unexpected number of MSSR clocks\n");
        return ERROR;
        }

    for (i = 0; i < numClks; i++)
        {
        /* allocate space for clock and add to array */

        pVxbClk = (VXB_CLK_ID)vxbMemAlloc(sizeof (VXB_CLK));
        if (pVxbClk == NULL)
            {
            RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
            return ERROR;
            }
        pClkList[i]   = pVxbClk;
        pClkList[i+1] = NULL;

        /* allocate clock specific context area and attach to clock */

        pVxbClk->clkContext = vxbMemAlloc (sizeof (MSSR_CLK_DATA));
        if (pVxbClk->clkContext == NULL)
            {
            RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
            return ERROR;
            }
        pMssrClkData = (MSSR_CLK_DATA *)pVxbClk->clkContext;

        /* reference our table of clock configuration */

        pClkDesc = (MSSR_CLK_DESC *) &rzg2MssrClocks[i];

        /* copy common elements */

        pVxbClk->pDev = pDev;
        pVxbClk->clkDomain = pDomain;
        pVxbClk->clkFuncs = &rzg2CpgMssrMethods;

        pVxbClk->clkName = vxbMemAlloc (strlen(pClkDesc->name) + 1);
        if (pVxbClk->clkName == NULL)
            {
            RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
            return ERROR;
            }
        strncpy (pVxbClk->clkName, pClkDesc->name, strlen(pClkDesc->name) + 1);

        pVxbClk->clkType = VXB_CLK_GATE;
        if (pClkDesc->parentIndex <= 0)
            {
            pVxbClk->parentClock = NULL;
            }
        else
            {
            pVxbClk->parentClock =
                (pClkList - RZG2_CPG_TOTAL_CLOCKS)[pClkDesc->parentIndex];
            }

        /* initialise context */

        pMssrClkData->moduleReg = pClkDesc->moduleBit.moduleReg;
        pMssrClkData->regBit = pClkDesc->moduleBit.regBit;

        /*
         * A static clock driver should use DEFINE_VXB_CLOCK to initialise the
         * static clocks. We're dynamically creating a list of static clocks so
         * need to initialise the clocks in the same way.
         */

        DLL_INIT (&pVxbClk->clkNode);
        DLL_INIT (&pVxbClk->rootClkNode);
        pVxbClk->clkStatus = 0U;
        pVxbClk->clkRefs   = 0U;
        pVxbClk->parentName   = NULL;
        pVxbClk->parentNames  = NULL;
        pVxbClk->parentClocks = NULL;
        pVxbClk->parentNum = 1U;
        pVxbClk->parentIdx = 0U;
        DLL_INIT (&pVxbClk->parentNode);
        DLL_INIT (&pVxbClk->childClkList);
        }

    return OK;
    }

/*******************************************************************************
*
* vxbFdtRzg2CpgMssrProbe - probe for device presence at specific address
*
* This routine check for Renesas RZ/G2 CPG (or compatible) device at the
* specified base address.
*
* RETURNS: OK if probe passes and assumed a valid (or compatible) device,
* ERROR otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS vxbFdtRzg2CpgMssrProbe
    (
    VXB_DEV_ID pDev
    )
    {

    return vxbFdtDevMatch (pDev, vxbFdtRzg2CpgMssrMatch, NULL);
    }

/******************************************************************************
*
* vxbFdtRzg2CpgMssrAttach - attach CPG MSSR device
*
* This is the Renesas RZ/G2 CPG MSSR initialisation function.
*
* RETURNS: OK, or ERROR if initialisation failed.
*
* ERRNO: N/A.
*/

LOCAL STATUS vxbFdtRzg2CpgMssrAttach
    (
    VXB_DEV_ID          pDev
    )
    {
    VXB_FDT_CPG_INSTANCE * pCpg;
    VXB_RESOURCE_ADR *  pResAdr;
    VXB_CLK_DOMAIN *    pCpgDomain;
    VXB_CLK_DOMAIN *    pMssrDomain;
    VXB_CLK_ID          pClk;

    RZG2_DBG_MSG (CPG_DBG_INFO, "Enter %s \n", __FUNCTION__);

    /* check for valid parameter */

    if (pDev == NULL)
        {
        return ERROR;
        }

    /* allocate the memory for the CPG instance */

    pCpg = (VXB_FDT_CPG_INSTANCE *)vxbMemAlloc (sizeof(VXB_FDT_CPG_INSTANCE));
    if (pCpg == NULL)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
        return ERROR;
        }

    vxbDevSoftcSet (pDev, (void *)pCpg);
    pCpg->pDev = pDev;

    /* dynamically allocate and initialise mutual-exclusion semaphore */

    pCpg->semMutex = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE |
                                 SEM_INVERSION_SAFE);
    if (pCpg->semMutex == SEM_ID_NULL)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "semMCreate error\n");
        goto errOut;
        }

    /* allocate register memory resource */

    pCpg->memRes = vxbResourceAlloc (pDev, VXB_RES_MEMORY, 0);
    if (pCpg->memRes == NULL)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "vxbResourceAlloc(MEMORY) error\n");
        goto errOut;
        }

    /* extract register virtual address and resource handle */

    pResAdr = (VXB_RESOURCE_ADR *) pCpg->memRes->pRes;
    if (pResAdr == NULL)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "virtual address error\n");
        goto errOut;
        }
    pCpg->regBase = (VIRT_ADDR) pResAdr->virtual;
    pCpg->handle = pResAdr->pHandle;

    /* configure CPG MSSR */

    if (rzg2RstModePinsRead(&rstModePins) == ERROR)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "CPG rzg2RstModePinsRead error\n");
        goto errOut;
        }

    RZG2_DBG_MSG (CPG_DBG_INFO, "reset mode pin 0x%x\n", rstModePins);

#ifdef RZG2_CPG_DBG
    /* check for CPG write protection */

    if ((rzg2CpgRead32 (pCpg, CPGWPCR) & CPGWPCR_WPE) == CPGWPCR_WPE)
        {
        RZG2_DBG_MSG (CPG_DBG_INFO, "CPG write protection is enabled\n");
        }
#endif /* RZG2_CPG_DBG */

    /* register the CPG clock domain */

    pCpgDomain = vxbClkDomainRegister (&rzg2CpgDomainReg);
    if (pCpgDomain == NULL)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "vxbClkDomainRegister error\n");
        goto errOut;
        }

    /* register the MSSR clock domain */

    pMssrDomain = vxbClkDomainRegister (&rzg2MssrDomainReg);
    if (pMssrDomain == NULL)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "vxbClkDomainRegister error\n");
        goto errOut;
        }

    /*
     * Allocate the memory for the static clock list. This list will start with
     * the CPG clocks, and the MSSR clocks will be contiguous. This completed
     * list needs to be NULL terminated, so requires an additional list entry.
     */

    pCpg->pCpgClkList = vxbMemAlloc (sizeof (VXB_CLK_ID) *
                                     (RZG2_CPG_TOTAL_CLOCKS +
                                      RZG2_MSSR_TOTAL_CLOCKS + 1));
    if (pCpg->pCpgClkList == NULL)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "vxbMemAlloc error\n");
        goto errOut;
        }

    /* create the CPG and MSSR clock domains */

    if (rzg2CpgClocksCreate (pDev, pCpgDomain,
                             (VXB_CLK_ID *) pCpg->pCpgClkList,
                             RZG2_CPG_TOTAL_CLOCKS) == ERROR)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "cpgClksCreate error\n");
        goto errOut;
        }

    if (rzg2MssrClocksCreate (pDev, pMssrDomain,
                              (VXB_CLK_ID *) (pCpg->pCpgClkList) +
                              RZG2_CPG_TOTAL_CLOCKS,
                              RZG2_MSSR_TOTAL_CLOCKS) == ERROR)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "mssrClksCreate error\n");
        goto errOut;
        }

    /* initialise the static clocks from the created list */

    if (vxbClksInit (pDev, (VXB_CLK_ID *)pCpg->pCpgClkList, NULL) == ERROR)
        {
        RZG2_DBG_MSG (CPG_DBG_ERR, "vxbClksInit error\n");
        goto errOut;
        }

    RZG2_DBG_MSG (CPG_DBG_INFO, "vxbFdtRzg2CpgMssrAttach OK\n");

    return OK;

errOut:

    RZG2_DBG_MSG (CPG_DBG_ERR, "vxbFdtRzg2CpgMssrAttach ERROR\n");

    /* free CPG clock resources */

    pClk = (VXB_CLK_ID)pCpg->pCpgClkList;
    while (pClk != NULL)
        {
        (void)vxbMemFree (pClk->clkContext);
        (void)vxbMemFree (pClk->clkName);
        (void)vxbMemFree (pClk);

        /* coverity[use_after_free]: FALSE */

        pClk++;
        }
    (void)vxbMemFree (pCpg->pCpgClkList);

    (void)vxbResourceFree (pDev, pCpg->memRes);
    (void)semDelete (pCpg->semMutex);
    vxbDevSoftcSet (pDev, NULL);
    vxbMemFree (pCpg);

    return ERROR;
    }
