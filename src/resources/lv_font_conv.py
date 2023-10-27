#!/usr/bin/env python3
import argparse
import pathlib
import sys
import decimal
from PIL import Image

# pacman -S python-fonttools
# apt install -y python3-fonttools
#import fonttools

__version__ = "0.0.1"

class FontArg:
    font : str # path to input font file
    ranges : list = [] # ranges of glyps to copy
    symbols : str = "" # list of characters to copy

def main():
    parser = argparse.ArgumentParser()

    # arguments in comments are the help text of the original lv_font_conv.js scrip
    # not implemented in this minimal port
    parser.add_argument("--version",
        help="show program's version number and exit",
        action="version",
        version=__version__)
    parser.add_argument("--size",
        help="Output font size, pixels.",
        metavar="PIXELS",
        type=int,
        required=True)
    parser.add_argument("-o", "--output",
        help="Output path.")
    parser.add_argument("--bpp",
        help="Bits per pixel, for antialiasing.",
        choices=[1,2,3,4,8],
        type=int,
        required=True)
#  --lcd                 Enable subpixel rendering (horizontal pixel layout).
#  --lcd-v               Enable subpixel rendering (vertical pixel layout).
#  --use-color-info      Try to use glyph color info from font to create grayscale icons. Since gray tones
#                        are emulated via transparency, result will be good on contrast background only.
    parser.add_argument("--format",
        help="Output format.",
        choices=["dump", "bin", "lvgl"],
        required=True)
    parser.add_argument("--font",
        help="Source font path. Can be used multiple times to merge glyphs from different fonts.",
        action=argparse._AppendAction,
        required=True)
#  -r RANGE, --range RANGE
#                        Range of glyphs to copy. Can be used multiple times, belongs to previously declared
#                        "--font". Examples:
#                        -r 0x1F450
#                        -r 0x20-0x7F
#                        -r 32-127
#                        -r 32-127,0x1F450
#                        -r '0x1F450=>0xF005'
#                        -r '0x1F450-0x1F470=>0xF005'
#  --symbols SYMBOLS     List of characters to copy, belongs to previously declared "--font". Examples:
#                        --symbols ,.0123456789
#                        --symbols abcdefghigklmnopqrstuvwxyz
#  --autohint-off        Disable autohinting for previously declared "--font"
#  --autohint-strong     Use more strong autohinting for previously declared "--font" (will break kerning)
#  --force-fast-kern-format
#                        Always use kern classes instead of pairs (might be larger but faster).
    parser.add_argument("--no-compress",
        help="Disable built-in RLE compression.",
        action="store_true")
#  --no-prefilter        Disable bitmap lines filter (XOR), used to improve compression ratio.
#  --no-kerning          Drop kerning info to reduce size (not recommended).
#  --lv-include <path>   Set alternate "lvgl.h" path (for --format lvgl).
#  --lv-font-name LV_FONT_NAME
#                        Variable name of the lvgl font structure. Defaults to the output's basename.
#  --full-info           Don't shorten "font_info.json" (include pixels data).

    args = parser.parse_args()

    # only implemented the bare minimum, everything else is not implemented
    if args.bpp != 1:
        raise NotImplementedError(f"--bpp '{args.bpp}' not implemented, only '--bpp 1' implemented")
    if args.format not in ["bin", "lvgl"]:
        raise NotImplementedError(f"--format '{args.format}' not implemented")
    if not args.no_compress:
        raise NotImplementedError("compression not implemented, '--no-compress' required")

    if args.size <= 0:
        raise RuntimeError("--size must be a positive integer greater than 0")

    return 0


if __name__ == '__main__':
    if "--test" in sys.argv:
        # run small set of tests and exit
        print("running tests")
        test_classify_pixel()
        print("success!")
        sys.exit(0)
    # run normal program
    sys.exit(main())
