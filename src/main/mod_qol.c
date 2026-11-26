/*
 * MOD (quality of life): what the speed-ups share (docs/mods/quality-of-life.md).
 *
 * Plates. A battle shows the pad's R2 button and fast-forward arrows in the bottom right corner (patches 0184, 0196, 0197),
 * an evolution a line "Press START to skip the animation" at the bottom (0185, 0196), on a plate in the digital clock's
 * style: dark, with a light blue edge. modPlate draws a plate of 3x5 letters; its letters are the ones these need.
 *
 * Training block. After each session of a block (src/trn/trn_block.h), modTrainStopReason says whether the block
 * stops: the partner is hungry, needs the toilet, is sleepy, tired, hurt or sick, is due to digivolve or at the end of
 * its life (the gym's own box, a technique or new orders, is the block's own check).
 */
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#include <dw/types.h>
#include <dw/entity.h>
#include <dw/evl.h>
#include <dw/font.h>
#include <dw/level.h>
#include <dw/mod_qol.h>
#include <dw/mod_present.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/sound.h>
#include <dw/world_object.h>

#include <text/main/mod_qol.h>

#include "common.h"

extern GsOT *ACTIVE_ORDERING_TABLE;
void renderString(int32_t color, int32_t x, int32_t y, int32_t w, int32_t h, int32_t u, int32_t v, int32_t layer,
                  int32_t shadow);
extern uint32_t POLLED_INPUT;
extern uint32_t POLLED_INPUT_PREVIOUS;
extern uint8_t PAUSE_STATE;

/* each row of a letter is 3 bits, bit 2 the left column */
static const char MOD_PLATE_CHARS[] = "R2x15STA>";
static const uint8_t MOD_PLATE_GLYPHS[9][5] = {
	{ 6, 5, 6, 5, 5 }, /* R */
	{ 7, 1, 7, 4, 7 }, /* 2 */
	{ 0, 0, 5, 2, 5 }, /* x */
	{ 2, 6, 2, 2, 7 }, /* 1 */
	{ 7, 4, 7, 1, 7 }, /* 5 */
	{ 7, 4, 7, 1, 7 }, /* S */
	{ 7, 2, 2, 2, 2 }, /* T */
	{ 2, 5, 7, 5, 5 }, /* A */
	{ 4, 6, 7, 6, 4 }, /* > (fast forward) */
};


static void modPlateRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, int32_t g, int32_t b, int32_t z)
{
	POLY_F4 *p;

	p = (POLY_F4 *)GsGetWorkBase();
	SetPolyF4(p);
	setRGB0(p, r, g, b);
	setXYWH(p, x, y, w, h);
	AddPrim(ACTIVE_ORDERING_TABLE->org + z, p++);
	GsSetWorkBase((PACKET *)p);
}

static int32_t modPlateWidth(const char *text)
{
	int32_t w = 5;

	for (; *text != 0; text++) {
		w += *text == ' ' ? 2 : 4;
	}
	return w;
}

/* a letter advances 4 pixels, a space 2; the plate is 9 high and as wide as its text plus 5. A later prim at the same
   depth is drawn under an earlier one, so edge, inside and letters go 6, 5, 4 */
/* the letters alone, their top left at (x, y) */
static void modPlateText(int32_t x, int32_t y, const char *text, int32_t hi, int32_t r, int32_t g, int32_t b)
{
	int32_t px = x;
	int32_t i;
	int32_t row;
	int32_t col;
	int32_t k;

	for (i = 0; text[i] != 0; i++) {
		if (text[i] == ' ') {
			px += 2;
			continue;
		}
		for (k = 0; MOD_PLATE_CHARS[k] != 0 && MOD_PLATE_CHARS[k] != text[i]; k++) {
		}
		for (row = 0; row < 5 && MOD_PLATE_CHARS[k] != 0; row++) {
			for (col = 0; col < 3; col++) {
				if (MOD_PLATE_GLYPHS[k][row] & (4 >> col)) {
					if (i >= hi) {
						modPlateRect(px + col, y + row, 1, 1, r, g, b, 4);
					} else {
						modPlateRect(px + col, y + row, 1, 1, 0xF0, 0xF0, 0xF0, 4);
					}
				}
			}
		}
		px += 4;
	}
}

void modPlate(int32_t x, int32_t y, const char *text, int32_t hi, int32_t r, int32_t g, int32_t b)
{
	int32_t w = modPlateWidth(text);

	modPlateRect(x, y, w, 9, 0x62, 0xB4, 0xF6, 6);
	modPlateRect(x + 1, y + 1, w - 2, 7, 0x18, 0x18, 0x18, 5);
	modPlateText(x + 3, y + 2, text, hi, r, g, b);
}

/* the evolution's skip hint: a line of the game's font (in the language shown), drawn once into a row of the text area
   when the scene begins, then shown each frame on a dark plate at the bottom of the screen, its shadow on */
#define MOD_SKIP_ROW 0xD8
static int16_t MOD_SKIP_W;
static int16_t modSkipState;

void modSkipHintDraw(void)
{
	RECT area;
	char *text = QOL_STR_SKIP_EVOLUTION;

	setRECT(&area, 0, MOD_SKIP_ROW, 256, 12);
	clearTextSubArea(&area);
	setTextColor(1);
	MOD_SKIP_W = drawStringMode(text, 0, MOD_SKIP_ROW, fitStringMode(text, 240));
}

/* each tick of the scene: armed (3, the hint drawn and shown) once START is up and no pause is open, so the START
   that closes a pause (the lab menu evolves under it, and the scene runs under the pause) is not a skip; a pause
   disarms it (its box reuses the text area); START while armed asks for the skip (2) */
void modEvoSkipInput(void)
{
	if (MOD_EVO_SKIP == 3 && PAUSE_STATE != 0) {
		MOD_EVO_SKIP = 1;
	} else if (MOD_EVO_SKIP == 1 && PAUSE_STATE == 0 && !(POLLED_INPUT & 0x800)) {
		MOD_EVO_SKIP = 3;
		modSkipHintDraw();
	} else if (MOD_EVO_SKIP == 3 && (POLLED_INPUT & 0x800)) {
		MOD_EVO_SKIP = 2;
	} else if (MOD_EVO_SKIP == 3 && (EVO_SEQUENCE_DATA.state != modSkipState || (EVO_SEQUENCE_DATA.timer & 15) == 0)) {
		modSkipHintDraw(); /* the scene's own loads reach the text area (the new form's textures): drawn again */
	}
	modSkipState = EVO_SEQUENCE_DATA.state;
}

void modSkipPlate(void)
{
	int32_t x = -MOD_SKIP_W / 2;

	modPlateRect(x - 6, 0x58, MOD_SKIP_W + 12, 18, 0x62, 0xB4, 0xF6, 6);
	modPlateRect(x - 5, 0x59, MOD_SKIP_W + 10, 16, 0x18, 0x18, 0x18, 5);
	renderString(0, x, 0x5B, MOD_SKIP_W, 12, 0, MOD_SKIP_ROW, 4, 1);
}

/* the battle speed of the three battle overlays (BTL field battles, STD arena and tournaments, VS): R2 switches x1 /
   x2, the overlay says before each tick whether the fight is held at x1 (BTL: the item menu, the defeat countdown) */
int8_t MOD_SPEED_FIGHT; /* the plate is up: a fight of STD or VS reads R2 and may run fast */
static int8_t MOD_SPEED_HELD;

void modSpeedBeforeTick(int32_t held)
{
	MOD_SPEED_HELD = held;
	MOD_BATTLE_FAST_TICK = MOD_BATTLE_FAST && !held;
}

/* after the pad is read: R2 of the first pad */
void modSpeedInput(void)
{
	if ((POLLED_INPUT & ~POLLED_INPUT_PREVIOUS) & 2) {
		MOD_BATTLE_FAST ^= 1;
		playSound(0, 2);
	}
}

/* the pad's R2 button, a grey pill, then fast-forward arrows: one white at x1, two yellow at x2, two grey while held
   (owner's choice B of six, 2026-10-24) */
static void modSpeedPlate(int32_t instanceId)
{
	int32_t x = 0x7E;
	int32_t y = 0x60; /* the item menu's bottom box ends just above 0x66 */

	(void)instanceId;
	modPlateRect(x + 2, y, 9, 9, 0xB0, 0xB0, 0xB8, 6);
	modPlateRect(x + 1, y + 1, 11, 7, 0xB0, 0xB0, 0xB8, 6);
	modPlateRect(x, y + 2, 13, 5, 0xB0, 0xB0, 0xB8, 6);
	modPlateRect(x + 2, y + 1, 9, 7, 0x50, 0x50, 0x58, 5);
	modPlateRect(x + 1, y + 2, 11, 5, 0x50, 0x50, 0x58, 5);
	modPlateText(x + 3, y + 2, "R2", 2, 0, 0, 0);
	if (!MOD_BATTLE_FAST) {
		modPlateText(x + 19, y + 2, ">", 1, 0, 0, 0);
	} else if (MOD_SPEED_HELD) {
		modPlateText(x + 16, y + 2, ">>", 0, 0x80, 0x80, 0x80);
	} else {
		modPlateText(x + 16, y + 2, ">>", 0, 0xF8, 0xD8, 0x30);
	}
}

void modSpeedPlateAdd(void)
{
	MOD_SPEED_HELD = 0;
	MOD_SPEED_FIGHT = 1;
	addObject(0x19b, 0, NULL, (RenderFunction)modSpeedPlate);
}

void modSpeedPlateRemove(void)
{
	MOD_SPEED_FIGHT = 0;
	removeObject(0x19b, 0);
}

int32_t modTrainStopReason(int32_t t0, int32_t tiredAt)
{
	int32_t level = DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type).level;
	int32_t d;

	if (PARTNER_PARA.remainingLifetime <= 0) {
		return MOD_TB_LIFE;
	}
	/* the hour its next form is looked for; a Champion or Ultimate past it is looked at every hour, which a session
	   does not hold up */
	if (level >= 1 && level <= LEVEL_ULTIMATE && MOD_EVO_TIMING[level] >= 0 &&
	    PARTNER_PARA.evoTimer >= MOD_EVO_TIMING[level] && (level < 4 || PARTNER_PARA.evoTimer == MOD_EVO_TIMING[level])) {
		return MOD_TB_EVOLVE;
	}
	if (PARTNER_PARA.condition & 0x60) {
		return MOD_TB_UNWELL;
	}
	if ((PARTNER_PARA.condition & 4) || PARTNER_PARA.foodLevel <= 0) {
		return MOD_TB_HUNGRY;
	}
	if ((PARTNER_PARA.condition & 8) || PARTNER_PARA.poopLevel <= 0) {
		return MOD_TB_TOILET;
	}
	/* the session ran from t0 to t0 + 60 minutes: its sleepy time fell in it */
	d = (PARTNER_PARA.sleepyHour * 60 + PARTNER_PARA.sleepyMinute - t0 + 1440) % 1440;
	if ((PARTNER_PARA.condition & 1) || (d > 0 && d <= 60)) {
		return MOD_TB_SLEEPY;
	}
	if (tiredAt >= 0 && PARTNER_PARA.tiredness >= tiredAt) {
		return MOD_TB_TIRED;
	}
	return MOD_TB_GO;
}
