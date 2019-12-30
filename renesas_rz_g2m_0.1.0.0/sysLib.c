/* sysLib.c - Renesas RZ/G2 system-dependent library */

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
This library provides board-specific functions for Renesas RZ/G2 processors.

INCLUDE FILES:

SEE ALSO:
\tb VxWorks Programmer's Guide: Configuration
*/

/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <boardLib.h>
#include <prjParams.h>
#include <vxFdtCpu.h>

#include <rzg2Lib.h>

/* externs */

IMPORT void cpuArmVerShow (void);

/* globals */

/*
 * sysCpuStarAddr defines the cold boot entry point for secondary cores in
 * an SMP system.
 *
 * For 32-bit VxWorks the code at this location must contain ARMv8 AArch64
 * instructions that will change the execution state to AArch32.
 */
#ifdef INCLUDE_RZG2_AARCH32_TRAMPOLINE
const VIRT_ADDR sysCpuStartAddr = (VIRT_ADDR)(LOCAL_MEM_LOCAL_ADRS +
                                              TRAMPOLINE_OFFSET);
#else
const VIRT_ADDR sysCpuStartAddr = (VIRT_ADDR)(LOCAL_MEM_LOCAL_ADRS +
                                              RAM_LOW_ADRS);
#endif

/* locals */

#ifdef INCLUDE_SHOW_ROUTINES
LOCAL void   rzg2BoardInfoShow (void);
LOCAL char * rzg2CpuRevGet     (void);
LOCAL char * rzg2CpuNameGet    (void);
#endif /* INCLUDE_SHOW_ROUTINES */

/* RZ_G2M board function table initialisation */

LOCAL BOARD_FUNC_TBL rzg2FuncTbl =
    {
    /* .earlyInit  = */ rzg2EarlyInit,
    /* .init       = */ rzg2Init,
    /* .reset      = */ rzg2Reset,
    /* .model      = */ rzg2ModelGet,
    /* .usDelay    = */ rzg2UsDelay,

#ifdef _WRS_CONFIG_SMP
    /* .cpuEn      = */ rzg2CpuEnable,
    /* .cpuAvail   = */ vxFdtCpuAvail,
    /* .cpuDis     = */ NULL,
#endif /*_WRS_CONFIG_SMP*/

#ifdef INCLUDE_SHOW_ROUTINES
    /* .infoShow   = */ rzg2BoardInfoShow,
#else
    /* .infoShow   = */ NULL,
#endif /* INCLUDE_SHOW_ROUTINES */

    /* .endMacGet  = */ NULL
    };

/* RZ_G2M board description table initialisation */

LOCAL BOARD_DESC renesasRzg2 =
    {
    /* .uVer     = */ BOARD_DESC_VER_DEF,
    /* .pCompat  = */ "hoperun,hihope-rzg2m",
    
#ifdef INCLUDE_DEBUG_KPRINTF
    /* .uFlag    = */ BOARD_DESC_FLAG (BOARD_DESC_FLAG_DBG, 0),
#else
    /* .uFlag    = */ BOARD_DESC_FLAG (0, 0),
#endif

    /* .probe    = */ rzg2Probe,
    /* .pFuncTbl = */ &rzg2FuncTbl
    };

BOARD_DEF (renesasRzg2)

#ifdef INCLUDE_SHOW_ROUTINES

/*******************************************************************************
*
* rzg2BoardInfoShow - print board information
*
* This function prints the board information: board type, CPU and core name and
* revision.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2BoardInfoShow (void)
    {
    char * boardName;
    char * boardRev;

    boardName = "Hoperun Hihope RZ/G2M";
    boardRev  = "(unknown revision)";

    printf ("%s %s, CPU:%s Rev %s, ",
            boardName, boardRev, rzg2CpuNameGet (), rzg2CpuRevGet ());

    cpuArmVerShow ();
    }

/*******************************************************************************
 *
 * rzg2CpuRevGet - get RZ/G 2nd Series CPU name
 *
 * This function returns the RZ/G 2nd Series CPU name.
 *
 * RETURNS: pointer to RZ/G 2nd Series CPU name string.
 *
 * ERRNO: N/A.
 */

LOCAL char * rzg2CpuNameGet (void)
    {
    char * name;
    UINT32 prodId = rzg2ProductIdGet ();

    switch (RZ_G2_PRODUCT(prodId))
        {
        case RZG2_M:
            name = "RZ/G2M";
            break;
        default:
            name = "Unknown";
            break;
        }

    return name;
    }

/*******************************************************************************
 *
 * rzg2CpuRevGet - get RZ/G 2nd Series CPU revision string
 *
 * This function returns the RZ/G 2nd Series CPU revision.
 *
 * RETURNS: pointer to RZ/G 2nd Series CPU revision string.
 *
 * ERRNO: N/A.
 */

LOCAL char * rzg2CpuRevGet (void)
    {
    char * rev;
    UINT32 prodId = rzg2ProductIdGet ();

    switch (RZ_G2_CUT(prodId))
        {
        default:
            rev = "unknown";
            break;
        }

    return rev;
    }

#endif /* INCLUDE_SHOW_ROUTINES */
