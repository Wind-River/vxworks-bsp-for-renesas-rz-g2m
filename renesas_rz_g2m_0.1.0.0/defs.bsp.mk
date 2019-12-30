# defs.bsp.mk - make variable definitions
#
# Copyright (c) 2019 Wind River Systems, Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1) Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2) Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# 3) Neither the name of Wind River Systems nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# modification history
# --------------------
# 17sep19,hli  created (VXWPG-394)
#
# DESCRIPTION
# 
# This file contains definations for building VxWorks for the
# Renesas RZ/G2 family boards.
#

ifdef SUBPROJECT_DIR_ARM_HV_BSP
LD_OPTION_LOW_ENTRY_RAM = TRUE
LD_OPTION_VX_LOCORE_START = TRUE
LOAD_ADDR       = 0x0
LD_RAM_FLAGS    += --defsym _VX_LOCORE_START=0x$(KERNEL_LOAD_ADRS)
else
LOAD_ADDR       = 0x48000000
endif

ifdef _WRS_CONFIG_LP64
LD_LOCAL_MEM_PHYS_ADRS_MASK = 0x00000000ffffffff
else
LD_LOCAL_MEM_PHYS_ADRS_MASK = 0xffffffff
LOAD_ADDR       = 0x0
endif

# force to use flag 'arm64' instead of 'arm' for 64 bit U-Boot to load uVxworks
# built in ARMV7 32 bit compatible mode

ifndef  _WRS_CONFIG_LP64
MKIMAGE_FLAGS   = -A arm64
endif

DTC_FLAGS += -Wno_simple_bus_reg

# build binary target

vxWorks.bin:    vxWorks
		- @ $(RM) $@
		$(EXTRACT_BIN) vxWorks $@
