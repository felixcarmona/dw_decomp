#ifndef DW_TECH_RULES_H
#define DW_TECH_RULES_H

#include <dw/entity.h>
#include <dw/move.h>

/* MOD (new technique rows): what the techniques of the new rows do besides damage, shared by the
 * field battle, the tournament and the versus fight. Move.status says which and Move.statusChance is its percent:
 *   5  gives back that share of the caster's full HP
 *   6  raises the caster's Defense, 7 its Speed (as the original buff techniques do)
 *   8  lowers the Defense of the fighter it reaches, once per fighter and battle
 *   9  raises the caster's Offense */
#define TECH_HEAL	5
#define TECH_DEF_UP	6
#define TECH_SPEED_UP	7
#define TECH_DEF_DOWN	8
#define TECH_OFF_UP	9

#define TECH_HEALS_PARTNER	3	/* heals per battle: the partner (and both sides of a versus fight) */
#define TECH_HEALS_ENEMY	2	/* an enemy or a tournament rival */
#define TECH_HEAL_AUTO_PARTNER	60	/* without an order, only below this percent of the full HP */
#define TECH_HEAL_AUTO_ENEMY	50
#define TECH_AI_HEAL_WEIGHT	60	/* added to the technique's weight in the fighter's choice once it may heal */

typedef struct {
	uint8_t heals[4];	/* by fighter slot */
	uint8_t weakened;	/* bit per fighter slot */
	uint8_t ordered;	/* the check is for an order of the player */
} TechRules;

/* 0: the fighter in slot may not start the technique now (target: the slot it aims at, 0xff none) */
static inline int32_t techRulesAllow(TechRules *r, Move *m, Stats *s, int32_t slot, int32_t own, int32_t target)
{
	if (m->status == TECH_HEAL) {
		if (s->current.currentHP >= s->base.hp) return 0;
		if (r->heals[slot & 3] >= (own ? TECH_HEALS_PARTNER : TECH_HEALS_ENEMY)) return 0;
		if (!r->ordered && s->current.currentHP * 100 >= s->base.hp * (own ? TECH_HEAL_AUTO_PARTNER : TECH_HEAL_AUTO_ENEMY)) return 0;
	} else if (m->status == TECH_DEF_DOWN) {
		if (target == 0xff || ((r->weakened >> (target & 3)) & 1)) return 0;
	}
	return 1;
}

/* What the fighter's choice adds for the technique (prio: its fading taste for buffs) */
static inline int32_t techRulesWeight(Move *m, int32_t prio)
{
	return m->status == TECH_HEAL ? TECH_AI_HEAL_WEIGHT : m->status == TECH_DEF_DOWN ? prio : 0;
}

#define techRulesReset(r) ((r)->heals[0] = (r)->heals[1] = (r)->heals[2] = (r)->heals[3] = (r)->weakened = (r)->ordered = 0)

#endif
