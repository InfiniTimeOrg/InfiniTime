#!/usr/bin/env bash
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
#

if [ ! -f "project.yml" ]; then
    echo "This script should be executed from project root directory"
    exit 1
fi

declare -A targets=(
    ["nuttx"]="repos/apache-mynewt-nimble/porting/examples/nuttx/"
    ["linux"]="repos/apache-mynewt-nimble/porting/examples/linux/"
    ["linux_blemesh"]="repos/apache-mynewt-nimble/porting/examples/linux_blemesh/"
    ["porting_default"]="repos/apache-mynewt-nimble/porting/nimble"
    ["riot"]="repos/apache-mynewt-nimble/porting/npl/riot/"
)

for target in "${!targets[@]}"; do
    echo "Updating target $target"
    newt build "@apache-mynewt-nimble/porting/targets/$target" > /dev/null 2>&1
    cp "bin/@apache-mynewt-nimble/porting/targets/${target}/generated/include" "${targets[$target]}" -r
    # Remove repo version and hash MYNEWT_VALS as it doesn't make much sense to commit them and they
    # defeat the purpose of this script.
    find "${targets[$target]}/include" -type f -name 'syscfg.h' -exec sed -i '/MYNEWT_VAL_REPO_*/,/#endif/d' {} \;
    find "${targets[$target]}/include" -type f -name 'syscfg.h' -exec sed -i '/\/\*\*\* Repository/,/\*\//d' {} \;
    find "${targets[$target]}/include" -type f -name 'syscfg.h' -exec sed -i '$!N;/^\n$/{$q;D;};P;D;' {} \;
done
