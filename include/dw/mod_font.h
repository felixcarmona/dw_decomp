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
extern uint8_t TEXTBOX_CELLS;
#define DRAW_STRING_CELLS(str, x, y) drawStringCells(str, x, y)
#else
#define DRAW_STRING_CELLS(str, x, y) drawString(str, x, y)
#endif

/* the rows of a text box (script_textbox.c) that a screen cuts into pieces: after createTextbox, in the original's cells */
#if VERSION_IS(US) && FONT_PROPORTIONAL
#define TEXTBOX_IN_CELLS(boxId) (TEXTBOX_CELLS |= 1 << (boxId))
#else
#define TEXTBOX_IN_CELLS(boxId)
#endif

#endif
