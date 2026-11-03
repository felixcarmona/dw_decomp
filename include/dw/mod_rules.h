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

#endif
