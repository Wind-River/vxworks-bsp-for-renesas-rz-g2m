# Makefile - make rules
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
# This file contains definitions for building VxWorks for the RZ/G2 family boards.
#
# INTERNAL
# This file should only contain rules specific to the BSP.  Definitions
# specific to this BSP should be placed in the defs file (defs.bsp.mk)
#

ifdef SUBPROJECT_DIR_ARM_HV_BSP
__WRVX_BSP_POST_BUILD_RULE += $(CP) $@ $@_swap ;    \
	$(OBJCPY) -I elf64-littleaarch64 -O elf32-littleaarch64 $@ $@.elf32; \
	$(ENV_BIN)image-tag -tag $@;
else
__WRVX_BSP_POST_BUILD_RULE += $(CP) $@ $@_swap ;    \
	$(OBJCPY) --change-section-lma .*-0x$(LOCAL_MEM_LOCAL_ADRS) --no-change-warnings $@ > /dev/null 2>&1; \
	$(OBJCPY) --adjust-start -0x$(LOCAL_MEM_LOCAL_ADRS) --no-change-warnings $@ > /dev/null 2>&1; \
	$(ENV_BIN)image-tag -tag $@;
endif
