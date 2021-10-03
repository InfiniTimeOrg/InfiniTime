<!--
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
-->

## How to run NimBLE controller on Dialog DA1469x

Dialog DA1469x has separate Cortex-M0+ core inside CMAC hw block which can run
NimBLE controller. This means DA1469x can run full NimBLE stack: host is running
on M33 core while controller is running on M0+ core. Both communicate using
standard HCI H4 protocol exchanged over mailboxes located in shared memory.

### Basic setup

In order to run full NimBLE stack on DA1469x you will need two newt targets: one
for M33 (e.g. `dialog_da1469x-dk-pro` BSP) and one for M0+ (`dialog_cmac` BSP).

Once everything is configured properly, you only need to build target for M33.
Target configured for M0+ will be build automatically and image is linked with
M33 image so everything can be flashed at once just as if there is only single
target used.

Target for M33 should be set and configured as any other BLE application. In
order to use NimBLE controller on CMAC, set proper HCI transport via syscfg:

    BLE_HCI_TRANSPORT: dialog_cmac

This will include proper transport, driver and add M0+ target to build process.

For M0+, there is sample target provided in `targets/dialog_cmac` and it's used
by default unless overrided by syscfg in M33 target:

    CMAC_IMAGE_TARGET_NAME: "@apache-mynewt-nimble/targets/dialog_cmac"

If you wish to create own target for M0+, make sure your target is set the same
way (`app`, `bsp` and `build_profile`) as sample. Also it is recommended to use
syscfg settings from sample target in new target.

### NimBLE configuration

Since host and controller are running on different cores, they both use separate
configuration: host configuration is in M33 target, controller configuration is
in M0+ target. There is currently no way to automatically synchronize both, so
care needs to be taken when enabling features in either of targets.

A possible workaround is to use separate `.yml` file with all the NimBLE syscfg
values settings and include it in both targets using `$import` directive which
is supported by recent versions of `newt` tool.

### Advanced settings

(tbd)
