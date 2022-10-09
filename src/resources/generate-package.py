#!/usr/bin/env python

import io
import sys
import json
import shutil
import typing
import os.path
import argparse
import subprocess
from zipfile import ZipFile

def main():
    ap = argparse.ArgumentParser(description='auto generate LVGL font files from fonts')
    ap.add_argument('--config', '-c', type=str, action='append', help='config file to use')
    ap.add_argument('--obsolete', type=str, help='List of obsolete files')
    ap.add_argument('--output', type=str, help='output file name')
    args = ap.parse_args()

    for config_file in args.config:
        if not os.path.exists(config_file):
            sys.exit(f'Error: the config file {config_file} does not exist.')
        if not os.access(config_file, os.R_OK):
            sys.exit(f'Error: the config file {config_file} is not accessible (permissions?).')

    if args.obsolete:
        obsolete_file_path = os.path.join(os.path.dirname(sys.argv[0]), args.obsolete)
        if not os.path.exists(obsolete_file_path):
            sys.exit(f'Error: the "obsolete" file {args.obsolete} does not exist.')
        if not os.access(obsolete_file_path, os.R_OK):
            sys.exit(f'Error: the "obsolete" file {args.obsolete} is not accessible (permissions?).')

    zf = ZipFile(args.output, mode='w')
    resource_files = []

    for config_file in args.config:
        with open(config_file, 'r') as fd:
            data = json.load(fd)

        resource_names = set(data.keys())
        for name in resource_names:
            resource = data[name]
            resource_files.append({
                "filename": name+'.bin',
                "path": resource['target_path'] + name+'.bin'
            })

            path = name + '.bin'
            if not os.path.exists(path):
                path = os.path.join(os.path.dirname(sys.argv[0]), path)
            zf.write(path)

    if args.obsolete:
        obsolete_file_path = os.path.join(os.path.dirname(sys.argv[0]), args.obsolete)
        with open(obsolete_file_path, 'r') as fd:
            obsolete_data = json.load(fd)
    else:
        obsolete_data = {}
    output = {
        'resources': resource_files,
        'obsolete_files': obsolete_data
    }


    with open("resources.json", 'w') as fd:
        json.dump(output, fd, indent=4)

    zf.write('resources.json')
    zf.close()

if __name__ == '__main__':
    main()
