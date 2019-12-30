/* rzg2Lib.c - Renesas RZ/G2 processor support library */

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
This library provides platform-specific functions for the Renesas RZ/G2
processor.

INCLUDE FILES: sysLib.h string.h intLib.h

SEE ALSO:
\tb VxWorks Programmer's Guide: Configuration
\tb ARMv8-A Architecture Reference Manual
*/

/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <boardLib.h>
#include <sysLib.h>
#include <string.h>
#include <intLib.h>
#include <vxLib.h>
#include <cacheLib.h>
#include <dllLib.h>
#include <pmapLib.h>
#include <vxFdtLib.h>
#include <vxFdtCpu.h>
#include <hwif/vxBus.h>
#include <arch/arm/mmuArmLib.h>
#include <private/adrSpaceLibP.h>
#include <private/vmLibP.h>
#include <private/windLibP.h>
#include <private/pmapLibP.h>
#include <private/vxCpuLibP.h>
#include <sys/arm/psciLib.h>

#include <rzg2Lib.h>
#include <vxbFdtRsScifSio.h>

/* defines */

#undef RZG2_LIB_DBG
#ifdef RZG2_LIB_DBG

#include <private/kwriteLibP.h>     /* _func_kprintf */

#undef LOCAL
#define LOCAL

#define RZG2_LIB_DBG_MSG(...)                           \
    do                                                  \
        {                                               \
        if (_func_kprintf != NULL)                      \
            {                                           \
            (* _func_kprintf)(__VA_ARGS__);             \
            }                                           \
        }                                               \
    while (FALSE)
#else
#define RZG2_LIB_DBG_MSG(...)
#endif  /* RZG2_LIB_DBG */

#define RZ_G2_READ_32(addr)           *(volatile UINT32 *)(addr)
#define RZ_G2_WRITE_32(addr, data)    *(volatile UINT32 *)(addr) = (data)

/*
 * default MMU attribute for device registers mapped into kernel virtual 
 * address space by the PSL
 */

#define RZ_G2_DEVICE_MMU_ATTR         (MMU_ATTR_SUP_RW    |     \
                                       MMU_ATTR_CACHE_OFF |     \
                                       MMU_ATTR_CACHE_GUARDED)

/*
 * RAM start physical address
 *
 * The start of RAM can be found in 32-bit and 64-bit address space.
 * The 32-bit RAM address range mirrors the first 2 GB of memory in 64-bit
 * address space.
 */

#define RZ_G2_PHYS_START_HIGH         0x400000000ULL
#define RZ_G2_PHYS_START_LOW          0x040000000ULL
#define RZ_G2_PHYS_END_LOW            0x0BFFFFFFFULL

/* product register definitions */

#define RZ_G2_PRR_OFFSET              0x0044U

#define RZ_G2_PRR(base)     *(volatile UINT32 *)((base) + RZ_G2_PRR_OFFSET)

/* SCIF register definitions */

#define SCIF_SCSCR_REG      *(volatile UINT16 *)(rzg2UartBase + SCIF_SCSCR)
#define SCIF_SCFTDR_REG     *(volatile UINT8 * )(rzg2UartBase + SCIF_SCFTDR)
#define SCIF_SCFSR_REG      *(volatile UINT16 *)(rzg2UartBase + SCIF_SCFSR)
#define SCIF_SCFDR_REG      *(volatile UINT16 *)(rzg2UartBase + SCIF_SCFDR)

/*
 * transmit polling timeouts
 *
 * The default UART baud rate set by ARM Trusted Firmware is 115200, which
 * gives a character period of 87us.
 *
 * The SCIF TDFE flag trigger level is configurable from 0 to 8 characters.
 * Therefore worst case, FIFO will take at least 8 characters periods to drain.
 */

#define SCIF_TX_POLL_DELAY_US           1U
#define SCIF_TDFE_POLL_COUNT_MAX        100U
#define SCIF_TEND_POLL_COUNT_MAX        (SCIF_TDFE_POLL_COUNT_MAX * 10U)

/* define for early debug out */

#define DEBUG_EARLY_PRINT

/* default RZ/G2 model string */

#define RZG2_MODEL_DEFAULT          "Unknown RZ/G2 board"

#define RZG2_PRODCUT_NODE           "renesas,r8a774a1-prr"
#define RZG2_SCIF_NODE              "renesas,rcar-h3-scif"

/* externs */

IMPORT void sysInit (void);
IMPORT void armSysToMonitor      (FUNCPTR core0ExitFunc,
                                  FUNCPTR pspExitFunc,
                                  int startType);
IMPORT UINT32 vxCpuIdGetByIndex (UINT32);

#ifdef _WRS_CONFIG_SMP
IMPORT STATUS armMonitorSpinRelease (UINT32);
IMPORT STATUS armMpCoreInit     (UINT32, WIND_CPU_STATE *, FUNCPTR);
#endif /* _WRS_CONFIG_SMP */

IMPORT VIRT_ADDR mmuPtMemBase;

/* locals */

/* RZ/G2 product identifier. 0 indicates an invalid Id */

LOCAL UINT32 rzg2ProductId;

/* debug console register base in CPU virtual address space */

LOCAL VIRT_ADDR rzg2UartBase;

/* ARM core generic timer frequency (in Hz) */

LOCAL UINT32 rzg2GenTimerFreq;

/* function declarations */

LOCAL void   rzg2ProductRegRead  (void);
LOCAL UINT32 rzg2CounterFreqGet  (void);
LOCAL UINT64 rzg2CounterValueGet (void);

#ifdef DEBUG_EARLY_PRINT
LOCAL void   rzg2EarlyDebugInit       (void);
#endif /* DEBUG_EARLY_PRINT */

LOCAL void   rzg2KernelDebugInit (void);
LOCAL void   rzg2DebugInit (BOOL earlyInit);
LOCAL STATUS rzg2UartBaseAddrGet (BOOL earlyInit, VIRT_ADDR * pBaseAddr);
LOCAL STATUS rzg2DebugWrite      (char * buffer, size_t len);
LOCAL void   rzg2DebugWriteCh      (char ch);
LOCAL void   rzg2Core0Exit     (UINT32 apCore);

#ifdef _WRS_CONFIG_SMP
LOCAL void   rzg2PspExit (UINT32 apCore);
#endif /*_WRS_CONFIG_SMP*/

/*******************************************************************************
*
* rzg2Probe - determine if PSL supports board described by device tree
*
* This function compares <boardCompatStr> with the compatibility string at
* the root of the device tree to determine if the PSL supports the board.
*
* This function is called in the context of usrInit().
*
* NOTE: only the first compatibility string in the FDT will be checked.
*
* RETURNS: TRUE if PSL supports board described by device tree, FALSE otherwise.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

BOOL rzg2Probe
    (
    char * boardCompatStr   /* board compatibility string to check */
    )
    {
    int    offset;

    offset = vxFdtPathOffset ("/");

    if (offset < 0)
        {
        /* badly formed device tree - can't find root node */

        return FALSE;
        }

    return (vxFdtNodeCheckCompatible (offset, boardCompatStr) == 0);
    }

/*******************************************************************************
*
* rzg2EarlyInit - early board initialisation
*
* This function initialises sysPhysMemDesc from the memory description in the
* device tree.
*
* If the BSP sets the BOARD_DESC_FLAG_DBG the debug console is initialised.
*
* This function is called in the context of usrInit() and runs before MMU and
* caches are enabled.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

void rzg2EarlyInit (void)
    {
    
    UINT state = MMU_ATTR_VALID | MMU_ATTR_SUP_RWX |
                 MMU_ATTR_SPL_3 | MMU_ATTR_CACHE_COPYBACK
#ifdef _WRS_CONFIG_SMP
                 | MMU_ATTR_CACHE_COHERENCY
#endif
                 ;

    /* reset debug UART base virtual address */

    rzg2UartBase = 0;

    /* initialise the generic timer frequency */

    rzg2GenTimerFreq = rzg2CounterFreqGet();

#ifdef DEBUG_EARLY_PRINT
    /* initialise the debug console */

    if (boardFlagCheck (BOARD_DESC_FLAG_DBG, FALSE))
        {
        rzg2EarlyDebugInit ();
        }
#endif /* DEBUG_EARLY_PRINT */

    /* read the RZ/G2 product register */

    rzg2ProductRegRead ();

    /* initialise sysPhysMemDesc using the memory description */

    sysPhysMemDescNumEnt = vxFdtPhysMemInfoGet (sysPhysMemDesc,
                                                sysPhysMemDescNumEnt,
                                                state);

    if (sysPhysMemDescNumEnt == 0)
        {
        RZG2_LIB_DBG_MSG ("No RAM entries in sysMemDesc - check device tree\n");
        }

#ifdef _WRS_CONFIG_ARM_ARMV8A	 	 
    mmuPtMemBase = KERNEL_SYS_MEM_RGN_BASE - sysPhysMemDesc[0].physicalAddr;       
#endif
    }

#ifdef DEBUG_EARLY_PRINT

/*******************************************************************************
*
* rzg2EarlyDebugInit - initialise the debug console (pre MMU initialisation)
*
* This function finds debug console UART device, maps the registers into
* kernel address space and initialises the kprintf and kwrite hooks.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2EarlyDebugInit (void)
    {
    rzg2DebugInit (TRUE);
    }

#endif /* DEBUG_EARLY_PRINT */

/*******************************************************************************
*
* rzg2ProductIdGet - read the processor product register
*
* This function reads the processor product register and stores it for later use.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2ProductRegRead (void)
    {

    int          offset;
    VIRT_ADDR    virAddr;
    PHYS_ADDR    phyAddr;
    size_t       barSize;

    /* find the product-register node */

    offset = vxFdtNodeOffsetByCompatible (0, RZG2_PRODCUT_NODE);

    if (offset < 0)
       {
       rzg2ProductId = 0;
       return;
       }

    /* get the base address */

    if (vxFdtDefRegGet (offset, 0, &phyAddr, &barSize) != OK)
       {
       rzg2ProductId = 0;
       return;
       }

    virAddr = vxMmuEarlyRegMap (phyAddr, PAGE_SIZE);

    if (virAddr == 0)
        {
        rzg2ProductId = 0;
        }
    else
        {
        rzg2ProductId = RZ_G2_PRR(virAddr);
        }

    return;
    }

/*******************************************************************************
*
* rzg2ProductIdGet - get the processor product identifier
*
* This function returns processor product identifier. A value of 0 indicates
* the identifier could not be read.
*
* RETURNS: the processor product identifier.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

UINT32 rzg2ProductIdGet (void)
    {
    return rzg2ProductId;
    }

/*******************************************************************************
*
* rcarH3ProductIdGet - get the processor product identifier
*
* This function returns processor product identifier. A value of 0 indicates
* the identifier could not be read.
*
* RETURNS: the processor product identifier
*
* ERRNO: N/A
*/

UINT32 rcarH3ProductIdGet (void)
    {
    return rzg2ProductId;
    }

/*******************************************************************************
*
* rzg2ModelGet - get the board model description
*
* This function returns the board model description given in the device tree.
*
* RETURNS: A pointer to the board model description.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

char * rzg2ModelGet (void)
    {
    int    offset;
    char * model = RZG2_MODEL_DEFAULT;

    /* look for model property in root node of device tree */

    offset = vxFdtPathOffset ("/");

    if (offset >= 0)
        {
        model = (char *)vxFdtPropGet (offset, "model", NULL);

        if (model == NULL)
            {
            /* model property not provided - use the default */

            model = RZG2_MODEL_DEFAULT;
            }
        }

    return model;
    }

/*******************************************************************************
*
* rzg2Init - initialise system hardware
*
* This function initialises various parts of the RZ/G2 hardware that are not
* explicitly managed by device drivers, or that require specific initialisation
* to allow board initialisation to proceed.
*
* This function is called in the context of usrRoot() after MMU and kernel
* memory have been initialised.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

void rzg2Init (void)
    {
    /* initialise console debug */

    if (boardFlagCheck (BOARD_DESC_FLAG_DBG, FALSE))
        {
        rzg2KernelDebugInit ();
        }
    }

/*******************************************************************************
*
* rzg2KernelDebugInit - initialise the debug console (post MMU initialisation)
*
* This function finds debug console UART device, maps the registers into
* kernel address space and initialises the kprintf and kwrite hooks.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2KernelDebugInit (void)
    {
    rzg2DebugInit (FALSE);
    }

/*******************************************************************************
*
* rzg2DebugInit - initialise the debug console
*
* This function finds debug console UART device, maps the registers into
* kernel address space and initialises the kprintf and kwrite hooks.
*
* NOTE: it is assumed that clocks to the SCIF module and the initial baud rate
* have been set by the ARM trusted firmware or boot loader.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2DebugInit
    (
    BOOL earlyInit
    )
    {
    VIRT_ADDR virtAddr;

    /* get debug UART virtual address */

    if (rzg2UartBaseAddrGet (earlyInit, &virtAddr) != OK)
        {
        rzg2UartBase = 0;
        }
    else
        {
        rzg2UartBase = virtAddr;

        /* enable the transmitter */

        SCIF_SCSCR_REG |= SCSCR_TE;

        /* initialise kernel debug hook functions */

        _func_kwrite   = rzg2DebugWrite;
        _func_kprintf  = kprintf;
        }
    }

/*******************************************************************************
*
* rzg2UartBaseAddrGet - get the base address of the debug UART
*
* This function gets the base address of the UART to use for debug output during
* system initialisation. The UART base virtual address is returned in
* <pBaseAddr>.
*
* If <earlyInit> is TRUE the virtual address will be valid only until the MMU
* is fully initialised by usrMmuInit.
*
* NOTE: currently this function will use the first SCIF UART found in the
* device tree.
*
* RETURNS: OK, or ERROR if it is not possible to get virtual address of UART
* register base.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2UartBaseAddrGet
    (
    BOOL        earlyInit,
    VIRT_ADDR * pBaseAddr
    )
    {
    int          offset;
    PHYS_ADDR    physAddr;
    size_t       barSize;

    /* find the first SCIF UART device */

    offset = vxFdtNodeOffsetByCompatible (0, RZG2_SCIF_NODE);

    if (offset < 0)
       {
       return ERROR;
       }

    /* get the UART base address */

    if (vxFdtDefRegGet (offset, 0, &physAddr, &barSize) != OK)
       {
       return ERROR;
       }

    /* get register virtual address based on state of MMU */

    if (earlyInit)
        {
        VIRT_ADDR virtAddr = vxMmuEarlyRegMap (physAddr, PAGE_SIZE);

        if (virtAddr == 0)
            {
            return ERROR;
            }

        *pBaseAddr = virtAddr;
        }
    else
        {
        /* after usrMmuInit() has been called */

        void * virtAddr = pmapGlobalMap (physAddr, (size_t)PAGE_SIZE,
                                         RZ_G2_DEVICE_MMU_ATTR);

        if (virtAddr == PMAP_FAILED)
            {
            return ERROR;
            }

        *pBaseAddr = (VIRT_ADDR)virtAddr;
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2DebugWrite - write to the debug console
*
* This function outputs the contents of <buffer> to the debug console using a
* polled output method.
*
* RETURNS: OK, or ERROR if the debug console is not initialised.
*
* ERRNO: N/A.
*/

LOCAL STATUS rzg2DebugWrite
    (
    char * buffer,
    size_t len
    )
    {
    unsigned i;
    unsigned txPollCount = 0;

    if (rzg2UartBase == 0 || buffer == NULL)
        {
        return ERROR;
        }

    /* send buffer contents to the debug console */

    for (i = 0; i < len; ++i, ++buffer)
        {
        rzg2DebugWriteCh (*buffer);

        /* insert carriage return after newline */

        if (*buffer == '\n')
            {
            rzg2DebugWriteCh ('\r');
            }
        }

    /* wait for end of transmission */

    while ((SCIF_SCFSR_REG & SCFSR_TEND) == 0 &&
           ++txPollCount < SCIF_TEND_POLL_COUNT_MAX)
        {
        rzg2UsDelay (SCIF_TX_POLL_DELAY_US);
        }

    return OK;
    }

/*******************************************************************************
*
* rzg2DebugWriteCh - write character to the debug console
*
* This function outputs <ch> to the debug console.
*
* NOTE: this function will busy-wait until there is space in the Tx FIFO.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2DebugWriteCh
    (
    char ch
    )
    {
    unsigned txPollCount = 0;

    UINT16 scfsrTxFlags = (SCFSR_TDFE | SCFSR_TEND |
                           SCFSR_PER  | SCFSR_FER) ;

    /* wait for space in the Tx FIFO */

    while ((SCIF_SCFSR_REG & SCFSR_TDFE) == 0 &&
           ++txPollCount < SCIF_TDFE_POLL_COUNT_MAX)
        {
        rzg2UsDelay (SCIF_TX_POLL_DELAY_US);
        }

    /* put the character into Tx FIFO and clear Tx event flags */

    SCIF_SCFTDR_REG = ch;
    SCIF_SCFSR_REG  = ~scfsrTxFlags;
    }

/*******************************************************************************
*
* rzg2UsDelay - delay (busy-wait) for a number of microseconds
*
* This function delays for the requested number of microseconds.
*
* NOTE: This function performs a busy-wait and does not relinquish the CPU.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

void rzg2UsDelay
    (
    int delayUs   /* microseconds */
    )
    {
    volatile UINT64 oldTicks;
    volatile UINT64 newTicks;
    volatile UINT64 delayTicks;
    volatile UINT64 elapsedTicks = 0;

    /* confirm delay is non-zero number of microseconds */

    if (delayUs <= 0)
        {
        return;
        }

    /* convert delay period to counter ticks */

    delayTicks = ((rzg2GenTimerFreq * (UINT64)delayUs) + (1000000 - 1)) / 1000000;

    /*
     * Repeatedly read the counter until the elapsed number of ticks is greater
     * than the delay period.
     */

    oldTicks = rzg2CounterValueGet ();

    while (elapsedTicks <= delayTicks)
        {
        newTicks = rzg2CounterValueGet ();

        if (newTicks >= oldTicks)
            {
            elapsedTicks += newTicks - oldTicks;
            }
        else
            {
            elapsedTicks += 0xffffffffffffffffUL - oldTicks + newTicks + 1;
            }

        oldTicks = newTicks;
        }
    }

/*******************************************************************************
*
* __inline__Rzg2GetCntFreq - get the ARM core counter frequency
*
* This routine gets the ARM core counter frequency from the CNTFRQ system
* register.
*
* RETURNS: the ARM core counter frequency.
*
* ERRNO: N/A.
*/

#ifdef _WRS_CONFIG_LP64

UINT32 __inline__Rzg2GetCntFreq (void)
    {
    UINT64  tempVal;
    WRS_ASM ("MRS %0, CNTFRQ_EL0"
              : "=r" (tempVal)
              : );
    return (UINT32)tempVal;
    }

#else /* !_WRS_CONFIG_LP64 */

UINT32 __inline__Rzg2GetCntFreq (void)
    {
    UINT32  tempVal;
    WRS_ASM ("MRC p15, 0, %0, c14, c0, 0"
              : "=r"(tempVal)
              : );

    return tempVal;
    }

#endif /* _WRS_CONFIG_LP64 */

/*******************************************************************************
*
* rzg2CounterFreqGet - get free-running counter frequency
*
* This function gets the ARM core generic timer counter frequency.
*
* RETURNS: counter frequency (in Hz).
*
* ERRNO: N/A.
*/

LOCAL UINT32 rzg2CounterFreqGet  (void)
    {
    return __inline__Rzg2GetCntFreq ();
    }

/*******************************************************************************
*
* __inline__Rzg2GetVirtTimerCnt - get the ARM core counter value
*
* This routine gets the ARM core virtual counter frequency from the
* CNTVCT system register.
*
* RETURNS: the ARM core counter frequency.
*
* ERRNO: N/A.
*/

#ifdef _WRS_CONFIG_LP64

LOCAL UINT64 __inline__Rzg2GetVirtTimerCnt (void)
    {
    UINT64  tempVal;
    WRS_ASM ("MRS %0, CNTVCT_EL0"
              : "=r" (tempVal)
              : );
    return tempVal;
    }

#else /* !_WRS_CONFIG_LP64 */

LOCAL UINT64 __inline__Rzg2GetVirtTimerCnt (void)
    {
    UINT64  tempVal;
    WRS_ASM ("MRRC p15, 1, %Q0, %R0, c14"
              : "=r" (tempVal)
              : );
    return tempVal;
    }

#endif /* _WRS_CONFIG_LP64 */

/*******************************************************************************
*
* rzg2GenTimerCountGet - get free-running counter value
*
* This function gets the ARM core generic timer virtual counter value.
*
* The virtual counter rather than the physical counter is used because the
* the virtual counter is always accessible to software running at non-secure
* EL0. Access to the physical counter depends on the configuration of
* system register CNTHCTL so access is not guaranteed.
*
* RETURNS: Counter value.
*
* ERRNO: N/A.
*/

LOCAL UINT64 rzg2CounterValueGet (void)
    {
    /*
     * Reads from CNTVCT can occur speculatively and out of order relative
     * to other instructions. The ISB ensures that the CNTVCT is read after
     * preceding instructions.
     */

    WRS_ASM ("ISB");

    return __inline__Rzg2GetVirtTimerCnt();
    }

/*******************************************************************************
*
* rzg2Reset - system reset
*
* This function resets the board. It is usually called
* only by reboot() -- which services ^X -- and aborts at interrupt
* level.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

void rzg2Reset
    (
    int startType
    )
    {

    if ((sysWarmBootFunc == NULL) ||(startType == BOOT_CLEAR))
        {
        goto cold;
        }

#ifdef _WRS_CONFIG_SMP
    armSysToMonitor ((FUNCPTR)rzg2Core0Exit, (FUNCPTR)rzg2PspExit, startType);
#else
    armSysToMonitor ((FUNCPTR)rzg2Core0Exit, NULL, startType);
#endif

cold:
    RZG2_LIB_DBG_MSG ("Try cold boot\n");

    /* now reset */

    vxPsciSysRst ();

    RZG2_LIB_DBG_MSG ("Cold boot failed - enter busy wait\n");
    }

/*******************************************************************************
*
* rzg2Core0Exit - core zero's shutdown function
*
* This function disables the GIC before system shutdown.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2Core0Exit
    (
    UINT32 apCore
    )
    {
    /* mask interrupt */

    (void) intCpuLock();

    vxCpuOffHooksInvoke (apCore);
    }

#ifdef _WRS_CONFIG_SMP

/*******************************************************************************
*
* rzg2PspExit - secondary cores exit function
*
* This function is the exit function for secondary cores. It deactivates the
* active interrupt, clears all remaining pending interrupt and then disables the
* interrupt controller.
*
* RETURNS: N/A.
*
* ERRNO: N/A.
*/

LOCAL void rzg2PspExit
    (
    UINT32 cpuId
    )
    {
    /* mask interrupt */

    (void) intCpuLock();

    vxCpuOffHooksInvoke (cpuId);
    }

/*******************************************************************************
*
* rzg2CpuEnable - enable a secondary CPU core
*
* This function brings the secondary CPU identified by <cpuId> out of reset.
*
* RETURNS: OK, or ERROR if the CPU is not started or is already running.
*
* ERRNO: N/A.
*
* \NOMANUAL
*/

STATUS rzg2CpuEnable
    (
    unsigned         cpuId,
    WIND_CPU_STATE * cpuState
    )
    {
    UINT32    cpuPhyId;
    PHYS_ADDR physAddr;
    STATUS    stat;

    if (cpuId >= vxFdtCpuAvail ())
        {
        return ERROR;
        }

    RZG2_LIB_DBG_MSG ("CPU %d enable - begin\n", cpuId);

    /* initialise secondary core initialisation arguments */

    if (armMpCoreInit ((UINT32)cpuId, cpuState, NULL) != OK)
        {
        return ERROR;
        }

    if (sysStartType == BOOT_COLD)
        {
        stat = vmTranslate (NULL, (VIRT_ADDR)sysInit, &physAddr);
        if (stat == ERROR)
            {
            RZG2_LIB_DBG_MSG ("%s vmTranslate failed for cpuId:%d\n",
                              __func__, cpuId);
            return ERROR;
            }

        cpuPhyId = vxCpuIdGetByIndex (cpuId);

        RZG2_LIB_DBG_MSG ("CPU id 0x%d, CPU phy id 0x%x\n", cpuId, cpuPhyId);
        
#ifdef _WRS_CONFIG_LP64
        if (vxPsciCpuOn((SMC_SZ) cpuPhyId, physAddr, NULL) == PSCI_NOT_SUPPORTED)
            {
            return ERROR;
            }
#else /* !_WRS_CONFIG_LP64 */
        if (vxPsciCpuOn(cpuPhyId, ((UINT32) physAddr) + 4, NULL) == PSCI_NOT_SUPPORTED)
            {
            return ERROR;
            }
#endif /* _WRS_CONFIG_LP64 */
    
            return OK;
        }
    else
        {
        return armMonitorSpinRelease (cpuId);
        }
    }

#endif /* _WRS_CONFIG_SMP */
