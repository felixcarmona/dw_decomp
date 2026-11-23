#ifndef DW_MOD_FONT_H
#define DW_MOD_FONT_H

/*
 * MOD (font): the build's font (Makefile `FONT=original|b`, generated font_sel.h; docs/mods/font.md "Fonts").
 * FONT_SWITCH: a FONT=b build carries both fonts, and the title menu's OPTIONS (FONT) chooses at run time: NEW (font
 * b), CLASSIC (the original letters placed to the pixel, evenly spaced) or ORIGINAL (the original code). FONT_PROPORTIONAL
 * is then true for NEW and CLASSIC: glyphs placed to the pixel (font.c drawGlyph), their width their advance in pixels;
 * otherwise (ORIGINAL, or a FONT=original build) the original code: a 12-pixel cell on the 4-pixel grid of the text
 * area.
 */
#include <font_sel.h>
#include <dw/types.h>
#include <dw/version.h>

#if defined(FONT_B) && VERSION_IS(US)
#define FONT_SWITCH 1
#define MOD_FONT_NEW		0 /* font b */
#define MOD_FONT_ORIGINAL	1 /* the original font, as the original draws it */
#define MOD_FONT_CLASSIC	2 /* the original letters placed to the pixel */
extern int8_t MOD_FONT; /* OPTIONS' FONT (font.c) */
#define FONT_PROPORTIONAL (MOD_FONT != MOD_FONT_ORIGINAL)
/* the font changes (OPTIONS, a loaded save): the text from now on is drawn in it, and the script text is read from the
 * files paged for it (script_interp.c modFontScriptsChanged; CLASSIC reads the original font's, which it draws narrower) */
void modSetFont(int32_t font);
void modFontScriptsChanged(void);
/* `path`, or with the original letters (ORIGINAL, CLASSIC) the file `originalFont` (no directory) paged for the original
 * font when the disc has it (file.c) */
char *modFontFile(char *path, char *originalFont);
#define FONT_FILE(path, originalFont) modFontFile(path, originalFont)
#else
#define FONT_SWITCH 0
#define FONT_PROPORTIONAL 0
#define FONT_FILE(path, originalFont) (path)
#endif

/* a blank as wide as a digit: intToStringSJIS pads numbers with it, so they stay right-aligned (Shift-JIS 824E is unassigned) */
#define FONT_FIGURE_SPACE 0x824e

/* a C string that a screen cuts into pieces at the original's offsets: drawn in the original's cells (font.c) */
#if FONT_SWITCH
void drawStringCells(char *str, int32_t x, int32_t y);
#define DRAW_STRING_CELLS(str, x, y) (FONT_PROPORTIONAL ? drawStringCells(str, x, y) : drawString(str, x, y))
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
 * drawStringPieces); PIECE_WORDS assigns whitespace-separated answers to successive crops
 * instead of cutting by cell advances. The original font also uses that layout.
 */
#define PIECE_CENTRE 0x8000
#define PIECE_LEFT   0x4000
#define PIECE_WORDS  0x2000 /* whitespace-separated answers, independent of translated cell widths */
#define PIECE_FLAGS  (PIECE_CENTRE | PIECE_LEFT | PIECE_WORDS)
#define PIECES_END   0xffff
#if FONT_SWITCH
void drawStringPieces(char *str, int32_t x, int32_t y, const uint16_t *pieces);
void drawGlyphPieces(const uint16_t *glyphs, const uint8_t *cells, int32_t n, int32_t x, int32_t y, const uint16_t *pieces);
extern const uint8_t *PIECE_COLORS; /* MOD (language): each glyph's colour for drawGlyphPieces, or NULL */
#define DRAW_STRING_PIECES(str, x, y, ...)                                         \
	do {                                                                       \
		static const uint16_t PIECES_[] = { __VA_ARGS__, PIECES_END };       \
		if (FONT_PROPORTIONAL || (PIECES_[0] & PIECE_WORDS)) {                \
			drawStringPieces(str, x, y, PIECES_);                      \
		} else {                                                           \
			drawString(str, x, y);                                     \
		}                                                                  \
	} while (0)
/* the same with the crops' offsets in a table (PIECES_END at its end): a language's own (dw/lang.h) */
#define DRAW_STRING_PIECES_AT(str, x, y, pieces)                                   \
	do {                                                                       \
		if (FONT_PROPORTIONAL || ((pieces)[0] & PIECE_WORDS)) {               \
			drawStringPieces(str, x, y, pieces);                       \
		} else {                                                           \
			drawString(str, x, y);                                     \
		}                                                                  \
	} while (0)
#else
#define DRAW_STRING_PIECES(str, x, y, ...) drawString(str, x, y)
#define DRAW_STRING_PIECES_AT(str, x, y, pieces) drawString(str, x, y)
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
#if FONT_SWITCH
void textboxRowPieces(int32_t boxId, int32_t row, const uint16_t *pieces);
#define TEXTBOX_ROW_PIECES(boxId, row, ...)                                        \
	do {                                                                       \
		static const uint16_t PIECES_[] = { __VA_ARGS__, PIECES_END };       \
		if (FONT_PROPORTIONAL || (PIECES_[0] & PIECE_WORDS)) {                \
			textboxRowPieces(boxId, row, PIECES_);                     \
		} else {                                                           \
			TEXTBOX_IN_CELLS(boxId);                                   \
		}                                                                  \
	} while (0)
#else
#define TEXTBOX_ROW_PIECES(boxId, row, ...) TEXTBOX_IN_CELLS(boxId)
#endif

#endif
