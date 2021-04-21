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

NEWT=${MYNEWT_NEWT_PATH}
OBJCOPY=${MYNEWT_OBJCOPY_PATH}
AR=${MYNEWT_AR_PATH}
LIBCMAC_A=${MYNEWT_USER_SRC_DIR}/libcmac.a

export WORK_DIR=${MYNEWT_USER_WORK_DIR}
export BASENAME_ROM=cmac.rom
export BASENAME_RAM=cmac.ram

if [ ${MYNEWT_VAL_CMAC_IMAGE_SINGLE} -eq 0 ]; then
    # Create empty binary for ROM image (1 byte required for objcopy)
    truncate -s 1 ${WORK_DIR}/${BASENAME_ROM}.bin
    # Create fixed size RAM image
    truncate -s ${MYNEWT_VAL_CMAC_IMAGE_RAM_SIZE} ${WORK_DIR}/${BASENAME_RAM}.bin
else
    ${NEWT} build ${MYNEWT_VAL_CMAC_IMAGE_TARGET_NAME}
fi

cd ${WORK_DIR}

# Convert both binaries to objects and create archive to link
${OBJCOPY} -I binary -O elf32-littlearm -B armv8-m.main \
    --rename-section .data=.libcmac.rom ${BASENAME_ROM}.bin ${BASENAME_ROM}.o
${OBJCOPY} -I binary -O elf32-littlearm -B armv8-m.main \
    --rename-section .data=.libcmac.ram ${BASENAME_RAM}.bin ${BASENAME_RAM}.o
${AR} -rcs ${LIBCMAC_A} ${BASENAME_ROM}.o ${BASENAME_RAM}.o
