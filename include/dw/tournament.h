#ifndef DW_TOURNAMENT_H
#define DW_TOURNAMENT_H

#include <dw/types.h>

#define TOURNAMENT_ARRAY_SIZE	30

extern uint8_t TOURNAMENT_SCHEDULE[];
extern char *TOURNAMENT_GRADES[];

extern uint8_t *DGET_BUFFER;
extern uint8_t TOURNAMENT_SELECTED_COLUMN;
extern uint8_t TOURNAMENT_SELECTED_ROW;

uint32_t minutesOfDay(void);
uint8_t *getCupDataJumpTable(uint8_t section, uint8_t id);
uint8_t *getCupDataJumpTableEntry(uint8_t *scriptPtr, uint8_t id);
void updateTournamentRegistration(void);
void scriptStartTournament(void);
void scriptCheckTournamentMedal(void);
int32_t checkTournamentMedalConditions(void);

/*
 * MOD (medals as data): when a medal is awarded. The rows start empty (medal -1) and tools/balance.py build writes them from
 * content/tables/medals.json. A row is checked every tamer tick (MOD_MEDAL_TICK), by the script routine 0x16 (MOD_MEDAL_SCRIPT, `text`
 * is the textbox shown), or both, and may raise the tamer level (MOD_MEDAL_LEVEL_UP).
 */
#define MOD_MEDAL_MAX		16
#define MOD_MEDAL_TICK		1
#define MOD_MEDAL_SCRIPT	2
#define MOD_MEDAL_LEVEL_UP	4
enum {
	MOD_MEDAL_ALL_TRIGGERS = 1, /* a = first trigger, b = how many: all set */
	MOD_MEDAL_WINS,             /* tournament wins >= a */
	MOD_MEDAL_ALL_CARDS,        /* every card owned, and trigger a set (a = -1: no trigger) */
	MOD_MEDAL_ALL_MOVES,        /* the first a techniques learned */
	MOD_MEDAL_STATS,            /* HP and MP >= a, the other four stats >= b */
	MOD_MEDAL_MONEY,            /* money == a */
	MOD_MEDAL_YEAR,             /* year == a */
	MOD_MEDAL_ALL_RAISED,       /* the species a .. b - 1 raised once */
	MOD_MEDAL_FISH,             /* fish caught >= a */
};
typedef struct {
	int16_t medal; /* -1 = unused row */
	int8_t kind;
	int8_t flags;
	int8_t text;
	int8_t pad;
	int32_t a;
	int32_t b;
} ModMedal;
extern ModMedal MOD_MEDALS[MOD_MEDAL_MAX];
int32_t modMedalHolds(ModMedal *m);

void initTournamentSchedule(void);
void initTournamentInfo(int32_t source);
void fillEnabledTournamentTable(void);
void buildScheduleLabels(void);
void buildScheduleEntries(void);
void tickTournamentSchedule(void);
void renderTournamentSchedule(void);
void renderTournamentTextbox(void);
void tickTournamentInfo(void);
void renderTournamentInfo(void);
int32_t isTournamentEnabled(uint8_t tournament);
int32_t tournamentCheckFair(uint8_t value);
int32_t tournamentCheckEligible(uint8_t tournament);

#endif
