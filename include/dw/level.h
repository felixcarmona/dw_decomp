#ifndef DW_LEVEL_H
#define DW_LEVEL_H

/*
 * MOD (mega): Digimon levels (DigimonPara.level) and the Mega level above
 * Ultimate (docs/mods/mega.md). The original game has 1..5 and 0 for NPCs
 * that never evolve; the code compares with literals, so only the places that
 * had to learn about 6 use these names.
 */

#define LEVEL_FRESH		1
#define LEVEL_IN_TRAINING	2
#define LEVEL_ROOKIE		3
#define LEVEL_CHAMPION		4
#define LEVEL_ULTIMATE		5
#define LEVEL_MEGA		6

/* hours as an Ultimate before a Mega evolution is looked for (Champion: 144) */
#define MEGA_EVOLUTION_HOURS	120
/* lifetime added on reaching Mega naturally (Ultimate: +96) */
#define MEGA_LIFETIME_BONUS	120

#endif
