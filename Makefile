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
