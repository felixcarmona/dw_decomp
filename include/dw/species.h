#ifndef DW_SPECIES_H
#define DW_SPECIES_H

/*
 * MOD (species): Digimon species beyond the disc's 180, described by the
 * disc file DIGI.DAT (docs/mods/ids.md, "Species beyond 180").
 *
 * DIGI.DAT is written by tools/species.py:
 *
 *   sector 0           SpeciesHeader
 *   sector 1..         one 512-byte SpeciesRecord per id, 4 per sector, ids
 *                      0..count-1 (0..179 are generated from the executable's
 *                      tables; the game only reads ids >= NUM_ORIGINAL_DIGIMON)
 *   after the records  each new species' MMD and its 0x4800-byte texture
 *                      (an ALLTIM.TIM entry), sector aligned, found through
 *                      the record (no fixed stride)
 *
 * The 180 original species keep their resident tables (DIGIMON_DATA,
 * DIGIMON_SKELETONS, ...): they are read every frame and never miss. A new
 * species' record is read into a small cache (SPECIES_CACHE_SLOTS records)
 * when its model is loaded, and its slot is not given away while a model of
 * it is in use (partner, NPC slots, evolution target). The per-partner
 * tables (RAISE_DATA, EVOLUTION_*, ...) are resident and sized NUM_DIGIMON;
 * the rows of new partner species are filled from DIGI.DAT at boot.
 */

#include <dw/evl.h>
#include <dw/ids.h>
#include <dw/model.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/types.h>

#define SPECIES_FILE_NAME	"DIGI.DAT"
#define SPECIES_MAGIC		"DIGI"
#define SPECIES_VERSION		1
#define SPECIES_RECORD_SIZE	0x200
#define SPECIES_RECORDS_PER_SECTOR (0x800 / SPECIES_RECORD_SIZE)
#define SPECIES_MAX_BONES	64
#define SPECIES_CACHE_SLOTS	8

/* SpeciesRecord.flags */
#define SPECIES_FLAG_DEFINED	0x01	/* the id exists */
#define SPECIES_FLAG_PARTNER	0x02	/* the partner block is valid: it can be a partner */
#define SPECIES_FLAG_COND_FX	0x04	/* conditionFx replaces CONDITION_FX_OFFSETS[originalType] */

#define SPECIES_EFFECT_ORIGINS	8
#define SPECIES_ANY_OFFSET	(-0x8000)	/* EffectOrigin.from[1]: any offset of that bone */

/* SpeciesRecord.texFile */
#define SPECIES_TEX_ALLTIM	0	/* CHDAT\ALLTIM.TIM */
#define SPECIES_TEX_ARCHIVE	1	/* DIGI.DAT */

typedef struct {
	char magic[4];		/* "DIGI" */
	uint16_t version;	/* SPECIES_VERSION */
	uint16_t recordSize;	/* SPECIES_RECORD_SIZE */
	uint16_t count;		/* ids 0..count-1 */
	uint16_t firstRecord;	/* id of the first record (0) */
	uint32_t recordsSector;	/* sector of the first record */
	uint32_t totalSectors;
} SpeciesHeader;

/*
 * MOD (dcb-import): where this species' technique effects start. The EFE data
 * of each technique lists, per original type, a bone and an offset in that
 * bone's space (EfeTechBoneOffset). A new species borrows the entries of its
 * originalType, whose bone numbers belong to another skeleton: an entry here
 * whose `from` equals the borrowed {bone, x, y, z} (or {bone, ANY_OFFSET, ..})
 * replaces it with `to`, one of this species' bones and a point in it. Bone 0
 * (the entity) offsets are skeleton-independent and are normally left alone.
 */
typedef struct {
	int16_t from[4];		/* bone, x, y, z of the originalType's entry */
	int16_t to[4];			/* bone, x, y, z for this species (an EfeBoneOffset) */
} SpeciesEffectOrigin;

typedef struct {
	DigimonPara para;		/* 0x000 name, bones, radius, height, level, specials, drop, techniques */
	char modelName[8];		/* 0x034 "AGUM" = CHDAT\MMDn\AGUM.MMD; empty: the model is in DIGI.DAT */
	uint32_t modelSector;		/* 0x03C in DIGI.DAT (modelName empty) */
	uint32_t modelSize;		/* 0x040 bytes */
	uint32_t texSector;		/* 0x044 sector of the texture in texFile */
	uint16_t texFile;		/* 0x048 SPECIES_TEX_* */
	uint16_t texSectors;		/* 0x04A 9 (0x4800 bytes) */
	int16_t originalType;		/* 0x04C getOriginalType: technique effect offsets, aura, condition icons */
	uint8_t vballId;		/* 0x04E VBALL voice */
	uint8_t vlallId;		/* 0x04F VLALL voice (partners) */
	uint16_t flags;			/* 0x050 SPECIES_FLAG_* */
	uint16_t reserved52;
	SkeletonBone bones[SPECIES_MAX_BONES]; /* 0x054 para.boneCount used */
	/* partner block (SPECIES_FLAG_PARTNER) */
	RaiseData raise;		/* 0x0D4 */
	EvoRequirements evoReqs;	/* 0x0F0 requirements to evolve into this species */
	EvoStatsGains evoGains;		/* 0x10C */
	EvolutionPath evoPaths;		/* 0x11A from[5] / to[6]; each "from" also gets this id in its "to" */
	int8_t itemTakeDistance;	/* 0x130 */
	int8_t trainingAnims[2];	/* 0x131 TRN_TRAINING_ANIM_IDS row */
	int8_t training2Anims[2];	/* 0x133 TRN2_TRAINING_ANIM_IDS row */
	uint8_t reserved135;
	/* MOD (dcb-import): effect origins of a species with its own skeleton */
	int16_t conditionFx[4];		/* 0x136 x, y, z, bone (SPECIES_FLAG_COND_FX) */
	uint8_t numEffectOrigins;	/* 0x13E */
	uint8_t reserved13F;
	SpeciesEffectOrigin effectOrigins[SPECIES_EFFECT_ORIGINS]; /* 0x140 */
	uint8_t reserved1C0[SPECIES_RECORD_SIZE - 0x1C0];
} SpeciesRecord;

#define SPECIES_LAYOUT_CHECK(name, cond) typedef char species_layout_##name[(cond) ? 1 : -1]
SPECIES_LAYOUT_CHECK(size, sizeof(SpeciesRecord) == SPECIES_RECORD_SIZE);
SPECIES_LAYOUT_CHECK(para, sizeof(DigimonPara) == 0x34);
SPECIES_LAYOUT_CHECK(bones, __builtin_offsetof(SpeciesRecord, bones) == 0x54);
SPECIES_LAYOUT_CHECK(raise, __builtin_offsetof(SpeciesRecord, raise) == 0xD4);
SPECIES_LAYOUT_CHECK(evoReqs, __builtin_offsetof(SpeciesRecord, evoReqs) == 0xF0);
SPECIES_LAYOUT_CHECK(evoGains, __builtin_offsetof(SpeciesRecord, evoGains) == 0x10C);
SPECIES_LAYOUT_CHECK(evoPaths, __builtin_offsetof(SpeciesRecord, evoPaths) == 0x11A);
SPECIES_LAYOUT_CHECK(item, __builtin_offsetof(SpeciesRecord, itemTakeDistance) == 0x130);
SPECIES_LAYOUT_CHECK(condfx, __builtin_offsetof(SpeciesRecord, conditionFx) == 0x136);
SPECIES_LAYOUT_CHECK(origins, __builtin_offsetof(SpeciesRecord, effectOrigins) == 0x140);
SPECIES_LAYOUT_CHECK(header, sizeof(SpeciesHeader) == 0x14);

/* ids 0..SPECIES_COUNT-1 exist (NUM_ORIGINAL_DIGIMON without DIGI.DAT) */
extern int32_t SPECIES_COUNT;
/* records read from the disc (cache misses), for tests */
extern int32_t SPECIES_RECORD_READS;

void initializeSpecies(void);
/* the record of a new species (id >= NUM_ORIGINAL_DIGIMON), NULL if it does not exist */
SpeciesRecord *getSpeciesRecord(int32_t type);
/* 1 if a model can be loaded for the type */
int32_t isValidDigimonType(int32_t type);

SkeletonBone *getDigimonSkeleton(int32_t type);
/* the 4-letter model name (CHDAT\MMDn\NAME.MMD); empty for a model inside DIGI.DAT */
char *getDigimonModelName(int32_t type);
int32_t getDigimonVballId(int32_t type);
/* where the model and the texture of a type are: file name (or NULL for the
 * CHDAT\MMDn\NAME.MMD path), first sector and size */
int32_t getDigimonModelRef(int32_t type, char **file, int32_t *sector, int32_t *size);
int32_t getDigimonTextureRef(int32_t type, char **file, int32_t *sector, int32_t *sectors);
/* the evolution paths of a type (EVOLUTION_PATHS covers types 1..62, new
 * species have their own rows; any other type has none) */
EvolutionPath *getEvolutionPath(int32_t type);
/* MOD (dcb-import): the bone offset a technique effect of `type` starts
 * from: `borrowed` (the originalType's EFE entry) or this species' own one */
int16_t *getSpeciesEffectOrigin(int32_t type, int16_t *borrowed);
/* the condition icon offset (x, y, z, bone) of a type */
SVECTOR *getConditionFxOffset(int32_t type);
/* a training animation row (TRN/TRN2 tables live in the overlays) */
int8_t *getTrainingAnimRow(int8_t (*table)[2], int32_t type, int32_t which);

#endif
