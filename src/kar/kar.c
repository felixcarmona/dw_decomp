#include <stdlib.h>

#include <inline_n.h>
#include <libetc.h>
#include <libgpu.h>
#include <libgs.h>
#include <libgte.h>

#include <dw/mod_kar.h>
#include <dw/mod_font.h>
#include <dw/anim.h>
#include <dw/btl.h>
#include <dw/entity.h>
#include <dw/file.h>
#include <dw/font.h>
#include <dw/garbage.h>
#include <dw/input.h>
#include <dw/kar.h>
#include <dw/map.h>
#include <dw/mod_fps.h>
#include <dw/script.h>
#include <dw/sound.h>
#include <dw/types.h>
#include <dw/tfs_stream.h>
#include <dw/ui.h>
#include <dw/version.h>
#include <dw/world_object.h>
#include <text/kar/kar.h>

#include "common.h"

extern int32_t MOD_RENDER_ALPHA;
extern int32_t MOD_PERF_TICK_SUM, MOD_PERF_RENDER_SUM;
extern int32_t MOD_FPS_TEST_TICK, MOD_FPS_TICKS;

/* Keep the draw history in KAR, outside the resident executable's small-data
 * sections. A reset or a discontinuity never blends two unrelated stones. */
typedef struct {
	VECTOR pos;
	SVECTOR rotation;
	int32_t state;
} KarDrawStone;
static struct {
	KarDrawStone previous[15];
	KarDrawStone current[15];
	uint8_t blend[15];
	int32_t valid;
	int32_t hudState;
	VECTOR arrowOrigin;
	SVECTOR arrowRotation;
	VECTOR previousArrowOrigin;
	SVECTOR previousArrowRotation;
	int32_t arrowReady, arrowBlend;
	uint32_t tickSum, renderSum;
	int32_t warmup, samples, overBudget, steady30;
} KAR_MOD __attribute__((section(".bss.kar_mod")));

/* Component copies avoid the SDK's ROM memcpy on every stone/tick. */
static void KAR_copyVector(VECTOR *to, const VECTOR *from)
{
	to->vx = from->vx;
	to->vy = from->vy;
	to->vz = from->vz;
	to->pad = from->pad;
}

static void KAR_lateStones(int32_t instanceId);

typedef struct {
	int16_t start;
	int16_t count;
} KarRingObject;
/* Starts and counts are four bytes apart in this interleaved table. */
typedef union {
	KarRingObject ring[4];
	int16_t starts[8];
	struct {
		int16_t firstStart;
		int16_t counts[7];
	} countView;
} KarRingObjects;
typedef struct {
	int8_t ring[4];
} KarRingFlags;

extern KarRingFlags MAIN_D_80134A48;
void setMapObjectsFlag(int32_t start, int32_t count, int32_t flag);

extern int32_t ACTIVE_FRAMEBUFFER;
extern int32_t VIEWPORT_DISTANCE;
extern GsOT GS_ORDERING_TABLE[];
extern GsRVIEW2 GS_VIEWPOINT;

void renderSelectionCursor(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e);
void renderString(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h, int32_t i);
void renderUIBox(int32_t id);

void KAR_tickMatchState(void);
void KAR_renderAimArrow(void);
void KAR_updateRingMarkers(void);
void KAR_checkStonesStopped(void);
int32_t KAR_distance(long x, long y);
void KAR_debugCollisionStone(KarStone *stone, VECTOR *b, VECTOR *a);
void KAR_debugCollisionPair(KarStone *stoneA, VECTOR *a, KarStone *stoneB, VECTOR *b);
void KAR_reflectOffDiagonal(int32_t *p, int32_t dir);
void KAR_turnAimLeft(int32_t delta);
void KAR_turnAimRight(int32_t delta);
int32_t KAR_findUnusedStoneOfType(int32_t key);
void KAR_initializeOrderingTables(void);
int8_t KAR_tickYesNoPrompt(void);
void KAR_resolveStoneCollision(KarStone *stoneA, VECTOR a, KarStone *stoneB, VECTOR b);
void KAR_collideRestingStone(KarStone *resting, VECTOR a, KarStone *mover, VECTOR b);
void KAR_collideMovingStones(KarStone *stoneA, VECTOR a, KarStone *stoneB, VECTOR b);
int32_t KAR_tickHintBox(int32_t n);
int32_t KAR_computeSeparation(VECTOR *out, KarStone *stone, VECTOR a, VECTOR b);
void KAR_collidePeggedStone(KarStone *pegged, VECTOR a, KarStone *mover, VECTOR b);
void KAR_beginAiming(void);
void KAR_renderSprite(KarSprite *sp);
int32_t KAR_findWallContact(VECTOR *out, KarStone *stone, int32_t flag);
int32_t KAR_aimAtRandomStone(void);
int32_t KAR_computeImpactShare(KarStone *stone, VECTOR a, VECTOR b);
int32_t KAR_getWallZone(int16_t x, int16_t z);
void KAR_selectPreviousStone(void);
void KAR_selectNextStone(void);
void KAR_tickStones(int32_t instanceId);
void KAR_renderScene(int32_t instanceId);
void KAR_setupMatch(int32_t arg);
void KAR_renderStoneCursor(void);
void KAR_renderReadyPrompt(void);
void KAR_registerThrownStone(void);
int32_t KAR_chooseOpponentShot(void);
int8_t KAR_tickScoreTally(void);
void KAR_bounceOffWall(void);
void KAR_updateCollisions(void);
void KAR_classifyStoneRings(void);
void KAR_renderScores(void);
void KAR_finishMatch(void);
int32_t KAR_drawHintPagePenguinmon(int32_t idx, int8_t n);
int32_t KAR_drawHintPageMetalMamemon(int32_t idx, int8_t n);
void KAR_renderPowerMeter(void);
int32_t KAR_computeThrowPower(uint32_t a, int32_t b, int32_t c);
void moveCameraByOffset(int32_t diffX, int32_t diffY);
void KAR_setOpponentShot(int8_t row, int32_t val, int16_t b, int16_t c);
void KAR_rotatePoint(SVECTOR *p, int32_t ang);
int32_t KAR_aimBankShot(KarStone *stone, int32_t x, int32_t z);
void KAR_placeAtContact(KarStone *stone, VECTOR p);
void KAR_handleAimScroll(void);
int32_t KAR_findClearShotAngle(int16_t x, int16_t z);
int32_t KAR_aimAtStoneInRing(int32_t player, int32_t key, int16_t *outX, int16_t *outZ);
void KAR_renderNamePlates(void);
void KAR_beginThrow(void);

int32_t tickCameraMoveTo(int16_t x, int16_t z, int32_t speed);
void uploadMapTileImages();
void moveCameraByDiff(VECTOR *from, VECTOR *to);

static void *kar_functions[] = {
	KAR_renderSprite,
	KAR_rotatePoint,
	KAR_registerThrownStone,
	KAR_computeThrowPower,
	KAR_findUnusedStoneOfType,
	KAR_aimBankShot,
	KAR_aimAtRandomStone,
	KAR_setOpponentShot,
	KAR_aimAtStoneInRing,
	KAR_findClearShotAngle,
	KAR_drawHintPageMetalMamemon,
	KAR_drawHintPagePenguinmon,
	KAR_chooseOpponentShot,
	KAR_tickScoreTally,
	KAR_classifyStoneRings,
	KAR_tickHintBox,
	KAR_tickYesNoPrompt,
	KAR_beginThrow,
	KAR_turnAimRight,
	KAR_turnAimLeft,
	KAR_selectNextStone,
	KAR_selectPreviousStone,
	KAR_beginAiming,
	KAR_placeAtContact,
	KAR_reflectOffDiagonal,
	KAR_findWallContact,
	KAR_getWallZone,
	KAR_debugCollisionPair,
	KAR_computeSeparation,
	KAR_computeImpactShare,
	KAR_debugCollisionStone,
	KAR_collideMovingStones,
	KAR_collideRestingStone,
	KAR_collidePeggedStone,
	KAR_resolveStoneCollision,
	KAR_distance,
	KAR_bounceOffWall,
	KAR_updateCollisions,
	KAR_checkStonesStopped,
	KAR_updateRingMarkers,
	KAR_handleAimScroll,
	KAR_renderNamePlates,
	KAR_renderReadyPrompt,
	KAR_renderPowerMeter,
	KAR_renderScores,
	KAR_renderStoneCursor,
	KAR_renderAimArrow,
	KAR_tickMatchState,
	KAR_tick,
	KAR_finishMatch,
	KAR_renderScene,
	KAR_tickStones,
	KAR_start,
	KAR_setupMatch,
	KAR_initializeOrderingTables,
};

uint8_t MAIN_D_80135220;
int32_t MAIN_D_80135224;
uint32_t MAIN_D_80135228;
uint8_t MAIN_D_8013522C;
int8_t MAIN_D_8013522D;
int8_t MAIN_D_8013522E;
int32_t MAIN_D_80135230;
int32_t MAIN_D_80135234;
uint16_t MAIN_D_80135238;
int16_t MAIN_D_8013523A;
int8_t MAIN_D_8013523C;
int16_t MAIN_D_8013523E;
u_long *MAIN_D_80135240;
int32_t MAIN_D_80135244;
uint8_t MAIN_D_80135248;
int32_t MAIN_D_8013524C;
int8_t MAIN_D_80135250;
uint16_t MAIN_D_80135252;
int16_t MAIN_D_80135254;
int8_t MAIN_D_80135256;
uint8_t MAIN_D_80135257;

static void *kar_sbss_order[] = {
	&MAIN_D_80135257,
	&MAIN_D_80135256,
	&MAIN_D_80135254,
	&MAIN_D_80135252,
	&MAIN_D_80135250,
	&MAIN_D_8013524C,
	&MAIN_D_80135248,
	&MAIN_D_80135244,
	&MAIN_D_80135240,
	&MAIN_D_8013523E,
	&MAIN_D_8013523C,
	&MAIN_D_8013523A,
	&MAIN_D_80135238,
	&MAIN_D_80135234,
	&MAIN_D_80135230,
	&MAIN_D_8013522E,
	&MAIN_D_8013522D,
	&MAIN_D_8013522C,
	&MAIN_D_80135228,
	&MAIN_D_80135224,
	&MAIN_D_80135220,
};

// clang-format off
KarModelIds KAR_D_8005AB80 = { { 0x00000002, 0x00000001, 0x00000000 } };

KarOffTbl KAR_D_8005AB8C = {
	{
		0x03, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00,
		0x00, 0x00,
	},
};

#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
KarSpawnX KAR_D_8005AB98 = { { 0x000002c1, 0x000000eb, 0xffffff15 } };
#endif

char KAR_D_8005ABA4[20] = "\\ETCDAT\\KARRING.TMD";

MENU_TEXT

KarWeightTbl MAIN_D_80134A08 = { { 30, 25, 35, 30 } };
RECT MAIN_D_80134A10 = { -130, 42, 262, 59 };
RECT MAIN_D_80134A18 = { 75, -5, 10, 10 };
#if !VERSION_IS(US)
KarStrPair MAIN_D_80134A20 = { { KAR_STR_START_GAME, KAR_STR_EXPLAIN_GAME } };
#else
KarStrPair MAIN_D_80134A20 = { { KAR_D_8005ABB8, NULL } };
#endif
RECT MAIN_D_80134A28 = { -130, 42, 262, 59 };
RECT MAIN_D_80134A30 = { 0, 0, 10, 10 };

PENGUINMON_DIALOGUE_TEXT

KarStrTbl KAR_D_8005AF58 = {{
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_1,
	KAR_PENGUINMON_LINE_2,
	KAR_PENGUINMON_LINE_3,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_5,
	KAR_PENGUINMON_LINE_6,
	KAR_PENGUINMON_LINE_7,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_9,
	KAR_PENGUINMON_LINE_10,
	KAR_PENGUINMON_LINE_11,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_13,
	KAR_PENGUINMON_LINE_14,
	KAR_PENGUINMON_LINE_15,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_17,
	KAR_PENGUINMON_LINE_18,
	KAR_PENGUINMON_LINE_19,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_21,
	KAR_PENGUINMON_LINE_22,
	KAR_PENGUINMON_LINE_23,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_25,
	KAR_PENGUINMON_LINE_26,
	KAR_PENGUINMON_LINE_27,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_29,
	KAR_PENGUINMON_LINE_30,
	KAR_PENGUINMON_LINE_31,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_33,
	KAR_PENGUINMON_LINE_34,
	KAR_PENGUINMON_LINE_35,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_37,
	KAR_PENGUINMON_LINE_38,
	KAR_PENGUINMON_LINE_39,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_41,
	KAR_PENGUINMON_LINE_42,
	KAR_PENGUINMON_LINE_43,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_45,
	KAR_PENGUINMON_LINE_46,
	KAR_PENGUINMON_LINE_47,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_49,
	KAR_PENGUINMON_LINE_50,
	KAR_PENGUINMON_LINE_51,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_53,
	KAR_PENGUINMON_LINE_54,
	KAR_PENGUINMON_LINE_55,
	KAR_STR_PENGUINMON,
	KAR_PENGUINMON_LINE_57,
	KAR_PENGUINMON_LINE_58,
	KAR_PENGUINMON_LINE_59,
	NULL,
}};

PENGUINMON_HIGHLIGHT_MARKS_TEXT

#if !VERSION_IS(US)
KarStrPair KAR_PENGUINMON_HIGHLIGHTS = { { KAR_STR_BUTTON_MARK_COL11, KAR_STR_BUTTON_MARK_COL1 } };
#endif

KarOffTbl KAR_D_8005B04C = {
	{
		0x00, 0x0c, 0x14, 0x18, 0x28, 0x2c, 0x30, 0x34,
		0x38, 0x3c,
	},
};

METALMAMEMON_DIALOGUE_TEXT

KarStrTbl KAR_D_8005B318 = {{
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_1,
	KAR_METALMAMEMON_LINE_2,
	KAR_METALMAMEMON_LINE_3,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_5,
	KAR_METALMAMEMON_LINE_6,
	KAR_METALMAMEMON_LINE_7,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_9,
	KAR_METALMAMEMON_LINE_10,
	KAR_METALMAMEMON_LINE_11,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_13,
	KAR_METALMAMEMON_LINE_14,
	KAR_METALMAMEMON_LINE_15,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_17,
	KAR_METALMAMEMON_LINE_18,
	KAR_METALMAMEMON_LINE_19,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_21,
	KAR_METALMAMEMON_LINE_22,
	KAR_METALMAMEMON_LINE_23,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_25,
	KAR_METALMAMEMON_LINE_26,
	KAR_METALMAMEMON_LINE_27,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_29,
	KAR_METALMAMEMON_LINE_30,
	KAR_METALMAMEMON_LINE_31,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_33,
	KAR_METALMAMEMON_LINE_34,
	KAR_METALMAMEMON_LINE_35,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_37,
	KAR_METALMAMEMON_LINE_38,
	KAR_METALMAMEMON_LINE_39,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_41,
	KAR_METALMAMEMON_LINE_42,
	KAR_METALMAMEMON_LINE_43,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_45,
	KAR_METALMAMEMON_LINE_46,
	KAR_METALMAMEMON_LINE_47,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_49,
	KAR_METALMAMEMON_LINE_50,
	KAR_METALMAMEMON_LINE_51,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_53,
	KAR_METALMAMEMON_LINE_54,
	KAR_METALMAMEMON_LINE_55,
	KAR_STR_METALMAMEMON,
	KAR_METALMAMEMON_LINE_57,
	KAR_METALMAMEMON_LINE_58,
	KAR_METALMAMEMON_LINE_59,
	KAR_METALMAMEMON_LINE_60,
}};

METALMAMEMON_HIGHLIGHT_MARKS_TEXT

#if !VERSION_IS(US)
KarStrPair KAR_METALMAMEMON_HIGHLIGHTS = { { KAR_STR_BUTTON_MARK_COL11, KAR_STR_BUTTON_MARK_COL5 } };
#endif

KarOffTbl KAR_D_8005B40C = {
	{
		0x00, 0x0c, 0x14, 0x18, 0x28, 0x2c, 0x30, 0x34,
		0x38, 0x3c,
	},
};

KarTallyValues MAIN_D_80134A44 = { { 2, 1, -2, 2 } };
KarRingFlags MAIN_D_80134A48 = { { 0 } };
KarPeggedModelIds MAIN_D_80134A4C = { { 11, 10, 9 } };
int8_t MAIN_D_80134A4F = -1;

KarShotPlan KAR_D_8005B418 = {
	{
		0x00000001, 0x00000000, 0x00000002, 0x00000003,
		0x00000003, 0x00000002, 0x00000000, 0x00000001,
	},
};

KarZones KAR_D_8005B438 = {
	{
		{ 0x0000, 0x06b6, 0x0091 },
		{ 0x0000, 0x06b6, 0x0177 },
		{ 0xfe70, 0x090e, 0x00cd },
		{ 0x0190, 0x090e, 0x00cd },
	},
};

KarRingObjects KAR_D_8005B450 = {
	{
		{0x0019, 0x0001},
		{0x0013, 0x0004},
		{0x0010, 0x0003},
		{0x000d, 0x0003},
	},
};

KarZones KAR_D_8005B460 = {
	{
		{0x0000, 0x06b6, 0x0091},
		{0x0000, 0x06b6, 0x0177},
		{0xfe70, 0x090e, 0x00cd},
		{0x0190, 0x090e, 0x00cd},
	},
};

KarSprite KAR_D_8005B478 = {
	0xff6a,
	0x0000,
	0x0080,
	0x01e6,
	0x0000,
	0x0028,
	0x0018,
	0x0018,
	0x80,
	0x80,
	0x80,
	0x00,
};

KarSprite KAR_D_8005B48C = {
	0xff6a,
	0x0000,
	0x0010,
	0x01e6,
	0x0000,
	0x0028,
	0x0010,
	0x0010,
	0x80,
	0x80,
	0x80,
	0x00,
};

KarDigits KAR_D_8005B4A0 = {
	{
		{ 0x0040, 0x0018 },
		{ 0x0050, 0x0018 },
		{ 0x0060, 0x0018 },
		{ 0x0040, 0x0028 },
		{ 0x0050, 0x0028 },
		{ 0x0060, 0x0028 },
		{ 0x0048, 0x0038 },
		{ 0x0058, 0x0038 },
		{ 0x0048, 0x0048 },
		{ 0x0058, 0x0048 },
		{ 0x0000, 0x0040 },
	},
};

KarSpritePair KAR_D_8005B4CC = {
	{
		{
			0xffce,
			0xffce,
			0x0080,
			0x01e6,
			0x0030,
			0x0018,
			0x0010,
			0x0010,
			0x80,
			0x80,
			0x80,
			0x00,
		},
		{
			0xffce,
			0xffce,
			0x0070,
			0x01e6,
			0x0000,
			0x0018,
			0x0030,
			0x0010,
			0x80,
			0x80,
			0x80,
			0x00,
		},
	},
};

KarSpriteSet KAR_D_8005B4F4 = {
	{
		{
			0xff74,
			0xffa1,
			0x0090,
			0x01e6,
			0x0070,
			0x0018,
			0x0020,
			0x0020,
			0x80,
			0x80,
			0x80,
			0x00,
		},
		{
			0xff7e,
			0xffba,
			0x0070,
			0x01e6,
			0x0018,
			0x0038,
			0x0030,
			0x0020,
			0x80,
			0x80,
			0x80,
			0x00,
		},
		{
			0x0064,
			0xffa1,
			0x00a0,
			0x01e6,
			0x0090,
			0x0018,
			0x0020,
			0x0020,
			0x80,
			0x80,
			0x80,
			0x00,
		},
		{
			0x0050,
			0xffba,
			0x0080,
			0x01e6,
			0x0018,
			0x0038,
			0x0030,
			0x0020,
			0x80,
			0x80,
			0x80,
			0x00,
		},
		{
			0x0000,
			0x0000,
			0x0000,
			0x0000,
			0x0000,
			0x0000,
			0x0000,
			0x0000,
			0x00,
			0x00,
			0x00,
			0x00,
		},
	},
};

KarSprite KAR_D_8005B558 = {
	0x0064,
	0xffa1,
	0x00c0,
	0x01e6,
	0x00b0,
	0x0018,
	0x0020,
	0x0020,
	0x80,
	0x80,
	0x80,
	0x00,
};

KarSprite KAR_D_8005B56C = {
	0xffcc,
	0x0000,
	0x00b0,
	0x01e6,
	0x0068,
	0x003e,
	0x0097,
	0x000a,
	0x80,
	0x80,
	0x80,
	0x00,
};

GsRVIEW2 KAR_D_8005B580 = {
	0x00000000,
	0x00000000,
	0xfffff308,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	NULL,
};
// clang-format on

GARBAGE(KAR_initializeOrderingTables, 6);

void KAR_initializeOrderingTables(void)
{
	KAR_D_800638CC[0].length = 5;
	KAR_D_800638CC[0].org = KAR_D_800637CC;
	KAR_D_800638CC[1].length = 5;
	KAR_D_800638CC[1].org = KAR_D_8006384C;
}

void KAR_setupMatch(int32_t mode)
{
	int32_t seed;
	int32_t obstacles;
	KarModelIds models;
	KarWeightTbl weights;
	KarOffTbl types;
	int32_t n;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	KarSpawnX spawnX;
#endif
	int32_t p;
	int32_t i;
	KarStone *stone;

	KAR_MOD.valid = 0;
	KAR_MOD.arrowReady = KAR_MOD.arrowBlend = 0;
	KAR_MOD.tickSum = MOD_PERF_TICK_SUM;
	KAR_MOD.renderSum = MOD_PERF_RENDER_SUM;
	KAR_MOD.warmup = KAR_MOD.samples = KAR_MOD.overBudget = 0;
	KAR_MOD.steady30 = 1;
	obstacles = rand() % BKS(OBST_MAX);
	MAIN_D_8013523A = 0;
	MAIN_D_8013523C = 0;
	MAIN_D_8013523E = 0;

	for (p = 0; p < 3; p++) {
		stone = KAR_D_8005B5A0[p].row.stones;
		seed = rand();
		KAR_D_8005B5A0[p].row.score = 0;
		KAR_D_8005B5A0[p].row.thrown = 0;
		KAR_D_8005B5A0[p].row.unk2 = 0;
		KAR_D_8005B5A0[p].row.unk4 = 0;

		for (i = 0; i < 5; i++, stone++) {
			models = KAR_D_8005AB80;
			weights = MAIN_D_80134A08;
			types = KAR_D_8005AB8C;

			n = rand();
			stone->speed = 0;
			stone->angle = 0;
			stone->ring = 0;
			stone->prevRing = 0;
			n = n % 10;
			if (mode == 0) {
				stone->type = types.start[n];
			} else {
				stone->type = i;
				if (stone->type >= 4) {
					stone->type = 3;
				}
			}
			GsLinkObject4((u_long)(MAIN_D_80135240 + 3), &stone->obj, stone->type * 3 + models.id[p]);
			GsInitCoordinate2(NULL, &stone->coord);
			stone->obj.attribute = 0;
			stone->obj.coord2 = &stone->coord;
			if (p < 2) {
				stone->pos.vy = i * 0xaa - 0x44c;
				stone->pos.vx = (p != 0) ? 0x226 : -0x384 - i * 0x1e;
				stone->pos.vz = 0;
				stone->pos.vy = i * 0xaa - 0x50;
				stone->pos.vx = (p != 0) ? 0x2bc : -0x2bc;
				stone->pos.vz = 0;
				stone->rotation.vx = 0x96;
				stone->rotation.vy = 0;
				stone->rotation.vz = 0;
				stone->state = -1;
			} else if (i < obstacles) {
#if VERSION_IS(JP)
				stone->pos.vy = 0;
				stone->pos.vx = rand() % 0x5dc - 0x2ee;
#else
				spawnX = KAR_D_8005AB98;
				stone->pos.vy = 0;
				stone->pos.vx = spawnX.x[i] - rand() % BKS(OBST_X_SPAN);
#endif
				stone->pos.vz = -(rand() % BKS(OBST_Z_SPAN) + BK(OBST_Z_BASE));
				stone->rotation.vx = 0;
				stone->rotation.vy = 0;
				stone->rotation.vz = 0;
				stone->state = 1;
				stone->speed = 0;
			} else {
				stone->pos.vy = 0;
				stone->pos.vx = 0;
				stone->pos.vz = 0;
				stone->rotation.vx = 0;
				stone->rotation.vy = 0;
				stone->rotation.vz = 0;
				stone->state = -0x65;
				stone->speed = 0;
			}
			stone->weight = weights.weight[stone->type];
		}
	}
}

void KAR_start(void)
{
#ifdef __MWERKS__
	extern int32_t readPStat(int32_t index);
#endif

	MAIN_D_80135240 = KAR_D_8005BFCC;
	SOME_SCRIPT_SYNC_BIT = 0;
	readFile(KAR_D_8005ABA4, MAIN_D_80135240);
	GsMapModelingData(MAIN_D_80135240 + 1);
	KAR_initializeOrderingTables();
	ENTITY_TABLE[0]->isOnMap = 1;
	MAIN_D_80135244 = 0;
	KAR_setupMatch(0);
	addObjectLate(0x1388, 0xf, KAR_tickStones, KAR_lateStones, KAR_renderScene);
	MAIN_D_80135248 = readPStat(0x7a);
	MAIN_D_80135248 += 2;
}

void KAR_tickStones(int32_t instanceId)
{
	KarStone *stone;
	int32_t p;
	int32_t i;
	int16_t move;
	uint32_t speed;
	KarPeggedModelIds models;
	int16_t step;

	for (p = 0; p < 3; p++) {
		stone = KAR_D_8005B5A0[p].row.stones;
		for (i = 0; i < 5; i++, stone++) {
			if (stone->state != 3 && stone->state != 2) {
				KAR_copyVector(&stone->target, &stone->pos);
			}
			if (MAIN_D_80135244 == 5) {
				if (i == MAIN_D_8013523A && p == MAIN_D_8013523C) {
					stone->rotation.vy += 0x14;
				} else {
					stone->rotation.vy = 0;
				}
			}
			if (stone->state > 0 && stone->speed >= 0) {
				move = stone->weight > 0 ? stone->speed / stone->weight : 0; /* MOD (curling as data): no weights, no movement */
				speed = stone->speed * BK(FRICTION); /* MOD (curling as data) */
				stone->speed = speed / 100;
				if (move < BK(MIN_MOVE)) {
					stone->speed = 0;
				}
				if (stone->state < 3) {
					stone->pos.vx += move * rcos(stone->angle) / 4096;
					stone->pos.vz += move * rsin(stone->angle) / 4096;
				}
				if (stone->state == 3) {
					if (stone->speed > 0) {
						step = move / 10;
						if ((stone->angle -= 0x800) < 0) {
							stone->angle += 0x1000;
						}
						stone->pos.vx = stone->target.vx + step * rcos(stone->angle) / 4096;
						stone->pos.vz = stone->target.vz + step * rsin(stone->angle) / 4096;
						if (step == 0) {
							stone->speed = 0;
						}
					} else {
						stone->speed = 0;
						stone->state = 2;
					}
				}
				if (stone->state == 1 && move <= 0 && stone->type == 3) {
					models = MAIN_D_80134A4C;
					stone->state = 2;
					GsLinkObject4((u_long)(MAIN_D_80135240 + 3), &stone->obj, models.id[p]);
					playSound2(8, 4);
				}
			}
		}
	}

	KAR_updateCollisions();
}

static void KAR_heldPosition(VECTOR *pos)
{
	int32_t idx = MAIN_D_8013523C != 0 ? MAIN_D_80135248 : 0;
	MATRIX world;

	if (idx == 2) {
		KAR_copyVector(pos, &ENTITY_TABLE[idx]->posData->location);
		pos->vx -= 0x50;
	} else {
		/* Evaluate the hand from this pass's skeleton, not a stale workm.
		 * The late tick calls this before interpolation for the logic pose. */
		GsGetLw(&ENTITY_TABLE[idx]->posData[idx == 0 ? 9 : 4].posMatrix, &world);
		pos->vx = world.t[0];
		pos->vy = world.t[1] + 0x78;
		pos->vz = world.t[2];
	}
}

static void KAR_lateStones(int32_t instanceId)
{
	int32_t p, i, n = 0;
	KarStone *stone;
	KarDrawStone *old, *now;

	(void)instanceId;
	for (p = 0; p < 3; p++) {
		stone = KAR_D_8005B5A0[p].row.stones;
		for (i = 0; i < 5; i++, stone++, n++) {
			if (stone->state == 0) {
				KAR_heldPosition(&stone->pos);
				if ((MAIN_D_8013523C != 0 ? MAIN_D_80135248 : 0) != 2) {
					stone->rotation.vx = stone->rotation.vy = stone->rotation.vz = 0;
				}
			}
			old = &KAR_MOD.previous[n];
			now = &KAR_MOD.current[n];
			/* Aggregate assignments call the SDK memcpy once per field.
			 * Copy the live components directly; padding is never rendered. */
			old->pos.vx = now->pos.vx;
			old->pos.vy = now->pos.vy;
			old->pos.vz = now->pos.vz;
			old->rotation.vx = now->rotation.vx;
			old->rotation.vy = now->rotation.vy;
			old->rotation.vz = now->rotation.vz;
			old->state = now->state;
			now->pos.vx = stone->pos.vx;
			now->pos.vy = stone->pos.vy;
			now->pos.vz = stone->pos.vz;
			now->rotation.vx = stone->rotation.vx;
			now->rotation.vy = stone->rotation.vy;
			now->rotation.vz = stone->rotation.vz;
			now->state = stone->state;
			/* Releasing a held stone is continuous; selection, removal and
			 * resets are cuts. setupMatch also invalidates the whole history. */
			KAR_MOD.blend[n] = KAR_MOD.valid &&
				(old->state == now->state || (old->state == 0 && now->state == 1)) &&
				abs(now->pos.vx - old->pos.vx) <= 512 &&
				abs(now->pos.vy - old->pos.vy) <= 512 &&
				abs(now->pos.vz - old->pos.vz) <= 512 &&
				(now->state == 0 || now->pos.vx != old->pos.vx || now->pos.vy != old->pos.vy ||
				 now->pos.vz != old->pos.vz || now->rotation.vx != old->rotation.vx ||
				 now->rotation.vy != old->rotation.vy || now->rotation.vz != old->rotation.vz);
		}
	}
	KAR_MOD.valid = 1;
}

static int32_t KAR_lerp(int32_t a, int32_t b)
{
	return a + (((b - a) * MOD_RENDER_ALPHA + 2048) >> 12);
}

static int16_t KAR_lerpAngle(int16_t a, int16_t b)
{
	int32_t delta = ((b - a) << 20) >> 20;
	return a + ((delta * MOD_RENDER_ALPHA + 2048) >> 12);
}

void KAR_renderScene(int32_t instanceId)
{
	MATRIX lw, ls;
	VECTOR pos;
	SVECTOR rotation;
	VECTOR *drawPos;
	SVECTOR *drawRotation;
	KarStone *stone;
	KarDrawStone *old;
	int32_t p, i, n = 0;
	int32_t view = -1;
	int32_t preview;

	(void)instanceId;
	/* These primitives used to be emitted from KAR_tick, so they vanished
	 * on extra passes. Their animation counters now advance only in the tick. */
	KAR_renderAimArrow();
	KAR_renderStoneCursor();
	KAR_renderScores();
	KAR_renderPowerMeter();
	KAR_renderReadyPrompt();
	KAR_renderNamePlates();
	GsClearOt(0, 0xa, &KAR_D_800638CC[ACTIVE_FRAMEBUFFER]);

	for (p = 0; p < 3; p++) {
		stone = KAR_D_8005B5A0[p].row.stones;
		for (i = 0; i < 5; i++, stone++, n++) {
			if (p == 1 && MAIN_D_80135244 != 3 && MAIN_D_80135244 < 0x10 &&
			    (MAIN_D_80135250 != 0 || MAIN_D_80135244 < 5)) {
				continue;
			}
			if (MAIN_D_80135244 < 4 && MAIN_D_80135252 < i * 0x3c + 0x3c) {
				continue;
			}
			if (stone->state < -0x64 || stone->state >= 0x65) {
				continue;
			}
			drawPos = &stone->pos;
			drawRotation = &stone->rotation;
			if (stone->state == 0) {
				if (MOD_RENDER_ALPHA == 4096 || KAR_MOD.blend[n]) {
					KAR_heldPosition(&pos);
					drawPos = &pos;
				}
			} else if (MOD_RENDER_ALPHA < 4096 && KAR_MOD.blend[n]) {
				old = &KAR_MOD.previous[n];
				pos.vx = KAR_lerp(old->pos.vx, stone->pos.vx);
				pos.vy = KAR_lerp(old->pos.vy, stone->pos.vy);
				pos.vz = KAR_lerp(old->pos.vz, stone->pos.vz);
				rotation.vx = KAR_lerpAngle(old->rotation.vx, stone->rotation.vx);
				rotation.vy = KAR_lerpAngle(old->rotation.vy, stone->rotation.vy);
				rotation.vz = KAR_lerpAngle(old->rotation.vz, stone->rotation.vz);
				drawPos = &pos;
				drawRotation = &rotation;
			}
			RotMatrix(drawRotation, &stone->coord.coord);
			TransMatrix(&stone->coord.coord, drawPos);
			stone->coord.flg = 0;

			/* The fixed preview view and the court view only need rebuilding
			 * when crossing between them, not once for every stone. */
			preview = stone->state < 0;
			if (preview != view) {
				GsSetProjection(preview ? 0x200 : VIEWPORT_DISTANCE);
				GsSetRefView2(preview ? &KAR_D_8005B580 : &GS_VIEWPOINT);
				view = preview;
			}
			GsGetLws(stone->obj.coord2, &lw, &ls);
			GsSetLightMatrix(&lw);
			GsSetLsMatrix(&ls);
			if (preview) {
				GsSortObject4(&stone->obj, &KAR_D_800638CC[ACTIVE_FRAMEBUFFER], 5, getScratchAddr(0));
			} else {
				GsSortObject4(&stone->obj, &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER], 2, getScratchAddr(0));
			}
		}
	}
	GsSortOt(&KAR_D_800638CC[ACTIVE_FRAMEBUFFER], &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
	GsSetProjection(VIEWPORT_DISTANCE);
	GsSetRefView2(&GS_VIEWPOINT);
}

void KAR_finishMatch(void)
{
	if (MAIN_D_80135244 == 0x14) {
		return;
	}

	MAIN_D_80135244 = 0x14;
	ENTITY_TABLE[MAIN_D_80135248]->posData->rotation.vy = 0x800;
	ENTITY_TABLE[1]->posData->rotation.vy = 0x800;
	ENTITY_TABLE[0]->posData->rotation.vy = 0x800;
	startAnimation(ENTITY_TABLE[0], 2);
	startAnimation(ENTITY_TABLE[1], 2);
	startAnimation(ENTITY_TABLE[MAIN_D_80135248], 2);
	SOME_SCRIPT_SYNC_BIT = 1;
	removeObject(0x1388, 0xf);
	setTextColor(1);

	if (KAR_D_8005B5A0[0].row.score > KAR_D_8005B5A0[1].row.score) {
		if (KAR_D_8005B5A0[0].row.score >= BK(BIG_WIN)) {
			writePStat(0x79, 2);
		} else {
			writePStat(0x79, 1);
		}
	} else {
		writePStat(0x79, 0);
	}
}

/* The original force-upload was cheap with the whole TFS in RAM. The
 * streaming build otherwise rereads three CD rows on every qualifying camera
 * tick. Keep cells already in VRAM, and let the ring serve actual replacements. */
extern int8_t PREV_TILE_Y;

void KAR_flushTiles(void)
{
	MapTileData *tiles = MAP_TILE_DATA.tiles;
	int32_t stride = MAP_TILE_DATA.width;
	int32_t count = stride > 3 ? 4 : stride;
	int32_t first = MAP_TILE_X + MAP_TILE_Y * stride;
	int32_t row, col, tile, cell;
	RECT rect;

	for (row = 0; row < 3 && row < MAP_TILE_DATA.height; row++) {
		for (col = 0; col < count; col++) {
			tile = first + row * stride + col;
			rect.x = tiles[tile].texU;
			rect.y = tiles[tile].texV;
			rect.w = 64;
			rect.h = 128;
			cell = ((rect.x - 768) >> 6) + (rect.y >> 7) * 4;
			if (cell >= 0 && cell < 12 &&
			    TFS_CELL[cell] == (tiles[tile].tileId == -1 ? -1 : tile)) {
				continue;
			}
			if (tiles[tile].tileId == -1) {
				tfsClearTile(&rect);
			} else {
				tfsLoadTile(&rect, tile);
			}
			DrawSync(0);
		}
	}
	PREV_TILE_Y = MAP_TILE_Y;
}

void KAR_tick(void)
{
	int32_t player;
	uint32_t tick = (uint32_t)MOD_PERF_TICK_SUM - KAR_MOD.tickSum;
	uint32_t render = (uint32_t)MOD_PERF_RENDER_SUM - KAR_MOD.renderSum;

	KAR_MOD.tickSum = MOD_PERF_TICK_SUM;
	KAR_MOD.renderSum = MOD_PERF_RENDER_SUM;
	/* Decide once during the intro, then keep a steady cadence for the
	 * match. Ignore entry/loading ticks and an external perf-counter reset.
	 * Allow half a render of headroom for the HUD and previews appearing
	 * after this intro. At original CPU speed the participants cost most of a
	 * refresh; faster CPUs may keep the player's 60 fps request. */
	if (KAR_MOD.warmup < 10) {
		if (KAR_MOD.warmup >= 3 && tick < 2000 && render < 2000) {
			KAR_MOD.samples++;
			if (tick + render + render / 2 > 245) {
				KAR_MOD.overBudget = 1;
			}
		}
		if (++KAR_MOD.warmup == 10) {
			KAR_MOD.steady30 = KAR_MOD.overBudget || KAR_MOD.samples == 0;
		}
	}
	MOD_RENDER_CADENCE = 4;
	if (MOD_FPS_TEST_TICK == 0 && MOD_FPS_MODE > 0 &&
		(KAR_MOD.steady30 || MOD_FPS_MODE == 1)) {
		MOD_RENDER_CADENCE = (MOD_FPS_TICKS & 1) ? 1 : 2;
	}

	KAR_tickMatchState();
	KAR_MOD.hudState = MAIN_D_80135244;
	KAR_MOD.arrowBlend = KAR_MOD.arrowReady;
	KAR_MOD.arrowReady = MAIN_D_80135244 >= 6 && MAIN_D_80135244 < 8;
	KAR_MOD.arrowBlend &= KAR_MOD.arrowReady;
	/* These were advanced by the tick's drawing functions. Keep their exact
	 * order and integer behaviour, but draw their result on every pass. */
	if (MAIN_D_80135244 >= 6 && MAIN_D_80135244 < 8) {
		if (MAIN_D_80135252-- != 0) {
			MAIN_D_80135252 += 0xbf;
		}
		if (MAIN_D_80135238++ >= 0x28) {
			MAIN_D_80135238 = 0;
		}
		player = MAIN_D_8013523C != 0 ? MAIN_D_80135248 : 0;
		KAR_copyVector(&KAR_MOD.previousArrowOrigin, &KAR_MOD.arrowOrigin);
		KAR_MOD.previousArrowRotation = KAR_MOD.arrowRotation;
		KAR_copyVector(&KAR_MOD.arrowOrigin, &KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].pos);
		KAR_MOD.arrowRotation = ENTITY_TABLE[player]->posData->rotation;
		KAR_MOD.arrowBlend &=
			abs(KAR_MOD.arrowOrigin.vx - KAR_MOD.previousArrowOrigin.vx) <= 512 &&
			abs(KAR_MOD.arrowOrigin.vz - KAR_MOD.previousArrowOrigin.vz) <= 512;
	}
	if (MAIN_D_80135244 == 3) {
		if (MAIN_D_80135220++ >= 0x31) {
			MAIN_D_80135220 = 0x30;
		}
	} else {
		MAIN_D_80135220 = 0;
	}
	KAR_handleAimScroll();
	KAR_updateRingMarkers();
	KAR_checkStonesStopped();
}

void KAR_renderAimArrow(void)
{
	MATRIX m;
	VECTOR origin;
	SVECTOR rotation;
	SVECTOR out;
	SVECTOR prev;
	SVECTOR vec;
	GsOT_TAG *ot;
	int32_t otz;
	int32_t i;
	DVECTOR sxy;
	SVECTOR pts[4];
	POLY_FT4 *prim;
	int32_t rot;
	int32_t k;
	int16_t w;
	uint8_t shade;

	if (KAR_MOD.hudState < 6) {
		return;
	}
	if (KAR_MOD.hudState >= 8) {
		return;
	}

	ot = ACTIVE_ORDERING_TABLE->org;

	KAR_copyVector(&origin, &KAR_MOD.arrowOrigin);
	rotation = KAR_MOD.arrowRotation;
	if (MOD_RENDER_ALPHA < 4096 && KAR_MOD.arrowBlend) {
		origin.vx = KAR_lerp(KAR_MOD.previousArrowOrigin.vx, origin.vx);
		origin.vz = KAR_lerp(KAR_MOD.previousArrowOrigin.vz, origin.vz);
		rotation.vx = KAR_lerpAngle(KAR_MOD.previousArrowRotation.vx, rotation.vx);
		rotation.vy = KAR_lerpAngle(KAR_MOD.previousArrowRotation.vy, rotation.vy);
		rotation.vz = KAR_lerpAngle(KAR_MOD.previousArrowRotation.vz, rotation.vz);
	}
	RotMatrix(&rotation, &m);
	for (i = 0; i < 0xf; i++) {
		vec.vx = 0;
		vec.vy = 0;
		vec.vz = (i + 1) * -200;
		ApplyMatrixSV(&m, &vec, &out);
		out.vx += origin.vx;
		out.vz += origin.vz;
		if ((shade = MAIN_D_80135252 + (i * 0xbf) / 15) >= 0xbf) {
			shade -= 0xbf;
		}
		while ((out.vx < -0x2ee) || (out.vx >= 0x2ef)) {
			if (out.vx < -0x2ee) {
				w = out.vx + 0x2ee;
				out.vx -= w * 2;
			}
			if (out.vx >= 0x2ef) {
				w = out.vx - 0x2ee;
				out.vx -= w * 2;
			}
		}
		prim = (POLY_FT4 *)GsGetWorkBase();
		SetPolyFT4(prim);
		GsSetLsMatrix(&GsWSMATRIX);
		setRGB0(prim, shade + 0x40, shade + 0x40, shade + 0x40);
		gte_ldv0(&out);
		gte_rtps();
		gte_stsxy(&sxy);
		gte_stszotz(&otz);
		if (i < 0xe) {
			setUVWH(prim, 0x18, 0x38, 16, -16);
			setXY4(prim, sxy.vx - 8, sxy.vy + 8, sxy.vx + 8, sxy.vy + 8, sxy.vx - 8,
			       sxy.vy - 8, sxy.vx + 8, sxy.vy - 8);
		} else {
			rot = ratan2(prev.vz - out.vz, prev.vx - out.vx);
			rot = rot - 0xc00;
			rot = rot % 0x1000;
			rot = -rot;
			pts[0].vx = -8;
			pts[0].vy = 0;
			pts[0].vz = 8;
			pts[1].vx = 8;
			pts[1].vy = 0;
			pts[1].vz = 8;
			pts[2].vx = -8;
			pts[2].vy = 0;
			pts[2].vz = -8;
			pts[3].vx = 8;
			pts[3].vy = 0;
			pts[3].vz = -8;
			for (k = 0; k < 4; k++) {
				KAR_rotatePoint(&pts[k], rot);
			}
			for (k = 0; k < 4; k++) {
				pts[k].vx += sxy.vx;
				pts[k].vz += sxy.vy;
			}
			if (rot == 0x800) {
				setUVWH(prim, 0x28, 0x28, 16, 16);
			} else {
				setUVWH(prim, 0x28, 0x28, 15, 15);
			}
			setXY4(prim, pts[0].vx, pts[0].vz, pts[1].vx, pts[1].vz, pts[2].vx,
			       pts[2].vz, pts[3].vx, pts[3].vz);
		}
		setClut(prim, 0x80, 0x1e6);
		prim->tpage = getTPage(0, 0, 640, 0);
		if (i != (MAIN_D_80135238 / 3)) {
			AddPrim(ot + otz, prim);
		}
		prim++;
		GsSetWorkBase((PACKET *)prim);
		prev = out;
	}
}

void KAR_renderStoneCursor(void)
{
	KarSprite sprite;
	KarStone *stone;

	sprite = KAR_D_8005B478;
	stone = KAR_D_8005B5A0[0].row.stones;
	if (KAR_MOD.hudState == 5) {
		sprite.x = (MAIN_D_8013523C != 0) ? 0x64 : -0x78;
		sprite.y = (MAIN_D_8013523A * 30) - 0x2d;
		KAR_renderSprite(&sprite);
	}
}

void KAR_renderScores(void)
{
	KarSprite spr;
	KarDigits dig;
	int32_t i;
	int16_t lo;
	int16_t hi;
	int16_t x;

	spr = KAR_D_8005B48C;
	dig = KAR_D_8005B4A0;
	for (i = 0; i < 2; i++) {
		lo = KAR_D_8005B5A0[i].row.score % 10;
		hi = KAR_D_8005B5A0[i].row.score / 10;
		x = (i != 0) ? 0x68 : -0x69;
		if (KAR_D_8005B5A0[i].row.score < 0) {
			hi = 10;
		}
		if (lo < 0) {
			lo *= -1;
		}
		spr.u = dig.glyph[lo].u;
		spr.v = dig.glyph[lo].v;
		spr.x = x;
		spr.y = -0x3e;
		KAR_renderSprite(&spr);

		spr.u = dig.glyph[hi].u;
		spr.v = dig.glyph[hi].v;
		spr.x = x - 0x10;
		spr.y = -0x3e;
		KAR_renderSprite(&spr);
	}
}

void KAR_renderPowerMeter(void)
{
	KarSpritePair arr;
	int32_t i;
	int16_t step;

	arr = KAR_D_8005B4CC;
	step = 250;
	if ((KAR_MOD.hudState >= 8) && (KAR_MOD.hudState < 0xa)) {
		arr.sprite[1].x = (MAIN_D_8013523C != 0) ? 0x32 : -0x32;
		arr.sprite[1].clutX = (MAIN_D_8013523C != 0) ? 0x80 : 0x70;

		for (i = 10; i > 0; i--) {
			if ((step * i) <= MAIN_D_8013523E) {
				arr.sprite[0].x = (arr.sprite[1].x + i * 3) - 3;
				if (i == 10) {
					arr.sprite[0].clutX = 0x30;
				} else {
					arr.sprite[0].clutX = 0x70;
				}
				KAR_renderSprite(&arr.sprite[0]);
			}
		}
		KAR_renderSprite(&arr.sprite[1]);
	}
}

void KAR_renderReadyPrompt(void)
{
	KarSprite sprite;

	sprite = KAR_D_8005B56C;
	if (KAR_MOD.hudState == 3) {
		if ((MAIN_D_80135220 & 8) != 0) {
			KAR_renderSprite(&sprite);
		}
	}
}

void KAR_renderNamePlates(void)
{
	KarSpriteSet set;
	KarSprite extra;
	KarSprite *p;
	int32_t i;

	set = KAR_D_8005B4F4;
	extra = KAR_D_8005B558;
	p = set.sprite;
	if (MAIN_D_80135248 == 3) {
		p[2] = extra;
	}

	for (i = 0; i < 4; i++, p++) {
		KAR_renderSprite(p);
	}
}

void KAR_handleAimScroll(void)
{
	int16_t dx;
	int16_t dy;

	if (MAIN_D_80135244 != 6) {
		return;
	}
	dx = 0;
	dy = 0;
	if (MAIN_D_8013523C != 0) {
		return;
	}
	if ((MAIN_D_80135254 >= 10) || (MAIN_D_80135254 < -9) ||
	    (MAIN_D_80135250 != 0)) {
		if (POLLED_INPUT & 0x8000) {
			dx -= 8;
		}
		if (POLLED_INPUT & 0x2000) {
			dx += 8;
		}
	}
	if (POLLED_INPUT & 0x1000) {
		dy -= 10;
	}
	if (POLLED_INPUT & 0x4000) {
		dy += 10;
	}
	if ((dx != 0) || (dy != 0)) {
		moveCameraByOffset(dx, dy);
	}
}

void KAR_updateRingMarkers(void)
{
	int32_t zoneIndex;
	KarRingObjects objects;
	KarRingFlags flags;
	KarZones zones;
	int32_t distance;
	int8_t mask;
	int32_t p;
	int32_t stoneIndex;
	KarStone *stone;

	objects = KAR_D_8005B450;
	flags = MAIN_D_80134A48;
	zones = KAR_D_8005B460;
	if (MAIN_D_80135244 == 0xB) {
		MAIN_D_80135257 = 0;
		for (p = 0; p < 2; p++) {
			p = p;
			stone = KAR_D_8005B5A0[p].row.stones;
			for (stoneIndex = 0; stoneIndex < 5; stoneIndex++, stone++) {
				if ((stone->state > 0) && (stone->speed > 0)) {
					for (zoneIndex = 0; zoneIndex < 4; zoneIndex++) {
						/* This never-null guard preserves the compiler's loop weighting. */
						if (!objects.ring) {
						}
						mask = (MAIN_D_80135257 >> zoneIndex) & 1;
						if (mask == 0) {
							distance = KAR_distance(stone->pos.vx + zones.ring[zoneIndex].dx,
							                        stone->pos.vz + zones.ring[zoneIndex].dz);
							if (distance < zones.ring[zoneIndex].radius) {
								mask = 1;
								mask <<= (int8_t)zoneIndex;
								MAIN_D_80135257 |= (int8_t)mask;
								zoneIndex = 4;
							}
						}
					}
				}
			}
		}
		for (p = 0; p < 4; p++) {
			if ((MAIN_D_80135257 >> p) & 1) {
				flags.ring[p] = 1;
			}
			setMapObjectsFlag(objects.ring[p].start, objects.ring[p].count, flags.ring[p] ^ 1);
		}
	}
	if (MAIN_D_80135244 == 0xD) {
		int8_t ring;

		if ((KAR_D_800639C0[MAIN_D_80135256]->ring != KAR_D_800639C0[MAIN_D_80135256]->prevRing) &&
		    (KAR_D_800639C0[MAIN_D_80135256]->state < 0x65)) {
			ring = KAR_D_800639C0[MAIN_D_80135256]->ring & 0xF;
			if (ring != 0) {
				flags.ring[ring - 1] = 1;
			}
		}
		for (stoneIndex = 0; stoneIndex < 4; stoneIndex++) {
			setMapObjectsFlag(objects.ring[stoneIndex].start, objects.ring[stoneIndex].count,
			                  flags.ring[stoneIndex] ^ 1);
		}
	}
	if ((MAIN_D_80135244 != 0xD) && (MAIN_D_80135244 != 0xB)) {
		for (stoneIndex = 0; stoneIndex < 4; stoneIndex++) {
			setMapObjectsFlag(objects.ring[stoneIndex].start, objects.ring[stoneIndex].count,
			                  flags.ring[stoneIndex] ^ 1);
		}
	}
}

void KAR_checkStonesStopped(void)
{
	int32_t p;
	int32_t n;
	int8_t moving;
	KarStone *stone;
	KarStone *stone2;

	moving = 0;
	if (MAIN_D_80135244 != 0xb) {
		return;
	}
	for (p = 0; p < 3; p++) {
		p = p;
		stone = KAR_D_8005B5A0[p].row.stones;
		for (n = 0; n < 5; n++, stone++) {
			if (stone->state > 0) {
				if (stone->speed > 0) {
					moving = 1;
					break;
				}
				stone->speed = 0;
			}
		}
	}
	if (moving == 0) {
		MAIN_D_80135244 = 0xc;
	}
	for (p = 0; p < 3; p++) {
		stone2 = KAR_D_8005B5A0[p].row.stones;
		stone2 = stone2;
		for (n = 0; n < 5; n++, stone2++) {
			if (stone2->state > 0) {
				if (stone2->pos.vz >= 0xbe) {
					if (stone2->speed <= 0) {
						stone2->state = -0x65;
					}
				}
			}
		}
	}
}

void KAR_updateCollisions(void)
{
	KarStone *active[15];
	KarStone *stone;
	VECTOR cur[15], prev[15];
	int32_t count = 0;
	int32_t p, i, a, b, step;
	int32_t tx1, tx2, tz1, tz2, dist;
	int8_t collided = 0;

	KAR_bounceOffWall();
	if (MAIN_D_80135244 != 0xb) {
		return;
	}
	/* Collision resolution only changes positive states to other positive
	 * states (the pegged stone becomes 3). Keep all positive stones, even
	 * stationary ones, in the original row/slot order. No pairs are pruned. */
	for (p = 0; p < 3; p++) {
		stone = KAR_D_8005B5A0[p].row.stones;
		for (i = 0; i < 5; i++, stone++) {
			if (stone->state > 0) {
				active[count] = stone;
				cur[count].vy = prev[count].vy = 0;
				cur[count].pad = prev[count].pad = 0;
				count++;
			}
		}
	}
	for (step = 0; step < 0xb; step++) {
		for (i = 0; i < count; i++) {
			stone = active[i];
			tx1 = stone->target.vx * (10 - step);
			tx2 = stone->pos.vx * step;
			tz1 = stone->target.vz * (10 - step);
			tz2 = stone->pos.vz * step;
			cur[i].vx = (tx1 + tx2) / 10;
			cur[i].vz = (tz1 + tz2) / 10;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			cur[i].vy = stone->target.vy;
#endif
		}
		for (a = 0; a < count; a++) {
			for (b = a + 1; b < count; b++) {
				/* Keep the SDK approximation and its exact cutoff. */
				dist = KAR_distance(cur[a].vx - cur[b].vx, cur[a].vz - cur[b].vz);
				if (dist < 0x96) {
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
					collided = 1;
#endif
					if (step == 0) {
						KAR_resolveStoneCollision(active[b], cur[b], active[a], cur[a]);
					} else {
						KAR_resolveStoneCollision(active[b], prev[b], active[a], prev[a]);
					}
				}
			}
		}
		for (i = 0; i < count; i++) {
			prev[i].vx = cur[i].vx;
			prev[i].vy = cur[i].vy;
			prev[i].vz = cur[i].vz;
			prev[i].pad = cur[i].pad;
		}
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
		if (collided != 0 && step == 0xa) {
			step--;
		}
		collided = 0;
#endif
	}
}

void KAR_bounceOffWall(void)
{
	int32_t p;
	int32_t n;
	VECTOR contact;
	int16_t zone;
	KarStone *stone;
	int16_t d;

	for (p = 0; p < 3; p++) {
		stone = KAR_D_8005B5A0[p].row.stones;
		for (n = 0; n < 5; n++, stone++) {
			if (stone->state <= 0) {
				continue;
			}
			if (stone->speed == 0) {
				continue;
			}
			zone = KAR_getWallZone(stone->pos.vx, stone->pos.vz);
			if ((zone > 0) && (zone < 6)) {
				playSound2(8, 3);
			}
			switch (zone) {
			case 1:
				d = stone->pos.vx + 0x2d5;
				stone->pos.vx -= d * 2;
				stone->angle = 0x800 - stone->angle;
				break;
			case 2:
				stone->angle = 0x1800 - stone->angle;
				d = stone->pos.vx - 0x2d5;
				stone->pos.vx -= d * 2;
				break;
			case 3:
				KAR_findWallContact(&contact, stone, 0);
				KAR_reflectOffDiagonal((int32_t *)stone, 0);
				KAR_placeAtContact(stone, contact);
				break;
			case 4:
				KAR_findWallContact(&contact, stone, 1);
				KAR_reflectOffDiagonal((int32_t *)stone, 1);
				KAR_placeAtContact(stone, contact);
				break;
			case 5:
				stone->angle = -stone->angle;
				d = -0x9dd - stone->pos.vz;
				stone->pos.vz += d * 2;
				break;
			case 6:
				if (stone->target.vz < 0x465) {
					playSound2(8, 3);
					stone->angle = -stone->angle;
					d = stone->pos.vz - 0x465;
					stone->pos.vz -= d * 2;
				}
				break;
			}
		}
	}
}

int32_t KAR_distance(long x, long y)
{
	int32_t d;
	uint32_t xx;
	uint32_t yy;

	xx = x * x;
	yy = y * y;
	d = SquareRoot0(xx + yy);
	return d;
}

void KAR_resolveStoneCollision(KarStone *stoneA, VECTOR a, KarStone *stoneB, VECTOR b)
{
	int16_t speed;

	speed = stoneA->speed + stoneB->speed;
	if (speed >= 0xbb8) {
		playSound2(8, 1);
	} else {
		playSound2(8, 2);
	}

	if (stoneA->state == 2 || stoneA->state == 3 || stoneB->state == 2 || stoneB->state == 3) {
		if (stoneA->state == 2 || stoneA->state == 3) {
			KAR_collidePeggedStone(stoneA, a, stoneB, b);
		} else {
			KAR_collidePeggedStone(stoneB, b, stoneA, a);
		}
	} else if (stoneA->speed <= 0 || stoneB->speed <= 0) {
		if (stoneA->speed <= 0) {
			KAR_collideRestingStone(stoneA, a, stoneB, b);
		} else {
			KAR_collideRestingStone(stoneB, b, stoneA, a);
		}
	} else {
		KAR_collideMovingStones(stoneB, b, stoneA, a);
	}
}

void KAR_collidePeggedStone(KarStone *pegged, VECTOR a, KarStone *mover, VECTOR b)
{
	int32_t ang1;
	int32_t dd;
	int32_t da;
	int32_t r;
	int32_t t;
	int32_t dist;

	KAR_debugCollisionStone(mover, &b, &a);
	ang1 = ratan2(mover->target.vz - mover->pos.vz, mover->target.vx - mover->pos.vx) & 0xfff;
	r = ratan2(b.vz - a.vz, b.vx - a.vx) & 0xfff;
	da = r * 2 - ang1;
	if (da >= 0x1000) {
		da = da % 0x1000;
	}

	dd = abs(r - ang1);
	dd = dd % 0x400;
	dd = 0x400 - dd;
	r = KAR_distance(mover->pos.vx - b.vx, mover->pos.vz - b.vz);
	r = dd * r / 0x400;
	do {
		mover->pos.vx = b.vx + r * rcos(da) / 4096;
		mover->pos.vz = b.vz + r * rsin(da) / 4096;
		r++;
		dist = KAR_distance(mover->pos.vx - pegged->pos.vx, mover->pos.vz - pegged->pos.vz);
	} while (dist < 0x97);

	t = (mover->speed >> 1) * dd / 0x400;
	pegged->speed = t;
	mover->speed = mover->speed - t;
	pegged->angle = ratan2(mover->target.vz - b.vz, mover->target.vx - b.vx);
	mover->angle = da;
	pegged->state = 3;
	mover->target = b;
}

void KAR_collideRestingStone(KarStone *resting, VECTOR a, KarStone *mover, VECTOR b)
{
	int32_t share;
	int32_t d;
	int32_t ang;
	int32_t n;
	VECTOR p1;
	VECTOR p2;
	int32_t r;

	KAR_debugCollisionStone(mover, &b, &a);
	if (resting->speed == 0 && mover->speed == 0) {
		resting->pos = a;
		mover->pos = b;
		return;
	}

	share = KAR_computeImpactShare(mover, b, a);
	KAR_computeSeparation(&p1, mover, b, a);
	resting->pos.vx = resting->pos.vx + p1.vx;
	resting->pos.vz = resting->pos.vz + p1.vz;
	mover->pos.vx = mover->pos.vx - p1.vx;
	mover->pos.vz -= p1.vz;
	d = KAR_distance(resting->pos.vx - mover->pos.vx, resting->pos.vz - mover->pos.vz);
	ang = ratan2(p1.vz, p1.vx);
	n = KAR_distance(p1.vx, p1.vz);
	r = 0;
	p1 = resting->pos;
	p2 = mover->pos;
	if (n != 0) {
		while (d < 0x97) {
			resting->pos.vx = p1.vx + r * rcos(ang) / 4096;
			resting->pos.vz = p1.vz + r * rsin(ang) / 4096;
			mover->pos.vx = p2.vx - r * rcos(ang) / 4096;
			mover->pos.vz = p2.vz - r * rsin(ang) / 4096;
			d = KAR_distance(resting->pos.vx - mover->pos.vx, resting->pos.vz - mover->pos.vz);
			r++;
		}
	} else {
		int32_t pushAng;

		pushAng = ratan2(mover->pos.vz - resting->pos.vz, mover->pos.vx - resting->pos.vx);
		while (d < 0x97) {
			mover->pos.vx = p2.vx + r * rcos(pushAng) / 4096;
			mover->pos.vz = p2.vz + r * rsin(pushAng) / 4096;
			d = KAR_distance(resting->pos.vx - mover->pos.vx, resting->pos.vz - mover->pos.vz);
			r++;
		}
	}

	resting->target = a;
	mover->target = b;
	mover->speed = mover->speed - share;
	resting->speed = share;
	resting->angle = ratan2(resting->pos.vz - a.vz, resting->pos.vx - a.vx) & 0xfff;
	mover->angle = ratan2(mover->pos.vz - b.vz, mover->pos.vx - b.vx) & 0xfff;
}

void KAR_collideMovingStones(KarStone *stoneA, VECTOR a, KarStone *stoneB, VECTOR b)
{
	int32_t ang;
	int32_t d;
	int32_t share1;
	int32_t share2;
	VECTOR p1;
	VECTOR p2;
	VECTOR delta;
	int32_t r;

	KAR_debugCollisionPair(stoneA, &a, stoneB, &b);
	share1 = KAR_computeImpactShare(stoneA, a, b);
	share2 = KAR_computeImpactShare(stoneB, b, a);
	KAR_computeSeparation(&p1, stoneB, b, a);
	KAR_computeSeparation(&p2, stoneA, a, b);
	delta.vx = p2.vx - p1.vx;
	delta.vz = p2.vz - p1.vz;
	ang = KAR_distance(delta.vx, delta.vz);
	if (ang != 0) {
		ang = ratan2(delta.vz, delta.vx);
		stoneB->pos.vx = stoneB->pos.vx + delta.vx;
		stoneB->pos.vz = stoneB->pos.vz + delta.vz;
		stoneA->pos.vx = stoneA->pos.vx - delta.vx;
		stoneA->pos.vz -= delta.vz;
		d = KAR_distance(stoneA->pos.vx - stoneB->pos.vx, stoneA->pos.vz - stoneB->pos.vz);
		r = 0;
		p1 = stoneA->pos;
		p2 = stoneB->pos;
		stoneA->speed += (int16_t)(share2 - share1);
		stoneB->speed += (int16_t)(share1 - share2);
		if (stoneA->speed > stoneB->speed) {
			while (d < 0x97) {
#if VERSION_IS(JP)
				stoneA->pos.vx = p1.vx - r * rcos(ang) / 4096;
				stoneA->pos.vz = p1.vz - r * rsin(ang) / 4096;
#else
				stoneA->pos.vx = p1.vx + r * rcos(ang) / 4096;
				stoneA->pos.vz = p1.vz + r * rsin(ang) / 4096;
#endif
				d = KAR_distance(stoneB->pos.vx - stoneA->pos.vx, stoneB->pos.vz - stoneA->pos.vz);
				r++;
			}
		} else {
			while (d < 0x97) {
				stoneB->pos.vx = p2.vx + r * rcos(ang) / 4096;
				stoneB->pos.vz = p2.vz + r * rsin(ang) / 4096;
				d = KAR_distance(stoneB->pos.vx - stoneA->pos.vx, stoneB->pos.vz - stoneA->pos.vz);
				r++;
			}
		}
		stoneA->target = a;
		stoneB->target = b;
		stoneA->angle = ratan2(stoneA->pos.vz - a.vz, stoneA->pos.vx - a.vx);
		stoneB->angle = ratan2(stoneB->pos.vz - b.vz, stoneB->pos.vx - b.vx);
	} else {
		d = KAR_distance(stoneA->pos.vx - stoneB->pos.vx, stoneA->pos.vz - stoneB->pos.vz);
		if (stoneB->speed / stoneA->weight < 5 || stoneA->speed / stoneB->weight < 5) {
			stoneB->speed = 0;
			stoneB->pos = stoneB->target;
			stoneA->pos = stoneA->target;
		} else {
			stoneB->pos = b;
			stoneA->pos = a;
		}
	}
}

void KAR_debugCollisionStone(KarStone *stone, VECTOR *b, VECTOR *a)
{
	int32_t r;
	int32_t ang2;
	int32_t ang;
	int32_t flag;
	int32_t dist;
	VECTOR base;

	flag = 1;
	base = *b;
	return;

	r = 0;
	ang2 = stone->angle;
	ang2 = (ang2 + 0x800) & 0xfff;
	do {
		flag = (flag != 0) ? 0 : 1;
		if (flag == 0) {
			b->vx = base.vx + r * rcos(ang) / 4096;
			b->vz = base.vz + r * rsin(ang) / 4096;
		} else {
			b->vx = base.vx + r * rcos(ang2) / 4096;
			b->vz = base.vz + r * rsin(ang2) / 4096;
		}
		dist = KAR_distance(b->vx - a->vx, b->vz - a->vz);
	} while (dist < 0x97);
}

int32_t KAR_computeImpactShare(KarStone *stone, VECTOR a, VECTOR b)
{
	int32_t ang;
	int32_t impactAng;
	int32_t d;
	int32_t ret;

	ang = stone->angle & 0xfff;
	impactAng = ratan2(b.vz - a.vz, b.vx - a.vx) & 0xfff;
	d = (ang - impactAng) & 0xfff;
	ret = 0;
	if (d < 0x400 || d > 0xc00) {
		if (d > 0xc00) {
			d = (0x1000 - d) & 0xfff;
		}
		d = 0x400 - (d % 0x400);
		ret = d * stone->speed / 0x400;
	}

	return ret;
}

int32_t KAR_computeSeparation(VECTOR *out, KarStone *stone, VECTOR a, VECTOR b)
{
	int32_t cur;
	int32_t ang;
	int32_t d;
	int32_t r;
	int32_t dist;
	int32_t n;
	VECTOR v;

	cur = stone->angle & 0xfff;
	ang = ratan2(b.vz - a.vz, b.vx - a.vx) & 0xfff;
	d = abs(cur - ang);
	v.vz = 0;
	v.vy = 0;
	v.vx = 0;
	if (d < 0x400 || d > 0xc00) {
		d = d % 0x400;
		dist = KAR_distance(stone->pos.vz - a.vz, stone->pos.vx - a.vx);
		r = (0x400 - d) * dist / 0x400;
		do {
			v.vx = r * rcos(ang) / 4096;
			v.vz = r * rsin(ang) / 4096;
			n = KAR_distance(v.vx, v.vz);
			r++;
		} while (n == 0);
	}

	*out = v;
}

void KAR_debugCollisionPair(KarStone *stoneA, VECTOR *a, KarStone *stoneB, VECTOR *b)
{
	VECTOR baseA;
	VECTOR baseB;
	int32_t angA2;
	int32_t angB2;
	int32_t angA;
	int32_t angB;
	int32_t dist;
	int32_t flag;
	int32_t r;

	flag = 1;
	return;

	baseA = *a;
	baseB = *b;
	angA = stoneA->angle;
	angB = stoneB->angle;
	angA2 = (angA + 0x800) & 0xfff;
	angB2 = (angB + 0x800) & 0xfff;
	r = 0;
	do {
		flag = (flag != 0) ? 0 : 1;
		if (flag == 0) {
			a->vx = baseA.vx + r * rcos(angA) / 4096;
			a->vz = baseA.vz + r * rsin(angA) / 4096;
			b->vx = baseB.vx + r * rcos(angB) / 4096;
			b->vz = baseB.vz + r * rsin(angB) / 4096;
		} else {
			a->vx = baseA.vx + r * rcos(angA2) / 4096;
			a->vz = baseA.vz + r * rsin(angA2) / 4096;
			b->vx = baseB.vx + r * rcos(angB2) / 4096;
			b->vz = baseB.vz + r * rsin(angB2) / 4096;
		}
		dist = KAR_distance(a->vx - b->vx, a->vz - b->vz);
		if (r >= 0x96) {
			*a = baseA;
			*b = baseB;
			break;
		}
	} while (dist < 0x97);
}

int32_t KAR_getWallZone(int16_t x, int16_t z)
{
	if (z >= 0x465) {
		return 6;
	}

	if (z >= -0x7d0) {
		if (x < -0x2d4) {
			return 1;
		}
		if (x >= 0x2d5) {
			return 2;
		}
	} else {
		if (z + (x * 2) < -0xdc4 && x < 0x1aa) {
			return 3;
		}
		if (z - (x * 2) < -0xdc4 && x >= 0x1a9) {
			return 4;
		}
		if (z < -0x9dc) {
			return 5;
		}
	}

	return 0;
}

// clang-format off
int32_t KAR_findWallContact(out, stone, flag)
	VECTOR *out;
	KarStone *stone;
	int8_t flag;
// clang-format on
{
	int32_t t;
	int32_t i;
	VECTOR v;
	int32_t ang;

	i = 0;
	ang = (stone->angle + 0x800) & 0xfff;
	do {
		v.vx = stone->pos.vx + i * rcos(ang) / 4096;
		v.vz = stone->pos.vz + i * rsin(ang) / 4096;
		i++;
		if (flag == 0) {
			t = (v.vz + 0xdc5) + v.vx * 2;
		} else {
			t = (v.vz + 0xdc5) - v.vx * 2;
		}
	} while (t <= 0);

	*out = v;
}

// clang-format off
void KAR_reflectOffDiagonal(p, dir)
	int32_t *p;
	int8_t dir;
// clang-format on
{
	int32_t cur;
	int32_t ang;

	cur = (p[3] + 0x800) & 0xfff;
	if (dir == 0) {
		ang = ratan2(1, 2);
	} else {
		ang = ratan2(1, -2);
	}

	p[3] = ang * 2 - cur;
}

void KAR_placeAtContact(KarStone *stone, VECTOR p)
{
	int32_t d;

	d = KAR_distance(p.vx - stone->target.vx, p.vz - stone->target.vz);
	d = KAR_distance(stone->target.vz - stone->pos.vz, stone->target.vx - stone->pos.vx) - d;
	stone->pos.vx = p.vx + ((d * rcos(stone->angle)) / 4096);
	stone->pos.vz = p.vz + ((d * rsin(stone->angle)) / 4096);
}

void KAR_beginAiming(void)
{
	int8_t idx;
	int8_t n;

	idx = (MAIN_D_8013523C != 0) ? MAIN_D_80135248 : 0;

	MAIN_D_80135244 = 6;
	if (idx == 0) {
		startAnimation(ENTITY_TABLE[0], 0x23);
	} else if (MAIN_D_80135248 == 2) {
		startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0x1c);
	}

	KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].state = 0;
	if (KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].type == 3) {
		n = (MAIN_D_8013523C != 0) ? 1 : 2;
		GsLinkObject4((u_long)(MAIN_D_80135240 + 3), &KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].obj, n);
	}

	MAIN_D_8013524C = -0x400;
	ENTITY_TABLE[idx]->posData->rotation.vy = 0;
}

void KAR_selectPreviousStone(void)
{
	int32_t row;

	if (MAIN_D_8013523A > 0) {
		for (row = MAIN_D_8013523A - 1; row >= 0; row--) {
			if (KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[row].state == -1) {
				MAIN_D_8013523A = row;
				return;
			}
		}
	}
}

void KAR_selectNextStone(void)
{
	int32_t row;

	if (MAIN_D_8013523A < 4) {
		for (row = MAIN_D_8013523A + 1; row < 5; row++) {
			if (KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[row].state == -1) {
				MAIN_D_8013523A = row;
				return;
			}
		}
	}
}

// clang-format off
void KAR_turnAimLeft(delta)
	int16_t delta;
// clang-format on
{
	int8_t idx;

	idx = (MAIN_D_8013523C != 0) ? MAIN_D_80135248 : 0;
	ENTITY_TABLE[idx]->posData->rotation.vy -= delta;
	MAIN_D_8013524C += delta;
}

// clang-format off
void KAR_turnAimRight(delta)
	int16_t delta;
// clang-format on
{
	int8_t idx;

	idx = (MAIN_D_8013523C != 0) ? MAIN_D_80135248 : 0;
	ENTITY_TABLE[idx]->posData->rotation.vy -= delta;
	MAIN_D_8013524C += delta;
}

void KAR_beginThrow(void)
{
	MAIN_D_80135244 = 7;
	if (MAIN_D_8013523C == 0) {
		startAnimation(ENTITY_TABLE[0], 0x24);
	} else if (MAIN_D_80135248 == 2) {
		startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0x1d);
	} else {
		startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0x1f);
	}
}

int8_t KAR_tickYesNoPrompt(void)
{
	RECT finalPos;
	RECT startPos;
	KarStrPair strs;

	finalPos = MAIN_D_80134A10;
	startPos = MAIN_D_80134A18;
	strs = MAIN_D_80134A20;
	while (MAIN_D_80135250 < 2) {
		DRAW_STRING_PIECES(strs.text[MAIN_D_80135250], 0, (MAIN_D_80135250 * 13) + 0xdd, 0, 0x53);
		MAIN_D_80135250++;
		break;
	}

	if (UI_BOX_DATA[0].state == 0) {
		if (MAIN_D_80135250 == 2) {
			createAnimatedUIBox(0, 0, 2, &finalPos, &startPos, NULL, NULL);
			MAIN_D_80135250 = 3;
		} else if (MAIN_D_80135250 == 3 || MAIN_D_80135250 == 4) {
			return (int8_t)(MAIN_D_80135250 - 2);
		}
	}

	if (UI_BOX_DATA[0].state == 1) {
		if ((POLLED_INPUT & CONFIRM_BUTTON) != 0 && (POLLED_INPUT_PREVIOUS & CONFIRM_BUTTON) == 0) {
			removeAnimatedUIBox(0, NULL);
		} else {
			if ((POLLED_INPUT & 0x1000) != 0 && (POLLED_INPUT_PREVIOUS & 0x1000) == 0) {
				MAIN_D_80135250 = (MAIN_D_80135250 == 3) ? 4 : 3;
				playSound(0, 2);
			}
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			else
#endif
			if ((POLLED_INPUT & 0x4000) != 0 && (POLLED_INPUT_PREVIOUS & 0x4000) == 0) {
				MAIN_D_80135250 = (MAIN_D_80135250 == 3) ? 4 : 3;
				playSound(0, 2);
			}
#if !VERSION_IS(US)
			renderString(0, UI_BOX_DATA[0].finalPos.x + 6, UI_BOX_DATA[0].finalPos.y + 6, 0xfc, 0x1a, 0, 0xdc, 6, 1);
			renderSelectionCursor(finalPos.x + 6, finalPos.y + 7 + ((MAIN_D_80135250 - 3) * 13), 0x80, 0xd, 6);
#else
			renderString(0, -0x7c, 0x30, 0x52, 0xd, 0, 0xdc, 6, 1);
			renderString(0, -0x7c, 0x3d, 0x5c, 0xd, 0x53, 0xdc, 6, 1);
			renderSelectionCursor(finalPos.x + 6, finalPos.y + 7 + ((MAIN_D_80135250 - 3) * 13), 0x5e, 0xd, 6);
#endif
		}
	}

	return 0;
}

int32_t KAR_tickHintBox(int32_t n)
{
	RECT finalPos;
	RECT startPos;

	finalPos = MAIN_D_80134A28;
	startPos = MAIN_D_80134A30;
	if (MAIN_D_80135250 == 0) {
		return 0;
	}

	if (n < 4) {
		startPos.x = 0x4b;
		startPos.y = -5;
	}

	if (UI_BOX_DATA[0].state == 0) {
		if (MAIN_D_80135250 == 1) {
			createAnimatedUIBox(0, 0, 2, &finalPos, &startPos, NULL, NULL);
			clearTextArea();
			MAIN_D_8013522C = 0;
			return 1;
		}
		return 0;
	}

	if (MAIN_D_80135248 == 2) {
		n = (int8_t)KAR_drawHintPagePenguinmon(n, MAIN_D_8013522C);
	} else {
		n = (int8_t)KAR_drawHintPageMetalMamemon(n, MAIN_D_8013522C);
	}

	if (MAIN_D_8013522C < n * 4) {
		MAIN_D_8013522C++;
	}

	if (UI_BOX_DATA[0].state == 1) {
		if ((POLLED_INPUT & CONFIRM_BUTTON) != 0 && (POLLED_INPUT_PREVIOUS & CONFIRM_BUTTON) == 0) {
			MAIN_D_80135250++;
			playSound(0, 3);
			if (n < MAIN_D_80135250) {
				removeAnimatedUIBox(0, NULL);
				MAIN_D_80135250 = -1;
			}
		}
		if (MAIN_D_80135250 != -1) {
			renderString(0, UI_BOX_DATA[0].finalPos.x + 4, UI_BOX_DATA[0].finalPos.y + 2, 0xfc, 0x34, 0,
			             (MAIN_D_80135250 - 1) * 52, 6, 1);
			renderUIBox(0);
		}
	}

	return 1;
}

void KAR_classifyStoneRings(void)
{
	KarStone *stone;
	int32_t p;
	int32_t dist;
	int32_t i;
	int32_t zi;

	for (p = 0; p < 2; p++) {
		stone = KAR_D_8005B5A0[p].row.stones;
		KAR_D_8005B5A0[p].row.score = 0;
		for (i = 0; i < 5; i++, stone++) {
			stone->prevRing = stone->ring;
			stone->ring = 0;
			if (stone->state <= 0) {
				continue;
			}
			if (stone->pos.vz >= -0x564) {
				continue;
			}
			dist = KAR_distance(stone->pos.vx, stone->pos.vz + 0x6a4);
			for (zi = 0; zi < 4; zi++) {
				KarZones z;

				z = KAR_D_8005B438;
				dist = KAR_distance(stone->pos.vx + z.ring[zi].dx, stone->pos.vz + z.ring[zi].dz);
				if (dist > z.ring[zi].radius) {
					continue;
				}
				stone->ring = zi + 1;
				if (p != 0) {
					stone->ring |= 0x10;
				}
				break;
			}
		}
	}

	KAR_registerThrownStone();
}

int8_t KAR_tickScoreTally(void)
{
	KarTallyValues values;
	int32_t done;
	int32_t j;
	int32_t i;
	KarStone *rowStone;

	values = MAIN_D_80134A44;
	if ((POLLED_INPUT & CONFIRM_BUTTON) != 0 && (POLLED_INPUT_PREVIOUS & CONFIRM_BUTTON) == 0) {
		MAIN_D_8013522D = 1;
	}

	if (MAIN_D_8013522D == 1 && (MAIN_D_8013522E == 2 || MAIN_D_8013522E == 1)) {
		int8_t player;
		int8_t ring;

		MAIN_D_80135256 = KAR_D_8005B5A0[0].row.thrown + KAR_D_8005B5A0[1].row.thrown;
		MAIN_D_80135256--;
		KAR_D_8005B5A0[1].row.score = KAR_D_8005B5A0[0].row.score = 0;
		while (MAIN_D_80135256 >= 0) {
			player = (KAR_D_800639C0[MAIN_D_80135256]->ring & 0x10) ? 1 : 0;
			ring = KAR_D_800639C0[MAIN_D_80135256]->ring & 0xf;
			if (KAR_D_800639C0[MAIN_D_80135256]->state > 0 && ring != 0) {
				KAR_D_8005B5A0[player].row.score += values.value[ring - 1];
			}
			MAIN_D_80135256--;
		}

		for (i = 0; i < 3; i++) {
			rowStone = KAR_D_8005B5A0[i].row.stones;
			for (j = 0; j < 5; j++, rowStone++) {
				if (rowStone->state >= 100) {
					rowStone->state -= 100;
				}
			}
		}
		MAIN_D_80135230 = 0;
		MAIN_D_80135234 = 0;
		MAIN_D_8013522D = MAIN_D_8013522E = 0;
		return 1;
	}

	if (MAIN_D_80134A4F != MAIN_D_80135256) {
		int8_t ring;

		while (MAIN_D_80135256 >= 0) {
			ring = KAR_D_800639C0[MAIN_D_80135256]->ring & 0xf;
			if (ring != 0) {
				MAIN_D_80134A4F = MAIN_D_80135256;
				MAIN_D_8013522E = 0;
				break;
			}
			MAIN_D_80135256--;
		}
		if (MAIN_D_80135256 < 0) {
			MAIN_D_8013522D = 0;
			return 1;
		}
	}

	if (MAIN_D_8013522E == 0) {
		if (KAR_D_800639C0[MAIN_D_80135256]->ring != KAR_D_800639C0[MAIN_D_80135256]->prevRing) {
			done = tickCameraMoveTo(KAR_D_800639C0[MAIN_D_80135256]->pos.vx,
			                        KAR_D_800639C0[MAIN_D_80135256]->pos.vz, 5);
			if (done == 1) {
				MAIN_D_8013522E = 1;
			} else {
				return 0;
			}
		} else {
			MAIN_D_8013522E = 1;
		}
	}

	if (MAIN_D_8013522E == 1) {
		int8_t player;
		int8_t ring;

		player = (KAR_D_800639C0[MAIN_D_80135256]->ring & 0x10) ? 1 : 0;
		ring = KAR_D_800639C0[MAIN_D_80135256]->ring & 0xf;
		KAR_D_8005B5A0[player].row.score += values.value[ring - 1];
		MAIN_D_8013522E = 2;
		if (KAR_D_800639C0[MAIN_D_80135256]->ring != KAR_D_800639C0[MAIN_D_80135256]->prevRing &&
		    (KAR_D_800639C0[MAIN_D_80135256]->ring & 0xf) != 0) {
			if (ring == 3) {
				playSound2(8, 6);
			} else {
				playSound2(8, 5);
			}
		}
	}

	if (MAIN_D_8013522E == 2 && MAIN_D_80135256 >= 0) {
		if (KAR_D_800639C0[MAIN_D_80135256]->ring != KAR_D_800639C0[MAIN_D_80135256]->prevRing &&
		    (KAR_D_800639C0[MAIN_D_80135256]->ring & 0xf) != 0) {
			if (MAIN_D_80135234++ >= 0xb) {
				if (KAR_D_800639C0[MAIN_D_80135256]->state < 100) {
					KAR_D_800639C0[MAIN_D_80135256]->state += 100;
				} else {
					KAR_D_800639C0[MAIN_D_80135256]->state -= 100;
				}
				MAIN_D_80135234 -= 10;
				if (MAIN_D_80135230++ >= 7) {
					MAIN_D_80135230 = 0;
					MAIN_D_8013522E = 4;
					MAIN_D_80135256--;
				}
			}
		} else {
			MAIN_D_80135256--;
		}

		if (MAIN_D_80135256 < 0) {
			return 1;
		}
		return 0;
	}

	return 0;
}

int32_t KAR_chooseOpponentShot(void)
{
	KarStone *stone;
	int32_t clear;
	int32_t i;
	int16_t tx;
	int16_t tz;
	int32_t angle;
	int16_t px;
	int16_t pz;

	if (MAIN_D_80135248 == 2) {
		angle = KAR_findClearShotAngle(0, -0x6a4);
		if (angle == -1) {
			if (KAR_D_8005B5A0[0].row.score > KAR_D_8005B5A0[1].row.score) {
				stone = KAR_D_8005B5A0[1].row.stones;
				angle = KAR_aimAtStoneInRing(1, 0x14, &tx, &tz);
				if (angle != -1) {
					KAR_setOpponentShot(0, angle, tx, tz);
				} else {
					angle = KAR_aimAtStoneInRing(0, 2, &tx, &tz);
					if (angle != -1) {
						KAR_setOpponentShot(0, angle, tx, tz);
					} else {
						angle = KAR_findClearShotAngle(-0x190, -0x960);
						if (angle != -1) {
							KAR_setOpponentShot(1, angle, 0, -0x960);
						} else {
							angle = KAR_aimAtRandomStone();
							KAR_setOpponentShot(0, angle, 0, -0x960);
							KAR_D_8005B5A0[1].row.unk2 = 0x960;
						}
					}
				}
			} else {
				angle = KAR_aimAtStoneInRing(0, 2, &tx, &tz);
				if (angle != -1) {
					KAR_setOpponentShot(0, angle, tx, tz);
				} else {
					angle = KAR_findClearShotAngle(-0x190, -0x960);
					if (angle != -1) {
						KAR_setOpponentShot(1, angle, 0, -0x960);
					} else {
						angle = KAR_aimAtRandomStone();
						KAR_setOpponentShot(0, angle, 0, -0x960);
						KAR_D_8005B5A0[1].row.unk2 = 0x960;
					}
				}
			}
		} else {
			KAR_setOpponentShot(1, angle, 0, -0x6a4);
		}
	} else {
		px = ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[0];
		pz = ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[2];

		if (KAR_D_8005B5A0[1].row.thrown == 0) {
			angle = ratan2(-0x8fc - pz, -0x190 - px);
			clear = KAR_findClearShotAngle(-0x190, -0x8fc);
			if (angle == clear) {
				KAR_setOpponentShot(1, angle, -0x190, -0x8fc);
			} else {
				angle = ratan2(-0x6a4 - pz, -px);
				clear = KAR_findClearShotAngle(0, -0x6a4);
				if (angle == clear) {
					KAR_setOpponentShot(1, angle, 0, -0x6a4);
				} else {
					angle = ratan2(-0x6a4 - pz, -px);
					if ((KAR_D_8005B5A0[1].row.thrown % 2) != 0) {
						angle += rand() % BKS(AI_NOISE);
					} else {
						angle -= rand() % BKS(AI_NOISE);
					}
					KAR_setOpponentShot(0, angle, 0, -0x6a4);
				}
			}
		} else {
			for (i = 0; i < 5; i++) {
				if (KAR_D_8005B5A0[1].row.stones[(uint32_t)i].state == -1) {
					if (KAR_D_8005B5A0[1].row.stones[(uint32_t)i].type == 3) {
						KAR_D_8005B5A0[1].row.unk4 = i;
						angle = ratan2(-0x6a4 - pz, -px);
						clear = KAR_findClearShotAngle(0, -0x6a4);
						if (angle == clear) {
							KAR_setOpponentShot(1, angle, 0, -0x6a4);
						} else {
							KAR_D_8005B5A0[1].row.unk4 = 6;
						}
						break;
					}
					KAR_D_8005B5A0[1].row.unk4 = 6;
				}
			}

			if (KAR_D_8005B5A0[1].row.unk4 == 6) {
				switch (KAR_D_80063914.stone.type) {
				case 0:
				case 1:
				case 2:
					if ((KAR_D_80063914.stone.ring == 3) || (KAR_D_80063914.stone.ring == 0)) {
						angle = ratan2(-0x8fc - pz, -0x190 - px);
						clear = KAR_findClearShotAngle(-0x190, -0x8fc);
						KAR_setOpponentShot(1, angle, -0x190, -0x8fc);
					} else if (KAR_D_80063914.stone.state < 0) {
						angle = ratan2(-0x6a4 - pz, -px);
						if ((KAR_D_8005B5A0[1].row.thrown % 2) != 0) {
							angle += rand() % BKS(AI_NOISE);
						} else {
							angle -= rand() % BKS(AI_NOISE);
						}
						KAR_setOpponentShot(0, angle, 0, -0x6a4);
					} else if ((rand() % 2) != 0) {
						angle = ratan2(-0x8fc - pz, -0x190 - px);
						clear = KAR_findClearShotAngle(-0x190, -0x8fc);
						if (angle == clear) {
							KAR_setOpponentShot(1, angle, -0x190, -0x8fc);
						} else {
							angle = ratan2(KAR_D_80063914.stone.pos.vz - pz, KAR_D_80063914.stone.pos.vx - px);
							KAR_setOpponentShot(0, angle, (int16_t)KAR_D_80063914.stone.pos.vx, (int16_t)KAR_D_80063914.stone.pos.vz);
						}
					} else {
						angle = KAR_aimBankShot(&KAR_D_80063914.stone, 0x190, -0x8fc);
						KAR_setOpponentShot(0, angle, 0x190, -0x8fc);
						KAR_D_8005B5A0[1].row.unk2 = 0x960;
					}
					break;
				case 3:
					angle = ratan2(-0x6a4 - pz, -px);
					if ((KAR_D_8005B5A0[1].row.thrown % 2) != 0) {
						angle += rand() % BKS(AI_NOISE);
					} else {
						angle -= rand() % BKS(AI_NOISE);
					}
					KAR_setOpponentShot(0, angle, 0, -0x6a4);
					break;
				}
			}
		}
	}
}

int32_t KAR_drawHintPagePenguinmon(int32_t idx, int8_t n)
{
	KarStrTbl strs;
#if !VERSION_IS(US)
	KarStrPair highlights;
#endif
	KarOffTbl offs;

	strs = KAR_D_8005AF58;
#if !VERSION_IS(US)
	highlights = KAR_PENGUINMON_HIGHLIGHTS;
#endif
	offs = KAR_D_8005B04C;
	if ((n % 4) == 0) {
		setTextColor(7);
	} else {
		setTextColor(1);
	}

	drawString(strs.text[offs.start[idx] + n], 0, (n * 13) + 1);
#if !VERSION_IS(US)
	if ((idx == 1) && (n == 7)) {
		setTextColor(10);
		drawString(highlights.text[0], 0, (n * 13) + 1);
	}
	if ((idx == 2) && (n == 3)) {
		setTextColor(10);
		drawString(highlights.text[1], 0, (n * 13) + 1);
	}
#endif

	return (int8_t)((offs.start[idx + 1] - offs.start[idx]) / 4);
}

int32_t KAR_drawHintPageMetalMamemon(int32_t idx, int8_t n)
{
	KarStrTbl strs;
#if !VERSION_IS(US)
	KarStrPair highlights;
#endif
	KarOffTbl offs;

	strs = KAR_D_8005B318;
#if !VERSION_IS(US)
	highlights = KAR_METALMAMEMON_HIGHLIGHTS;
#endif
	offs = KAR_D_8005B40C;
	if ((n % 4) == 0) {
		setTextColor(7);
	} else {
		setTextColor(1);
	}

	drawString(strs.text[offs.start[idx] + n], 0, (n * 13) + 1);
#if !VERSION_IS(US)
	if ((idx == 1) && (n == 7)) {
		setTextColor(10);
		drawString(highlights.text[0], 0, (n * 13) + 1);
	}
	if ((idx == 2) && (n == 3)) {
		setTextColor(10);
		drawString(highlights.text[1], 0, (n * 13) + 1);
	}
#endif

	return (int8_t)((offs.start[idx + 1] - offs.start[idx]) / 4);
}

int32_t KAR_findClearShotAngle(int16_t x, int16_t z)
{
	int32_t n;
	int32_t p;
	int32_t ang;
	int32_t dist;
	int32_t ret;
	KarStone *stone;
	int16_t px;
	int16_t pz;
	int16_t step;

	px = ENTITY_TABLE[MAIN_D_80135248]->posData->location.vx;
	pz = ENTITY_TABLE[MAIN_D_80135248]->posData->location.vz;
	if (MAIN_D_80135248 != 2) {
		px = ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[0];
		pz = ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[2];
	}
	ang = ratan2(z - pz, x - px);
	if (MAIN_D_80135248 == 2) {
		step = 30;
	} else {
		step = 10;
	}
	while (z < pz) {
		px += (int16_t)((step * rcos(ang)) / 4096);
		pz += (int16_t)((step * rsin(ang)) / 4096);
		for (p = 0; p < 3; p++) {
			stone = KAR_D_8005B5A0[p].row.stones;
			for (n = 0; n < 5; n++, stone++) {
				if (stone->state > 0) {
					dist = KAR_distance(px - stone->pos.vx, pz - stone->pos.vz);
					if (dist < 0x97) {
						if (MAIN_D_80135248 == 2) {
							return -1;
						}
						ret = ratan2(stone->pos.vz - z, stone->pos.vx - x);
						return ret;
						/* scheduling barrier: mwcc needs the extra
						 * block here to colour the loop registers */
						do {
						} while (0);
					}
				}
			}
		}
	}
	return ang;
}

// clang-format off
int32_t KAR_aimAtStoneInRing(player, key, outX, outZ)
	int8_t player;
	int8_t key;
	int16_t *outX;
	int16_t *outZ;
// clang-format on
{
	KarStone *stone;
	int32_t i;
	int32_t r;

	stone = KAR_D_8005B5A0[player].row.stones;
	for (i = 0; i < 5; i++, stone++) {
		if ((stone->state < 2) && (stone->ring == key)) {
			r = KAR_findClearShotAngle((int16_t)stone->pos.vx, (int16_t)stone->pos.vz);
			*outX = stone->pos.vx;
			*outZ = stone->pos.vz;
			return r;
		}
	}

	return -1;
}

void KAR_setOpponentShot(int8_t row, int32_t val, int16_t b, int16_t c)
{
	int32_t i;
	KarStone *stone;
	int32_t n;
	int32_t power;

	for (i = 0; i < 4; i++) {
		KarShotPlan t;

		t = KAR_D_8005B418;
		n = KAR_findUnusedStoneOfType((int8_t)t.type[row][i]);
		if (n >= 0) {
			KAR_D_8005B5A0[1].row.unk4 = n;
			stone = &KAR_D_8005B5A0[1].row.stones[n];
			break;
		}
	}

	stone->angle = val;
	power = KAR_computeThrowPower(stone->weight, b, c);
	KAR_D_8005B5A0[1].row.unk2 = power;
}

int32_t KAR_aimAtRandomStone(void)
{
	uint8_t count;
	int32_t i;
	int32_t ret;
	KarStone *cands[5];
	KarStone *stone;
	int16_t px;
	int16_t pz;

	stone = KAR_D_8005B5A0[0].row.stones;
	if (KAR_D_80063914.stone.state < 0) {
		ret = ratan2(-0xc1c, -0x190);
		return ret;
	}

	for (i = 0, count = 0; i < 5; i++, stone++) {
		if (stone->state == 1) {
			cands[count] = stone;
			count++;
		}
	}

	if (count == 0) {
		px = 0;
		pz = -0x6a4;
	} else {
		uint8_t idx;

		idx = rand() % count;
		px = cands[idx]->pos.vx;
		pz = cands[idx]->pos.vz;
	}

	ret = ratan2(pz - 0x578, px - 0x190);
	return ret;
}

// clang-format off
int32_t KAR_aimBankShot(stone, x, z)
	KarStone *stone;
	int16_t x;
	int16_t z;
// clang-format on
{
	int32_t ang;
	int32_t cx;
	int32_t ret;
	int32_t cz;
	int32_t t;

	ang = ratan2(stone->pos.vz - z, stone->pos.vx - x);
	cx = stone->pos.vx + ((rcos(ang) * 150) / 4096);
	cz = stone->pos.vz + ((rsin(ang) * 150) / 4096);
	t = cx + 0x2d5;
	cx = -0x2d5 - t;
	ret = ratan2(cz - ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[2],
		     cx - ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[0]);
	return ret;
}

// clang-format off
int32_t KAR_findUnusedStoneOfType(key)
	int8_t key;
// clang-format on
{
	KarStone *stone;
	int32_t i;

	stone = KAR_D_8005B5A0[1].row.stones;
	for (i = 0; i < 5; i++, stone++) {
		if ((stone->state == -1) && (stone->type == key)) {
			return i;
		}
	}

	return -1;
}

// clang-format off
int32_t KAR_computeThrowPower(a, b, c)
	uint32_t a;
	int16_t b;
	int16_t c;
// clang-format on
{
	int32_t dist;
	uint32_t acc;
	uint32_t value;
	uint32_t cur;

	dist = KAR_distance(0x190 - b, 0x578 - c);
	acc = 0;
	cur = a;
	value = cur * 100;
	while (acc < dist) {
		acc += cur / a;
		value = value * 100 / BKS(FRICTION);
		cur = value / 100;
	}
	if (MAIN_D_80135248 == 2) {
		cur -= BK(POWER_SUB_A);
	} else {
		cur -= BK(POWER_SUB_B);
	}
	return cur;
}

void KAR_registerThrownStone(void)
{
	MAIN_D_80135256 = KAR_D_8005B5A0[0].row.thrown + KAR_D_8005B5A0[1].row.thrown;
	KAR_D_800639C0[MAIN_D_80135256] =
		&KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A];
	MAIN_D_80134A4F = -1;
}

void KAR_rotatePoint(SVECTOR *p, int32_t ang)
{
	SVECTOR t;

	t = *p;
	p->vx = ((t.vx * rcos(ang)) / 4096) - ((t.vz * rsin(ang)) / 4096);
	p->vz = ((t.vx * rsin(ang)) / 4096) + ((t.vz * rcos(ang)) / 4096);
}

void KAR_renderSprite(KarSprite *sp)
{
	POLY_FT4 *p;

	p = (POLY_FT4 *)GsGetWorkBase();
	SetPolyFT4(p);
	p->tpage = getTPage(0, 0, 640, 0);
	setRGB0(p, sp->r, sp->g, sp->b);
	setClut(p, sp->clutX, sp->clutY);
	setUVWH(p, sp->u, sp->v, sp->w, sp->h);
	setXYWH(p, sp->x, sp->y, sp->w, sp->h);
	AddPrim(ACTIVE_ORDERING_TABLE->org + 0xa, p++);
	GsSetWorkBase((PACKET *)p);
}

/* Definition order preserves compiler scheduling; kar_functions preserves link order. */
void KAR_tickMatchState(void)
{
	int32_t j;
	int32_t i;
	int8_t prompt;
	int8_t player;

	player = (MAIN_D_8013523C != 0) ? MAIN_D_80135248 : 0;
	switch (MAIN_D_80135244) {
	case 0:
		MAIN_D_80135252 = 0;
		ENTITY_TABLE[0]->posData->location.vx = -0x190;
		ENTITY_TABLE[0]->posData->location.vz = 0x578;
		ENTITY_TABLE[0]->posData->rotation.vy = 0;
		ENTITY_TABLE[MAIN_D_80135248]->posData->location.vx = 0x190;
		ENTITY_TABLE[MAIN_D_80135248]->posData->location.vz = 0x578;
		ENTITY_TABLE[MAIN_D_80135248]->posData->rotation.vy = 0;
		startAnimation(ENTITY_TABLE[0], 0);
		startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0);
		MAIN_D_80135244 = 1;
		clearTextArea();
		MAIN_D_80135250 = 0;
		break;
	case 1:
		prompt = KAR_tickYesNoPrompt();
		if (prompt == 1) {
			MAIN_D_80135244 = 3;
			MAIN_D_80135250 = 0;
		} else if (prompt == 2) {
			clearTextArea();
			MAIN_D_80135244 = 2;
			MAIN_D_80135250 = 0;
			KAR_setupMatch(1);
		}
		break;
	case 3:
		if (MAIN_D_80135220 >= 0x30) {
			if ((uint16_t)(MAIN_D_80135252 += 6) >= 0x169) {
				MAIN_D_80135252 = 0;
				MAIN_D_80135244 = 5;
			}
		}
		break;
	case 2:
		if ((uint16_t)(MAIN_D_80135252 += 6) >= 0x169) {
			MAIN_D_80135252 = 0;
			MAIN_D_80135244 = 4;
		}
		break;
	case 4:
		MAIN_D_80135250 = 1;
		MAIN_D_80135244 = 5;
		KAR_D_8005B5A0[0].row.unk4 = rand() % 5;
		KAR_D_8005B5A0[0].row.unk2 = 0x7D0;
		KAR_D_8005B5A0[0].row.stones[KAR_D_8005B5A0[0].row.unk4].angle = ratan2(-0xC1C, 0x190);
		break;
	case 5:
		if ((MAIN_D_8013523C != 0) || (MAIN_D_80135250 != 0)) {
			if (KAR_tickHintBox(0) != 0) {
				break;
			}
			if (MAIN_D_80135252++ >= 0x10) {
				MAIN_D_80135252 -= 0xF;
				if (MAIN_D_8013523A == KAR_D_8005B5A0[MAIN_D_8013523C].row.unk4) {
					KAR_D_8005B5A0[2].row.stones[4] =
						KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A];
					KAR_D_8005B5A0[2].row.stones[4].state = -0x65;
					KAR_beginAiming();
					MAIN_D_80135252 = 0;
					if (MAIN_D_80135250 != 0) {
						MAIN_D_80135250 = 1;
					}
				}
				if (KAR_D_8005B5A0[MAIN_D_8013523C].row.unk4 > MAIN_D_8013523A) {
					KAR_selectNextStone();
				}
			}
		} else {
			if ((POLLED_INPUT & CONFIRM_BUTTON) && !(POLLED_INPUT_PREVIOUS & CONFIRM_BUTTON)) {
				KAR_beginAiming();
			}
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			else
#endif
			if ((POLLED_INPUT & 0x1000) && !(POLLED_INPUT_PREVIOUS & 0x1000)) {
				KAR_selectPreviousStone();
			}
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			else
#endif
			if ((POLLED_INPUT & 0x4000) && !(POLLED_INPUT_PREVIOUS & 0x4000)) {
				KAR_selectNextStone();
			}
		}
		MAIN_D_80135254 = 0;
		break;
	case 6:
		if ((MAIN_D_8013523C != 0) || (MAIN_D_80135250 != 0)) {
			int32_t targetAngle;

			targetAngle = KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].angle;
#if VERSION_IS(JP)
			if (KAR_tickHintBox(1) != 0) {
				if ((POLLED_INPUT & 0x2000) && (MAIN_D_8013524C < -0x18F)) {
					if (MAIN_D_80135254 > 0) {
						MAIN_D_80135254++;
					} else {
						MAIN_D_80135254 = 1;
					}
					KAR_turnAimLeft(MAIN_D_80135254);
				}
				if ((POLLED_INPUT & 0x8000) && (MAIN_D_8013524C >= -0x5B6)) {
					if (MAIN_D_80135254 < 0) {
						MAIN_D_80135254--;
					} else {
						MAIN_D_80135254 = -1;
					}
					KAR_turnAimRight(MAIN_D_80135254);
				}
				if (!(POLLED_INPUT & 0x8000) && !(POLLED_INPUT & 0x2000)) {
					MAIN_D_80135254 = 0;
				}
			} else if ((targetAngle <= MAIN_D_8013524C + 0xA) &&
			           (targetAngle >= MAIN_D_8013524C - 0xA)) {
				KAR_beginThrow();
				if (MAIN_D_80135250 != 0) {
					MAIN_D_80135250 = 1;
				}
			} else if (targetAngle > MAIN_D_8013524C) {
				KAR_turnAimLeft(0xA);
			} else {
				KAR_turnAimRight(-0xA);
			}
#else
			if (KAR_tickHintBox(1) != 0) {
				if ((POLLED_INPUT & 0x2000) && (MAIN_D_8013524C < -0x18F)) {
					if (MAIN_D_80135254 > 0) {
						MAIN_D_80135254++;
					} else {
						MAIN_D_80135254 = 1;
					}
					KAR_turnAimLeft(MAIN_D_80135254);
				} else if ((POLLED_INPUT & 0x8000) && (MAIN_D_8013524C >= -0x5B6)) {
					if (MAIN_D_80135254 < 0) {
						MAIN_D_80135254--;
					} else {
						MAIN_D_80135254 = -1;
					}
					KAR_turnAimRight(MAIN_D_80135254);
				} else if (!(POLLED_INPUT & 0x8000) && !(POLLED_INPUT & 0x2000)) {
					MAIN_D_80135254 = 0;
					break;
				} else {
					break;
				}
			}
			if ((targetAngle <= MAIN_D_8013524C + 0xA) &&
			    (targetAngle >= MAIN_D_8013524C - 0xA)) {
				KAR_beginThrow();
				if (MAIN_D_80135250 != 0) {
					MAIN_D_80135250 = 1;
				}
			} else if (targetAngle > MAIN_D_8013524C) {
				KAR_turnAimLeft(0xA);
			} else {
				KAR_turnAimRight(-0xA);
			}
#endif
		} else {
			if ((POLLED_INPUT & CONFIRM_BUTTON) && !(POLLED_INPUT_PREVIOUS & CONFIRM_BUTTON)) {
				KAR_beginThrow();
			}
			if ((POLLED_INPUT & 0x2000) && (MAIN_D_8013524C < -0x18F)) {
				if (MAIN_D_80135254 > 0) {
					MAIN_D_80135254++;
				} else {
					MAIN_D_80135254 = 1;
				}
				KAR_turnAimLeft(MAIN_D_80135254);
			}
			if ((POLLED_INPUT & 0x8000) && (MAIN_D_8013524C >= -0x5B6)) {
				if (MAIN_D_80135254 < 0) {
					MAIN_D_80135254--;
				} else {
					MAIN_D_80135254 = -1;
				}
				KAR_turnAimRight(MAIN_D_80135254);
			}
			if (!(POLLED_INPUT & 0x8000) && !(POLLED_INPUT & 0x2000)) {
				MAIN_D_80135254 = 0;
			}
		}
		break;
	case 7:
		MAIN_D_80135224 = tickCameraMoveTo(
			KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].pos.vx,
			KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].pos.vz, 0x14);
		if (MAIN_D_80135224 == 1) {
			MAIN_D_80135244 = 8;
		}
		MAIN_D_80135254 = 0;
		break;
	case 8:
		if ((MAIN_D_8013523E += 0x64) >= 0xBB8) {
			MAIN_D_8013523E = 0;
			MAIN_D_80135244 = 6;
			ENTITY_TABLE[0]->posData->location.vx = -0x190;
			ENTITY_TABLE[0]->posData->location.vz = 0x578;
			ENTITY_TABLE[MAIN_D_80135248]->posData->location.vx = 0x190;
			ENTITY_TABLE[MAIN_D_80135248]->posData->location.vz = 0x578;
			if (MAIN_D_8013523C == 0) {
				startAnimation(ENTITY_TABLE[0], 0);
			} else {
				startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0);
			}
		} else if ((MAIN_D_8013523C != 0) || (MAIN_D_80135250 != 0)) {
			if (KAR_tickHintBox(2) != 0) {
				if (KAR_D_8005B5A0[MAIN_D_8013523C].row.unk2 <= MAIN_D_8013523E) {
					MAIN_D_8013523E = KAR_D_8005B5A0[MAIN_D_8013523C].row.unk2;
				}
			} else if (KAR_D_8005B5A0[1].row.unk2 <= MAIN_D_8013523E) {
				MAIN_D_80135244 = 9;
				KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].rotation.vy =
					ENTITY_TABLE[MAIN_D_80135248]->posData->rotation.vy;
				if (MAIN_D_80135250 != 0) {
					MAIN_D_80135250 = 1;
				}
			}
		} else if ((POLLED_INPUT & CONFIRM_BUTTON) && !(POLLED_INPUT_PREVIOUS & CONFIRM_BUTTON)) {
			MAIN_D_80135244 = 9;
			KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].angle =
				MAIN_D_8013524C;
			KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].rotation.vy =
				TAMER_ENTITY.entity.posData->rotation.vy;
		}
		break;
	case 9:
		if (MAIN_D_8013523C == 0) {
			startAnimation(ENTITY_TABLE[0], 0x25);
		} else if (MAIN_D_80135248 == 2) {
			startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0x1E);
		} else {
			startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0x20);
		}
		KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].speed =
			MAIN_D_8013523E + 0x258;
		MAIN_D_80135244 = 0xA;
		MAIN_D_80135252 = 0;
		MAIN_D_80135228 = playSound2(8, 0);
		KAR_D_800638F4 = KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].pos;
		break;
	case 0xA:
		if (MAIN_D_80135252++ >= 3) {
			MAIN_D_80135252 = 0;
			KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].state = 1;
			KAR_D_800638F4 =
				KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].pos;
			MAIN_D_80135244 = 0xB;
		}
		break;
	case 0xB:
		KAR_D_80063904 = KAR_D_800638F4;
		KAR_D_800638F4 =
			KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[MAIN_D_8013523A].pos;
		moveCameraByDiff(&KAR_D_80063904, &KAR_D_800638F4);
		break;
	case 0xC: {
		KarStone *stone;

		thunkStopSoundMask(MAIN_D_80135228);
		KAR_classifyStoneRings();
		KAR_D_8005B5A0[MAIN_D_8013523C].row.thrown++;
		MAIN_D_80135244 = 0xD;
		for (i = 0; i < 2; i++) {
			stone = KAR_D_8005B5A0[i].row.stones;
			KAR_D_8005B5A0[i].row.thrown = 0;
			for (j = 0; j < 5; j++, stone++) {
				if (stone->state != -1) {
					KAR_D_8005B5A0[i].row.thrown++;
				}
			}
		}
		break;
	}
	case 0xD:
		prompt = KAR_tickScoreTally();
		if (prompt) {
		}
		if (prompt != 0) {
			if (MAIN_D_80135250 != 0) {
				MAIN_D_80135244 = 0xE;
			} else {
				MAIN_D_80135244 = 0xF;
			}
		}
		break;
	case 0xE:
		MAIN_D_80135224 = tickCameraMoveTo(0, -0x6A4, 0xA);
		if (MAIN_D_80135224 == 1) {
			MAIN_D_80135244 = 0xF;
		}
		break;
	case 0xF:
		if (KAR_tickHintBox(3) == 0) {
			TAMER_ENTITY.entity.posData->location.vx = -0x190;
			TAMER_ENTITY.entity.posData->location.vz = 0x578;
			ENTITY_TABLE[MAIN_D_80135248]->posData->location.vx = 0x190;
			ENTITY_TABLE[MAIN_D_80135248]->posData->location.vz = 0x578;
			startAnimation(ENTITY_TABLE[0], 0);
			startAnimation(ENTITY_TABLE[MAIN_D_80135248], 0);
			if (MAIN_D_8013523C == 0) {
				KAR_D_80063914.stone =
					KAR_D_8005B5A0[0].row.stones[MAIN_D_8013523A];
			}
			if (MAIN_D_80135250 != 0) {
				MAIN_D_80135224 = tickCameraMoveTo(ENTITY_TABLE[player]->posData->location.vx,
				                                   ENTITY_TABLE[player]->posData->location.vz, 0x14);
				if (MAIN_D_80135224 == 1) {
					ENTITY_TABLE[player]->posData->rotation.vy = 0;
					MAIN_D_80135250 = 0;
					MAIN_D_80135244 = 3;
					KAR_setupMatch(0);
					MAIN_D_80135252 = 0;
				}
			} else {
				if (KAR_D_8005B5A0[0].row.thrown == KAR_D_8005B5A0[1].row.thrown) {
					if (KAR_D_8005B5A0[0].row.thrown == 5) {
						MAIN_D_8013523C = 1;
					} else if (KAR_D_8005B5A0[0].row.score < KAR_D_8005B5A0[1].row.score) {
						MAIN_D_8013523C = 0;
					} else {
						MAIN_D_8013523C = 1;
					}
				} else if (KAR_D_8005B5A0[0].row.thrown < KAR_D_8005B5A0[1].row.thrown) {
					MAIN_D_8013523C = 0;
				} else {
					MAIN_D_8013523C = 1;
				}
				for (j = 0; j < 5; j++) {
					if (KAR_D_8005B5A0[MAIN_D_8013523C].row.stones[j].state == -1) {
						MAIN_D_8013523A = j;
						break;
					}
				}
				MAIN_D_80135252 = 0;
				MAIN_D_8013523E = 0;
				MAIN_D_80135244 = 0x10;
			}
		}
		break;
	case 0x10:
		MAIN_D_80135224 = tickCameraMoveTo(ENTITY_TABLE[player]->posData->location.vx,
		                                   ENTITY_TABLE[player]->posData->location.vz, 0x14);
		if (MAIN_D_80135224 == 1) {
			if ((KAR_D_8005B5A0[0].row.thrown >= 5) && (KAR_D_8005B5A0[1].row.thrown >= 5)) {
				MAIN_D_80135250 = 1;
				clearTextArea();
				MAIN_D_80135244 = 0x13;
				if (KAR_D_8005B5A0[0].row.score > KAR_D_8005B5A0[1].row.score) {
					playSound2(8, 7);
					MAIN_D_80135252 = 8;
				} else {
					playSound2(8, 8);
					MAIN_D_80135252 = 7;
				}
			} else if (MAIN_D_8013523C != 0) {
				MAIN_D_80135244 = 0x11;
			} else {
				MAIN_D_80135244 = 5;
			}
		}
		break;
	case 0x11:
		KAR_chooseOpponentShot();
		MAIN_D_80135244 = 0x13;
		MAIN_D_80135250 = 1;
		MAIN_D_80135252 = 0;
		MAIN_D_8013523E = 0;
		if (KAR_D_8005B5A0[0].row.score > KAR_D_8005B5A0[1].row.score) {
			MAIN_D_80135252 = 5;
		} else if (KAR_D_8005B5A0[0].row.score == KAR_D_8005B5A0[1].row.score) {
			MAIN_D_80135252 = 6;
		} else {
			MAIN_D_80135252 = 4;
		}
		break;
	case 0x12:
		if (UI_BOX_DATA[0].state == 1) {
			MAIN_D_80135244 = 0x13;
		}
		break;
	case 0x13:
		if (KAR_tickHintBox((int8_t)MAIN_D_80135252) == 0) {
			MAIN_D_80135250 = 0;
			if (MAIN_D_80135252 >= 7) {
				KAR_finishMatch();
			} else {
				MAIN_D_80135244 = 5;
				MAIN_D_80135252 = 0;
				MAIN_D_8013523E = 0;
			}
		}
		break;
	case 0x14:
		MAIN_D_80135252 = 0;
		MAIN_D_8013523E = 0;
		KAR_finishMatch();
		break;
	}
}
