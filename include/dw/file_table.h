#ifndef DW_FILE_TABLE_H
#define DW_FILE_TABLE_H

#include <dw/types.h>

extern int16_t ATAN_LOOKUP_TABLE[];
extern int16_t SIN_LOOKUP_TABLE[];

/* Packed DWFH v2: exact 19-bit LBA, lossless 21-bit size code,
 * and a collision-checked 24-bit FNV key. Sizes below 1 MiB are literal;
 * larger archives use 64-byte units (up to 64 MiB). The disc writer rejects
 * unrepresentable sectors/sizes; packed access is byte-aligned on R3000. */
#define FILE_NAME_MAX 12
#define FILE_SIZE_CODE(n) ((n) < 0x100000 ? (n) : 0x100000 | ((n) >> 6))

typedef struct __attribute__((packed)) {
    uint32_t sector : 19;
    uint32_t sizeCode : 21;
    uint32_t nameHash : 24;
} FileEntry;
_Static_assert(sizeof(FileEntry) == 8, "packed file entry must occupy eight bytes");

static inline uint32_t fileEntrySize(const FileEntry *entry)
{
    uint32_t n = entry->sizeCode;
    return n & 0x100000 ? (n & 0xFFFFF) << 6 : n;
}

extern FileEntry FILE_OFFSET_TABLE[];
/* MOD (file table): 1 when the entry is the file `name` */
int32_t fileEntryIs(FileEntry *entry, char *name);
uint32_t fileNameHash(char *name);
FileEntry *findFileEntry(char *name);
extern const int16_t FILE_TABLE_NEW_FIRST; /* MOD (language): the first of the new files (NEW_FILES) */

#endif
