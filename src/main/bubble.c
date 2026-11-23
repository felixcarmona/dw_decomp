#include <libgs.h>

#include <dw/mod_options.h>
#include <dw/btl.h> /* MOD (care menu): ACTIVE_ORDERING_TABLE for the X badge */
#include <dw/bubble.h>
#include <dw/clock.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/script.h>
#include <dw/world_object.h>
#include <dw/math.h>

#define NUM_CONDITION_BUBBLES	3

typedef struct {
	int16_t duration;
	int16_t nextIcon;
	int16_t icon;
} ConditionIcon;

typedef struct {
	int8_t frame[19];
} ConditionFramesHungry;

typedef struct {
	int8_t frame[13];
} ConditionFramesPoopy;

typedef struct {
	int8_t frame[20];
} ConditionFramesInjured;

typedef struct {
	int8_t frame[19];
} ConditionFramesEvolving;

typedef struct {
	int16_t frameCount;
	int16_t activeHeight;
	int16_t baseHeight;
	int16_t frameCount2;
	ConditionIcon *iconPtr;
	uint32_t scale;
	uint32_t scale2;
	Entity *entity;
} ConditionBubble;

void setInt16WithStride(int16_t* ptr, int16_t value, int32_t count,
			int32_t stride);
int32_t lerp(int32_t start, int32_t end, int32_t tMin, int32_t tMax,
	     int32_t tCurrent);
int32_t worldPosToScreenPos(SVECTOR* worldPos, DVECTOR* screenPos);
void translateConditionFXToEntity(Entity *entity, SVECTOR *out);
void renderSprite(GsSPRITE* sprite, int32_t x, int32_t y, int32_t distance,
		  int32_t scaleX, int32_t scaleY);

extern int32_t VIEWPORT_DISTANCE;
extern int32_t DRAWING_OFFSET_Y;
extern int32_t IS_IN_MENU;
extern int8_t GAME_STATE;
extern int8_t PARTNER_STATE;
extern int32_t CONDITION_BUBBLE_ID;
extern int32_t CONDITION_BUBBLE_TYPE;

GsSPRITE CONDITION_SPRITES[12] = {
	/* 0: Hungry */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x3F,		/* tpage */
		0x90,		/* u */
		0x80,		/* v */
		0x110,		/* cx */
		0x1E3,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x1A,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 1: Poopy */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x3F,		/* tpage */
		0xD0,		/* u */
		0x90,		/* v */
		0x110,		/* cx */
		0x1E3,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x1A,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 2: Dying (unused) */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x3F,		/* tpage */
		0xE0,		/* u */
		0xB0,		/* v */
		0x110,		/* cx */
		0x1E2,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x1A,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 3: Sleepy, low Z */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x8,		/* w */
		0x8,		/* h */
		0x3F,		/* tpage */
		0x80,		/* u */
		0xB0,		/* v */
		0x110,		/* cx */
		0x1E2,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x0,		/* mx */
		0xD,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 4: Sleepy, middle Z */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x8,		/* w */
		0x8,		/* h */
		0x3F,		/* tpage */
		0x98,		/* u */
		0x90,		/* v */
		0x110,		/* cx */
		0x1E2,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x13,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 5: Sleepy, high Z */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x8,		/* w */
		0x8,		/* h */
		0x3F,		/* tpage */
		0x90,		/* u */
		0x90,		/* v */
		0x110,		/* cx */
		0x1E2,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x3,		/* mx */
		0x1C,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 6: Tired */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x3F,		/* tpage */
		0xC0,		/* u */
		0x80,		/* v */
		0x110,		/* cx */
		0x1E1,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x17,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 7: Sick */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x3F,		/* tpage */
		0xC0,		/* u */
		0x90,		/* v */
		0x110,		/* cx */
		0x1E1,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x1A,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 8: Injured */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x3F,		/* tpage */
		0xD0,		/* u */
		0xA0,		/* v */
		0x110,		/* cx */
		0x1E3,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x1A,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 9: Evolving */
	{
		0x50000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x3F,		/* tpage */
		0x80,		/* u */
		0xB0,		/* v */
		0x110,		/* cx */
		0x1E2,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x1A,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 10: Bubble bursting */
	{
		0x40000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x10,		/* w */
		0x10,		/* h */
		0x5F,		/* tpage */
		0xD0,		/* u */
		0xB0,		/* v */
		0x110,		/* cx */
		0x1E4,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x8,		/* mx */
		0x8,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
	/* 11: Bubble */
	{
		0x40000000,	/* attribute */
		0x0,		/* x */
		0x0,		/* y */
		0x20,		/* w */
		0x20,		/* h */
		0x5F,		/* tpage */
		0xA0,		/* u */
		0x90,		/* v */
		0x110,		/* cx */
		0x1E4,		/* cy */
		0x80,		/* r */
		0x80,		/* g */
		0x80,		/* b */
		0x10,		/* mx */
		0x1F,		/* my */
		0x0,		/* scalex */
		0x0,		/* scaley */
		0x0,		/* rotate */
	},
};

GsSPRITE *CONDITION_SPRITES_NONE[] = { NULL };

GsSPRITE *CONDITION_SPRITES_HUNGRY[] = {
	&CONDITION_SPRITES[0], NULL,
};

GsSPRITE *CONDITION_SPRITES_POOPY[] = {
	&CONDITION_SPRITES[1], NULL,
};

GsSPRITE *CONDITION_SPRITES_DYING[] = {
	&CONDITION_SPRITES[2], NULL,
};

GsSPRITE *CONDITION_SPRITES_SLEEPY_0[] = {
	&CONDITION_SPRITES[3], NULL,
};

GsSPRITE *CONDITION_SPRITES_SLEEPY_1[] = {
	&CONDITION_SPRITES[3], &CONDITION_SPRITES[4], NULL,
};

GsSPRITE *CONDITION_SPRITES_SLEEPY_2[] = {
	&CONDITION_SPRITES[3], &CONDITION_SPRITES[4],
	&CONDITION_SPRITES[5], NULL,
};

GsSPRITE *CONDITION_SPRITES_SLEEPY_3[] = {
	&CONDITION_SPRITES[4], &CONDITION_SPRITES[5], NULL,
};

GsSPRITE *CONDITION_SPRITES_SLEEPY_4[] = {
	&CONDITION_SPRITES[5], NULL,
};

GsSPRITE *CONDITION_SPRITES_SICK[] = {
	&CONDITION_SPRITES[7], NULL,
};

GsSPRITE *CONDITION_SPRITES_INJURED[] = {
	&CONDITION_SPRITES[8], NULL,
};

GsSPRITE **CONDITION_SPRITES_ICON[12] = {
	CONDITION_SPRITES_NONE,
	CONDITION_SPRITES_HUNGRY,
	CONDITION_SPRITES_POOPY,
	CONDITION_SPRITES_DYING,
	CONDITION_SPRITES_SLEEPY_0,
	CONDITION_SPRITES_SLEEPY_1,
	CONDITION_SPRITES_SLEEPY_2,
	CONDITION_SPRITES_SLEEPY_3,
	CONDITION_SPRITES_SLEEPY_4,
	NULL,
	CONDITION_SPRITES_SICK,
	CONDITION_SPRITES_INJURED,
};

ConditionFramesHungry CONDITION_HUNGRY_FRAMES = {
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3 }
};

ConditionFramesPoopy CONDITION_POOPY_FRAMES = {
	{ 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 1 }
};

ConditionFramesInjured CONDITION_INJURED_FRAMES = {
	{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0 }
};

ConditionFramesEvolving CONDITION_EVOLVING_FRAMES = {
	{ 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3 }
};

ConditionIcon CONDITION_ICON_HUNGRY = { 100, 0, 1 };
ConditionIcon CONDITION_ICON_POOPY = { 100, 0, 2 };
ConditionIcon CONDITION_ICON_DYING = { 100, 0, 3 };
ConditionIcon CONDITION_ICON_TIRED = { 1000, 0, 9 };
ConditionIcon CONDITION_ICON_SICK = { 100, 0, 10 };
ConditionIcon CONDITION_ICON_INJURED = { 100, 0, 11 };
ConditionIcon CONDITION_ICON_EVOLVING = { 100, 0, 12 };

int16_t CONDITION_TIRED_FRAME_U[14] = {
	0, 0, 16, 16, 32, 32, 48, 48, 48, 48, 48, 48, 48, 48,
};

int16_t CONDITION_TIRED_FRAME_MY[14] = {
	9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 6, 3, 0, -6,
};

ConditionIcon CONDITION_ICON_SLEEPY[7] = {
	{ 10, 1, 0 },
	{ 4, 1, 4 },
	{ 4, 1, 5 },
	{ 8, 1, 6 },
	{ 1, 1, 7 },
	{ 1, 1, 8 },
	{ 1, -5, 0 },
};

ConditionIcon *CONDITION_BUBBLE_TYPES[8] = {
	&CONDITION_ICON_HUNGRY,
	&CONDITION_ICON_POOPY,
	&CONDITION_ICON_SICK,
	CONDITION_ICON_SLEEPY,
	&CONDITION_ICON_TIRED,
	&CONDITION_ICON_DYING,
	&CONDITION_ICON_INJURED,
	&CONDITION_ICON_EVOLVING,
};

ConditionBubble CONDITION_BUBBLES[3];

void tickConditionRingObject(int32_t instanceId);
void renderConditionRingObject(int32_t instanceId);

void initializeConditionBubbles(void)
{
	setInt16WithStride((int16_t *)CONDITION_BUBBLES, -1,
			   NUM_CONDITION_BUBBLES, sizeof(ConditionBubble));
	removeObject(0x819, 0);
	addObject(0x819, 0, tickConditionRingObject, renderConditionRingObject);
}

int32_t addConditionBubble(int32_t type, Entity *entity)
{
	ConditionBubble *bubble;
	int32_t i;

	for (i = 0; i < NUM_CONDITION_BUBBLES; ++i) {
		if (CONDITION_BUBBLES[i].frameCount < 0) {
			break;
		}
	}

	if (i == NUM_CONDITION_BUBBLES) {
		return -1;
	}

	bubble = &CONDITION_BUBBLES[i];
	bubble->frameCount = 0;
	bubble->baseHeight = -DIGI_PARA(entity->type).height + 20;
	bubble->activeHeight = bubble->baseHeight;
	bubble->entity = entity;
	bubble->scale = 0;
	bubble->scale2 = 0;
	bubble->frameCount2 = 0;
	bubble->iconPtr = CONDITION_BUBBLE_TYPES[type];
	addObject(0x802, i, tickConditionBubble, renderConditionBubble);

	return i;
}

static void drawConditionIcon(int32_t icon, int32_t fc, int32_t x, int32_t y,
			      int32_t depth, uint32_t scale)
{
	GsSPRITE **sprites;

	switch (icon) {
	case 13:
	case 14: {
		/* Depleted native HUD heart/orb, derived into the existing ETCTIM upload. */
		GsSPRITE gauge = CONDITION_SPRITES[0];
		gauge.u = 224;
		gauge.v = 192 + (icon - 13) * 16;
		gauge.w = 11;
		gauge.h = 11;
		gauge.mx = 5;
		gauge.my = 16;
		gauge.tpage = 0x1f;
		gauge.cx = 272;
		gauge.cy = 493 + icon - 13;
		/* The 11 native texels span 19.25 within the unchanged 26-texel circle. */
		renderSprite(&gauge, x, y, depth, scale * 7 / 4, scale * 7 / 4);
		break;
	}
	case 9: {
		int32_t tiredIndex = fc % 13;

		CONDITION_SPRITES[6].u = CONDITION_TIRED_FRAME_U[tiredIndex] + 0xc0;
		CONDITION_SPRITES[6].my = CONDITION_TIRED_FRAME_MY[tiredIndex] + 0x17;
		renderSprite(&CONDITION_SPRITES[6], x, y,
			     depth, scale, scale);
		break;
	}
	case 1: {
		int32_t hungryIndex = fc % 19;
		ConditionFramesHungry frames = CONDITION_HUNGRY_FRAMES;

		CONDITION_SPRITES[0].u = (frames.frame[hungryIndex] << 4) + 0x90;
		if (frames.frame[hungryIndex] != 3) {
			CONDITION_SPRITES[0].v = 0x80;
		} else {
			CONDITION_SPRITES[0].v = 0xb0;
		}
		CONDITION_SPRITES[0].my = 0x1a;
		if (((hungryIndex + 1) % 5) == 0) {
			CONDITION_SPRITES[0].my += 3;
		}
		renderSprite(&CONDITION_SPRITES[0], x, y,
			     depth, scale, scale);
		break;
	}
	case 2: {
		int32_t poopyIndex = fc % 13;
		ConditionFramesPoopy frames = CONDITION_POOPY_FRAMES;

		CONDITION_SPRITES[1].u = (frames.frame[poopyIndex] << 4) + 0xd0;
		renderSprite(&CONDITION_SPRITES[1], x, y,
			     depth, scale, scale);
		break;
	}
	case 3:
		CONDITION_SPRITES[2].u = (((fc / 15) & 1) << 4) + 0xe0;
		renderSprite(&CONDITION_SPRITES[2], x, y,
			     depth, scale, scale);
		break;
	case 10:
		CONDITION_SPRITES[7].v = (((fc / 20) & 1) << 4) + 0x90;
		renderSprite(&CONDITION_SPRITES[7], x, y,
			     depth, scale, scale);
		break;
	case 11: {
		int32_t injuredIndex = fc % 20;
		ConditionFramesInjured frames = CONDITION_INJURED_FRAMES;

		CONDITION_SPRITES[8].u = (frames.frame[injuredIndex] << 4) + 0xd0;
		renderSprite(&CONDITION_SPRITES[8], x, y,
			     depth, scale, scale);
		break;
	}
	case 12: {
		int32_t evolvingIndex = fc % 19;
		ConditionFramesEvolving frames = CONDITION_EVOLVING_FRAMES;

		CONDITION_SPRITES[9].u = (frames.frame[evolvingIndex] << 4) + 0x80;
		if (frames.frame[evolvingIndex] == 0) {
			CONDITION_SPRITES[9].v = 0xb8;
			CONDITION_SPRITES[9].my = 0x12;
			CONDITION_SPRITES[9].h = 8;
		} else {
			CONDITION_SPRITES[9].v = 0xb0;
			CONDITION_SPRITES[9].my = 0x1a;
			CONDITION_SPRITES[9].h = 0x10;
		}
		renderSprite(&CONDITION_SPRITES[9], x, y,
			     depth, scale, scale);
		break;
	}
	default:
		sprites = CONDITION_SPRITES_ICON[icon];
		while (*sprites != NULL) {
			renderSprite(*sprites, x, y, depth,
				     scale, scale);
			++sprites;
		}
		break;
	}

}

int32_t MOD_STATUS_BUBBLES = 1;

/*
 * MOD (status-bubbles): the ACTIVE conditions as a fixed row of equal bubbles
 * over the partner, in a fixed order (hungry, poopy, sleepy, tired, sick,
 * injured, HP, MP): one or two in one row, from three on in two rows (the first half
 * on top). Only a bubble on its own has a tail. Nothing rotates: a bubble keeps its
 * place while its condition lasts; when one appears it grows in its place,
 * when one goes it shrinks, and the others slide to their new places. The
 * layout is a function of (anchor, active conditions), never of the bubbles'
 * own positions, so the cluster cannot move.
 *
 * One state machine: each condition has a slot it rests in (FROM) and, during
 * a step, the slot it goes to (TO). A step lasts RING_STEP_TICKS ticks and is
 * drawn with the render pass's fraction of the tick (MOD_RENDER_ALPHA), eased
 * in and out, so it is smooth at 60 fps. Slots are only reassigned when a
 * step ends; a step is never cancelled: what changes meanwhile is looked at
 * when it ends.
 * Stored in .data (initialised) so that the linker keeps them.
 */
#define RING_STEP_TICKS	10	/* ticks per step: 0.5 s */
#define RING_SIZE	205	/* every bubble, circle and icon: 0.8 of the original's (256) */
#define RING_COL	22	/* texels between the centres of a row (a 0.8 circle is 21 wide) */
#define RING_ROW	18	/* texels from the bottom row up to the top one */
#define RING_NONE	0x10000	/* slot codes: no bubble, else (x + 128) << 8 | (y + 128), texels, */
#define RING_TAIL	0x20000	/* | this when the bubble has its tail */

/* conditions in a fixed order: hungry, poopy, sleepy, tired, sick, injured, HP, MP */
static const uint8_t RING_BIT[8] = { 0x04, 0x08, 0x01, 0x02, 0x40, 0x20, 0, 0 };
static const uint8_t RING_ICON[8] = { 1, 2, 4, 9, 10, 11, 13, 14 };

int32_t MOD_RING_CLOCK = 1;
int32_t MOD_RING_T = -1;	/* ticks into the running step, -1 = at rest */
int32_t MOD_RING_STEPS = 0;	/* steps started (for tools/bubblecheck.py) */
int32_t MOD_RING_FROM[8] = { RING_NONE, RING_NONE, RING_NONE,
			     RING_NONE, RING_NONE, RING_NONE, RING_NONE, RING_NONE };
int32_t MOD_RING_TO[8] = { RING_NONE, RING_NONE, RING_NONE,
			   RING_NONE, RING_NONE, RING_NONE, RING_NONE, RING_NONE };
/* what the last render pass drew, one word each (tools/bubblecheck.py):
 * [0] passes drawn, [1] anchor before the bob (x | y << 16, screen pixels),
 * [2] biggest bubble: condition << 24 | shown size (0..256) << 8 | 1 if a
 * second bubble is above 90 % too, [3] step phase (0..4096) | steps << 16,
 * [4] the anchor as drawn (bob and top clamp included) */
int32_t MOD_RING_SHOWN[5] = { 0, 0, 0, 0, 0 };
extern int32_t MOD_RENDER_ALPHA;

static int32_t ringSlotSize(int32_t slot)
{
	return slot == RING_NONE ? 0 : RING_SIZE;
}

/* x, y in 1/16 texel from the anchor; the tail's height 0..256 */
static void ringSlotPos(int32_t slot, int32_t *x, int32_t *y, int32_t *tail)
{
	if (slot == RING_NONE) {
		*x = 0;
		*y = 0;
		*tail = 0;
		return;
	}
	*x = (((slot >> 8) & 0xFF) - 128) * 16;
	*y = ((slot & 0xFF) - 128) * 16;
	*tail = (slot & RING_TAIL) ? 256 : 0;
}

/* ease in and out: 3p^2 - 2p^3, p and the result in 0..4096 */
static int32_t ringEase(int32_t p)
{
	if (p <= 0) {
		return 0;
	}
	if (p >= 4096) {
		return 4096;
	}
	return (((p * p) >> 12) * (3 * 4096 - 2 * p)) >> 12;
}

/* the step's phase this render pass shows: 0..4096 (0 at rest) */
static int32_t ringPhase(void)
{
	int32_t p;

	if (MOD_RING_T < 0) {
		return 0;
	}
	p = ((MOD_RING_T - 1) * 4096 + MOD_RENDER_ALPHA) / RING_STEP_TICKS;
	return p < 0 ? 0 : (p > 4096 ? 4096 : p);
}

/* where condition j is drawn at phase p: x, y in 1/16 texel from the anchor,
 * size 0..256, tail height 0..256 */
static void ringBubbleAt(int32_t j, int32_t p, int32_t *x, int32_t *y,
			 int32_t *size, int32_t *tail)
{
	int32_t from = MOD_RING_FROM[j];
	int32_t to = MOD_RING_T < 0 ? from : MOD_RING_TO[j];
	int32_t x0, y0, t0, x1, y1, t1, e;

	ringSlotPos(from, &x0, &y0, &t0);
	ringSlotPos(to, &x1, &y1, &t1);
	if (from == to) {
		*x = x0;
		*y = y0;
		*tail = t0;
		*size = ringSlotSize(from);
		return;
	}
	e = ringEase(p);
	*size = ringSlotSize(from) + (((ringSlotSize(to) - ringSlotSize(from)) * e) >> 12);
	if (from == RING_NONE) {
		*x = x1;
		*y = y1;
		*tail = t1;
	} else if (to == RING_NONE) {
		*x = x0;
		*y = y0;
		*tail = t0;
	} else {
		*x = x0 + (((x1 - x0) * e) >> 12);
		*y = y0 + (((y1 - y0) * e) >> 12);
		*tail = t0 + (((t1 - t0) * e) >> 12);
	}
}

/* one bubble: its circle centred on (cx, cy), the icon at the circle's size,
 * the tail under it with the given height */
static void drawRingBubble(int32_t j, int32_t cx, int32_t cy, int32_t depth,
			   uint32_t scale, int32_t size, int32_t tail, int32_t k)
{
	GsSPRITE *bg = &CONDITION_SPRITES[11];
	uint32_t s = (scale * size) >> 8;
	int32_t kk = (k * size) >> 8; /* screen pixels per texel, 4096 = 1 */
	int32_t icon = RING_ICON[j];

	if (icon == 4) {
		icon = 4 + ((MOD_RING_CLOCK / 8) % 5);
	}
	/* icon pivots are 18 texels under the circle's centre (in the icon's
	 * own scale, so that it stays centred) */
	drawConditionIcon(icon, MOD_RING_CLOCK, cx, cy + ((18 * kk) >> 12),
			  depth, s);
	bg->h = 26;
	bg->my = 13;
	renderSprite(bg, cx, cy, depth, s, s);
	if (tail > 0) {
		bg->v += 26;
		bg->h = 6;
		bg->my = 0;
		renderSprite(bg, cx, cy + ((13 * kk) >> 12), depth, s,
			     (s * tail) >> 8);
		bg->v -= 26;
	}
	bg->h = 32;
	bg->my = 0x1f;
}

/*
 * MOD (care menu): the Square badge. While Square would open the care menu (mod_feed.c sets
 * MOD_FEED_HINT for that tick), the care-only native button cap (the UI texture page,
 * three 16x12 frames pressing it) is drawn beside the bottom right bubble of the row, or where the bubbles would be
 * when it has none (a pending scolding needs no bubble). It grows in and shrinks out in
 * RING_BADGE_TICKS and moves with the row (its bob and its steps), one ordering table slot nearer than the bubbles.
 */
#define RING_BADGE_TICKS	4	/* 0.2 s to grow in or shrink out */
#define RING_BADGE_W	16	/* texels, as the dialogue box draws it */
#define RING_BADGE_H	12

extern int32_t MOD_FEED_HINT;
int32_t MOD_RING_BADGE = 0; /* the badge's size in ticks, 0..RING_BADGE_TICKS */

/* the badge centred on (x, y); k as renderConditionRing's, size 0..256 */
static void renderBadge(int32_t x, int32_t y, int32_t depth, int32_t k, int32_t size)
{
	int32_t kk = (k * size) >> 8;
	int32_t w = (RING_BADGE_W * kk) >> 12;
	int32_t h = (RING_BADGE_H * kk) >> 12;
	int32_t slot = (depth >> 4) - 1;
	POLY_FT4 *p;

	if (slot < 0 || slot >= 0x1000 || w < 1 || h < 1) {
		return;
	}
	p = (POLY_FT4 *)GsGetWorkBase();
	setPolyFT4(p);
	setSemiTrans(p, 1);
	p->tpage = GetTPage(0, 1, 320, 128);
	p->clut = GetClut(MOD_CARE_CLUT_X, MOD_CARE_CLUT_Y);
	setRGB0(p, 0x80, 0x80, 0x80);
	setUVWH(p, MOD_CARE_BUTTON_U((MOD_RING_CLOCK / 4) % 3),
	        MOD_CARE_BUTTON_V((MOD_RING_CLOCK / 4) % 3), RING_BADGE_W, RING_BADGE_H);
	setXYWH(p, x - w / 2, y - h / 2, w, h);
	AddPrim(ACTIVE_ORDERING_TABLE->org + slot, p);
	GsSetWorkBase((PACKET *)(p + 1));
}

static void renderConditionRing(int32_t x, int32_t y, int32_t depth,
				uint32_t scale)
{
	int32_t k = (scale * VIEWPORT_DISTANCE) / depth;
	int32_t cy0 = y - ((18 * k) >> 12);
	int32_t p = ringPhase();
	int32_t bx[8], by[8], bs[8], bt[8];
	int32_t drawn = 0;
	int32_t big = -1;
	int32_t full = 0;
	int32_t left = 0x8000;
	int32_t right = -0x8000;
	int32_t j;

	for (j = 0; j < 8; ++j) {
		ringBubbleAt(j, p, &bx[j], &by[j], &bs[j], &bt[j]);
		if (bs[j] > 24) {
			int32_t edge = (13 * bs[j]) >> 4;
			if (bx[j] - edge < left) {
				left = bx[j] - edge;
			}
			/* Include the legacy badge's full lower-right footprint. */
			if (MOD_RING_BADGE > 0) {
				edge = ((168 * bs[j]) >> 8) + RING_BADGE_W * 16 / 3 + RING_BADGE_W * 8;
			}
			if (bx[j] + edge > right) {
				right = bx[j] + edge;
			}
		}
		if (bs[j] > (RING_SIZE * 9) / 10) {
			++full;
		}
		if (bs[j] > 24 && (big < 0 || bs[j] > bs[big])) {
			big = j;
		}
	}
	/* Move the complete group at the screen sides, preserving every relative
	 * bubble and badge offset. The existing top clamp covers both rows. */
	if (left != 0x8000) {
		int32_t screenLeft = x + ((left * k) >> 16) + DRAWING_OFFSET_X;
		int32_t screenRight = x + ((right * k) >> 16) + DRAWING_OFFSET_X;
		int32_t inset = 3;
		int32_t clockRight = CLOCK_OFFSET_X + 160 + 50;
		/* The sliding clock occupies the upper-left corner, including its
		 * digital plate. Its HUD origin is fixed at 160, independent of the
		 * world camera offset. Keep the group beside those visible pixels. */
		if (clockRight > 0 && cy0 + DRAWING_OFFSET_Y - ((32 * k) >> 12) < 86 &&
		    screenLeft < clockRight + 3 &&
		    screenRight + (screenLeft < 3 ? 3 - screenLeft : 0) > clockRight - 52) {
			inset = clockRight + 3;
		}
		if (screenLeft < inset) {
			x += inset - screenLeft;
		} else if (screenRight > 317) {
			x -= screenRight - 317;
		}
	}
	/* the bottom row first, then the bigger: drawn first is on top (same
	 * depth), so the bottom row covers the top row's edge */
	while (drawn < 8) {
		int32_t b = -1;

		for (j = 0; j < 8; ++j) {
			if (bs[j] > 24 &&
			    (b < 0 || by[j] > by[b] || (by[j] == by[b] && bs[j] > bs[b]))) {
				b = j;
			}
		}
		if (b < 0) {
			break;
		}
		drawRingBubble(b, x + ((bx[b] * k) >> 16), cy0 + ((by[b] * k) >> 16),
			       depth, scale, bs[b], bt[b], k);
		bs[b] = -bs[b];
		++drawn;
	}
	if (MOD_RING_BADGE > 0) {
		int32_t right = -0x8000;
		int32_t bottom = 0;
		int32_t bx0 = 0;
		int32_t by0 = 0;

		/* beside the right edge of the bottom row, a bit under its centre */
		for (j = 0; j < 8; ++j) {
			if (bs[j] < -24) {
				if (bx[j] + ((168 * -bs[j]) >> 8) > right) {
					right = bx[j] + ((168 * -bs[j]) >> 8);
				}
				if (by[j] > bottom) {
					bottom = by[j];
				}
			}
		}
		if (right != -0x8000) {
			bx0 = right + RING_BADGE_W * 16 / 3;
			by0 = bottom + 96;
		}
		renderBadge(x + ((bx0 * k) >> 16), cy0 + ((by0 * k) >> 16), depth, k,
			    (MOD_RING_BADGE * 256) / RING_BADGE_TICKS);
	}
	MOD_RING_SHOWN[2] = big < 0 ? 0 :
			    ((big << 24) | (-bs[big] << 8) | (full > 1));
	MOD_RING_SHOWN[3] = p | (MOD_RING_STEPS << 16);
	MOD_RING_SHOWN[4] = (x & 0xFFFF) | (cy0 << 16);
	MOD_RING_SHOWN[0]++;
}

static int32_t ringAnyBubble(void)
{
	int32_t j;

	for (j = 0; j < 8; ++j) {
		if (MOD_RING_FROM[j] != RING_NONE ||
		    (MOD_RING_T >= 0 && MOD_RING_TO[j] != RING_NONE)) {
			return 1;
		}
	}
	return 0;
}

/*
 * MOD (farm bubble): over the meat farmer, a still bubble with the meat while a harvest waits. The upper city's
 * twelve layouts (168-179) always talk through NPC slot 6, script 207's section 11, so the test here is that
 * section's: a day has passed since the last harvest (p31), or this is the first visit to Tanemon, who gives Meat
 * at once; Palmon and Vegiemon have no such first gift. A full stack keeps the bubble, as the harvest is still
 * there. Nothing is drawn while a script runs: the bubble would hang over the farmer's own dialogue.
 */
#define FARM_FIRST_SCREEN	168
#define FARM_LAST_SCREEN	179
#define FARM_ENTITY		(6 + 2)	/* NPC slot 6 in ENTITY_TABLE */
#define FARM_TRIGGER_EXPLAINED	74
#define FARM_TRIGGER_VEGIEMON	225
#define FARM_TRIGGER_PALMON	246
#define FARM_PSTAT_DAYS		31
#define FARM_SCALE		((0x6000 * RING_SIZE) >> 8)	/* the size of the partner's bubbles */

static void renderFarmBubble(void)
{
	Entity *entity = ENTITY_TABLE[FARM_ENTITY];
	SVECTOR pos;
	DVECTOR screen;
	int32_t depth;

	if (CURRENT_SCREEN < FARM_FIRST_SCREEN || CURRENT_SCREEN > FARM_LAST_SCREEN ||
	    IS_SCRIPT_PAUSED != 1 || entity == NULL || entity->isOnMap == 0 ||
	    entity->isOnScreen == 0 ||
	    (readPStat(FARM_PSTAT_DAYS) < 1 &&
	     (isTriggerSet(FARM_TRIGGER_EXPLAINED) || isTriggerSet(FARM_TRIGGER_VEGIEMON) ||
	      isTriggerSet(FARM_TRIGGER_PALMON)))) {
		return;
	}
	pos.vx = entity->posData->location.vx;
	pos.vz = entity->posData->location.vz;
	pos.vy = -DIGI_PARA(entity->type).height + 20 - 65 + entity->posData->location.vy +
		 ((_sin((MOD_RING_CLOCK % 20) * 204) * 12) >> 12);
	depth = worldPosToScreenPos(&pos, &screen);
	if ((depth <= 0x200) || (depth >= 0x10000)) {
		return;
	}
	/* the partner's hungry icon on its first frame and the whole bubble, tail included. Nearer in the ordering
	 * table than the map's objects, as the partner's row. Only sprites here: a second caller of the row's drawing
	 * code stops the compiler inlining it, and that costs 400 B of heap. */
	CONDITION_SPRITES[0].u = 0x90;
	CONDITION_SPRITES[0].v = 0x80;
	CONDITION_SPRITES[0].my = 0x1a;
	depth -= 0x140;
	renderSprite(&CONDITION_SPRITES[0], screen.vx, screen.vy, depth, FARM_SCALE, FARM_SCALE);
	renderSprite(&CONDITION_SPRITES[11], screen.vx, screen.vy, depth, FARM_SCALE, FARM_SCALE);
}

void renderConditionRingObject(int32_t instanceId)
{
	Entity *entity = ENTITY_TABLE[1];
	SVECTOR pos;
	DVECTOR screen;
	int32_t depth;
	int32_t value;
	int32_t top;
	int32_t base;
	uint32_t scale = 0x6000;

	if ((IS_IN_MENU != 1) && (GAME_STATE == 0)) {
		renderFarmBubble();
	}
	if ((MOD_STATUS_BUBBLES == 0) || (IS_IN_MENU == 1) || (GAME_STATE != 0) ||
	    ((PARTNER_STATE != 1) && (PARTNER_STATE != 0xa)) ||
	    (entity == NULL) || (ringAnyBubble() == 0 && MOD_RING_BADGE == 0)) {
		MOD_RING_SHOWN[2] = 0;
		return;
	}

	/* the anchor is the partner's root, not the head bone the original
	 * bubble follows (translateConditionFXToEntity): the head moves with
	 * every animation and the cluster jumped with it */
	pos.vx = entity->posData->location.vx;
	pos.vz = entity->posData->location.vz;
	base = -DIGI_PARA(entity->type).height + 20 - 65 +
	       entity->posData->location.vy;
	/* the anchor without the bob, for tools/bubblecheck.py */
	pos.vy = base;
	worldPosToScreenPos(&pos, &screen);
	MOD_RING_SHOWN[1] = (screen.vx & 0xFFFF) | (screen.vy << 16);
	pos.vy = base +
		 ((_sin((((MOD_RING_CLOCK % 20) * 4096 + MOD_RENDER_ALPHA) * 512) / 81920) * 12) >> 12);
	depth = worldPosToScreenPos(&pos, &screen);
	if (depth <= 0) {
		return;
	}

	if (entity->isOnScreen != 0) {
		top = (60 * ((scale * VIEWPORT_DISTANCE) / depth)) >> 12;
		if ((screen.vy + DRAWING_OFFSET_Y) < top) {
			screen.vy = -DRAWING_OFFSET_Y + top;
		}
		value = (depth >> 4) - 20;
		if (value < 36) {
			value = 36;
		}
		scale = ((int32_t)scale * value) / (depth >> 4);
		depth = value << 4;
	}

	if ((depth <= 0x200) || (depth >= 0x10000)) {
		return;
	}

	renderConditionRing(screen.vx, screen.vy, depth, scale);
}

void tickConditionRingObject(int32_t instanceId)
{
	uint32_t cond = PARTNER_PARA.condition;
	int32_t act[8];
	int32_t to[8];
	int32_t n = 0;
	int32_t top;
	int32_t rank = 0;
	int32_t change = 0;
	int32_t j;

	if (IS_IN_MENU == 1) {
		return;
	}
	++MOD_RING_CLOCK;
	/* MOD (care menu): the badge follows the hint, which lasts one tick */
	if (MOD_FEED_HINT) {
		if (MOD_RING_BADGE < RING_BADGE_TICKS) {
			++MOD_RING_BADGE;
		}
	} else if (MOD_RING_BADGE > 0) {
		--MOD_RING_BADGE;
	}
	MOD_FEED_HINT = 0;

	/* a running step goes on to its end; it ends the tick after its last
	 * frame, so that every pass of that tick still shows it */
	if (MOD_RING_T >= 0) {
		if (MOD_RING_T < RING_STEP_TICKS) {
			++MOD_RING_T;
			return;
		}
		for (j = 0; j < 8; ++j) {
			MOD_RING_FROM[j] = MOD_RING_TO[j];
		}
		MOD_RING_T = -1;
	}

	for (j = 0; j < 8; ++j) {
		if (j < 6) {
			act[j] = (cond & RING_BIT[j]) != 0;
		} else if (j == 6) {
			act[j] = modResourceNeed(PARTNER_ENTITY.digimonEntity.stats.current.currentHP,
			                         PARTNER_ENTITY.digimonEntity.stats.base.hp);
		} else {
			act[j] = modResourceNeed(PARTNER_ENTITY.digimonEntity.stats.current.currentMP,
			                         PARTNER_ENTITY.digimonEntity.stats.base.mp);
		}
		n += act[j];
	}

	/* one or two in one row; from three on the first half goes on top */
	top = n > 2 ? (n + 1) / 2 : 0;
	for (j = 0; j < 8; ++j) {
		to[j] = RING_NONE;
		if (act[j]) {
			int32_t col = rank < top ? rank : rank - top;
			int32_t cnt = rank < top ? top : n - top;
			int32_t x = ((2 * col - (cnt - 1)) * RING_COL) / 2;
			int32_t y = rank < top ? -RING_ROW : 0;

			++rank;
			to[j] = ((x + 128) << 8) | (y + 128) | (n == 1 ? RING_TAIL : 0);
		}
		if (to[j] != MOD_RING_FROM[j]) {
			change = 1;
		}
	}
	if (change) {
		for (j = 0; j < 8; ++j) {
			MOD_RING_TO[j] = to[j];
		}
		MOD_RING_T = 0;
		MOD_RING_STEPS++;
	}
}

void renderConditionBubble(int32_t instanceId)
{
	ConditionBubble *bubble;
	SVECTOR pos;
	DVECTOR screen;
	int32_t top;
	int32_t value;
	uint32_t scale;
	int32_t depth;

	bubble = &CONDITION_BUBBLES[instanceId];

	if ((IS_IN_MENU == 1) || (MOD_STATUS_BUBBLES != 0)) {
		return;
	}

	translateConditionFXToEntity(bubble->entity, &pos);
	pos.vy = bubble->activeHeight + bubble->entity->posData->location.vy;
	depth = worldPosToScreenPos(&pos, &screen);
	scale = bubble->scale;

	if (bubble->entity->isOnScreen != 0) {
		if (bubble->frameCount >= 51) {
			top = 8;
		} else {
			top = 32;
		}
		top = (top * ((scale * VIEWPORT_DISTANCE) / depth)) >> 12;

		if ((screen.vy + DRAWING_OFFSET_Y) < top) {
			screen.vy = -DRAWING_OFFSET_Y + top;
		}

		value = (depth >> 4) - 20;
		if (value < 36) {
			value = 36;
		}
		scale = ((int32_t)scale * value) / (depth >> 4);
		depth = value << 4;
	}

	if ((depth <= 0x200) || (depth >= 0x10000)) {
		return;
	}

	if (bubble->frameCount >= 51) {
		renderSprite(&CONDITION_SPRITES[10], screen.vx, screen.vy,
			     depth, scale, scale);
		return;
	}

	drawConditionIcon(bubble->iconPtr->icon, bubble->frameCount, screen.vx,
			  screen.vy, depth, scale);

	renderSprite(&CONDITION_SPRITES[11], screen.vx, screen.vy, depth,
		     scale, scale);
}

void tickConditionBubble(int32_t instanceId)
{
	ConditionBubble *bubble;
	int32_t angle;
	int32_t sine;

	bubble = &CONDITION_BUBBLES[instanceId];

	if (IS_IN_MENU == 1) {
		return;
	}

	if (bubble->frameCount >= 50) {
		if (bubble->frameCount > 50) {
			removeConditionBubble(instanceId);
			return;
		}
		bubble->activeHeight = bubble->baseHeight - 230;
		bubble->scale = 0xf000;
		bubble->scale2 = 0xf000;
	} else {
		if (bubble->frameCount < 10) {
			bubble->scale = lerp(0, 0x6000, 0, 10,
					     bubble->frameCount);
			bubble->scale2 = bubble->scale;
			bubble->activeHeight =
				bubble->baseHeight +
				lerp(0, -65, 0, 10, bubble->frameCount - 10);
		} else {
			bubble->scale = 0x6000;
			bubble->scale2 = 0x6000;
			bubble->activeHeight = bubble->baseHeight - 65;
			angle = lerp(0, 512, 0, 20, bubble->frameCount);
			sine = _sin(angle);
			bubble->activeHeight += (sine * 12) >> 12;
		}
	}

	if (bubble->frameCount2 >= bubble->iconPtr->duration) {
		bubble->frameCount2 = 0;
		bubble->iconPtr += bubble->iconPtr->nextIcon;
	}

	++bubble->frameCount2;
	++bubble->frameCount;
}

void removeConditionBubble(int32_t instanceId)
{
	if ((instanceId >= 0) &&
	    (instanceId < NUM_CONDITION_BUBBLES) &&
	    (CONDITION_BUBBLES[instanceId].frameCount >= 0)) {
		CONDITION_BUBBLES[instanceId].frameCount = -1;
		removeObject(0x802, instanceId);
	}
}
