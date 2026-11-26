#ifndef DW_MOD_HARD_H
#define DW_MOD_HARD_H
#include <dw/caps.h>
#include <dw/entity.h>
#include <dw/mod_battle.h>
#include <dw/mod_options.h>

/* Overlay-only helpers. Missing factors are neutral; products fit signed 32 bits. */
static inline int32_t modHardScale(int32_t value, int32_t slot, int32_t cap)
{
    int32_t factor, result;
    if (MOD_OPT_DIFFICULTY != MOD_DIFF_HARD) return value;
    factor = MOD_BATTLE[slot];
    if (factor <= 0) return value;
    result = value * factor / 100;
    return result > cap ? cap : result;
}

/* A late-game enemy (battleFormulas.normal): one whose maximum HP, after modHardEnemy's step, is what the step gives an
 * enemy of fromHp. An unscaled enemy stays under fromHp (HARD's 110% included), so the scaled HP tells them apart. */
static inline int32_t modLateEnemy(int32_t maxHP)
{
    return BM(NORMAL_FROM_HP) > 0 && maxHP >= BM(NORMAL_FROM_HP) * modBtSpan(MOD_BT_NORMAL_HP) / 100;
}

/* The least an enemy hit does, in percent of the same hit against Defense zero: HARD's floor for every enemy, NORMAL's
 * for the late-game ones (HARD keeps the larger). EASY has none. */
static inline int32_t modFloorPercent(int32_t attackerMaxHP)
{
    int32_t late = modLateEnemy(attackerMaxHP) ? BM(NORMAL_FLOOR) : 0;
    int32_t hard = BM(HARD_FLOOR);
    if (MOD_OPT_DIFFICULTY == MOD_DIFF_EASY) return 0;
    if (MOD_OPT_DIFFICULTY == MOD_DIFF_HARD) return hard > late ? hard : late;
    return late;
}

/* Round the minimum upward, including after ordinary block chip rounding. */
static inline int32_t modHardMinimum(int32_t zeroDamage, int32_t percent)
{
    return (zeroDamage * percent + 99) / 100;
}

static inline int32_t modHardBlockedFloor(int32_t damage, int32_t zeroDamage, int32_t blockPercent, int32_t percent)
{
    int32_t minimum;
    if (percent <= 0 || zeroDamage <= 0) return damage;
    minimum = modHardMinimum(zeroDamage * blockPercent / 100, percent);
    return damage < minimum ? minimum : damage;
}

/* Called once after encounter generation, before combat UI/AI setup. Original
 * field stats remain in INITIAL_COMBAT_STATS for rewards and flee restoration. */
static inline void modHardEnemy(Stats *stats)
{
    int32_t oldHP = stats->base.hp;
    if (MOD_OPT_DIFFICULTY == MOD_DIFF_EASY) return;
    if (BM(NORMAL_FROM_HP) > 0 && oldHP >= BM(NORMAL_FROM_HP)) {
        /* NORMAL and HARD: the late-game enemies are tougher (tools/balance.py battleFormulas normal.*); the early game is untouched */
        stats->base.hp = oldHP * modBtSpan(MOD_BT_NORMAL_HP) / 100;
        if (stats->base.hp > HPMP_CAP) stats->base.hp = HPMP_CAP;
        stats->current.currentHP = (int32_t)stats->current.currentHP * stats->base.hp / oldHP;
        stats->base.off = stats->base.off * modBtSpan(MOD_BT_NORMAL_OFF) / 100;
        if (stats->base.off > STAT_CAP) stats->base.off = STAT_CAP;
        oldHP = stats->base.hp;
    }
    if (MOD_OPT_DIFFICULTY != MOD_DIFF_HARD) return;
    stats->base.hp = modHardScale(stats->base.hp, MOD_BT_HARD_HP, HPMP_CAP);
    if (oldHP > 0) stats->current.currentHP = (int32_t)stats->current.currentHP * stats->base.hp / oldHP;
    stats->base.off = modHardScale(stats->base.off, MOD_BT_HARD_OFF, STAT_CAP);
    stats->base.def = modHardScale(stats->base.def, MOD_BT_HARD_DEF, STAT_CAP);
    stats->base.speed = modHardScale(stats->base.speed, MOD_BT_HARD_SPEED, STAT_CAP);
    stats->base.brain = modHardScale(stats->base.brain, MOD_BT_HARD_BRAIN, STAT_CAP);
}
#endif
