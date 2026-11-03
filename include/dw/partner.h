#ifndef DW_PARTNER_H
#define DW_PARTNER_H

#include <dw/ids.h>
#include <dw/types.h>

typedef struct {
	int8_t hungerTimes[8];
	uint8_t energyCap;
	uint8_t energyThreshold;
	uint8_t energyUsage;
	int16_t poopTimer;
	int16_t unk2;
	uint8_t poopSize;
	uint8_t favoriteFood;
	uint8_t sleepCycle;
	uint8_t favoredRegion;
	uint8_t trainingType;
	uint8_t defaultWeight;
	int16_t viewX;
	int16_t viewY;
	int16_t viewZ;
} RaiseData;

typedef struct {
	uint8_t map;
	uint8_t x;
	uint8_t y;
	int8_t size;
} PoopPile;

/*
 * MOD (scale): the per-partner rows of the 180 original species are resident
 * (RAISE_DATA, EVOLUTION_*, ITEM_TAKE_DISTANCE, DIGIMON_VLALL_SOUND_ID, TRN/TRN2
 * animation rows); the row of a species >= NUM_ORIGINAL_DIGIMON is in its
 * DIGI.DAT record (mod_species.c, species.h), read through the record cache.
 * RAISE_ROW(type) is the row as an lvalue, valid until the next species lookup.
 */
extern RaiseData RAISE_DATA[NUM_ORIGINAL_DIGIMON];
RaiseData *getSpeciesRaise(int32_t type);
int32_t getItemTakeDistance(int32_t type);
static inline RaiseData *raiseRow(int32_t type)
{
	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON) {
		return &RAISE_DATA[type];
	}
	return getSpeciesRaise(type);
}
#define RAISE_ROW(type) (*raiseRow(type))
extern PoopPile WORLD_POOP[];

void partnerTick(int32_t instanceId);
void updateConditionAnimation(void);
void partnerSetState(int32_t state);
void setSomeDyingState(void);
int32_t partnerGetState(void);
void partnerStartAnimation(int32_t animId);
void callDigimonRoutine(int32_t routine);
int32_t getScriptSyncBit(void);

#endif
