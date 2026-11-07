#include <dw/memmap.h>

SECTIONS
{
    .ZH 0x80400800 : AT(0) {
        BUILD_DIR/src/main/chinese.c.o(.text.* .rodata .rodata.* .data.* .bss.* .sdata.* .sbss.*);
        . = ALIGN(., 4);
    }
    ASSERT(SIZEOF(.ZH) <= 0xF800, "Chinese runtime exceeds its extra-RAM module slot")

	OVERLAY OVL_BTL_BASE : NOCROSSREFS
	{
		.BTL {
			btl_START = .;

			KEEP(BUILD_DIR/asm/btl/data/texture.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/btl/data/model_0.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/btl/data/model_1.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/btl/data/model_2.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/btl/data/model_3.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			btl_HEADER = .;

			KEEP(BUILD_DIR/asm/btl/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/btl/battle_setup.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_main.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_shout.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_menu.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_hud.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_effect.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/btl/battle_main.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_setup.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_menu.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_shout.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_hud.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_effect.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/btl/battle_setup.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_main.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_shout.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_menu.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_hud.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_effect.c.o(.data.*);
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/src/btl/btl_bss_us.c.o(.data.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/btl/battle_setup.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_main.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_shout.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/command_menu.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/btl/battle_hud.c.o(.bss.*);
			. = ALIGN(., 4);

			/*
			 * MOD (resident code moved, docs/mods/memory.md "Code out of the executable"): code that only
			 * this overlay calls (tools/xref.py: no reference from the executable's own code), appended here
			 * so every address of the original overlay stays where it was. Its data stays in the executable.
			 */
			BUILD_DIR/src/main/battle_ui.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/main/battle_ui.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/btl.c.o(.text.modBattleFleeStart);
            BUILD_DIR/src/main/btl.c.o(.text.modBattleLoadFinish);
            BUILD_DIR/src/main/buff_model.c.o(.text.removeBuffModelObject);
            BUILD_DIR/src/main/item.c.o(.text.spawnDroppedItems);
            BUILD_DIR/src/main/map.c.o(.text.getRViewCopy);
            BUILD_DIR/src/main/map.c.o(.text.getViewportDistanceCopy);
            BUILD_DIR/src/main/script_common.c.o(.text.econPercent);
            . = ALIGN(., 4);

			/* Trailer */
			LONG(0);

			. = ALIGN(., 4);
			btl_END = .; /* MOD (quick battle start): the preload checksum covers the whole overlay */
		}
	}

	OVERLAY OVL_SHOP_BASE : NOCROSSREFS
	{
		.DGET {
			dget_START = .;
			dget_HEADER = .;

			KEEP(BUILD_DIR/asm/dget/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/dget/dget.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/dget/dget.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/dget/dget.c.o(.data.*);
			. = ALIGN(., 4);

			/* Trailer */
			LONG(0);

			. = ALIGN(., 4);
            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/script_common.c.o(.text.renderVerticalLine);
            . = ALIGN(., 4);

		}
	}

	OVERLAY OVL_FISH_BASE : NOCROSSREFS
	{
		.DOO2 {
			doo2_START = .;
			doo2_HEADER = .;

			KEEP(BUILD_DIR/asm/doo2/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/doo2/doo2.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/doo2/doo2.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/doo2/doo2.c.o(.data.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/doo2/doo2_bss_us.c.o(.data.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/doo2/doo2.c.o(.bss.*);
			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_DOOA_BASE : NOCROSSREFS
	{
		.DOOA {
			dooa_START = .;
			dooa_HEADER = .;

			KEEP(BUILD_DIR/asm/dooa/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/dooa/dooa.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/dooa/dooa.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/dooa/dooa.c.o(.data.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/dooa/dooa.c.o(.bss.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/dooa/dooa_bss_us.c.o(.data.*);
			. = ALIGN(., 4);

			/* Trailer */
			LONG(0);

			. = ALIGN(., 4);
            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/efe.c.o(.text.setEFEFlashOffset);
            BUILD_DIR/src/main/evolution.c.o(.text.reincarnatePartner);
            BUILD_DIR/src/main/map.c.o(.text.DOOA_getStoredDigimonY);
            BUILD_DIR/src/main/map.c.o(.text.DOOA_storeDigimonY);
            BUILD_DIR/src/main/partner.c.o(.text.setSomeDyingState);
            BUILD_DIR/src/main/partner_impl.c.o(.text.initializeReincarnatedPartner);
            BUILD_DIR/src/main/partner_impl.c.o(.text.setReincarnateStats);
            . = ALIGN(., 4);

		}
	}

	OVERLAY OVL_EVL_BASE : NOCROSSREFS
	{
		.EAB {
			eab_START = .;
			eab_HEADER = .;

			KEEP(BUILD_DIR/asm/eab/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/eab/eab.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/eab/eab.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/eab/eab.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/eab/eab_bss_us.c.o(.data.*);
			. = ALIGN(., 4);

			/* Trailer */
			LONG(0);
			. = ALIGN(., 4);

			BUILD_DIR/src/eab/eab.c.o(.bss.*);
			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_EVL_BASE : NOCROSSREFS
	{
		.ENDI {
			endi_START = .;
			endi_HEADER = .;

			KEEP(BUILD_DIR/asm/endi/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/endi/endi.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/endi/endi.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/endi/endi.c.o(.data.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/endi/endi.c.o(.bss.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/endi/endi_bss_us.c.o(.data.*);
			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_EVL_BASE : NOCROSSREFS
	{
		.EVL {
			evl_START = .;
			evl_HEADER = .;

			KEEP(BUILD_DIR/asm/evl/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/evl/evl.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/evl/evl.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/evl/evl.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/evl/evl_bss_us.c.o(.data.*);
			. = ALIGN(., 4);

			/* Trailer */
			LONG(0);
			. = ALIGN(., 4);

			BUILD_DIR/src/evl/evl.c.o(.bss.*);
			. = ALIGN(., 4);
            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/evolution.c.o(.text.evoFlag);
            BUILD_DIR/src/main/map.c.o(.text.downloadCLUT2);
            BUILD_DIR/src/main/map.c.o(.text.fadeoutCLUT2);
            BUILD_DIR/src/main/partner_impl.c.o(.text.initializeEvolvedPartner);
            BUILD_DIR/src/main/script_interp.c.o(.text.forceUpdateBGM);
            . = ALIGN(., 4);

		}
	}

	OVERLAY OVL_FISH_BASE : NOCROSSREFS
	{
		.FISH {
			fish_START = .;
			fish_HEADER = .;

			KEEP(BUILD_DIR/asm/fish/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/fish/fish.c.o(.text.*);
			BUILD_DIR/src/fish/fish_model.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/fish/fish.c.o(.rodata .rodata.*);
			BUILD_DIR/src/fish/fish_model.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/fish/fish.c.o(.data.*);
			BUILD_DIR/src/fish/fish_model.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/fish/fish_bss_us.c.o(.data.*);
			. = ALIGN(., 4);

			/* Trailer */
			LONG(0);
			. = ALIGN(., 4);

			BUILD_DIR/src/fish/fish.c.o(.bss.*);
			BUILD_DIR/src/fish/fish_model.c.o(.bss.*);
			. = ALIGN(., 4);
            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/vecmath.c.o(.text.mapToWorldPosition);
            BUILD_DIR/src/main/vecmath.c.o(.text.rotateVector);
            BUILD_DIR/src/main/vecmath.c.o(.text.transposeRefMatrix);
            . = ALIGN(., 4);

		}
	}

	OVERLAY OVL_KAR_BASE : NOCROSSREFS
	{
		.KAR {
			kar_START = .;
			kar_HEADER = .;

			KEEP(BUILD_DIR/asm/kar/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/kar/kar.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/kar/kar.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/kar/kar.c.o(.data.*);
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/src/kar/kar_bss_us.c.o(.data.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/kar/kar.c.o(.bss.*);
			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_MOV_BASE : NOCROSSREFS
	{
		.MOV {
			mov_START = .;
			mov_HEADER = .;

			KEEP(BUILD_DIR/asm/mov/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/mov/mov.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/mov/mov.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/mov/mov.c.o(.data.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/mov/mov_bss_us.c.o(.data.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/mov/mov.c.o(.bss.*);
			. = ALIGN(., 4);

			/*
			 * MOD (resident code moved, docs/mods/memory.md "Code out of the executable"): code that only
			 * this overlay calls (tools/xref.py: no reference from the executable's own code), appended here
			 * so every address of the original overlay stays where it was. libpress (the MDEC decoder) with its
			 * tables, and libcd's streaming functions (St*, CdRead2): mov.c unhooks their callbacks
			 * (DecDCToutCallback(NULL), StUnSetRing) before it returns.
			 */
			BUILD_DIR/asm/main/psyq/libpress.s.o(.text .text.*);
			. = ALIGN(., 4);
			BUILD_DIR/asm/main/psyq/libcd.s.o(.text.StSetRing .text.StClearRing .text.StUnSetRing .text.StSetStream
				.text.StFreeRing .text.StGetNext .text.StSetMask .text.StGetBackloc .text.StCdInterrupt
				.text.StCdInterrupt2 .text.data_ready_callback .text.init_ring_status .text.dma_execute
				.text.mem2mem .text.CdRead2 .text.DsDataCallback .text.DsReadyCallback);
			. = ALIGN(., 4);
			BUILD_DIR/asm/main/psyq/libgpu.s.o(.text.SetDefDispEnv);
			. = ALIGN(., 4);
			BUILD_DIR/asm/main/data/psyq/libpress.rodata.s.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/asm/main/data/psyq/libpress.data.s.o(.data .data.*);
			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_MURD_BASE : NOCROSSREFS
	{
		.MURD {
			murd_START = .;
			murd_HEADER = .;

			KEEP(BUILD_DIR/asm/murd/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/murd/murd.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/murd/murd.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/murd/murd.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/murd/murd_bss_us.c.o(.data.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/murd/murd.c.o(.bss.*);
			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_BTL_BASE : NOCROSSREFS
	{
		.STD {
			std_START = .;
			KEEP(BUILD_DIR/asm/std/data/texture.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/std/data/model_0.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/std/data/model_1.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/std/data/model_2.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/std/data/model_3.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			std_HEADER = .;

			KEEP(BUILD_DIR/asm/std/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/std/std_setup.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_main.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_hud.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_effect.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/std/std_setup.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_main.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_hud.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_effect.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/std/std_setup.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_main.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_hud.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_effect.c.o(.data.*);
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/src/std/std_bss_us.c.o(.data.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/std/std_setup.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_main.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_hud.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/std/std_effect.c.o(.bss.*);
			. = ALIGN(., 4);

			/* Trailer */
			LONG(0);

			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_TRN_BASE : NOCROSSREFS
	{
		.TRN2 {
			trn2_START = .;
			trn2_HEADER = .;

			KEEP(BUILD_DIR/asm/trn2/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/trn2/trn2_hp_map99.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map99.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hp_map107.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_mp.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_off.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map108.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_reward.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hud.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_slots.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/trn2/trn2_hp_map107.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hp_map99.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_mp.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_off.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_reward.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map108.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map99.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hud.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_slots.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/trn2/trn2_hp_map99.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map99.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hp_map107.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_mp.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_off.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map108.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_reward.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hud.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_slots.c.o(.data.*);
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/src/trn2/trn2_bss_us.c.o(.data.*));
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hp_map99.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map99.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hp_map107.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_mp.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_off.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_def_map108.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_reward.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_hud.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn2/trn2_slots.c.o(.bss.*);
			. = ALIGN(., 4);
		}
	}

	OVERLAY OVL_TRN_BASE : NOCROSSREFS
	{
		.TRN {
			trn_START = .;
			trn_HEADER = .;

			KEEP(BUILD_DIR/asm/trn/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/trn/trn_reward.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_hp.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_mp.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_off.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_def.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_speed.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_brain.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_hud.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_slots.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/trn/trn_hp.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_off.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_speed.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_def.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_mp.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_brain.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_reward.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_slots.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_hud.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/trn/trn_reward.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_hp.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_mp.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_off.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_def.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_speed.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_brain.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_hud.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_slots.c.o(.data.*);
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/src/trn/trn_bss_us.c.o(.data.*));
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_reward.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_hp.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_mp.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_off.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_def.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_speed.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_brain.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_hud.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/trn/trn_slots.c.o(.bss.*);
			. = ALIGN(., 4);
            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/overworld.c.o(.text.moveMapObjects);
            . = ALIGN(., 4);

		}
	}

	OVERLAY OVL_BTL_BASE : NOCROSSREFS
	{
		.VS {
			vs_START = .;
			KEEP(BUILD_DIR/asm/vs/data/texture.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/vs/data/model_0.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/vs/data/model_1.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/vs/data/model_2.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/asm/vs/data/model_3.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			vs_HEADER = .;

			KEEP(BUILD_DIR/asm/vs/data/header.rodata.s.o(.rodata.*));
			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_scene.c.o(.text.*);

			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_camera.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_main.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_hud.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_effect.c.o(.text.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_scene.c.o(.rodata .rodata.*);

			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_camera.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_main.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_hud.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_effect.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_scene.c.o(.data.*);

			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_camera.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_main.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_hud.c.o(.data.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_effect.c.o(.data.*);
			. = ALIGN(., 4);
			KEEP(BUILD_DIR/src/vs/vs_bss_us.c.o(.data.*));
			. = ALIGN(., 4);
			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_scene.c.o(.bss.*);

			. = ALIGN(., 4);

			BUILD_DIR/src/vs/vs_camera.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_main.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_hud.c.o(.bss.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_effect.c.o(.bss.*);
			. = ALIGN(., 4);

			/*
			 * MOD (resident code moved, docs/mods/memory.md "Code out of the executable"): code that only
			 * this overlay calls (tools/xref.py: no reference from the executable's own code), appended here
			 * so every address of the original overlay stays where it was. Its data stays in the executable.
			 */
			BUILD_DIR/src/vs/vs_intro.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_combat.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_intro.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_combat.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			/* the fighter selection (VS__initialize): modMenuVsBattle reads VS before it calls it */
			BUILD_DIR/src/vs/vs_select.c.o(.text.*);
			. = ALIGN(., 4);
			BUILD_DIR/src/vs/vs_select.c.o(.rodata .rodata.*);
			. = ALIGN(., 4);
			/* MOD (heap): read-only tables and paths that only this overlay reaches (tools/xref.py classes) */
			BUILD_DIR/src/vs/vs_select.c.o(
				.data.MAIN_D_8012F590 .data.DIGIMON_SPRITE_CLUT .data.MODE_SELECT_SPRITES .data.MAP_SELECT_SPRITES
				.data.VS__ROW_PANELS .data.STAT_OFFSETS .data.STAT_LIMITS .data.VS__PATH_STDDAT_TAISEN_F_TIM
				.data.VS__PATH_STDDAT_16TAISEN_TIM .data.VS__PATH_ETCDAT_SYSTEM_W_TIM .data.VS__PATH_STDDAT_TAISEN2_TIM
				.data.VS__PATH_STDDAT_TAISEN1_TIM .data.VS__PATH_ETCDAT_ETCTIM_BIN .data.VS__PATH_ETCNA_TITLE2_TIM
				.data.VS__PATH_STDDAT_TIME_TIM);
			BUILD_DIR/src/vs/vs_intro.c.o(.data.VS__INTRO_NAME_COLORS .data.VS__STAT_BAR_LIMITS);
			. = ALIGN(., 4);

			/* Trailer */
			LONG(0);

			. = ALIGN(., 4);
            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/sound.c.o(.text.VSLoadSounds);
            . = ALIGN(., 4);

		}
	}

	/*
	 * MOD (menu overlay, docs/mods/memory.md "The menu overlay"): main_menu.c's code that only runs while a card menu
	 * is open (the title's menus, new game and continue, the save and load screens of the title, the save machine,
	 * the sleep save and the game-clear save), read by loadMenuOverlay where each opens. The functions are named
	 * one by one: one this list does not name (a new one) stays in the executable (main.ld takes the rest of the
	 * file), which is always safe. Its data and bss stay in the executable (they outlive the overlay) unless named below.
	 */
	OVERLAY OVL_BTL_BASE : NOCROSSREFS
	{
		.MENU {
			menu_START = .;
			BUILD_DIR/src/main/main_menu.c.o(
				.text._strncpy
				.text.awardMachinedramonData
				.text.countRegisteredDigimon
				.text.createByteSum
				.text.createSavegameChecksum
				.text.drawMainMenuStrings
				.text.drawMoveName
				.text.drawOptionsText
				.text.drawRegisteredDigimonSlots
				.text.drawSaveSlotText
				.text.dropAbsentRegistered.constprop.0
				.text.getCardUsedBlockCount
				.text.getMenuOnCardChange
				.text.getSaveFileBytes
				.text.getSaveVersion
				.text.initializeDefaultSavegame
				.text.isExtensionValid
				.text.isMoveAbsent
				.text.loadSaveCopy
				.text.loadSaveSlotData
				.text.loadSavegame
				.text.modDrawSlot2.constprop.0
				.text.modDrawSlot2.part.0.constprop.0
				.text.modDrawSlotName
				.text.modOptionValue
				.text.modRegisterLong
				.text.modTitleNameTail .text.modTitleNames
				.text.modTitleOptions
				.text.modTitlePlayerName
				.text.pickSaveCopy
				.text.prepareSaveFile
				.text.putRecord
				.text.registerBattleData
				.text.renderCantRegister
				.text.renderCantRegisterBaby
				.text.renderCheckingCard
				.text.renderConfirmNoCard
				.text.renderConfirmOverwrite
				.text.renderConfirmRegister
				.text.renderConfirmSlotSelection
				.text.renderConfirmVSSlot
				.text.renderDoYouWantToSave
				.text.renderFormatMemoryCard
				.text.renderInitialMenu
				.text.renderMainMenu
				.text.renderMemoryCardReadError
				.text.renderMenuBox
				.text.renderMenuSelector
				.text.renderRegisterDigimon
				.text.renderSaveSlotBox
				.text.renderSelectCardSlot
				.text.renderSelectNewGameCard
				.text.renderSelectRegisterSlot
				.text.renderSelectSlot
				.text.renderSleepMenu
				.text.renderText
				.text.saveExtOf
				.text.saveExtSize
				.text.saveFileStart
				.text.setMemoryCardReadError
				.text.tickMainMenu
				.text.tickMenuInput
				.text.tickSelectSlotInput
				.text.updateMemoryCardState
				.text.writeSaveFile
				.text.writeSavegame
			);
			. = ALIGN(., 4);
			/* their switch tables and literals, which only they read */
			BUILD_DIR/src/main/main_menu.c.o(.rodata.tickMainMenu .rodata.renderMainMenu .rodata.drawMainMenuStrings
				.rodata.drawMainMenuStrings.str1.4 .rodata.loadSaveCopy .rodata.modOptionValue
                .rodata.MOD_FLAG_PALETTE .rodata.MOD_FLAG_OFFSET .rodata.MOD_FLAG_RUNS);
			. = ALIGN(., 4);
			/* MOD (heap): read-only data of the card menus that only this overlay reaches (tools/xref.py classes): their
			 * messages, labels and the save icon. Nothing that is ever written goes here: a reload would undo it */
			BUILD_DIR/src/main/main_menu.c.o(
				.data.PLAIN_LETTERS .data.SAVE_ICON_FRAME_1 .data.SAVE_ICON_FRAME_0 .data.MENU_VIEWS
				.data.MEMORY_CARD_ERROR_MESSAGES .data.SAVE_ICON_CLUT .data.MAIN_D_801315A0
				.data.MAIN_D_80131390 .data.MAIN_D_8013134C .data.MAIN_D_801311D4 .data.MAIN_D_80131580
				.data.MAIN_D_80131500 .data.MAIN_D_801313E4 .data.MAIN_D_80131264 .data.MAIN_D_80131200
				.data.MAIN_D_801311EC .data.MAIN_D_801315C0 .data.MAIN_D_8013154C .data.MAIN_D_801314B0
				.data.MAIN_D_8013147C .data.MAIN_D_80131460 .data.MAIN_D_80131290 .data.MAIN_D_80131218
				.data.SAVE_TITLE_RESERVED .data.MAIN_D_801315EC .data.MAIN_D_801315D0 .data.MAIN_D_80131538
				.data.MAIN_D_801314C8 .data.MAIN_D_8013141C .data.MAIN_D_80131400 .data.MAIN_D_801313B0
				.data.MAIN_D_801312FC .data.MAIN_D_80131568 .data.MAIN_D_8013151C .data.MAIN_D_801313D0
				.data.MAIN_D_80131278 .data.MAIN_D_8013119C .data.MAIN_D_80131150 .data.MAIN_D_801314E4
				.data.MAIN_D_80131228 .data.MAIN_D_8013118C .data.MAIN_D_8013111C .data.MAIN_D_801310CC
				.data.MEMORY_CARD_OPERATION_MESSAGES .data.MAIN_D_8013136C .data.MAIN_D_80131498 .data.MAIN_D_80131378
				.data.MAIN_D_80131328 .data.MAIN_D_80131318 .data.MAIN_D_801312A0 .data.MAIN_D_80131240
				.data.MAIN_D_801311B4 .data.MAIN_D_80131134 .data.MAIN_D_80131108 .data.MAIN_D_801312E4
				.data.MAIN_D_801312D0 .data.MAIN_D_801312B8 .data.MAIN_D_801311C0 .data.MAIN_D_80131090
				.data.MAIN_D_8013107C .data.MAIN_D_8013104C .data.MAIN_D_80131168 .data.MAIN_D_801310F4
				.data.MAIN_D_801310E0 .data.MAIN_D_80131234 .data.MAIN_D_80131178 .data.MAIN_D_8013106C
				.data.MAIN_D_8013103C .data.MAIN_D_80131144 .data.MAIN_D_801310A4 .data.MAIN_D_80131030
				.data.MAIN_D_80131254 .data.MAIN_D_8013105C .data.MAIN_D_80131024 .data.TITLE_MENU_ITEMS
				.data.MAIN_D_80131638 .data.MAIN_D_80131008 .data.BATTLE_MODE_ITEMS .data.STR_YES_NO_SPACED
				.data.STR_YES_NO .data.MAIN_D_801346A4 .data.MAIN_D_80131448 .data.MAIN_D_80131340
				.data.MAIN_D_80131454 .data.MAIN_D_8013143C .data.MAIN_D_801346C4 .data.MAIN_D_80131014
				.data.MAIN_D_8013468C
			);
			. = ALIGN(., 4);
            /* Player chart opens MENU before reaching its modal renderer (and the detail view's lists, built while it is open).
             * The cell writer and primitive/number helpers stay resident to reserve scrolling room below EVL. */
            BUILD_DIR/src/main/game_menu.c.o(
                .text.chartCardBuild .text.chartCardBuild.*
                .text.chartCardRender .text.chartCardRender.*
                .text.chartCardPrompt .text.chartCardPrompt.*
                .text.chartAppend .text.chartAppend.*
                .text.chartBuildLists .text.chartBuildLists.*
                .text.chartListAdd .text.chartListAdd.*
                .rodata.CHART_POPUP_AUTOMATIC
                .rodata.CHART_POPUP_BATTLES
                .rodata.CHART_POPUP_BONUS_ANY
                .rodata.CHART_POPUP_BONUS_ONE_OF
                .rodata.CHART_POPUP_CLOSE
                .rodata.CHART_POPUP_DISCIPLINE
                .rodata.CHART_POPUP_EVOLVE
                .rodata.CHART_POPUP_FROM
                .rodata.CHART_POPUP_HAPPINESS
                .rodata.CHART_POPUP_HAVE
                .rodata.CHART_POPUP_MISTAKES
                .rodata.CHART_POPUP_MISTAKES_ANY
                .rodata.CHART_POPUP_NEEDS
                .rodata.CHART_POPUP_NO
                .rodata.CHART_POPUP_READY
                .rodata.CHART_POPUP_SHORT
                .rodata.CHART_POPUP_STATS
                .rodata.CHART_POPUP_STATS_ANY
                .rodata.CHART_POPUP_STAT_BRN
                .rodata.CHART_POPUP_STAT_DEF
                .rodata.CHART_POPUP_STAT_HP
                .rodata.CHART_POPUP_STAT_MP
                .rodata.CHART_POPUP_STAT_OFF
                .rodata.CHART_POPUP_STAT_SPD
                .rodata.CHART_POPUP_TECHS
                .rodata.CHART_POPUP_TOP
                .rodata.CHART_POPUP_VERDICT
                .rodata.CHART_POPUP_VIEW
                .rodata.CHART_POPUP_WEIGHT
                .rodata.CHART_POPUP_YES
                .rodata.CHART_STAT_NAMES
            );
            . = ALIGN(., 4);

            /* Lab UI runs synchronously after the resident entry loads MENU.
             * The roster callbacks and state stay resident. */
            BUILD_DIR/src/main/mod_lab.c.o(.text.modLabFieldPtr .text.modLabFit .text.modLabBound
                .text.modLabClamp .text.modLabList .text.modLabPerform
                .text.modLabDrawText .text.modLabRowY .text.modLabRender
                .text.modLabMenuLoaded* .text.labInt
                .rodata.modLabMenuLoaded*);
            /* UI-only storage: the anonymous translated-literal pool, immutable
             * field/home tables and redraw cache. Roster captions, their level
             * table and all shared state remain resident. labText is scratch:
             * modLabMenuLoaded clears it before the first read on every entry;
             * the BIOS does not zero overlay bss. Card reloads occur only after
             * the lab's render callback has been removed. */
            BUILD_DIR/src/main/mod_lab.c.o(.rodata .rodata.LAB_FIELDS
                .rodata.LAB_HOME_TEXT .bss.labText);
            . = ALIGN(., 4);

			LONG(0);
			. = ALIGN(., 4);
            /* Resident helpers reached only by this loaded overlay (xref). */
            BUILD_DIR/src/main/main_menu.c.o(.text.modFlagPixel);
            BUILD_DIR/src/main/main_menu.c.o(.text.modFlagRect);
            BUILD_DIR/src/main/main_menu.c.o(.text.modOptionBytes);
            BUILD_DIR/src/main/main_menu.c.o(.text.modOptionCount* .text.setRegisteredSpecies*);
            BUILD_DIR/src/main/main_menu.c.o(.text.swapPartnerSpecies);
            BUILD_DIR/src/main/mod_options.c.o(.text.modFrameRate);
            BUILD_DIR/src/main/sjis.c.o(.text.asciiToShiftJIS);
            . = ALIGN(., 4);

		}
	}
}
