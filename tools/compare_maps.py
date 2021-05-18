# vim: set fileencoding=utf8 :

import argparse
import re, os
from itertools import groupby

class Objectfile:
    def __init__ (self, section, offset, size, comment):
        self.section = section.strip ()
        self.offset = offset
        self.size = size
        self.path = (None, None)
        self.basepath = None
        if comment:
            self.path = re.match (r'^(.+?)(?:\(([^\)]+)\))?$', comment).groups ()
            self.basepath = os.path.basename (self.path[0])
        self.children = []

    def __repr__ (self):
        return '<Objectfile {} {:x} {:x} {} {}>'.format (self.section, self.offset, self.size, self.path, repr (self.children))

def parseSections (fd):
    """
    Quick&Dirty parsing for GNU ld’s linker map output, needs LANG=C, because
    some messages are localized.
    """

    sections = []

    # skip until memory map is found
    found = False
    while True:
        l = fd.readline ()
        if not l:
            break
        if l.strip () == 'Memory Configuration':
            found = True
            break
    if not found:
        return None

    # long section names result in a linebreak afterwards
    sectionre = re.compile ('(?P<section>.+?|.{14,}\n)[ ]+0x(?P<offset>[0-9a-f]+)[ ]+0x(?P<size>[0-9a-f]+)(?:[ ]+(?P<comment>.+))?\n+', re.I)
    subsectionre = re.compile ('[ ]{16}0x(?P<offset>[0-9a-f]+)[ ]+(?P<function>.+)\n+', re.I)
    s = fd.read ()
    pos = 0
    while True:
        m = sectionre.match (s, pos)
        if not m:
            # skip that line
            try:
                nextpos = s.index ('\n', pos)+1
                pos = nextpos
                continue
            except ValueError:
                break
        pos = m.end ()
        section = m.group ('section')
        v = m.group ('offset')
        offset = int (v, 16) if v is not None else None
        v = m.group ('size')
        size = int (v, 16) if v is not None else None
        comment = m.group ('comment')
        if section != '*default*' and size > 0:
            of = Objectfile (section, offset, size, comment)
            if section.startswith (' '):
                sections[-1].children.append (of)
                while True:
                    m = subsectionre.match (s, pos)
                    if not m:
                        break
                    pos = m.end ()
                    offset, function = m.groups ()
                    offset = int (offset, 16)
                    if sections and sections[-1].children:
                        sections[-1].children[-1].children.append ((offset, function))
            else:
                sections.append (of)

    return sections

def measure_map(fname):
    sections = parseSections(open(fname, 'r'))
    if sections is None:
        print ('start of memory config not found, did you invoke the compiler/linker with LANG=C?')
        return

    sectionWhitelist = {'.text', '.data', '.bss', '.rodata'}
    whitelistedSections = list (filter (lambda x: x.section in sectionWhitelist, sections))
    grouped_objects = {s: {} for s in sectionWhitelist}
    for s in whitelistedSections:
        for k, g in groupby (sorted (s.children, key=lambda x: x.basepath), lambda x: x.basepath):
            size = sum (map (lambda x: x.size, g))
            grouped_objects[s.section][k] = size
    return grouped_objects

def main(old_fname, new_fname):
    old_by_section = measure_map(old_fname)
    new_by_section = measure_map(new_fname)

    diffs = []
    total_diff = 0
    for section in old_by_section.keys(): # sections are on both
        new = new_by_section[section]
        old = old_by_section[section]
        total_new = sum(new.values())
        total_old = sum(old.values())
        diff = total_new-total_old
        total_diff += diff

        if diff == 0:
            continue

        all_keys = set(new.keys()).union(old.keys())

        for k in all_keys:
            delta = new.get(k, 0) - old.get(k, 0)
            if delta == 0:
                continue
            name = k
            if section != '.text':
                name = f'({section}){name}'
            diffs.append((delta, name))

    if diffs:
        print('Object|Change (bytes)')
        print('|:----|------------:|')
        for change, obj in sorted(diffs):
            sign = "+" if change > 0 else "-"
            print(f'{obj}|{sign}{abs(change)}')

        sign = "+" if total_diff > 0 else "-"
        print(f'**Total**|{sign}{abs(total_diff)}')

def parse_args():
    parser = argparse.ArgumentParser(description="Compare the respective sizes in 2 map files", epilog='''
    Example:

    compare_maps.py old.map new.map
    ''')
    parser.add_argument("old")
    parser.add_argument("new")
    args = parser.parse_args()
    main(args.old, args.new)

if __name__ == '__main__':
    parse_args()
