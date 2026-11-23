/*
 * MOD (species): Digimon species beyond the disc's 180 (include/dw/species.h,
 * docs/mods/ids.md "Species beyond 180").
 */
#include <string.h>
#include <dw/entity.h>
#include <dw/file.h>
#include <dw/main.h>
#include <dw/memmap.h>
#include <dw/mod_rules.h>
#include <dw/script.h>
#include <dw/sound.h>
#include <dw/species.h>
#include <dw/mod_options.h>
#include <dw/chinese.h>

#include "common.h"

extern int8_t GAME_STATE;
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
/* Written from content/tables/city-residents.json by the balance build. */
const uint8_t CITY_RESIDENTS[14] = {0};

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
static uint32_t SPECIES_CITY_SECTOR; /* optional header extension at 0x38 */
static uint32_t SPECIES_CHART_SECTOR; /* MOD (chart) */
static uint8_t SPECIES_PORTRAIT_BUFFER[SPECIES_PORTRAIT_SIZE] __attribute__((aligned(4)));
static EvolutionPath NO_EVOLUTION_PATH;
/* what the per-partner rows read for a type that is no partner (zeroes, like the tables past the originals' rows) */
/* what an id that is no species reads (the original read the bytes past DIGIMON_DATA) */
static DigimonPara NO_PARA = { "?" };
static RaiseData NO_RAISE;
static EvoRequirements NO_EVO_REQS;
static EvoStatsGains NO_EVO_GAINS;

static uint8_t SPECIES_VANILLA[SPECIES_VANILLA_BYTES];
static int8_t SPECIES_HAS_VANILLA;
/* MOD (canon levels): {id, level with NEW DIGIMON on, off} (SPECIES_HEADER_LEVEL_OFF) */
static uint8_t SPECIES_LEVEL_SWAP[SPECIES_LEVEL_SWAP_MAX][3];
static int32_t SPECIES_LEVEL_SWAPS;
static void readSpeciesSector(int32_t sector);

void modApplySpeciesLevels(void)
{
	int32_t i;
	int32_t j;
	uint16_t *row;
	int16_t *target;

	for (i = 0; i < SPECIES_LEVEL_SWAPS; i++) {
		DIGIMON_DATA[SPECIES_LEVEL_SWAP[i][0]].level = SPECIES_LEVEL_SWAP[i][MOD_OPT_NEW_DIGIMON ? 1 : 2];
	}
	/* Read targets on demand: the shared sector buffer costs no additional heap. */
	if (!SPECIES_INDEX_SECTOR) return;
	readSpeciesSector(0);
	row = (uint16_t *)((uint8_t *)SPECIES_SECTOR + SPECIES_HEADER_STATS_OFF);
	j = *row++;
	if (j > SPECIES_STATS_SWAP_MAX) j = SPECIES_STATS_SWAP_MAX;
	for (i = 0; i < j; i++, row += 13) {
		int32_t k;
		if (row[0] >= NUM_ORIGINAL_DIGIMON) continue;
		target = &EVOLUTION_STATS_GAINS[row[0]].hp;
		for (k = 0; k < 6; k++) target[k] = row[1 + (MOD_OPT_NEW_DIGIMON ? 0 : 6) + k];
	}
	/* Explicit on/off path and requirement rows live in a separate sector. */
	i = ((uint32_t *)SPECIES_SECTOR)[SPECIES_HEADER_EVOLUTION_OFF / 4];
	if (!i) return;
	readSpeciesSector(i);
	row = (uint16_t *)SPECIES_SECTOR;
	j = *row++;
	if (j > SPECIES_EVOLUTION_SWAP_MAX) j = SPECIES_EVOLUTION_SWAP_MAX;
	for (i = 0; i < j; i++, row += 51) {
		int32_t k;
		int16_t *path;
		if (row[0] < 1 || row[0] > 62) continue;
		path = &EVOLUTION_PATHS[row[0] - 1].from[0];
		for (k = 0; k < 11; k++) path[k] = row[1 + (MOD_OPT_NEW_DIGIMON ? 0 : 11) + k];
		target = &EVOLUTION_REQS[row[0]].digimon;
		for (k = 0; k < 14; k++) target[k] = row[23 + (MOD_OPT_NEW_DIGIMON ? 0 : 14) + k];
	}
}

int32_t modOriginalLevel(int32_t type)
{
	int32_t i;

	for (i = 0; i < SPECIES_LEVEL_SWAPS; i++) {
		if (SPECIES_LEVEL_SWAP[i][0] == type) {
			return SPECIES_LEVEL_SWAP[i][2];
		}
	}
	return DIGI_PARA(type).level;
}

int32_t isVanillaSpecies(int32_t type)
{
	if (!SPECIES_HAS_VANILLA) {
		return 1;
	}
	return (uint32_t)type < NUM_DIGIMON && ((SPECIES_VANILLA[type >> 3] >> (type & 7)) & 1);
}

int32_t getSpeciesVanillaAs(int32_t type)
{
	SpeciesRecord *rec;

	rec = getSpeciesRecord(type);
	return (rec != NULL && (rec->flags & SPECIES_FLAG_PARTNER)) ? rec->vanillaAs : 0;
}

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

static int16_t EXTRA_PARENT[SPECIES_EXTRA_PARENTS];		/* 0: unused */
static DigimonId EXTRA_TO[SPECIES_EXTRA_PARENTS][SPECIES_EXTRA_TARGETS];	/* 0: empty */

int32_t getEvolutionExtraTargets(int32_t type, DigimonId **targets)
{
	int32_t k;
	int32_t n;

	for (k = 0; k < SPECIES_EXTRA_PARENTS; k++) {
		if (EXTRA_PARENT[k] == type && type > 0) {
			for (n = 0; n < SPECIES_EXTRA_TARGETS && EXTRA_TO[k][n] > 0; n++) {
			}
			*targets = EXTRA_TO[k];
			return n;
		}
	}
	return 0;
}

/* MOD (chart lists): the Baby II of extra list k (0: none) and its targets */
int32_t getEvolutionExtraParent(int32_t k, DigimonId **targets)
{
	*targets = EXTRA_TO[k];
	return EXTRA_PARENT[k];
}

/* a Baby II's Rookie target with every "to" slot taken: into its extra list */
static void addExtraTarget(int32_t orig, int32_t other)
{
	int32_t k;
	int32_t j;

	if (DIGIMON_DATA[orig].level != 2) {
		return;
	}
	for (k = 0; k < SPECIES_EXTRA_PARENTS && EXTRA_PARENT[k] != orig && EXTRA_PARENT[k] != 0; k++) {
	}
	if (k == SPECIES_EXTRA_PARENTS) {
		return;
	}
	EXTRA_PARENT[k] = orig;
	for (j = 0; j < SPECIES_EXTRA_TARGETS && EXTRA_TO[k][j] != other && EXTRA_TO[k][j] > 0; j++) {
	}
	if (j < SPECIES_EXTRA_TARGETS) {
		EXTRA_TO[k][j] = other;
	}
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
	} else if (!isFrom) {
		addExtraTarget(orig, other); /* MOD (evolution) */
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
	SPECIES_CHART_SECTOR = 0;
	SPECIES_CITY_SECTOR = 0;
	SPECIES_PARTNER_COUNT = 0;
	SPECIES_FIRST_MEGA = 0;
	SPECIES_LEVEL_SWAPS = 0;
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
	SPECIES_HAS_VANILLA = 0;
	for (i = 0; i < SPECIES_VANILLA_BYTES; i++) {
		SPECIES_VANILLA[i] = ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_VANILLA + i];
		SPECIES_HAS_VANILLA |= SPECIES_VANILLA[i] != 0;
	}
	/* MOD (canon levels): the originals' levels with NEW DIGIMON on and off */
	for (i = 0; i < ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_LEVEL_OFF] && i < SPECIES_LEVEL_SWAP_MAX; i++) {
		SPECIES_LEVEL_SWAP[i][0] = ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_LEVEL_OFF + 1 + i * 3];
		SPECIES_LEVEL_SWAP[i][1] = ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_LEVEL_OFF + 2 + i * 3];
		SPECIES_LEVEL_SWAP[i][2] = ((uint8_t *)SPECIES_SECTOR)[SPECIES_HEADER_LEVEL_OFF + 3 + i * 3];
	}
	SPECIES_LEVEL_SWAPS = i;
	SPECIES_FALLBACK_PORTRAIT = header->fallbackPortrait;
	SPECIES_CHART_SECTOR = header->chartSector;
	SPECIES_CITY_SECTOR = ((uint32_t *)header)[14];
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
	modApplySpeciesLevels();
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
	return MOD_LANG == MOD_LANG_ZH ? zhSpeciesPara(&rec->para, type) : &rec->para;
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

uint8_t *readSpeciesChart(int32_t k, int32_t city)
{
	uint32_t sector;

	sector = city ? SPECIES_CITY_SECTOR : SPECIES_CHART_SECTOR;
	if (SPECIES_INDEX_SECTOR == 0 || sector == 0) {
		return NULL;
	}
	readSpeciesSector(sector + k);
	return (uint8_t *)SPECIES_SECTOR;
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

/*
 * MOD (cold animations): see COLD_ANIM_FLAG in species.h. Returns where the animation's first word is, or NULL when it cannot
 * be played (not the partner, a model not in DIGI.DAT, a bad entry): the caller then starts nothing, as for a missing slot.
 */
uint8_t COLD_ANIM_BUFFER[COLD_ANIM_MAX];

/*
 * MOD (cold animations with 8 MB of RAM): a console has 2 MB, mirrored through the 8 MB window, so a word and the same word 2 MB
 * up are one. With real RAM there, the partner's cold part stays above 2 MB and its animations start without the disc; a clip
 * past COLD_CACHE_SECTORS still reads the disc, as everything does with 2 MB (MOD_COLD_CACHE_TYPE -1).
 */
#define COLD_CACHE		((uint8_t *)0x80200000)
#define COLD_CACHE_SECTORS	16
#define COLD_CACHE_MIN_TYPE	180
#define RAM_PROBE_VALUE		0x5AA5C33Cu

static uint32_t RAM_PROBE;
int32_t MOD_COLD_CACHE_TYPE = -1;	/* the species in the cache, 0 none yet, -1 no RAM above 2 MB */

int32_t modProbeRam(void)
{
	volatile uint32_t *low = &RAM_PROBE;
	volatile uint32_t *high = (volatile uint32_t *)((uint8_t *)&RAM_PROBE + 0x200000);

	*low = RAM_PROBE_VALUE;
	*high = ~RAM_PROBE_VALUE;
	if (*low != RAM_PROBE_VALUE || *high != ~RAM_PROBE_VALUE) {
		return 0;
	}
	MOD_COLD_CACHE_TYPE = 0;
	return 1;
}

/* From the partner's normal field tick: once for each species, while nothing else is reading */
void refreshColdCache(void)
{
	int32_t type = PARTNER_ENTITY.digimonEntity.entity.type;
	char *file;
	int32_t sector;
	int32_t size;

	if (MOD_COLD_CACHE_TYPE < 0 || MOD_COLD_CACHE_TYPE == type || type < COLD_CACHE_MIN_TYPE || isFileReadQueueBusy()) {
		return;
	}
	MOD_COLD_CACHE_TYPE = type;
	if (getDigimonModelRef(type, &file, &sector, &size) && file != NULL) {
		readFileSectors(file, COLD_CACHE, sector + ((size + 0x7FF) >> 11), COLD_CACHE_SECTORS);
	}
}

/*
 * MOD (cold animations with 2 MB): in the world a cold clip no longer stops the game for the disc. The partner holds its pose,
 * with the clip as its animation and a frame count nothing reaches, so whatever waits for the clip keeps waiting; the file queue
 * reads the clip, tickAnimation asks here each tick, and it starts when it has arrived. The clip shares COLD_ANIM_BUFFER with the
 * one being played, so the animation does not advance meanwhile.
 */
uint32_t MOD_COLD_PENDING;		/* the table entry the partner waits for, 0 none */
static uint32_t COLD_READ_ENTRY;	/* the entry of the last request */
static uint32_t COLD_READY;		/* the entry COLD_ANIM_BUFFER holds, for startAnimation */
static uint8_t COLD_READ_BUSY;		/* the file queue's flag */
static uint8_t COLD_PENDING_ANIM;
void startAnimation(Entity *entity, uint8_t animId);
int32_t addFileReadRequestSection(char *path, uint8_t *buffer, int32_t offset, int32_t sectors, uint8_t *isRunning, int32_t a, int32_t b);

/* tickAnimation, for an entity whose animation is off */
void tickColdAnimation(Entity *entity)
{
	if (entity == (Entity *)&PARTNER_ENTITY.digimonEntity.entity && MOD_COLD_PENDING != 0 && !COLD_READ_BUSY) {
		COLD_READY = COLD_READ_ENTRY;
		MOD_COLD_PENDING = 0;
		startAnimation(entity, COLD_PENDING_ANIM);
	}
}

uint8_t *loadColdAnimation(void *ent, uint32_t entry, int32_t animId)
{
	Entity *entity = ent;
	char *file;
	int32_t sector;
	int32_t size;
	int32_t first;
	int32_t count;
	int32_t offset;
	int32_t length;

	if (entity != (Entity *)&PARTNER_ENTITY.digimonEntity.entity) {
		return NULL;
	}
	offset = (entry & 0xFFFF) * 4;
	length = ((entry >> 16) & 0x7FFF) * 4;
	if (entity->type == MOD_COLD_CACHE_TYPE && offset + length <= COLD_CACHE_SECTORS << 11) {
		return COLD_CACHE + offset;
	}
	if (entry == COLD_READY) {
		COLD_READY = 0;
		return COLD_ANIM_BUFFER + (offset & 0x7FF);
	}
	if (!getDigimonModelRef(entity->type, &file, &sector, &size) || file == NULL) {
		return NULL;
	}
	count = ((offset & 0x7FF) + length + 0x7FF) >> 11;
	if (length == 0 || (count << 11) > COLD_ANIM_MAX) {
		return NULL;
	}
	first = sector + ((size + 0x7FF) >> 11) + (offset >> 11);
	if (MOD_COLD_CACHE_TYPE < 0 && GAME_STATE == 0) {
		while (COLD_READ_BUSY) { /* a clip replaced while it was being read */
			tickFileReadQueue(0);
		}
		COLD_READ_ENTRY = entry;
		addFileReadRequestSection(file, COLD_ANIM_BUFFER, first, count, &COLD_READ_BUSY, 0, 0);
		entity->anim.animId = animId;
		entity->anim.animFrame = 1;
		entity->anim.frameCount = 0x7FFF;
		entity->anim.loopCount = 0;
		entity->anim.animFlag &= ~1;
		MOD_COLD_PENDING = entry;
		COLD_PENDING_ANIM = animId;
		return NULL;
	}
	readFileSectors(file, COLD_ANIM_BUFFER, first, count);
	return COLD_ANIM_BUFFER + (offset & 0x7FF);
}

/* MOD (species rules as data): see mod_rules.h. Every row starts empty (species -1) and tools/balance.py writes them from content/tables/species-rules.json */
int16_t MOD_MODEL_SCALE[MOD_MODEL_SCALE_ROWS][2] = { [0 ... MOD_MODEL_SCALE_ROWS - 1] = { -1, -1 } };
int16_t MOD_ANIM_TEXTURE[MOD_ANIM_TEXTURE_ROWS][2] = { [0 ... MOD_ANIM_TEXTURE_ROWS - 1] = { -1, -1 } };

int32_t modModelScale(int32_t type)
{
	int32_t i;

	for (i = 0; i < MOD_MODEL_SCALE_ROWS; i++) {
		if (MOD_MODEL_SCALE[i][0] == type && MOD_MODEL_SCALE[i][1] > 0) {
			return MOD_MODEL_SCALE[i][1] * 0x1000 / 100;
		}
	}
	return 0x1000;
}

int32_t modAnimatedTextureWidth(int32_t type)
{
	int32_t i;

	for (i = 0; i < MOD_ANIM_TEXTURE_ROWS; i++) {
		if (MOD_ANIM_TEXTURE[i][0] == type && MOD_ANIM_TEXTURE[i][1] > 0) {
			return MOD_ANIM_TEXTURE[i][1];
		}
	}
	return 0;
}

/* Keep script/map identities stable while field NPCs use the disc originals.
 * Battle/tournament requests retain their type; partner loading bypasses it. */
int32_t npcSpeciesType(int32_t type, int32_t reverse)
{
    static const int16_t twins[][2] = {
        {119, 462}, {120, 463}, {121, 464}, {122, 465},
        {123, 466}, {124, 467}, {126, 468}
    };
    int32_t i;
    if (!reverse && GAME_STATE != 0) return type;
    for (i = 0; i < 7; i++) {
        if (twins[i][reverse != 0] == type) return twins[i][reverse == 0];
    }
    return type;
}

/*
 * MOD (flat sprites): a flattened Digimon is drawn as one of four 16x16 frames of its own texture
 * (renderFlatDigimon: idle, idle 2, attack, hurt). The originals keep them at u 0xE0/0xF0, v 0x60/0x70 with CLUT
 * row 15; a texture the build made (tools/flatsprite.py) ends, after the TIM, with "FLT2", the (u, v) of each frame
 * and their four CLUT rows. Kept per texture slot (its CLUT column: 0 the tamer, 1 the partner, 2..6 the NPC slots) when
 * the texture is uploaded.
 */
static uint32_t FLAT_SPRITES[7][3];
static const uint32_t FLAT_SPRITE_NATIVE[3] = { 0x60F060E0, 0x70F070E0, 0x0F0F0F0F };

uint8_t *getFlatSprite(ModelComponent *model)
{
	uint32_t i;

	i = model->clutPage & 0x3F;
	return (uint8_t *)(i < 7 ? FLAT_SPRITES[i] : FLAT_SPRITE_NATIVE);
}

void loadFlatSprite(void *textureData, ModelComponent *model)
{
	uint32_t *tim;
	uint32_t *dst;
	const uint32_t *src;
	uint8_t *p;

	tim = textureData;
	dst = (uint32_t *)getFlatSprite(model);
	if (dst == FLAT_SPRITE_NATIVE) {
		return;
	}
	p = (uint8_t *)tim + 8 + tim[2]; /* past the CLUT block (a model texture has one), then the pixel block */
	p += *(uint32_t *)p;
	src = FLAT_SPRITE_NATIVE;
	if (p + 16 <= (uint8_t *)tim + 0x4800 && *(uint32_t *)p == 0x32544C46) { /* "FLT2" */
		src = (uint32_t *)p + 1;
	}
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}
