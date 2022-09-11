#!/usr/bin/env python

import io
import sys
import json
import shutil
import typing
import os.path
import argparse
import subprocess

def gen_lvconv_line(lv_img_conv: str, dest: str, color_format: str, output_format: str, binary_format: str, sources: str):
    args = [lv_img_conv, sources, '--force', '--output-file', dest, '--color-format', color_format, '--output-format', output_format, '--binary-format', binary_format]

    return args

def main():
    ap = argparse.ArgumentParser(description='auto generate LVGL font files from fonts')
    ap.add_argument('config', type=str, help='config file to use')
    ap.add_argument('-i', '--image', type=str, action='append', help='Choose specific images to generate (default: all)', default=[])
    ap.add_argument('--lv-img-conv', type=str, help='Path to "lv_img_conf" executable', default="lv_img_conv")
    args = ap.parse_args()

    if not shutil.which(args.lv_img_conv):
        sys.exit(f"Missing lv_img_conv. Make sure it's findable (in PATH) or specify it manually")
    if not os.path.exists(args.config):
        sys.exit(f'Error: the config file {args.config} does not exist.')
    if not os.access(args.config, os.R_OK):
        sys.exit(f'Error: the config file {args.config} is not accessible (permissions?).')
    with open(args.config, 'r') as fd:
        data = json.load(fd)

    images_to_run = set(data.keys())

    if args.image:
        enabled_images = set()
        for image in args.image:
            enabled_images.add(image[:-2] if image.endswith('.c') else image)
        d = enabled_images.difference(images_to_run)
        if d:
            print(f'Warning: requested image{"s" if len(d)>1 else ""} missing: {" ".join(d)}')
        images_to_run = images_to_run.intersection(enabled_images)

    for name in images_to_run:
        image = data[name]
        if not os.path.exists(image['sources']):
            image['sources'] = os.path.join(os.path.dirname(sys.argv[0]), image['sources'])
        extension = 'bin'
        image.pop('target_path')
        line = gen_lvconv_line(args.lv_img_conv, f'{name}.{extension}', **image)
        subprocess.check_call(line)



if __name__ == '__main__':
    main()
