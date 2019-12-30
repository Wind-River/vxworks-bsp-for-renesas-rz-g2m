/* vxbFdtRsRzg2PfcPinMux.c - Renesas PFC pin mux driver for VxBus */

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
17sep19,hli  (VXWPG-394)
*/

/*
DESCRIPTION

This module implements a VxBus compliant pin mux controller driver for the
Renesas RZ/G2M Pin Function Controller.

To add the driver to the vxWorks image, add the DRV_PINMUX_FDT_RZG2_PFC component
to the kernel configuration.

The PFC device should be bound to a device tree node with the following
properties:

\cs
compatible:     Specify the programming model for the device.
                It should be set to "renesas,pfc-r8a774a1" for RZ/G2M board
                and is used by VxBus GEN2 for device driver selection.

reg:            Specify the address and length of the device register block.

pinmux-0:       Specify the default pin mux configuration, if any.

status:         Must be "okay" to enable VxBus driver initialisation.
\ce

Below is an example:

\cs
        pfc: pfc@e6060000
            {
            compatible = "renesas,pfc-r8a774a1";
            reg = <0xe6060000 0x 0x50c>;
            pinmux-0;
            status = "okay";
            };
\ce

To configure pin multiplexing and other pin control, the PFC device tree node
must also contain pin configuration nodes that are referenced by the pin mux
client user. This is described in the Pin Mux Drivers section of the VxWorks 7
BSP and Driver Guide, and vxbPinMuxLib reference documentation.

Node names and labels are not specified by the driver but can be used to
identify meaningful groups of pin configurations. In order to group pin
configurations, this driver supports subnodes within a configuration node.

For example, this pin mux configuration node applies one configuration to one
group of pins:

\cs
    scif1Pins: scif1
        {
        /@ TX1_A, TX1_A, CTS1#, RTS1# @/
        pins = "sipE38", "sipE39", "sipH38", "sipH39";
        function = "scif1";
        bias-disable;
        drive-control = <8>;
        };
\ce

PFC pin mux controller configuration node properties:

\cs
pins:       Required. A string list of pins to which the configuration will
            apply. Pins are named using the package location, eg "socAE33" or
            "sipF38", for HFBGA/SoC and BGA/SiP packages. These are defined in
            the pin function spreadsheet referenced in the RZ/G2M hardware
            manual, and can be found on the board schematic. The "soc" or "sip"
            prefix must be used to identify the package.
\ce

One or more of the following properties should be present to specify the
configuration to apply:

\cs
function:       A single string function to select on the multiplexed pins.
                Currently supported functions are "etherAvb", "mmc0", "scif1",
                and "scif2".

bias-disable:   Disable pull-up/down control on pins.

bias-pull-down: Enable pull-down control on pins.

bias-pull-up:   Enable pull-up control on pins.

drive-control:  An integer value, in eighths, to set the output driving ability
                for pins.

gpio-enable:    Enables GPIO, instead of peripheral function, on pins.
\ce

vxbPinMuxDisable is not supported.

INCLUDE FILES: vxBus.h vxbClkLib.h string.h vxbFdtLib.h

SEE ALSO: vxbPinMuxLib,
\tb VxWorks 7 BSP and Driver Guide,
\tb RZ/G Series, 2nd Generation User’s Manual: Hardware
*/

/* includes */

#include <vxWorks.h>
#include <string.h>
#include <stdio.h>
#include <intLib.h>
#include <semLib.h>
#include <vxFdtLib.h>

#include <hwif/vxBus.h>
#include <hwif/buslib/vxbFdtLib.h>
#include <subsys/pinmux/vxbPinMuxLib.h>

#include <vxbFdtRsRzg2PfcPinMux.h>

/* defines */

/* debug macro */

#undef  RZG2_PFC_DBG
#ifdef  RZG2_PFC_DBG

/* turning local symbols into global symbols */

#ifdef  LOCAL
#undef  LOCAL
#define LOCAL
#endif

#include <private/kwriteLibP.h>    /* _func_kprintf */
#define PFC_DBG_OFF          0x00000000U
#define PFC_DBG_ISR          0x00000001U
#define PFC_DBG_ERR          0x00000002U
#define PFC_DBG_INFO         0x00000004U
#define PFC_DBG_ALL          0xffffffffU

LOCAL UINT32 pfcDbgMask = PFC_DBG_ALL;

#define RZG2_DBG_MSG(mask, ...)                                 \
    do                                                          \
        {                                                       \
        if ((pfcDbgMask & (mask)) || ((mask) == PFC_DBG_ALL))  \
            {                                                   \
            if (_func_kprintf != NULL)                          \
                {                                               \
                (* _func_kprintf)(__VA_ARGS__);                 \
                }                                               \
            }                                                   \
        }                                                       \
    while (FALSE)

#else
#define RZG2_DBG_MSG(...)
#endif  /* RZG2_PFC_DBG */

/* PFC device tree pin mux properties */

#define PFC_FDT_FUNCTION     "function"
#define PFC_FDT_PINS         "pins"

/* PFC power-source value in mv*/

#define PFC_MILLIVOLT_3V3     3300
#define PFC_MILLIVOLT_1V8     1800

/*
 * The longest expected pin, group or function name (AVB_AVTP_CAPTURE_A).
 * This is used as a safety net for strncmp string comparisons.
 */

#define PFC_NAME_LEN_MAX      20

/*
 * The "drive-control" property is specified by a range from 1 to 8 inclusive.
 * The DRV Control (DRVCTRL) Register values for each pin can be 3 bits for H
 * type buffers (1/8 to Full), or 2 bits for F type buffers (1/4 to Full), but
 * the property value is always specified in eighths.
 */

#define PFC_DRVCTRL_VAL_MIN   1
#define PFC_DRVCTRL_VAL_MAX   8

/* pfcPins table terminates with NULL pin name, so used for end check */

#define PFC_PIN_DESC_TBL_END(pPin)     (pPin->sipName == NULL)

/* typedefs */

typedef struct pfcPinmuxDrvCtrl
    {
    VXB_PINMUX_CTRL *   pPinMuxCtrl;    /* pin mux controller representation */
    VXB_RESOURCE *      memRes;         /* mapped memory resource */
    VIRT_ADDR           regBase;        /* mapped PFC register base */
    void *              pHandle;        /* register memory access handle */
    SEM_ID              mutex;          /* register access protection */
    } PFC_PINMUX_DRVCTRL;

/*
 * Supported pin groups, named after a pin group's primary function. This list
 * is incomplete and will be expanded to support additional pin groups, as
 * required by future drivers.
 */

typedef enum pfcGroupId
    {
    GRP_NONE,
    GRP_CANFD0_A,
    GRP_CANFD0_B,
    GRP_SCIF0,
    GRP_SCIF1_A,
    GRP_SCIF1_B,
    GRP_SCIF2_A,
    GRP_SCIF2_B,
    GRP_SCIF3_A,
    GRP_SCIF3_B,
    GRP_SCIF4_A,
    GRP_SCIF4_B,
    GRP_SCIF4_C,
    GRP_SCIF5_A,
    GRP_SCIF5_B,
    GRP_ETHERAVB,
    GRP_HSCIF1_A,
    GRP_HSCIF1_B,
    GRP_SDHI0,
    GRP_SDHI0_DATA4,
    GRP_SDHI0_CTRL,
    GRP_SDHI1,
    GRP_SDHI2,
    GRP_SDHI3,
    GRP_SDHI3_DATA4,
    GRP_SDHI3_CTRL,
    GRP_I2C0,
    GRP_I2C1_A,
    GRP_I2C1_B,
    GRP_I2C2_A,
    GRP_I2C2_B,
    GRP_I2C3,
    GRP_I2C5,
    GRP_I2C6_A,
    GRP_I2C6_B,
    GRP_I2C6_C,
    GRP_USB0,
    GRP_USB1,
    GRP_USB3,
    } PFC_GROUP_ID;

/*
 * Supported functions. This list is incomplete and will be expanded to support
 * additional functions, as required by future drivers.
 */

typedef enum pfcFuncId
    {
    FN_NONE,
    FN_SCIF0,
    FN_SCIF1,
    FN_SCIF2,
    FN_SCIF3,   
    FN_SCIF4,
    FN_SCIF5,
    FN_ETHERAVB,
    FN_SDHI0,
    FN_SDHI1,
    FN_SDHI2,
    FN_SDHI3,
    FN_MMC0,
    FN_MMC1,
    FN_I2C0,
    FN_I2C1,
    FN_I2C2,
    FN_I2C3,
    FN_I2C4,
    FN_I2C5,
    FN_I2C6,
    FN_USB0,
    FN_USB1,
    FN_USB3
    } PFC_FUNC_ID;

typedef struct pfcFunction
    {
    PFC_FUNC_ID  funcId;
    const char * funcName;
    } PFC_FUNCTION;

/* pin pull state used for verbose show method */

typedef enum pfcPinPullState
    {
    PIN_PU_DISABLED,
    PIN_PU_PULL_UP,
    PIN_PU_PULL_DOWN
    } PFC_PIN_PULL_STATE;

/* pin descriptor used to define supported pins in pfcPins table */

typedef struct pfcPinDesc
    {
    const char *    sipName;            /* SiP/BGA pin name */
    UINT            gpsrRegBit;         /* GPRS register and bit position */
    UINT            puRegBit;           /* pull register and bit position */
    UINT            drvMask;            /* DRVCTRL register mask position */
    PFC_GROUP_ID    groupId;            /* module selection group */
    } PFC_PIN_DESC;

/* function descriptor used to define supported functions in pfcFuncSel table */

typedef struct pfcFuncDesc
    {
    PFC_GROUP_ID    groupId;            /* group to which function applies */
    PFC_FUNC_ID     funcId;             /* function to apply */
    UINT            ipsrMask;           /* function selection field */
    UINT8           ipsrVal;            /* function selection value */
    } PFC_FUNC_DESC;

/* module descriptor used to define module selection in pfcModSel table */

typedef struct pfcModDesc
    {
    PFC_GROUP_ID    groupId;            /* group to which function applies */
    PFC_FUNC_ID     funcId;             /* function to apply */
    UINT            modSelMask;         /* module selection field */
    UINT8           modSelVal;          /* module selection value */
    } PFC_MOD_DESC;

typedef struct pfcPocDesc
    {
    PFC_FUNC_ID     funcId;             /* function to apply */
    UINT32          pocMask;            /* POC selection field */
    UINT32          pocVal;             /* POC value */
    } PFC_POC_DESC;

typedef STATUS (PFC_PIN_HANDLER)       /* pin configuration function */
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_PIN_DESC *          pPin,       /* pin descriptor */
    char *                  pProp,      /* configuration property */
    int                     propLen     /* configuration property length */
    );

typedef struct pfcPinConf
    {
    const char *        name;           /* pin name */
    PFC_PIN_HANDLER *   func;           /* pin configuration function */
    } PFC_PIN_CONF;

/* locals */

/* forward declarations for methods */

LOCAL STATUS vxbFdtRzg2PfcPinMuxProbe         (VXB_DEV_ID pDev);
LOCAL STATUS vxbFdtRzg2PfcPinMuxAttach (VXB_DEV_ID pDev);
LOCAL STATUS vxbFdtRzg2PfcPinMuxShutdown (VXB_DEV_ID pDev);
LOCAL STATUS vxbFdtRzg2PfcPinMuxDetach (VXB_DEV_ID pDev);
LOCAL STATUS vxbFdtRzg2PfcPinMuxEnable (VXB_DEV_ID pDev, INT32 offset);
LOCAL void   vxbFdtRzg2PfcPinMuxShow     (VXB_DEV_ID pDev, INT32 verbose);

LOCAL VXB_DRV_METHOD vxbFdtPfcPinMuxMethodList[] =
    {
        { VXB_DEVMETHOD_CALL(vxbDevProbe),      vxbFdtRzg2PfcPinMuxProbe },
        { VXB_DEVMETHOD_CALL(vxbDevAttach),     vxbFdtRzg2PfcPinMuxAttach },
        { VXB_DEVMETHOD_CALL(vxbDevShutdown),   vxbFdtRzg2PfcPinMuxShutdown },
        { VXB_DEVMETHOD_CALL(vxbDevDetach),     vxbFdtRzg2PfcPinMuxDetach },
        { VXB_DEVMETHOD_CALL(vxbPinMuxEnable),  vxbFdtRzg2PfcPinMuxEnable },
        { VXB_DEVMETHOD_CALL(vxbPinMuxShow),    (FUNCPTR)vxbFdtRzg2PfcPinMuxShow },
        VXB_DEVMETHOD_END
    };

/*
 * supported PFC pin configuration definitions
 *
 * The names in this table are the pin names that can be referenced in the
 * device tree when setting pin configurations.
 * This table can be expanded to support additional pins and groups.
 */

LOCAL const PFC_PIN_DESC pfcPins [] =
    {
    /* sipName  gpsrRegBit              puRegBit   drvMask         group */

    /* SCIF0 */

    {"sipF37",  GPSR_REG_PIN (5U, 3U),  PU_CTS0,   DRVCTRL_CTS0,   GRP_SCIF0},

    /* SCIF1 */
    
    {"sipE39",  GPSR_REG_PIN (5U, 5U),  PU_RX1,    DRVCTRL_RX1,    GRP_SCIF1_A},
    {"sipE38",  GPSR_REG_PIN (5U, 6U),  PU_TX1,    DRVCTRL_TX1,    GRP_SCIF1_A},
    {"sipH38",  GPSR_REG_PIN (5U, 7U),  PU_CTS1,   DRVCTRL_CTS1,   GRP_SCIF1_A},
    {"sipH39",  GPSR_REG_PIN (5U, 8U),  PU_RTS1,   DRVCTRL_RTS1,   GRP_SCIF1_A},

    /* SCIF2 */
    
    {"sipL39",  GPSR_REG_PIN (5U, 9U),  PU_SCK2,   DRVCTRL_SCK2,   GRP_SCIF2_A},
    {"sipF38",  GPSR_REG_PIN (5U, 10U), PU_TX2,    DRVCTRL_TX2,    GRP_SCIF2_A},
    {"sipF39",  GPSR_REG_PIN (5U, 11U), PU_RX2,    DRVCTRL_RX2,    GRP_SCIF2_A},

    /* etheravb */

    {"sipA16",  GPSR_NONE,  PU_AVB_RX_CTL,    DRVCTRL_AVB_RX_CTL,    GRP_ETHERAVB},
    {"sipB19",  GPSR_NONE,  PU_AVB_RXC,       DRVCTRL_AVB_RXC,       GRP_ETHERAVB},
    {"sipA13",  GPSR_NONE,  PU_AVB_RD0,       DRVCTRL_AVB_RD0,       GRP_ETHERAVB},
    {"sipB13",  GPSR_NONE,  PU_AVB_RD1,       DRVCTRL_AVB_RD1,       GRP_ETHERAVB},
    {"sipA14",  GPSR_NONE,  PU_AVB_RD2,       DRVCTRL_AVB_RD2,       GRP_ETHERAVB},
    {"sipB14",  GPSR_NONE,  PU_AVB_RD3,       DRVCTRL_AVB_RD3,       GRP_ETHERAVB},
    {"sipA8",   GPSR_NONE,  PU_AVB_TX_CTL,    DRVCTRL_AVB_TX_CTL,    GRP_ETHERAVB},
    {"sipA19",  GPSR_NONE,  PU_AVB_TXC,       DRVCTRL_AVB_TXC,       GRP_ETHERAVB},
    {"sipA18",  GPSR_NONE,  PU_AVB_TD0,       DRVCTRL_AVB_TD0,       GRP_ETHERAVB},
    {"sipB18",  GPSR_NONE,  PU_AVB_TD1,       DRVCTRL_AVB_TD1,       GRP_ETHERAVB},
    {"sipA17",  GPSR_NONE,  PU_AVB_TD2,       DRVCTRL_AVB_TD2,       GRP_ETHERAVB},
    {"sipB17",  GPSR_NONE,  PU_AVB_TD3,       DRVCTRL_AVB_TD3,       GRP_ETHERAVB},
    {"sipA12",  GPSR_NONE,  PU_AVB_TXCREFCLK, DRVCTRL_AVB_TXCREFCLK, GRP_ETHERAVB},
    {"sipA9",   GPSR_NONE,  PU_AVB_MDIO,      DRVCTRL_AVB_MDIO,      GRP_ETHERAVB},
    {"sipA15",  GPSR_REG_PIN (2U, 9U),  PU_AVB_MDC,    DRVCTRL_AVB_MDC,    GRP_ETHERAVB},
    {"sipB15",  GPSR_REG_PIN (2U, 10U), PU_AVB_MAGIC,  DRVCTRL_AVB_MAGIC,  GRP_ETHERAVB},
    {"sipB16",  GPSR_REG_PIN (2U, 11U), PU_AVB_PHY_INT,DRVCTRL_AVB_PHY_INT,GRP_ETHERAVB},
    {"sipB9",   GPSR_REG_PIN (2U, 12U), PU_AVB_LINK,   DRVCTRL_AVB_LINK,   GRP_ETHERAVB},

    /* SD0 */
    
    {"sipT35", GPSR_REG_PIN (3U, 0U),  PU_SD0_CLK,    DRVCTRL_SD0_CLK,  GRP_SDHI0 },
    {"sipU33", GPSR_REG_PIN (3U, 1U),  PU_SD0_CMD,    DRVCTRL_SD0_CMD,  GRP_SDHI0 },
    {"sipT33", GPSR_REG_PIN (3U, 2U),  PU_SD0_DAT0,   DRVCTRL_SD0_DAT0, GRP_SDHI0 },
    {"sipT32", GPSR_REG_PIN (3U, 3U),  PU_SD0_DAT1,   DRVCTRL_SD0_DAT1, GRP_SDHI0 },
    {"sipU35", GPSR_REG_PIN (3U, 4U),  PU_SD0_DAT2,   DRVCTRL_SD0_DAT2, GRP_SDHI0 },
    {"sipU34", GPSR_REG_PIN (3U, 5U),  PU_SD0_DAT3,   DRVCTRL_SD0_DAT3, GRP_SDHI0 },

    /* SD1 */

    {"sipA27",  GPSR_REG_PIN (3U, 8U),  PU_SD1_DAT0,   DRVCTRL_SD1_DAT0, GRP_SDHI1},
    {"sipB27",  GPSR_REG_PIN (3U, 9U),  PU_SD1_DAT1,   DRVCTRL_SD1_DAT1, GRP_SDHI1},
    {"sipC27",  GPSR_REG_PIN (3U, 10U), PU_SD1_DAT2,   DRVCTRL_SD1_DAT2, GRP_SDHI1},
    {"sipD27",  GPSR_REG_PIN (3U, 11U), PU_SD1_DAT3,   DRVCTRL_SD1_DAT3, GRP_SDHI1},

    /* SD2 */

    {"sipA26",  GPSR_REG_PIN (4U, 0U),  PU_SD2_CLK,    DRVCTRL_SD2_CLK,  GRP_SDHI2},
    {"sipA25",  GPSR_REG_PIN (4U, 1U),  PU_SD2_CMD,    DRVCTRL_SD2_CMD,  GRP_SDHI2},
    {"sipD26",  GPSR_REG_PIN (4U, 2U),  PU_SD2_DAT0,   DRVCTRL_SD2_DAT0, GRP_SDHI2},
    {"sipC25",  GPSR_REG_PIN (4U, 3U),  PU_SD2_DAT1,   DRVCTRL_SD2_DAT1, GRP_SDHI2},
    {"sipD24",  GPSR_REG_PIN (4U, 4U),  PU_SD2_DAT2,   DRVCTRL_SD2_DAT2, GRP_SDHI2},
    {"sipD23",  GPSR_REG_PIN (4U, 5U),  PU_SD2_DAT3,   DRVCTRL_SD2_DAT3, GRP_SDHI2},
    {"sipB25",  GPSR_REG_PIN (4U, 6U),  PU_SD2_DS,     DRVCTRL_SD2_DAT3, GRP_SDHI2},

    /* SD3 */

    {"sipM35", GPSR_REG_PIN (4U, 7U),  PU_SD3_CLK,    DRVCTRL_SD3_CLK,  GRP_SDHI3 },
    {"sipM31", GPSR_REG_PIN (4U, 8U),  PU_SD3_CMD,    DRVCTRL_SD3_CMD,  GRP_SDHI3 },
    {"sipL33", GPSR_REG_PIN (4U, 9U),  PU_SD3_DAT0,   DRVCTRL_SD3_DAT0, GRP_SDHI3 },
    {"sipM30", GPSR_REG_PIN (4U, 10U), PU_SD3_DAT1,   DRVCTRL_SD3_DAT1, GRP_SDHI3 },
    {"sipM32", GPSR_REG_PIN (4U, 11U), PU_SD3_DAT2,   DRVCTRL_SD3_DAT2, GRP_SDHI3 },
    {"sipL34", GPSR_REG_PIN (4U, 12U), PU_SD3_DAT3,   DRVCTRL_SD3_DAT3, GRP_SDHI3 },
    {"sipM33", GPSR_REG_PIN (4U, 13U), PU_SD3_DAT4,   DRVCTRL_SD3_DAT4, GRP_SDHI3 },
    {"sipN33", GPSR_REG_PIN (4U, 14U), PU_SD3_DAT5,   DRVCTRL_SD3_DAT5, GRP_SDHI3 },
    {"sipN32", GPSR_REG_PIN (4U, 15U), PU_SD3_DAT6,   DRVCTRL_SD3_DAT6, GRP_SDHI3 },
    {"sipN31", GPSR_REG_PIN (4U, 16U), PU_SD3_DAT7,   DRVCTRL_SD3_DAT7, GRP_SDHI3 },
    {"sipN30", GPSR_REG_PIN (4U, 17U), PU_SD3_DS,     DRVCTRL_SD3_DAT7, GRP_SDHI3 },
    
    /* I2C0 */

    {"sipC29",  GPSR_REG_PIN (3U, 14U), PU_SD1_CD,  DRVCTRL_SD1_CD, GRP_I2C0},
    {"sipB29",  GPSR_REG_PIN (3U, 15U), PU_SD1_WP,  DRVCTRL_SD1_WP, GRP_I2C0},

    /* I2C1_A */

    {"sipAB34", GPSR_REG_PIN (5U, 10U), PU_TX2,     DRVCTRL_TX2,    GRP_I2C1_A},
    {"sipAB33", GPSR_REG_PIN (5U, 11U), PU_RX2,     DRVCTRL_RX2,    GRP_I2C1_A},

    /* I2C1_B */

    {"sipAA39", GPSR_REG_PIN (5U, 23U), PU_MLB_CLK, DRVCTRL_MLB_CLK,GRP_I2C1_B},
    {"sipJ39",  GPSR_REG_PIN (5U, 24U), PU_MLB_SIG, DRVCTRL_MLB_SIG,GRP_I2C1_B},

    /* I2C2_A */

    {"sipG39",  GPSR_REG_PIN (5U, 4U),  PU_RTS0,    DRVCTRL_RTS0,   GRP_I2C2_A},
    {"sipK39",  GPSR_REG_PIN (5U, 0U),  PU_SCK0,    DRVCTRL_SCK0,   GRP_I2C2_A},

    /* I2C2_B */

    {"sipV31",  GPSR_REG_PIN (3U, 12U), PU_SD0_CD,  DRVCTRL_SD0_CD, GRP_I2C2_B},
    {"sipV30",  GPSR_REG_PIN (3U, 13U), PU_SD0_WP,  DRVCTRL_SCK0,   GRP_I2C2_B},

    /* I2C3 */

    {"sipB6",   GPSR_REG_PIN (2U, 7U),  PU_PWM1,    DRVCTRL_PWM1,   GRP_I2C3},
    {"sipA6",   GPSR_REG_PIN (2U, 8U),  PU_PWM2,    DRVCTRL_PWM2,   GRP_I2C3},

    /* I2C5 */

    {"sipB11",  GPSR_REG_PIN (2U, 13U), PU_AVB_MATCH,   DRVCTRL_AVB_MATCH,   GRP_I2C5},
    {"sipA11",  GPSR_REG_PIN (2U, 14U), PU_AVB_CAPTURE, DRVCTRL_AVB_CAPTURE, GRP_I2C5},

    /* I2C6_A */

    {"sipJ1",   GPSR_REG_PIN (1U, 11U), PU_A11,     DRVCTRL_A11,    GRP_I2C6_A},
    {"sipK2",   GPSR_REG_PIN (1U, 8U),  PU_A8,      DRVCTRL_A8,     GRP_I2C6_A},

    /* I2C6_B */

    {"sipB1",   GPSR_REG_PIN (1U, 25U), PU_WE0,     DRVCTRL_WE0,    GRP_I2C6_B},
    {"sipD2",   GPSR_REG_PIN (1U, 26U), PU_WE1,     DRVCTRL_WE1,    GRP_I2C6_B},

    /* I2C6_C */

    {"sipR2",   GPSR_REG_PIN (0U, 14U), PU_D14,     DRVCTRL_D14,    GRP_I2C6_C},
    {"sipR1",   GPSR_REG_PIN (0U, 15U), PU_D15,     DRVCTRL_D15,    GRP_I2C6_C},

    /* USB 3.0 */

    {"sipAJ33", GPSR_REG_PIN (6U, 28U), PU_USB3_PWEN, DRVCTRL_USB3_PWEN,  GRP_USB3 },
    {"sipAH30", GPSR_REG_PIN (6U, 29U), PU_USB3_OVC,  DRVCTRL_USB3_OVC,   GRP_USB3 },

    /* USB2 ch1 */

    {"sipAH34", GPSR_REG_PIN (6U, 26U), PU_USB1_PWEN, DRVCTRL_USB1_PWEN,  GRP_USB1 },
    {"sipAH33", GPSR_REG_PIN (6U, 27U), PU_USB1_OVC,  DRVCTRL_USB1_OVC,   GRP_USB1 },

    /* USB2 ch0 */

    {"sipAH31", GPSR_REG_PIN (6U, 24U), PU_USB0_PWEN, DRVCTRL_USB0_PWEN,  GRP_USB0 },
    {"sipAH32", GPSR_REG_PIN (6U, 25U), PU_USB0_OVC,  DRVCTRL_USB0_OVC,   GRP_USB0 },

    {NULL,      GPSR_NONE,              0U,          0,             GRP_NONE}
    };

/* function names referenced in the device tree, mapped to enumeration */

LOCAL const PFC_FUNCTION pfcFuncs [] =
    {
    { FN_SCIF0,     "scif0" },
    { FN_SCIF1,     "scif1" },
    { FN_SCIF2,     "scif2" },
    { FN_SCIF3,     "scif3" },
    { FN_SCIF4,     "scif4" },
    { FN_SCIF4,     "scif5" },
    { FN_ETHERAVB,  "etheravb" },
    { FN_SDHI0,     "sdhi0" },
    { FN_MMC0,      "mmc0" },
    { FN_MMC1,      "mmc1" },
    { FN_SDHI3,     "sdhi3" },
    { FN_I2C0,      "i2c0" },
    { FN_I2C1,      "i2c1" },
    { FN_I2C2,      "i2c2" },
    { FN_I2C3,      "i2c3" },
    { FN_I2C3,      "i2c4" },
    { FN_I2C5,      "i2c5" },
    { FN_I2C6,      "i2c6" },
    { FN_USB0,      "usb0" },
    { FN_USB1,      "usb1" },
    { FN_USB3,      "usb3" }
    };
#define PFC_NUM_FUNCS   (NELEMENTS (pfcFuncs))

/*
 * supported PFC peripheral function selections
 *
 * This table defines the supported peripheral functions associated with groups
 * of pins, together with the setting required in the appropriate IPSR register.
 * This table can be expanded to support additional function options.
 */

LOCAL const PFC_FUNC_DESC xPfcFuncSel [] =
    {
    /*  group       function     ipsrMask            ipsrVal */

    { GRP_SCIF0,    FN_SCIF0,    IPSR_RX0,           0U }, /* RX0 */
    { GRP_SCIF0,    FN_SCIF0,    IPSR_TX0,           0U }, /* TX0 */

    { GRP_SCIF1_A,  FN_SCIF1,    IPSR_RX1_A,         0U }, /* RX1_A */
    { GRP_SCIF1_A,  FN_SCIF1,    IPSR_TX1_A,         0U }, /* TX1_A */
    { GRP_SCIF1_A,  FN_SCIF1,    IPSR_CTS1,          0U }, /* CTS1# */
    { GRP_SCIF1_A,  FN_SCIF1,    IPSR_RTS1,          0U }, /* RTS1# */
    { GRP_SCIF1_B,  FN_SCIF1,    IPSR_RX1_B,         1U }, /* RX1_B */
    { GRP_SCIF1_B,  FN_SCIF1,    IPSR_TX1_B,         1U }, /* TX1_B */
    { GRP_SCIF1_B,  FN_SCIF1,    IPSR_CTS1,          0U }, /* CTS1# */
    { GRP_SCIF1_B,  FN_SCIF1,    IPSR_RTS1,          0U }, /* RTS1# */

    { GRP_SCIF2_A,  FN_SCIF2,    IPSR_TX2_A,         0U }, /* TX2_A */
    { GRP_SCIF2_A,  FN_SCIF2,    IPSR_RX2_A,         0U }, /* RX2_A */
    { GRP_SCIF2_B,  FN_SCIF2,    IPSR_TX2_B,         1U }, /* TX2_B */
    { GRP_SCIF2_B,  FN_SCIF2,    IPSR_RX2_B,         1U }, /* RX2_B */

    { GRP_SCIF3_A,  FN_SCIF2,    IPSR_TX3_A,         3U }, /* TX3_A */
    { GRP_SCIF3_A,  FN_SCIF2,    IPSR_RX3_A,         3U }, /* RX3_A */
    { GRP_SCIF3_B,  FN_SCIF3,    IPSR_TX3_B,         1U }, /* TX3_B */
    { GRP_SCIF3_B,  FN_SCIF3,    IPSR_RX3_B,         1U }, /* RX3_B */

    { GRP_SCIF4_A,  FN_SCIF4,    IPSR_TX4_A,         3U }, /* TX4_A */
    { GRP_SCIF4_A,  FN_SCIF4,    IPSR_RX4_A,         3U }, /* RX4_A */
    { GRP_SCIF4_B,  FN_SCIF4,    IPSR_TX4_B,         3U }, /* TX4_B */
    { GRP_SCIF4_B,  FN_SCIF4,    IPSR_RX4_B,         3U }, /* RX4_B */
    { GRP_SCIF4_C,  FN_SCIF4,    IPSR_TX4_C,         3U }, /* TX4_C */
    { GRP_SCIF4_C,  FN_SCIF4,    IPSR_RX4_C,         3U }, /* RX4_C */

    { GRP_SCIF5_A,  FN_SCIF5,    IPSR_TX5_A,         1U }, /* TX5_A */
    { GRP_SCIF5_A,  FN_SCIF5,    IPSR_RX5_A,         1U }, /* RX5_A */
    { GRP_SCIF5_B,  FN_SCIF5,    IPSR_TX5_B,         0xaU }, /* TX5_B */
    { GRP_SCIF5_B,  FN_SCIF5,    IPSR_RX5_B,         0xaU }, /* RX5_B */

    { GRP_ETHERAVB, FN_ETHERAVB, IPSR_AVB_MDC,       0U }, /* AVB_MDC */
    { GRP_ETHERAVB, FN_ETHERAVB, IPSR_AVB_PHY_INT,   0U }, /* AVB_PHY_INT */
    { GRP_ETHERAVB, FN_ETHERAVB, IPSR_AVB_LINK,      0U }, /* AVB_AVB_LINK */

    { GRP_SDHI0,	FN_SDHI0,	 IPSR_SD0_DAT0,      0U }, /* SD0 DAT0 */
    { GRP_SDHI0,	FN_SDHI0,	 IPSR_SD0_DAT1,      0U }, /* SD0 DAT1 */
    { GRP_SDHI0,	FN_SDHI0,	 IPSR_SD0_DAT2,      0U }, /* SD0 DAT2 */
    { GRP_SDHI0,	FN_SDHI0,	 IPSR_SD0_DAT3,      0U }, /* SD0 DAT3 */
    { GRP_SDHI0,	FN_SDHI0,	 IPSR_SD0_CLK,	     0U }, /* SD0 CLK */
    { GRP_SDHI0,	FN_SDHI0,	 IPSR_SD0_CMD,	     0U }, /* SD0 CMD */

    { GRP_SDHI1,    FN_MMC0,     IPSR_SD1_DAT0,      1U }, /* MMC0_DAT4 */
    { GRP_SDHI1,    FN_MMC0,     IPSR_SD1_DAT1,      1U }, /* MMC0_DAT5 */
    { GRP_SDHI1,    FN_MMC0,     IPSR_SD1_DAT2,      1U }, /* MMC0_DAT6 */
    { GRP_SDHI1,    FN_MMC0,     IPSR_SD1_DAT3,      1U }, /* MMC0_DAT7 */
    { GRP_SDHI2,    FN_MMC0,     IPSR_SD2_CLK,       0U }, /* MMC0_CLK */
    { GRP_SDHI2,    FN_MMC0,     IPSR_SD2_CMD,       0U }, /* MMC0_CMD */
    { GRP_SDHI2,    FN_MMC0,     IPSR_SD2_DAT0,      0U }, /* MMC0_DAT0 */
    { GRP_SDHI2,    FN_MMC0,     IPSR_SD2_DAT1,      0U }, /* MMC0_DAT1 */
    { GRP_SDHI2,    FN_MMC0,     IPSR_SD2_DAT2,      0U }, /* MMC0_DAT2 */
    { GRP_SDHI2,    FN_MMC0,     IPSR_SD2_DAT3,      0U }, /* MMC0_DAT3 */
    { GRP_SDHI2,    FN_MMC0,     IPSR_SD2_DS,        0U }, /* MMC0_DS */

    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_CMD,       0U }, /* MMC1 CMD */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_CLK,       0U }, /* MMC1_CLK */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT0,      0U }, /* MMC1_DAT0 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT1,      0U }, /* MMC1_DAT1 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT2,      0U }, /* MMC1_DAT2 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT3,      0U }, /* MMC1_DAT3 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT4,      0U }, /* MMC1_DAT4 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT5,      0U }, /* MMC1_DAT5 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT6,      0U }, /* MMC1_DAT6 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DAT7,      0U }, /* MMC1_DAT7 */
    { GRP_SDHI3,    FN_MMC1,     IPSR_SD3_DS,        0U }, /* MMC1_DS */

    /* no function select for I2C 0,3,4,5 */

    { GRP_I2C1_A,   FN_I2C1,     IPSR_SCL1_A,        4U }, /* SCL1_A */
    { GRP_I2C1_A,   FN_I2C1,     IPSR_SDA1_A,        4U }, /* SDA1_A */
    { GRP_I2C1_B,   FN_I2C1,     IPSR_SCL1_B,        4U }, /* SCL1_B */
    { GRP_I2C1_B,   FN_I2C1,     IPSR_SDA1_B,        4U }, /* SDA1_B */

    { GRP_I2C2_A,   FN_I2C2,     IPSR_SCL2_A,        4U }, /* SCL2_A */
    { GRP_I2C2_A,   FN_I2C2,     IPSR_SDA2_A,        4U }, /* SDA2_A */
    { GRP_I2C2_B,   FN_I2C2,     IPSR_SCL2_B,        4U }, /* SCL2_A */
    { GRP_I2C2_B,   FN_I2C2,     IPSR_SDA2_B,        4U }, /* SDA2_A */

    { GRP_I2C6_A,   FN_I2C6,     IPSR_SCL6_A,        7U }, /* SCL6_A */
    { GRP_I2C6_A,   FN_I2C6,     IPSR_SDA6_A,        7U }, /* SDA6_A */
    { GRP_I2C6_B,   FN_I2C6,     IPSR_SCL6_B,        7U }, /* SCL6_B */
    { GRP_I2C6_B,   FN_I2C6,     IPSR_SDA6_B,        7U }, /* SDA6_B */
    { GRP_I2C6_C,   FN_I2C6,     IPSR_SCL6_C,        7U }, /* SCL6_C */
    { GRP_I2C6_C,   FN_I2C6,     IPSR_SDA6_C,        7U }, /* SDA6_C */

    { GRP_USB0,   	FN_USB0,     IPSR_USB0_PWEN,     0U }, /* USB0_PWEN */
    { GRP_USB0,   	FN_USB0,     IPSR_USB0_OVC,      0U }, /* USB0_OVC */

    { GRP_USB1,   	FN_USB1,     IPSR_USB1_PWEN,     0U }, /* USB1_PWEN */
    { GRP_USB1,   	FN_USB1,     IPSR_USB1_OVC,      0U }, /* USB1_OVC */

    { GRP_USB3,   	FN_USB3,     IPSR_USB30_PWEN,    0U }, /* USB30_PWEN */
    { GRP_USB3,   	FN_USB3,     IPSR_USB30_OVC,     0U }, /* USB30_OVC */

    { GRP_NONE,     FN_NONE,     0U,                 0U }  /* list terminator */
    };

/*
 *  supported PFC module selections
 *
 *  This table defines the function assignments to pin groups, together with the
 *  setting required in the MOD_SEL register.
 *  This table can be expanded to support additional module selections.
 */

LOCAL const PFC_MOD_DESC pfcModuleSel [] =
    {
    /* group        function   modSelMask      modSelVal */
    { GRP_SCIF1_A,  FN_SCIF1,  MOD_SEL_SCIF1,  0U },    /* RX1_A/TX1_A */
    { GRP_SCIF1_B,  FN_SCIF1,  MOD_SEL_SCIF1,  1U },    /* RX1_B/TX1_B */

    { GRP_SCIF2_A,  FN_SCIF2,  MOD_SEL_SCIF2,  0U },    /* RX2_A/TX2_A */
    { GRP_SCIF2_B,  FN_SCIF2,  MOD_SEL_SCIF2,  1U },    /* RX2_B/TX2_B */

    { GRP_SCIF3_A,  FN_SCIF3,  MOD_SEL_SCIF3,  0U },    /* RX3_A/TX3_A */
    { GRP_SCIF3_B,  FN_SCIF3,  MOD_SEL_SCIF3,  1U },    /* RX3_B/TX3_B */

    { GRP_SCIF4_A,  FN_SCIF4,  MOD_SEL_SCIF4,  0U },    /* RX4_A/TX4_A */
    { GRP_SCIF4_B,  FN_SCIF4,  MOD_SEL_SCIF4,  1U },    /* RX4_B/TX4_B */
    { GRP_SCIF4_C,  FN_SCIF4,  MOD_SEL_SCIF4,  2U },    /* RX4_C/TX4_C */

    { GRP_SCIF5_A,  FN_SCIF5,  MOD_SEL_SCIF5,  0U },    /* RX5_A/TX5_A */
    { GRP_SCIF5_B,  FN_SCIF5,  MOD_SEL_SCIF5,  1U },    /* RX5_B/TX5_B */

    { GRP_I2C0,     FN_I2C0,   MOD_SEL_I2C0,   1U },    /* SCL0/SDA0 */

    { GRP_I2C1_A,   FN_I2C1,   MOD_SEL_I2C1,   0U },    /* SCL1/SDA1_A */
    { GRP_I2C1_B,   FN_I2C1,   MOD_SEL_I2C1,   1U },    /* SCL1/SDA1_B */

    { GRP_I2C2_A,   FN_I2C2,   MOD_SEL_I2C2,   0U },    /* SCL2/SDA2_A */
    { GRP_I2C2_B,   FN_I2C2,   MOD_SEL_I2C2,   1U },    /* SCL2/SDA2_B */

    { GRP_I2C3,     FN_I2C3,   MOD_SEL_I2C3,   1U },    /* SCL3/SDA3 */
    { GRP_I2C5,     FN_I2C5,   MOD_SEL_I2C5,   1U },    /* SCL5/SDA5 */

    { GRP_I2C6_A,   FN_I2C6,   MOD_SEL_I2C6,   0U },    /* SCL6/SDA6_A */
    { GRP_I2C6_B,   FN_I2C6,   MOD_SEL_I2C6,   1U },    /* SCL6/SDA6_B */
    { GRP_I2C6_C,   FN_I2C6,   MOD_SEL_I2C6,   2U },    /* SCL6/SDA6_C */

    { GRP_NONE,     FN_NONE,   0U,             0U }     /* list terminator */
    };

LOCAL const PFC_POC_DESC pfcPocSel [] =
    {
    /* function pocMask  pocVal */
    { FN_SDHI0, POC_SD0, 0x3fU  },      /* RX1_A/TX1_A/SCIF_CLK_A */
    { FN_SDHI3, POC_SD3, 0x7ffU },      /* RX2_A/TX2_A/SCK2 */
    { FN_NONE,  0U,      0U }           /* list terminator */
    };

/* forward declarations for pin configuration handlers */

LOCAL PFC_PIN_HANDLER rzg2PfcPullDisable;
LOCAL PFC_PIN_HANDLER rzg2PfcPullDownEnable;
LOCAL PFC_PIN_HANDLER rzg2PfcPullUpEnable;
LOCAL PFC_PIN_HANDLER rzg2PfcDriveControl;
LOCAL PFC_PIN_HANDLER rzg2PfcGpioEnable;

/*
 * supported device tree pin configurations
 *
 * This table defines the supported pin configuration properties in the device
 * tree. Each property is associated with a handler function.
 */

LOCAL const PFC_PIN_CONF pfcConfs [] =
    {
    { "bias-disable",   rzg2PfcPullDisable },
    { "bias-pull-down", rzg2PfcPullDownEnable },
    { "bias-pull-up",   rzg2PfcPullUpEnable },
    { "drive-control",  rzg2PfcDriveControl },
    { "gpio-enable",    rzg2PfcGpioEnable },
    { NULL,             NULL }              /* list terminator */
    };

LOCAL const char pfcPocConfs [] = "power-source";

/* pin pull state used for verbose show method */

LOCAL const char * pfcPinPullStateStr [] =
    {
    [PIN_PU_DISABLED]  = "pull disabled",
    [PIN_PU_PULL_UP]   = "pull-up",
    [PIN_PU_PULL_DOWN] = "pull-down"
    };

/* forward declarations */

LOCAL PFC_PIN_HANDLER rzg2PfcPeriphEnable;

LOCAL const VXB_FDT_DEV_MATCH_ENTRY vxbFdtPfcPinMuxMatch [] =
    {
        {
        "renesas,pfc-r8a774a1",
        (void *) &xPfcFuncSel[0]
        },
        {}                              /* Empty terminated list */
    };

/* declare VxBus attachment */

VXB_DRV vxbFdtRsRzg2PfcPinMuxDrv =
    {
    { NULL } ,                          /* Linked list header */
    "pfc",                              /* Name */
    "Renesas RZG2 PFC",                 /* Description */
    VXB_BUSID_FDT,                      /* Class */
    0U,                                 /* Flags */
    0,                                  /* Reference count */
    vxbFdtPfcPinMuxMethodList,          /* Method table */
    };

VXB_DRV_DEF (vxbFdtRsRzg2PfcPinMuxDrv);

/*******************************************************************************
*
* rzg2PfcRegRead - read a PFC register
*
* This function reads a PFC register at <offset> and returns the value.
*
* RETURNS: the value read from the register.
*
* ERRNO: N/A.
*/

LOCAL UINT32 rzg2PfcRegRead
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    UINT32                  offset      /* PFC register offset */
    )
    {
    return vxbRead32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + offset));
    }

/*******************************************************************************
*
* rzg2PfcRegBitSet - set individual bits in a PFC register
*
* This function sets just the individual <bits> in a PFC register at <offset>.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2PfcRegBitSet
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    UINT32                  offset,     /* PFC register offset */
    UINT32                  bits        /* bits to set */
    )
    {
    UINT32 val;

    (void)semTake (pPfc->mutex, WAIT_FOREVER);

    val = vxbRead32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + offset));
    val |= bits;

    /*
     * For all registers except PUENn and PUDn, enable writing to PFC register
     * by first writing inverted data to PMMR.
     */

    if ((offset < PFC_PUEN(0)) || (offset > PFC_PUD(6)))
        {
        vxbWrite32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + PFC_PMMR), ~val);
        }

    vxbWrite32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + offset), val);

    (void)semGive (pPfc->mutex);
    }

/*******************************************************************************
*
* rzg2PfcRegBitClr - clear individual bits in a PFC register
*
* This function clears just the individual <bits> in a PFC register at <offset>.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2PfcRegBitClr
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    UINT32                  offset,     /* PFC register offset */
    UINT32                  bits        /* bits to clear */
    )
    {
    UINT32 val;

    (void)semTake (pPfc->mutex, WAIT_FOREVER);

    val = vxbRead32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + offset));
    val &= ~bits;

    /*
     * For all registers except PUENn and PUDn, enable writing to PFC register
     * by first writing inverted data to PMMR.
     */

    if ((offset < PFC_PUEN(0)) || (offset > PFC_PUD(6)))
        {
        vxbWrite32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + PFC_PMMR), ~val);
        }
    vxbWrite32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + offset), val);

    (void)semGive (pPfc->mutex);
    }

/*******************************************************************************
*
* rzg2PfcRegMaskAndSet - mask and set bits in a PFC register
*
* This function clears the <mask> bits then sets <bits> in a PFC register at
* <offset>.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2PfcRegMaskAndSet
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    UINT32                  offset,     /* PFC register offset */
    UINT32                  mask,       /* bits to clear */
    UINT32                  bits        /* bits to set */
    )
    {
    UINT32 val;

    (void)semTake (pPfc->mutex, WAIT_FOREVER);

    val = vxbRead32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + offset));
    val &= ~mask;
    val |= bits;

    /*
     * For all registers except PUENn and PUDn, enable writing to PFC register
     * by first writing inverted data to PMMR.
     */

    if ((offset < PFC_PUEN(0)) || (offset > PFC_PUD(6)))
        {
        vxbWrite32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + PFC_PMMR), ~val);
        }

    vxbWrite32 (pPfc->pHandle, (UINT32 *)(pPfc->regBase + offset), val);

    (void)semGive (pPfc->mutex);
    }

/*******************************************************************************
*
* rzg2PfcFuncIdGet - get the PFC function ID
*
* This function gets the function ID for the named group.
*
* RETURNS: Function ID, or FN_NONE if the function name is not valid.
*
* ERRNO: N/A.
*/

LOCAL PFC_FUNC_ID rzg2PfcFuncIdGet
    (
    char *          funcName
    )
    {
    PFC_FUNC_ID     funcId = FN_NONE;
    UINT32          i;

    if (funcName == NULL)
        {
        return FN_NONE;
        }

    /* search the list of function names */

    for (i = 0; i < PFC_NUM_FUNCS; i++)
        {
        if (strncmp (pfcFuncs[i].funcName, funcName, PFC_NAME_LEN_MAX) == 0)
            {
            funcId = pfcFuncs[i].funcId;
            break;
            }
        }

    return funcId;
    }

/*******************************************************************************
*
* rzg2PfcPinGet - get the PFC pin descriptor
*
* This function gets the pin descriptor for the named pin.
*
* RETURNS: pin descriptor, or NULL if no matching pin found.
*
* ERRNO: N/A.
*/

LOCAL PFC_PIN_DESC * rzg2PfcPinGet
    (
    char *  pinName
    )
    {
    PFC_PIN_DESC * pPin;

    if (pinName == NULL)
        {
        return NULL;
        }

    pPin = (PFC_PIN_DESC *) &pfcPins [0];

    while (!PFC_PIN_DESC_TBL_END(pPin))
        {
        if ((strncmp (pPin->sipName, pinName, PFC_NAME_LEN_MAX) == 0))
            {
            return pPin;
            }
        pPin++;
        }

    return NULL;
    }

/*******************************************************************************
*
* rzg2PfcPinsGroupGet - validate and get the PFC group ID for a list of pins
*
* This function validates the list of pins as all belonging to the same
* functional group and returns the group ID. The pins must also have a GPSR
* entry.
*
* RETURNS: group ID, or GRP_NONE if the pins are not in a single group or any
* don't have a GPSR entry.
*
* ERRNO: N/A.
*/

LOCAL PFC_GROUP_ID rzg2PfcPinsGroupGet
    (
    char *          pPinsProp,      /* pins property */
    int             pPinsPropLen,   /* pins property length */
    PFC_FUNC_ID     funcId          /* function ID */
    )
    {
    PFC_GROUP_ID    groupId = GRP_NONE;
    PFC_PIN_DESC *  pPin;
    int             stringLen;

    if ((pPinsProp == NULL) || (pPinsPropLen <= 0))
        {
        return GRP_NONE;
        }

    /* operate on each pin in the "pins" property stringList */

    while (pPinsPropLen >  0)
        {

        /* retrieve, and validate, the descriptor for the pin name */

        pPin = rzg2PfcPinGet (pPinsProp);
        if ((pPin == NULL) || (pPin->gpsrRegBit == GPSR_NONE))
            {
            return GRP_NONE;
            }

        /* compare group ID of all pins */

        if (groupId == GRP_NONE)
            {
            groupId = pPin->groupId;
            }
        else
            {
            /* invalidate group ID if mismatched */

            if (pPin->groupId != groupId)
                {
                groupId = GRP_NONE;
                break;
                }
            }

        /* potentially move onto the next pin in the "pins" stringList */

        stringLen = (int) (strlen (pPinsProp) + 1);
        pPinsPropLen  -= stringLen;
        pPinsProp += stringLen;
        }

    return groupId;
    }

/*******************************************************************************
*
* rzg2PfcPeriphEnable - enable PFC Peripheral function
*
* This function enables peripheral functionality of the requested pin using the
* GPIO/Peripheral Function Select Registers.
*
* It is passed as a parameter to, and called from, rzg2PfcFdtPinListHandle so it
* is of PFC_PIN_HANDLE type. The pProp and propLen properties defined by the
* PFC_PIN_HANDLE type are not used in this handler.
*
* RETURNS: OK if pin configured, ERROR if pin controller, pin descriptor or pin
* register is invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcPeriphEnable
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_PIN_DESC *          pPin,       /* pin descriptor */
    char *                  pProp,      /* property (not used) */
    int                     propLen     /* property length (not used) */
    )
    {
    if ((pPfc == NULL) || (pPin == NULL) || (pPin->gpsrRegBit == GPSR_NONE))
        {
        return ERROR;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "Enabling peripheral function on %s\n",
                  pPin->sipName);

    /* GPSR register bit is 1 for Peripheral function */

    rzg2PfcRegBitSet (pPfc, PFC_GPSR (GPSR_REG (pPin->gpsrRegBit)),
                      GPSR_MASK (pPin->gpsrRegBit));

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcGpioEnable - enable PFC GPIO function
*
* This function enables GPIO functionality of the requested pin using the
* GPIO/Peripheral Function Select Registers.
*
* RETURNS: OK if pin configured, ERROR if pin controller, pin descriptor or pin
* register is invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcGpioEnable
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_PIN_DESC *          pPin,       /* pin descriptor */
    char *                  pProp,      /* property (not used) */
    int                     propLen     /* property length (not used) */
    )
    {
    if ((pPfc == NULL) || (pPin == NULL) || (pPin->gpsrRegBit == GPSR_NONE))
        {
        return ERROR;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "Enabling GPIO on %s, reg: 0x%x, bit: 0x%x\n",
                  pPin->sipName,
                  PFC_GPSR (GPSR_REG (pPin->gpsrRegBit)),
                  GPSR_MASK (pPin->gpsrRegBit));

    /* GPSR register bit is 0 for GPIO */

    rzg2PfcRegBitClr (pPfc, PFC_GPSR (GPSR_REG (pPin->gpsrRegBit)),
                      GPSR_MASK (pPin->gpsrRegBit));

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcPullDisable - disable PFC pull-up/pull-down function
*
* This function disables the pull-up/pull down functionality of the requested pin
* using the Pull Enable Registers.
*
* RETURNS: OK if pin configured, ERROR if pin controller, or pin descriptor is
* invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcPullDisable
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_PIN_DESC *          pPin,       /* pin descriptor */
    char *                  pProp,      /* property (not used) */
    int                     propLen     /* property length (not used) */
    )
    {
    if ((pPfc == NULL) || (pPin == NULL))
        {
        return ERROR;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "Disabling pull on %s\n", pPin->sipName);

    /* PUEN register bit is 0 for pull-up/down function disable */

    rzg2PfcRegBitClr (pPfc, PFC_PUEN (PU_REG (pPin->puRegBit)),
                      PU_MASK (pPin->puRegBit));

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcPullUpEnable - enable PFC pull-up function
*
* This function enables the pull up functionality of the requested pin
* using the Pull Enable, and Pull Up Down Control Registers.
*
* RETURNS: OK if pin configured, ERROR if pin controller, or pin descriptor is
* invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcPullUpEnable
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_PIN_DESC *          pPin,       /* pin descriptor */
    char *                  pProp,      /* property (not used) */
    int                     propLen     /* property length (not used) */
    )
    {
    if ((pPfc == NULL) || (pPin == NULL))
        {
        return ERROR;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "Enabling pull-up on %s\n", pPin->sipName);

    /* PUD register bit is 1 for pull-up function */

    rzg2PfcRegBitSet (pPfc, PFC_PUD (PU_REG (pPin->puRegBit)),
                      PU_MASK (pPin->puRegBit));

    /* PUEN register bit is 1 for pull function enable */

    rzg2PfcRegBitSet (pPfc, PFC_PUEN (PU_REG (pPin->puRegBit)),
                      PU_MASK (pPin->puRegBit));

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcPullDownEnable - enable PFC pull-down function
*
* This function enables the pull down functionality of the requested pin
* using the Pull Enable, and Pull Up Down Control Registers.
*
* RETURNS: OK if pin configured, ERROR if pin controller, or pin descriptor is
* invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcPullDownEnable
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_PIN_DESC *          pPin,       /* pin descriptor */
    char *                  pProp,      /* property (not used) */
    int                     propLen     /* property length (not used) */
    )
    {

    if ((pPfc == NULL) || (pPin == NULL))
        {
        return ERROR;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "Enabling pull-down on %s/%s\n", pPin->sipName);

    /* PUD register bit is 0 for pull-down function */

    rzg2PfcRegBitClr (pPfc, PFC_PUD (PU_REG (pPin->puRegBit)),
                      PU_MASK (pPin->puRegBit));

    /* PUEN register bit is 1 for pull function enable */

    rzg2PfcRegBitSet (pPfc, PFC_PUEN (PU_REG (pPin->puRegBit)),
                      PU_MASK (pPin->puRegBit));

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcDriveControl - configure PFC drive control function
*
* This function configures the driving ability of the requested pin
* using the DRV Control Registers.
*
* RETURNS: OK if pin configured, ERROR if pin controller, pin descriptor, or
* drive control property is invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcDriveControl
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_PIN_DESC *          pPin,       /* pin descriptor */
    char *                  pProp,      /* configuration property */
    int                     propLen     /* configuration property length */
    )
    {
    UINT    mask;                       /* DRVCTRL mask */
    UINT32  drvVal;                     /* drive-control property value */

    /* validate parameters and expect <u32> property value */

    if ((pPfc == NULL) || (pPin == NULL) || (pProp == NULL) ||
        (propLen != sizeof (UINT32)))
        {
        return ERROR;
        }

    mask = DRVCTRL_BITMASK (pPin->drvMask);
    drvVal = vxFdt32ToCpu (*(UINT32 *)pProp);

    RZG2_DBG_MSG (PFC_DBG_INFO, "Configuring %d/8 DRV control on %s\n",
                  drvVal, pPin->sipName);

    /*
     * The drive-control property value must be in the range 1 to 8. For
     * F type buffers, which have 2-bit masks, the value must be an even
     * number of eighths (quarters).
     */

    if ((drvVal < PFC_DRVCTRL_VAL_MIN) || (drvVal > PFC_DRVCTRL_VAL_MAX) ||
        ((mask == DRVCTRL_F_MASK) && ((drvVal % 2U) != 0U)))
        {
        return ERROR;
        }

    /* convert F type from eighths to quarters */

    if (mask == DRVCTRL_F_MASK)
        {
        drvVal /= 2;
        }

    /* DRVCTRL register field is 0-7 or 0-3 */

    drvVal--;

    rzg2PfcRegMaskAndSet (pPfc, PFC_DRVCTRL (DRVCTRL_REG (pPin->drvMask)),
                          DRVCTRL_MASK (pPin->drvMask),
                          drvVal << (DRVCTRL_SHIFT (pPin->drvMask)));

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcPocCtrlFuncSet - set PFC POC Control Register
*
* This function set PFC POC Control Register.
*
* RETURNS: OK if POC control register configured, ERROR if pin controller is
* invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcPocCtrlFuncSet
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_FUNC_ID             funcId,     /* function to set */
    char *                  pProp,      /* property */
    int                     propLen     /* property length */
    )
    {
    PFC_POC_DESC *          pPocCtrl;
    UINT32 mv = 0;
    UINT32 pocVal = 0;

    if (pPfc == NULL)
        {
        return ERROR;
        }

    mv = vxFdt32ToCpu(*(UINT32*)pProp);

    /* process the functions in the POC table */

    pPocCtrl = (PFC_POC_DESC *) &pfcPocSel [0];

    while (pPocCtrl->funcId != FN_NONE)
        {
        if (pPocCtrl->funcId == funcId)
            {
            if (mv == PFC_MILLIVOLT_3V3)
                {
                pocVal = POC_BITMASK (pPocCtrl->pocMask);
                RZG2_DBG_MSG(PFC_DBG_INFO, "power-source is 3300mv \r\n");
                }
            else if (mv == PFC_MILLIVOLT_1V8)
                {
                pocVal = 0;
                RZG2_DBG_MSG(PFC_DBG_INFO, "power-source is 1800mv \r\n");
                }
            else
                {
                RZG2_DBG_MSG(PFC_DBG_ERR, "invalid power-source \r\n");
                return ERROR;
                }
            rzg2PfcRegMaskAndSet (pPfc,
                                  PFC_POCCTRL,
                                  POC_MASK (pPocCtrl->pocMask),
                                  (UINT32) pocVal <<
                                  (POC_SHIFT (pPocCtrl->pocMask)));
            break;
            }
        pPocCtrl++;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "\nPOC Control Register\n");
    RZG2_DBG_MSG (PFC_DBG_INFO, "POCCTRL: 0x%08X\n",
                  rzg2PfcRegRead (pPfc, PFC_POCCTRL));

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcFdtPinListValidate - validate a FDT stringList of pins
*
* This function uses the "pins" stringList from the device table node, and splits
* the list into individual pins. Every pin in the list must have a valid
* descriptor for the list to be successfully validated.
*
* RETURNS: OK if all pins are valid, ERROR if pin controller, pin list or any
* pins are invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcFdtPinListValidate
    (
    PFC_PINMUX_DRVCTRL *    pPfc,           /* pin mux controller instance */
    char *                  pPinsProp,      /* pins property */
    int                     pinsPropLen     /* pins property length */
    )
    {
    int                     stringLen;
    PFC_PIN_DESC *          pPin;

    if ((pPfc == NULL) || (pPinsProp == NULL))
        {
        return ERROR;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "pin:");

    /* validate all pins in pin list */

    while (pinsPropLen >  0)
        {
        RZG2_DBG_MSG (PFC_DBG_INFO, "%s, ", pPinsProp);

        /* retrieve the descriptor for the pin name */

        pPin = rzg2PfcPinGet (pPinsProp);
        if (pPin == NULL)
            {
            RZG2_DBG_MSG (PFC_DBG_ERR, "Invalid pin: %s\n", pPinsProp);
            return ERROR;
            }

        /* potentially move onto the next pin in the "pins" stringList */

        stringLen = (int) (strlen (pPinsProp) + 1);
        pinsPropLen -= stringLen;
        pPinsProp += stringLen;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "\n");

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcFdtPinListHandle - handle a FDT stringList of pins with supplied function
*
* This function uses the "pins" stringList from the device table node, and splits
* the list into individual pins. It retrieves the descriptor for the pin and
* calls the provided function with each pin descriptor.
*
* RETURNS: OK if all pins are handled, ERROR if pin controller, pin list or
* handler function is invalid, or if the pin handler function fails.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcFdtPinListHandle
    (
    PFC_PINMUX_DRVCTRL *    pPfc,           /* pin mux controller instance */
    char *                  pPinsProp,      /* pins property */
    int                     pinsPropLen,    /* pins property length */
    PFC_PIN_HANDLER *       handler,        /* pin configuration handler */
    char *                  pProp,          /* configuration property */
    int                     propLen         /* configuration property length */
    )
    {
    int                     stringLen;
    PFC_PIN_DESC *          pPin;

    if ((pPfc == NULL) || (pPinsProp == NULL) || (handler == NULL))
        {
        return ERROR;
        }

    /* operate on all pins in pin list */

    while (pinsPropLen >  0)
        {

        /* retrieve the descriptor for the pin name */

        pPin = rzg2PfcPinGet (pPinsProp);
        if (pPin == NULL)
            {
            return ERROR;
            }

        /* configure the pin with the handler */

        if (handler (pPfc, pPin, pProp, propLen) == ERROR)
            {
            return ERROR;
            }

        /* potentially move onto the next pin in the "pins" stringList */

        stringLen = (int) (strlen (pPinsProp) + 1);
        pinsPropLen  -= stringLen;
        pPinsProp += stringLen;
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcIpsrGroupFuncSet - set the IPSR for a pin group
*
* This function sets the Peripheral Function Select Register (IPSR) to the
* requested function for all pins in the provided group.
*
* RETURNS: OK if set successfully or nothing to do, ERROR if pin controller is
* invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcIpsrGroupFuncSet
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_GROUP_ID            groupId,    /* pin group */
    PFC_FUNC_ID             funcId      /* function to set */
    )
    {
    PFC_FUNC_DESC *         pFunc;

    if (pPfc == NULL)
        {
        return ERROR;
        }


    /* process all group/function pairs in the function selection table */

    pFunc = (PFC_FUNC_DESC *) vxbDevDrvDataGet(pPfc->pPinMuxCtrl->pDev);

    while (pFunc->groupId != GRP_NONE)
        {
        if ((pFunc->groupId == groupId) && (pFunc->funcId == funcId))
            {
            rzg2PfcRegMaskAndSet (pPfc, PFC_IPSR (IPSR_REG (pFunc->ipsrMask)),
                                  IPSR_MASK (pFunc->ipsrMask),
                                  ((UINT32) pFunc->ipsrVal <<
                                  (IPSR_SHIFT(pFunc->ipsrMask))));
            }
        pFunc++;
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcModSelGroupFuncSet - set the module selection for a pin group
*
* This function sets the Module Selection Register (MOD_SEL) to the
* requested selection for all pins in the provided group.
*
* RETURNS: OK if set successfully, or nothing to do, ERROR if pin controller is
* invalid.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcModSelGroupFuncSet
    (
    PFC_PINMUX_DRVCTRL *    pPfc,       /* pin mux controller instance */
    PFC_GROUP_ID            groupId,    /* pin group */
    PFC_FUNC_ID             funcId      /* function to set */
    )
    {
    PFC_MOD_DESC *          pModSel;

    if (pPfc == NULL)
        {
        return ERROR;
        }

    /* process the group/function pair in the module selection table */

    pModSel = (PFC_MOD_DESC *) &pfcModuleSel [0];

    while (pModSel->groupId != GRP_NONE)
        {
        if ((pModSel->groupId == groupId) && (pModSel->funcId == funcId))
            {
            rzg2PfcRegMaskAndSet (pPfc,
                                  PFC_MOD_SEL (MOD_SEL_REG (pModSel->modSelMask)),
                                  MOD_SEL_MASK (pModSel->modSelMask),
                                  (UINT32) pModSel->modSelVal <<
                                  (MOD_SEL_SHIFT (pModSel->modSelMask)));
            break;
            }
        pModSel++;
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcFdtFunctionHandle - handle a FDT pin mux function property
*
* This function handles the pin mux "function" property from the device tree
* node. The process requires that GPIO mode is selected while changing function.
* The peripheral function and module selection registers are set while GPIO mode
* is selected.
*
* RETURNS: OK if pin mux function handled successfully, ERROR if pin controller,
* or pins property is invalid, if pins are not from the same functional
* group, or if any step in the configuration process fails.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcFdtFunctionHandle
    (
    PFC_PINMUX_DRVCTRL *    pPfc,           /* pin mux controller instance */
    char *                  pPinsProp,      /* pins property */
    int                     pinsPropLen,    /* pins property length */
    PFC_FUNC_ID             funcId          /* function ID */
    )
    {
    PFC_GROUP_ID            groupId;

    if ((pPfc == NULL) || (pPinsProp == NULL) || (pinsPropLen <= 0))
        {
        return ERROR;
        }

    /* all pins must be in the same defined functional group */

    groupId = rzg2PfcPinsGroupGet (pPinsProp, pinsPropLen, funcId);
    if (groupId == GRP_NONE)
        {
        return ERROR;
        }

    /* select GPSR GPIO mode for all pins specified */

    if (rzg2PfcFdtPinListHandle (pPfc, pPinsProp, pinsPropLen, rzg2PfcGpioEnable,
                                 NULL, 0) == ERROR)
        {
        return ERROR;
        }

    /* set Peripheral Function (IPSR) for all functions in group */

    if (rzg2PfcIpsrGroupFuncSet (pPfc, groupId, funcId) == ERROR)
        {
        return ERROR;
        }

    /* set Module Selection (MOD_SEL) for this group function */

    if (rzg2PfcModSelGroupFuncSet (pPfc, groupId, funcId) == ERROR)
        {
        return ERROR;
        }

    /* select GPSR Peripheral Function mode for all pins specified */

    if (rzg2PfcFdtPinListHandle (pPfc, pPinsProp, pinsPropLen, rzg2PfcPeriphEnable,
                                 NULL, 0) == ERROR)
        {
        return ERROR;
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcFdtConfigHandle - handle the enabling of a pin mux state
*
* This function handles the enabling of a specified pin mux state in the PFC. The
* state is specified in the FDT configuration node at <offset>.
*
* RETURNS: OK if state enabled, ERROR if the "pins" property is empty or
* missing, if the pin function is invalid, of if the pin configuration fails.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2PfcFdtConfigHandle
    (
    PFC_PINMUX_DRVCTRL *    pPfc,   /* pin mux controller instance */
    int                     offset  /* FDT configuration node */
    )
    {
    PFC_PIN_CONF *  pConf;          /* pin configuration descriptor */
    char *          pProp;          /* function/configuration property */
    int             propLen;        /* function/configuration property length */
    char *          pPinsProp;      /* pins property */
    int             pinsPropLen;    /* pins property length */
    PFC_FUNC_ID     funcId = FN_NONE;   /* function ID */

    RZG2_DBG_MSG (PFC_DBG_INFO, "%s offset %d\n", __FUNCTION__, offset);

    /* pin configuration and multiplexing both require "pins" property */

    pPinsProp = (char *) vxFdtPropGet (offset, PFC_FDT_PINS, &pinsPropLen);
    if ((pPinsProp == NULL) || (pinsPropLen <= 0))
        {
        RZG2_DBG_MSG (PFC_DBG_ERR, "Empty or no " PFC_FDT_PINS " property\n");
        return ERROR;
        }

    /* ensure all the pins are valid before starting configuration */

    if (rzg2PfcFdtPinListValidate (pPfc, pPinsProp, pinsPropLen) == ERROR)
        {
        return ERROR;
        }

    /*
     * Pin multiplexing is determined by the "function" property. The single
     * <string> value defines the function to be applied.
     */
    pProp = (char *)vxFdtPropGet (offset, PFC_FDT_FUNCTION, &propLen);
    if (pProp != NULL)
        {
        RZG2_DBG_MSG (PFC_DBG_INFO, "pin mux " PFC_FDT_FUNCTION " property: %s\n", pProp);

        /* retrieve, and validate, the function ID for the function name */

        funcId = rzg2PfcFuncIdGet (pProp);
        if (funcId == FN_NONE)
            {
            RZG2_DBG_MSG (PFC_DBG_ERR, "Invalid function %s\n", pProp);
            return ERROR;
            }

        if (rzg2PfcFdtFunctionHandle (pPfc, pPinsProp,
                                      pinsPropLen, funcId) == ERROR)
            {
            RZG2_DBG_MSG (PFC_DBG_ERR, "pin mux function %s failed\n", pProp);
            return ERROR;
            }
        }

    /*
     * Pin configuration is determined by a single <string> property. Supported
     * configuration properties and their handling functions are listed in the
     * pfcConfs table.
     */

    pConf = (PFC_PIN_CONF *) &pfcConfs [0];
    while (pConf->name != NULL)
        {
        pProp = (char *)vxFdtPropGet (offset, (char *)pConf->name, &propLen);
        if ((pProp != NULL) && (pConf->func != NULL))
            {
            RZG2_DBG_MSG (PFC_DBG_INFO, "%s found\n", pConf->name);

            if (rzg2PfcFdtPinListHandle (pPfc, pPinsProp, pinsPropLen,
                                         pConf->func, pProp, propLen) == ERROR)
                {
                RZG2_DBG_MSG (PFC_DBG_ERR, "%s failed\n", pConf->name);
                return ERROR;
                }
            }
        pConf++;
        }

    /* POC configuration is determined by "function" and "power-source" */

    pProp = (char *)vxFdtPropGet (offset, (char *)pfcPocConfs, &propLen);
    if (pProp != NULL)
        {
        RZG2_DBG_MSG (PFC_DBG_INFO, "%s found\n", pfcPocConfs);

        if (rzg2PfcPocCtrlFuncSet (pPfc, funcId, pProp, propLen) == ERROR)
            {
            RZG2_DBG_MSG (PFC_DBG_ERR, "%s failed\n", pfcPocConfs);
            return ERROR;
            }
        }

    return OK;
    }

/*******************************************************************************
*
* vxbFdtRzg2PfcPinMuxEnable - enable pin mux configuration
*
* The vxbPinMuxEnable method enables pin mux configuration in the hardware. This
* method is required for a pin mux controller device driver.
*
* RETURNS: OK if state enabled, ERROR if the device or offset is invalid, or the
* configuration fails.
*
* ERRNO: N/A.
*/

LOCAL STATUS vxbFdtRzg2PfcPinMuxEnable
    (
    VXB_DEV_ID  pDev,   /* VxBus device instance */
    INT32       offset  /* FDT configuration node */
    )
    {
    int                     nodeOffset;     /* FDT configuration subnodes */
    PFC_PINMUX_DRVCTRL *    pPfc;           /* pin mux controller instance */

    if ((pDev == NULL) || (offset <= 0))
        {
        return ERROR;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "%s offset %d\n", __FUNCTION__, offset);

    pPfc = (PFC_PINMUX_DRVCTRL *)vxbDevSoftcGet (pDev);

    /* handle this node if it has pins properties and not just subnodes */

    if (vxFdtPropGet ((int)offset, PFC_FDT_PINS, NULL) != NULL)
        {
        if (rzg2PfcFdtConfigHandle (pPfc, (int)offset) == ERROR)
            {
            return ERROR;
            }
        }

    /* handle the device tree subnodes if there are any */

    for (nodeOffset = vxFdtFirstSubnode ((int)offset); nodeOffset > 0;
         nodeOffset = vxFdtNextSubnode (nodeOffset))
        {
        if (rzg2PfcFdtConfigHandle (pPfc, nodeOffset) == ERROR)
            {
            return ERROR;
            }
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2PfcPinConfShow - print verbose pin configuration information
*
* This function prints PFC pin pull-up/pull-down configuration information for
* supported pins.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2PfcPinConfShow
    (
    PFC_PINMUX_DRVCTRL *    pPfc
    )
    {
    PFC_PIN_DESC *          pPin;
    PFC_PIN_PULL_STATE      pullState;

    pPin = (PFC_PIN_DESC *) &pfcPins [0];

    while (!PFC_PIN_DESC_TBL_END(pPin))
        {
        if ((rzg2PfcRegRead (pPfc, PFC_PUEN (PU_REG (pPin->puRegBit))) &
             PU_MASK (pPin->puRegBit)) == PUEN_DIS)
            {
            pullState = PIN_PU_DISABLED;
            }
        else if ((rzg2PfcRegRead (pPfc, PFC_PUD (PU_REG (pPin->puRegBit))) &
                  PU_MASK (pPin->puRegBit)) == PUD_DOWN)
            {
            pullState = PIN_PU_PULL_DOWN;
            }
        else
            {
            pullState = PIN_PU_PULL_UP;
            }

        if ((pPin->gpsrRegBit != GPSR_NONE) &&
            ((rzg2PfcRegRead (pPfc, PFC_GPSR (GPSR_REG (pPin->gpsrRegBit))) &
              GPSR_MASK (pPin->gpsrRegBit)) == GPSR_GPIO ))
            {
            printf ("  GPIO");
            }

        printf ("  %s: %s\n", pPin->sipName, pfcPinPullStateStr [pullState]);

        pPin++;
        }
    }

/*******************************************************************************
*
* vxbFdtRzg2PfcPinMuxShow - print pin mux information
*
* The vxbPinMuxShow method prints specific information on the console provided
* by the pin mux controller driver. This method is optional for a pin mux
* controller device driver.
*
* This function prints PFC pin mux information. Basic, non-verbose, output is a
* dump of PFC registers. In verbose mode additional information is shown for
* supported pins.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void vxbFdtRzg2PfcPinMuxShow
    (
    VXB_DEV_ID  pDev,       /* VxBus device instance */
    INT32       verbose     /* uses 0 for basic output */
    )
    {
    PFC_PINMUX_DRVCTRL *    pPfc;
    UINT                    regIndex;

    if (pDev == NULL)
        {
        RZG2_DBG_MSG (PFC_DBG_ERR, "Invalid VxBus device instance\n");
        return;
        }

    /* ignore verbose mode and just dump registers for now */

    pPfc = (PFC_PINMUX_DRVCTRL *)vxbDevSoftcGet (pDev);

    printf ("\nGPIO/Peripheral Function Select Registers\n");
    for (regIndex = 0; regIndex < NUM_GPSR_REGS; regIndex++)
        {
        printf ("GPSR%-2d: 0x%08X\n",
                regIndex, rzg2PfcRegRead (pPfc, PFC_GPSR(regIndex)));
        }

    printf ("\nPeripheral Function Select Registers\n");
    for (regIndex = 0; regIndex < NUM_IPSR_REGS; regIndex++)
        {
        printf ("IPSR%-2d: 0x%08X\n",
                regIndex, rzg2PfcRegRead (pPfc, PFC_IPSR(regIndex)));
        }

    printf ("\nDRV Control Registers\n");
    for (regIndex = 0; regIndex < NUM_DRVTRL_REGS; regIndex++)
        {
        printf ("DRVCTRL%-2d: 0x%08X\n",
                regIndex, rzg2PfcRegRead (pPfc, PFC_DRVCTRL(regIndex)));
        }

    printf ("\nPOC Control Register\n");
    printf ("POCCTRL: 0x%08X\n", rzg2PfcRegRead (pPfc, PFC_POCCTRL));

    printf ("\nTDSEL Control Register\n");
    printf ("TDSELCTRL: 0x%08X\n", rzg2PfcRegRead (pPfc, PFC_TDSELCTRL));

    printf ("\nIO Cell Control for IICDVFS\n");
    printf ("IOCTRL: 0x%08X\n", rzg2PfcRegRead (pPfc, PFC_IOCTRL));

    printf ("\nFuse Monitor Register\n");
    printf ("FUSEMON: 0x%08X\n", rzg2PfcRegRead (pPfc, PFC_FUSEMON));

    printf ("\nLSI Pin Pull-Enable Registers\n");
    for (regIndex = 0; regIndex < NUM_PUEN_REGS; regIndex++)
        {
        printf ("PUEN%d: 0x%08X\n",
                regIndex, rzg2PfcRegRead (pPfc, PFC_PUEN(regIndex)));
        }

    printf ("\nLSI Pin Pull-Up/Down Control Registers\n");
    for (regIndex = 0; regIndex < NUM_PUD_REGS; regIndex++)
        {
        printf ("PUD%d: 0x%08X\n",
                regIndex, rzg2PfcRegRead (pPfc, PFC_PUD(regIndex)));
        }

    printf ("\nModule Select Registers\n");
    for (regIndex = 0; regIndex < NUM_MOD_SEL_REGS; regIndex++)
        {
        printf ("MOD_SEL%d: 0x%08X\n",
                regIndex, rzg2PfcRegRead (pPfc, PFC_MOD_SEL(regIndex)));
        }

    if (verbose != 0)
        {
        rzg2PfcPinConfShow (pPfc);
        }

    }

/*******************************************************************************
*
* vxbFdtRzg2PfcPinMuxProbe - probe to match PFC pin mux driver to a device
*
* Check for Renesas RZ/G2 PFC (or compatible) device in the FDT.
*
* RETURNS: OK if probe passes and assumed a valid (or compatible) device, ERROR
* otherwise.
*
* ERRNO: N/A.
*/

LOCAL STATUS vxbFdtRzg2PfcPinMuxProbe
    (
    VXB_DEV_ID pDev        /* VxBus device instance */
    )
    {
    VXB_FDT_DEV_MATCH_ENTRY * pMatch;

    if (vxbFdtDevMatch (pDev, vxbFdtPfcPinMuxMatch, &pMatch) == ERROR)
        {
        return ERROR;
        }

    vxbDevDrvDataSet (pDev, (void *) pMatch->data);
    return OK;
    }

/******************************************************************************
*
* vxbFdtRzg2PfcPinMuxAttach - attach PFC device
*
* This is the Renesas RZ/G2 Pin Function Controller (PFC) initialisation
* function.
*
* RETURNS: OK if driver attached, ERROR if the device is invalid, or resources
* cannot be allocated.
*
* ERRNO: N/A
*/

LOCAL STATUS vxbFdtRzg2PfcPinMuxAttach
    (
    VXB_DEV_ID pDev                         /* VxBus device instance */
    )
    {
    PFC_PINMUX_DRVCTRL *    pPfc;           /* PFC driver control data */
    VXB_RESOURCE_ADR *      pResAdr;        /* device registers */
    VXB_PINMUX_CTRL *       pPinMuxCtrl;    /* vxbPinMuxLib control data */

    /* check for valid parameter */

    if (pDev == NULL)
        {
        return ERROR;
        }

    /* allocate the memory for the PFC driver control */

    pPfc = (PFC_PINMUX_DRVCTRL *)vxbMemAlloc (sizeof(PFC_PINMUX_DRVCTRL));
    if (pPfc == NULL)
        {
        RZG2_DBG_MSG (PFC_DBG_ERR, "vxbMemAlloc error\n");
        return ERROR;
        }

    /* dynamically allocate and initialise mutual-exclusion semaphore */

    pPfc->mutex = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE |
                              SEM_INVERSION_SAFE);
    if (pPfc->mutex == SEM_ID_NULL)
        {
        RZG2_DBG_MSG (PFC_DBG_ERR, "semMCreate error\n");
        goto errOut;
        }

    /* allocate register memory resource */

    pPfc->memRes = vxbResourceAlloc (pDev, VXB_RES_MEMORY, 0);
    if (pPfc->memRes == NULL)
        {
        RZG2_DBG_MSG (PFC_DBG_ERR, "vxbResourceAlloc(MEMORY) error\n");
        goto errOut;
        }

    /* extract register virtual address and resource handle */

    pResAdr = (VXB_RESOURCE_ADR *)pPfc->memRes->pRes;
    if (pResAdr == NULL)
        {
        RZG2_DBG_MSG (PFC_DBG_ERR, "virtual address error\n");
        goto errOut;
        }
    pPfc->regBase = (VIRT_ADDR)pResAdr->virtual;
    pPfc->pHandle = pResAdr->pHandle;

    /* allocate pin mux library representation of a pin mux controller */

    pPinMuxCtrl = (VXB_PINMUX_CTRL *) vxbMemAlloc (sizeof (VXB_PINMUX_CTRL));
    if (pPinMuxCtrl == NULL)
        {
        goto errOut;
        }

    /* save references between VxBus device and pin mux controller */

    pPinMuxCtrl->pDev = pDev;
    pPfc->pPinMuxCtrl = pPinMuxCtrl;
    vxbDevSoftcSet (pDev, (void *)pPfc);

    /* register the PFC pin mux driver */

    if (vxbPinMuxRegister (pPinMuxCtrl) == ERROR)
        {
        goto errOut;
        }

    /* enable default pin mux configuration if it exists */

    if (vxbPinMuxEnable (pDev) == ERROR)
        {
        goto errOut;
        }

    RZG2_DBG_MSG (PFC_DBG_INFO, "vxbFdtRzg2PfcPinMuxAttach OK\n");

    return OK;

errOut:

    RZG2_DBG_MSG (PFC_DBG_ERR, "vxbFdtRzg2PfcPinMuxAttach ERROR\n");

    /* free PFC resources */

    (void)vxbResourceFree (pDev, pPfc->memRes);
    vxbMemFree (pPfc->pPinMuxCtrl);
    (void)semDelete (pPfc->mutex);
    vxbDevSoftcSet (pDev, NULL);
    vxbMemFree (pPfc);

    return ERROR;
    }

/*******************************************************************************
*
* vxbFdtRzg2PfcPinMuxShutdown - shutdown PFC device
*
* Turn off the hardware and put it into a quiescent state.
*
* RETURNS: OK.

* ERRNO: N/A.
*/

LOCAL STATUS vxbFdtRzg2PfcPinMuxShutdown
    (
    VXB_DEV_ID pDev     /* VxBus device instance */
    )
    {
    /*
     * All PFC pin mux states are under the control of their respective
     * drivers, so there's nothing to do here.
     */

    return OK;
    }

/*******************************************************************************
*
* vxbFdtRzg2PfcPinMuxDetach - detach PFC device
*
* This function implements the VxBus driver detach method for the PFC pin mux
* driver. The pin mux controller is unregistered and allocated resources are
* freed.
*
* RETURNS: OK if driver detached, ERROR if device is invalid.

* ERRNO: N/A.
*/

LOCAL STATUS vxbFdtRzg2PfcPinMuxDetach
    (
    VXB_DEV_ID pDev     /* VxBus device instance */
    )
    {
    PFC_PINMUX_DRVCTRL *    pPfc;
    STATUS                  retVal;

    if (pDev == NULL)
        {
        return ERROR;
        }

    pPfc = (PFC_PINMUX_DRVCTRL *)vxbDevSoftcGet (pDev);

    retVal = vxbPinMuxUnregister (pPfc->pPinMuxCtrl);

    if (retVal == OK)
        {
        /* free PFC resources */

        (void)vxbResourceFree (pDev, pPfc->memRes);
        vxbMemFree (pPfc->pPinMuxCtrl);
        (void)semDelete (pPfc->mutex);
        vxbDevSoftcSet (pDev, NULL);
        vxbMemFree (pPfc);
        }

    return retVal;
    }
