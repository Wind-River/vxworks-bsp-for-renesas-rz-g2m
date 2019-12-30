/* 20bsp.cdf - BSP component description file */
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
 *
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
This file contains descriptions for this BSP components.
*/

/*
 * VX_SMP_NUM_CPUS is a SMP parameter only and only available for SMP
 * builds. Due to a limitation of the project tool at the time this
 * parameter is created where the tool can not recognize the ifdef SMP
 * selection, this parameter is set up such that _CFG_PARAMS is not
 * specified here. In the 00vxWorks.cdf file, where the parameter
 * VX_SMP_NUM_CPUS is defined, the _CFG_PARAMS is specified only for
 * VxWorks SMP. Hence the redefining of VX_SMP_NUM_CPUS here should only
 * override the value and not the rest of the properties. And for UP, the
 * parameter is ignored since the parameter is not tied to any component
 * (_CFG_PARAMS is not specified).
 */

Parameter VX_SMP_NUM_CPUS {
    NAME        Number of CPUs to be enabled for SMP
    SYNOPSIS    This parameter determines the number of CPUs to be enabled for \
                SMP. This value must be greater than zero.
    TYPE        UINT
    DEFAULT     6
}

Selection BOARD_SELECTION {
    NAME        Board selection
    SYNOPSIS    This BSP supports multiple boards. Choose your desired hardware \
                from this list.
    COUNT       1-1
    CHILDREN    INCLUDE_RENESAS_RZG2M
    DEFAULTS    INCLUDE_RENESAS_RZG2M
    _CHILDREN   FOLDER_BSP_CONFIG
}

Parameter DTS_FILE {
    NAME        DTS file name to be used
    SYNOPSIS    This parameter determines which dts file is used.
    DEFAULT     (INCLUDE_RENESAS_RZG2M)::(r8a774a1-hihope-rzg2m.dts)
}

Component INCLUDE_RENESAS_RZG2M {
    NAME        RENESAS RZ/G2M board support
    SYNOPSIS    This component configures your project to support the \
                RENESAS RZ/G2M board.
}

Parameter CONSOLE_TTY {
    NAME        Console serial port
    SYNOPSIS    This parameter determines which console serial port is used.
    DEFAULT     0
}

Parameter NUM_TTY {
     NAME       Number of serial ports
     DEFAULT    2
}

Parameter CONSOLE_BAUD_RATE {
    NAME        Baud rate of console port
    DEFAULT     115200
}

Parameter RAM_LOW_ADRS {
    NAME        Run-time kernel entry address
    SYNOPSIS    This parameter specifies the RAM address which is the entry \
                point of VxWorks.
#ifdef _WRS_CONFIG_LP64
    DEFAULT     (INCLUDE_BOOT_APP)::(0xffffffff82000000) \
                0xffffffff80100000
#else
    DEFAULT     (INCLUDE_BOOT_APP)::(0x02000000) \
                0x00100000
#endif /* _WRS_CONFIG_LP64 */
}

Parameter RAM_HIGH_ADRS {
    NAME        Run-time kernel high address
    SYNOPSIS    This parameter specifies the RAM address which is the entry \
                point of the boot application.
#ifdef _WRS_CONFIG_LP64
    DEFAULT     (INCLUDE_BOOT_APP)::(0xffffffff82800000) \
                0xffffffff81000000
#else
    DEFAULT     (INCLUDE_BOOT_APP)::(0x04000000) \
                0x02000000
#endif /* _WRS_CONFIG_LP64 */
}

Parameter LOCAL_MEM_LOCAL_ADRS {
    NAME        System memory start address
    SYNOPSIS    This parameter specifies the virtual base address for memory.
#ifdef _WRS_CONFIG_LP64
    DEFAULT     0xffffffff80000000
#else
    DEFAULT     0x00000000
#endif /* _WRS_CONFIG_LP64 */
}

Parameter KERNEL_LOAD_ADRS {
    NAME        Run-time kernel load address
    SYNOPSIS    This parameter specifies the physical address for the VxWorks \
                image entry point.
    DEFAULT     (INCLUDE_BOOT_APP)::(0x4a000000) \
                0x48100000
}

Parameter DTB_RELOC_ADDR {
    NAME        Device tree blob (DTB) relocation address
    SYNOPSIS    The DTB must be relocated to a safe address so that it is not \
                overwritten. This address should be below RAM_LOW_ADRS and the \
                reserved start region. This address must allow sufficient space\
                for the entire DTB.
    TYPE        void *
    DEFAULT     (LOCAL_MEM_LOCAL_ADRS + 0x10000)
}

Parameter LOCAL_MEM_PHYS_ADRS {
    NAME        Local memory physical base address
    SYNOPSIS    This parameter specifies the physical base address for memory.
    DEFAULT     0x48000000
}

Parameter DTB_MAX_LEN {
    NAME        Device tree blob (DTB) maximum length
    SYNOPSIS    This parameter defines the maximum length of the DTB. You \
                should take this length into account when setting the DTB \
                relocation address. This value must equal to or greater   \
                than GUEST_FDT_SIZE if vxWorks is a guest OS of WRHV.
    TYPE        int
    DEFAULT     0x20000
}

Parameter IMA_SIZE {
    NAME        Initial mapped area (IMA) size
    SYNOPSIS    IMA is mapped to the MMU during the early initialization phase \
                (before usrMmuInit()). Therefore, the size must be large enough\
                to accommodate the entire VxWorks kernel image.
    DEFAULT     0x10000000
}

Parameter ISR_STACK_SIZE {
    NAME        ISR stack size
    SYNOPSIS    This parameter defines the stack size in bytes for the \
                interrupt service routine (ISR).
    DEFAULT     0x2000
}

Parameter DEFAULT_BOOT_LINE {
    NAME        default boot line
    SYNOPSIS    This parameter is used as the default boot line string.
    TYPE        string
    DEFAULT     "etherAvb(0,0) host:vxWorks h=192.168.0.1 e=192.168.0.2 u=target pw=vxTarget"
}

Profile BSP_DEFAULT {
    NAME        Default BSP profile
    SYNOPSIS    Include default BSP components and parameters.
    PROFILES    PROFILE_OS_DEFAULT
    COMPONENTS  += INCLUDE_PINMUX_SYS \
                   INCLUDE_SIO
    PARAMETER_VALUES { XHCI_RCAR_ENABLE TRUE }
}

Parameter KERNEL_COMMON_HEAP_INIT_SIZE {
    NAME            Kernel common heap initial size
    SYNOPSIS        This parameter specifies the kernel common heap initial size.
    TYPE            ULONG
    DEFAULT         0x2000000
}

Component DRV_CLK_FDT_RZG2_CPG_MSSR {
    INCLUDE_WHEN    INCLUDE_DEVCLK_SYS
}

Component DRV_PINMUX_FDT_RZG2_PFC {
    INCLUDE_WHEN    INCLUDE_PINMUX_SYS
}

Component DRV_SIO_FDT_SCIF {
    INCLUDE_WHEN    INCLUDE_SIO
}

Component DRV_FDT_RCAR_H3_PCIE {
    INCLUDE_WHEN    INCLUDE_PCI_BUS
}

Component DRV_RCAR_H3_GPIO {
    INCLUDE_WHEN    INCLUDE_GPIO_SYS
}

Component DRV_END_FDT_ETHERAVB {
    REQUIRES +=     INCLUDE_REALTEK_PHY
    INCLUDE_WHEN    INCLUDE_END
}

Component DRV_AUXCLK_TIMER {
    REQUIRES        DRV_TIMER_FDT_TMU
    INCLUDE_WHEN    INCLUDE_VXB_AUX_CLK
    _CHILDREN       FOLDER_NOT_VISIBLE
}

Parameter AUXCLK_TIMER_NAME {
    NAME            Auxiliary Clock Device Name (NULL is auto-assign)
    SYNOPSIS        This parameter sets the auxiliary clock device. If the \
                    value is NULL, the device is auto-selected.
    TYPE            string
    DEFAULT         "tmu0"
}

Component DRV_SYSCLK_TIMER {
    REQUIRES        DRV_ARM_GEN_TIMER
    INCLUDE_WHEN    INCLUDE_TIMER_SYS
    _CHILDREN       FOLDER_NOT_VISIBLE
}

Parameter SYSCLK_TIMER_NAME {
    NAME            System clock device name (NULL is auto-assign)
    SYNOPSIS        This parameter sets the system clock device. If the \
                    value is NULL, the device is auto-selected.
    TYPE            string
    DEFAULT         "armGenTimer"
}

Component DRV_TIMESTAMP_TIMER {
    REQUIRES        DRV_ARM_GEN_TIMER
    INCLUDE_WHEN    INCLUDE_SYS_TIMESTAMP
    _CHILDREN       FOLDER_NOT_VISIBLE
}

Parameter TIMESTAMP_TIMER_NAME {
    NAME            Timestamp Device Name (NULL is auto-assign)
    SYNOPSIS        This parameter sets the timerstamp device. If the \
                    value is NULL, the device is auto-selected.
    TYPE            string
    DEFAULT         "armGenTimer"
}
