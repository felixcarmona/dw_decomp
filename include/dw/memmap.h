#ifndef DW_MEMMAP_H
#define DW_MEMMAP_H

/*
 * The fixed memory map of the game (docs/mods/ids.md § 9, memory.md).
 *
 * Plain #defines and nothing else: the linker scripts include this file too
 * (psx.mk runs every .ld through the C preprocessor), so it must not
 * contain C declarations, casts or suffixes.
 */

/* Load area: overlays, map backgrounds, EFE heap (512 KB) */
#define LOAD_AREA_START		0x80010000
#define LOAD_AREA_END		0x80090000

/* PS-X EXE header and the main executable */
#define EXE_HEADER_ADDR		0x80090000

/*
 * Stack (_stack_addr / _stack_size, in KB) and the malloc3 heap between.
 * The original reserved 64 KB of stack; the deepest use measured in this
 * build is STACK_MEASURED_MAX bytes (a battle with two enemies and their
 * effects, Secret Beach Cave; the cup, menus and map loads stay under
 * 1.7 KB; memory.md "Stack"). MOD (heap): 16 KB, the 4x margin the link
 * asserts, and the other 48 KB go to the heap (was 20 KB; re-measured
 * 2026-10-24 on the series tip: a two-enemy battle at x2 with a finisher,
 * the triangle menu's screens, map changes, a save from bed, the trade
 * menu all stay above 0x801FEE00, under 4352 bytes). A canary at the stack
 * limit counts overflows (MOD_STACK_GUARD_HITS, main.c).
 */
#define STACK_ADDR		0x801FFF00
#define STACK_SIZE_KB		0x10
#define STACK_MEASURED_MAX	0xF78
/*
 * The heap the game needs in its worst case: the MAPHEAD setup path that
 * takes the most (tools/membudget.py worst --trimmed: File City 198-203,
 * Betamon + Kunemon + Megadramon, 157680 bytes with NPC models trimmed on
 * load and the loading peak of the last one; 185304 bytes with whole
 * models, Ogre Fortress 48), rounded up. The link fails if main's bss
 * leaves less than this.
 */
#define MIN_HEAP		0x26800

/* Overlay bases (config/us/overlay.ld) */
#define OVL_MOV_BASE		0x80010000	/* videos */
#define OVL_BTL_BASE		0x80052AE0	/* BTL, STD, VS: TIM + TMD prefix, then code */
#define OVL_KAR_BASE		0x80053800
#define OVL_EVL_BASE		0x80060000	/* EVL, ENDI, EAB */
#define OVL_FISH_BASE		0x80070000	/* FISH, DOO2 */
#define OVL_MURD_BASE		0x8007C000
#define OVL_DOOA_BASE		0x80080000
#define OVL_SHOP_BASE		0x80080800	/* DGET (shop_START) */
#define OVL_TRN_BASE		0x80088000	/* TRN, TRN2: include one free scratch sector below TEXTURE_BUFFER */
/* TRN/TRN2 must end here: the original's largest, in sectors (memory.md) */
#define OVL_TRN_MAX_END		0x8008F800

/* Fixed buffers in the load area */
#define GENERAL_BUFFER_ADDR	0x80010000	/* .MAP then .TFS, VHB staging, EFE base */
#define DOOA_WORK_OFFSET	0x4B000		/* GENERAL_BUFFER + this: DOOA work area */
/*
 * Map background streaming (MOD, src/main/tfs_stream.c, docs/mods/memory.md):
 * the .TFS header (palettes) in its first two sectors, then a ring of
 * TFS_RING_SLOTS tile slots of 9 sectors (a 128x128 8 bpp tile is 0x4004
 * bytes in the file and never sector-aligned). The rest of the old TFS area
 * is free in the world (WORLD_SCRATCH_*): every mode that writes there
 * (battle, evolution, death, menus, sound staging) is followed by a
 * readMapTFS, the same as when the whole TFS lived there.
 * Nothing in the world may read the header after the screen is set up: sound
 * banks and music fonts are staged over it at any time (the lab's roster
 * switch, a script's music change), and the ring only notices. The palettes
 * the time of day loads are a resident copy (MOD_TFS_CLUTS, tfs_stream.h).
 */
#define TFS_HEADER_ADDR		0x80010000
#define TFS_HEADER_SIZE		0x1000
#define TFS_RING_ADDR		0x80011000
#define TFS_SLOT_SIZE		0x4800
#define TFS_RING_SLOTS		8
#define TFS_RING_END		0x80035000	/* TFS_RING_ADDR + 8 * TFS_SLOT_SIZE */
/* free in the world on every screen; above it FISH (0x80070000) and KAR
 * (0x80053800) coexist with their screens' maps */
#define WORLD_SCRATCH_START	TFS_RING_END
#define WORLD_SCRATCH_END	0x80053800
/*
 * MOD (save extension area, docs/mods/memory.md "Save extensions"): the extensions of a save
 * file (file offsets 0x2000 on: v2 two of 0x1000, tier 2's v3 two of 0x2000) are read and written
 * here, not in SAVE_FILE. SAVE_FILE stays the original's resident v1 file (0x2000): its battle
 * registrations live in it between card operations. An extension is only used from a card read to
 * loadSaveCopy and from prepareSaveFile to the end of the write, inside one tick of the card menu
 * (the title's or the save machine's in the world), when this part of the world scratch is free:
 * the title's TIMs end at 0x8002F000, the world keeps only the TFS ring below WORLD_SCRATCH_START.
 * It is above every sound bank staged at GENERAL_BUFFER (the largest: an ESALL chunk of 124 sectors,
 * to 0x8004E000; SB.VHB to 0x8004D000), the evolution model area (EVO_MMD_ADDR, to 0x80048800) and
 * DOOA_SHARD, and below the battle prefix (0x80052AE0). (A battle's EFE heap covers it: no card
 * operation runs in a battle.)
 */
/* NPC CD reads are sector-rounded; compact in field scratch, then copy to
 * the heap. Never retain a pointer here. Drained sound staging may reuse it. */
#define NPC_STAGE_ADDR WORLD_SCRATCH_START
#define NPC_STAGE_MAX  0x19000
#define SAVE_EXT_ADDR		0x8004E000
/*
 * MOD (save robustness): the most a sound bank staged at GENERAL_BUFFER may take (a VHB chunk or file, in
 * whole sectors: the original's largest, ESALL's 124 sectors), so it ends at or below SAVE_EXT_ADDR (main.ld
 * checks it; tools/sound.py refuses a bigger chunk and tools/build-full.sh reads this line).
 */
#define SOUND_STAGING_MAX	0x3E000
#define SAVE_EXT_MAX		0x4000
#define TEXTURE_BUFFER_ADDR	0x80088800	/* temporary TIM, card image, script arrays */
#define TEXTURE_BUFFER_SIZE	0x7800
#define BOSS_EFE_TMD_BUFFER_ADDR 0x80058000
#define ARENA_TMD_ADDR		0x80038000	/* STD/VS arena model */
#define EVO_MMD_ADDR		0x80020000	/* EVL and DOOA evolution / egg model */
#define DOOA_SHARD_ADDR		0x80044800
#define DOOA_SHARD_SIZE		0x5DC0
#define OVL_OT_0_ADDR		0x8008C000	/* DOOA and MURD ordering tables */
#define OVL_OT_1_ADDR		0x8008E000

/* Battle prefix: the TIM and TMDs at the start of BTL_REL, STD_REL, VS_REL */
#define BATTLE_FINISHER_TIM_ADDR	0x80052AE0
#define BATTLE_FINISHER_MODEL_ADDR	0x80053800
#define BATTLE_CONFUSION_MODEL_ADDR	0x80054838
#define BATTLE_STUN_MODEL_ADDR		0x80054D00
#define BATTLE_BUFF_MODEL_ADDR		0x80055328

/* Sizes of buffers that receive files (largest original in a comment) */
/*
 * The partner model buffer (PARTNER_MODEL_BUFFER, main.c). The original's was
 * 0x18800 (ETEM.MMD 0x17E14). The size comes from the build (Makefile:
 * PARTNER_MMD_MAX, default 0x20000 = 128 KB; 0x24000 = 144 KB is the most the
 * link-time MIN_HEAP assert leaves) through a generated header; every byte over 0x18800 is a
 * byte less of heap on every screen (docs/mods/memory.md, "Partner model
 * buffer"). The disc tools check the models against the same number.
 */
#include <partner_mmd_max.h>
#define TAMER_MMD_MAX		0xE000	/* BOYS.MMD 0xDECC (GCC: one array, mod 0002) */
#define MAP_SCRIPT_MAX		0x4000	/* a DG.SCN entry; the largest is 0x3800 */
/*
 * MOD (MAPHEAD buffer, docs/mods/memory.md "MAPHEAD buffer"): the offset table
 * of DG.SCN is its first sector (u32 offsets, at most 511 entries + the end;
 * entry 0 starts at 0x800, the DWS assembler keeps the table in that sector). The
 * original read 0x2000 bytes at boot: the other 0x1800 were the start of entry
 * 0, a copy of MAPHEAD.SCN nothing reads. They go to MAPHEAD.SCN, which stays
 * resident from boot (shared dialogue, shops, the new-game questions) and is
 * read in whole sectors: 0x6800 bytes, 26 KB (the original 25 000). The eight
 * languages take at most 24 554 bytes; the build refuses a file that does not fit.
 */
#define SCRIPT_OFFSET_TABLE_SIZE 0x800
#define MAPHEAD_SCRIPT_MAX	0x6800
/*
 * The SEQ block of the current music font (every track of it), copied from
 * the FAALL chunk (sound.c). Original 0x5800 (font 0xB uses 0x55F8); the
 * extra 0x2800 bytes come out of the malloc3 heap (MIN_HEAP assert below).
 */
#define SEQ_BUFFER_SIZE		0x8000

/*
 * SPU RAM (512 KB, sound.c): what the sound banks may use. Below it the
 * libspu capture buffers and silence block, above it the STUDIO_B reverb
 * work area (0x4840 bytes).
 */
#define SPU_BANKS_START		0x1010
#define SPU_REVERB_START	0x7B7C0

#endif
