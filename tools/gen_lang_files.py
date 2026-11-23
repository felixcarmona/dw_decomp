#!/usr/bin/env python3
"""MOD (language): lang_files.h, the rows of modLangFile (src/main/mod_options.c) from the Makefile's LANG_FILES.

A row is the hash of the file's name (FNV-1a, 32 bits) and the place of its Spanish copy and of its copy with English
signs among the new files (NEW_FILES: the entries after FILE_TABLE_NEW_FIRST), 0xFFFF for none. The names themselves are
already in FILE_OFFSET_TABLE: a row of three names cost 40 bytes of the executable's heap, this one 8 (the bilingual
disc has some 140 rows). A name of the disc whose hash meets a row's stops the build.

  gen_lang_files.py OUT.h --lang "NAME:ES:EN ..." --new "NAME ..." --table src/main/file_table_us.c
"""
import argparse, os, re, sys


def fnv1a(name):
    h = 0x811C9DC5
    for c in name.encode('ascii'):
        h = ((h ^ c) * 0x01000193) & 0xFFFFFFFF
    return h


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('out')
    ap.add_argument('--lang', default='')
    ap.add_argument('--new', default='')
    ap.add_argument('--table', required=True)
    a = ap.parse_args()
    new = a.new.split()
    where = {n: i for i, n in enumerate(new)}
    names = set(re.findall(r'"([A-Z0-9_]+\.[A-Z0-9_]+)"', open(a.table).read())) | set(new)
    hashes = {}
    for n in names:
        hashes.setdefault(fnv1a(n), set()).add(n)
    rows = []
    for spec in a.lang.split():
        key, es, en = spec.split(':')
        if len(hashes.get(fnv1a(key), ())) > 1:
            sys.exit(f'gen_lang_files: {key} shares its hash with {sorted(hashes[fnv1a(key)] - {key})}')
        idx = []
        for c in (es, en):
            if c == '-':
                idx.append(0xFFFF)
            elif c in where:
                idx.append(where[c])
            else:
                sys.exit(f'gen_lang_files: {c} (a copy of {key}) is not in NEW_FILES')
        rows.append(f'{{ 0x{fnv1a(key):08X}, {idx[0]}, {idx[1]} }}, /* {key} */')
    text = ''.join(r + '\n' for r in rows)
    if not os.path.exists(a.out) or open(a.out).read() != text:
        open(a.out, 'w').write(text)


if __name__ == '__main__':
    main()
