#ifndef DW_SCREEN_H
#define DW_SCREEN_H

#include <dw/ids.h>
#include <dw/map.h>

/*
 * MOD (screens beyond 255, docs/mods/ids.md § 8.5, docs/mods/maps.md "New screens"): a screen
 * NUM_ORIGINAL_SCREENS..NUM_SCREENS-1 is one sector of SCRN.DAT, read from the disc when the screen is named
 * (its caption before the fade, its MAPHEAD section when it is entered) and kept in a small cache, so a new
 * screen costs no resident RAM. The 255 original screens (0..254) keep their MAP_ENTRIES row and MAPHEAD
 * section, resident as in the original.
 *
 * SCRN.DAT: sector 0 is the file's header (SCREEN_FILE_MAGIC, u16 first id, u16 count); the record of
 * screen id is sector 1 + id - NUM_ORIGINAL_SCREENS. A sector whose magic or id does not match is no
 * screen (a disc without the file: the table entry points at sector 0 of the disc, zeros).
 */
#define SCREEN_FILE_NAME	"SCRN.DAT"
#define SCREEN_FILE_MAGIC	"SCRN"
#define SCREEN_RECORD_MAGIC	"SCR1"
#define SCREEN_RECORD_SIZE	0x800
#define SCREEN_SCRIPT_OFFSET	0x80	/* the record's MAPHEAD section: a script of its own */
#define SCREEN_SCRIPT_MAX	(SCREEN_RECORD_SIZE - SCREEN_SCRIPT_OFFSET)
#define SCREEN_CAPTION_MAX	36
#define SCREEN_CACHE_SLOTS	4

/* MapEntry.doorsId / toiletId / loadingName of a record: the record's own door, toilet or caption */
#define SCREEN_OWN		(-1)

/* ScreenRecordHead.flags: what the original decides by screen id in code, as data for a new screen */
#define SCREEN_FLAG_CONCAVE	0x0001	/* battles on a concave arena (btl.c CONCAVE_SCREENS) */
#define SCREEN_FLAG_MIST	0x0002	/* the mist layers (overworld.c isMistActive: 115-119, 121, 163, 220) */
#define SCREEN_FLAG_SHOP	0x0004	/* the SHOP overlay is loaded on entering (SHOP_MAPS) */
#define SCREEN_FLAG_ARENA	0x0008	/* the DGET overlay (tournament registration) is loaded on entering (ARENA_MAPS) */

/*
 * The first 0x80 bytes of a record. The script (scriptSize bytes at SCREEN_SCRIPT_OFFSET) has the layout of
 * MAPHEAD.SCN: u16 size of its section table, (u16 section, u16 offset) pairs ending with 0xFFFF, the code;
 * offsets are from the script's start. Its section `id` runs when the screen is entered, as MAPHEAD's would.
 */
typedef struct {
	char magic[4];			/* 0x00 SCREEN_RECORD_MAGIC */
	ScreenId id;			/* 0x04 */
	uint16_t scriptSize;		/* 0x06 0 = no section: the screen falls back to MAPHEAD's */
	MapEntry entry;			/* 0x08 the MAP_ENTRIES row: doorsId, toiletId, loadingName may be SCREEN_OWN */
	uint32_t flags;			/* 0x18 SCREEN_FLAG_* */
	ToiletData toilet;		/* 0x1C */
	uint8_t door[54];		/* 0x24 a DoorStruct (map_object.h): 6 model ids, then x, y, z, rotation of each */
	uint8_t pad[2];			/* 0x5A */
	char caption[SCREEN_CAPTION_MAX]; /* 0x5C the area name (loadingName SCREEN_OWN) */
} ScreenRecordHead;

static inline int32_t isNewScreen(int32_t id)
{
	return id >= NUM_ORIGINAL_SCREENS && id < NUM_SCREENS;
}

MapEntry *getScreenEntry(int32_t id);
#define SCREEN_ENTRY(id) (*getScreenEntry(id))
uint32_t getScreenFlags(int32_t id);
char *getScreenCaption(int32_t id);
ToiletData *getScreenToilet(int32_t id);
void *getScreenDoor(int32_t id); /* a DoorStruct */
uint8_t *getMapHeadScript(int32_t section);

#endif
