#!/usr/bin/env python

import sys

with open('jetbrains_mono_bold_20.c', 'r') as fd:
    src = fd.read()

zero_area = src.find('U+0030 "0"')
zero_data_start = src[zero_area:].find('\n') + zero_area + 1
zero_data_end = src[zero_data_start:].find('\n\n')

curr = src[zero_data_start:zero_data_start+zero_data_end]

OLD="""    0x3f, 0x1f, 0xef, 0x3f, 0x87, 0xe1, 0xf8, 0x7f,
    0xdf, 0xf7, 0xe1, 0xf8, 0x7e, 0x1f, 0xcf, 0x7f,
    0x8f, 0xc0,"""
NEW="""    0x3f, 0x1f, 0xef, 0x3f, 0x87, 0xe1, 0xf8, 0x7e,
    0xdf, 0xb7, 0xe1, 0xf8, 0x7e, 0x1f, 0xcf, 0x7f,
    0x8f, 0xc0,"""

if curr != OLD:
    if curr == NEW:
        print('Already patched')
        sys.exit()
    sys.exit('Target differers from expected data')


with open('jetbrains_mono_bold_20.c', 'r+') as fd:
    fd.seek(zero_data_start) #NOTE: since they are both the same size, this is allowed. Otherwise more editing needs to happen.
    fd.write(NEW)
