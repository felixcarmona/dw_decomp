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

/* Digimon types */
#define NUM_DIGIMON		180	/* DIGIMON_DATA, models, skeletons, voices */
#define NUM_PARTNER_DIGIMON	66	/* RAISE_DATA, cards; partners are 1..65 */
#define NUM_EVOLVING_DIGIMON	63	/* EVOLUTION_REQS (by type), raised flags; EVOLUTION_PATHS is by type - 1 */
#define NUM_EVO_CHART_DIGIMON	61	/* EVO_CHART_BOXES, by type - 1 */
#define NUM_RECRUITABLE_DIGIMON	0x70	/* recruited flags = trigger 200 + type, types 0..111 */

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
