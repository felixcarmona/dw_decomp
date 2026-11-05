#!/usr/bin/env python3
# MOD (text sources, docs/handoff/text-sources-design.md): a text bank can replace any entry's text by its key, and the
# characters of the mod executable's extra glyphs (F040.., docs/mods/font.md) encode as their two-byte codes. The bank is
# the JSON file $TEXT_BANK (default: text-bank.json in the working directory, if it exists), written by the project's
# tools/build-full.sh: {"font": {"á": "F040", ...}, "text": {"main.MOVE_NAMES.12": "...", "main.STR_YES": "..."}}. Keys:
# <dir>.<SYMBOL> for a `strings:` entry, <dir>.<BLOCK> for a single entry, <dir>.<BLOCK>.<index> for a table row (<dir> is
# the folder of the yaml under text/). An empty bank gives the yaml's own text: the same headers as without it.
# `storage: define` on a `strings:` block writes `#define SYMBOL "text"` (a literal where the C uses it, as the original
# literal was), so text that was a literal in the code can be keyed without changing the binary.

import argparse
import json
import os
import sys
from pathlib import Path

import yaml

__BANK = {'font': {}, 'text': {}}
# MOD (language): $TEXT_BANK_PAD, the bank of the other language of a two-language build (docs/mods/language.md). Each
# string is padded to the longer of its two texts and two strings share one array only when both their texts are equal
# in both languages, and the literals become arrays: the two builds then lay out every symbol at the same address and
# differ only in the bytes of the strings, so the game switches language by writing the other build's bytes over them.
__PAD = None
__PAD_ON = False

__LANGUAGES = {
    'jp': 'ja',
    'jp_trial': 'ja',
    'jp_bombom': 'ja',
    'jp_rev1': 'ja',
    'us': 'en_us',
    'eu': 'en_eu',
}

__FALLBACKS = {'en_eu': 'en_us'}


def __full_width(text):
    out = ''
    for c in text:
        if c == ' ':
            out += '　'
        elif c == "'":
            out += '’'
        elif '!' <= c <= '~':
            out += chr(ord(c) + 0xfee0)
        else:
            out += c
    return out


__CHARMAPS = {
    'ja': lambda text: text,
    'en_us': lambda text: text,
    'en_eu': __full_width,
}

__STORAGES = ('named', 'literal', 'embedded')
__TABLE_KEYS = {'storage', 'macro', 'lines', 'empty', 'entries'}


def __fail(where, message):
    sys.exit('{}: {}'.format(where, message))


def __c_bytes(text, where):
    n = 0
    for c in text:
        if c in __BANK['font']:
            n += 2
            continue
        try:
            n += len(c.encode('cp932'))
        except UnicodeEncodeError:
            __fail(where, 'no Shift-JIS character for {!r} (and no glyph of the mod font)'.format(c))
    return n


def __c_string(text, where, pad_to=0):
    out = ''
    for c in text:
        if c in __BANK['font']:
            code = int(__BANK['font'][c], 16)
            out += '\\{:03o}\\{:03o}'.format(code >> 8, code & 0xff)
            continue
        try:
            b = c.encode('cp932')
        except UnicodeEncodeError:
            __fail(where, 'no Shift-JIS character for {!r} (and no glyph of the mod font)'.format(c))
        if c in '"\\':
            out += '\\' + c
        elif c == '?' and out.endswith('?'):
            out += '\\?'
        elif len(b) == 1 and 0x20 <= b[0] < 0x7f:
            out += c
        else:
            out += ''.join('\\{:03o}'.format(x) for x in b)
    out += '\\000' * max(0, pad_to - __c_bytes(text, where))
    return '"' + out + '"'


def __check_keys(keys, allowed, where):
    for key in keys:
        language = key.split('@')[0]
        release = key.split('@')[1] if '@' in key else None
        if key in allowed:
            continue
        if language in __CHARMAPS and release in (None, *__LANGUAGES):
            continue
        __fail(where, 'unknown key {}'.format(key))


def __per_release(field, release, default, where):
    if field is None:
        return default
    if not isinstance(field, dict):
        __fail(where, 'expected a mapping of releases')
    return field.get(release, field.get('default', default))


def __text(entry, release, where):
    charmap = __CHARMAPS[__LANGUAGES[release]]
    language = __LANGUAGES[release]
    while language:
        for key in ('{}@{}'.format(language, release), language):
            if key in entry:
                value = entry[key]
                if isinstance(value, list):
                    return [None if v == 'NULL' else charmap(v)
                            for v in value]
                return None if value == 'NULL' else charmap(value)
        language = __FALLBACKS.get(language)
    __fail(where, 'no {} text'.format(__LANGUAGES[release]))


def __banked(key, value):
    # an entry with no text for this release stays without one (the same symbol may be another file's in this release)
    if key is not None and value is not None and key in __BANK['text']:
        return __BANK['text'][key]
    return value


def __banked_pad(key, value):
    if __PAD_ON and key is not None and value is not None and key in __PAD['text']:
        return __PAD['text'][key]
    return value


def __other_string(entry, release, where, key=None):
    """the other language's text of a string (a two-language build), else the string's own"""
    value = __banked_pad(key, __text(entry, release, where)) if __PAD_ON else __banked(key, __text(entry, release, where))
    if __PAD_ON and __PAD.get('identity'):
        original = __text(entry, release, where)
        if isinstance(original, str) and (not isinstance(value, str) or __c_bytes(original, where) > __c_bytes(value, where)):
            value = original
    return value if not isinstance(value, list) else None


def __other_lines(entry, release, count, where, key=None):
    value = __banked_pad(key, __text(entry, release, where)) if __PAD_ON else __banked(key, __text(entry, release, where))
    if not isinstance(value, list):
        value = [value]
    out = (value + [''] * count)[:count]
    if __PAD_ON and __PAD.get('identity'):
        original = __text(entry, release, where)
        original = original if isinstance(original, list) else [original]
        for n, old in enumerate(original[:count]):
            if isinstance(old, str) and (out[n] is None or __c_bytes(old, where) > __c_bytes(out[n], where)):
                out[n] = old
    return out


def __padded(text, other, where):
    """the C literal of text, padded with NULs to the length of the other language's text"""
    if not __PAD_ON or other is None:
        return __c_string(text, where)
    return __c_string(text, where, __c_bytes(other, where))


def __pair(text, other, identity):
    if __PAD_ON and __PAD.get("identity"):
        return (identity, None)
    return (text, other if __PAD_ON else None)


def __string(entry, release, where, key=None):
    value = __banked(key, __text(entry, release, where))
    if isinstance(value, list):
        __fail(where, 'expected one string, not lines')
    return value


def __lines(entry, release, count, where, key=None):
    value = __banked(key, __text(entry, release, where))
    if not isinstance(value, list):
        value = [value]
    if len(value) > count:
        __fail(where, '{} lines, but the table has {}'.format(
            len(value), count))
    return value + [''] * (count - len(value))


def __generate(release, blocks, source):
    header = []
    macros = {}
    named = {}
    folder = Path(source).parent.name

    def define(symbol, entry, where, key=None, as_macro=False):
        text = __string(entry, release, where, key)
        if text is None:
            return []
        other = __other_string(entry, release, where, key)
        literal = __padded(text, other, where)
        if as_macro:
            # MOD (language): still a literal (the C may initialise an array with it), with one NUL more than the longer
            # text, so the compiler never merges it with an equal literal (both languages keep the same layout)
            if __PAD_ON:
                literal = __c_string(text, where, max(__c_bytes(text, where), __c_bytes(other or '', where)) + 1)
            header.append('#define {} {}'.format(symbol, literal))
            return []
        pair = __pair(literal, other, key or where)
        if pair in named:
            header.append('#define {} {}'.format(symbol, named[pair]))
            return []
        named[pair] = symbol
        return ['char {}[] = {};'.format(symbol, literal)]

    for block, body in blocks.items():
        where = '{}: {}'.format(source, block)
        if not isinstance(body, dict):
            __fail(where, 'expected a mapping')

        if 'strings' in body:
            __check_keys(body, {'strings', 'storage'}, where)
            as_macro = body.get('storage') == 'define'
            lines = []
            for symbol, entry in body['strings'].items():
                where = '{}: {}'.format(source, symbol)
                __check_keys(entry, (), where)
                lines += define(symbol, entry, where, '{}.{}'.format(folder, symbol), as_macro)
            macros[block + '_TEXT'] = lines
            continue

        if 'entries' not in body:
            __check_keys(body, (), where)
            macros[block + '_TEXT'] = define(block, body, where, '{}.{}'.format(folder, block))
            continue

        __check_keys(body, __TABLE_KEYS, where)
        storage = __per_release(body.get('storage'), release, 'named',
                                where)
        if storage not in __STORAGES:
            __fail(where, 'unknown storage {}'.format(storage))
        count = int(__per_release(body.get('lines'), release, '1', where))
        macro = body.get('macro')
        if storage == 'embedded' and not macro:
            __fail(where, 'embedded storage needs a macro')
        if storage == 'named' and count > 1:
            __fail(where, 'named storage has one line per entry')

        if macro:
            if storage == 'embedded':
                header.append('#define {0}(name) {0}_##name'.format(macro))
            else:
                header.append('#define {}(name)'.format(macro))

        strings = []
        table = []
        for index, entry in enumerate(body['entries']):
            where = '{}: {}[{}]'.format(source, block, index)
            if entry == 'NULL':
                table.append(['NULL'] * count)
                continue
            if not isinstance(entry, dict):
                __fail(where, 'expected a mapping or NULL')
            __check_keys(entry, {'name', 'named'}, where)
            name = entry.get('name')

            key = '{}.{}.{}'.format(folder, block, index)
            if storage == 'embedded':
                if not name:
                    __fail(where, 'embedded entries need a name')
                text = __string(entry, release, where, key)
                if text is None:
                    continue
                literal = __c_string(text, where)
                header.append('#define {}_{} {},'.format(macro, name,
                                                         literal))
                continue

            named_here = release in entry.get('named', ())
            refs = []
            others = __other_lines(entry, release, count, where, key)
            for n, line in enumerate(__lines(entry, release, count, where, key)):
                if line is None:
                    refs.append('NULL')
                    continue
                literal = __padded(line, others[n], where)
                pair = __pair(literal, others[n], (key, n))
                if storage == 'literal' and line == '' and body.get('empty'):
                    symbol = body['empty']
                elif storage == 'literal' and not named_here and not __PAD_ON:
                    refs.append(literal)
                    continue
                elif pair in named:
                    refs.append(named[pair])
                    continue
                elif storage == 'literal' and not named_here:   # MOD (language): the literal as an array of its own
                    symbol = '{}_LANG_{}_{}'.format(block, index, n)
                    named[pair] = symbol
                    header.append('static char {}[] = {};'.format(symbol, literal))
                    refs.append(symbol)
                    continue
                elif not name and (key in __BANK['text'] or (__PAD_ON and key in __PAD['text'])):
                    symbol = '{}_BANK_{}'.format(block, index)      # a banked text no other entry has
                elif not name:
                    __fail(where,
                           'a new {} text needs a name'.format(release))
                else:
                    symbol = name
                if pair not in named:
                    named[pair] = symbol
                    strings.append('char {}[] = {};'.format(symbol, literal))
                refs.append(named[pair])
            table.append(refs)

        lines = []
        if storage != 'embedded':
            size = sum(len(refs) for refs in table)
            lines.append('char *{}[{}] = {{'.format(block, size))
            lines += ['\t{},'.format(', '.join(refs)) for refs in table]
            lines.append('};')
        macros[block + '_STRINGS'] = strings
        macros[block + '_TEXT'] = lines

    return header, macros


def __macro(name, lines):
    if not lines:
        return ['#define {}'.format(name)]
    return (['#define {} \\'.format(name)]
            + ['\t{} \\'.format(line) for line in lines[:-1]]
            + ['\t' + lines[-1]])


def __parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('release', choices=__LANGUAGES)
    parser.add_argument('yaml', type=Path)
    parser.add_argument('out', type=Path)

    return parser.parse_args()


def __main():
    args = __parse_args()
    bank = os.environ.get('TEXT_BANK') or 'text-bank.json'
    if os.path.exists(bank):
        with open(bank, encoding='utf-8') as f:
            __BANK.update(json.load(f))
    pad = os.environ.get('TEXT_BANK_PAD')
    if pad:
        global __PAD, __PAD_ON
        with open(pad, encoding='utf-8') as f:
            __PAD = json.load(f)
        __PAD_ON = True

    with open(args.yaml, encoding='utf-8') as f:
        blocks = yaml.load(f, Loader=yaml.BaseLoader)
    if not isinstance(blocks, dict):
        __fail(args.yaml, 'expected a mapping of blocks')

    header, macros = __generate(args.release, blocks, args.yaml)
    for name, lines in macros.items():
        header += __macro(name, lines)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(''.join(line + '\n' for line in header))


if __name__ == '__main__':
    __main()
