#include <dw/rng.h>
#include <dw/types.h>

#define CUSTOM_RNG_FACTOR	0x41c650ad
#define CUSTOM_RNG_VALUE	0x3039

// clang-format off
uint32_t CUSTOM_RNG_VAL2 = 0x0013cc25;
// clang-format on

uint32_t CUSTOM_RNG_VAL1;

int32_t customRandom(long min, long max)
{
	int32_t tmp;

	if (max == min) {
		return min;
	}

	if (max < min) {
		tmp = min;
		min = max;
		max = tmp;
	}

	CUSTOM_RNG_VAL1 = CUSTOM_RNG_VAL1 * CUSTOM_RNG_FACTOR + CUSTOM_RNG_VALUE;
	CUSTOM_RNG_VAL2 = CUSTOM_RNG_VAL2 * CUSTOM_RNG_FACTOR + CUSTOM_RNG_VALUE;

	return min + (int32_t)(((CUSTOM_RNG_VAL1 >> 16) | (CUSTOM_RNG_VAL2 << 16)) %
	                       (max - min + 1));
}

/* MOD (fps): customRandom for render code. The extra render passes of a tick
 * (dw/mod_fps.h) do not advance the generator, so it advances exactly as with
 * one render per tick. */
extern int32_t MOD_RENDER_PASS;

int32_t modRenderCustomRandom(long min, long max)
{
	uint32_t val1;
	uint32_t val2;
	int32_t result;

	if (MOD_RENDER_PASS == 0) {
		return customRandom(min, max);
	}
	val1 = CUSTOM_RNG_VAL1;
	val2 = CUSTOM_RNG_VAL2;
	CUSTOM_RNG_VAL1 ^= MOD_RENDER_PASS * 0x9E3779B9;
	result = customRandom(min, max);
	CUSTOM_RNG_VAL1 = val1;
	CUSTOM_RNG_VAL2 = val2;
	return result;
}
