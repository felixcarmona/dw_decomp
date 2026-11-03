/*
 * MOD (species): Digimon species beyond the disc's 180 (include/dw/species.h,
 * docs/mods/ids.md "Species beyond 180").
 */
#include <string.h>
#include <dw/file.h>
#include <dw/main.h>
#include <dw/sound.h>
#include <dw/species.h>

#include "common.h"

extern int16_t EVOLUTION_TARGET;
extern ModelComponent NPC_MODELS[5];
extern int8_t ITEM_TAKE_DISTANCE[NUM_DIGIMON];
extern SkeletonBone *DIGIMON_SKELETONS[NUM_ORIGINAL_DIGIMON];
extern char *PTR_DIGIMON_FILE_NAMES[NUM_ORIGINAL_DIGIMON];
extern uint8_t DIGIMON_VBALL_SOUND_ID[NUM_ORIGINAL_DIGIMON];
void tickFileReadQueue(int32_t unused);
int32_t isFileReadQueueBusy(void);

int32_t SPECIES_COUNT;
int32_t SPECIES_RECORD_READS;
static int32_t SPECIES_RECORDS_SECTOR;
static SpeciesRecord SPECIES_CACHE[SPECIES_CACHE_SLOTS];
static int16_t SPECIES_CACHE_ID[SPECIES_CACHE_SLOTS];
static uint16_t SPECIES_CACHE_AGE[SPECIES_CACHE_SLOTS];
static uint16_t SPECIES_CACHE_CLOCK;
static uint32_t SPECIES_SECTOR[0x800 / 4];
/* the evolution paths of the new species (EVOLUTION_PATHS ends at type 62) */
static EvolutionPath SPECIES_EVOLUTION_PATHS[NUM_DIGIMON - NUM_ORIGINAL_DIGIMON];
static EvolutionPath NO_EVOLUTION_PATH;

EvolutionPath *getEvolutionPath(int32_t type)
{
	if (type >= 1 && type < NUM_EVOLVING_DIGIMON) {
		return &EVOLUTION_PATHS[type - 1];
	}
	if (type >= NUM_ORIGINAL_DIGIMON && type < SPECIES_COUNT) {
		return &SPECIES_EVOLUTION_PATHS[type - NUM_ORIGINAL_DIGIMON];
	}
	return &NO_EVOLUTION_PATH;
}

static void readSpeciesSector(int32_t sector)
{
	/* never in the middle of an asynchronous read (evolution, sounds, overlays) */
	while (isFileReadQueueBusy()) {
		tickFileReadQueue(0);
	}
	readFileSectors(SPECIES_FILE_NAME, SPECIES_SECTOR, sector, 1);
}

/* 1 while a model of the type is in use: its record must stay in the cache */
static int32_t isSpeciesInUse(int32_t type)
{
	int32_t i;

	if (PARTNER_ENTITY.digimonEntity.entity.type == type || EVOLUTION_TARGET == type ||
	    EVO_SEQUENCE_DATA.digimonId == type) {
		return 1;
	}
	for (i = 0; i < 5; i++) {
		if (NPC_MODELS[i].useCount != 0 && NPC_MODELS[i].digiType == type) {
			return 1;
		}
	}
	return 0;
}

static void applyPartnerRows(int32_t type, SpeciesRecord *rec)
{
	int32_t i;
	int32_t j;
	int32_t other;
	EvolutionPath *path;

	RAISE_DATA[type] = rec->raise;
	EVOLUTION_REQS[type] = rec->evoReqs;
	EVOLUTION_STATS_GAINS[type] = rec->evoGains;
	ITEM_TAKE_DISTANCE[type - 1] = rec->itemTakeDistance;
	DIGIMON_VLALL_SOUND_ID[type] = rec->vlallId;
	*getEvolutionPath(type) = rec->evoPaths;
	/* each species it evolves from lists it among its targets, and the other way round */
	for (i = 0; i < 5; i++) {
		other = rec->evoPaths.from[i];
		path = getEvolutionPath(other);
		if (path == &NO_EVOLUTION_PATH) {
			continue;
		}
		for (j = 0; j < 6 && path->to[j] != type; j++) {
		}
		if (j == 6) {
			for (j = 0; j < 6 && path->to[j] != -1; j++) {
			}
			if (j < 6) {
				path->to[j] = type;
			}
		}
	}
	for (i = 0; i < 6; i++) {
		other = rec->evoPaths.to[i];
		path = getEvolutionPath(other);
		if (path == &NO_EVOLUTION_PATH) {
			continue;
		}
		for (j = 0; j < 5 && path->from[j] != type; j++) {
		}
		if (j == 5) {
			for (j = 0; j < 5 && path->from[j] != -1; j++) {
			}
			if (j < 5) {
				path->from[j] = type;
			}
		}
	}
}

void initializeSpecies(void)
{
	SpeciesHeader *header;
	SpeciesRecord *rec;
	FileLookup lookup;
	int32_t count;
	int32_t type;
	int32_t i;

	SPECIES_COUNT = NUM_ORIGINAL_DIGIMON;
	SPECIES_RECORD_READS = 0;
	for (i = 0; i < 5; i++) {
		NO_EVOLUTION_PATH.from[i] = -1;
	}
	for (i = 0; i < 6; i++) {
		NO_EVOLUTION_PATH.to[i] = -1;
	}
	for (i = 0; i < NUM_DIGIMON - NUM_ORIGINAL_DIGIMON; i++) {
		SPECIES_EVOLUTION_PATHS[i] = NO_EVOLUTION_PATH;
	}
	/* the partner voice of types that were outside DIGIMON_VLALL_SOUND_ID: the
	 * 15 the guards gave them, unless a row was given (0 is the tamer's) */
	for (i = NUM_PARTNER_DIGIMON; i < NUM_DIGIMON; i++) {
		if (DIGIMON_VLALL_SOUND_ID[i] == 0) {
			DIGIMON_VLALL_SOUND_ID[i] = 15;
		}
	}
	SPECIES_CACHE_CLOCK = 0;
	for (i = 0; i < SPECIES_CACHE_SLOTS; i++) {
		SPECIES_CACHE_ID[i] = -1;
		SPECIES_CACHE_AGE[i] = 0;
	}
	if (lookupFileTable(&lookup, SPECIES_FILE_NAME) == 0 || lookup.size < 0x800) {
		return;
	}
	readSpeciesSector(0);
	header = (SpeciesHeader *)SPECIES_SECTOR;
	if (header->magic[0] != 'D' || header->magic[1] != 'I' || header->magic[2] != 'G' ||
	    header->magic[3] != 'I' || header->version != SPECIES_VERSION ||
	    header->recordSize != SPECIES_RECORD_SIZE || header->firstRecord != 0) {
		return;
	}
	count = header->count;
	if (count > NUM_DIGIMON) {
		count = NUM_DIGIMON;
	}
	if (count <= NUM_ORIGINAL_DIGIMON) {
		return;
	}
	SPECIES_RECORDS_SECTOR = header->recordsSector;
	SPECIES_COUNT = count;
	/* the partner rows of new species go into the resident per-partner tables */
	for (type = NUM_ORIGINAL_DIGIMON; type < count; type++) {
		if (type == NUM_ORIGINAL_DIGIMON || type % SPECIES_RECORDS_PER_SECTOR == 0) {
			readSpeciesSector(SPECIES_RECORDS_SECTOR + type / SPECIES_RECORDS_PER_SECTOR);
		}
		rec = (SpeciesRecord *)SPECIES_SECTOR + type % SPECIES_RECORDS_PER_SECTOR;
		if ((rec->flags & (SPECIES_FLAG_DEFINED | SPECIES_FLAG_PARTNER)) ==
		    (SPECIES_FLAG_DEFINED | SPECIES_FLAG_PARTNER)) {
			applyPartnerRows(type, rec);
		}
	}
}

SpeciesRecord *getSpeciesRecord(int32_t type)
{
	int32_t i;
	int32_t slot;
	int32_t oldest;
	int32_t age;

	if (type < NUM_ORIGINAL_DIGIMON || type >= SPECIES_COUNT) {
		return NULL;
	}
	SPECIES_CACHE_CLOCK++;
	for (i = 0; i < SPECIES_CACHE_SLOTS; i++) {
		if (SPECIES_CACHE_ID[i] == type) {
			SPECIES_CACHE_AGE[i] = SPECIES_CACHE_CLOCK;
			return (SPECIES_CACHE[i].flags & SPECIES_FLAG_DEFINED) ? &SPECIES_CACHE[i] : NULL;
		}
	}
	/* a free slot, else the least recently used one whose species is not in use */
	slot = -1;
	oldest = -1;
	for (i = 0; i < SPECIES_CACHE_SLOTS; i++) {
		if (SPECIES_CACHE_ID[i] == -1) {
			slot = i;
			break;
		}
		age = (uint16_t)(SPECIES_CACHE_CLOCK - SPECIES_CACHE_AGE[i]);
		if (age > oldest && !isSpeciesInUse(SPECIES_CACHE_ID[i])) {
			oldest = age;
			slot = i;
		}
	}
	if (slot == -1) {
		slot = 0;
		for (i = 1; i < SPECIES_CACHE_SLOTS; i++) {
			if ((uint16_t)(SPECIES_CACHE_CLOCK - SPECIES_CACHE_AGE[i]) >
			    (uint16_t)(SPECIES_CACHE_CLOCK - SPECIES_CACHE_AGE[slot])) {
				slot = i;
			}
		}
	}
	readSpeciesSector(SPECIES_RECORDS_SECTOR + type / SPECIES_RECORDS_PER_SECTOR);
	memcpy(&SPECIES_CACHE[slot], (SpeciesRecord *)SPECIES_SECTOR + type % SPECIES_RECORDS_PER_SECTOR,
	       SPECIES_RECORD_SIZE);
	SPECIES_RECORD_READS++;
	SPECIES_CACHE_ID[slot] = type;
	SPECIES_CACHE_AGE[slot] = SPECIES_CACHE_CLOCK;
	return (SPECIES_CACHE[slot].flags & SPECIES_FLAG_DEFINED) ? &SPECIES_CACHE[slot] : NULL;
}

DigimonPara *getSpeciesPara(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	if (rec == NULL) {
		/* not a species: what the original read past the table */
		return &DIGI_PARA(type);
	}
	return &rec->para;
}

int32_t isValidDigimonType(int32_t type)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON) {
		return 1;
	}
	rec = getSpeciesRecord(type);
	return rec != NULL && (rec->modelName[0] != '\0' || rec->modelSize != 0);
}

SkeletonBone *getDigimonSkeleton(int32_t type)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON || (rec = getSpeciesRecord(type)) == NULL) {
		return DIGIMON_SKELETONS[type];
	}
	return rec->bones;
}

int32_t getDigimonVballId(int32_t type)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON || (rec = getSpeciesRecord(type)) == NULL) {
		return DIGIMON_VBALL_SOUND_ID[type];
	}
	return rec->vballId;
}

int32_t getDigimonModelRef(int32_t type, char **file, int32_t *sector, int32_t *size)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON) {
		*file = NULL;
		*sector = 0;
		*size = 0;
		return 1;
	}
	rec = getSpeciesRecord(type);
	if (rec == NULL) {
		return 0;
	}
	if (rec->modelName[0] != '\0') {
		/* an original model under another id: the path is built from modelName */
		*file = NULL;
		*sector = 0;
		*size = 0;
		return 1;
	}
	*file = SPECIES_FILE_NAME;
	*sector = rec->modelSector;
	*size = rec->modelSize;
	return rec->modelSize != 0;
}

/* the 4-letter model name of a type (CHDAT\MMDn\NAME.MMD) */
char *getDigimonModelName(int32_t type)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON || (rec = getSpeciesRecord(type)) == NULL) {
		return PTR_DIGIMON_FILE_NAMES[type];
	}
	return rec->modelName;
}

int32_t getDigimonTextureRef(int32_t type, char **file, int32_t *sector, int32_t *sectors)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON || (rec = getSpeciesRecord(type)) == NULL) {
		/* ALLTIM.TIM: entry `type`, 9 sectors each */
		*file = "ALLTIM.TIM";
		*sector = type * 9;
		*sectors = 9;
		return 1;
	}
	*file = rec->texFile == SPECIES_TEX_ARCHIVE ? SPECIES_FILE_NAME : "ALLTIM.TIM";
	*sector = rec->texSector;
	*sectors = rec->texSectors;
	return 1;
}

int8_t *getTrainingAnimRow(int8_t (*table)[2], int32_t type, int32_t which)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON || (rec = getSpeciesRecord(type)) == NULL) {
		return table[type];
	}
	return which == 0 ? rec->trainingAnims : rec->training2Anims;
}
