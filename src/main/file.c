#include <libcd.h>
#include <libgpu.h>
#include <libgs.h>
#include <string.h>
#include <sys/types.h>
#include <dw/file.h>
#include <dw/file_table.h>
#include <dw/main.h>
#include <dw/mod_font.h>
#include <dw/mod_options.h>
#include <dw/tfs_stream.h>

#include "common.h"

static void empty_800a31f0(char *path)
{
}

#if FONT_SWITCH
/*
 * MOD (font): the script text is paged for the font that draws it. A build with both fonts puts on the disc a copy of
 * DG.SCN, MAPHEAD.SCN and SCRN.DAT paged for the original one (tools/build-full.sh): with NEW FONT off the game reads
 * that copy, when build-disc.py wrote its sector (an entry it did not fill keeps sector 0)
 */
char *modFontFile(char *path, char *originalFont)
{
	FileEntry *entry;

	if (MOD_FONT == MOD_FONT_NEW) {
		return path;
	}
	for (entry = FILE_OFFSET_TABLE; entry->size != 0; ++entry) {
		if (fileEntryIs(entry, originalFont)) {
			return entry->sector != 0 ? originalFont : path;
		}
	}
	return path;
}
#endif

int32_t fileEntryIs(FileEntry *entry, char *name)
{
    uint32_t hash = 0x811C9DC5;
    int32_t i;
    if (name[0] == '@') { /* internal language-copy key; not an ISO filename */
        hash = 0;
        for (i = 1; i <= 8; i++) hash = (hash << 4) | (name[i] <= '9' ? name[i]-'0' : name[i]-'A'+10);
    } else {
        for (i = 0; name[i]; i++) hash = (hash ^ (uint8_t)name[i]) * 0x01000193;
    }
    return hash == entry->nameHash;
}

int32_t lookupFileTable(FileLookup *lookup, char *path)
{
	char *filename;
	FileEntry *entry;

	entry = (FileEntry *)&FILE_OFFSET_TABLE;
	if (NULL == (filename = strrchr(path, '\\'))) {
		filename = path;
	} else {
		++filename;
	}
	filename = modStoryFile(modLangFile(filename)); /* MOD (language, story): the language's copy, then its story's */

	for (; entry->size != 0; ++entry) {
		if (fileEntryIs(entry, filename)) {
			lookup->size = entry->size;
			CdIntToPos(entry->sector, &lookup->pos);
			break;
		}
	}

	if (entry->size) {
		return 1;
	} else {
		empty_800a31f0(path);
		return 0;
	}
}

uint32_t lookupFileSize(char *path)
{
	FileLookup lookup;

	lookupFileTable(&lookup, path);

	return lookup.size;
}

int32_t loadTextureFile(char *path, uint32_t *outTPage, uint32_t *outClut)
{
	u_long *addr;
	GsIMAGE image;
	RECT rect;
	int32_t size;
	u_long *buf;

	size = lookupFileSize(path);
	addr = buf = (u_long *)TEXTURE_BUFFER;
	readFile(path, buf);

	GsGetTimInfo(buf + 1, &image);

	setRECT(&rect, image.px, image.py, image.pw, image.ph);
	LoadImage(&rect, image.pixel);

	*outTPage = GetTPage(image.pmode & 3, 0, image.px, image.py);

	if (((image.pmode >> 3) & 1) != 0) {
		setRECT(&rect, image.cx, image.cy, image.cw, image.ch);
		LoadImage(&rect, image.clut);

		*outClut = GetClut(image.cx, image.cy);
	}
}

int32_t readFile(char *path, void *buffer)
{
	FileLookup lookup;
	int32_t result;

	if (lookupFileTable(&lookup, path) != 0) {
		tfsStreamCheckTarget(buffer, lookup.size); /* MOD (memreclaim) */
		do {
			do {
				while (CdControl(CdlSetloc,
						 (u_char *)&lookup.pos,
						 NULL) == 0);
			} while (CdRead(((lookup.size + 0x7ff) & ~0x7ff) >> 11,
					(u_long *)buffer, CdlModeSpeed) == 0);
			while ((result = CdReadSync(0, NULL)) > 0);
		} while (result != 0);
	}
}

int32_t loadTIMFile(char *path, void *buffer)
{
	GsIMAGE image;
	RECT rect;

	readFile(path, buffer);

	GsGetTimInfo((u_long *)buffer + 1, &image);

	setRECT(&rect, image.px, image.py, image.pw, image.ph);
	LoadImage(&rect, image.pixel);

	if (((image.pmode >> 3) & 1) != 0) {
		setRECT(&rect, image.cx, image.cy, image.cw, image.ch);
		LoadImage(&rect, image.clut);
	}
}

int32_t loadStackedTIMEntry(char *path, void *buffer, int32_t offset,
			    int32_t sectors)
{
	GsIMAGE image;
	RECT rect;

	readFileSectors(path, buffer, offset, sectors);
	GsGetTimInfo((u_long *)buffer + 1, &image);

	setRECT(&rect, image.px, image.py, image.pw, image.ph);
	LoadImage(&rect, image.pixel);

	if (((image.pmode >> 3) & 1) != 0) {
		setRECT(&rect, image.cx, image.cy, image.cw, image.ch);
		LoadImage(&rect, image.clut);
	}
}

int32_t readFileSectors(char *path, void *buffer, int32_t offset,
			int32_t sectors)
{
	FileLookup lookup;
	int32_t result;

	if (lookupFileTable(&lookup, path) != 0) {
		CdIntToPos(offset + CdPosToInt(&lookup.pos), &lookup.pos);
		tfsStreamCheckTarget(buffer, sectors << 11); /* MOD (memreclaim) */
		do {
			do {
				while (CdControl(CdlSetloc,
						 (u_char *)&lookup.pos,
						 NULL) == 0);
			} while (CdRead(sectors, buffer, CdlModeSpeed) == 0);
			while ((result = CdReadSync(0, NULL)) > 0);
		} while (result != 0);
	}
}
