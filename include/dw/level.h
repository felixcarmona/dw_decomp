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

/*
 * MOD (evolution rules as data): the special evolutions that were if-chains. All of them start empty (-1) in the C;
 * tools/balance.py build writes them from content/tables/evolution-rules.json.
 *   MOD_EVO_FRESH[]     a Fresh (Baby I) becomes the Baby II of the pair {from, to}
 *   MOD_EVO_ULTIMATE[]  an Ultimate {from, to} looks for `to` when its requirements score 3
 *   MOD_EVO_RULES[]     rules checked after waking (mode 2) and after praise or scolding (mode 3)
 *   MOD_EVO_SPECIAL[]   the virus-bar evolution (threshold, target, two species it does not apply to), the Champion hour
 *                       that triggers the mode 3 check (level, hours), the scolding that arms the flag (level,
 *                       discipline, happiness before it)
 */
#define MOD_EVO_PAIR_MAX	8
#define MOD_EVO_RULES_MAX	10
#define MOD_EVO_COND_MAX	4
#define MOD_EVO_SPECIAL_SIZE	9
#define MOD_SPECIAL_VIRUS_THRESHOLD	0
#define MOD_SPECIAL_VIRUS_TARGET	1
#define MOD_SPECIAL_VIRUS_IMMUNE_A	2
#define MOD_SPECIAL_VIRUS_IMMUNE_B	3
#define MOD_SPECIAL_CHECK_LEVEL		4
#define MOD_SPECIAL_CHECK_HOURS		5
#define MOD_SPECIAL_SCOLD_LEVEL		6
#define MOD_SPECIAL_SCOLD_DISCIPLINE	7
#define MOD_SPECIAL_SCOLD_HAPPINESS	8

/* condition stats of a rule */
#define MOD_STAT_DISCIPLINE	0
#define MOD_STAT_HAPPINESS	1
#define MOD_STAT_TIREDNESS	2
#define MOD_STAT_BATTLES	3
#define MOD_STAT_DEFENSE	4
#define MOD_STAT_EVOTIMER	5
#define MOD_STAT_SCREEN		6
#define MOD_STAT_SCOLD_FLAG	7

typedef struct {
	int8_t stat;  /* MOD_STAT_*, -1 = no more conditions */
	int8_t op;    /* 0: ==, 1: >= */
	int16_t value;
} ModEvoCond;

typedef struct {
	int16_t target;  /* -1 = unused row */
	int16_t from[4]; /* the species it applies to (-1 = end; the first -1 = any species) */
	int8_t mode;     /* 2 after waking, 3 after praise or scolding */
	int8_t chance;   /* percent of the roll that passes (100 = always) */
	int8_t level;    /* -1 = any level */
	int8_t flags;    /* 1: clears the scold flag when it matches */
	ModEvoCond cond[MOD_EVO_COND_MAX];
} ModEvoRule;

extern int16_t MOD_EVO_FRESH[MOD_EVO_PAIR_MAX][2];
extern int16_t MOD_EVO_ULTIMATE[MOD_EVO_PAIR_MAX][2];
extern ModEvoRule MOD_EVO_RULES[MOD_EVO_RULES_MAX];
extern int16_t MOD_EVO_SPECIAL[MOD_EVO_SPECIAL_SIZE];

/*
 * MOD (tamer level as data): the chances (percent) of the events that change the tamer level, and the formulas that use it.
 * MOD_TAMER_LEVEL[]: every slot -1 = the event does not exist; tools/balance.py build writes them from
 * content/tables/tamer-level.json. The sign of each event (a good year +1, a chip -1...) is its meaning and stays in the code.
 */
#define MOD_TAMER_LEVEL_SIZE		13
#define MOD_TL_YEAR_HAPPY_CHANCE	0  /* a year ends with the partner's happiness exactly MOD_TL_YEAR_HAPPY_AT: +1 */
#define MOD_TL_YEAR_HAPPY_AT		1
#define MOD_TL_YEAR_UNHAPPY_CHANCE	2  /* a year ends with happiness below MOD_TL_YEAR_UNHAPPY_BELOW: -1 */
#define MOD_TL_YEAR_UNHAPPY_BELOW	3
#define MOD_TL_EATS_POOP_CHANCE		4  /* the partner eats its poop: -1 */
#define MOD_TL_SLEEPS_LATE_CHANCE	5  /* the partner falls asleep late: -1 */
#define MOD_TL_CHIP_CHANCE		6  /* a chip item (0x1D-0x1F): -1 */
#define MOD_TL_EVOLVE_CHAMPION_CHANCE	7  /* evolving into a Champion: +1 */
#define MOD_TL_EVOLVE_ULTIMATE_CHANCE	8  /* into an Ultimate: +1 */
#define MOD_TL_EVOLVE_MEGA_CHANCE	9  /* into a Mega: +1 */
#define MOD_TL_REBIRTH_PERCENT		10 /* percent of each old base stat inherited per tamer level on a natural death */
#define MOD_TL_FORGET_BASE		11 /* a lost life forgets (BASE - level) * STEP percent of the learned moves */
#define MOD_TL_FORGET_STEP		12
extern int16_t MOD_TAMER_LEVEL[MOD_TAMER_LEVEL_SIZE];
void modTamerLevelEvent(int32_t slot, int32_t amount);

/* items past the original evolution items (0x47-0x72) that force an evolution: {item, target, level the partner must have}, -1 = unused */
#define MOD_EVO_ITEM_MAX	4
typedef struct {
	int16_t item;
	int16_t target;
	int16_t level;
} ModEvoItem;
extern ModEvoItem MOD_EVO_ITEMS[MOD_EVO_ITEM_MAX];
ModEvoItem *getModEvoItem(int32_t item);
/* lifetime added on reaching Mega naturally (Ultimate: +96) */
#define MEGA_LIFETIME_BONUS	120

#endif
