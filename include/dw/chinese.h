#ifndef DW_CHINESE_H
#define DW_CHINESE_H
#include <dw/types.h>
#include <dw/file.h>
#include <dw/params.h>
/* Reserved extra RAM: independent of the cold animation cache at 0x80200000. */
#define ZH_BASE 0x80400000
#define ZH_PREFIX_SIZE 0x70000
#define ZH_FONT 0x80410000
#define ZH_NAMES 0x80430000
#define ZH_DIRECTORY 0x80440000
#define ZH_PATCH 0x80450000
#define ZH_MAGIC 0x5A435744u
DigimonPara *zhSpeciesPara(DigimonPara *para, int32_t type);
int32_t zhStart(void);
int32_t zhPreviousFont(void);
int32_t zhLookupFile(FileLookup *lookup, char *name);
int32_t zhDrawGlyph(uint16_t codepoint, uint16_t x, uint16_t y);
int32_t zhGlyphWidth(uint16_t codepoint);
void zhVsReset(void);
void zhRegisterName(char *name, int32_t type);
#endif
