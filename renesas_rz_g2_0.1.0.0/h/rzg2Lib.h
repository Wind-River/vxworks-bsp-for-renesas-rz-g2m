/* rzg2Lib.h - Renesas RZ/G2 processor support library header */

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

#ifndef __INCrzg2Libh
#define __INCrzg2Libh

#include <vxWorks.h>

#ifdef __cplusplus
extern "C" {
#endif

/* defines */

/* product Register definitions */

#define RZG2_M                    0x52U
#define RZ_G2_PRODUCT(id)         (((id) & 0x7F00U) >> 8)
#define RZ_G2_CUT(id)             (((id) & 0x00FFU))

/* function declarations */

IMPORT BOOL    rzg2Probe         (char * boardCompatibility);
IMPORT void    rzg2Init          (void);
IMPORT void    rzg2EarlyInit     (void);
IMPORT char *  rzg2ModelGet      (void);
IMPORT void    rzg2UsDelay       (int delay);
IMPORT void    rzg2Reset         (int startType);

#ifdef _WRS_CONFIG_SMP
IMPORT STATUS  rzg2CpuEnable     (unsigned cpuId, WIND_CPU_STATE * cpuState);
#endif

IMPORT UINT32  rzg2ProductIdGet (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCrzg2Libh */
