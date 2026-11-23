#ifndef DW_MOD_BATTLE_H
#define DW_MOD_BATTLE_H

#include <dw/types.h>

/* MOD (battle as data): the numbers of the fight (damage, hit chance, blocks, buffs, status, MP, speed, senility, setup), MOD_BATTLE[]
 * (16-bit slots, -1 = the rule does not exist), written by tools/balance.py from content/tables/battle-formulas.json.
 * One table in the executable serves the BTL, STD and VS overlays. A slot left out reads as 0 (a divisor or a die span as 1 / no division). */
enum {
	MOD_BT_GUARD_PCT = 0,
	MOD_BT_DMG_DIV,
	MOD_BT_DIFF_CLAMP,
	MOD_BT_DIFF_DIV,
	MOD_BT_NEUTRAL_MULT,
	MOD_BT_RAND_MIN,
	MOD_BT_RAND_SPAN,
	MOD_BT_ENEMY_FIN_MIN,
	MOD_BT_ENEMY_FIN_SPAN,
	MOD_BT_CHARGE_FROM,
	MOD_BT_CHARGE_DIV,
	MOD_BT_DMG_MIN,
	MOD_BT_HIT_AUTO_LEVEL,
	MOD_BT_HIT_SPEED_DIV,
	MOD_BT_HIT_SPEED_CLAMP,
	MOD_BT_HIT_ACC_DIV,
	MOD_BT_HIT_SPEED_NORM,
	MOD_BT_HIT_MOVING_PCT,
	MOD_BT_BLOCK_MIN_MP,
	MOD_BT_BLOCK_DMG_MIN,
	MOD_BT_BLOCK_DMG_SPAN,
	MOD_BT_BLOCK_INVULN,
	MOD_BT_BLOCK_GAIN_DEF,
	MOD_BT_BLOCK_GAIN_ATK,
	MOD_BT_BIGHIT_PCT,
	MOD_BT_BUFF_MOVE0,
	MOD_BT_BUFF_MOVE1,
	MOD_BT_BUFF_MOVE2,
	MOD_BT_BUFF_MOVE3,
	MOD_BT_BUFF_MOVE4,
	MOD_BT_BUFF_OFF0,
	MOD_BT_BUFF_OFF1,
	MOD_BT_BUFF_OFF2,
	MOD_BT_BUFF_OFF3,
	MOD_BT_BUFF_OFF4,
	MOD_BT_BUFF_DEF0,
	MOD_BT_BUFF_DEF1,
	MOD_BT_BUFF_DEF2,
	MOD_BT_BUFF_DEF3,
	MOD_BT_BUFF_DEF4,
	MOD_BT_BUFF_SPD0,
	MOD_BT_BUFF_SPD1,
	MOD_BT_BUFF_SPD2,
	MOD_BT_BUFF_SPD3,
	MOD_BT_BUFF_SPD4,
	MOD_BT_POISON_FRAMES,
	MOD_BT_POISON_MIN,
	MOD_BT_POISON_SPAN,
	MOD_BT_CONF_BASE,
	MOD_BT_CONF_SPAN,
	MOD_BT_STUN_BASE,
	MOD_BT_STUN_SPAN,
	MOD_BT_MP_MULT,
	MOD_BT_MP_COND_PCT,
	MOD_BT_MP_BRAIN0,
	MOD_BT_MP_BRAIN1,
	MOD_BT_MP_BRAIN2,
	MOD_BT_MP_BRAIN3,
	MOD_BT_MP_DISC0,
	MOD_BT_MP_DISC1,
	MOD_BT_MP_DISC2,
	MOD_BT_MP_DISC3,
	MOD_BT_SPEED_MAX,
	MOD_BT_SPEED_DIV,
	MOD_BT_SPEED_PLUS,
	MOD_BT_SPEED_FLOOR,
	MOD_BT_SPEED_START,
	MOD_BT_CD_ATTACK,
	MOD_BT_CD_IDLE,
	MOD_BT_SEN_BRAIN,
	MOD_BT_SEN_PARTNER,
	MOD_BT_SEN_ENEMY,
	MOD_BT_SEN_ENEMY_DIV,
	MOD_BT_SEN_FRAMES,
	MOD_BT_SEN_INT_DIV,
	MOD_BT_SEN_INT_MULT,
	MOD_BT_FIN_ATTACK_PCT,
	MOD_BT_FIN_GOAL,
	MOD_BT_COND_STAT_PCT,
	MOD_BT_BUFFUSE_DIV,
	MOD_BT_BUFFUSE_T1,
	MOD_BT_BUFFUSE_T2,
	MOD_BT_BUFFUSE_N1,
	MOD_BT_BUFFUSE_N2,
	MOD_BT_PRIO_DIV,
	MOD_BT_PRIO_BASE,
	MOD_BT_IDLE_STOP_PCT,
	MOD_BT_FLAT_BASE,
	MOD_BT_FLAT_SPAN,
	MOD_BT_FLEE_IGNORE,
	MOD_BT_FLEE_LOCK,
	MOD_BT_CONF_NOTARGET,
	MOD_BT_WEAKEST_BRAIN,
	MOD_BT_ENEMY_AIM,
	MOD_BT_LOWHP_DIV,
	MOD_BT_AI_EMPTY_WEIGHT,
	MOD_BT_AI_MODE1_WEIGHT,
	MOD_BT_AI_GROUP_BRAIN,
	MOD_BT_AI_GROUP_BONUS0,
	MOD_BT_AI_GROUP_BONUS1,
	MOD_BT_AI_GROUP_BONUS2,
	MOD_BT_AI_LOWHP_PCT,
	MOD_BT_AI_ENEMY_GROUP_WEIGHT,
	MOD_BT_PRIO_POWER0,
	MOD_BT_PRIO_POWER1,
	MOD_BT_PRIO_POWER2,
	MOD_BT_PRIO_POWER3,
	MOD_BT_PRIO_MP0,
	MOD_BT_PRIO_MP1,
	MOD_BT_PRIO_MP2,
	MOD_BT_PRIO_MP3,
	MOD_BT_PRIO_WIDE0,
	MOD_BT_PRIO_WIDE1,
	MOD_BT_PRIO_WIDE2,
	MOD_BT_PRIO_WIDE3,
	MOD_BT_ELEM0,
	MOD_BT_ELEM1,
	MOD_BT_ELEM2,
	MOD_BT_ELEM3,
	MOD_BT_ELEM4,
	MOD_BT_TSCORE_INV0,
	MOD_BT_TSCORE_INV1,
	MOD_BT_TSCORE_INV2,
	MOD_BT_TSCORE_INV3,
	MOD_BT_TSCORE_DEF,
	MOD_BT_TSCORE_DIV,
	MOD_BT_SMART_BRAIN,
	MOD_BT_SWITCH_GAP,
	MOD_BT_CMD_STEP,
	MOD_BT_CMD_MAX,
	MOD_BT_CMD_BTL0,
	MOD_BT_CMD_BTL1,
	MOD_BT_CMD_BTL2,
	MOD_BT_CMD_BTL3,
	MOD_BT_CMD_BTL4,
	MOD_BT_CMD_BTL_TOP,
	MOD_BT_CMD_STD0,
	MOD_BT_CMD_STD1,
	MOD_BT_CMD_STD2,
	MOD_BT_CMD_STD3,
	MOD_BT_CMD_STD4,
	MOD_BT_CMD_STD_TOP,
	MOD_BT_HEAL_HP,
	MOD_BT_HEAL_MP,
	MOD_BT_CHG_MAX,
	MOD_BT_CHG_PRESS,
	MOD_BT_CHG_WINDOW,
	MOD_BT_FIN_HOLD,
	MOD_BT_T_FLAT_RETARGET,
	MOD_BT_T_PRIO_STEP,
	MOD_BT_T_FLAT_REPEAT,
	MOD_BT_T_SPEED_STEP,
	MOD_BT_T_APPROACH,
	MOD_BT_T_APPROACH_STD,
	MOD_BT_T_APPROACH_REST,
	MOD_BT_T_FLAT_ATTACK,
	MOD_BT_T_DEFEAT_SHOW,
	MOD_BT_T_DEFEAT_END,
	MOD_BT_T_SETTLE,
	MOD_BT_T_DIGIT,		/* ticks a digit of the defeat countdown shows (it pulses 20 ticks of them; the countdown ends after 5) */
	MOD_BT_MUSIC_LAST_SPECIES,	/* the species that, as the first enemy of the fight (COMBAT_DATA_PTR->player.entityIds[1], Machinedramon in the original), makes the battle play the last-battle track (-1: none) */
	MOD_BT_MUSIC_LAST_TRACK,	/* the track (font 0x21) it plays */
	MOD_BT_TAIL_HALF,
	MOD_BT_TAIL_DIV,
	MOD_BT_HARD_HP,
	MOD_BT_HARD_OFF,
	MOD_BT_HARD_DEF,
	MOD_BT_HARD_SPEED,
	MOD_BT_HARD_BRAIN,
	MOD_BT_HARD_FIN,
	MOD_BT_HARD_DEALT,
	MOD_BT_HARD_IDLE,
	MOD_BT_HARD_FLOOR,
	MOD_BT_DEF_KNEE,
	MOD_BT_DEF_HALF,
	MOD_BT_SIZE
};

extern int16_t MOD_BATTLE[MOD_BT_SIZE];

static inline int32_t modBtVal(int32_t slot) { return MOD_BATTLE[slot] > 0 ? MOD_BATTLE[slot] : 0; }
static inline int32_t modBtSpan(int32_t slot) { return MOD_BATTLE[slot] > 1 ? MOD_BATTLE[slot] : 1; }
/* every `slot` ticks (a time key in the JSON is in milliseconds, tools/balance.py divides by one tick = 50 ms); a rule left out never fires */
static inline int32_t modBtEvery(int32_t count, int32_t slot) { return MOD_BATTLE[slot] > 0 && count % MOD_BATTLE[slot] == 0; }
static inline int32_t modBtDiv(int32_t a, int32_t slot) { return MOD_BATTLE[slot] > 0 ? a / MOD_BATTLE[slot] : 0; }
int32_t modBtBrainDiscount(int32_t brain);

#define BM(X) modBtVal(MOD_BT_##X)
#define BMS(X) modBtSpan(MOD_BT_##X)
#define BMD(a, X) modBtDiv((a), MOD_BT_##X)
#define BMV(X) ((int32_t)MOD_BATTLE[MOD_BT_##X])

/* Preserve signed division and rounding inside the original range. Above the
 * positive knee, a bounded rational tail approaches one extra power. The
 * supported nonnegative int16 stats/power keep power * excess within int32. */
static inline int32_t modBtStatPower(int32_t power, int32_t difference)
{
	int32_t knee = BM(DIFF_CLAMP);
	int32_t limited = difference;
	int32_t result;

	if (limited > knee) limited = knee;
	if (limited < -knee) limited = -knee;
	result = power + BMD(limited * power, DIFF_DIV);
	if (difference > knee && BM(TAIL_HALF) > 0 && BM(TAIL_DIV) > 0) {
		int32_t excess = difference - knee;
		result += power * excess / ((excess + BM(TAIL_HALF)) * BM(TAIL_DIV));
	}
	return result;
}

/* Continue excess Defense with a reciprocal tail once it exceeds both the
 * original range and Offense. Positive attack advantages keep their existing
 * curve. Counter transforms Defense first; the original raw range is exact. */
static inline int32_t modBtDefensePower(int32_t power, int32_t off, int32_t defense, int32_t counter)
{
    int32_t knee = BM(DEF_KNEE), half = BM(DEF_HALF);
    int32_t excess = 0, effective = defense, result;
    if (counter) effective = effective * BM(GUARD_PCT) / 100;
    if (knee > 0 && half > 0 && defense > knee) {
        if (off > knee) knee = off;
        if (effective > knee) {
            excess = effective - knee;
            effective = knee;
        }
    }
    result = modBtStatPower(power, off - effective);
    if (excess) result = result * half / (half + excess);
    return result;
}

/* the command menu a partner gets from its brains: the table row for brains / CMD_STEP below CMD_MAX, else the top menu (`first` is the row's first slot) */
static inline int32_t modBtCommands(int32_t brain, int32_t first, int32_t top)
{
	int32_t i = brain / modBtSpan(MOD_BT_CMD_STEP);

	if (brain < modBtVal(MOD_BT_CMD_MAX) && i < 5) {
		i = modBtVal(first + i);
	} else {
		i = modBtVal(top);
	}
	return i > 0 ? i : 1; /* the menu always has the finisher entry: with none the command menu hangs */
}
#define BMEVERY(count, X) modBtEvery((count), MOD_BT_##X)
#define BMBRAIN(brain) modBtBrainDiscount(brain)

#endif
