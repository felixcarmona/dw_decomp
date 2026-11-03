#ifndef DW_FILE_TABLE_H
#define DW_FILE_TABLE_H

#include <dw/types.h>

extern int16_t ATAN_LOOKUP_TABLE[];
extern int16_t SIN_LOOKUP_TABLE[];

typedef struct {
    int32_t sector;
    int32_t size;
    char *filename;
} FileEntry;

extern FileEntry FILE_OFFSET_TABLE[];
extern const int16_t FILE_TABLE_NEW_FIRST; /* MOD (language): the first of the new files (NEW_FILES) */

#endif
