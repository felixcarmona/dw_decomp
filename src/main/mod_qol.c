/*
 * MOD (quality of life): what the speed-ups share (docs/mods/quality-of-life.md).
 *
 * Plates. A battle (R2 x1 / x2, patch 0184), an evolution that can be skipped (START, 0185) and a gym session (R2 x1 /
 * x5, 0186) show a small plate in the bottom right corner in the digital clock's style: 3x5 letters on a dark plate
 * with a light blue edge. modPlate draws one; its letters are the ones these plates need.
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
#include <dw/level.h>
#include <dw/mod_qol.h>
#include <dw/params.h>
#include <dw/partner.h>

#include "common.h"

extern GsOT *ACTIVE_ORDERING_TABLE;

/* each row of a letter is 3 bits, bit 2 the left column */
static const char MOD_PLATE_CHARS[] = "R2x15STA";
static const uint8_t MOD_PLATE_GLYPHS[8][5] = {
	{ 6, 5, 6, 5, 5 }, /* R */
	{ 7, 1, 7, 4, 7 }, /* 2 */
	{ 0, 0, 5, 2, 5 }, /* x */
	{ 2, 6, 2, 2, 7 }, /* 1 */
	{ 7, 4, 7, 1, 7 }, /* 5 */
	{ 7, 4, 7, 1, 7 }, /* S */
	{ 7, 2, 2, 2, 2 }, /* T */
	{ 2, 5, 7, 5, 5 }, /* A */
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

/* a letter advances 4 pixels, a space 2; the plate is 9 high and as wide as its text plus 5. A later prim at the same
   depth is drawn under an earlier one, so edge, inside and letters go 6, 5, 4 */
void modPlate(int32_t x, int32_t y, const char *text, int32_t hi, int32_t r, int32_t g, int32_t b)
{
	const char *t;
	int32_t w = 5;
	int32_t px;
	int32_t i;
	int32_t row;
	int32_t col;
	int32_t k;

	for (t = text; *t != 0; t++) {
		w += *t == ' ' ? 2 : 4;
	}
	modPlateRect(x, y, w, 9, 0x62, 0xB4, 0xF6, 6);
	modPlateRect(x + 1, y + 1, w - 2, 7, 0x18, 0x18, 0x18, 5);
	px = x + 3;
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
						modPlateRect(px + col, y + 2 + row, 1, 1, r, g, b, 4);
					} else {
						modPlateRect(px + col, y + 2 + row, 1, 1, 0xF0, 0xF0, 0xF0, 4);
					}
				}
			}
		}
		px += 4;
	}
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
