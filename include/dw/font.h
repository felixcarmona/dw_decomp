#ifndef DW_FONT_H
#define DW_FONT_H

#include <libgpu.h>

#include <dw/types.h>
#include <dw/mod_font.h>
#include <dw/version.h>

void initializeFontCLUT(void);
#define TEXT_VRAM_X		704
#define TEXT_VRAM_Y		256

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
#define FIT_COMPACT	3 /* condensed glyphs with no extra gap, for composed reward rows */
#define FIT_CONDENSED	2 /* the same, with a 5-column lowercase */
int32_t measureStringMode(char *str, int32_t mode);
int32_t fitStringMode(char *str, int32_t maxW);
int32_t drawStringMode(char *str, int32_t x, int32_t y, int32_t mode);
/*
 * MOD (item names): drawString2 codes of the mod for the item lists that are script-text lines (shop, keeper, amount
 * box, Mojyamon's trades). Scripts never pass these bytes to drawString2 (showTextbox expands 0x10-0x15).
 *   TEXT_FIT_TIGHT / TEXT_FIT_CONDENSED, colour (0xFF keeps it): the name that follows, up to the next code, drawn
 *     with drawStringMode; textFitCode gives 1 (the plain colour code) for a name that fits as the font draws it
 *   TEXT_COLUMN, x: clears to column x and moves the pen there
 */
#define TEXT_FIT_TIGHT		0x10
#define TEXT_FIT_CONDENSED	0x11
#define TEXT_COLUMN		0x12
int32_t textFitCode(char *str, int32_t maxW);
int32_t drawFittedRow(char *str, int32_t y, int32_t mode, const uint8_t *colors);
/* MOD (item names): the item name field of the inventory, the fishing bait list, the shop's amount box, the keeper and
 * Mojyamon's trades: 96 pixels (the shop's lists: 93) */
#define ITEM_FIELD_W		0x60
/* MOD (font): only in a build whose font is placed to the pixel (dw/mod_font.h): a glyph's advance in pixels; set while
 * text is laid out in the original's cells (font.c FONT_CELLS) */
int32_t glyphWidth(uint16_t codepoint);
int32_t glyphCell(uint16_t codepoint); /* the original glyph's width */
extern int8_t FONT_CELLS;
#endif

#endif
