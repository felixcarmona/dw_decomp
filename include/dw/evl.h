#ifndef DW_EVL_H
#define DW_EVL_H

#include <dw/entity.h>
#include <dw/ids.h>
#include <dw/params.h>
#include <dw/types.h>

typedef struct {
	uint8_t *modelPtr;
	int32_t unk1;
	int32_t unk2;
	int32_t modelSize;
	uint8_t *imagePtr;
	int32_t imageSize;
} EvoModelData;

typedef struct {
	int32_t timer;
	PartnerEntity *partner;
	int16_t unk_0x8;
	int16_t digimonId;
	EvoModelData modelData;
	int32_t hasFinishedLoading;
	PartnerPara *para;
	int16_t evoTarget;
	int16_t state;
	int32_t heightFactor;
} EvoSequenceData;

typedef struct {
	int16_t hp;
	int16_t mp;
	int16_t offense;
	int16_t defense;
	int16_t speed;
	int16_t brains;
	int16_t targetDigimon;
} EvoStatsGains;

typedef struct {
	DigimonId from[5]; /* MOD: was s8 (ids.md step 3); -1 = none */
	DigimonId to[6];
} EvolutionPath;

typedef struct {
	int16_t digimon;
	int16_t hp;
	int16_t mp;
	int16_t offense;
	int16_t defense;
	int16_t speed;
	int16_t brain;
	int16_t care;
	int16_t weight;
	int16_t discipline;
	int16_t happiness;
	int16_t battles;
	int16_t techs;
	int8_t flags;
	uint8_t padding;
} EvoRequirements;

typedef struct {
	int32_t timer;
	SVECTOR pos;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t pad;
} EvlParticle;

typedef struct {
	int16_t bone;
	int16_t timer;
	Entity *entity;
} EvlSpark;

extern EvlSpark EVL_D_80068F84[];
extern EvoSequenceData EVO_SEQUENCE_DATA;

extern EvoStatsGains EVOLUTION_STATS_GAINS[NUM_ORIGINAL_DIGIMON];
extern EvolutionPath EVOLUTION_PATHS[NUM_EVOLVING_DIGIMON - 1];
extern EvoRequirements EVOLUTION_REQS[NUM_ORIGINAL_DIGIMON];
/* MOD (scale): rows of species >= NUM_ORIGINAL_DIGIMON come from their DIGI.DAT record (partner.h) */
EvoStatsGains *getSpeciesEvoGains(int32_t type);
EvoRequirements *getSpeciesEvoReqs(int32_t type);
static inline EvoStatsGains *evoGainsRow(int32_t type)
{
	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON) {
		return &EVOLUTION_STATS_GAINS[type];
	}
	return getSpeciesEvoGains(type);
}
static inline EvoRequirements *evoReqRow(int32_t type)
{
	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON) {
		return &EVOLUTION_REQS[type];
	}
	return getSpeciesEvoReqs(type);
}
#define EVO_GAINS_ROW(type) (*evoGainsRow(type))
#define EVO_REQ_ROW(type) (*evoReqRow(type))

extern int8_t MAIN_D_801349F4;

void tickEvoSequenceLoading(int32_t instanceId);
void renderEvoSequenceLoading(int32_t instanceId);
int32_t getEvoSequenceState(PartnerEntity *partner, void *buffer,
                            PartnerPara *para, int32_t target,
                            int32_t isInitialized);
int32_t evoSequenceAlwaysTrue(int32_t unused);

void EVL_initEvoSequence(void);

#endif
