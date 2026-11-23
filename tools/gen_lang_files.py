#!/usr/bin/env python3
"""Generate active-language file routes for modLangFile.

Each language has identical row layout: a 24-bit collision-checked FNV key in
u32, a u16 copy index and a u16 original-story index (0xFFFF when absent).
LANGxx.BIN patches just the active language's eight-byte rows. All filenames
in the original table and NEW_FILES must have distinct nonzero keys.

  gen_lang_files.py OUT.h --lang "NAME:ES:EN[:PT:FR:DE:IT:JA:CA] ..."
                         --new "NAME ..." --table src/main/file_table_us.c
"""
import argparse, os, re, sys


def fnv1a(name):
    h = 0x811C9DC5
    for c in name.encode('ascii'):
        h = ((h ^ c) * 0x01000193) & 0xFFFFFFFF
    return h & 0xFFFFFF


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
    for value, group in hashes.items():
        if value == 0 or len(group) > 1:
            sys.exit(f'gen_lang_files: ambiguous filename hash {value:08X}: {sorted(group)}')
    rows = [[] for _ in range(8)]
    for spec in a.lang.split():
        parts = spec.split(':')
        if not 3 <= len(parts) <= 9:
            sys.exit(f'gen_lang_files: expected NAME:ES:EN[:PT:FR:DE:IT:JA:CA], got {spec}')
        key = parts[0]
        copies = parts[1:] + ['-'] * (9 - len(parts))
        if len(hashes.get(fnv1a(key), ())) > 1:
            sys.exit(f'gen_lang_files: {key} shares its hash with {sorted(hashes[fnv1a(key)] - {key})}')
        idx = []
        for c in copies:
            if c == '-':
                idx.append(0xFFFF)
            elif c in where:
                idx.append(where[c])
            else:
                sys.exit(f'gen_lang_files: {c} (a copy of {key}) is not in NEW_FILES')
        original = [where.get('_' + c[1:], 0xFFFF) if c != '-' else 0xFFFF for c in copies]
        for lang in range(8):
            col = 1 if lang == 0 else 0 if lang == 1 else lang
            rows[lang].append('{ 0x%08X, %d, %d }, /* %s */' % (fnv1a(key), idx[col], original[col], key))
    text = ''.join('#if MOD_LANG_BUILD == %d\n%s#endif\n' % (lang, ''.join(r + '\n' for r in table)) for lang, table in enumerate(rows))
    if not os.path.exists(a.out) or open(a.out).read() != text:
        open(a.out, 'w').write(text)


if __name__ == '__main__':
    main()
