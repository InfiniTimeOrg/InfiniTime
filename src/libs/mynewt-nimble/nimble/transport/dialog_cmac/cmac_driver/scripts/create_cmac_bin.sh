#!/bin/bash

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

OBJCOPY=${MYNEWT_OBJCOPY_PATH}
ELF=${MYNEWT_APP_BIN_DIR}/blehci.elf

cd ${WORK_DIR}

# Strip .ram section from ROM image
${OBJCOPY} -R .ram -O binary ${ELF} ${BASENAME_ROM}.bin
# RAM image is the same as binary created by newt
cp ${ELF}.bin ${BASENAME_RAM}.bin

# Create a copy of ROM image to flash to partition, if required
cp ${BASENAME_ROM}.bin ${ELF}.rom.bin
