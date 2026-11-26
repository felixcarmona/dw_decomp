/*
 * MOD (quality of life): what the speed-ups share (docs/mods/quality-of-life.md).
 *
 * Plates. A battle shows the pad's R2 button and fast-forward arrows in the bottom right corner (patches 0184, 0196, 0197),
 * an evolution a line "Press START to skip the animation" in the bottom right corner (0185, 0196, 0200, 0201). modPlate
 * draws a plate of 3x5 letters in the digital clock's style; its letters are the ones these need.
 *
 * Training block. After each session of a block (src/trn/trn_block.h), modTrainStopReason says whether the block
 * stops: the partner is hungry, needs the toilet, is sleepy, tired, hurt or sick, is due to digivolve or at the end of
 * its life (the gym's own box, a technique or new orders, is the block's own check).
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#include <dw/types.h>
#include <dw/entity.h>
#include <dw/evl.h>
#include <dw/evolution.h>
#include <dw/font.h>
#include <dw/ids.h>
#include <dw/level.h>
#include <dw/line.h>
#include <dw/mod_options.h>
#include <dw/mod_qol.h>
#include <dw/mod_present.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/sound.h>
#include <dw/pstat.h>
#include <dw/script.h>
#include <dw/sjis.h>
#include <dw/species.h>
#include <dw/world_object.h>

#include <text/main/mod_qol.h>

#include "common.h"

extern GsOT *ACTIVE_ORDERING_TABLE;
void initStringFT4(POLY_FT4 *p);
void renderString(uint8_t color, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t u, uint8_t v, long layer,
		  int32_t shadow);
void setUVDataPolyFT4(POLY_FT4 *prim, int16_t xPos, int16_t yPos, int16_t width, int16_t height);
void setPosDataPolyFT4(POLY_FT4 *prim, int16_t posX, int16_t posY, int16_t width, int16_t height);
extern uint32_t POLLED_INPUT;
extern uint32_t POLLED_INPUT_PREVIOUS;
extern uint8_t PAUSE_STATE;

/* each row of a letter is 3 bits, bit 2 the left column */
static const char MOD_PLATE_CHARS[] = "R2x15STA>0346789/B";
static const uint8_t MOD_PLATE_GLYPHS[][5] = {
	{ 6, 5, 6, 5, 5 }, /* R */
	{ 7, 1, 7, 4, 7 }, /* 2 */
	{ 0, 0, 5, 2, 5 }, /* x */
	{ 2, 6, 2, 2, 7 }, /* 1 */
	{ 7, 4, 7, 1, 7 }, /* 5 */
	{ 7, 4, 7, 1, 7 }, /* S */
	{ 7, 2, 2, 2, 2 }, /* T */
	{ 2, 5, 7, 5, 5 }, /* A */
	{ 4, 6, 7, 6, 4 }, /* > (fast forward) */
	{ 7, 5, 5, 5, 7 }, /* 0 */
	{ 7, 1, 3, 1, 7 }, /* 3 */
	{ 5, 5, 7, 1, 1 }, /* 4 */
	{ 7, 4, 7, 5, 7 }, /* 6 */
	{ 7, 1, 1, 2, 2 }, /* 7 */
	{ 7, 5, 7, 5, 7 }, /* 8 */
	{ 7, 5, 7, 1, 7 }, /* 9 */
	{ 1, 1, 2, 4, 4 }, /* / */
	{ 6, 5, 6, 5, 6 }, /* B */
};


void modPlateRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, int32_t g, int32_t b, int32_t z)
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

/* s049-49/script.py pays by shop_hi = profit / 256: <=6, <=9, else recruit. */
#define MOD_SHOP_MIDDLE 1792
#define MOD_SHOP_RECRUIT 2560
#define MOD_SHOP_PLATE_OBJECT 0x1fb3

static void modShopPlateRender(int32_t instanceId)
{
	char text[20];
	int32_t customer = readPStat(123);
	int32_t profit = SHOP_VARIABLE;
	int32_t w, x, fill;

	if (CURRENT_SCREEN_ID != 49) {
		removeObject(MOD_SHOP_PLATE_OBJECT, 0);
		return;
	}
	if (customer > 20) customer = 20;
	if (profit > 9999) profit = 9999;
	sprintf(text, "%d B  %d/20", profit, customer);
	w = modPlateWidth("9999 B  20/20");
	/* UI primitives use a screen-centred origin. */
	x = 320 - w - 8 - 160;
	modPlateRect(x, 10 - 120, w, 9, 0x62, 0xB4, 0xF6, 6);
	modPlateRect(x + 1, 11 - 120, w - 2, 7, 0x18, 0x18, 0x18, 5);
	modPlateText(x + w - modPlateWidth(text) + 3, 12 - 120, text, 0, 0xF0, 0xF0, 0xF0);
	fill = profit < MOD_SHOP_RECRUIT ? (w - 2) * profit / MOD_SHOP_RECRUIT : w - 2;
	modPlateRect(x, 18 - 120, w, 6, 0x62, 0xB4, 0xF6, 6);
	modPlateRect(x + 1, 19 - 120, w - 2, 4, 0x18, 0x18, 0x18, 5);
	if (fill > 0) modPlateRect(x + 1, 19 - 120, fill, 4, 0x2E, 0x6C, 0xD8, 4);
	modPlateRect(x + 1 + (w - 2) * MOD_SHOP_MIDDLE / MOD_SHOP_RECRUIT,
		     18 - 120, 1, 6, 0xF0, 0xF0, 0xF0, 3);
}

void modShopPlateAdd(void)
{
	/* The object is the arm: p123 also stays at 20 between shifts. */
	removeObject(MOD_SHOP_PLATE_OBJECT, 0);
	addObject(MOD_SHOP_PLATE_OBJECT, 0, NULL, modShopPlateRender);
}

/* the evolution's cancel (EVL overlay, src/evl/evl.c): bit 0 says the evolution that is starting came from the hours
   and the requirements (or is the Rookie's fallback) and may be cancelled; bit 1 that one was cancelled, so none is
   offered again until the partner has slept. The hint's line lives here, where the language's text is switched */
int8_t MOD_EVO_CANCEL;
char MOD_EVO_CANCEL_TEXT[] = QOL_STR_EVO_CANCEL;

/* the cancel's button, bottom left, worked out a pixel at a time from its centre (squared distances): a dark disc, a
   black gap, a band from 6.6 to 9.5 pixels that fills clockwise from the top (yellow, white once full) over a dark
   track, a black edge round the outside, and the cancel button inside as an outline (a green triangle, with
   JAPANESE CONTROLS a blue cross), brighter while held. Each row goes out as runs of one colour; the line beside it
   is the text area's row the EVL overlay drew, dim, white while held */
#define EVO_CANCEL_X (-141)
#define EVO_CANCEL_Y 104
static int32_t modEvoCancelPixel(int32_t x, int32_t y, int32_t hold, int32_t ticks)
{
	int32_t ax = x < 0 ? -x : x;
	int32_t ay = y < 0 ? -y : y;
	int32_t d = x * x + y * y;
	int32_t hw = (y + 4) >> 1;

	if (MOD_OPT_BUTTONS ? (ax == ay && ax <= 2) : (y >= -3 && y <= 2 && (ax == hw || (y == 2 && ax <= 3)))) {
		if (MOD_OPT_BUTTONS) {
			return hold ? 0x68a8ff : 0x4078d8;
		}
		return hold ? 0x48e8b8 : 0x30a888;
	}
	if (d <= 31) {
		return 0x2c2c34;
	}
	if (d <= 43) {
		return 0;
	}
	if (d <= 90) {
		if (((ratan2(x, -y) + 4096) & 4095) * ticks < hold * 4096) {
			return hold >= ticks ? 0xffffff : 0xffcc28;
		}
		return 0x383840;
	}
	ax = ax ? ax - 1 : 0;
	ay = ay ? ay - 1 : 0;
	return ax * ax + ay * ay <= 90 ? 0 : -1;
}

void modEvoCancelDraw(int32_t hold, int32_t ticks, int32_t w, int32_t row)
{
	int32_t x, y, c, prev, start;

	for (y = -11; y <= 11; y++) {
		prev = -1;
		start = 0;
		for (x = -11; x <= 12; x++) {
			c = x > 11 ? -1 : modEvoCancelPixel(x, y, hold, ticks);
			if (c != prev) {
				if (prev >= 0) {
					modPlateRect(EVO_CANCEL_X + start, EVO_CANCEL_Y + y, x - start, 1, prev >> 16, (prev >> 8) & 0xff,
					             prev & 0xff, 5);
				}
				prev = c;
				start = x;
			}
		}
	}
	renderString(hold ? 0 : 8, EVO_CANCEL_X + 15, EVO_CANCEL_Y - 6, w, 12, 0, row, 4, 1);
}

/* the tallies' skip hint (patch 0333): "Press × to skip" in a row of its own at the bottom of the box that counts (the
   gym's stats box, the battle's bits box), under a divider like the battle box's. The line is drawn once into a row of
   the text area when the box opens, its button in the colour of the button that confirms: × blue (6), with JAPANESE
   CONTROLS the font draws it as ○ (modButtonGlyph), red (3) */
#define MOD_COUNT_ROW 0xD8
#define MOD_COUNT_MAX_W 168
static int16_t MOD_COUNT_W;

void modCountHintDraw(void)
{
	RECT area;
	char *text = QOL_STR_SKIP_COUNT;
	uint8_t colors[48];
	char *s;
	int32_t n;
	int32_t mode;

	for (s = text, n = 0; *s != '\0' && n < 48; n++) {
		if (isAsciiEncoded(s)) {
			colors[n] = 1;
			s++;
		} else {
			colors[n] = ((uint8_t)s[0] == 0x81 && (uint8_t)s[1] == 0xA2) ? (MOD_OPT_BUTTONS == MOD_BUTTONS_JP ? 3 : 6) : 1;
			s += 2;
		}
	}
	setRECT(&area, 0, MOD_COUNT_ROW, 256, 12);
	clearTextSubArea(&area);
	mode = fitStringMode(text, MOD_COUNT_MAX_W);
	if (mode == FIT_VANILLA) {
		mode = FIT_TIGHT; /* drawString would take the text's colour: the strip takes one per glyph */
	}
	MOD_COUNT_W = drawFittedRow(text, MOD_COUNT_ROW, mode, colors);
	setTextColor(1);
}

void modCountHintRow(int32_t x, int32_t w, int32_t y, int32_t show, int32_t layer)
{
	drawLine2P(0x8e8e8e, x + 2, y, x + w - 4, y, layer, 0);
	drawLine2P(0x121212, x + 3, y + 1, x + w - 3, y + 1, layer, 0);
	if (show) {
		renderString(0, x + (w - MOD_COUNT_W) / 2, y + 4, MOD_COUNT_W, 12, 0, MOD_COUNT_ROW, layer, 0);
	}
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
	/* Preview the tick's actual target. Champion/Ultimate stops keep their first-hour window;
	   later field checks still run every hour. No random special-evolution rules are previewed. */
	if (level >= 1 && level <= LEVEL_ULTIMATE && MOD_EVO_TIMING[level] >= 0 &&
	    PARTNER_PARA.evoTimer >= MOD_EVO_TIMING[level] && (level < 4 || PARTNER_PARA.evoTimer == MOD_EVO_TIMING[level]) &&
	    getNaturalEvolutionTarget(PARTNER_ENTITY.digimonEntity.entity.type) != -1) {
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

/*
 * MOD (partner rewind): the line of the partner. Each form of this egg writes the state it arrived with (species,
 * base stats, lifetime, age, weight) on top of MOD_PARTNER_LINE; the save machine of Jijimon's houses, in a New Game
 * Plus run, can rewind the partner: the top entry goes and the one under it is the partner again, exactly as it was
 * when it arrived, with its techniques kept. The line keeps two forms (the save has room for no more), so a rewind goes
 * one form back. An egg may rewind as many times as the save has started New Game Plus runs (pstat 20). Sukamon and the
 * King of Sukamon's way back are not written: the top entry is then not the current form, and the machine says it does
 * not remember. Only what must be resident is here; the evolution side is in the EVL overlay (evl.c).
 */
extern int16_t EVOLUTION_TARGET;
ModPartnerLine MOD_PARTNER_LINE;

static void modPartnerLineFill(ModLineEntry *e)
{
	Stats *s = &PARTNER_ENTITY.digimonEntity.stats;

	e->species = PARTNER_ENTITY.digimonEntity.entity.type;
	e->hp = s->base.hp;
	e->mp = s->base.mp;
	e->off = s->base.off;
	e->def = s->base.def;
	e->speed = s->base.speed;
	e->brain = s->base.brain;
	e->lifetime = PARTNER_PARA.remainingLifetime;
	e->age = PARTNER_PARA.age;
	e->weight = PARTNER_PARA.weight;
}

/* oldType -1: a new egg (a new game or a rebirth), whose line starts with the form it hatched as. Otherwise the end of
 * EVL_applyEvolution (not a rewind): the form the partner arrived at is written on top of the forms before it; a full
 * line drops its oldest. Turning into Sukamon is not written, and neither is his way back */
void modPartnerLinePush(int32_t oldType)
{
	ModPartnerLine *l = &MOD_PARTNER_LINE;

	if (oldType < 0) {
		l->count = l->used = l->rewound = l->pending = 0;
	} else if (PARTNER_ENTITY.digimonEntity.entity.type == DIGIMON_SUKAMON || oldType == DIGIMON_SUKAMON) {
		return;
	}
	if (l->count >= MOD_LINE_MAX) {
		l->count = MOD_LINE_MAX - 1;
	}
	memmove(&l->entry[1], &l->entry[0], (MOD_LINE_MAX - 1) * sizeof(ModLineEntry));
	modPartnerLineFill(&l->entry[0]);
	l->count++;
}

/* digimon routines 13 (start 0) and 14 (start 1). 13: p254 = 0 the machine does not remember an earlier form, 1 none
 * left, 2 it can, 3 it can and this egg has not gone back yet; p253 = how many this egg has left (read only on 2 and
 * 3). 14: the rewind's evolution scene starts into the form before (p254 = 1) or, when 13 would refuse, does not (0) */
void modPartnerLineQuery(int32_t start)
{
	ModPartnerLine *l = &MOD_PARTNER_LINE;
	int32_t left = readPStat(20) - l->used;
	int32_t r = 0;

	if (l->count >= 2 && l->entry[0].species == PARTNER_ENTITY.digimonEntity.entity.type) {
		r = left <= 0 ? 1 : l->used ? 2 : 3;
	}
	if (start) {
		r = r >= 2;
		if (r) {
			l->pending = 1;
			l->count--; /* the current form goes: the one before is entry 0, for EVL_applyEvolution */
			memmove(&l->entry[0], &l->entry[1], (MOD_LINE_MAX - 1) * sizeof(ModLineEntry));
			EVOLUTION_TARGET = l->entry[0].species;
			partnerSetState(13);
		}
	}
	writePStat(PSTAT_253, left);
	writePStat(PSTAT_254, r);
}

/* the line as save record SAVE_TAG_PARTNER_LINE (main_menu.c): count, used and rewound, then every entry's fields
 * packed low bit first in the widths below, which hold caps.h's limits, a lifetime and an age or weight up to 255 */
static const uint8_t lineBits[10] = {10, 15, 15, 14, 14, 14, 14, 15, 8, 8};

void modPartnerLineRecord(uint8_t *rec, int32_t load)
{
	ModPartnerLine *l = &MOD_PARTNER_LINE;
	int16_t *v = &l->entry[0].species;
	int32_t i, j, b, w, x, pos = 24;

	if (load) {
		memset(l, 0, sizeof(*l));
		if (rec[0] > MOD_LINE_MAX) {
			return;
		}
		l->count = rec[0];
		l->used = rec[1];
		l->rewound = rec[2];
	} else {
		memset(rec, 0, MOD_LINE_RECORD);
		rec[0] = l->count;
		rec[1] = l->used;
		rec[2] = l->rewound;
	}
	for (i = 0; i < MOD_LINE_MAX; i++) {
		for (j = 0; j < 10; j++, v++) {
			w = lineBits[j];
			x = load ? 0 : *v;
			for (b = 0; b < w; b++, pos++) {
				if (load) {
					x |= (rec[pos >> 3] >> (pos & 7) & 1) << b;
				} else {
					rec[pos >> 3] |= (x >> b & 1) << (pos & 7);
				}
			}
			*v = x;
		}
	}
}
