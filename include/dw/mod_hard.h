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

/* How far into the late game an enemy of this original maximum HP is, in thousandths (battleFormulas.normal): 0 below
 * fromHp, 1000 from fullHp on, a straight line between them. Without a fullHp above fromHp it is a single step. */
static inline int32_t modLatePermille(int32_t hp)
{
    int32_t from = BM(NORMAL_FROM_HP), span = BM(NORMAL_FULL_HP) - from;
    if (from <= 0 || hp < from) return 0;
    if (span <= 0 || hp - from >= span) return 1000;
    return (hp - from) * 1000 / span;
}

/* A percent of the late-game step for an enemy that far into it: 100 at 0, `full` at 1000. */
static inline int32_t modLatePercent(int32_t full, int32_t permille)
{
    return 100 + (full - 100) * permille / 1000;
}

/* NORMAL's floor for an attacker, from its maximum HP after modHardEnemy's step (the fight keeps no other trace of
 * it): nothing up to fromHp, all of it from what the step gives an enemy of fullHp, a straight line between them.
 * An enemy the step left alone stays at or under fromHp (HARD's own HP factor lifts it a little past it, where the
 * line is still about zero). */
static inline int32_t modLateFloor(int32_t maxHP)
{
    int32_t from = BM(NORMAL_FROM_HP), full = BM(NORMAL_FULL_HP), top;
    if (from <= 0) return 0;
    if (full < from) full = from;
    top = full * modBtSpan(MOD_BT_NORMAL_HP) / 100;
    if (maxHP >= top) return BM(NORMAL_FLOOR);
    if (full == from || maxHP <= from) return 0;
    return BM(NORMAL_FLOOR) * (maxHP - from) / (top - from);
}

/* The least an enemy hit does, in percent of the same hit against Defense zero: HARD's floor for every enemy, NORMAL's
 * for the late-game ones (HARD keeps the larger). EASY has none. */
static inline int32_t modFloorPercent(int32_t attackerMaxHP)
{
    int32_t late = modLateFloor(attackerMaxHP);
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
    int32_t oldHP = stats->base.hp, late;
    if (MOD_OPT_DIFFICULTY == MOD_DIFF_EASY) return;
    late = modLatePermille(oldHP);
    if (late > 0) {
        /* NORMAL and HARD: the late-game enemies are tougher, the more the later they are (tools/balance.py
         * battleFormulas normal.*); the early game is untouched */
        /* in 32 bits until capped: the stats are 16-bit fields */
        int32_t hp = oldHP * modLatePercent(modBtSpan(MOD_BT_NORMAL_HP), late) / 100;
        int32_t off = stats->base.off * modLatePercent(modBtSpan(MOD_BT_NORMAL_OFF), late) / 100;
        if (hp > HPMP_CAP) hp = HPMP_CAP;
        stats->base.hp = hp;
        stats->current.currentHP = (int32_t)stats->current.currentHP * hp / oldHP;
        stats->base.off = off > STAT_CAP ? STAT_CAP : off;
        oldHP = hp;
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
