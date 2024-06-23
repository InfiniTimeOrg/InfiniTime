#!/usr/bin/env python3
import argparse
import math
import pathlib
import sys
import decimal
from PIL import Image, ImageFont, ImageDraw
from fontTools import ttLib

# pacman -S python-fonttools
# apt install -y python3-fonttools
#import fonttools

__version__ = "0.0.1"

class FontArg:
    def __init__(
        self,
        font: str, # path to input font file
         ):
        self.font = font
        self.symbols : str = "" # list of characters to copy
        self.args: str = ""
    def add_ranges(self, ranges: str):
        if self.args == "":
            self.args = f"range({ranges})"
        else:
            self.args += f" range({ranges})"
        for code_str in ranges.split(","):
            # to char
            if "-" in code_str:
                begin_str, end_str = code_str.split("-")
                begin_code = int(begin_str, 0)
                end_code = int(end_str, 0)
                for code in range(begin_code, end_code+1):
                    self.symbols += chr(code)
            else:
                code = int(code_str, 0)
                self.symbols += chr(code)
    def add_symbols(self, symbols: str):
        if self.args == "":
            self.args = f"symbols({symbols})"
        else:
            self.args += f" symbols({symbols})"
        self.symbols += symbols

class FontAction(argparse.Action):
    def __init__(self, option_strings, dest, nargs=None, **kwargs):
        # store all into 'font' destination
        dest = "font"
        super().__init__(option_strings, dest, **kwargs)
    def __call__(self, parser, namespace, values, option_string=None):
        font = getattr(namespace, self.dest)
        if option_string == "--font":
            if font is None:
                font = [FontArg(values)]
            else:
                font.append(FontArg(values))
        elif option_string == "--range":
            font[-1].add_ranges(values)
        elif option_string == "--symbols":
            font[-1].add_symbols(values)
        else:
            raise argparse.ArgumentError("unhandled option_string: " + option_string)
        setattr(namespace, self.dest, font)

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
        action=FontAction,
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
    parser.add_argument("-r", "--range",
        help="""Range of glyphs to copy. Can be used multiple times, belongs to previously declared
      "--font". Examples:
      -r 0x1F450
      -r 0x20-0x7F
      -r 32-127
      -r 32-127,0x1F450
      -r '0x1F450=>0xF005'
      -r '0x1F450-0x1F470=>0xF005'""",
      action=FontAction)
    parser.add_argument("--symbols",
        help="List of characters to copy, belongs to previously declared \"--font\". Examples:"
        "     '--symbols ,.0123456789' or '--symbols abcdefghigklmnopqrstuvwxyz'",
      action=FontAction)
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
    if args.font is None:
        raise RuntimeError("no --font argument supplied")

    for idx, font_arg in enumerate(args.font):
        if not isinstance(font_arg, FontArg):
            raise RuntimeError(f"font_arg is expected to be a FontArg type, but got type: {type(font_arg)}")

        if not pathlib.Path(font_arg.font).is_file():
            raise RuntimeError(f"provided font file doesn't exist: {font_arg.font}")

        print(f"processing font: {font_arg.font}")
        print(f"- args: {font_arg.args}")
        print(f"- characters: {font_arg.symbols}")
        #tt = ttLib.TTFont(args.font[0]) # Load an existing font file
        #print(tt['maxp'].numGlyphs)
        #print(tt['OS/2'].achVendID)
        #print(tt['head'].unitsPerEm)
        #print(tt.getGlyphNames())
        #print(tt.getGlyphID("zero"))
        #print(tt.keys())
        # https://stackoverflow.com/questions/70368410/how-to-render-a-ttf-glyf-to-a-image-with-fonttools
        # https://pillow.readthedocs.io/en/stable/reference/ImageFont.html
        font = ImageFont.truetype(font_arg.font, args.size)
        ascent, descent = font.getmetrics()
        print(f"- ascent/descent: {ascent}, {descent}")
        text = font_arg.symbols
        image = Image.new(mode='L', size=(args.size*len(text), args.size), color=224)
        draw = ImageDraw.Draw(image)
        for c in text:
            (width, baseline), (offset_x, offset_y) = font.font.getsize(c)
            print(f"- '{c}': w x h: {width} x {baseline}, o_xy: {offset_x}, {offset_y}")
            print(draw.textlength(c, font=font))

        length = math.ceil(draw.textlength(text, font=font))
        image = Image.new(mode='L', size=(length, args.size), color=224)
        draw = ImageDraw.Draw(image)
        draw.text((0,-descent), text, font=font)
        image.save(f"out_{idx}.png")

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
