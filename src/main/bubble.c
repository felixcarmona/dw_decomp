#include <dw/bubble.h>
#include <dw/params.h>
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
 * MOD (status-bubbles): the ACTIVE conditions only: one big bubble and
 * (N - 1) small ones on a symmetric arc over it. Each condition has a smoothed
 * position and size (x, y in 1/16 texel from the big bubble's centre, size
 * 0..256); the targets change when the big bubble moves on to the next active
 * condition or a condition appears or goes, and the bubbles slide there.
 * Stored in .data (initialised) so that the linker keeps them.
 */
#define RING_HOLD_TICKS	50
#define RING_RADIUS	28

/* conditions in a fixed order: hungry, poopy, sleepy, tired, sick, injured */
static const uint8_t RING_BIT[6] = { 0x04, 0x08, 0x01, 0x02, 0x40, 0x20 };
static const uint8_t RING_ICON[6] = { 1, 2, 4, 9, 10, 11 };
/* half span of the arc in degrees by the number of small bubbles */
static const uint8_t RING_SPAN[6] = { 0, 0, 36, 60, 72, 82 };

int32_t MOD_RING_CLOCK = 1;
int32_t MOD_RING_BIG = 1; /* the big condition, +1 */
int32_t MOD_RING_HOLD = 1;
int32_t MOD_RING_POS[18] = { 1 }; /* x, y, size per condition */
int32_t MOD_RING_PREV[18] = { 1 }; /* the same before the last tick */
extern int32_t MOD_RENDER_ALPHA;

static void drawRingBubble(int32_t j, int32_t cx, int32_t cy, int32_t depth,
			   uint32_t scale, int32_t size, int32_t k)
{
	uint32_t s = (scale * size) >> 8;
	int32_t iconY = cy + ((18 * k * size) >> 20);
	int32_t icon = RING_ICON[j];

	if (icon == 4) {
		icon = 4 + ((MOD_RING_CLOCK / 8) % 5);
	}
	drawConditionIcon(icon, MOD_RING_CLOCK, cx, iconY, depth, s);
	if (size >= 192) {
		renderSprite(&CONDITION_SPRITES[11], cx, iconY, depth, s, s);
	} else {
		CONDITION_SPRITES[11].h = 26;
		CONDITION_SPRITES[11].my = 13;
		renderSprite(&CONDITION_SPRITES[11], cx, cy, depth, s, s);
		CONDITION_SPRITES[11].h = 32;
		CONDITION_SPRITES[11].my = 0x1f;
	}
}

static int32_t ringAnySize(void)
{
	int32_t j;

	for (j = 0; j < 6; ++j) {
		if (MOD_RING_POS[j * 3 + 2] > 4) {
			return 1;
		}
	}
	return 0;
}

static void renderConditionRing(int32_t x, int32_t y, int32_t depth,
				uint32_t scale)
{
	int32_t k = (scale * VIEWPORT_DISTANCE) / depth;
	int32_t cy0 = y - ((18 * k) >> 12);
	int32_t j;

	for (j = 0; j < 6; ++j) {
		int32_t *v = &MOD_RING_POS[j * 3];
		int32_t *o = &MOD_RING_PREV[j * 3];
		int32_t a = MOD_RENDER_ALPHA;
		int32_t px = o[0] + (((v[0] - o[0]) * a) >> 12);
		int32_t py = o[1] + (((v[1] - o[1]) * a) >> 12);
		int32_t ps = o[2] + (((v[2] - o[2]) * a) >> 12);

		if (ps > 4) {
			drawRingBubble(j, x + ((px * k) >> 16),
				       cy0 + ((py * k) >> 16), depth, scale,
				       ps, k);
		}
	}
}

static int32_t ringStep(int32_t v, int32_t target)
{
	int32_t d = target - v;

	if ((d / 2) == 0) {
		return target;
	}
	return v + d / 2;
}

void renderConditionRingObject(int32_t instanceId)
{
	Entity *entity = ENTITY_TABLE[1];
	SVECTOR pos;
	DVECTOR screen;
	int32_t depth;
	int32_t value;
	int32_t top;
	uint32_t scale = 0x6000;

	if ((MOD_STATUS_BUBBLES == 0) || (IS_IN_MENU == 1) || (GAME_STATE != 0) ||
	    ((PARTNER_STATE != 1) && (PARTNER_STATE != 0xa)) ||
	    (entity == NULL) || (ringAnySize() == 0)) {
		return;
	}

	translateConditionFXToEntity(entity, &pos);
	pos.vy = -DIGI_PARA(entity->type).height + 20 - 65 +
		 ((_sin((((MOD_RING_CLOCK % 20) * 4096 + MOD_RENDER_ALPHA) * 512) / 81920) * 12) >> 12) +
		 entity->posData->location.vy;
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
	int32_t big = MOD_RING_BIG - 1;
	int32_t act[6];
	int32_t n = 0;
	int32_t rankBig = 0;
	int32_t rank = 0;
	int32_t j;
	int32_t m;

	if (IS_IN_MENU == 1) {
		return;
	}
	++MOD_RING_CLOCK;

	for (j = 0; j < 6; ++j) {
		act[j] = (cond & RING_BIT[j]) != 0;
		n += act[j];
	}

	/* the big bubble: stays RING_HOLD_TICKS, then the next active one */
	++MOD_RING_HOLD;
	if ((n != 0) && ((!act[big]) || (MOD_RING_HOLD > RING_HOLD_TICKS))) {
		for (j = 1; j <= 6; ++j) {
			if (act[(big + j) % 6]) {
				big = (big + j) % 6;
				break;
			}
		}
		MOD_RING_BIG = big + 1;
		MOD_RING_HOLD = 1;
	}

	for (j = 0; j < big; ++j) {
		rankBig += act[j];
	}
	m = n - 1;

	for (j = 0; j < 6; ++j) {
		int32_t *v = &MOD_RING_POS[j * 3];
		int32_t tx = v[0];

		MOD_RING_PREV[j * 3] = v[0];
		MOD_RING_PREV[j * 3 + 1] = v[1];
		MOD_RING_PREV[j * 3 + 2] = v[2];
		int32_t ty = v[1];
		int32_t ts = 0;

		if (act[j]) {
			int32_t r = (rank - rankBig + n) % n;

			++rank;
			if (r == 0) {
				tx = 0;
				ty = 0;
				ts = 256;
			} else {
				int32_t span = RING_SPAN[m];
				int32_t a = (m > 1) ?
					    (-span + (2 * span * (r - 1)) / (m - 1)) : 0;
				int32_t ang = (a * 512) / 360; /* 512 = full turn */

				tx = (RING_RADIUS * 16 * _sin(ang)) >> 12;
				ty = -((RING_RADIUS * 16 * _cos(ang)) >> 12);
				ts = 128;
			}
			if (v[2] <= 4) {
				v[0] = tx;
				v[1] = ty;
				MOD_RING_PREV[j * 3] = tx;
				MOD_RING_PREV[j * 3 + 1] = ty;
			}
		}
		v[0] = ringStep(v[0], tx);
		v[1] = ringStep(v[1], ty);
		v[2] = ringStep(v[2], ts);
	}
}

void renderConditionBubble(int32_t instanceId)
{
	ConditionBubble *bubble;
	SVECTOR pos;
	DVECTOR screen;
	int32_t icon;
	GsSPRITE **sprites;
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
