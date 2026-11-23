/*
 * MOD (species): Digimon species beyond the disc's 180 (include/dw/species.h,
 * docs/mods/ids.md "Species beyond 180").
 */
#include <string.h>
#include <dw/file.h>
#include <dw/main.h>
#include <dw/memmap.h>
#include <dw/script.h>
#include <dw/sound.h>
#include <dw/species.h>

#include "common.h"

extern int16_t EVOLUTION_TARGET;
extern ModelComponent NPC_MODELS[5];
extern int8_t ITEM_TAKE_DISTANCE[NUM_ORIGINAL_DIGIMON];
extern SkeletonBone *DIGIMON_SKELETONS[NUM_ORIGINAL_DIGIMON];
extern char *PTR_DIGIMON_FILE_NAMES[NUM_ORIGINAL_DIGIMON];
extern uint8_t DIGIMON_VBALL_SOUND_ID[NUM_ORIGINAL_DIGIMON];
extern int16_t ORIGINAL_DIGIMON_TYPES[NUM_ORIGINAL_DIGIMON];
extern uint8_t SPECIES_BATTLE_MUSIC[NUM_ORIGINAL_DIGIMON];
void tickFileReadQueue(int32_t unused);
int32_t isFileReadQueueBusy(void);

int32_t SPECIES_COUNT;
int32_t SPECIES_RECORD_READS;
uint8_t SPECIES_RAISED_BITS[NUM_DIGIMON / 8];
uint8_t SPECIES_RECRUITED_BITS[NUM_DIGIMON / 8];
static int32_t SPECIES_RECORDS_SECTOR;
/* MOD (record originals): the originals carried by their DIGI.DAT record (species.h) */
static uint8_t SPECIES_RECORD_ORIGINALS[SPECIES_RECORD_ORIGINALS_BYTES];
static int32_t SPECIES_INDEX_SECTOR;
static int32_t SPECIES_PARTNER_COUNT;
static uint16_t SPECIES_PARTNER_IDS[SPECIES_MAX_PARTNERS]; /* the partner list of DIGI.DAT, for the lab menu */
static int32_t SPECIES_FIRST_MEGA;
static SpeciesRecord SPECIES_CACHE[SPECIES_CACHE_SLOTS];
static int16_t SPECIES_CACHE_ID[SPECIES_CACHE_SLOTS];
static uint16_t SPECIES_CACHE_AGE[SPECIES_CACHE_SLOTS];
static uint16_t SPECIES_CACHE_CLOCK;
static int32_t SPECIES_LAST_SLOT;
static uint32_t SPECIES_SECTOR[0x800 / 4];
static SVECTOR CONDITION_FX_COPY __attribute__((aligned(4)));
static uint32_t SPECIES_FALLBACK_PORTRAIT;
static uint8_t SPECIES_PORTRAIT_BUFFER[SPECIES_PORTRAIT_SIZE] __attribute__((aligned(4)));
static EvolutionPath NO_EVOLUTION_PATH;
/* what the per-partner rows read for a type that is no partner (zeroes, like the tables past the originals' rows) */
/* what an id that is no species reads (the original read the bytes past DIGIMON_DATA) */
static DigimonPara NO_PARA = { "?" };
static RaiseData NO_RAISE;
static EvoRequirements NO_EVO_REQS;
static EvoStatsGains NO_EVO_GAINS;

int32_t isRecordOriginal(int32_t type)
{
	if ((uint32_t)type >= NUM_ORIGINAL_DIGIMON) {
		return 0;
	}
	return (SPECIES_RECORD_ORIGINALS[type >> 3] >> (type & 7)) & 1;
}

EvolutionPath *getEvolutionPath(int32_t type)
{
	SpeciesRecord *rec;

	if (type >= 1 && type < NUM_EVOLVING_DIGIMON) {
		return &EVOLUTION_PATHS[type - 1]; /* a record original among them keeps its row: the links fill it */
	}
	if ((rec = getSpeciesRecord(type)) != NULL &&
	    (rec->flags & SPECIES_FLAG_PARTNER)) {
		/* in the cache slot: kept while the species is the partner (isSpeciesInUse) */
		return &rec->evoPaths;
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

void resetSpeciesFlags(void)
{
	int32_t i;

	for (i = 0; i < NUM_DIGIMON / 8; i++) {
		SPECIES_RAISED_BITS[i] = 0;
		SPECIES_RECRUITED_BITS[i] = 0;
	}
}

int32_t isDigimonRecruited(int32_t type)
{
	if ((uint32_t)type < NUM_RECRUITABLE_DIGIMON) {
		return isTriggerSet((uint16_t)(TRIGGER_RECRUITED_BASE + type)) != 0;
	}
	if ((uint32_t)type >= NUM_DIGIMON) {
		return 0;
	}
	return (SPECIES_RECRUITED_BITS[type >> 3] >> (type & 7)) & 1;
}

void setDigimonRecruited(int32_t type)
{
	if ((uint32_t)type < NUM_RECRUITABLE_DIGIMON) {
		setTrigger((uint16_t)(TRIGGER_RECRUITED_BASE + type));
	} else if ((uint32_t)type < NUM_DIGIMON) {
		SPECIES_RECRUITED_BITS[type >> 3] |= 1 << (type & 7);
	}
}

/* the partner rows of a new species: its record's partner block (a non-partner has zero rows) */
RaiseData *getSpeciesRaise(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	return (rec != NULL && (rec->flags & SPECIES_FLAG_PARTNER)) ? &rec->raise : &NO_RAISE;
}

EvoRequirements *getSpeciesEvoReqs(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	return (rec != NULL && (rec->flags & SPECIES_FLAG_PARTNER)) ? &rec->evoReqs : &NO_EVO_REQS;
}

EvoStatsGains *getSpeciesEvoGains(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	return (rec != NULL && (rec->flags & SPECIES_FLAG_PARTNER)) ? &rec->evoGains : &NO_EVO_GAINS;
}

int32_t getItemTakeDistance(int32_t type)
{
	SpeciesRecord *rec;

	if (type >= 1 && type < NUM_ORIGINAL_DIGIMON) {
		return ITEM_TAKE_DISTANCE[type - 1]; /* the originals' table is by type - 1 */
	}
	rec = getSpeciesRecord(type);
	return (rec != NULL && (rec->flags & SPECIES_FLAG_PARTNER)) ? rec->itemTakeDistance : 10;
}

/* the VLALL voice of a partner type (15 for the tamer, 0 or a non-partner) */
int32_t getPartnerVlallSoundId(int32_t type)
{
	SpeciesRecord *rec;

	if (type <= 0) {
		return 15;
	}
	if (type < NUM_ORIGINAL_DIGIMON) {
		return DIGIMON_VLALL_SOUND_ID[type];
	}
	rec = getSpeciesRecord(type);
	return (rec != NULL && (rec->flags & SPECIES_FLAG_PARTNER)) ? rec->vlallId : 15;
}

static void addEvolutionLink(int32_t orig, int32_t other, int32_t isFrom)
{
	EvolutionPath *path;
	DigimonId *list;
	int32_t n;
	int32_t j;

	if (orig < 1 || orig >= NUM_EVOLVING_DIGIMON) {
		return;
	}
	path = &EVOLUTION_PATHS[orig - 1];
	list = isFrom ? path->from : path->to;
	n = isFrom ? 5 : 6;
	for (j = 0; j < n && list[j] != other; j++) {
	}
	if (j < n) {
		return;
	}
	for (j = 0; j < n && list[j] != -1; j++) {
	}
	if (j < n) {
		list[j] = other;
	}
}

/*
 * MOD (record originals): the record's rows over the resident tables of an original it carries (at boot; the
 * record stays on the disc, read into the cache when its model, skeleton or paths are needed).
 */
static void copyRecordOriginal(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	if (rec == NULL) {
		return;
	}
	DIGIMON_DATA[type] = rec->para;
	DIGIMON_VBALL_SOUND_ID[type] = rec->vballId;
	SPECIES_BATTLE_MUSIC[type] = rec->battleMusic;
	if (type <= 0xb0) {
		ORIGINAL_DIGIMON_TYPES[type] = rec->originalType;
	}
	if (rec->flags & SPECIES_FLAG_PARTNER) {
		RAISE_DATA[type] = rec->raise;
		EVOLUTION_REQS[type] = rec->evoReqs;
		EVOLUTION_STATS_GAINS[type] = rec->evoGains;
		ITEM_TAKE_DISTANCE[type - 1] = rec->itemTakeDistance;
		DIGIMON_VLALL_SOUND_ID[type] = rec->vlallId;
	}
}

void initializeSpecies(void)
{
	SpeciesHeader *header;
	FileLookup lookup;
	int32_t count;
	int32_t i;
	SpeciesPartnerList *partners;
	SpeciesLinks *links;
	uint8_t hasRecordOriginals;

	SPECIES_COUNT = NUM_ORIGINAL_DIGIMON;
	SPECIES_RECORD_READS = 0;
	SPECIES_INDEX_SECTOR = 0;
	SPECIES_FALLBACK_PORTRAIT = 0;
	SPECIES_PARTNER_COUNT = 0;
	SPECIES_FIRST_MEGA = 0;
	for (i = 0; i < SPECIES_RECORD_ORIGINALS_BYTES; i++) {
		SPECIES_RECORD_ORIGINALS[i] = 0;
	}
	resetSpeciesFlags();
	for (i = 0; i < 16; i++) {
		NO_PARA.moves[i] = 0xFF;
	}
	for (i = 0; i < 5; i++) {
		NO_EVOLUTION_PATH.from[i] = -1;
	}
	for (i = 0; i < 6; i++) {
		NO_EVOLUTION_PATH.to[i] = -1;
	}
	SPECIES_CACHE_CLOCK = 0;
	SPECIES_LAST_SLOT = 0;
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
	/* MOD (record originals): a file of 180 ids is read when it carries originals */
	hasRecordOriginals = 0;
	for (i = 0; i < SPECIES_RECORD_ORIGINALS_BYTES; i++) {
		hasRecordOriginals |= ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_RECORD_ORIGINALS + i];
	}
	if (count < NUM_ORIGINAL_DIGIMON || (count == NUM_ORIGINAL_DIGIMON && hasRecordOriginals == 0)) {
		return;
	}
	for (i = 0; i < SPECIES_RECORD_ORIGINALS_BYTES; i++) {
		SPECIES_RECORD_ORIGINALS[i] = ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_RECORD_ORIGINALS + i];
	}
	SPECIES_FALLBACK_PORTRAIT = header->fallbackPortrait;
	SPECIES_RECORDS_SECTOR = header->recordsSector;
	SPECIES_INDEX_SECTOR = header->indexSector;
	SPECIES_COUNT = count;
	/* the partner list (first Mega) and the evolution links with the original species */
	readSpeciesSector(SPECIES_INDEX_SECTOR);
	partners = (SpeciesPartnerList *)SPECIES_SECTOR;
	SPECIES_PARTNER_COUNT = partners->count > SPECIES_MAX_PARTNERS ? SPECIES_MAX_PARTNERS : partners->count;
	SPECIES_FIRST_MEGA = partners->firstMega;
	for (i = 0; i < SPECIES_PARTNER_COUNT; i++) {
		SPECIES_PARTNER_IDS[i] = partners->ids[i];
	}
	readSpeciesSector(SPECIES_INDEX_SECTOR + 1);
	links = (SpeciesLinks *)SPECIES_SECTOR;
	for (i = 0; i < links->count && i < SPECIES_MAX_LINKS; i++) {
		if (links->link[i][1] & SPECIES_LINK_FROM) {
			addEvolutionLink(links->link[i][0], links->link[i][1] & ~SPECIES_LINK_FROM, 1);
		} else {
			addEvolutionLink(links->link[i][0], links->link[i][1], 0);
		}
	}
	for (i = 1; i < NUM_ORIGINAL_DIGIMON; i++) {
		if (isRecordOriginal(i)) {
			copyRecordOriginal(i);
		}
	}
}

int32_t getSpeciesPartnerCount(void)
{
	return SPECIES_PARTNER_COUNT;
}

/* the n-th partner species id of DIGI.DAT (0 <= n < getSpeciesPartnerCount()) */
int32_t getSpeciesPartnerId(int32_t n)
{
	if (n < 0 || n >= SPECIES_PARTNER_COUNT) {
		return -1;
	}
	return SPECIES_PARTNER_IDS[n];
}

int32_t getFirstMegaSpecies(void)
{
	return SPECIES_FIRST_MEGA;
}

SpeciesRecord *getSpeciesRecord(int32_t type)
{
	int32_t i;
	int32_t slot;
	int32_t oldest;
	int32_t age;

	if ((type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) || type >= SPECIES_COUNT) {
		return NULL;
	}
	SPECIES_CACHE_CLOCK++;
	/* the slot of the last hit first: the partner is asked for several times a frame */
	i = SPECIES_LAST_SLOT;
	if (SPECIES_CACHE_ID[i] == type) {
		SPECIES_CACHE_AGE[i] = SPECIES_CACHE_CLOCK;
		return (SPECIES_CACHE[i].flags & SPECIES_FLAG_DEFINED) ? &SPECIES_CACHE[i] : NULL;
	}
	for (i = 0; i < SPECIES_CACHE_SLOTS; i++) {
		if (SPECIES_CACHE_ID[i] == type) {
			SPECIES_CACHE_AGE[i] = SPECIES_CACHE_CLOCK;
			SPECIES_LAST_SLOT = i;
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
	SPECIES_LAST_SLOT = slot;
	return (SPECIES_CACHE[slot].flags & SPECIES_FLAG_DEFINED) ? &SPECIES_CACHE[slot] : NULL;
}

DigimonPara *getSpeciesPara(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	if (rec == NULL) {
		return &NO_PARA;
	}
	return &rec->para;
}

int32_t isValidDigimonType(int32_t type)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) {
		return 1;
	}
	rec = getSpeciesRecord(type);
	return rec != NULL && (rec->modelName[0] != '\0' || rec->modelSize != 0);
}

int32_t isPartnerModelFit(int32_t type)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) {
		return 1; /* the originals and the partner-only files of the mods fit the original size */
	}
	rec = getSpeciesRecord(type);
	return rec != NULL && rec->modelSize <= PARTNER_MMD_MAX;
}

SkeletonBone *getDigimonSkeleton(int32_t type)
{
	SpeciesRecord *rec;

	if (((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) || (rec = getSpeciesRecord(type)) == NULL) {
		return DIGIMON_SKELETONS[type];
	}
	return rec->bones;
}

int32_t getDigimonVballId(int32_t type)
{
	SpeciesRecord *rec;

	if (((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) || (rec = getSpeciesRecord(type)) == NULL) {
		return DIGIMON_VBALL_SOUND_ID[type];
	}
	return rec->vballId;
}

extern uint8_t SPECIES_BATTLE_MUSIC[NUM_ORIGINAL_DIGIMON];

int32_t getSpeciesBattleMusic(int32_t type)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON) {
		return SPECIES_BATTLE_MUSIC[type];
	}
	if ((rec = getSpeciesRecord(type)) == NULL) {
		return 1;
	}
	return rec->battleMusic;
}

int32_t getDigimonModelRef(int32_t type, char **file, int32_t *sector, int32_t *size)
{
	SpeciesRecord *rec;

	if ((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) {
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

	if (((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) || (rec = getSpeciesRecord(type)) == NULL) {
		return PTR_DIGIMON_FILE_NAMES[type];
	}
	return rec->modelName;
}

int32_t getDigimonTextureRef(int32_t type, char **file, int32_t *sector, int32_t *sectors)
{
	SpeciesRecord *rec;

	if (((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) || (rec = getSpeciesRecord(type)) == NULL) {
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

	if (((uint32_t)type < NUM_ORIGINAL_DIGIMON && !isRecordOriginal(type)) || (rec = getSpeciesRecord(type)) == NULL) {
		return table[type];
	}
	return which == 0 ? rec->trainingAnims : rec->training2Anims;
}

/* MOD (dcb-import): effect origins (include/dw/species.h, SpeciesEffectOrigin) */
extern SVECTOR CONDITION_FX_OFFSETS[177];
int32_t getOriginalType(int32_t type);

int16_t *getSpeciesEffectOrigin(int32_t type, int16_t *borrowed)
{
	SpeciesRecord *rec;
	SpeciesEffectOrigin *o;
	int32_t i;

	rec = getSpeciesRecord(type);
	if (rec == NULL) {
		return borrowed;
	}
	for (i = 0; i < rec->numEffectOrigins && i < SPECIES_EFFECT_ORIGINS; i++) {
		o = &rec->effectOrigins[i];
		if (o->from[0] != borrowed[0]) {
			continue;
		}
		if (o->from[1] == SPECIES_ANY_OFFSET ||
		    (o->from[1] == borrowed[1] && o->from[2] == borrowed[2] && o->from[3] == borrowed[3])) {
			/* lives in the record cache slot, which is kept while the model is in use */
			return o->to;
		}
	}
	return borrowed;
}

SVECTOR *getConditionFxOffset(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	if (rec != NULL && (rec->flags & SPECIES_FLAG_COND_FX)) {
		/* the record's x, y, z, bone are 2-aligned: the GTE loads an SVECTOR with LWC2, which needs 4 (the
		 * condition bubble of a hungry WarGrowlmon crashed on it) */
		CONDITION_FX_COPY.vx = rec->conditionFx[0];
		CONDITION_FX_COPY.vy = rec->conditionFx[1];
		CONDITION_FX_COPY.vz = rec->conditionFx[2];
		CONDITION_FX_COPY.pad = rec->conditionFx[3];
		return &CONDITION_FX_COPY;
	}
	return &CONDITION_FX_OFFSETS[getOriginalType(type)];
}

/*
 * MOD (portraits): the icon of a species. The record of the type is read like the cache does (the originals'
 * records 0..179 exist in DIGI.DAT too, so a partner-only original can have an icon), then the sector of the
 * icon; the CLUT and the 32x32 image are copied to `dest`.
 */
int32_t getSpeciesPortrait(int32_t type, void *dest)
{
	SpeciesRecord *rec;
	uint32_t ref;
	int32_t own;

	if (SPECIES_INDEX_SECTOR == 0) {
		return -1;
	}
	ref = 0;
	if (type >= NUM_ORIGINAL_DIGIMON && (rec = getSpeciesRecord(type)) != NULL) {
		ref = rec->portrait;
	} else if (type > 0 && type < NUM_ORIGINAL_DIGIMON) {
		readSpeciesSector(SPECIES_RECORDS_SECTOR + type / SPECIES_RECORDS_PER_SECTOR);
		ref = ((SpeciesRecord *)SPECIES_SECTOR + type % SPECIES_RECORDS_PER_SECTOR)->portrait;
	}
	own = ref != 0;
	if (!own) {
		ref = SPECIES_FALLBACK_PORTRAIT;
	}
	if (ref == 0) {
		memset(dest, 0, SPECIES_PORTRAIT_SIZE);
		return 0;
	}
	readSpeciesSector(ref & 0xFFFFFF);
	memcpy(dest, (uint8_t *)SPECIES_SECTOR + (ref >> 24) * SPECIES_PORTRAIT_SIZE, SPECIES_PORTRAIT_SIZE);
	return own;
}

void loadSpeciesPortrait(int32_t type, int32_t x, int32_t y, int32_t rows, int32_t clutX, int32_t clutY)
{
	RECT rect;

	if (getSpeciesPortrait(type, SPECIES_PORTRAIT_BUFFER) < 0) {
		return;
	}
	setRECT(&rect, clutX, clutY, 16, 1);
	LoadImage(&rect, (u_long *)SPECIES_PORTRAIT_BUFFER);
	setRECT(&rect, x, y, 8, rows);
	LoadImage(&rect, (u_long *)(SPECIES_PORTRAIT_BUFFER + 0x20));
	DrawSync(0);
}

int32_t getSpeciesPartnersByLevel(int32_t lo, int32_t hi, uint16_t *out, int32_t max)
{
	int32_t i;
	int32_t n;
	uint8_t level;

	if (SPECIES_INDEX_SECTOR == 0) {
		return 0;
	}
	readSpeciesSector(0);
	n = 0;
	for (i = 0; i < SPECIES_PARTNER_COUNT && n < max; i++) {
		level = ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_LEVELS + i];
		if (level >= lo && level <= hi) {
			out[n++] = SPECIES_PARTNER_IDS[i];
		}
	}
	return n;
}
