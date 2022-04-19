#!/usr/bin/env python

import io
import sys
import json
import typing
import os.path
import argparse
import subprocess

class FontArg(object):
    def __init__(self, d):
        self.font = d['font']
        self.range = d.get('range')
        self.symbols = d.get('symbols')


def gen_lvconv_line(dest: str, size: int, bpp: int, fonts: typing.List[FontArg], compress:bool=False):
    args = ['lv_font_conv', '--size', str(size), '--output', dest, '--bpp', str(bpp), '--format', 'lvgl']
    if not compress:
        args.append('--no-compress')
    for font in fonts:
        args.extend(['--font', font.font])
        if font.range:
            args.extend(['--range', font.range])
        if font.symbols:
            args.extend(['--symbols', font.symbols])

    return args

def main():
    ap = argparse.ArgumentParser(description='auto generate lvGL font files from fonts')
    ap.add_argument('config', type=str, help='config file to use')
    ap.add_argument('-e', '--enable', type=str, action='append', help='optional feature to enable in font generation', default=[], metavar='features', dest='features')
    ap.add_argument('-f', '--font', type=str, action='append', help='Choose specific fonts to generate (default: all)', default=[])
    args = ap.parse_args()

    if not os.path.exists(args.config):
        sys.exit(f'Error: the config file {args.config} does not exist.')
    if not os.access(args.config, os.R_OK):
        sys.exit(f'Error: the config file {args.config} is not accessable (permissions?).')
    with open(args.config, 'r') as fd:
        data = json.load(fd)

    fonts_to_run = args.font

    if fonts_to_run:
        d = set(fonts_to_run).difference(data['fonts'].keys())
        if d:
            print(f'Warning: requested font{"s" if len(d)>1 else ""} missing: {" ".join(d)}')
            fonts_to_run = list(set(fonts_to_run).intersection(data['fonts'].keys()))

    for (name,font) in data['fonts'].items():
        if fonts_to_run and name not in fonts_to_run:
            continue
        sources = font.pop('sources')
        patches = font.pop('patches') if 'patches' in font else  []
        features = font.pop('features') if 'features' in font else []
        for enabled_feature in args.features:
            if enabled_feature in features:
                sources.extend(features[enabled_feature])
        font['fonts'] = [FontArg(thing) for thing in sources]
        line = gen_lvconv_line(f'{name}.c', **font)
        subprocess.check_call(line)
        if patches:
            for patch in patches:
                subprocess.check_call(patch)



if __name__ == '__main__':
    main()
