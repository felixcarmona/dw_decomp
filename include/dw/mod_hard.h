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

/* Round the minimum upward, including after ordinary block chip rounding. */
static inline int32_t modHardMinimum(int32_t zeroDamage)
{
    return (zeroDamage * BM(HARD_FLOOR) + 99) / 100;
}

static inline int32_t modHardBlockedFloor(int32_t damage, int32_t zeroDamage, int32_t blockPercent)
{
    int32_t minimum;
    if (MOD_OPT_DIFFICULTY != MOD_DIFF_HARD || zeroDamage <= 0) return damage;
    minimum = modHardMinimum(zeroDamage * blockPercent / 100);
    return damage < minimum ? minimum : damage;
}

/* Called once after encounter generation, before combat UI/AI setup. Original
 * field stats remain in INITIAL_COMBAT_STATS for rewards and flee restoration. */
static inline void modHardEnemy(Stats *stats)
{
    int32_t oldHP = stats->base.hp;
    if (MOD_OPT_DIFFICULTY != MOD_DIFF_HARD) return;
    stats->base.hp = modHardScale(stats->base.hp, MOD_BT_HARD_HP, HPMP_CAP);
    if (oldHP > 0) stats->current.currentHP = (int32_t)stats->current.currentHP * stats->base.hp / oldHP;
    stats->base.off = modHardScale(stats->base.off, MOD_BT_HARD_OFF, STAT_CAP);
    stats->base.def = modHardScale(stats->base.def, MOD_BT_HARD_DEF, STAT_CAP);
    stats->base.speed = modHardScale(stats->base.speed, MOD_BT_HARD_SPEED, STAT_CAP);
    stats->base.brain = modHardScale(stats->base.brain, MOD_BT_HARD_BRAIN, STAT_CAP);
}
#endif
