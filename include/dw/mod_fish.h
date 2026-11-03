#ifndef DW_MOD_FISH_H
#define DW_MOD_FISH_H

#include <dw/types.h>

/* MOD (fishing as data): the tension gauge, the hooked fish, the bites, the bait theft and the fish population, MOD_FISH[] (16-bit slots,
 * -1 = the rule does not exist), written by tools/balance.py from content/tables/fishing-rules.json. A slot left out reads as 0.
 * Fishing runs one logic tick per 50 ms at any frame rate, so a count of "frames" is ticks: the JSON gives MOD_FS_SNAP_MULT, MOD_FS_HOLD_FRAMES
 * and MOD_FS_THEFT_EVERY in real milliseconds and balance.py writes them as ticks */
enum {
	MOD_FS_LIMIT_SMALL = 0,
	MOD_FS_LIMIT_BIG,
	MOD_FS_SIZE_MIN,
	MOD_FS_SIZE_MAX,
	MOD_FS_SLACK,
	MOD_FS_HOLD_LO,
	MOD_FS_HOLD_HI,
	MOD_FS_PULL_LO,
	MOD_FS_PULL_HI,
	MOD_FS_BOTH_LO,
	MOD_FS_BOTH_HI,
	MOD_FS_SNAP_BASE,
	MOD_FS_SNAP_NEAR,
	MOD_FS_SNAP_FAR,
	MOD_FS_SNAP_MULT,
	MOD_FS_BIG_SIZE,
	MOD_FS_NIB_S_MIN,
	MOD_FS_NIB_S_MAX,
	MOD_FS_NIB_B_MIN,
	MOD_FS_NIB_B_MAX,
	MOD_FS_CATCH_S,
	MOD_FS_CATCH_B,
	MOD_FS_POWER_ADD,
	MOD_FS_POWER_MAX_ADD,
	MOD_FS_TIRED_DIV,
	MOD_FS_STEP_TIRED,
	MOD_FS_STEP_FRESH,
	MOD_FS_SLACK_ROLL,
	MOD_FS_SLACK_BELOW,
	MOD_FS_HOLD_FRAMES,
	MOD_FS_MOVE_MIN,
	MOD_FS_MOVE_MAX,
	MOD_FS_PAUSE_MIN,
	MOD_FS_PAUSE_MAX,
	MOD_FS_BITE_RADIUS,
	MOD_FS_SEA_POLL,
	MOD_FS_SEA_CHANCE,
	MOD_FS_BITE_POLL,
	MOD_FS_BITE_ROLL,
	MOD_FS_THEFT_FROM,
	MOD_FS_THEFT_EVERY,
	MOD_FS_THEFT_ROLL,
	MOD_FS_THEFT_BELOW,
	MOD_FS_POOL_MAX,
	MOD_FS_LIFE_MIN,
	MOD_FS_LIFE_MAX,
	MOD_FS_SIZE
};

extern int16_t MOD_FISH[MOD_FS_SIZE];

static inline int32_t modFishVal(int32_t slot) { return MOD_FISH[slot] > 0 ? MOD_FISH[slot] : 0; }
static inline int32_t modFishSpan(int32_t slot) { return MOD_FISH[slot] > 1 ? MOD_FISH[slot] : 1; }
static inline int32_t modFishDiv(int32_t a, int32_t slot) { return MOD_FISH[slot] > 0 ? a / MOD_FISH[slot] : 0; }

#define BF(X) modFishVal(MOD_FS_##X)
#define BFS(X) modFishSpan(MOD_FS_##X)
#define BFD(a, slot) modFishDiv((a), (slot))
#define BFDV(a, X) modFishDiv((a), MOD_FS_##X)

#endif
