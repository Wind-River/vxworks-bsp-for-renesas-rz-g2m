VxWorks® 7 Community BSP for Renesas RZ/G2M
===
---

# Overview

This document describes the features of the renesas_rz_g2 BSP/PSL, which is designed
to run on the HIHOPE RZ/G2(M) V2 board. This is an ARM Cortex-A57 and Cortex-A53
processor-based platform.

# Project License

Copyright (c) 2019 Wind River Systems, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1) Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2) Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3) Neither the name of Wind River Systems nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

# Prerequisite(s)

* You must have Wind River® VxWorks® 7 SR0640 released source code and
  development environment to support "Renesas RZ/G2M Community BSP".

# Building and Using

### Preparation

IMPORTANT: Your existing VxWorks 7 installation may already contains the 
renesas_rz_g2m BSP and renesas_rz_g2 PSL, please check the following installation 
location to see whether the BSP and PSL existed already. 
The renesas_rz_g2m BSP is installed at:
```Bash
***installDir***/vxworks-7/pkgs_v2/unsupported/renesas_rz_g2/renesas_rz_g2m-W.X.Y.Z
```
The renesas_rz_g2 PSL is installed at:
```Bash
***installDir***/vxworks-7/pkgs_v2/unsupported/renesas_rz_g2/renesas_rz_g2-W.X.Y.Z
```
If the installed version of BSP/PSL is the same or newer than the Open Source BSP 
published here, there's no need to download and install the Open Source BSP from 
GitHub again.

If your existing VxWorks 7 installation contains older version of renesas_rz_g2m BSP 
and renesas_rz_g2 PSL, it's recommended to download the latest code from GitHub and 
then install.

### Download

Download all layers from Github.
```Bash
git clone https://github.com/Wind-River/Renesas-RZ-G2M.git
cd Renesas-RZ-G2M
```

### Installation

There are two ways to install this BSP: inside existing VxWorks 7 Installation or outside
existing VxWorks 7 Installation.

#### Install into the source tree

All layers in this BSP go to their respective destination among the existing installation. 
The advantage is the BSP will always be accessible once you complete the installation. The 
disadvantage is you can't shut down this BSP unless you manually delete all the installed 
layers among the source tree.

Here's how it’s done:

```Bash
cp -r renesas_rz_g2m-W.X.Y.Z ***installDir***/vxworks-7/pkgs_v2/unsupported/renesas_rz_g2/
cp -r renesas_rz_g2-W.X.Y.Z ***installDir***/vxworks-7/pkgs_v2/unsupported/renesas_rz_g2/
```

#### Install beside the source tree

All layers are copied in a place that's outside the default scanning folder, i.e., 
vxworks-7/pkgs_v2, and when launching the Development Shell or Workbench, the path containing 
this BSP is provided to the development environment. The advantage of this method is obvious, 
the package can be easily turn on or off, and the source code stays in one unified location 
external to the default installation, which makes it easier to manage.

Suppose all packages are in /home/renesas_rz_g2/, then do the following when launching wrenv
or Workbench:

```Bash
export WIND_LAYER_PATHS=/home/renesas_rz_g2
export WIND_BSP_PATHS=/home/renesas_rz_g2
```
then enter into the existing VxWorks 7 installation directory
```Bash
cd ***installDir***
```
if use the Development Shell
```Bash
./wrenv.sh -p vxworks-7
```
or Workbench
```Bash
./workbench-4/startWorkbench.sh
```

### Building

The building process comprises three parts, U-Boot, VSB project and VIP project.
You can get the U-Boot code from related official server. The building processes of these
three parts are all described in target.txt. Target.txt can be found under
renesas_rz_g2m-W.X.Y.Z directory.

### Drivers

By now, the support of drivers in this Open Source BSP is as below:

Hardware Interface | Controller       | Driver/Component           | Status
--------------------------------------------------------------------------------
Mini UART          | on-chip          | DRV_SIO_FDT_SCIF           | SUPPORTED
ARM Generic Timer  | on-chip          | DRV_ARM_GEN_TIMER          | SUPPORTED
TMU                | on-chip          | DRV_TIMER_FDT_TMU          | SUPPORTED
ARM GIC            | on-chip          | DRV_INTCTLR_FDT_ARM_GIC    | SUPPORTED
CPG MSSR           | on-chip          | DRV_CLK_FDT_RZG2_CPG_MSSR  | SUPPORTED
PinMux             | on-chip          | DRV_PINMUX_FDT_RZG2_PFC    | SUPPORTED
PCIe               | on-chip          | DRV_FDT_RCAR_H3_PCIE       | SUPPORTED
GPIO               | on-chip          | DRV_RCAR_H3_GPIO           | SUPPORTED
I2C                | on-chip          | DRV_RCAR_H3_I2C            | SUPPORTED
ETHERNET           | on-chip          | DRV_END_FDT_ETHERAVB       | SUPPORTED
Watchdog Module    | on-chip          | DRV_FDT_RS_WATCHDOG        | SUPPORTED
SD/eMMC            | on-chip          | DRV_RCAR_H3_MMC            | SUPPORTED
USB (USB3.0)       | on-chip          | USB Host Stack 3.0 (XHCI)  | SUPPORTED
USB (USB2.0)       | on-chip          | USB Host Stack 2.0 (EHCI)  | SUPPORTED
HDMI               | on-chip          |                            | UNSUPPORTED
SPI                | on-chip          |                            | UNSUPPORTED
QSPI               | on-chip          |                            | UNSUPPORTED
QSPI FLASH         | W25M512JWX_TFBGA |                            | UNSUPPORTED
SATA               | on-chip          |                            | UNSUPPORTED
CAN                | on-chip          |                            | UNSUPPORTED
LVDS               | on-chip          |                            | UNSUPPORTED

The detailed introduction of these drivers and usage can also be found in target.txt.

# Legal Notices

All product names, logos, and brands are property of their respective owners. All company, product 
and service names used in this software are for identification purposes only. Wind River and VxWorks 
are registered trademarks of Wind River Systems.

Disclaimer of Warranty / No Support: Wind River does not provide support and maintenance services 
for this software, under Wind River’s standard Software Support and Maintenance Agreement or otherwise. 
Unless required by applicable law, Wind River provides the software (and each contributor provides its 
contribution) on an “AS IS” BASIS, WITHOUT WARRANTIES OF ANY KIND, either express or implied, including, 
without limitation, any warranties of TITLE, NONINFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR 
PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the 
software and assume ay risks associated with your exercise of permissions under the license.

This BSP is not covered by Wind River's standard support. Should you need support, please work with 
the community, or should you need commercial support, please contact your local Wind River Sales Representative.
