#!/usr/bin/env python3
import argparse
from dataclasses import dataclass, field
import itertools
import math
import pathlib
import sys
from PIL import Image, ImageFont, ImageDraw, ImageOps
# from fontTools import ttLib
from typing import Tuple

# pacman -S python-fonttools
# apt install -y python3-fonttools

__version__ = "0.0.1"


class FontArg:
    def __init__(
        self,
        font: str,  # path to input font file
         ):
        self.font = font
        self.symbols: str = ""  # list of characters to copy
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


@dataclass
class LVGlyph:
    character: str
    glyph_id: int
    bitmap: bytearray
    bitmap_index: int
    # length of horizontal skip for glyp, multiplied by 16 and rounded
    # original js code: adv_w = Math.round(d.glyph.advanceWidth * 16),
    advance_width: int
    bbox_width: int
    bbox_height: int
    offset_x: int
    offset_y: int


@dataclass
class LVFont:
    name: str
    size: int
    bpp: int = 1
    opts: str = ""
    glyphs: list[LVGlyph] = field(default_factory=list)
    cmaps: list[list[LVGlyph]] = field(default_factory=list)


def main():
    parser = argparse.ArgumentParser()

    # arguments in comments are the help text of the original lv_font_conv.js scrip
    # not implemented in this minimal port
    parser.add_argument(
        "--version",
        help="show program's version number and exit",
        action="version",
        version=__version__)
    parser.add_argument(
        "--size",
        help="Output font size, pixels.",
        metavar="PIXELS",
        type=int,
        required=True)
    parser.add_argument(
        "-o", "--output",
        help="Output path.")
    parser.add_argument(
        "--bpp",
        help="Bits per pixel, for antialiasing.",
        choices=[1, 2, 3, 4, 8],
        type=int,
        required=True)
#  --lcd                 Enable subpixel rendering (horizontal pixel layout).
#  --lcd-v               Enable subpixel rendering (vertical pixel layout).
#  --use-color-info      Try to use glyph color info from font to create grayscale icons. Since gray tones
#                        are emulated via transparency, result will be good on contrast background only.
    parser.add_argument(
        "--format",
        help="Output format.",
        choices=["dump", "bin", "lvgl"],
        required=True)
    parser.add_argument(
        "--font",
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
    parser.add_argument(
        "-r", "--range",
        help="""Range of glyphs to copy. Can be used multiple times, belongs to
      previously declared "--font". Examples:
      -r 0x1F450
      -r 0x20-0x7F
      -r 32-127
      -r 32-127,0x1F450
      -r '0x1F450=>0xF005'
      -r '0x1F450-0x1F470=>0xF005'""",
        action=FontAction)
    parser.add_argument(
        "--symbols",
        help="List of characters to copy, "
        "belongs to previously declared \"--font\". Examples:"
        "     '--symbols ,.0123456789' or"
        "     '--symbols abcdefghigklmnopqrstuvwxyz'",
        action=FontAction)
#  --autohint-off        Disable autohinting for previously declared "--font"
#  --autohint-strong     Use more strong autohinting for previously declared "--font" (will break kerning)
#  --force-fast-kern-format
#                        Always use kern classes instead of pairs (might be larger but faster).
    parser.add_argument(
        "--no-compress",
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
        raise NotImplementedError(
            f"--bpp '{args.bpp}' not implemented, only '--bpp 1' implemented")
    if args.format not in ["bin", "lvgl"]:
        raise NotImplementedError(f"--format '{args.format}' not implemented")
    if not args.no_compress:
        raise NotImplementedError(
            "compression not implemented, '--no-compress' required")

    if args.size <= 0:
        raise RuntimeError("--size must be a positive integer greater than 0")
    if args.font is None:
        raise RuntimeError("no --font argument supplied")

    lv_font = LVFont(
        name=pathlib.Path(args.output).stem.replace("-", "_"),
        size=args.size,
        bpp=args.bpp,
        opts=" ".join(sys.argv[1:]),
    )
    def Img_from_list_of_bytes(size: Tuple[int,int], b_data: list[int]) -> Image:
        img = Image.new("1", size, color = 1)
        byte_idx = 0
        bit_idx = 7
        for y in range(size[1]):
            for x in range(size[0]):
                val = b_data[byte_idx] & (1 << bit_idx)
                img.putpixel((x,y), 0 if val else 1)
                bit_idx -= 1
                if bit_idx < 0:
                    byte_idx += 1
                    bit_idx = 7
        return img

    img = Img_from_list_of_bytes(
        (3,14),
        b_data = [0xff, 0xff, 0xff, 0xfc, 0xf, 0xc0,]
    )
    img.save("out_js_u21.png")
    img = Img_from_list_of_bytes(
        (7,6),
        b_data = [0xef, 0xdf, 0xbf, 0x7e, 0xfd, 0xc0,]
    )
    img.save("out_js_u22.png")
    img = Img_from_list_of_bytes(
        (10,14),
        b_data = [0xff, 0x3f, 0xee, 0x3f, 0x87, 0xe1, 0xf8, 0xff,
            0xfb, 0xfc, 0xe0, 0x38, 0xe, 0x3, 0x80, 0xe0,
            0x38, 0x0,]
    )
    img.save("out_js_u50.png")
    img = Img_from_list_of_bytes(
        (9,14),
        b_data = [0xe3, 0xf1, 0xf8, 0xfc, 0x7e, 0x3f, 0x1f, 0x8f,
            0xc7, 0xe3, 0xf1, 0xf8, 0xfc, 0x77, 0xf1, 0xf0]
    )
    img.save("out_js_u55.png")

    for idx, font_arg in enumerate(args.font):
        if not isinstance(font_arg, FontArg):
            raise RuntimeError(
                "font_arg is expected to be a FontArg type, "
                f"but got type: {type(font_arg)}")

        if not pathlib.Path(font_arg.font).is_file():
            raise RuntimeError(
                f"provided font file doesn't exist: {font_arg.font}")

        print(f"processing font: {font_arg.font}")
        print(f"- args: {font_arg.args}")
        print(f"- characters: {font_arg.symbols}")
        # tt = ttLib.TTFont(args.font[0]) # Load an existing font file
        # print(tt['maxp'].numGlyphs)
        # print(tt['OS/2'].achVendID)
        # print(tt['head'].unitsPerEm)
        # print(tt.getGlyphNames())
        # print(tt.getGlyphID("zero"))
        # print(tt.keys())
        # https://stackoverflow.com/questions/70368410/how-to-render-a-ttf-glyf-to-a-image-with-fonttools
        # https://pillow.readthedocs.io/en/stable/reference/ImageFont.html
        # https://pillow.readthedocs.io/en/stable/handbook/text-anchors.html
        # https://stackoverflow.com/questions/27631736/meaning-of-top-ascent-baseline-descent-bottom-and-leading-in-androids-font
        font = ImageFont.truetype(font_arg.font, args.size)
        ascent, descent = font.getmetrics()
        print(f"- ascent/descent: {ascent}, {descent}")
        # symbols are sorted by their ASCII index, each glyp only once
        text = "".join(sorted(set(font_arg.symbols), key=ord))

        bitmap_index = 0
        features = ["-liga", "-kern"]
        font_settings = {
            "features": features,
            "anchor": "ls",
        }
        # image with 1 pixel black line between glyphs
        image_length = sum([font.getlength(c)+1 for c in text])
        x_pos = 0
        image = Image.new(mode='RGB', size=(int(math.ceil(image_length))+1, args.size*2), color=(224,224,224))
        draw = ImageDraw.Draw(image)
        draw.line([0, args.size - ascent, image_length, args.size - ascent], fill="green")
        draw.line([0, args.size + descent, image_length, args.size + descent], fill="blue")
        draw.line([0, args.size, image_length, args.size], fill="gray")
        for c in text:
            (width, baseline), (offset_x, offset_y) = font.font.getsize(c)
            # (left, top, right, bottom) = font.getbbox(c, **font_settings)
            (left, top, right, bottom) = font.getbbox(c)
            # length = font.getlength(c, features=features)
            length = font.getlength(c)
            # mask: Image.core = font.getmask(c, **font_settings)
            mask: Image.core = font.getmask(c, mode="1")
            if mask.size[1] == 0:
                mask = b"\0"
                c_image = b"\0"
                b_array = b"\0"
                m_left = m_top = m_right = m_bottom = 0
                c_left = c_top = c_right = c_bottom = 0
                bbox_width = 1
                bbox_height = 1
                offset_x = 0
                offset_y = 0
            else:
                (m_left, m_top, m_right, m_bottom) = mask.getbbox()
                bbox_width = m_right - m_left
                bbox_height = m_bottom - m_top
                mask = mask.crop(mask.getbbox())
                ImageOps.invert(Image.frombytes(mask.mode, mask.size, bytearray(mask)))\
                    .save(f"out_{idx}_{ord(c):02x}_m.png")
                c_image = Image.new(
                    mode='1',
                    size=(int(math.ceil(length))+1, args.size*2),
                    color=255)
                c_draw = ImageDraw.Draw(c_image)
                c_draw.text((0, 0), text=c, font=font, fill=0)
                c_image.save(f"out_{idx}_{ord(c):02x}.png")
                c_left, c_top, c_right, c_bottom = bbox = \
                    ImageOps.invert(c_image).getbbox()
                c_image = c_image.crop(bbox)
                c_image.save(f"out_{idx}_{ord(c):02x}_c.png")
                b_array = c_image.tobytes()
            print(f"- '{c}': w x s: {width} x {baseline}, o_xy: {offset_x}, {offset_y}")
            print(f"- '{c}': bbox l - r: {left} - {right}, t - b: {top} - {bottom}")
            print(f"- '{c}': mask l - r: {m_left} - {m_right}, t - b: {m_top} - {m_bottom}")
            print(f"- '{c}': img  l - r: {c_left} - {c_right}, t - b: {c_top} - {c_bottom}")
            print(f"- '{c}': length: {length}")
            lv_glyph = LVGlyph(
                character=c,
                glyph_id=len(lv_font.glyphs) + 1,
                bitmap=bytearray(b_array),
                bitmap_index=bitmap_index,
                advance_width=round(length * 16),
                bbox_width=bbox_width,
                bbox_height=bbox_height,
                offset_x=c_left,
                offset_y=offset_y,
            )
            if len(lv_glyph.bitmap) == 1:
                lv_glyph.bbox_width = 1
                lv_glyph.bbox_height = 1
            bitmap_index += len(lv_glyph.bitmap)
            lv_font.glyphs.append(lv_glyph)
            # line between each glyp
            draw.line([x_pos-1,0, x_pos-1, args.size], fill=0)
            # draw glyp
            #draw.text((x_pos,0), c, font=font, features=features) #, **font_settings)
            draw.text((x_pos,0), c, font=font, fill="black") #, **font_settings)


            # mark bbox
            #draw.line([x_pos+offset_x,baseline, x_pos+offset_x+width, baseline], fill=128)
            #draw.line([x_pos,0, x_pos+length, 0], fill=128)
            x_pos += length+1

        #draw.text((0,-descent), text, font=font)
        image.save(f"out_{idx}.png")

    # cmaps of consecutive glyphs
    for k, g in itertools.groupby(enumerate(lv_font.glyphs), lambda x: x[0]-ord(x[1].character)):
        lv_font.cmaps.append(list(map(lambda x: x[1], g)))

    if args.format == "lvgl":
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(to_lvgl(lv_font))
    elif args.format == "bin":
        raise NotImplementedError("format 'bin' not yet implemented")
    else:
        raise RuntimeError(f"unhandled format: '{args.format}'")

    return 0


def to_lvgl(font: LVFont):
    macro_name = font.name.upper()
    out = \
f"""/*******************************************************************************
 * Size: {font.size} px
 * Bpp: {font.bpp}
 * Opts: {font.opts}
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef {macro_name}
#define {macro_name} 1
#endif

#if {macro_name}

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {{"""
    for i, g in enumerate(font.glyphs):
        bytes_str = ", ".join([f"0x{b:x}" for b in g.bitmap])
        if g.character.isprintable():
            char_str = g.character if g.character != '"' else '\\"'
        else:
            char_str = f"\\u{ord(g.character):04X}"
        out += f"""
    /* U+{ord(g.character):04X} "{char_str}" */
    {bytes_str}{",\n" if i+1 < len(font.glyphs) else ""}"""
    out += f"""
}};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {{
    {{.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0}} /* id = 0 reserved */,"""
    for i, g in enumerate(font.glyphs):
        bytes_str = ", ".join([f"0x{b:x}" for b in g.bitmap])
        out += f"""
    {{.bitmap_index = {g.bitmap_index}, .adv_w = {g.advance_width}, .box_w = {g.bbox_width}, .box_h = {g.bbox_height}, .ofs_x = {g.offset_x}, .ofs_y = {g.offset_y}}}{"," if i+1 < len(font.glyphs) else ""}"""
    out += f"""
}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

// TODO: figure out when to do a unicode_list_X[]
// TODO: figure out when to create SPARSE_TINY or FORMAT0_TINY

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{{"""
    for i, cmap in enumerate(font.cmaps):
        out += f"""
    {{
        .range_start = {ord(cmap[0].character)}, .range_length = {len(cmap)}, .glyph_id_start = {cmap[0].glyph_id},
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }}{"," if i+1 < len(font.cmaps) else ""}"""
    out += f"""
}};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR >= 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {{
#else
static lv_font_fmt_txt_dsc_t font_dsc = {{
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = {len(font.cmaps)},
    .bpp = {font.bpp},
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR >= 8
    .cache = &cache
#endif
}};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t jetbrains_mono_bold_20_js = {{
#else
lv_font_t jetbrains_mono_bold_20_js = {{
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = {max([g.bbox_height for g in font.glyphs])},          /*The maximum line height required by the font*/
    .base_line = {min([g.offset_y for g in font.glyphs])},             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = XXX,
    .underline_thickness = XXX,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
}};



#endif /*#if JETBRAINS_MONO_BOLD_20_JS*/

"""

    return out

if __name__ == '__main__':
    if "--test" in sys.argv:
        # run small set of tests and exit
        print("running tests")
        test_classify_pixel()
        print("success!")
        sys.exit(0)
    # run normal program
    sys.exit(main())
