#ifndef DW_FILE_TABLE_H
#define DW_FILE_TABLE_H

#include <dw/types.h>

extern int16_t ATAN_LOOKUP_TABLE[];
extern int16_t SIN_LOOKUP_TABLE[];

/*
 * MOD (file table): the name is in the entry (8.3 names of the disc, 12 letters at most; one of 12 has no NUL), not a
 * pointer to a string: 20 bytes an entry, not 12 and a string aligned to 4 (16 at most). The mod's discs add hundreds
 * of names (new screens, language and story copies) to the executable's resident data. Compare with fileEntryIs.
 */
#define FILE_NAME_MAX 12

typedef struct {
    int32_t sector;
    int32_t size;
    uint32_t nameHash; /* FNV-1a of the filename; the builder rejects collisions. */
} FileEntry;

extern FileEntry FILE_OFFSET_TABLE[];
/* MOD (file table): 1 when the entry is the file `name` */
int32_t fileEntryIs(FileEntry *entry, char *name);
extern const int16_t FILE_TABLE_NEW_FIRST; /* MOD (language): the first of the new files (NEW_FILES) */

#endif
