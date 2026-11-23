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

/* Stack (_stack_addr / _stack_size, in KB) and the malloc3 heap between */
#define STACK_ADDR		0x801FFF00
#define STACK_SIZE_KB		0x40
/*
 * The heap the game needs in its worst measured case (Native Forest with
 * five wild Digimon: 197520 - 91080 = 106440 bytes used, memory.md),
 * rounded up. The link fails if main's bss leaves less than this.
 */
#define MIN_HEAP		0x1A000

/* Overlay bases (config/us/overlay.ld) */
#define OVL_MOV_BASE		0x80010000	/* videos */
#define OVL_BTL_BASE		0x80052AE0	/* BTL, STD, VS: TIM + TMD prefix, then code */
#define OVL_KAR_BASE		0x80053800
#define OVL_EVL_BASE		0x80060000	/* EVL, ENDI, EAB */
#define OVL_FISH_BASE		0x80070000	/* FISH, DOO2 */
#define OVL_MURD_BASE		0x8007C000
#define OVL_DOOA_BASE		0x80080000
#define OVL_SHOP_BASE		0x80080800	/* DGET (shop_START) */
#define OVL_TRN_BASE		0x80088800	/* TRN, TRN2 */
/* TRN/TRN2 must end here: the original's largest, in sectors (memory.md) */
#define OVL_TRN_MAX_END		0x8008F800

/* Fixed buffers in the load area */
#define GENERAL_BUFFER_ADDR	0x80010000	/* .MAP then .TFS, VHB staging, EFE base */
#define DOOA_WORK_OFFSET	0x4B000		/* GENERAL_BUFFER + this: DOOA work area */
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
#define PARTNER_MMD_MAX		0x18800	/* ETEM.MMD 0x17E14 */
#define TAMER_MMD_MAX		0xE000	/* BOYS.MMD 0xDECC (GCC: one array, mod 0002) */
#define MAP_SCRIPT_MAX		0x4000	/* a DG.SCN entry; the largest is 0x3800 */
#define SCRIPT_OFFSET_TABLE_SIZE 0x2000	/* start of DG.SCN (offset table), read at boot */

#endif
