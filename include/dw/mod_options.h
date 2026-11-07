#ifndef DW_MOD_OPTIONS_H
#define DW_MOD_OPTIONS_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#include <dw/types.h>

/*
 * MOD (options): the title menu's OPTIONS (main_menu.c) and what they change (mod_options.c, docs/mods/options.md).
 */

#define MOD_BUTTONS_US	0	/* × confirms, △ cancels, ○ is the third button (the US disc) */
#define MOD_BUTTONS_JP	1	/* JAPANESE CONTROLS: ○ confirms, × cancels, △ is the third button (input.h) */

extern int32_t MOD_OPT_BUTTONS;	/* JAPANESE CONTROLS: MOD_BUTTONS_JP (input.h reads it) */
extern int32_t MOD_OPT_CLOCK;	/* DIGITAL CLOCK: the hour in digits under the clock (clock.c), 1 = shown */
extern int32_t MOD_OPT_BARS;	/* NEW STATUS BARS: happiness and discipline as one red/green fill (patches 0162-0164), 0 = the original's */
extern int32_t MOD_OPT_EVO_GUIDE; /* EVOLUTION GUIDE: the evolution chart's requirement card and lit lines (patch 0176), 0 = the original chart */
extern int32_t MOD_OPT_TOUCHED;	/* changed in OPTIONS since the console started: a loaded save keeps it */

/* the code of the button glyph that does what the US layout's glyph `code` (Shift-JIS, big endian) does */
uint16_t modButtonGlyph(uint16_t code);
/* the colour (setTextColor's) to draw the US layout's button glyph `code` in, given the text's colour `color` */
int32_t modButtonColor(uint16_t code, int32_t color);
/* the CLUT of the dialogue box's button frames (the ○ is red) */
uint16_t modButtonClut(void);
/* the button layout changes: the dialogue box's button in VRAM follows it */
void modSetButtons(int32_t layout);
/* FRAME RATE: the rate the game is shown at, 30 or 60 (20 in an FPS=20 build, which has no choice) */
int32_t modFrameRate(void);
void modSetFrameRate(int32_t fps);
/* a TIM of ETCTIM.BIN was uploaded (loadStackedTIMFile): the SYSTEM_W page's button frames */
void modButtonsTimLoaded(GsIMAGE *img);

#endif
