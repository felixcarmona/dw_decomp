#ifndef DW_MOD_RULES_H
#define DW_MOD_RULES_H

#include <dw/types.h>

/*
 * MOD (rules as data): numbers that were literals in the code. Every slot starts at -1 ("the rule does not exist") and
 * tools/balance.py build writes them from content/tables/*.json (tournament-rules.json, daily-rules.json, daytime.json).
 */

/* tournament registration: reminder times (minutes of the day), the script sections they run, the Beetle cup's registration */
#define MOD_TOURNAMENT_SIZE	15
#define MOD_TR_REMIND_FROM	0 /* before this minute no reminder; -1: no reminders at all */
#define MOD_TR_EVENING_FROM	1 /* from here the second reminder (trigger 39) instead of the first (38) */
#define MOD_TR_MISSED_FROM	2 /* from here a registration is missed */
#define MOD_TR_REMIND_SECTION	3 /* MAPHEAD section of the reminder */
#define MOD_TR_MISSED_SECTION	4 /* MAPHEAD section of "you missed it" */
#define MOD_TR_BEETLE_CUP	5 /* routine 0x21 registers this cup (-1: the routine does nothing) */
#define MOD_TR_BEETLE_DAY	6 /* the day that cup is held */
#define MOD_TR_BEETLE_OTHER	7 /* the day stored when it is registered on any other day */
#define MOD_TR_POOL_LOW	8
#define MOD_TR_POOL_HIGH	9
#define MOD_TR_POOL_EXTRA_MIN	10
#define MOD_TR_POOL_EXTRA_DIV	11
#define MOD_TR_WIN_RESULT	12
#define MOD_TR_WIN_SPECIAL_CUP	13
#define MOD_TR_WIN_SPECIAL_RESULT	14
extern int16_t MOD_TOURNAMENT[MOD_TOURNAMENT_SIZE];

/*
 * MOD (economy as data): money, merit and stack limits, what a defeat costs, the shops' ratios and the card packs, MOD_ECON[] (32-bit slots, -1 = the rule does not exist),
 * written by tools/balance.py from content/tables/economy.json. A cap left out is the limit of what holds the value (money 2^31 - 1, merit 32767, a stack 255); a
 * percent left out is 100 (a price ratio) or 0 (the part of the items lost on a defeat); an amount left out is 0.
 */
#define MOD_ECON_SIZE		12
#define MOD_ECON_MONEY_CAP	0
#define MOD_ECON_MERIT_CAP	1
#define MOD_ECON_STACK_CAP	2
#define MOD_ECON_LOSS_PERCENT	3
#define MOD_ECON_BABY_PERCENT	4
#define MOD_ECON_SELL_PERCENT	5
#define MOD_ECON_CARD_CUT0	6
#define MOD_ECON_CARD_CUT1	7
#define MOD_ECON_CARD_CUT2	8
#define MOD_ECON_CARD_CUT3	9
#define MOD_ECON_CARD_MAX	10
#define MOD_ECON_PACK_PRICE	11 /* MOD (amounts): what a card pack costs (left out: nothing) */
extern int32_t MOD_ECON[MOD_ECON_SIZE];
int32_t econCap(int32_t slot, int32_t whenLeftOut);
int32_t econAmt(int32_t slot);
int32_t econPercent(int32_t slot);

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

/* a new game and a new partner: MOD_NEWGAME[], every slot -1 = the rule does not exist (modNewGame answers 0), written by tools/balance.py
 * from content/tables/new-game.json */
#define MOD_NEWGAME_SIZE	20
#define MOD_NG_MONEY		0 /* the tamer's money in a new game */
#define MOD_NG_START_HOUR	1 /* the game clock in a new game */
#define MOD_NG_START_MINUTE	2
#define MOD_NG_FIRST_LIFETIME	3 /* the first partner's lifetime (hours) and age (days) */
#define MOD_NG_FIRST_AGE	4
#define MOD_NG_LIVES		5 /* the lives of a partner at its start (a new game, a reincarnation) */
#define MOD_NG_BIRTH_LIFETIME	6 /* the lifetime (hours) of every partner that is born or reborn */
#define MOD_NG_BIRTH_DISCIPLINE	7
#define MOD_NG_BIRTH_HAPPINESS	8
#define MOD_NG_STARTER0		9 /* the partner the Jijimon scene gives: the first choice (pstat 0xfe = 0) ... */
#define MOD_NG_STARTER1		10 /* ... and the second */
#define MOD_NG_START_SCRIPT	11 /* the MAPHEAD section that starts a new game (puts the tamer on its first screen) */
#define MOD_NG_START_EXIT	12 /* the exit stored in the empty save of a new slot */
#define MOD_NG_FIRST_SPECIES	13 /* the partner type stored in that empty save */
#define MOD_NG_SHOP_CARD0	14 /* the first six values of the card shop's script state (SAVE scriptState[0..5]) */
extern int32_t MOD_NEWGAME[MOD_NEWGAME_SIZE];
int32_t modNewGame(int32_t slot);

/*
 * MOD (training as data): the gains of each gym by the slot of its minigame (MOD_TRAIN_GAIN / MOD_TRAIN2_GAIN: rows of the gym, the slot (-2: any), the base gain in tenths of the
 * six stats and the tiredness, energy and happiness it costs) and the rules of a session (MOD_TRAIN_RULES / MOD_TRAIN2_RULES), for the first gym set (TRN overlay) and the second (TRN2).
 * Every value -1 = does not exist; tools/balance.py build writes them from content/tables/training-gains.json.
 */
#define MOD_TG_ROWS		16
#define MOD_TG_WIDTH		11
#define MOD_TR_SIZE		34
#define MOD_TG_HP	0
#define MOD_TG_MP	1
#define MOD_TG_OFF	2
#define MOD_TG_DEF	3
#define MOD_TG_SPD	4
#define MOD_TG_BRAIN	5
#define MOD_TGF_HP	2
#define MOD_TGF_MP	3
#define MOD_TGF_OFF	4
#define MOD_TGF_DEF	5
#define MOD_TGF_SPD	6
#define MOD_TGF_BRAIN	7
#define MOD_TGF_TIRED	8
#define MOD_TGF_ENERGY	9
#define MOD_TGF_HAPPY	10
#define MOD_TR_EXH_AT	0
#define MOD_TR_EXH_GAIN	1
#define MOD_TR_TIRED_AT	2
#define MOD_TR_PEN_HAPPY	3
#define MOD_TR_PEN_DISC	4
#define MOD_TR_LIFE_BELOW	5
#define MOD_TR_LIFE_PER	6
#define MOD_TR_STARVE	7
#define MOD_TR_FOOD	8
#define MOD_TR_POOP	9
#define MOD_TR_LIFE_COST	10
#define MOD_TR_PAY_JACKPOT	11
#define MOD_TR_PAY_MATCH	12
#define MOD_TR_PAY_TRIPLE	13
#define MOD_TR_PAY_MISS	14
#define MOD_TR_ORDER0	15
#define MOD_TR_ORDER1	16
#define MOD_TR_ORDER2	17
#define MOD_TR_ORDER3	18
#define MOD_TR_ORDER4	19
#define MOD_TR_LEARN0	20
#define MOD_TR_LEARN1	21
#define MOD_TR_LEARN2	22
#define MOD_TR_LEARN3	23
#define MOD_TR_LEARN4	24
#define MOD_TR_LEARN5	25
#define MOD_TR_LEARN6	26
#define MOD_TR_LEARN7	27
#define MOD_TR_LEARN8	28
#define MOD_TR_LEARN9	29
#define MOD_TR_LEARN10	30
#define MOD_TR_LEARN11	31
#define MOD_TR_SESSION	32 /* logic ticks (50 ms) a gym session runs before it ends by itself */
#define MOD_TR_STEP	33 /* ticks of one step of a session: the work shown (every step) and the reward (per step done) */
extern int16_t MOD_TRAIN_GAIN[MOD_TG_ROWS][MOD_TG_WIDTH];
extern int16_t MOD_TRAIN_RULES[MOD_TR_SIZE];
extern int16_t MOD_TRAIN2_GAIN[MOD_TG_ROWS][MOD_TG_WIDTH];
extern int16_t MOD_TRAIN2_RULES[MOD_TR_SIZE];
/* the length of a gym session and of one step of it; a session left out ends at once, a step left out is 1 tick */
#define TRN_SESSION_OF(rules)	((rules)[MOD_TR_SESSION] > 0 ? (rules)[MOD_TR_SESSION] : 0)
#define TRN_SESSION_SPAN_OF(rules)	((rules)[MOD_TR_SESSION] > 0 ? (rules)[MOD_TR_SESSION] : 1)
#define TRN_STEP_OF(rules)	((rules)[MOD_TR_STEP] > 0 ? (rules)[MOD_TR_STEP] : 1)
#define TG(row, field)		((row) ? (row)[field] : 0)
/* the row of a gym for a slot of its minigame: the exact slot, else the row for any slot (-2); NULL = no gain */
static inline const int16_t *trainGainRow(int16_t rows[][MOD_TG_WIDTH], int32_t gym, int32_t slot)
{
	int32_t i;
	const int16_t *any = 0;

	for (i = 0; i < MOD_TG_ROWS; i++) {
		if (rows[i][0] == gym && rows[i][1] == slot) {
			return rows[i];
		}
		if (rows[i][0] == gym && rows[i][1] == -2) {
			any = rows[i];
		}
	}
	return any;
}

/* MOD (species rules as data): the few things the game does for a species by number, MOD_MODEL_SCALE[] rows {species, percent} (the model is drawn at that
 * percent of its size; a species without a row is drawn at 100) and MOD_ANIM_TEXTURE[] rows {species, width} (the species whose texture is animated, the
 * flames, and the width of the strip that moves). A row with species -1 is empty; written by tools/balance.py from content/tables/species-rules.json. */
#define MOD_MODEL_SCALE_ROWS	8
#define MOD_ANIM_TEXTURE_ROWS	16
extern int16_t MOD_MODEL_SCALE[MOD_MODEL_SCALE_ROWS][2];
extern int16_t MOD_ANIM_TEXTURE[MOD_ANIM_TEXTURE_ROWS][2];
int32_t modModelScale(int32_t type);			/* the scale as GsCOORDINATE scale (0x1000 = 100 percent) */
int32_t modAnimatedTextureWidth(int32_t type);	/* 0 = the species has no animated texture */

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
/* the second gym set (TRN2 overlay) has its own copy: its numbers (training.json "training2") are the same today, its bonus is the training manual only */
extern int16_t MOD_TRAIN2_NUM[MOD_TRAIN_MODES][MOD_TRAIN_TYPES];
extern int16_t MOD_TRAIN2_LEVEL[MOD_TRAIN_LEVELS];
extern ModTrainBonus MOD_TRAIN2_BONUS[MOD_TRAIN_BONUS_MAX];

/* the battle registration record a game clear unlocks for the VS mode: species, three techniques, slot (-1: nothing is unlocked) */
extern int16_t MOD_VS_UNLOCK[5];

#endif
