#include <dw/caps.h> /* MOD (caps) */
#include <dw/anim.h>
#include <dw/mod_rules.h>
#include <dw/entity.h>
#include <dw/input.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/sound.h>
#include <dw/trn2.h>
#include <dw/types.h>
#include <dw/world_object.h>

extern int32_t TRAINING_COMPLETE;
extern int16_t STATS_GAINS[6];
extern uint32_t POLLED_INPUT;

void createCameraMovement(VECTOR *pos, int32_t speed);
void setMapObjectsFlag(int16_t start, int16_t count, int32_t flag);
void setCameraFollowPlayer(void);
void unsetCameraFollowPlayer(void);
void TRN2_awardSpeedTrainingGains(int32_t a, int32_t b, int32_t c);
void TRN2_func_8008AA84(int8_t arg);
void tamerSetState(int8_t state);
int32_t tickEntityWalkTo();

static void *trn2_reward_functions[] = {
	TRN2_awardDefenseTrainingGains,
	TRN2_awardSpeedTrainingGains,
	TRN2_awardOffenseTrainingGains,
	TRN2_awardMpTrainingGains,
	TRN2_statGainsAreZero,
	TRN2_awardHpTrainingGains,
	TRN2_func_8008AA84,
	TRN2_startSlotSessionIfEnabled,
	TRN2_tickSpeedTraining,
};

uint32_t MAIN_D_801353C8;
int16_t MAIN_D_801353CC;

void TRN2_tickSpeedTraining(int32_t instanceId)
{
	int32_t r;

	switch (MAIN_D_801353BD) {
	case 0:
		tamerSetState(8);
		unsetCameraFollowPlayer();
		partnerSetState(10);
		startAnimation(&PARTNER_ENTITY.digimonEntity.entity, 4);
		createCameraMovement(&TRN2_D_8008DC1C, 10);
		playSound(8, 9);
		MAIN_D_801353C2 = 0;
		MAIN_D_801353BD = 1;
		TRN2_startSlotSessionIfEnabled(4);
		break;
	case 1:
		TRN2_func_8008AA84(4);
		if (tickEntityWalkTo(0xfc, 0xff, TRN2_D_8008DC2C.vx, TRN2_D_8008DC2C.vz, 0) == 1) {
			MAIN_D_801353C8 = playSound2(8, 0xe);
			setMapObjectsFlag(MAIN_D_801353B8, MAIN_D_801353BA, 0);
			MAIN_D_801353BD = 2;
		}
		break;
	case 2:
		if (tickEntityWalkTo(0xfc, 0xff, TRN2_D_8008DC1C.vx, TRN2_D_8008DC1C.vz, 0) == 1) {
			PARTNER_ENTITY.digimonEntity.entity.posData->rotation.vy = 0xa50;
			startAnimation(&PARTNER_ENTITY.digimonEntity.entity, 4);
			MAIN_D_801353BD = 3;
			if (MAIN_D_801353BC == 1) {
				TRN2_startSlotSpin();
			}
		}
		break;
	case 3:
		MAIN_D_801353C2++;
		PARTNER_ENTITY.digimonEntity.entity.anim.animFlag |= 2;
		r = 10;
		if (MAIN_D_801353BC == 1) {
			r = TRN2_getSlotSessionResult();
		}
		if ((((MAIN_D_801353C2 >= 0x4b0) || (POLLED_INPUT & CANCEL_BUTTON)) && (MAIN_D_801353BC == 0)) || ((MAIN_D_801353BC == 1) && (r >= 0))) {
			setMapObjectsFlag(MAIN_D_801353B8, MAIN_D_801353BA, 1);
			TRN2_awardSpeedTrainingGains(PARTNER_ENTITY.digimonEntity.entity.type, instanceId, r);
			startAnimation(&PARTNER_ENTITY.digimonEntity.entity, 0x12);
			playSound(8, 0xa);
			thunkStopSoundMask(MAIN_D_801353C8);
			createCameraMovement(&TAMER_ENTITY.entity.posData->location, 10);
			MAIN_D_801353BD = 4;
		}
		break;
	case 4:
		if (TRN2_statGainsAreZero() == 1) {
			MAIN_D_801353C4 = 0;
			MAIN_D_801353BD = 5;
		}
		break;
	case 5:
		MAIN_D_801353C4++;
		if (MAIN_D_801353C4 >= 0x14) {
			startAnimation(&PARTNER_ENTITY.digimonEntity.entity, 4);
			MAIN_D_801353BD = 6;
		}
		break;
	case 6:
		if (tickEntityWalkTo(0xfc, 0xff, TRN2_D_8008DC2C.vx, TRN2_D_8008DC1C.vz, 0) == 1) {
			TRN2_applyBaseStats();
			TRN2_closeUIBox(1);
			partnerSetState(1);
			setCameraFollowPlayer();
			MAIN_D_801353C2 = 0;
			MAIN_D_801353BD = 0;
			removeObject(0xfad, instanceId);
			TRAINING_COMPLETE = 1;
		}
		break;
	}
}

void TRN2_startSlotSessionIfEnabled(int16_t arg)
{
	if (MAIN_D_801353BC == 1) {
		while (TRN2_startSlotSession(arg) != 0) {
		}
	}
}

void TRN2_func_8008AA84(int8_t arg)
{
	if (MAIN_D_801353BC == 1) {
		if (TRN2_getSlotSessionResult() == -2) {
			startAnimation(ENTITY_TABLE[1], 4);
			createCameraMovement(&TAMER_ENTITY.entity.posData->location, 10);
			MAIN_D_801353BD = arg;
		}
	}
}

/* MOD (training as data): written by tools/balance.py from content/tables/training-gains.json; -1 = does not exist */
int16_t MOD_TRAIN2_GAIN[MOD_TG_ROWS][MOD_TG_WIDTH] = { [0 ... MOD_TG_ROWS - 1] = { [0 ... MOD_TG_WIDTH - 1] = -1 } };
int16_t MOD_TRAIN2_RULES[MOD_TR_SIZE] = { [0 ... MOD_TR_SIZE - 1] = -1 };

static int32_t TRN2_rule(int32_t slot)
{
	return MOD_TRAIN2_RULES[slot];
}

static int32_t TRN2_amt(int32_t slot)
{
	return MOD_TRAIN2_RULES[slot] > 0 ? MOD_TRAIN2_RULES[slot] : 0;
}

void TRN2_awardHpTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t x;
	int16_t y;
	int16_t v;
	int16_t flag;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	const int16_t *row;

	x = flag = y = 0;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN2_GAIN, MOD_TG_HP, b);
	x = TG(row, MOD_TGF_HP);
	y = TG(row, MOD_TGF_DEF);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN2_calculateTrainingMultiplier(a, 0);
	if (PARTNER_PARA.trainBoostFlag & 0x10) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 0x10) {
		y = (y * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		y = (y * v) / 10;
	}

	x = (x * c) / 10;
	y = (y * c) / 10;
	if (PARTNER_PARA.tiredness == TRN2_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_HP) > 0) {
			x = TRN2_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_DEF) > 0) {
			y = TRN2_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN2_saveBaseStats();
	STATS_GAINS[0] = x;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = y;
	STATS_GAINS[4] = 0;
	STATS_GAINS[5] = 0;
	TRN2_createPostTrainingStatsBox();
	TRN2_advanceTrainingTime(tired, energy, happy);
}

int32_t TRN2_statGainsAreZero(void)
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

void TRN2_awardMpTrainingGains(int32_t a, int16_t b, int32_t c)
{
	int16_t flag;
	int16_t x;
	int16_t y;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	int16_t v;
	const int16_t *row;

	x = y = 0;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN2_GAIN, MOD_TG_MP, b);
	x = TG(row, MOD_TGF_MP);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN2_calculateTrainingMultiplier(a, 1);
	if (PARTNER_PARA.trainBoostFlag & 0x20) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	x = (x * c) / 10;
	if (PARTNER_PARA.tiredness == TRN2_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_MP) > 0) {
			x = TRN2_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN2_saveBaseStats();
	STATS_GAINS[0] = 0;
	STATS_GAINS[1] = x;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = 0;
	STATS_GAINS[4] = 0;
	STATS_GAINS[5] = 0;
	TRN2_createPostTrainingStatsBox();
	TRN2_advanceTrainingTime(tired, energy, happy);
}

void TRN2_awardOffenseTrainingGains(int32_t a, int16_t b, int32_t c)
{
	int16_t flag;
	int16_t x;
	int16_t y;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	int16_t v;
	const int16_t *row;

	flag = x = y = 0;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN2_GAIN, MOD_TG_OFF, b);
	x = TG(row, MOD_TGF_OFF);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN2_calculateTrainingMultiplier(a, 2);
	if (PARTNER_PARA.trainBoostFlag & 1) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	x = (x * c) / 10;
	if (PARTNER_PARA.tiredness == TRN2_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_OFF) > 0) {
			x = TRN2_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN2_saveBaseStats();
	STATS_GAINS[0] = 0;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = x;
	STATS_GAINS[3] = 0;
	STATS_GAINS[4] = 0;
	STATS_GAINS[5] = 0;
	TRN2_createPostTrainingStatsBox();
	TRN2_advanceTrainingTime(tired, energy, happy);
}

void TRN2_awardSpeedTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t flag;
	int16_t x;
	int16_t y;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	int16_t v;
	const int16_t *row;

	y = x = 0;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN2_GAIN, MOD_TG_SPD, b);
	x = TG(row, MOD_TGF_SPD);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN2_calculateTrainingMultiplier(a, 4);
	if (PARTNER_PARA.trainBoostFlag & 4) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	x = (x * c) / 10;
	if (PARTNER_PARA.tiredness == TRN2_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_SPD) > 0) {
			x = TRN2_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN2_saveBaseStats();
	STATS_GAINS[0] = 0;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = 0;
	STATS_GAINS[4] = x;
	STATS_GAINS[5] = 0;
	TRN2_createPostTrainingStatsBox();
	TRN2_advanceTrainingTime(tired, energy, happy);
}

void TRN2_awardDefenseTrainingGains(a, b, c)
int32_t a;
int16_t b;
int32_t c;
{
	int16_t x;
	int16_t y;
	int16_t v;
	int16_t unused1;
	int16_t unused2;
	int16_t unused3;
	int16_t flag;
	int16_t z;
	int16_t tired;
	int16_t energy;
	int16_t happy;
	const int16_t *row;

	flag = x = y = z = 0;
	/* MOD (training as data): the gym's row for this slot */
	row = trainGainRow(MOD_TRAIN2_GAIN, MOD_TG_DEF, b);
	x = TG(row, MOD_TGF_DEF);
	y = TG(row, MOD_TGF_SPD);
	tired = TG(row, MOD_TGF_TIRED);
	energy = TG(row, MOD_TGF_ENERGY);
	happy = TG(row, MOD_TGF_HAPPY);

	v = TRN2_calculateTrainingMultiplier(a, 3);
	if (PARTNER_PARA.trainBoostFlag & 2) {
		x = (x * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		x = (x * v) / 10;
	}

	if (PARTNER_PARA.trainBoostFlag & 2) {
		y = (y * (v * PARTNER_PARA.trainBoostValue)) / 100;
	} else {
		y = (y * v) / 10;
	}

	x = (x * c) / 10;
	y = (y * c) / 10;
	if (PARTNER_PARA.tiredness == TRN2_rule(MOD_TR_EXH_AT)) {
		if (TG(row, MOD_TGF_DEF) > 0) {
			x = TRN2_amt(MOD_TR_EXH_GAIN);
		}
		if (TG(row, MOD_TGF_SPD) > 0) {
			y = TRN2_amt(MOD_TR_EXH_GAIN);
		}
	}

	if (x > STAT_CAP) { /* MOD (caps): a gain up to the new limit (was 999) */
		x = STAT_CAP;
	}

	TRN2_saveBaseStats();
	STATS_GAINS[0] = 0;
	STATS_GAINS[1] = 0;
	STATS_GAINS[2] = 0;
	STATS_GAINS[3] = x;
	STATS_GAINS[4] = y;
	STATS_GAINS[5] = 0;
	TRN2_createPostTrainingStatsBox();
	TRN2_advanceTrainingTime(tired, energy, happy);
}
