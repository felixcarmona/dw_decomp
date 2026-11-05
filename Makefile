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

VERSION_MACRO := VERSION_$(shell echo $(VERSION) | tr a-z A-Z)

# MOD (scale): bytes of the partner model buffer (include/dw/memmap.h, docs/mods/memory.md
# "Partner model buffer"). The original's is 0x18800; the default fits the largest imported
# model. The value goes through a generated header so a change rebuilds what depends on it.
PARTNER_MMD_MAX ?= 0x20000
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

# MOD (font): the font the US executable draws its text with (docs/mods/font.md "Fonts"), chosen at build time:
#   original  the US font on its 4-pixel grid: the original code and glyphs, byte for byte;
#   a         the US letters placed to the pixel (src/main/font_a.h, tools/fontgen.py);
#   b         our font redrawn in the game's style, placed to the pixel (src/main/font_b.h, tools/font/b.txt).
FONT ?= original
ifeq ($(filter $(FONT),original a b),)
$(error FONT=$(FONT): the font must be original, a or b)
endif
$(shell mkdir -p $(GEN_DIR); printf '#define FONT_%s 1\n' '$(shell echo $(FONT) | tr a-z A-Z)' > $(GEN_DIR)/font_sel.h.tmp; \
	cmp -s $(GEN_DIR)/font_sel.h.tmp $(GEN_DIR)/font_sel.h && rm $(GEN_DIR)/font_sel.h.tmp \
	|| mv $(GEN_DIR)/font_sel.h.tmp $(GEN_DIR)/font_sel.h)

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
