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

/*
 * MOD (care rules as data): the numbers of the partner's daily care (sleep, sulking, tiredness, hunger, poop, sickness, lifetime).
 * MOD_CARE[]: every slot -1 = the rule does not exist; tools/balance.py build writes them from content/tables/care-rules.json.
 * A period or a threshold left out (-1) switches its rule off; an amount left out counts as 0. Periods are logic frames: 20 = one game minute,
 * 1200 = one game hour. The direction of each amount (a loss, a gain) is its name and stays in the code.
 */
#define MOD_CARE_SIZE		150
#define MOD_CARE_SLEEP_TIRED_PERIOD	0
#define MOD_CARE_SLEEP_TIRED_PERCENT	1
#define MOD_CARE_SLEEP_SLEEPY_AT	2
#define MOD_CARE_SLEEP_SLEEPY_PERIOD	3
#define MOD_CARE_SLEEP_FRESH_PERIOD	4
#define MOD_CARE_SLEEP_FRESH_HAPPY	5
#define MOD_CARE_SLEEP_FRESH_DISC	6
#define MOD_CARE_SLEEP_TRAIN_PERIOD	7
#define MOD_CARE_SLEEP_TRAIN_HAPPY	8
#define MOD_CARE_SLEEP_TRAIN_DISC	9
#define MOD_CARE_SLEEP_LATER_PERIOD	10
#define MOD_CARE_SLEEP_LATER_HAPPY	11
#define MOD_CARE_SLEEP_LATER_DISC	12
#define MOD_CARE_SLEEP_MISSED_PERIOD	13
#define MOD_CARE_SULK_PERIOD	14
#define MOD_CARE_SULK_RECOVER_PERIOD	15
#define MOD_CARE_SULK_STAT_PERIOD	16
#define MOD_CARE_SULK_STAT_PERCENT	17
#define MOD_CARE_AREA_PERIOD	18
#define MOD_CARE_AREA_GOOD_HAPPY	19
#define MOD_CARE_AREA_GOOD_TIRED	20
#define MOD_CARE_AREA_BAD_HAPPY	21
#define MOD_CARE_AREA_BAD_TIRED	22
#define MOD_CARE_TIRED_SUB	23
#define MOD_CARE_TIRED_HUNGER_AT	24
#define MOD_CARE_TIRED_EXHAUSTED_AT	25
#define MOD_CARE_TIRED_EXHAUSTED_PERIOD	26
#define MOD_CARE_TIRED_EXHAUSTED_HAPPY	27
#define MOD_CARE_HUNGER_FOOD_PERIOD	28
#define MOD_CARE_HUNGER_STARVE_PERIOD	29
#define MOD_CARE_HUNGER_STARVE_TIMER	30
#define MOD_CARE_HUNGER_WEIGHT_PERIOD	31
#define MOD_CARE_HUNGER_EMPTY_PERIOD	32
#define MOD_CARE_HUNGER_TIRED_AFTER	33
#define MOD_CARE_HUNGER_TIRED_PERIOD	34
#define MOD_CARE_HUNGER_TIRED_LOSS	35
#define MOD_CARE_HUNGER_ENERGY_PERIOD	36
#define MOD_CARE_POOP_PERIOD	37
#define MOD_CARE_POOP_HOLD_OFFSET	38
#define MOD_CARE_POOP_HOLD_SCALE	39
#define MOD_CARE_SICK_PERIOD	40
#define MOD_CARE_SICK_MIN_COUNTER	41
#define MOD_CARE_SICK_FREE_TRIES	42
#define MOD_CARE_SICK_PER_TRY	43
#define MOD_CARE_SICK_REST_TIRED	44
#define MOD_CARE_SICK_ONSET_HAPPY	45
#define MOD_CARE_SICK_AREA_AFTER	46
#define MOD_CARE_SICK_TICK_HAPPY	47
#define MOD_CARE_SICK_TICK_DISC	48
#define MOD_CARE_SICK_TICK_TIRED	49
#define MOD_CARE_SICK_INJURY_AFTER	50
#define MOD_CARE_SICK_LIFE_AFTER	51
#define MOD_CARE_LIFE_PERIOD	52
#define MOD_CARE_LIFE_BELOW	53
#define MOD_CARE_LIFE_PER_POINTS	54
#define MOD_CARE_SCOLD_VALID_DISC	55
#define MOD_CARE_SCOLD_UNJUST_DISC	56
#define MOD_CARE_SCOLD_UNJUST_HAPPY	57
#define MOD_CARE_PRAISE_DISC	58
#define MOD_CARE_PRAISE_HAPPY_BASE	59
#define MOD_CARE_PRAISE_HAPPY_DIV	60
#define MOD_CARE_REFUSE_BASE	61
#define MOD_CARE_REFUSE_OFFSET	62
#define MOD_CARE_REFUSE_JITTER	63
#define MOD_CARE_REFUSE_FAV_DIE	64
#define MOD_CARE_REFUSE_FAV_BELOW	65
#define MOD_CARE_LIM_HAPPY_MIN	66
#define MOD_CARE_LIM_HAPPY_MAX	67
#define MOD_CARE_LIM_DISC_MIN	68
#define MOD_CARE_LIM_DISC_MAX	69
#define MOD_CARE_LIM_TIRED_MIN	70
#define MOD_CARE_LIM_TIRED_MAX	71
#define MOD_CARE_LIM_WEIGHT_MIN	72
#define MOD_CARE_LIM_WEIGHT_MAX	73
#define MOD_CARE_SLEEP_AWAKE_FRESH	74
#define MOD_CARE_SLEEP_AWAKE_TRAIN	75
#define MOD_CARE_SLEEP_AWAKE_JITTER	76
#define MOD_CARE_HUNGER_EVERY_FRESH	77
#define MOD_CARE_HUNGER_EVERY_TRAIN	78
#define MOD_CARE_SLEEP_LATE_MISTAKES	79
#define MOD_CARE_SLEEP_HUNGRY_WEIGHT	80
#define MOD_CARE_SLEEP_SICK_HAPPY	81
#define MOD_CARE_SLEEP_SICK_DISC	82
#define MOD_CARE_SLEEP_SICK_TIRED	83
#define MOD_CARE_SLEEP_INJ_HAPPY	84
#define MOD_CARE_SLEEP_INJ_DISC	85
#define MOD_CARE_SLEEP_INJ_TIRED	86
#define MOD_CARE_REGEN_BED	87
#define MOD_CARE_REGEN_GOOD	88
#define MOD_CARE_REGEN_BAD	89
#define MOD_CARE_REGEN_HPMP_BASE	90
#define MOD_CARE_REGEN_HPMP_ROLL	91
#define MOD_CARE_REGEN_TIRED_BASE	92
#define MOD_CARE_REGEN_TIRED_ROLL	93
#define MOD_CARE_REGEN_WEIGHT_DIV	94
#define MOD_CARE_HUNGER_STARVE_MISTAKES	95
#define MOD_CARE_HUNGER_EMPTY_WEIGHT	96
#define MOD_CARE_SULK_STAT_FLOOR	97
#define MOD_CARE_FEED_HUNGRY_HAPPY	98
#define MOD_CARE_FEED_HUNGRY_DISC	99
#define MOD_CARE_FEED_FULL_HAPPY	100
#define MOD_CARE_FEED_FULL_DISC	101
#define MOD_CARE_TOILET_HAPPY	102
#define MOD_CARE_TOILET_DISC	103
#define MOD_CARE_POOPW_DIE	104
#define MOD_CARE_POOPW_DIV	105
#define MOD_CARE_WILD_MISTAKES	106
#define MOD_CARE_WILD_HAPPY	107
#define MOD_CARE_WILD_DISC	108
#define MOD_CARE_EAT_MED_FROM	109
#define MOD_CARE_EAT_LARGE_FROM	110
#define MOD_CARE_EAT_S_HP	111
#define MOD_CARE_EAT_S_MP	112
#define MOD_CARE_EAT_S_W	113
#define MOD_CARE_EAT_S_CURE	114
#define MOD_CARE_EAT_M_HP	115
#define MOD_CARE_EAT_M_MP	116
#define MOD_CARE_EAT_M_W	117
#define MOD_CARE_EAT_M_CURE	118
#define MOD_CARE_EAT_L_HP	119
#define MOD_CARE_EAT_L_MP	120
#define MOD_CARE_EAT_L_W	121
#define MOD_CARE_EAT_L_CURE	122
#define MOD_CARE_SKIP_STARVE	123
#define MOD_CARE_SKIP_FOOD	124
#define MOD_CARE_SKIP_POOP	125
#define MOD_CARE_BATTLE_TIRED	126
#define MOD_CARE_BATTLE_MP_LOW	127
#define MOD_CARE_BATTLE_MP_MULT	128
#define MOD_CARE_BATTLE_FOOD	129
#define MOD_CARE_LOST_HAPPY	130
#define MOD_CARE_LOST_DISC	131
#define MOD_CARE_FLED_HAPPY	132
#define MOD_CARE_FLED_DISC	133
#define MOD_CARE_FLED_TIRED	134
#define MOD_CARE_WON_HAPPY	135
#define MOD_CARE_BATTLE_TIME	136
#define MOD_CARE_FOOD_FAV_PERCENT	137
#define MOD_CARE_FOOD_FAV_HAPPY	138
#define MOD_CARE_FOOD_BOOST_UNIT	139
#define MOD_CARE_FOOD_SICK_DIE	140
#define MOD_CARE_MEDICINE_INJURY_DIE	141
#define MOD_CARE_MEDICINE_SICK_DIE	142
#define MOD_CARE_SULK_RECOVER_GAIN	143 /* happiness a sulking partner regains every recover period */
#define MOD_CARE_TIRED_SUB_GAIN	144 /* tiredness one full sub-tiredness counter makes (capped by the tiredness limit) */
#define MOD_CARE_EAT_REACH_NEAR	145 /* how close (world units) the tamer must be to a small poop for the partner to go and eat it */
#define MOD_CARE_EAT_REACH_FAR	146 /* the same for a poop of farFrom or more */
#define MOD_CARE_EAT_FAR_FROM	147
#define MOD_CARE_EAT_SPECIES0	148 /* the species that eat poop (-1 = none) */
#define MOD_CARE_EAT_SPECIES1	149
extern int16_t MOD_CARE[MOD_CARE_SIZE];
/*
 * MOD (item effects as data): what an item does when it is used, MOD_ITEM_FX[row][field]: field 0 the item id (-1 = the row is unused), then the fields below
 * (a field left out of the item's "effect" block is 0). Written by tools/balance.py from content/items/*.json. itemFx answers the row of an item or NULL (no effect).
 */
#define MOD_ITEM_FX_ROWS	64
#define MOD_FX_WIDTH		22
#define MOD_FX_ENERGY	1
#define MOD_FX_HAPPINESS	2
#define MOD_FX_WEIGHT	3
#define MOD_FX_TIREDNESS	4
#define MOD_FX_DISCIPLINE	5
#define MOD_FX_LIFETIME	6
#define MOD_FX_ADD_HP	7
#define MOD_FX_ADD_MP	8
#define MOD_FX_HEAL_HP	9
#define MOD_FX_HEAL_MP	10
#define MOD_FX_ADD_OFF	11
#define MOD_FX_ADD_DEF	12
#define MOD_FX_ADD_SPEED	13
#define MOD_FX_ADD_BRAIN	14
#define MOD_FX_BOOST_FLAG	15
#define MOD_FX_BOOST_VALUE	16
#define MOD_FX_BOOST_HOURS	17
#define MOD_FX_SICK_CHANCE	18
#define MOD_FX_HEAL_HP_DIV	19
#define MOD_FX_CURE_INJURY	20
#define MOD_FX_CURE_SICK	21
extern int16_t MOD_ITEM_FX[MOD_ITEM_FX_ROWS][MOD_FX_WIDTH];
const int16_t *itemFx(int32_t itemId);
#define MOD_FX(row, field)	((row) ? (row)[field] : 0)
/* the value of a care rule for the partner's species (its own override or the global one), whether it exists (>= 0), and as an amount (a missing one is 0) */
int32_t careGet(int32_t slot);
int32_t careHas(int32_t slot);
int32_t careAmt(int32_t slot);
int32_t carePercent(int32_t slot);
int32_t careDie(int32_t slot);
void careClamp(int16_t *value, int32_t minSlot, int32_t maxSlot);
/* a species' own values: {species, slot, value} rows, ended by species -1 (the rest of the table is filled with -1 at build) */
#define MOD_CARE_OVERRIDE_MAX	192
extern int16_t MOD_CARE_SPECIES[MOD_CARE_OVERRIDE_MAX][3];

/*
 * MOD (evolution formulas as data): the numbers of what happens when a partner evolves and of the score that picks the species it evolves into,
 * MOD_EVO_FORMULA[]: every slot -1 = the rule does not exist; tools/balance.py build writes them from content/tables/evolution-rules.json
 * (evolutionFormula). evoAmt: a number (a rule left out is 0); evoFlag: a switch (a rule left out is off).
 */
#define MOD_EVO_FORMULA_SIZE	31
#define MOD_EVF_STAT_GAIN_DIV	0
#define MOD_EVF_STAT_AVG_DIV	1
#define MOD_EVF_LIFE_ULTIMATE	2
#define MOD_EVF_LIFE_MEGA	3
#define MOD_EVF_RESET_WEIGHT	4
#define MOD_EVF_RESET_MISTAKES	5
#define MOD_EVF_RESET_BATTLES	6
#define MOD_EVF_SCALE_BELOW_LEVEL	7
#define MOD_EVF_SCALE_SPECIES0	8
#define MOD_EVF_SCALE_SPECIES1	9
#define MOD_EVF_SCALE_SPECIES2	10
#define MOD_EVF_SCALE_SPECIES3	11
#define MOD_EVF_SCALE_RESTORE	12
#define MOD_EVF_SCALE_PCT_DIV	13
#define MOD_EVF_SCALE_RESTORE_DIV	14
#define MOD_EVF_LEARN_BASE0	15
#define MOD_EVF_LEARN_BASE1	16
#define MOD_EVF_LEARN_BASE2	17
#define MOD_EVF_LEARN_BASE3	18
#define MOD_EVF_LEARN_BASE4	19
#define MOD_EVF_LEARN_BASE5	20
#define MOD_EVF_LEARN_BASE6	21
#define MOD_EVF_LEARN_WINDOW	22
#define MOD_EVF_SCORE_WEIGHT_TOL	23
#define MOD_EVF_SCORE_POINT_CARE	24
#define MOD_EVF_SCORE_POINT_WEIGHT	25
#define MOD_EVF_SCORE_POINT_STATS	26
#define MOD_EVF_SCORE_POINT_BONUS	27
#define MOD_EVF_SCORE_HPMP_DIV	28
#define MOD_EVF_SCORE_RAISED_BONUS	29
#define MOD_EVF_SCORE_PASS	30
extern int16_t MOD_EVO_FORMULA[MOD_EVO_FORMULA_SIZE];
int32_t evoAmt(int32_t slot);
int32_t evoFlag(int32_t slot);
int32_t evoPassPoints(void);
/* MOD (chart): the four scored requirements of an evolution, as evoRequirementsMet's bits (evolution.c) */
#define EVO_MET_CARE	1
#define EVO_MET_WEIGHT	2
#define EVO_MET_STATS	4
#define EVO_MET_BONUS	8
int32_t evoTargetWeight(int32_t current, int16_t target);
int32_t evoRequirementsMet(int32_t current, int16_t target, int8_t isMaxCM, int8_t isMaxBattles);
int32_t evoRequirementPoints(int32_t met);
int32_t evoPartnerTopStat(void);

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
