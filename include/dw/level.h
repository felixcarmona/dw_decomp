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

/*
 * MOD (evolution rules as data): the hours a partner of each level waits before it looks for an evolution, and the
 * other timing numbers, as plain int16s written by tools/balance.py build from content/tables/evolution-rules.json.
 * The build starts with every one at -1 (= the rule does not exist: nothing evolves by time) and the JSON puts them.
 *   MOD_EVO_TIMING[level] (1..5)  hours at that level (evoTimer) before its target is looked for
 *   MOD_EVO_ULTIMATE_AGAIN        an Ultimate with no special target looks for a Champion-style target from here
 *   MOD_EVO_MEGA                  hours as an Ultimate before a Mega is accepted (-1 = no wait)
 *   MOD_EVO_FALLBACK_HOURS/TARGET a Rookie with no candidate evolves into TARGET after HOURS (either -1: never)
 */
#define MOD_EVO_TIMING_SIZE	11
#define MOD_EVO_ULTIMATE_AGAIN	7
#define MOD_EVO_MEGA		8
#define MOD_EVO_FALLBACK_HOURS	9
#define MOD_EVO_FALLBACK_TARGET	10
extern int16_t MOD_EVO_TIMING[MOD_EVO_TIMING_SIZE];
/* lifetime added on reaching Mega naturally (Ultimate: +96) */
#define MEGA_LIFETIME_BONUS	120

#endif
