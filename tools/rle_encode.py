#!/usr/bin/env python3

# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2020 Daniel Thompson

import argparse
import sys
import os.path
from PIL import Image

def clut8_rgb888(i):
    """Reference CLUT for wasp-os.

    Technically speaking this is not a CLUT because the we lookup the colours
    algorithmically to avoid the cost of a genuine CLUT. The palette is
    designed to be fairly easy to generate algorithmically.

    The palette includes all 216 web-safe colours together 4 grays and
    36 additional colours that target "gaps" at the brighter end of the web
    safe set. There are 11 greys (plus black and white) although two are
    fairly close together.

    :param int i: Index (from 0..255 inclusive) into the CLUT
    :return:      24-bit colour in RGB888 format
    """
    if i < 216:
        rgb888  = ( i  % 6) * 0x33
        rg = i // 6
        rgb888 += (rg  % 6) * 0x3300
        rgb888 += (rg // 6) * 0x330000
    elif i < 252:
        i -= 216
        rgb888  =     0x7f + (( i  % 3) * 0x33)
        rg = i // 3
        rgb888 +=   0x4c00 + ((rg  % 4) * 0x3300)
        rgb888 += 0x7f0000 + ((rg // 4) * 0x330000)
    else:
        i -= 252
        rgb888 = 0x2c2c2c + (0x101010 * i)

    return rgb888

def clut8_rgb565(i):
    """RBG565 CLUT for wasp-os.

    This CLUT implements the same palette as :py:meth:`clut8_888` but
    outputs RGB565 pixels.

    .. note::

        This function is unused within this file but needs to be
        maintained alongside the reference clut so it is reproduced
        here.

    :param int i: Index (from 0..255 inclusive) into the CLUT
    :return:      16-bit colour in RGB565 format
    """
    if i < 216:
        rgb565  = (( i  % 6) * 0x33) >> 3
        rg = i // 6
        rgb565 += ((rg  % 6) * (0x33 << 3)) & 0x07e0
        rgb565 += ((rg // 6) * (0x33 << 8)) & 0xf800
    elif i < 252:
        i -= 216
        rgb565  = (0x7f + (( i  % 3) * 0x33)) >> 3
        rg = i // 3
        rgb565 += ((0x4c << 3) + ((rg  % 4) * (0x33 << 3))) & 0x07e0
        rgb565 += ((0x7f << 8) + ((rg // 4) * (0x33 << 8))) & 0xf800
    else:
        i -= 252
        gr6 = (0x2c + (0x10 * i)) >> 2
        gr5 = gr6 >> 1
        rgb565 = (gr5 << 11) + (gr6 << 5) + gr5

    return rgb565

class ReverseCLUT:
    def __init__(self, clut):
        l = []
        for i in range(256):
            l.append(clut(i))
        self.clut = tuple(l)
        self.lookup = {}

    def __call__(self, rgb888):
        """Compare rgb888 to every element of the CLUT and pick the
        closest match.
        """
        if rgb888 in self.lookup:
            return self.lookup[rgb888]

        best = 200000
        index = -1
        clut = self.clut
        r = rgb888 >> 16
        g = (rgb888 >> 8) & 0xff
        b = rgb888 & 0xff

        for i in range(256):
            candidate = clut[i]
            rd = r - (candidate >> 16)
            gd = g - ((candidate >> 8) & 0xff)
            bd = b - (candidate & 0xff)
            # This is the Euclidean distance (squared)
            distance = rd * rd + gd * gd + bd * bd
            if distance < best:
                best = distance
                index = i

        self.lookup[rgb888] = index
        #print(f'# #{rgb888:06x} -> #{clut8_rgb888(index):06x}')
        return index

def varname(p):
    return os.path.basename(os.path.splitext(p)[0])

def encode(im):
    pixels = im.load()

    rle = []
    rl = 0
    px = pixels[0, 0]

    def encode_pixel(px, rl):
        while rl > 255:
            rle.append(255)
            rle.append(0)
            rl -= 255
        rle.append(rl)

    for y in range(im.height):
        for x in range(im.width):
            newpx = pixels[x, y]
            if newpx == px:
                rl += 1
                assert(rl < (1 << 21))
                continue

            # Code the previous run
            encode_pixel(px, rl)

            # Start a new run
            rl = 1
            px = newpx

    # Handle the final run
    encode_pixel(px, rl)

    return (im.width, im.height, bytes(rle))

def encode_2bit(im):
    """2-bit palette based RLE encoder.

    This encoder has a reprogrammable 2-bit palette. This allows it to encode
    arbitrary images with a full 8-bit depth but the 2-byte overhead each time
    a new colour is introduced means it is not efficient unless the image is
    carefully constructed to keep a good locality of reference for the three
    non-background colours.

    The encoding competes well with the 1-bit encoder for small monochrome
    images but once run-lengths longer than 62 start to become frequent then
    this encoding is about 30% larger than a 1-bit encoding.
    """
    pixels = im.load()
    assert(im.width <= 255)
    assert(im.height <= 255)

    full_palette = ReverseCLUT(clut8_rgb888)

    rle = []
    rl = 0
    px = pixels[0, 0]
    # black, grey25, grey50, white
    palette = [0, 254, 219, 215]
    next_color = 1

    def encode_pixel(px, rl):
        nonlocal next_color
        px = full_palette((px[0] << 16) + (px[1] << 8) + px[2])
        if px not in palette:
            rle.append(next_color << 6)
            rle.append(px)
            palette[next_color] = px
            next_color += 1
            if next_color >= len(palette):
                next_color = 1
        px = palette.index(px)
        if rl >= 63:
            rle.append((px << 6) + 63)
            rl -= 63
            while rl >= 255:
                rle.append(255)
                rl -= 255
            rle.append(rl)
        else:
            rle.append((px << 6) + rl)

    # Issue the descriptor
    rle.append(2)
    rle.append(im.width)
    rle.append(im.height)

    for y in range(im.height):
        for x in range(im.width):
            newpx = pixels[x, y]
            if newpx == px:
                rl += 1
                assert(rl < (1 << 21))
                continue

            # Code the previous run
            encode_pixel(px, rl)

            # Start a new run
            rl = 1
            px = newpx

    # Handle the final run
    encode_pixel(px, rl)

    return bytes(rle)

def encode_8bit(im):
    """Experimental 8-bit RLE encoder.

    For monochrome images this is about 3x less efficient than the 1-bit
    encoder. This encoder is not currently used anywhere in wasp-os and
    currently there is no decoder either (so don't assume this code
    actually works).
    """
    pixels = im.load()

    rle = []
    rl = 0
    px = pixels[0, 0]

    def encode_pixel(px, rl):
        px = (px[0] & 0xe0) | ((px[1] & 0xe0) >> 3) | ((px[2] & 0xc0) >> 6)

        rle.append(px)
        if rl > 0:
            rle.append(px)
        rl -= 2
        if rl > (1 << 14):
            rle.append(0x80 | ((rl >> 14) & 0x7f))
        if rl > (1 <<  7):
            rle.append(0x80 | ((rl >>  7) & 0x7f))
        if rl >= 0:
            rle.append(         rl        & 0x7f )

    for y in range(im.height):
        for x in range(im.width):
            newpx = pixels[x, y]
            if newpx == px:
                rl += 1
                assert(rl < (1 << 21))
                continue

            # Code the previous run
            encode_pixel(px, rl)

            # Start a new run
            rl = 1
            px = newpx

    # Handle the final run
    encode_pixel(px, rl)

    return (im.width, im.height, bytes(rle))

def render_c(image, fname, indent, depth):
    extra_indent = ' ' * indent
    if len(image) == 3:
        print(f'{extra_indent}// {depth}-bit RLE, generated from {fname}, '
              f'{len(image[2])} bytes')
        (x, y, pixels) = image
    else:
        print(f'{extra_indent}// {depth}-bit RLE, generated from {fname}, '
              f'{len(image)} bytes')
        pixels = image

    print(f'{extra_indent}static const uint8_t {varname(fname)}[] = {{')
    print(f'{extra_indent} ', end='')
    i = 0
    for rl in pixels:
        print(f' {hex(rl)},', end='')

        i += 1
        if i == 12:
            print(f'\n{extra_indent} ', end='')
            i = 0
    print('\n};')

def render_py(image, fname, indent, depth):
    extra_indent = ' ' * indent
    if len(image) == 3:
        print(f'{extra_indent}# {depth}-bit RLE, generated from {fname}, '
              f'{len(image[2])} bytes')
        (x, y, pixels) = image
        print(f'{extra_indent}{varname(fname)} = (')
        print(f'{extra_indent}    {x}, {y},')
    else:
        print(f'{extra_indent}# {depth}-bit RLE, generated from {fname}, '
              f'{len(image)} bytes')
        pixels = image[3:]
        print(f'{extra_indent}{varname(fname)} = (')
        print(f'{extra_indent}    {image[0:1]}')
        print(f'{extra_indent}    {image[1:3]}')

    # Split the bytestring to ensure each line is short enough to
    # be absorbed on the target if needed.
    for i in range(0, len(pixels), 16):
        print(f'{extra_indent}    {pixels[i:i+16]}')
    print(f'{extra_indent})')


def decode_to_ascii(image):
    (sx, sy, rle) = image
    data = bytearray(2*sx)
    dp = 0
    black = ord('#')
    white = ord(' ')
    color = black

    for rl in rle:
        while rl:
            data[dp] = color
            data[dp+1] = color
            dp += 2
            rl -= 1

            if dp >= (2*sx):
                print(data.decode('utf-8'))
                dp = 0

        if color == black:
            color = white
        else:
            color = black

    # Check the image is the correct length
    assert(dp == 0)

parser = argparse.ArgumentParser(description='RLE encoder tool.')
parser.add_argument('files', nargs='+',
                    help='files to be encoded')
parser.add_argument('--ascii', action='store_true',
                    help='Run the resulting image(s) through an ascii art decoder')
parser.add_argument('--c', action='store_true',
                    help='Render the output as C instead of python')
parser.add_argument('--indent', default=0, type=int,
                    help='Add extra indentation in the generated code')
parser.add_argument('--2bit', action='store_true', dest='twobit',
                    help='Generate 2-bit image')
parser.add_argument('--8bit', action='store_true', dest='eightbit',
                    help='Generate 8-bit image')

args = parser.parse_args()
if args.eightbit:
    encoder = encode_8bit
    depth = 8
elif args.twobit:
    encoder = encode_2bit
    depth = 2
else:
    encoder = encode
    depth =1

for fname in args.files:
    image = encoder(Image.open(fname))

    if args.c:
        render_c(image, fname, args.indent, depth)
    else:
        render_py(image, fname, args.indent, depth)

    if args.ascii:
        print()
        decode_to_ascii(image)