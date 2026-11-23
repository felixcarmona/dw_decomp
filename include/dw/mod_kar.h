#ifndef DW_MOD_KAR_H
#define DW_MOD_KAR_H

#include <dw/types.h>

/* MOD (curling as data): friction, obstacles, the big-win score, the opponent's aim noise and the throw power cuts, MOD_KAR[] (16-bit slots,
 * -1 = the rule does not exist), written by tools/balance.py from content/tables/curling.json. A slot left out reads as 0 (a modulus as 1) */
enum {
	MOD_KR_FRICTION = 0,
	MOD_KR_MIN_MOVE,
	MOD_KR_OBST_MAX,
	MOD_KR_OBST_X_SPAN,
	MOD_KR_OBST_Z_BASE,
	MOD_KR_OBST_Z_SPAN,
	MOD_KR_BIG_WIN,
	MOD_KR_AI_NOISE,
	MOD_KR_POWER_SUB_A,
	MOD_KR_POWER_SUB_B,
	MOD_KR_SIZE
};

extern int16_t MOD_KAR[MOD_KR_SIZE];

static inline int32_t modKarVal(int32_t slot) { return MOD_KAR[slot] > 0 ? MOD_KAR[slot] : 0; }
static inline int32_t modKarSpan(int32_t slot) { return MOD_KAR[slot] > 1 ? MOD_KAR[slot] : 1; }

#define BK(X) modKarVal(MOD_KR_##X)
#define BKS(X) modKarSpan(MOD_KR_##X)

#endif
