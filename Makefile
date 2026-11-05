-include local.mk

VERSION ?= us
TOOLCHAIN ?= mwcc
PLATFORM ?= psx

VERSIONS := jp jp_trial jp_bombom us jp_rev1 eu
CONFIG := $(PLATFORM)/$(TOOLCHAIN)
CONFIGS := psx/mwcc psx/gcc
MATCHING_CONFIG := psx/mwcc

ifeq ($(filter $(VERSION),$(VERSIONS)),)
$(error unsupported VERSION $(VERSION); supported: $(VERSIONS))
endif

ifeq ($(filter $(CONFIG),$(CONFIGS)),)
$(error unsupported PLATFORM/TOOLCHAIN $(CONFIG); supported: $(CONFIGS))
endif

BUILD_DIR := build/$(VERSION)/$(CONFIG)
DISK_DIR := disks/$(VERSION)
CONFIG_DIR := config/$(VERSION)
ASM_DIR := asm/$(VERSION)
GEN_DIR := build/$(VERSION)/$(PLATFORM)/generated

VERSION_MACRO := VERSION_$(shell echo $(VERSION) | tr 'a-z-' 'A-Z_')

# MOD (scale): bytes of the partner model buffer (include/dw/memmap.h, docs/mods/memory.md
# "Partner model buffer"). The original's is 0x18800, the default: tools/build-full.sh passes the
# disc's largest model (0x18800 for every disc since the imports were fitted), and since the
# chart's criteria card (patch 0176) 0x20000 leaves less than MIN_HEAP. The value goes through
# a generated header so a change rebuilds what depends on it.
PARTNER_MMD_MAX ?= 0x18800
$(shell mkdir -p $(GEN_DIR); printf '#define PARTNER_MMD_MAX %s\n' '$(PARTNER_MMD_MAX)' > $(GEN_DIR)/partner_mmd_max.h.tmp; \
	cmp -s $(GEN_DIR)/partner_mmd_max.h.tmp $(GEN_DIR)/partner_mmd_max.h && rm $(GEN_DIR)/partner_mmd_max.h.tmp \
	|| mv $(GEN_DIR)/partner_mmd_max.h.tmp $(GEN_DIR)/partner_mmd_max.h)

# MOD (fps): the frame rate the build presents, 20, 30 or 60 fps (docs/mods/60fps.md); the
# game logic runs at 20 Hz in all three. Chosen here, at build time only (no menu option).
# The default below is the one line to change (docs/open-items.md: 60 or 30, pending).
FPS ?= 60
ifeq ($(filter $(FPS),20 30 60),)
$(error FPS=$(FPS): the frame rate must be 20, 30 or 60)
endif
$(shell mkdir -p $(GEN_DIR); printf '#define FPS %s\n' '$(FPS)' > $(GEN_DIR)/fps.h.tmp; \
	cmp -s $(GEN_DIR)/fps.h.tmp $(GEN_DIR)/fps.h && rm $(GEN_DIR)/fps.h.tmp \
	|| mv $(GEN_DIR)/fps.h.tmp $(GEN_DIR)/fps.h)

# MOD (font): the fonts the US executable draws its text with (docs/mods/font.md "Fonts"):
#   original  the US font on its 4-pixel grid: the original code and glyphs, byte for byte;
#   b         our font redrawn in the game's style, placed to the pixel (src/main/font_b.h, tools/font/b.txt), and the
#             original one too: the title menu's OPTIONS (NEW FONT) chooses between them at run time (patch 0173).
# (FONT=a, the US letters placed to the pixel, was a test font: left out by patch 0173.)
FONT ?= b
ifeq ($(filter $(FONT),original b),)
$(error FONT=$(FONT): the font must be original or b)
endif
$(shell mkdir -p $(GEN_DIR); printf '#define FONT_%s 1\n' '$(shell echo $(FONT) | tr 'a-z-' 'A-Z_')' > $(GEN_DIR)/font_sel.h.tmp; \
	cmp -s $(GEN_DIR)/font_sel.h.tmp $(GEN_DIR)/font_sel.h && rm $(GEN_DIR)/font_sel.h.tmp \
	|| mv $(GEN_DIR)/font_sel.h.tmp $(GEN_DIR)/font_sel.h)

# MOD (language): the language the executable's own strings are in at boot (docs/mods/language.md): en or es. A
# two-language disc carries the other one as a patch (LANGEN.BIN / LANGES.BIN) and OPTIONS (LANGUAGE) switches.
LANG_BASE ?= en
ifeq ($(filter $(LANG_BASE),en es pt-br fr de it ja),)
$(error LANG_BASE=$(LANG_BASE): en, es, pt-br, fr, de, it or ja)
endif
# LANG_BUILD: the language of this build's strings and language tables (tools/build-full.sh builds the tree twice, once
# per language; the two builds differ only in data, tools/langpatch.py checks it). Default: LANG_BASE.
LANG_BUILD ?= $(LANG_BASE)
$(shell mkdir -p $(GEN_DIR); printf '#define MOD_LANG_BUILD MOD_LANG_%s\n' '$(shell echo $(LANG_BUILD) | tr 'a-z-' 'A-Z_')' > $(GEN_DIR)/lang_build.h.tmp; \
	cmp -s $(GEN_DIR)/lang_build.h.tmp $(GEN_DIR)/lang_build.h && rm $(GEN_DIR)/lang_build.h.tmp \
	|| mv $(GEN_DIR)/lang_build.h.tmp $(GEN_DIR)/lang_build.h)
$(shell mkdir -p $(GEN_DIR); printf '#define MOD_LANG_BASE MOD_LANG_%s\n' '$(shell echo $(LANG_BASE) | tr 'a-z-' 'A-Z_')' > $(GEN_DIR)/lang_base.h.tmp; \
	cmp -s $(GEN_DIR)/lang_base.h.tmp $(GEN_DIR)/lang_base.h && rm $(GEN_DIR)/lang_base.h.tmp \
	|| mv $(GEN_DIR)/lang_base.h.tmp $(GEN_DIR)/lang_base.h)

# MOD (language): the disc files that have a copy per language (docs/mods/language.md), given by tools/build-full.sh:
# LANG_FILES="NAME.EXT:ES.EXT:EN.EXT ..." (the Spanish copy, the copy with the map signs in English; - for none). Each
# copy is also a NEW_FILES name. Empty by default: no copies, the files as before. A row is the name's hash and the
# copies' places among the new files (tools/gen_lang_files.py: 8 bytes, not three names).
LANG_FILES ?=

# MOD (new files): the disc files with names the original never had (a new screen's .MAP/.TFS, models, textures), one
# FILE_OFFSET_TABLE entry each after the mod entries (src/main/file_table_us.c), sector and size written by the project's
# tools/build-disc.py --relayout. NEW_FILES="RIFT01.MAP RIFT01.TFS ..." (8.3 upper-case names, given by tools/build-full.sh
# from the content); empty by default: the table and the executable as before. About 24 bytes of .data per name.
NEW_FILES ?=
$(shell mkdir -p $(GEN_DIR); printf '%s\n' $(foreach n,$(NEW_FILES),'{ 0x00000000, 0x00000800, "$(n)" },') > $(GEN_DIR)/file_table_new.h.tmp; \
	cmp -s $(GEN_DIR)/file_table_new.h.tmp $(GEN_DIR)/file_table_new.h && rm $(GEN_DIR)/file_table_new.h.tmp \
	|| mv $(GEN_DIR)/file_table_new.h.tmp $(GEN_DIR)/file_table_new.h)
$(shell mkdir -p $(GEN_DIR); printf '#define NUM_NEW_FILES %s\n' '$(words $(NEW_FILES))' > $(GEN_DIR)/file_table_count.h.tmp; \
	cmp -s $(GEN_DIR)/file_table_count.h.tmp $(GEN_DIR)/file_table_count.h && rm $(GEN_DIR)/file_table_count.h.tmp \
	|| mv $(GEN_DIR)/file_table_count.h.tmp $(GEN_DIR)/file_table_count.h)
$(shell python3 tools/gen_lang_files.py $(GEN_DIR)/lang_files.h --lang '$(LANG_FILES)' --new '$(NEW_FILES)' --table src/main/file_table_us.c >&2)

PYTHON := python3

.DEFAULT_GOAL := all

include mk/sources.mk
include mk/version/$(VERSION).mk
include mk/toolchain/$(TOOLCHAIN).mk
include mk/platform/$(PLATFORM).mk
include mk/text.mk

ifeq ($(CONFIG),$(MATCHING_CONFIG))
include mk/matching.mk
else
compare expected objdiff report:
	$(error $@ needs PLATFORM/TOOLCHAIN $(MATCHING_CONFIG))
endif

.EXTRA_PREREQS := $(abspath $(MAKEFILE_LIST))

regenerate: reset
	$(MAKE) generate

clean:
	rm -rf $(BUILD_DIR)

reset: clean
	rm -rf $(ASM_DIR) $(GEN_DIR)

-include $(DEP)

.PHONY: all generate regenerate clean reset compare expected objdiff report
