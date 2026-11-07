#ifndef DW_PARTNER_H
#define DW_PARTNER_H

#include <dw/ids.h>
#include <dw/types.h>

/* A resource is a care need strictly below 30%; zero maxima have no need. */
static inline int32_t modResourceNeed(int32_t current, int32_t maximum)
{
	return maximum > 0 && current * 10 < maximum * 3;
}

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

/* MOD (screens beyond 255): the screen is a ScreenId, -1 = none (the save keeps SavedPoopPile, main_menu.c) */
typedef struct {
	ScreenId map;
	uint8_t x;
	uint8_t y;
	int8_t size;
	uint8_t pad;
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
/* MOD (optional name): an unnamed partner is called by its species, partner_impl.c */
int32_t modPartnerNameIsSpecies(int32_t type);
void modPartnerNameSetSpecies(int32_t type);
void modPartnerNameFollow(int32_t from, int32_t to);

/* MOD (species JSON): a Fresh species' hatching stats (partner_impl.c; the DOOA overlay's new eggs read them too) */
typedef struct {
	int16_t type;
	int16_t hp, mp, offense, defense, speed, brains;
} ReincarnationStats;
extern ReincarnationStats REINCARNATION_STATS[4];
void setReincarnateStats(int16_t hp, int16_t mp, int16_t offense, int16_t defense, int16_t speed, int16_t brains);

#endif
