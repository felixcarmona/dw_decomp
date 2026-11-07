/*
 * MOD (options): what the title menu's OPTIONS change (main_menu.c draws and runs the menu; docs/mods/options.md).
 *
 * JAPANESE CONTROLS. The Japanese disc's code differs from the US one in three masks only (input.h): ○ confirms, ×
 * cancels, △ is the third button; whatever the code tests by its bit (△ opening the field menu) is the same on both.
 * With the option on, input.h's CONFIRM_BUTTON, CANCEL_BUTTON and ALT_BUTTON read MOD_OPT_BUTTONS and are the
 * Japanese disc's, so every screen answers as on that disc.
 * The text follows: the font draws each button glyph as the button that now does its job (modButtonGlyph, font.c), so
 * "×Select △Cancel" reads "○Select ×Cancel" without a text of its own; a △ painted green ({color 5}, the fishing help's
 * "submenu, under △") is the field menu's button and stays. A button glyph the text paints in its button's colour
 * (× cyan 6, △ green 5, ○ orange 10) takes the colour of the button it now is (modButtonColor). The dialogue box's
 * button (SYSTEM_W.TIM, three 16x12 frames pressing it, drawn by renderUIBox and the care menu's badge) is made a ○
 * from the disc's own frames: the cross is painted over with the button's face and a ring drawn where it was, in the
 * red of the PlayStation's ○ (its own CLUT, modButtonClut).
 *
 * DIGITAL CLOCK. The hour as HH:MM on a plate under the clock (patch 0166, renderDigitalClock in clock.c), on or off.
 *
 * NEW STATUS BARS. The field's happiness and discipline bars as patches 0162-0164 draw them (one fill, red below the
 * midpoint, green from it, on a grey track), or off, as the original drew them (renderStatusBars, partner_impl.c).
 *
 * FRAME RATE. 20 (the original: one render per tick) or 60 fps (MOD_FPS_MODE, dw/mod_fps.h); the game logic runs
 * at 20 Hz in both (30 is still a mode for an FPS=30 build). A build made with FPS=20
 * keeps 20 and the option shows it without a choice.
 */
#include <dw/mod_options.h>
#include <dw/species.h>
#include <dw/mod_fps.h>

#include "common.h"

extern uint8_t COLORCODE_LOWBITS; /* font.c: the text's colour */

int32_t MOD_OPT_BUTTONS = MOD_BUTTONS_US;
int32_t MOD_OPT_CLOCK = 1;
int32_t MOD_OPT_BARS = 1;
int32_t MOD_OPT_EVO_GUIDE = 1;
int32_t MOD_OPT_NEW_DIGIMON = 1;

int32_t modSpeciesAllowed(int32_t type)
{
	return MOD_OPT_NEW_DIGIMON || isVanillaSpecies(type);
}
int32_t MOD_OPT_TOUCHED = 0;

/* the colour the text draws a button glyph in (the US disc's: × cyan, △ green, ○ orange); -1 = not a button */
static int32_t buttonColor(uint16_t code)
{
	switch (code) {
	case 0x81A2:
		return 6;
	case 0x817E:
		return 5;
	case 0x819B:
		return 10;
	}
	return -1;
}

int32_t modButtonColor(uint16_t code, int32_t color)
{
	if (MOD_OPT_BUTTONS == MOD_BUTTONS_JP && color == buttonColor(code)) {
		return buttonColor(modButtonGlyph(code));
	}
	return color;
}

uint16_t modButtonGlyph(uint16_t code)
{
	if (MOD_OPT_BUTTONS != MOD_BUTTONS_JP) {
		return code;
	}
	if (code == 0x817E && COLORCODE_LOWBITS == 5) {
		return code; /* a green △: the field menu's button, △ on both discs */
	}
	switch (code) {
	case 0x81A2: /* × confirms: ○ */
		return 0x819B;
	case 0x817E: /* △ cancels: × */
		return 0x81A2;
	case 0x819B: /* ○, the third button: △ */
		return 0x817E;
	}
	return code;
}

/*
 * The button frames: u 0x4C, v 0xB7 of the UI texture page (320, 0), 4 bpp, CLUT (96, 500); in SYSTEM_W.TIM (at
 * (320, 128)) VRAM words 19..30, rows 55..66. Index 0xC is the cross's light blue, 0xD its dark blue, 3 the face.
 */
#define FRAMES_X	19	/* VRAM words from the TIM's left edge */
#define FRAMES_Y	55	/* rows from its top */
#define FRAMES_WORDS	12	/* 3 frames of 16 texels */
#define FRAMES_ROWS	12
#define INK_LIGHT	0xC
#define INK_DARK	0xD
#define FACE		0x3
#define FRAMES_CLUT_Y	12	/* CLUT (96, 500): row 12 of the TIM's 16 */
/*
 * The ring's CLUT: the frames' one with the cross's two blues made the ○'s red. It lives in the page's filler (u 0x80..0xBF
 * of v 0xE6, all index 15: nothing draws it) at VRAM (352, 230); with the US layout the filler is put back.
 */
#define RING_CLUT_X	352
#define RING_CLUT_Y	230
#define RING_LIGHT	0x359F	/* (248, 96, 104) */
#define RING_DARK	0x1CD4	/* (160, 48, 56) */

static uint16_t FRAMES_CROSS[FRAMES_ROWS * FRAMES_WORDS];
static uint16_t FRAMES_RING[FRAMES_ROWS * FRAMES_WORDS];
static int32_t FRAMES_READY = 0;
static uint16_t RING_CLUT[16];
static uint16_t RING_CLUT_FILLER[16];

/* the ring that takes the cross's 6x5 place: L light, D dark, . the face */
static const char RING[5][7] = {
	".DLLD.",
	"DL..LD",
	"L....L",
	"DL..LD",
	".DLLD.",
};

static int32_t texel(uint16_t *f, int32_t x, int32_t y)
{
	return (f[y * FRAMES_WORDS + x / 4] >> (4 * (x & 3))) & 0xF;
}

static void setTexel(uint16_t *f, int32_t x, int32_t y, int32_t c)
{
	uint16_t *w = &f[y * FRAMES_WORDS + x / 4];

	*w = (*w & ~(0xF << (4 * (x & 3)))) | (c << (4 * (x & 3)));
}

/* FRAMES_RING from FRAMES_CROSS; 0 when a frame's cross is not the 6x5 one this expects */
static int32_t makeRing(void)
{
	int32_t frame, x, y, c, x0, y0, x1, y1;

	for (x = 0; x < FRAMES_ROWS * FRAMES_WORDS; x++) {
		FRAMES_RING[x] = FRAMES_CROSS[x];
	}
	for (frame = 0; frame < 3; frame++) {
		x0 = y0 = 99;
		x1 = y1 = -1;
		for (y = 0; y < FRAMES_ROWS; y++) {
			for (x = frame * 16; x < frame * 16 + 16; x++) {
				c = texel(FRAMES_RING, x, y);
				if (c == INK_LIGHT || c == INK_DARK) {
					setTexel(FRAMES_RING, x, y, FACE);
					x0 = x < x0 ? x : x0;
					y0 = y < y0 ? y : y0;
					x1 = x > x1 ? x : x1;
					y1 = y > y1 ? y : y1;
				}
			}
		}
		if (x1 - x0 != 5 || y1 - y0 != 4) {
			return 0;
		}
		for (y = 0; y < 5; y++) {
			for (x = 0; x < 6; x++) {
				if (RING[y][x] != '.') {
					setTexel(FRAMES_RING, x0 + x, y0 + y, RING[y][x] == 'L' ? INK_LIGHT : INK_DARK);
				}
			}
		}
	}
	return 1;
}

static void uploadFrames(void)
{
	RECT rect;

	if (!FRAMES_READY) {
		return;
	}
	setRECT(&rect, 320 + FRAMES_X, 128 + FRAMES_Y, FRAMES_WORDS, FRAMES_ROWS);
	LoadImage(&rect, (u_long *)(MOD_OPT_BUTTONS == MOD_BUTTONS_JP ? FRAMES_RING : FRAMES_CROSS));
	setRECT(&rect, RING_CLUT_X, RING_CLUT_Y, 16, 1);
	LoadImage(&rect, (u_long *)(MOD_OPT_BUTTONS == MOD_BUTTONS_JP ? RING_CLUT : RING_CLUT_FILLER));
}

uint16_t modButtonClut(void)
{
	if (FRAMES_READY && MOD_OPT_BUTTONS == MOD_BUTTONS_JP) {
		return GetClut(RING_CLUT_X, RING_CLUT_Y);
	}
	return GetClut(96, 500);
}

void modButtonsTimLoaded(GsIMAGE *img)
{
	uint16_t *src;
	int32_t x, y;

	if (img->px != 320 || img->py != 128 || (img->pmode & 7) != 0 || img->pw < FRAMES_X + FRAMES_WORDS ||
	    img->ph < FRAMES_Y + FRAMES_ROWS) {
		return;
	}
	src = (uint16_t *)img->pixel;
	for (y = 0; y < FRAMES_ROWS; y++) {
		for (x = 0; x < FRAMES_WORDS; x++) {
			FRAMES_CROSS[y * FRAMES_WORDS + x] = src[(FRAMES_Y + y) * img->pw + FRAMES_X + x];
		}
	}
	FRAMES_READY = 0;
	if (img->cw < 16 || img->ch <= FRAMES_CLUT_Y || ((img->pmode >> 3) & 1) == 0) {
		return;
	}
	for (x = 0; x < 16; x++) {
		RING_CLUT[x] = ((uint16_t *)img->clut)[FRAMES_CLUT_Y * img->cw + x];
		RING_CLUT_FILLER[x] = src[(RING_CLUT_Y - 128) * img->pw + (RING_CLUT_X - 320) + x];
	}
	RING_CLUT[INK_LIGHT] = RING_LIGHT;
	RING_CLUT[INK_DARK] = RING_DARK;
	FRAMES_READY = makeRing();
	if (MOD_OPT_BUTTONS == MOD_BUTTONS_JP) {
		uploadFrames();
	}
}

void modSetButtons(int32_t layout)
{
	MOD_OPT_BUTTONS = layout == MOD_BUTTONS_JP ? MOD_BUTTONS_JP : MOD_BUTTONS_US;
	uploadFrames();
}

int32_t modFrameRate(void)
{
	return MOD_FPS_MODE == 2 ? 60 : MOD_FPS_MODE == 1 ? 30 : 20;
}

void modSetFrameRate(int32_t fps)
{
#if FPS != 20
	MOD_FPS_MODE_RUN = fps == 60 ? 2 : fps == 30 ? 1 : 0;
#endif
}
