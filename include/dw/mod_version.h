#ifndef DW_MOD_VERSION_H
#define DW_MOD_VERSION_H

#include <version_text.h> /* MOD_VERSION_STRING: the Makefile's MOD_VERSION (generated) */

/*
 * MOD (version): the release's version in the bottom left corner of the title screen (renderPressStartToContinue) and
 * of the title menu's first view (renderInitialMenu), main.c. Drawn into the text area's row MOD_VERSION_ROW, shown
 * in TEXT_COLORS[MOD_VERSION_COLOR] (a dim grey) with a drop shadow, from MOD_VERSION_LEFT, MOD_VERSION_TOP (screen pixels;
 * the bottom right holds TITLE2's logo). The sprite is MOD_VERSION_W wide: the row's pixels past the text are clear.
 */
#define MOD_VERSION_ROW		0x3C /* the title menu's rows are 0..0x30 (OPTIONS) */
#define MOD_VERSION_LEFT	8
#define MOD_VERSION_W		0xFF /* the whole row (a u of 8 bits) */
#define MOD_VERSION_TOP		224
#define MOD_VERSION_COLOR	8 /* 0x50 grey */

extern char MOD_VERSION_TEXT[32];
void modVersionDraw(void);
void modVersionRender(void);

#endif
