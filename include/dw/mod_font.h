#ifndef DW_MOD_FONT_H
#define DW_MOD_FONT_H

/*
 * MOD (font): the build's font (Makefile `FONT=original|a|b`, generated font_sel.h; docs/mods/font.md "Fonts").
 * FONT_PROPORTIONAL: glyphs are placed to the pixel (font.c drawGlyph) and their width is their advance in pixels;
 * otherwise the original code: a 12-pixel cell on the 4-pixel grid of the text area.
 */
#include <font_sel.h>
#include <dw/types.h>
#include <dw/version.h>

#if defined(FONT_A) || defined(FONT_B)
#define FONT_PROPORTIONAL 1
#else
#define FONT_PROPORTIONAL 0
#endif

/* a blank as wide as a digit: intToStringSJIS pads numbers with it, so they stay right-aligned (Shift-JIS 824E is unassigned) */
#define FONT_FIGURE_SPACE 0x824e

/* a C string that a screen cuts into pieces at the original's offsets: drawn in the original's cells (font.c) */
#if VERSION_IS(US) && FONT_PROPORTIONAL
void drawStringCells(char *str, int32_t x, int32_t y);
#define DRAW_STRING_CELLS(str, x, y) drawStringCells(str, x, y)
#else
#define DRAW_STRING_CELLS(str, x, y) drawString(str, x, y)
#endif
#if VERSION_IS(US)
extern uint8_t TEXTBOX_CELLS;
#endif

/*
 * a C string whose row a screen copies in pieces (the crops' offsets in pixels from x: 0, then each next crop's srcX - x;
 * PIECE_CENTRE marks a piece centred in its original span, PIECE_LEFT one that starts as far into its crop as the row's
 * first piece, for crops shown one under the other such as Yes / No): each piece drawn with the font's spacing (font.c
 * drawStringPieces); the original font draws it as drawString
 */
#define PIECE_CENTRE 0x8000
#define PIECE_LEFT   0x4000
#define PIECE_FLAGS  (PIECE_CENTRE | PIECE_LEFT)
#define PIECES_END   0xffff
#if VERSION_IS(US) && FONT_PROPORTIONAL
void drawStringPieces(char *str, int32_t x, int32_t y, const uint16_t *pieces);
void drawGlyphPieces(const uint16_t *glyphs, const uint8_t *cells, int32_t n, int32_t x, int32_t y, const uint16_t *pieces);
#define DRAW_STRING_PIECES(str, x, y, ...)                                         \
	do {                                                                       \
		static const uint16_t PIECES_[] = { __VA_ARGS__, PIECES_END };       \
		drawStringPieces(str, x, y, PIECES_);                              \
	} while (0)
#else
#define DRAW_STRING_PIECES(str, x, y, ...) drawString(str, x, y)
#endif

/*
 * the rows of a text box (script_textbox.c) that a screen cuts into pieces: after createTextbox, in the original's cells
 * (with the original font: a space advances as the original's, by the glyph before it, not the mod's 8 pixels)
 */
#if VERSION_IS(US)
#define TEXTBOX_IN_CELLS(boxId) (TEXTBOX_CELLS |= 1 << (boxId))
#else
#define TEXTBOX_IN_CELLS(boxId)
#endif

/*
 * one row of a text box that a screen copies in pieces (the crops' offsets as for DRAW_STRING_PIECES): after
 * createTextbox, that row laid out as drawStringPieces lays out a C string, each glyph's original cell advancing as
 * drawString2 advanced it (a space by the glyph before it); the box's other rows to the pixel. The original font: the
 * box in the original's cells (TEXTBOX_IN_CELLS). One such row at a time (script_textbox.c TEXTBOX_PIECES)
 */
#if VERSION_IS(US) && FONT_PROPORTIONAL
void textboxRowPieces(int32_t boxId, int32_t row, const uint16_t *pieces);
#define TEXTBOX_ROW_PIECES(boxId, row, ...)                                        \
	do {                                                                       \
		static const uint16_t PIECES_[] = { __VA_ARGS__, PIECES_END };       \
		textboxRowPieces(boxId, row, PIECES_);                             \
	} while (0)
#else
#define TEXTBOX_ROW_PIECES(boxId, row, ...) TEXTBOX_IN_CELLS(boxId)
#endif

#endif
