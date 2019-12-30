/* vxbFdtRsRzg2Rst.h - Renesas RZ/G2 reset controller header file */

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

#ifndef __INCvxbFdtRsRzg2Rsth
#define __INCvxbFdtRsRzg2Rsth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* defines */

#define FDT_RST_COMPATIBLE  "renesas,r8a774a1-rst"

/* reset (RST) register offsets */

#define MODEMR          0x0060U     /* Mode Monitor Register */
#define CA57RESCNT      0x0040U     /* CA57 Reset Control Register */
#define CA53RESCNT      0x0044U     /* CA53 Reset Control Register */
#define WDTRSTCR        0x0054U     /* Watchdog Timer Reset Control Register */
#define RSTOUTCR        0x0058U     /* PRESETOUT# Control Register */
#define SRESCR          0x0110U     /* Soft Power On Reset Control Register */
#define RRSTFR          0x0114U     /* RT Reset Flag Register */
#define SRSTFR          0x0118U     /* SYS Reset Flag Register */
#define CR7 CR          0x01F0U     /* CR7 Control Register */

/* reset (RST) register values */

#define MODEMR_MDT1     (1U << 30)
#define MODEMR_MDT0     (1U << 29)
#define MODEMR_MD(n)    (1U << n)   /* For MD[0:28] */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCvxbFdtRsRzg2Rsth */
