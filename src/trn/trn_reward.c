#include <string.h>

#include <dw/caps.h> /* MOD (caps) */
#include <dw/anim.h>
#include <dw/clock.h>
#include <dw/entity.h>
#include <dw/font.h>
#include <dw/math.h>
#include <dw/mod_rules.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/script.h>
#include <dw/trn.h>
#include <dw/types.h>
#include <dw/version.h>
#include <text/trn/trn_reward.h>

extern int16_t STATS_GAINS[6];
extern char *MOVE_NAMES[];

uint8_t getItemCount(ItemId type);
void renderString(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h, int32_t i);
void createCameraMovement(VECTOR *pos, int32_t speed);
void initializeDaytimeTransition(int32_t phase);
void tickConditionBoundaries(void);
void createCloudFX(SVECTOR *pos);
int32_t hasMove(int32_t moveId);
void learnMove(int32_t moveId);
void createMenuBox(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, void (*tick)(void), void (*render)(void));
void TRN_func_800888A0(int8_t arg);
int16_t TRN_calculateTrainingMultiplier(int32_t type, int16_t mode, int16_t flag);
void TRN_advanceTrainingTime(int16_t tiredGain, int16_t energyLoss, int16_t happyLoss);
void TRN_renderNewOrdersBox(void);
int32_t TRN_tryLearnMove(int32_t type);
void TRN_renderMoveLearnedBox(void);

static void *trn_reward_functions[] = {
	TRN_renderMoveLearnedBox,
	TRN_tryLearnMove,
	TRN_renderNewOrdersBox,
	TRN_advanceTrainingTime,
	TRN_calculateTrainingMultiplier,
	TRN_awardBrainsTrainingGains,
	TRN_awardMpTrainingGains,
	TRN_awardDefenseTrainingGains,
	TRN_awardSpeedTrainingGains,
	TRN_awardOffenseTrainingGains,
	TRN_statGainsAreZero,
	TRN_awardHpTrainingGains,
	TRN_createCloudFXLine,
	TRN_func_800888A0,
	TRN_startSlotSessionIfEnabled,
	TRN_saveTrainingStartTime,
};

// clang-format off
int8_t TRN_TRAINING_ANIM_IDS[NUM_ORIGINAL_DIGIMON][2] = { /* MOD: was [67] (types 0..66) */
	{ 0x0, 0x0 }, { 0x2e, 0xa }, { 0x2e, 0xa }, { 0x33, 0xc },
	{ 0x2e, 0x14 }, { 0x35, 0x14 }, { 0x2e, 0xa }, { 0x2e, 0xa },
	{ 0x2e, 0xa }, { 0x35, 0xd }, { 0x2e, 0xa }, { 0x2e, 0x14 },
	{ 0x31, 0xa }, { 0x2e, 0x8 }, { 0x2e, 0xa }, { 0x2e, 0xa },
	{ 0x2e, 0xa }, { 0x2e, 0x14 }, { 0x2e, 0xa }, { 0x2e, 0xa },
	{ 0x2e, 0xe }, { 0x2e, 0xf }, { 0x2e, 0xa }, { 0x2e, 0xa },
	{ 0x2e, 0xa }, { 0x2e, 0xa }, { 0x2e, 0xa }, { 0x2e, 0x8 },
	{ 0x33, 0xa }, { 0x2e, 0x14 }, { 0x2e, 0x14 }, { 0x2e, 0xa },
	{ 0x2e, 0xe }, { 0x2e, 0x14 }, { 0x31, 0xa }, { 0x2e, 0xa },
	{ 0x35, 0x14 }, { 0x2e, 0x14 }, { 0x2e, 0xa }, { 0x2e, 0x14 },
	{ 0x2e, 0xd }, { 0x2f, 0x1e }, { 0x2e, 0xf }, { 0x2e, 0xf },
	{ 0x2e, 0xa }, { 0x35, 0x17 }, { 0x2e, 0xa }, { 0x33, 0xa },
	{ 0x2e, 0xa }, { 0x2e, 0xa }, { 0x34, 0xa }, { 0x2e, 0xa },
	{ 0x2e, 0xa }, { 0x2e, 0xa }, { 0x2e, 0xa }, { 0x2e, 0xa },
	{ 0x30, 0x1e }, { 0x2e, 0xa }, { 0x2e, 0xa }, { 0x2e, 0x19 },
	{ 0x30, 0xa }, { 0x2e, 0xa }, { 0x2e, 0xa }, { 0x2e, 0xa },
	{ 0x2e, 0xa }, { 0x2e, 0xa }, { 0x2e, 0xa },
	/* MOD (master-tyrannomon): Tyrannomon's (8) */
	[DIGIMON_MASTER_TYRANNOMON] = { 0x2e, 0xa },
	/* MOD (cherrymon): its first technique, like Vegiemon (25) */
	[DIGIMON_CHERRYMON] = { 0x2e, 0xa },
	/* MOD (partners-npc-a): Sukamon's (39) */
	[DIGIMON_KING_SUKAMON] = { 0x2e, 0x14 },
	/* MOD (partners-npc-a): Megaton Punch, like Frigimon (23) */
	[DIGIMON_ICEMON] = { 0x2e, 0xa },
	/* MOD (partners-npc-a): Megaton Punch (animation 0x30) */
	[DIGIMON_METEORMON] = { 0x30, 0xa },
	/* MOD (partners-npc-b): Monzaemon's (14) */
	[DIGIMON_WARUMONZAEMON] = { 0x2e, 0xa },
	/* MOD (partners-npc-b): its first technique, like Whamon (24) */
	[DIGIMON_SHOGUNGEKOMON] = { 0x2e, 0xa },
	/* MOD (machinedramon): its first technique, Megaton Punch */
	[DIGIMON_MACHINEDRAMON] = { 0x2e, 0xa },
	/* MOD (jijimon): his first technique, Tremar */
	[DIGIMON_JIJIMON] = { 0x2e, 0xa },
	/* MOD (analogman): his first technique, Static Elect */
	[DIGIMON_ANALOGMAN] = { 0x2e, 0xa },
};

MESSAGES_TEXT

/* MOD (training as data): written by tools/balance.py from content/tables/training-gains.json; -1 = does not exist */
int16_t MOD_TRAIN_GAIN[MOD_TG_ROWS][MOD_TG_WIDTH] = { [0 ... MOD_TG_ROWS - 1] = { [0 ... MOD_TG_WIDTH - 1] = -1 } };
int16_t MOD_TRAIN_RULES[MOD_TR_SIZE] = { [0 ... MOD_TR_SIZE - 1] = -1 };

static int32_t TRN_rule(int32_t slot)
{
	return MOD_TRAIN_RULES[slot];
}

static int32_t TRN_amt(int32_t slot)
{
	return MOD_TRAIN_RULES[slot] > 0 ? MOD_TRAIN_RULES[slot] : 0;
}

/* MOD (training block) */
#define TB(name) TRN_##name
#define TB_SLOT MAIN_D_80135370
#define TB_RULES MOD_TRAIN_RULES
#define TB_START_MIN (TRN_CURRENT_HOUR * 60 + TRN_CURRENT_MINUTE)
#include "trn_block.h"

uint8_t TRN_D_8008F14C[56] = {
	0x2, 0x5, 0x0, 0x4, 0x3, 0x1, 0x6, 0x7,
	0x2b, 0x2c, 0x2a, 0x2e, 0x2d, 0x29, 0x28, 0x2f,
	0xc, 0xa, 0xd, 0x9, 0xb, 0xe, 0x8, 0xf,
	0x25, 0x26, 0x20, 0x23, 0x22, 0x24, 0x27, 0x21,
	0x17, 0x13, 0x14, 0x15, 0x12, 0x10, 0x11, 0x16,
	0x18, 0x1a, 0x1f, 0x1b, 0x1e, 0x1c, 0x19, 0x1d,
	0x37, 0x31, 0x32, 0x35, 0x36, 0x33, 0x34, 0x38,
};

int8_t TRN_D_8008F184[8][3] = {
	{ 0x0, 0xf, 0xa },
	{ 0x19, 0xd, 0x8 },
	{ 0x16, 0xb, 0x7 },
	{ 0x14, 0x9, 0x5 },
	{ 0x12, 0x8, 0x2 },
	{ 0xf, 0x7, 0x0 },
	{ 0xc, 0x6, 0x0 },
	{ 0xa, 0x5, 0x0 },
};

MASTERED_TEXT
// clang-format on

void TRN_saveTrainingStartTime(void)
{
	TRN_CURRENT_FRAME = CURRENT_FRAME;
	TRN_CURRENT_HOUR = HOUR;
	TRN_CURRENT_MINUTE = MINUTE;
	TRN_modBlockArm(); /* MOD (training block) */
}

void TRN_startSlotSessionIfEnabled(int16_t arg)
{
	if (MAIN_D_80135370 == 1) {
		while (TRN_startSlotSession(arg) != 0) {
		}
	}
}

void TRN_func_800888A0(int8_t arg)
{
	if (MAIN_D_80135370 == 1) {
		if (TRN_getSlotSessionResult() == -2) {
			startAnimation(ENTITY_TABLE[1], 4);
			createCameraMovement(&TAMER_ENTITY.entity.posData->location, 10);
			TRAINING_STATE = arg;
		}
	}
}

void TRN_createCloudFXLine(int16_t a, int16_t b, int16_t x, int16_t z, int16_t dx, int16_t dz, int8_t n)
{
	SVECTOR pos;
	int16_t d;
	int32_t i;
	int32_t r;

	if ((a % b) == 0) {
		if (a >= TRN_SESSION_OF(MOD_TRAIN_RULES)) {
			a = TRN_SESSION_OF(MOD_TRAIN_RULES);
		}
		d = (a / TRN_STEP_OF(MOD_TRAIN_RULES)) * b;
		for (i = 0; i < n; i++) {
			r = randomLimit(100);
			pos.vx = (x + d) - r;
			pos.vy = 0;
			pos.vz = (z + d) - r;
			createCloudFX(&pos);
			x += dx;
			z += dz;
		}
	}
}

void TRN_awardHpTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t x;
	int16_t y;
	int16_t v;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	const int16_t *row;

	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN_GAIN, MOD_TG_HP, b);
	x = TG(row, MOD_TGF_HP);
	y = TG(row, MOD_TGF_OFF);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);
	v = TRN_calculateTrainingMultiplier(a, 0, b);
	if (PARTNER_PARA.trainBoostFlag & 0x10) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 0x1) {
		y = (y * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		y = (y * v) / 10;
	}

	x = (x * c) / 10;
	y = (y * c) / 10;
	if (PARTNER_PARA.tiredness == TRN_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_HP) > 0) {
			x = TRN_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_OFF) > 0) {
			y = TRN_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN_saveBaseStats();
	STATS_GAINS[0] = x;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = y;
	STATS_GAINS[3] = 0;
	STATS_GAINS[4] = 0;
	STATS_GAINS[5] = 0;
	TRN_createPostTrainingStatsBox();
	TRN_advanceTrainingTime(tired, energy, happy);
}

int32_t TRN_statGainsAreZero(void)
{
	if (STATS_GAINS[0] != 0) {
		goto no;
	}

	if (STATS_GAINS[1] != 0) {
		goto no;
	}

	if (STATS_GAINS[2] != 0) {
		goto no;
	}

	if (STATS_GAINS[3] != 0) {
		goto no;
	}

	if (STATS_GAINS[4] != 0) {
		goto no;
	}

	if (STATS_GAINS[5] != 0) {
		goto no;
	}

	return 1;
no:
	return 0;
}

void TRN_awardOffenseTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t off;
	int16_t hp;
	int16_t spd;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	int16_t v;
	const int16_t *row;

	hp = off = spd = 0;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN_GAIN, MOD_TG_OFF, b);
	hp = TG(row, MOD_TGF_HP);
	off = TG(row, MOD_TGF_OFF);
	spd = TG(row, MOD_TGF_SPD);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN_calculateTrainingMultiplier(a, 2, b);
	if (PARTNER_PARA.trainBoostFlag & 0x10) {
		hp = (hp * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		hp = (hp * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 1) {
		off = (off * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		off = (off * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 4) {
		spd = (spd * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		spd = (spd * v) / 10;
	}

	hp = (hp * c) / 10;
	off = (off * c) / 10;
	spd = (spd * c) / 10;
	if (PARTNER_PARA.tiredness == TRN_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_HP) > 0) {
			hp = TRN_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_OFF) > 0) {
			off = TRN_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_SPD) > 0) {
			spd = TRN_amt(MOD_TR_EXH_GAIN);
		}
	}

	TRN_saveBaseStats();
	STATS_GAINS[0] = hp;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = off;
	STATS_GAINS[3] = 0;
	STATS_GAINS[4] = spd;
	STATS_GAINS[5] = 0;
	TRN_createPostTrainingStatsBox();
	TRN_advanceTrainingTime(tired, energy, happy);
}

void TRN_awardSpeedTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t x;
	int16_t y;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	int16_t v;
	const int16_t *row;

	x = y = 0;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN_GAIN, MOD_TG_SPD, b);
	x = TG(row, MOD_TGF_HP);
	y = TG(row, MOD_TGF_SPD);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN_calculateTrainingMultiplier(a, 4, b);
	if (PARTNER_PARA.trainBoostFlag & 0x10) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 4) {
		y = (y * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		y = (y * v) / 10;
	}

	x = (x * c) / 10;
	y = (y * c) / 10;
	if (PARTNER_PARA.tiredness == TRN_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_HP) > 0) {
			x = TRN_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_SPD) > 0) {
			y = TRN_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN_saveBaseStats();
	STATS_GAINS[0] = x;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = 0;
	STATS_GAINS[4] = y;
	STATS_GAINS[5] = 0;
	TRN_createPostTrainingStatsBox();
	TRN_advanceTrainingTime(tired, energy, happy);
}

void TRN_awardDefenseTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t x;
	int16_t y;
	int16_t v;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	const int16_t *row;

	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN_GAIN, MOD_TG_DEF, b);
	x = TG(row, MOD_TGF_DEF);
	y = TG(row, MOD_TGF_BRAIN);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);
	v = TRN_calculateTrainingMultiplier(a, 3, b);
	if (PARTNER_PARA.trainBoostFlag & 2) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 8) {
		y = (y * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		y = (y * v) / 10;
	}

	x = (x * c) / 10;
	y = (y * c) / 10;
	if (PARTNER_PARA.tiredness == TRN_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_DEF) > 0) {
			x = TRN_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_BRAIN) > 0) {
			y = TRN_amt(MOD_TR_EXH_GAIN);
		}
	}

	TRN_saveBaseStats();
	STATS_GAINS[0] = 0;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = x;
	STATS_GAINS[4] = 0;
	STATS_GAINS[5] = y;
	TRN_createPostTrainingStatsBox();
	TRN_advanceTrainingTime(tired, energy, happy);
}

void TRN_awardMpTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t x;
	int16_t y;
	int16_t v;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	const int16_t *row;

	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN_GAIN, MOD_TG_MP, b);
	x = TG(row, MOD_TGF_MP);
	y = TG(row, MOD_TGF_DEF);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);
	v = TRN_calculateTrainingMultiplier(a, 1, b);
	if (PARTNER_PARA.trainBoostFlag & 0x20) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 0x2) {
		y = (y * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		y = (y * v) / 10;
	}

	x = (x * c) / 10;
	y = (y * c) / 10;
	if (PARTNER_PARA.tiredness == TRN_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_MP) > 0) {
			x = TRN_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_DEF) > 0) {
			y = TRN_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN_saveBaseStats();
	STATS_GAINS[0] = 0;
	STATS_GAINS[1] = x;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = y;
	STATS_GAINS[4] = 0;
	STATS_GAINS[5] = 0;
	TRN_createPostTrainingStatsBox();
	TRN_advanceTrainingTime(tired, energy, happy);
}

/* MOD (training as data): a move can be learned when the brains pass one of the learn marks */
static int32_t TRN_crossedLearnMark(int32_t base, int32_t total)
{
	int32_t i;

	for (i = 0; i < 12; i++) {
		if (TRN_rule(MOD_TR_LEARN0 + i) >= 0 && base < TRN_rule(MOD_TR_LEARN0 + i) && total >= TRN_rule(MOD_TR_LEARN0 + i)) {
			return 1;
		}
	}
	return 0;
}

void TRN_awardBrainsTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t brains;
	int16_t mp;
	int16_t base;
	int16_t total;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	int16_t v;
	uint8_t shown;
	uint8_t y;
	const int16_t *row;

	MAIN_D_80135388 = 0;
	mp = brains = 0;
	base = PARTNER_ENTITY.digimonEntity.stats.base.brain;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN_GAIN, MOD_TG_BRAIN, b);
	mp = TG(row, MOD_TGF_MP);
	brains = TG(row, MOD_TGF_BRAIN);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN_calculateTrainingMultiplier(a, 1, b);
	if (PARTNER_PARA.trainBoostFlag & 0x20) {
		mp = (mp * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		mp = (mp * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 8) {
		brains = (brains * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		brains = (brains * v) / 10;
	}

	mp = (mp * c) / 10;
	brains = (brains * c) / 10;
	if (PARTNER_PARA.tiredness == TRN_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_MP) > 0) {
			mp = TRN_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_BRAIN) > 0) {
			brains = TRN_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (mp > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		mp = STAT_CAP;
	}

	if (brains > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		brains = STAT_CAP;
	}

	TRN_saveBaseStats();
	STATS_GAINS[0] = 0;
	STATS_GAINS[1] = mp;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = 0;
	STATS_GAINS[4] = 0;
	STATS_GAINS[5] = brains;
	TRN_createPostTrainingStatsBox();
	TRN_advanceTrainingTime(tired, energy, happy);
	total = base + brains;
	shown = 0;
	y = 0x84;
	if ((base < TRN_rule(MOD_TR_ORDER0)) && (total >= TRN_rule(MOD_TR_ORDER0))) {
		setTextColor(7);
		drawString(TRN_D_8008F0D8, 0, 0x78);
		shown = 1;
	}

	if ((base < TRN_rule(MOD_TR_ORDER1)) && (total >= TRN_rule(MOD_TR_ORDER1))) {
		setTextColor(7);
		drawString(TRN_D_8008F0F0, 0, 0x78);
		shown = 1;
	}

	if ((base < TRN_rule(MOD_TR_ORDER2)) && (total >= TRN_rule(MOD_TR_ORDER2))) {
		setTextColor(7);
		drawString(TRN_D_8008F100, 0, 0x78);
		drawString(TRN_D_8008F10C, 0, 0x84);
		shown = 1;
		y = 0x94;
	}

	if ((base < TRN_rule(MOD_TR_ORDER3)) && (total >= TRN_rule(MOD_TR_ORDER3))) {
		setTextColor(7);
		drawString(TRN_D_8008F11C, 0, 0x78);
		shown = 1;
	}

	if ((base < TRN_rule(MOD_TR_ORDER4)) && (total >= TRN_rule(MOD_TR_ORDER4))) {
		setTextColor(7);
		drawString(TRN_D_8008F12C, 0, 0x78);
		shown = 1;
	}

	if (shown == 1) {
		setTextColor(1);
		drawString(TRN_D_8008F140, 0, y);
		createMenuBox(2, -0x58, 0x12, 0xb0, 0x30, 2, 0, TRN_renderNewOrdersBox);
		MAIN_D_80135388 = 1;
	} else if (TRN_crossedLearnMark(base, total)) { /* MOD (caps): reaching 999 (was total == 999); MOD (training as data): the marks are MOD_TRAIN_RULES' */
		TRN_tryLearnMove(a);
	}
}

/*
 * MOD (rules as data): the tables below are written by tools/balance.py build from content/tables/training.json; -1 = does not exist.
 * The gain is num * level * bonuses / (10 * 10 * bonuses), in tenths.
 */
int16_t MOD_TRAIN_NUM[MOD_TRAIN_MODES][MOD_TRAIN_TYPES] = {
	{ -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1 },
};
int16_t MOD_TRAIN_LEVEL[MOD_TRAIN_LEVELS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
ModTrainBonus MOD_TRAIN_BONUS[MOD_TRAIN_BONUS_MAX] = {
	{ -1, 0, 0, 0, 1 }, { -1, 0, 0, 0, 1 }, { -1, 0, 0, 0, 1 }, { -1, 0, 0, 0, 1 }, { -1, 0, 0, 0, 1 }, { -1, 0, 0, 0, 1 },
};

int16_t TRN_calculateTrainingMultiplier(int32_t type, int16_t mode, int16_t flag)
{
	uint32_t num;
	uint32_t den;
	uint8_t tt;
	int32_t lv;
	int32_t i;
	ModTrainBonus *bonus;
	int16_t counter;
	int16_t result;

	if (mode < 0 || mode >= MOD_TRAIN_MODES) {
		return 0;
	}
	tt = RAISE_ROW(type).trainingType;
	if (tt >= MOD_TRAIN_TYPES) {
		tt = MOD_TRAIN_TYPES - 1;
	}
	lv = DIGI_PARA(type).level;
	if (lv >= MOD_TRAIN_LEVELS) {
		lv = MOD_TRAIN_LEVELS - 1;
	}
	if (MOD_TRAIN_NUM[mode][tt] < 0 || MOD_TRAIN_LEVEL[lv] < 0) {
		return 0;
	}
	num = MOD_TRAIN_NUM[mode][tt] * MOD_TRAIN_LEVEL[lv];
	den = 100;
	for (i = 0; i < MOD_TRAIN_BONUS_MAX; i++) {
		bonus = &MOD_TRAIN_BONUS[i];
		if (bonus->kind < 0 || bonus->den <= 0 || (bonus->modes & (1 << mode)) == 0) {
			continue;
		}
		if (bonus->kind == 0 && flag == 0 && isTriggerSet(bonus->arg) == 1) {
			num = num * bonus->num;
			den = den * bonus->den;
		} else if (bonus->kind == 1 && flag == 0) {
			counter = mode < 3 ? (&PARTNER_PARA.upgradeHPcounter)[mode] : (&PARTNER_PARA.upgradeDefenseCounter)[mode - 3];
			if (counter >= bonus->arg) {
				num = num * bonus->num;
				den = den * bonus->den;
			}
		} else if (bonus->kind == 2 && getItemCount(bonus->arg) > 0) {
			num = num * bonus->num;
			den = den * bonus->den;
		}
	}

	result = (num * 10) / den;
	return result;
}

void TRN_advanceTrainingTime(int16_t tiredGain, int16_t energyLoss, int16_t happyLoss)
{
	int16_t clockHour;

	PARTNER_PARA.tiredness += tiredGain;
	PARTNER_PARA.energyLevel -= energyLoss;
	PARTNER_PARA.happiness -= happyLoss;
	if (TRN_rule(MOD_TR_TIRED_AT) >= 0 && PARTNER_PARA.tiredness >= TRN_rule(MOD_TR_TIRED_AT)) { /* MOD (training as data): the session's rules */
		PARTNER_PARA.happiness -= TRN_amt(MOD_TR_PEN_HAPPY);
		PARTNER_PARA.discipline -= TRN_amt(MOD_TR_PEN_DISC);
	}

	CURRENT_FRAME = TRN_CURRENT_FRAME + 0x4b0;
	HOUR = TRN_CURRENT_HOUR + 1;
	if (HOUR >= 0x18) {
		PARTNER_PARA.age++;
		DAY++;
		dailyPStatTrigger();
		HOUR -= 0x18;
		CURRENT_FRAME -= 0x7080;
		if (DAY >= 0x1f) {
			DAY = 0;
			YEAR++;
			if (YEAR >= 0x64) {
				YEAR = 0;
			}
		}
	}

	clockHour = HOUR % 12;
	updateMinuteHand(HOUR, MINUTE);
	if ((HOUR % 4) == 0) {
		if (TRN_rule(MOD_TR_LIFE_BELOW) >= 0 && TRN_rule(MOD_TR_LIFE_PER) > 0 && PARTNER_PARA.happiness < TRN_rule(MOD_TR_LIFE_BELOW)) {
			PARTNER_PARA.remainingLifetime = PARTNER_PARA.remainingLifetime - (((PARTNER_PARA.happiness - TRN_rule(MOD_TR_LIFE_BELOW)) / -TRN_rule(MOD_TR_LIFE_PER)) + 1);
		}
	}

	if (HOUR == 0x10) {
		initializeDaytimeTransition(0);
	} else if (HOUR == 0x14) {
		initializeDaytimeTransition(1);
	} else if (HOUR == 6) {
		initializeDaytimeTransition(2);
	}

	if (PARTNER_PARA.condition & 1) {
		PARTNER_PARA.sicknessCounter++;
		PARTNER_PARA.missedSleepHours++;
	}

	if (PARTNER_PARA.condition & 4) {
		PARTNER_PARA.starvationTimer -= TRN_amt(MOD_TR_STARVE);
		if (PARTNER_PARA.starvationTimer <= 0) {
			if (PARTNER_PARA.energyLevel < RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyThreshold) {
				PARTNER_PARA.careMistakes++;
			}
		}
	} else {
		PARTNER_PARA.foodLevel -= TRN_amt(MOD_TR_FOOD);
	}

	PARTNER_PARA.energyLevel -= RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyUsage;
	if (PARTNER_PARA.condition & 8) {
		PARTNER_PARA.poopingTimer -= 0x4b0;
	} else {
		PARTNER_PARA.poopLevel -= TRN_amt(MOD_TR_POOP);
	}

	PARTNER_PARA.remainingLifetime -= TRN_amt(MOD_TR_LIFE_COST);
	PARTNER_PARA.evoTimer++;
	if (PARTNER_PARA.remainingLifetime < 0) {
		PARTNER_PARA.remainingLifetime = 0;
	}

	if (PARTNER_PARA.condition & 0x20) {
		PARTNER_PARA.injuryTimer++;
	}

	if (PARTNER_PARA.condition & 0x40) {
		PARTNER_PARA.sicknessTimer++;
	}

	tickConditionBoundaries();
}

void TRN_renderNewOrdersBox(void)
{
	renderString(0, -78, 24, 240, 48, 0, 120, 4, 0);
}

int32_t TRN_tryLearnMove(int32_t type)
{
	uint8_t special[3];
	int8_t unused[7] = { 0, 5, 2, 4, 2, 3, 6 };
	int8_t moveId[3];
	int8_t weight[3];
	int8_t move;
	int32_t n;
	int32_t r;
	int32_t j;
	int32_t i;
	int32_t k;
	int8_t best;

	for (n = 0; n < 3; n++) {
		special[n] = DIGI_PARA(type).special[n];
	}

	moveId[0] = moveId[1] = moveId[2] = -1;
	weight[0] = weight[1] = weight[2] = -1;
	for (i = 0; i < 3; i++) {
		if (special[i] == 0xff) {
			continue;
		}
		for (k = 0; k < 8; k++) {
			if (hasMove(TRN_D_8008F14C[k + special[i] * 8]) == 1) {
				continue;
			}
			for (j = 0; j < 16; j++) {
				if (TRN_D_8008F14C[k + special[i] * 8] == DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type).moves[j]) {
					moveId[i] = TRN_D_8008F14C[k + special[i] * 8];
					weight[i] = TRN_D_8008F184[k][i];
				}
			}
			if ((moveId[i] != -1) && (weight[i] != -1)) {
				break;
			}
		}
	}

	move = moveId[0];
	best = weight[0];
	if (best < weight[1]) {
		move = moveId[1];
		best = weight[1];
	}

	if (best < weight[2]) {
		move = moveId[2];
		best = weight[2];
	}

	r = randomLimit(100);
	if (best > r) {
		learnMove(move);
		drawString(MOVE_NAMES[move], 0, 0x78);
#if VERSION_IS(US)
		MAIN_D_80135392 = measureStringMode(MOVE_NAMES[move], FIT_VANILLA); /* MOD: its width (TRN_renderMoveLearnedBox) */
#else
		MAIN_D_80135392 = strlen(MOVE_NAMES[move]);
#endif
		drawString(TRN_D_8008F19C, 0, 0x84);
		createMenuBox(2, -0x58, 0x12, 0xb0, 0x23, 2, 0, TRN_renderMoveLearnedBox);
		MAIN_D_80135388 = 1;
		return 1;
	}

	return 0;
}

#if VERSION_IS(US)
/*
 * MOD: the box shows the technique's name (yellow) and the text after it ("was mastered!") as one line where it fits,
 * else the text on the next row. The original copied the name (strlen / 2) * 12 pixels wide, then 12 pixels of the
 * text and 48 more on the next row: the Japanese layout (two-byte characters in 12-pixel cells, "を" then "おぼえた"),
 * which cut the US line ("Spit F" + "w" / "as m"). Both strings were drawn whole into the text area by
 * TRN_tryLearnMove; MAIN_D_80135392 holds the name's width.
 */
#define LEARNED_TEXT_X	-78
#define LEARNED_TEXT_W	156 /* to the box's inner edge, as far right as the text starts left of it */
#define LEARNED_GAP	5   /* from the name's last ink to the text: a blank column and a space */

void TRN_renderMoveLearnedBox(void)
{
	int32_t nameW;
	int32_t textW;

	nameW = MAIN_D_80135392;
	textW = measureStringMode(TRN_D_8008F19C, FIT_VANILLA);
	renderString(3, LEARNED_TEXT_X, 24, nameW + 1, 12, 0, 120, 4, 0);
	if (nameW + LEARNED_GAP + textW <= LEARNED_TEXT_W) {
		renderString(0, LEARNED_TEXT_X + nameW + LEARNED_GAP, 24, textW + 1, 12, 0, 132, 4, 0);
	} else {
		renderString(0, LEARNED_TEXT_X, 36, textW + 1, 12, 0, 132, 4, 0);
	}
}
#else
void TRN_renderMoveLearnedBox(void)
{
	renderString(3, -78, 24, (MAIN_D_80135392 / 2) * 12, 12, 0, 120, 4, 0);
	renderString(0, ((MAIN_D_80135392 / 2) * 12) - 78, 24, 12, 12, 0, 132, 4, 0);
	renderString(0, -78, 36, 48, 12, 12, 132, 4, 0);
}
#endif
