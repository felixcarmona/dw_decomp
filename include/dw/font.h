#ifndef DW_FONT_H
#define DW_FONT_H

#include <libgpu.h>

#include <dw/types.h>
#include <dw/version.h>

void initializeFontCLUT(void);
void clearTextArea(void);
void clearTextSubArea(RECT *rect);
void setTextColor(int32_t color);
#if !VERSION_IS(US)
void drawGlyph(uint16_t codepoint, uint16_t x, uint16_t y);
#else
int32_t drawGlyph(/* uint16_t codepoint, uint16_t x, uint16_t y */);
#endif
void drawString(/* char *str, uint16_t x, uint16_t y */);
#if VERSION_IS(US)
/* MOD (long names): a name drawn tighter when it does not fit its field (font.c drawStringMode) */
#define FIT_VANILLA	0 /* drawString: the font's own spacing */
#define FIT_TIGHT	1 /* the same glyphs, one blank column between letters */
#define FIT_CONDENSED	2 /* the same, with a 5-column lowercase */
int32_t measureStringMode(char *str, int32_t mode);
int32_t fitStringMode(char *str, int32_t maxW);
int32_t drawStringMode(char *str, int32_t x, int32_t y, int32_t mode);
#endif

#endif
