#ifndef DW_IDS_H
#define DW_IDS_H

/*
 * Id types and counts of the game content (docs/mods/ids.md, § 8.1).
 *
 * The counts are those of the original disc. Every table and loop that
 * means "one per Digimon / screen / item / ..." uses these names, so new
 * content changes one number here (and the tables) instead of literals
 * spread over the code. The typedefs are the widths of the carriers in RAM;
 * the save keeps the v1 u8 layout until step 4 of the plan (ids.md § 10).
 */

#include <dw/types.h>

typedef int16_t DigimonId; /* -1 = none; 0 = the player (HIRO) */
typedef int16_t ScreenId;  /* -1 = none */
typedef int16_t ItemId;    /* -1 = none (0xFF in the old u8 layouts) */
typedef uint16_t MoveId;
typedef uint16_t CardId;

/* id meaning "none" in the u8 layouts (inventory slots, item ids, ...) */
#define LEGACY_ID_NONE 0xFF

/* no item in an ItemId carrier (inventory slot, chest, item on the ground) */
#define ITEM_NONE (-1)

/* Digimon types */
#define NUM_ORIGINAL_DIGIMON	180	/* the disc's species: DIGIMON_DATA, models, skeletons, voices, ALLTIM */
#define NUM_DIGIMON		256	/* MOD (species): id capacity; per-partner tables; DIGI.DAT ids < this */
#define NUM_PARTNER_DIGIMON	66	/* RAISE_DATA, cards; partners are 1..65 */
#define NUM_EVOLVING_DIGIMON	63	/* EVOLUTION_REQS (by type), raised flags; EVOLUTION_PATHS is by type - 1 */
#define NUM_EVO_CHART_DIGIMON	61	/* EVO_CHART_BOXES, by type - 1 */
#define NUM_RECRUITABLE_DIGIMON	0x70	/* recruited flags = trigger 200 + type, types 0..111 */

/*
 * MOD (master-tyrannomon): MasterTyrannomon is a partner too. Its per-partner
 * data lives at index 124 of tables sized NUM_DIGIMON (RAISE_DATA,
 * EVOLUTION_REQS, EVOLUTION_STATS_GAINS, ITEM_TAKE_DISTANCE, the VLALL voice
 * ids, the training animation ids); it reuses Tyrannomon's animations,
 * techniques and partner voice. Its "raised" flag is trigger 0x200 + 124 = 636,
 * a free trigger (scripts.md, "Free triggers"); the flags of the types between
 * NUM_EVOLVING_DIGIMON and 124 are used by scripts and stay out of range.
 */
#define DIGIMON_TYRANNOMON		8
#define DIGIMON_MASTER_TYRANNOMON	124
/*
 * MOD (cherrymon): Cherrymon is a partner too, reached from Vegiemon, with
 * its own model, animations and techniques (the Earth ones and Bit Bomb).
 * Its per-partner rows are at index 121 of the same tables; its "raised"
 * flag is trigger 0x200 + 121 = 633, also free (627-639).
 */
#define DIGIMON_VEGIEMON		25
#define DIGIMON_PIXIMON			55
#define DIGIMON_CHERRYMON		121
/*
 * MOD (partners-npc-a): King of Sukamon is a partner too, reached from
 * Sukamon, with Sukamon's animations and techniques on its own (scaled)
 * model. Its rows are at index 120 of the same tables; its raised flag is
 * trigger 0x200 + 120 = 632 (free).
 */
#define DIGIMON_SUKAMON			39
#define DIGIMON_KING_SUKAMON		120
/*
 * MOD (partners-npc-a): Icemon is a partner too, reached from Frigimon, on
 * its own model with a full animation set. Its raised flag would be
 * trigger 0x200 + 94 = 606, which script 221 uses ("X came to the city"),
 * so it has none (hasDigimonRaised returns 0 for it).
 */
#define DIGIMON_FRIGIMON		23
#define DIGIMON_GOTSUMON		68
#define DIGIMON_ICEMON			94
/*
 * MOD (partners-npc-a): Meteormon is a partner too, reached from
 * Monochromon, on its own model (the engine draws type 113 1.5 times
 * larger). No raised flag either: 0x200 + 113 = 625 is script 221's.
 */
#define DIGIMON_MERAMON			9
#define DIGIMON_MONOCHROMON		47
#define DIGIMON_METEORMON		113
/* MOD (partners-npc-a): natural evolutions for the item-only partners */
#define DIGIMON_PANJYAMON		63
#define DIGIMON_GIGADRAMON		64
#define DIGIMON_METALETEMON		65
#define HAS_RAISED_FLAG(type)                                                 \
	((uint32_t)(type) < NUM_EVOLVING_DIGIMON || (type) == DIGIMON_CHERRYMON || \
	 (type) == DIGIMON_MASTER_TYRANNOMON || (type) == DIGIMON_KING_SUKAMON)

/* Screens (maps) */
#define NUM_SCREENS		255	/* MAP_ENTRIES; screen ids are u8 */
/*
 * Value of CURRENT_SCREEN/PREVIOUS_SCREEN before the first map is loaded
 * (initializeMap) and the start section of a new game (initializeDefaultSavegame). It is
 * a real screen (TWNA01, File City) and not compared anywhere: renderMap
 * reads MAP_ENTRIES[CURRENT_SCREEN] before the first loadMap, so it must
 * stay a valid row and is not changed to -1 (ids.md, step 3).
 */
#define SCREEN_LEGACY_NONE	0xCC

/* Items */
#define NUM_ITEMS		128	/* ITEM_PARA, ITEM_FUNCTIONS, icons (16x8 grid) */

/* Techniques */
#define NUM_MOVES		122	/* MOVE_DATA rows; row 121 is the flattened Digimon's projectile */
#define NUM_MOVE_NAMES		121	/* MOVE_NAMES; drawMoveName checks this bound */
#define NUM_LEARNABLE_MOVES	58	/* MOVE_LEARN_CHANCES, the technique grid */

/* Cards */
#define NUM_CARDS		66	/* CARD_DATA, CARD_SPRITE_CLUT, CARD.ALL */

/* Trigger ranges used as per-id bitsets (scripts.md, "Free triggers") */
#define TRIGGER_RECRUITED_BASE	200	/* + Digimon type */
#define TRIGGER_SHOP_STOCK_BASE	0x180	/* + item id */
#define TRIGGER_RAISED_BASE	0x200	/* + Digimon type */

/*
 * The current and previous screen, 16-bit since step 3 of ids.md § 10.
 * The v1 save keeps them as u8 (exact while screen ids are < 255).
 */
extern ScreenId CURRENT_SCREEN;
extern ScreenId PREVIOUS_SCREEN;

#endif
