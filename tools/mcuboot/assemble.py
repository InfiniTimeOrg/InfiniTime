#! /usr/bin/env python3
#
# Copyright 2017 Linaro Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Assemble multiple images into a single image that can be flashed on the device.
"""

import argparse
import errno
import io
import re
import os.path
import sys

ZEPHYR_BASE = os.getenv("ZEPHYR_BASE")
if not ZEPHYR_BASE:
    sys.exit("$ZEPHYR_BASE environment variable undefined")

sys.path.insert(0, os.path.join(ZEPHYR_BASE, "scripts", "dts"))
import edtlib

def same_keys(a, b):
    """Determine if the dicts a and b have the same keys in them"""
    for ak in a.keys():
        if ak not in b:
            return False
    for bk in b.keys():
        if bk not in a:
            return False
    return True

offset_re = re.compile(r"^#define DT_FLASH_AREA_([0-9A-Z_]+)_OFFSET(_0)?\s+(0x[0-9a-fA-F]+|[0-9]+)$")
size_re   = re.compile(r"^#define DT_FLASH_AREA_([0-9A-Z_]+)_SIZE(_0)?\s+(0x[0-9a-fA-F]+|[0-9]+)$")

class Assembly():
    def __init__(self, output, bootdir, edt):
        self.find_slots(edt)
        try:
            os.unlink(output)
        except OSError as e:
            if e.errno != errno.ENOENT:
                raise
        self.output = output

    def find_slots(self, edt):
        offsets = {}
        sizes = {}

        part_nodes = edt.compat2nodes["fixed-partitions"]
        for node in part_nodes:
            for child in node.children.values():
                if "label" in child.props:
                    label = child.props["label"].val
                    offsets[label] = child.regs[0].addr
                    sizes[label] = child.regs[0].size

        if not same_keys(offsets, sizes):
            raise Exception("Inconsistent data in devicetree.h")

        # We care about the mcuboot, image-0, and image-1 partitions.
        if 'mcuboot' not in offsets:
            raise Exception("Board partition table does not have mcuboot partition")

        if 'image-0' not in offsets:
            raise Exception("Board partition table does not have image-0 partition")

        if 'image-1' not in offsets:
            raise Exception("Board partition table does not have image-1 partition")

        self.offsets = offsets
        self.sizes = sizes

    def add_image(self, source, partition):
        with open(self.output, 'ab') as ofd:
            pos = ofd.tell()
            print("partition {}, pos={}, offset={}".format(partition, pos, self.offsets[partition]))
            if pos > self.offsets[partition]:
                raise Exception("Partitions not in order, unsupported")
            if pos < self.offsets[partition]:
                buf = b'\xFF' * (self.offsets[partition] - pos)
                ofd.write(buf)
            with open(source, 'rb') as rfd:
                ibuf = rfd.read()
                if len(ibuf) > self.sizes[partition]:
                    raise Exception("Image {} is too large for partition".format(source))
            ofd.write(ibuf)

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument('-b', '--bootdir', required=True,
                        help='Directory of built bootloader')
    parser.add_argument('-p', '--primary', required=True,
                        help='Signed image file for primary image')
    parser.add_argument('-s', '--secondary',
                        help='Signed image file for secondary image')
    parser.add_argument('-o', '--output', required=True,
                        help='Filename to write full image to')

    args = parser.parse_args()

    # Extract board name from path
    board = os.path.split(os.path.split(args.bootdir)[0])[1]

    dts_path = os.path.join(args.bootdir, "zephyr", board + ".dts.pre.tmp")

    edt = edtlib.EDT(dts_path, [os.path.join(ZEPHYR_BASE, "dts", "bindings")],
                     warn_reg_unit_address_mismatch=False)

    output = Assembly(args.output, args.bootdir, edt)

    output.add_image(os.path.join(args.bootdir, 'zephyr', 'zephyr.bin'), 'mcuboot')
    output.add_image(args.primary, "image-0")
    if args.secondary is not None:
        output.add_image(args.secondary, "image-1")

if __name__ == '__main__':
    main()
