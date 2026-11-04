"""Compact FileEntry rows; actual sectors and sizes are filled by the disc writer."""
import argparse,pathlib
from gen_lang_files import fnv1a
parser=argparse.ArgumentParser();parser.add_argument('output');parser.add_argument('--new',default='');args=parser.parse_args()
text=''.join('{ 0, 0x800, 0x%08X }, /* %s */\n'%(fnv1a(name),name) for name in args.new.split())
p=pathlib.Path(args.output)
if not p.exists() or p.read_text()!=text:p.write_text(text)
