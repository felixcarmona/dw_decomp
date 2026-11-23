#ifndef DW_DOOA_H
#define DW_DOOA_H

#include <libgs.h>
#include <libgte.h>

#include <dw/efe.h>
#include <dw/entity.h>
#include <dw/evl.h>
#include <dw/mod_options.h>
#include <dw/types.h>

typedef struct {
	int16_t targetY;
	int16_t pad;
	VECTOR pos;
} DooaFlash;

typedef struct {
	long frame;
	Entity *entity;
	int16_t phase;
	int16_t phaseInitPending;
	int16_t fadeLevel;
	int16_t eggSlot;
	DooaFlash flash;
	EvoModelData modelData;
	int16_t sparkleIndex;
	int8_t isModelLoading;
	int8_t unk_3F;
} DooaSequence;

typedef struct {
	int16_t state;
	int16_t prevState;
	Entity *entity;
	int32_t unk_08;
	void *shardBuffer;
	void *shardWrite;
	int32_t shardBytes;
	uint8_t colorR;
	uint8_t colorG;
	uint8_t colorB;
	int8_t flash;
} DooaShardEffect;

extern EfeFlashBuffer DOOA_FLASH_DATA;
extern GsRVIEW2 DOOA_CAMERA_START_VIEW;
extern VECTOR DOOA_SAVED_LOCATION;
extern uint8_t DOOA_SAVED_ENTITY_CLUT[];
extern int16_t DOOA_MODEL_CLUT[];
extern int16_t DOOA_SHARD_CLUT[];
extern GsRVIEW2 DOOA_SAVED_VIEW;
extern VECTOR DOOA_PARTNER_POSITION;
extern int16_t DOOA_SCENE_CLUT[];
extern int16_t DOOA_FADED_CLUT[];
extern GsOT DOOA_ORDERING_TABLE[];
extern int8_t DOOA_SAVED_ENTITY_VISIBILITY[];
extern DooaShardEffect DOOA_SHARD_EFFECT;
extern DooaSequence DOOA_REINCARNATION_SEQ;

extern int16_t REINCARNATE_BABY_TYPE[4];

/*
 * MOD (eight eggs): the egg the player picks when the partner dies. Slots 0-3 are the original eggs and babies
 * (REINCARNATE_BABY_TYPE, which the main executable also reads); with NEW DIGIMON on, slots 4-7 hatch the hand-made
 * Fresh babies, species 470-473 in slot order (Pabumon, Pichimon, YukimiBotamon, Nyokimon). Their icons and CLUT rows
 * 497-500 are drawn into the disc's TAMA.TIM at build time (tools/egg_art.py); a new egg borrows the 3D model of an
 * original group of TAMA.TMD (the buffer has no room for more objects) and its row is copied over that group's.
 */
#define EGG_NEW_FIRST_SPECIES	470
#define EGG_NEW_CLUT_ROW	497	/* TAMA.TIM's CLUT is at VRAM 128,488: rows 488-491 the 3D groups */

static inline int32_t eggCount(void)
{
	return modSpeciesAllowed(EGG_NEW_FIRST_SPECIES) ? 8 : 4;
}

static inline int32_t eggBabyType(int32_t slot)
{
	return slot < 4 ? REINCARNATE_BABY_TYPE[slot] : EGG_NEW_FIRST_SPECIES + slot - 4;
}

/* the TAMA.TMD group (6 objects) the slot's egg is drawn with: Pabumon and Pichimon the plain-coloured group 2,
 * YukimiBotamon the wave egg (group 1, whose skin carries the zigzags), Nyokimon the spotted group 0 */
static inline int32_t eggModelGroup(int32_t slot)
{
	return slot < 4 ? slot : (0x0122 >> ((slot - 4) * 4)) & 0xf;
}

extern SVECTOR MAIN_D_80134BB4;
extern int8_t DOOA_ENTITIES_VISIBLE;

int32_t DOOA_getSequenceState(int32_t unused, int32_t isInitialized);
int32_t DOOA_tick(PartnerEntity *partner, void *buffer, int32_t isInitialized);

#endif
