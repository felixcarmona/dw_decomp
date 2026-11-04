#ifndef DW_CAPS_H
#define DW_CAPS_H

/* MOD (caps): the number limits. The original capped Offense, Defense, Speed
 * and Brains at 999, max HP/MP at 9999, and one hit and the pending damage
 * (hpDamageBuffer) at 9999. Every stat field stays an int16_t (the partner,
 * the fighters, COMBAT_DATA, DIGIMON_DATA and the v1 save all store them so),
 * so the new limits are what an int16_t holds with room for a sum:
 * 4 digits for the stats and 5 for HP, MP and damage. Below the old limits
 * every formula, number and bar is as in the original. */
#define STAT_CAP 9999    /* Offense, Defense, Speed, Brains (was 999) */
#define HPMP_CAP 30000   /* max HP and max MP (was 9999) */
#define DAMAGE_CAP 30000 /* one hit, and hpDamageBuffer (was 9999) */

#define OLD_STAT_CAP 999
#define OLD_HPMP_CAP 9999

/* a stat index of INITIAL_COMBAT_STATS / STATS_GAINS (hp, mp, off, def,
 * speed, brains): its cap */
#define CAP_OF_STAT_INDEX(i) ((i) < 2 ? HPMP_CAP : STAT_CAP)
#define OLD_CAP_OF_STAT_INDEX(i) ((i) < 2 ? OLD_HPMP_CAP : OLD_STAT_CAP)

/* "full" healing: the original healed 9999 (= all of a capped bar); with a
 * bigger bar it heals all of it, otherwise exactly as before */
#define CAPS_FULL_HEAL(amount, max) \
	(((amount) >= OLD_HPMP_CAP && (max) > (amount)) ? (max) : (amount))

#define CAPS_MIN(a, b) ((a) < (b) ? (a) : (b))

/* a stat where a formula was made for 0..999 and would break past it */
#define CAPS_OLD_STAT(v) ((v) > OLD_STAT_CAP ? OLD_STAT_CAP : (v))

/* hpDamageBuffer + a hit, summed in 32 bits and clamped (the original
 * summed into the int16_t and clamped at 9999) */
#define CAPS_ADD_DAMAGE(buffer, dmg) \
	((int32_t)(buffer) + (dmg) > DAMAGE_CAP ? DAMAGE_CAP : (int32_t)(buffer) + (dmg))

/* digits a number box needs: n as in the original, more only when the value
 * no longer fits (the box then grows to the left, see CAPS_SHIFT) */
#define CAPS_DIGITS(n, value)                                                  \
	((value) >= 100000 ? ((n) > 6 ? (n) : 6) :                             \
	 (value) >= 10000  ? ((n) > 5 ? (n) : 5) :                             \
	 (value) >= 1000   ? ((n) > 4 ? (n) : 4) :                             \
	 (value) >= 100    ? ((n) > 3 ? (n) : 3) :                             \
	                     (n))
/* how far a right-aligned box of n digits moves left for that */
#define CAPS_SHIFT(n, value, width) ((CAPS_DIGITS(n, value) - (n)) * (width))

#endif
