#ifndef DW_MOD_RULES_H
#define DW_MOD_RULES_H

#include <dw/types.h>

/*
 * MOD (rules as data): numbers that were literals in the code. Every slot starts at -1 ("the rule does not exist") and
 * tools/balance.py build writes them from content/tables/*.json (tournament-rules.json, daily-rules.json, daytime.json).
 */

/* tournament registration: reminder times (minutes of the day), the script sections they run, the Beetle cup's registration */
#define MOD_TOURNAMENT_SIZE	8
#define MOD_TR_REMIND_FROM	0 /* before this minute no reminder; -1: no reminders at all */
#define MOD_TR_EVENING_FROM	1 /* from here the second reminder (trigger 39) instead of the first (38) */
#define MOD_TR_MISSED_FROM	2 /* from here a registration is missed */
#define MOD_TR_REMIND_SECTION	3 /* MAPHEAD section of the reminder */
#define MOD_TR_MISSED_SECTION	4 /* MAPHEAD section of "you missed it" */
#define MOD_TR_BEETLE_CUP	5 /* routine 0x21 registers this cup (-1: the routine does nothing) */
#define MOD_TR_BEETLE_DAY	6 /* the day that cup is held */
#define MOD_TR_BEETLE_OTHER	7 /* the day stored when it is registered on any other day */
extern int16_t MOD_TOURNAMENT[MOD_TOURNAMENT_SIZE];

/* the daily reroll (dailyPStatTrigger): the card shop's stock and the counters */
#define MOD_DAILY_SIZE		7
#define MOD_DAILY_CARDS		0 /* cards in the daily stock (0..6; -1: none) */
#define MOD_DAILY_POOL		1 /* a card is 1 + randomLimit(pool) */
#define MOD_DAILY_EXCLUDED	2 /* this card is never drawn */
#define MOD_DAILY_COUNTER_FIRST	3 /* pstats counted up every day (255 stays) */
#define MOD_DAILY_COUNTER_COUNT	4
#define MOD_DAILY_CLEAR_PSTAT	5 /* this pstat is masked every day ... */
#define MOD_DAILY_CLEAR_MASK	6 /* ... with this mask */
extern int16_t MOD_DAILY[MOD_DAILY_SIZE];

/* the game clock: how many real seconds one game day lasts (MOD_CLOCK[0], -1: game time does not run). A frame of CURRENT_FRAME is
 * 1/20 of a game minute (3 game seconds) whatever the screen's frame rate; the logic runs at 20 ticks a second. */
#define MOD_CLOCK_SIZE		1
#define MOD_CLK_DAY_SECONDS	0
extern int16_t MOD_CLOCK[MOD_CLOCK_SIZE];
/* game frames the clock advanced this tick (at least 1), and whether a period of frames was crossed this tick (a jump of a script, more than 64 frames, crosses nothing) */
int32_t clockAdvance(void);
int32_t clockCrossed(int32_t period);

/* time of day: the hours the screen changes, the tint, the music variant and the clock icon */
#define MOD_DAYTIME_SIZE	9
#define MOD_DT_DUSK		0 /* the hour dusk starts (-1 in any of the first three: always day) */
#define MOD_DT_NIGHT		1
#define MOD_DT_DAWN		2
#define MOD_DT_DUSK_GREEN	3 /* tint of dusk (of 10) */
#define MOD_DT_NIGHT_TINT	4 /* tint of night (of 10) */
#define MOD_DT_BGM_FROM		5 /* the day music plays from this hour ... */
#define MOD_DT_BGM_TO		6 /* ... to this one (exclusive); -1: always the day music */
#define MOD_DT_CLOCK_FROM	7 /* the clock shows day ... */
#define MOD_DT_CLOCK_TO		8
extern int16_t MOD_DAYTIME[MOD_DAYTIME_SIZE];
/* 0 dusk, 1 night, 2 day at an hour */
int32_t modTimeOfDay(int32_t hour);

/* training: the gain multiplier of a stat (mode 0..5) by the species' training type, by level, and the bonus rows */
#define MOD_TRAIN_MODES		6
#define MOD_TRAIN_TYPES		5 /* training types 0..3 and "other" */
#define MOD_TRAIN_LEVELS	8 /* level 0..7 (a level past 7 uses 7) */
#define MOD_TRAIN_BONUS_MAX	6
/* a bonus row: kind 0 = a trigger is set (arg), 1 = the upgrade counter of the mode >= arg (both only when training for real), 2 = the
 * player holds the item (arg); modes = bit mask of the modes it applies to; the gain is multiplied by num/den; kind -1 = unused row */
typedef struct {
	int16_t kind;
	int16_t arg;
	int16_t modes;
	int16_t num;
	int16_t den;
} ModTrainBonus;
extern int16_t MOD_TRAIN_NUM[MOD_TRAIN_MODES][MOD_TRAIN_TYPES]; /* tenths; -1 = no gain */
extern int16_t MOD_TRAIN_LEVEL[MOD_TRAIN_LEVELS];               /* tenths of the multiplier by level; -1 = no gain */
extern ModTrainBonus MOD_TRAIN_BONUS[MOD_TRAIN_BONUS_MAX];

/* the battle registration record a game clear unlocks for the VS mode: species, three techniques, slot (-1: nothing is unlocked) */
extern int16_t MOD_VS_UNLOCK[5];

#endif
