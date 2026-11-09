/*
 * MOD (screens beyond 255): the screens NUM_ORIGINAL_SCREENS..NUM_SCREENS-1 are records of SCRN.DAT
 * (include/dw/screen.h, docs/mods/ids.md "Screens beyond 255", docs/mods/maps.md "New screens").
 * An original screen reads MAP_ENTRIES and MAPHEAD as before.
 */
#include <dw/file.h>
#include <dw/main.h>
#include <dw/screen.h>

#include "common.h"

extern char *MAP_NAME_PTR[];
extern uint8_t *MAPHEAD_SCRIPT_PTR;
void tickFileReadQueue(int32_t unused);
int32_t isFileReadQueueBusy(void);

/* the last record read: its whole sector (the script of a new screen runs from here) */
static uint32_t SCREEN_SECTOR[SCREEN_RECORD_SIZE / 4];
static int16_t SCREEN_SECTOR_ID = -1;
/* the heads of the last screens named; the current screen's slot is never replaced */
static ScreenRecordHead SCREEN_CACHE[SCREEN_CACHE_SLOTS];
static int16_t SCREEN_CACHE_ID[SCREEN_CACHE_SLOTS] = { -1, -1, -1, -1 };
static int32_t SCREEN_CACHE_NEXT;
int32_t SCREEN_RECORD_READS; /* for the tests (tools/playtest/screens.sh) */
/* what a screen without a record reads: no file, no sound bank, no doors, no toilet, an empty caption */
static ScreenRecordHead NO_SCREEN = { "", -1, 0, { "", 0, 0, 0x40, 0, 0, SCREEN_OWN } };

/* the record of a new screen into SCREEN_SECTOR; 0 = the disc has none */
static int32_t readScreenRecord(int32_t id)
{
	FileLookup lookup;
	ScreenRecordHead *rec = (ScreenRecordHead *)SCREEN_SECTOR;
	int32_t sector = 1 + id - NUM_ORIGINAL_SCREENS;

	if (SCREEN_SECTOR_ID == id) {
		return 1;
	}
	if (lookupFileTable(&lookup, SCREEN_FILE_NAME) == 0 || lookup.size < (uint32_t)(sector + 1) * SCREEN_RECORD_SIZE) {
		return 0;
	}
	/* never in the middle of an asynchronous read (evolution, sounds, overlays) */
	while (isFileReadQueueBusy()) {
		tickFileReadQueue(0);
	}
	readFileSectors(SCREEN_FILE_NAME, SCREEN_SECTOR, sector, 1);
	SCREEN_RECORD_READS++;
	if (rec->magic[0] != SCREEN_RECORD_MAGIC[0] || rec->magic[1] != SCREEN_RECORD_MAGIC[1] ||
	    rec->magic[2] != SCREEN_RECORD_MAGIC[2] || rec->magic[3] != SCREEN_RECORD_MAGIC[3] || rec->id != id ||
	    rec->scriptSize > SCREEN_SCRIPT_MAX) {
		SCREEN_SECTOR_ID = -1;
		return 0;
	}
	SCREEN_SECTOR_ID = id;
	return 1;
}

static ScreenRecordHead *getScreenHead(int32_t id)
{
	int32_t i;

	for (i = 0; i < SCREEN_CACHE_SLOTS; i++) {
		if (SCREEN_CACHE_ID[i] == id) {
			return &SCREEN_CACHE[i];
		}
	}
	if (!readScreenRecord(id)) {
		return &NO_SCREEN;
	}
	for (i = 0; i < SCREEN_CACHE_SLOTS; i++) {
		SCREEN_CACHE_NEXT = (SCREEN_CACHE_NEXT + 1) % SCREEN_CACHE_SLOTS;
		if (SCREEN_CACHE_ID[SCREEN_CACHE_NEXT] != CURRENT_SCREEN) {
			break;
		}
	}
	SCREEN_CACHE[SCREEN_CACHE_NEXT] = *(ScreenRecordHead *)SCREEN_SECTOR;
	SCREEN_CACHE_ID[SCREEN_CACHE_NEXT] = id;
	return &SCREEN_CACHE[SCREEN_CACHE_NEXT];
}

MapEntry *getScreenEntry(int32_t id)
{
	if ((uint32_t)id < NUM_ORIGINAL_SCREENS) {
		return &MAP_ENTRIES[id];
	}
	if (isNewScreen(id)) {
		return &getScreenHead(id)->entry;
	}
	return &NO_SCREEN.entry;
}

uint32_t getScreenFlags(int32_t id)
{
	return isNewScreen(id) ? getScreenHead(id)->flags : 0;
}

char *getScreenCaption(int32_t id)
{
	ScreenRecordHead *head;

	if ((uint32_t)id < NUM_ORIGINAL_SCREENS) {
		return MAP_NAME_PTR[MAP_ENTRIES[id].loadingName];
	}
	head = isNewScreen(id) ? getScreenHead(id) : &NO_SCREEN;
	if (head->entry.loadingName != SCREEN_OWN) {
		return MAP_NAME_PTR[head->entry.loadingName];
	}
	head->caption[SCREEN_CAPTION_MAX - 1] = '\0';
	return head->caption;
}

/* the toilet of the screen's toiletId (> 0: TOILET_DATA, SCREEN_OWN: the record's) */
ToiletData *getScreenToilet(int32_t id)
{
	MapEntry *e = getScreenEntry(id);

	if (isNewScreen(id) && e->toiletId == SCREEN_OWN) {
		return &getScreenHead(id)->toilet;
	}
	return &TOILET_DATA[e->toiletId - 1];
}

void *getScreenDoor(int32_t id)
{
	return getScreenHead(id)->door;
}

/*
 * The script whose section `section` callScriptSection(0, section) runs: a new screen's record when it has
 * one, else MAPHEAD (the original screens, the shared sections 1220.., a new screen without a record).
 */
uint8_t *getMapHeadScript(int32_t section)
{
	if (isNewScreen(section) && readScreenRecord(section) && ((ScreenRecordHead *)SCREEN_SECTOR)->scriptSize != 0) {
		getScreenHead(section); /* its row is needed next (loadMap) */
		return (uint8_t *)SCREEN_SECTOR + SCREEN_SCRIPT_OFFSET;
	}
	return MAPHEAD_SCRIPT_PTR;
}

/*
 * MOD (screen jump, temporary test tool): the new screen the disc has `step` (1 or -1) away from `from` in id order,
 * wrapping at the ends of SCRN.DAT's range; from an original screen, the first (step 1) or the last. -1 when the
 * disc has none. A sector found empty is remembered, so a gap in the ids is read once.
 */
static uint8_t SCREEN_ABSENT[NUM_SCREENS / 8];

int32_t findNewScreen(int32_t from, int32_t step)
{
	FileLookup lookup;
	int32_t count;
	int32_t i;
	int32_t k;

	if (lookupFileTable(&lookup, SCREEN_FILE_NAME) == 0) {
		return -1;
	}
	count = lookup.size / SCREEN_RECORD_SIZE - 1;
	if (count > NUM_SCREENS - NUM_ORIGINAL_SCREENS) {
		count = NUM_SCREENS - NUM_ORIGINAL_SCREENS;
	}
	if (count <= 0) {
		return -1;
	}
	k = from - NUM_ORIGINAL_SCREENS;
	if (k < 0 || k >= count) {
		k = step > 0 ? -1 : count;
	}
	for (i = 0; i < count; i++) {
		int32_t id;

		k = (k + step + count) % count;
		id = NUM_ORIGINAL_SCREENS + k;
		if (SCREEN_ABSENT[id >> 3] & (1 << (id & 7))) {
			continue;
		}
		if (readScreenRecord(id)) {
			return id;
		}
		SCREEN_ABSENT[id >> 3] |= 1 << (id & 7);
	}
	return -1;
}
