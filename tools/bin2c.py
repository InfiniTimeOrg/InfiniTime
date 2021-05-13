#!/usr/bin/env python3
#-*- coding: utf-8 -*-
"""
    bin2c
    ~~~~~

    Simple tool for creating C array from a binary file.

    :copyright: (c) 2016 by Dmitry Alimov.
    :license: The MIT License (MIT), see LICENSE for more details.
"""

import argparse
import os
import re
import sys

PY3 = sys.version_info[0] == 3


def bin2c(filename, varname='data', linesize=80, indent=4):
    """ Read binary data from file and return as a C array

    :param filename: a filename of a file to read.
    :param varname: a C array variable name.
    :param linesize: a size of a line (min value is 40).
    :param indent: an indent (number of spaces) that prepend each line.
    """
    if not os.path.isfile(filename):
        print('File "%s" is not found!' % filename)
        return ''
    if not re.match('[a-zA-Z_][a-zA-Z0-9_]*', varname):
        print('Invalid variable name "%s"' % varname)
        return
    with open(filename, 'rb') as in_file:
        data = in_file.read()
    # limit the line length
    if linesize < 40:
        linesize = 40
    byte_len = 6  # '0x00, '
    out = 'const char %s[%d] = {\n' % (varname, len(data))
    line = ''
    for byte in data:
        line += '0x%02x, ' % (byte if PY3 else ord(byte))
        if len(line) + indent + byte_len >= linesize:
            out += ' ' * indent + line + '\n'
            line = ''
    # add the last line
    if len(line) + indent + byte_len < linesize:
        out += ' ' * indent + line + '\n'
    # strip the last comma
    out = out.rstrip(', \n') + '\n'
    out += '};'
    return out


def main():
    """ Main func """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'filename', help='filename to convert to C array')
    parser.add_argument(
        'varname', nargs='?', help='variable name', default='data')
    parser.add_argument(
        'linesize', nargs='?', help='line length', default=80, type=int)
    parser.add_argument(
        'indent', nargs='?', help='indent size', default=4, type=int)
    args = parser.parse_args()
    # print out the data
    print(bin2c(args.filename, args.varname, args.linesize, args.indent))


if __name__ == '__main__':
    main()
