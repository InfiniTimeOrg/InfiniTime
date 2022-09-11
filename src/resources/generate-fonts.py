#!/usr/bin/env python

import io
import sys
import json
import shutil
import typing
import os.path
import argparse
import subprocess

class Source(object):
    def __init__(self, d):
        self.file = d['file']
        if not os.path.exists(self.file):
            self.file = os.path.join(os.path.dirname(sys.argv[0]), self.file)
        self.range = d.get('range')
        self.symbols = d.get('symbols')


def gen_lvconv_line(lv_font_conv: str, dest: str, size: int, bpp: int, format: str, sources: typing.List[Source], compress:bool=False):
    if format != "lvgl" and format != "bin":
        format = "bin" if dest.lower().endswith(".bin") else "lvgl"

    args = [lv_font_conv, '--size', str(size), '--output', dest, '--bpp', str(bpp), '--format', format]
    if not compress:
        args.append('--no-compress')
    for source in sources:
        args.extend(['--font', source.file])
        if source.range:
            args.extend(['--range', source.range])
        if source.symbols:
            args.extend(['--symbols', source.symbols])

    return args

def main():
    ap = argparse.ArgumentParser(description='auto generate LVGL font files from fonts')
    ap.add_argument('config', type=str, help='config file to use')
    ap.add_argument('-f', '--font', type=str, action='append', help='Choose specific fonts to generate (default: all)', default=[])
    ap.add_argument('--lv-font-conv', type=str, help='Path to "lv_font_conf" executable', default="lv_font_conv")
    args = ap.parse_args()

    if not shutil.which(args.lv_font_conv):
        sys.exit(f"Missing lv_font_conv. Make sure it's findable (in PATH) or specify it manually")
    if not os.path.exists(args.config):
        sys.exit(f'Error: the config file {args.config} does not exist.')
    if not os.access(args.config, os.R_OK):
        sys.exit(f'Error: the config file {args.config} is not accessible (permissions?).')
    with open(args.config, 'r') as fd:
        data = json.load(fd)

    fonts_to_run = set(data.keys())

    if args.font:
        enabled_fonts = set()
        for font in args.font:
            enabled_fonts.add(font[:-2] if font.endswith('.c') else font)
        d = enabled_fonts.difference(fonts_to_run)
        if d:
            print(f'Warning: requested font{"s" if len(d)>1 else ""} missing: {" ".join(d)}')
        fonts_to_run = fonts_to_run.intersection(enabled_fonts)

    for name in fonts_to_run:
        font = data[name]
        sources = font.pop('sources')
        patches = font.pop('patches') if 'patches' in font else  []
        font['sources'] = [Source(thing) for thing in sources]
        extension = 'c' if font['format'] != 'bin' else 'bin'
        font.pop('target_path')
        line = gen_lvconv_line(args.lv_font_conv, f'{name}.{extension}', **font)
        subprocess.check_call(line)
        if patches:
            for patch in patches:
                subprocess.check_call(['/usr/bin/env', 'patch', name+'.'+extension, patch])



if __name__ == '__main__':
    main()
