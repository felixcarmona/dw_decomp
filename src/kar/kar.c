#include <stdlib.h>
#include <stdio.h>

#include <inline_n.h>
#include <libetc.h>
#include <libgpu.h>
#include <libgs.h>
#include <libgte.h>

#include <dw/mod_kar.h>
#include <dw/clock.h>
#include <dw/mod_font.h>
#include <dw/anim.h>
#include <dw/btl.h>
#include <dw/entity.h>
#include <dw/file.h>
#include <dw/font.h>
#include <dw/garbage.h>
#include <dw/input.h>
#include <dw/item.h>
#include <dw/line.h>
#include <dw/species.h>
#include <dw/kar.h>
#include <dw/map.h>
#include <dw/mod_fps.h>
#include <dw/mod_options.h>
#include <dw/memmap.h>
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
	KarDrawStone previous[25];
	KarDrawStone current[25];
	uint8_t blend[25];
	int32_t valid;
	int32_t hudState;
	VECTOR arrowOrigin;
	SVECTOR arrowRotation;
	VECTOR previousArrowOrigin;
	SVECTOR previousArrowRotation;
	int32_t arrowReady, arrowBlend;
	uint32_t tickSum, renderSum;
	int32_t warmup, samples, overBudget, steady30;
	int32_t tickPrims, tickPrimArg; /* the prompt or hint text this tick drew */
} KAR_MOD __attribute__((section(".bss.kar_mod")));

/* MOD (curling cups): matches beyond the two original ones. A duel against any
 * rival, doubles (the partner and a second rival throw every other stone) and
 * three or four teams on the ice at once. The script chooses with a variable;
 * 0 keeps the original match untouched. */
enum { KAR_FMT_CLASSIC, KAR_FMT_DUEL, KAR_FMT_DOUBLES, KAR_FMT_THREE, KAR_FMT_FOUR };
enum { KAR_ACT_TAMER, KAR_ACT_PENGUINMON, KAR_ACT_METALMAMEMON, KAR_ACT_GENERIC };
#define KAR_FORMAT_PSTAT 0x80
#define KAR_RIVAL_PSTAT 0x81 /* three variables: a row of KAR_RIVALS each, 0 none */
#define KAR_CUP_PSTAT 0x84   /* the cup (1-5: four rows of KAR_RIVALS each), then its round */

/* the map's placed Digimon, as overworld.c keeps them */
typedef struct {
	int16_t typeId;
	int16_t padding0;
	VECTOR waypoints[8];
	int16_t aiSections[8];
	int16_t activeSection;
	int16_t padding1;
	VECTOR targetLocation;
	int16_t posX, posY, posZ, rotX, rotY, rotZ;
	int16_t trackingRange, targetAngle, ccDiff, cwDiff;
	int8_t followMode, waypointWaitTimer, animation, hasWaypointTarget;
	int8_t lookAtTamerState, stopAnim;
	uint8_t pad[2];
} KarMapDigimon;
extern KarMapDigimon MAP_DIGIMON_TABLE[8];
#define KAR_RIVAL_ROWS 32
#define KAR_TEAM_PRIM_WORDS 1500
/* free 16-colour rows for the third and fourth teams' stones */
#define KAR_TEAM_CLUT_X 0x100
#define KAR_TEAM_CLUT_Y 0x1e6
/* the evolution chart's icons (always in video memory): where each one is, and their colour rows */
typedef struct {
	int16_t posX, posY;
	uint8_t u, v, clut, pad;
} KarChartIcon;
extern KarChartIcon EVO_CHART_BOXES[];
extern int16_t EVO_CHART_VIEW_COLORS[5];
#define KAR_BOARD_Y (-0x6c)
#define KAR_GROUP_W 44
#define KAR_PICK_PREV 0x1000
#define KAR_PICK_NEXT 0x4000
#define KAR_RACK_PROJ 0x100
#define KAR_CHART_TPAGE 0x18
#define KAR_CHART_ICONS 61
/* free colour rows for the third and fourth teams' boards, and one for a partner's own icon */
#define KAR_BOARD_CLUT_X 0xd0
#define KAR_RED_FILL_CLUT_X 0x1c0
#define KAR_OWN_ICON_X 640
#define KAR_OWN_ICON_Y 128
#define KAR_OWN_ICON_CLUT_X 0x240
/* the rivals of one match: the heap is 192 KB and a model loading takes 18 KB more for its texture */
#define KAR_ROOM_MAX 160

/* How a Digimon plays (curlingRivals in content/tables/curling.json). Row 0
 * is whoever has no row of its own; a species below zero ends the table. */
typedef struct {
	int16_t species;
	uint8_t style;
	uint8_t aimClip, throwClip, releaseClip;
	uint8_t hold;       /* how far from its feet the stone waits */
	uint8_t aimNoise;   /* spread of the aim */
	uint8_t powerNoise; /* spread of the power, in fours */
	uint8_t hitPercent; /* how often it goes for a rival's stone */
	uint8_t icon;       /* the chart id whose icon is its face on the plate, 0 none */
	uint8_t room;       /* the KB of heap its model and its body take on the rink */
} KarRival;

KarRival KAR_RIVALS[KAR_RIVAL_ROWS] = {
	{ 0, KAR_ACT_GENERIC, 0, 0, 0, 0x50, 60, 40, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

typedef struct {
	u_long *vertices;
	u_long vertexCount;
	u_long *normals;
	u_long normalCount;
	u_long *prims;
	u_long primCount;
	long scale;
} KarTmdObject;

static struct {
	uint8_t format, teams, stones, rows;
	uint8_t shown;       /* the rival row the right side of the screen shows */
	uint8_t playerActor; /* who throws the player's stone: the tamer or the partner */
	uint8_t turnOrder[4]; /* the rows in this round's throwing order */
	uint8_t roundShown;  /* the round whose title is in the text area; 0 when none is */
	uint8_t orderFlash;  /* ticks the order numbers still blink for: the order just changed */
	uint8_t lunge;
	uint8_t endTimer;
	uint8_t actor[5][2]; /* the entity throwing for each row, by turn */
	const KarRival *rival[8];
	KarStone *order[16];
	KarStoneRowRaw extra[2];
	KarTmdObject teamObject[2][4];
	u_long teamPrims[2][KAR_TEAM_PRIM_WORDS];
} KAR_CUP __attribute__((section(".bss.kar_mod")));

static inline KarStoneRow *KAR_row(int32_t p)
{
	return p < 3 ? &KAR_D_8005B5A0[p].row : &KAR_CUP.extra[p - 3].row;
}

static inline int32_t KAR_actor(void)
{
	return MAIN_D_8013523C != 0 ? MAIN_D_80135248 : KAR_CUP.playerActor;
}

/* Whether the game throws this stone itself: a rival's, or in doubles the
 * player's mate's. */
#define KAR_AUTO() (MAIN_D_8013523C != 0 || KAR_CUP.playerActor > 1)

/* the menu the rink opens on, and the draw a cup shows before each round */
#define KAR_FMT_LOBBY 5
#define KAR_SEED_PSTAT 0x86   /* what a cup's draw is made from */
#define KAR_RESULT_LEFT 5
enum { KAR_LOBBY_OFF, KAR_LOBBY_MODE, KAR_LOBBY_LIST, KAR_LOBBY_DRAW, KAR_LOBBY_LEAVING };

static struct {
	uint8_t page, cursor, mode, written, note, hint;
	uint8_t slot[14]; /* the draw of the cup on show */
} KAR_LOBBY __attribute__((section(".bss.kar_mod")));
static int32_t KAR_ICON_LAYER = 0xa;
#define KAR_PROGRESS_PSTAT 7 /* the cups won: a saved variable */

static void KAR_tickLobby(void);

/* A cup is a knockout of eight: the player and seven of the city's players, the
 * rows of KAR_RIVALS from the cup's first on. Its draw comes whole from the seed
 * the cup was entered with: d[0..7] the places top to bottom (0 the player),
 * d[8..11] who goes through the quarter-finals, d[12..13] the semi-finals. The
 * player's own ties are the player's to win; in the others the better player is
 * the likelier, by the square of its place among the seven. */
static const uint8_t KAR_CUP_FIRST[5] = { 1, 4, 7, 10, 14 };
static void KAR_cupDraw(uint8_t *d)
{
	int32_t cup = readPStat(KAR_CUP_PSTAT), base, i, k, a, b;
	u_long state = readPStat(KAR_SEED_PSTAT) * 2654435761u + 12345;

	base = KAR_CUP_FIRST[cup >= 1 && cup <= 5 ? cup - 1 : 0];
	d[0] = 0;
	for (i = 1; i < 8; i++) {
		d[i] = base + i - 1;
	}
	for (i = 7; i > 1; i--) {
		state = state * 1664525 + 1013904223;
		k = 1 + (state >> 16) % i;
		a = d[i];
		d[i] = d[k];
		d[k] = a;
	}
	d[8] = d[12] = 0;
	for (i = 1; i < 5; i++) {
		a = i < 4 ? d[i * 2] : d[10];
		b = i < 4 ? d[i * 2 + 1] : d[11];
		k = (a - base + 2) * (a - base + 2);
		state = state * 1664525 + 1013904223;
		d[i < 4 ? 8 + i : 13] = (int32_t)((state >> 16) % (k + (b - base + 2) * (b - base + 2))) < k ? a : b;
	}
}
static void KAR_renderLobby(void);
static void KAR_loadBanners(void);

static int32_t KAR_style(int32_t ent)
{
	if (ent == 0) {
		return KAR_ACT_TAMER;
	}
	if (KAR_CUP.format == KAR_FMT_CLASSIC) {
		return ent == 2 ? KAR_ACT_PENGUINMON : KAR_ACT_METALMAMEMON;
	}
	return KAR_CUP.rival[ent]->style;
}

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
int32_t KAR_tickHintBox(int32_t n);
void KAR_collidePeggedStone(KarStone *pegged, VECTOR a, KarStone *mover, VECTOR b);
void KAR_beginAiming(void);
void KAR_renderSprite(KarSprite *sp);
static int32_t KAR_boardX(int32_t row);
int32_t KAR_findWallContact(VECTOR *out, KarStone *stone, int32_t flag);
int32_t KAR_aimAtRandomStone(void);
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
	KAR_debugCollisionStone,
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

static void KAR_linkStone(KarStone *stone, int32_t row, int32_t object)
{
	if (row >= 3) {
		/* linking is what prepares an object's primitives for drawing */
		GsLinkObject4((u_long)KAR_CUP.teamObject[row - 3], &stone->obj, object / 3);
	} else {
		GsLinkObject4((u_long)(MAIN_D_80135240 + 3), &stone->obj, object);
	}
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

	for (p = 0; p < KAR_CUP.rows; p++) {
		stone = KAR_row(p)->stones;
		seed = rand();
		KAR_row(p)->score = 0;
		KAR_row(p)->thrown = 0;
		KAR_row(p)->unk2 = 0;
		KAR_row(p)->unk4 = 0;

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
			KAR_linkStone(stone, p, stone->type * 3 + models.id[p < 3 ? p : 1]);
			GsInitCoordinate2(NULL, &stone->coord);
			stone->obj.attribute = 0;
			stone->obj.coord2 = &stone->coord;
			if (p != 2 && i >= KAR_CUP.stones) {
				stone->pos.vy = 0;
				stone->pos.vx = 0;
				stone->pos.vz = 0;
				stone->rotation.vx = 0;
				stone->rotation.vy = 0;
				stone->rotation.vz = 0;
				stone->state = -0x65;
			} else if (p != 2) {
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
				if (KAR_CUP.format != KAR_FMT_CLASSIC) {
					/* small, in a row under the team's board */
					stone->pos.vx = (KAR_boardX(p) + 26 +
						(i * 2 - (KAR_CUP.stones - 1)) * (KAR_CUP.teams == 2 ? 20 : 17) / 2) * 0xcf8 / KAR_RACK_PROJ;
					stone->pos.vy = (KAR_BOARD_Y + 57) * 0xcf8 / KAR_RACK_PROJ;
				}
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

static const KarRival *KAR_findRival(int32_t species)
{
	const KarRival *rival;

	for (rival = KAR_RIVALS + 1; rival < KAR_RIVALS + KAR_RIVAL_ROWS && rival->species >= 0; rival++) {
		if (rival->species == species) {
			return rival;
		}
	}
	return KAR_RIVALS;
}

/* The third and fourth teams throw the opponent's stones in colours of their
 * own: its primitives again, pointing at palettes made from its two. */
static void KAR_makeTeamColours(void)
{
	KarTmdObject *object;
	u_long *out;
	uint8_t *prim;
	uint16_t clut[16];
	RECT rect;
	int32_t team, group, n, words, i, x, r, g, b, swap;

	for (team = 0; team < KAR_CUP.teams - 2; team++) {
		out = KAR_CUP.teamPrims[team];
		for (group = 0; group < 4; group++) {
			object = (KarTmdObject *)(MAIN_D_80135240 + 3) + group * 3 + 1;
			KAR_CUP.teamObject[team][group] = *object;
			KAR_CUP.teamObject[team][group].prims = out;
			prim = (uint8_t *)object->prims;
			for (n = 0; n < object->primCount; n++) {
				words = prim[1] + 1;
				for (i = 0; i < words; i++) {
					out[i] = ((u_long *)prim)[i];
				}
				if (prim[3] & 4) {
					x = (out[1] >> 16) & 0x3f;
					if (x == 3 || x == 5) {
						x = (KAR_TEAM_CLUT_X >> 4) + team * 2 + (x == 5);
						out[1] = (out[1] & ~(0xffffUL << 16)) |
							((u_long)((KAR_TEAM_CLUT_Y << 6) | x) << 16);
					}
				}
				out += words;
				prim += words * 4;
			}
		}
		for (n = 0; n < 2; n++) {
			rect.x = n ? 80 : 48;
			rect.y = 0x1e6;
			rect.w = 16;
			rect.h = 1;
			StoreImage(&rect, (u_long *)clut);
			DrawSync(0);
			for (i = 0; i < 16; i++) {
				r = clut[i] & 31;
				g = (clut[i] >> 5) & 31;
				b = (clut[i] >> 10) & 31;
				if (team == 0) {
					swap = g; g = r > b ? r : b; r = swap; b = swap; /* green */
				} else {
					swap = r > b ? r : b; r = swap; g = swap; b = swap / 6; /* yellow */
				}
				clut[i] = (clut[i] & 0x8000) | r | (g << 5) | (b << 10);
			}
			rect.x = KAR_TEAM_CLUT_X + team * 32 + n * 16;
			rect.y = KAR_TEAM_CLUT_Y;
			LoadImage(&rect, (u_long *)clut);
			DrawSync(0);
		}
	}
}

/* The red score board's colours turned green and yellow, for the third and
 * fourth teams' boards. */
static void KAR_makeBoardColours(void)
{
	uint16_t clut[16];
	RECT rect;
	int32_t team, i, r, g, b, top;

	for (team = 0; team < 2; team++) {
		setRECT(&rect, 0x80, KAR_TEAM_CLUT_Y, 16, 1);
		StoreImage(&rect, (u_long *)clut);
		DrawSync(0);
		for (i = 0; i < 16; i++) {
			r = clut[i] & 31;
			g = (clut[i] >> 5) & 31;
			b = (clut[i] >> 10) & 31;
			top = r > g ? r : g;
			top = top > b ? top : b;
			if (top - (r < g ? (r < b ? r : b) : (g < b ? g : b)) > 6) { /* only what is coloured */
				if (team == 0) {
					r = top / 4; g = top * 3 / 4; b = top / 4;
				} else {
					r = top; g = top; b = top / 8;
				}
			}
			clut[i] = (clut[i] & 0x8000) | r | (g << 5) | (b << 10);
		}
		setRECT(&rect, KAR_BOARD_CLUT_X + team * 16, KAR_TEAM_CLUT_Y, 16, 1);
		LoadImage(&rect, (u_long *)clut);
		DrawSync(0);
	}
}

/* The blue and red boards' rows share one orange for the power bar's filling and the aiming dots: with
 * more than two colours on the ice each takes its own team's colour, like the green and yellow rows. */
static void KAR_tintFills(void)
{
	/* the row's entries 5-12: three of the bar's filling, five of the dots; the last row is the original's */
	static const uint8_t tint[3][8][3] = {
		{ { 9, 19, 29 }, { 6, 12, 22 }, { 3, 5, 10 }, { 11, 21, 31 }, { 9, 19, 29 }, { 8, 16, 26 }, { 7, 14, 23 }, { 6, 11, 20 } },
		{ { 29, 11, 13 }, { 20, 7, 8 }, { 9, 3, 3 }, { 31, 12, 14 }, { 28, 10, 12 }, { 25, 9, 10 }, { 22, 8, 9 }, { 19, 7, 7 } },
		{ { 31, 22, 0 }, { 20, 11, 0 }, { 9, 4, 0 }, { 28, 27, 19 }, { 25, 24, 16 }, { 22, 21, 13 }, { 19, 18, 11 }, { 16, 15, 8 } },
	};
	uint16_t clut[16];
	RECT rect;
	int32_t team, i;
	const uint8_t *c;

	/* the blue row in place; the red row's copy on a free row, as the hand that picks a stone draws
	 * from the red row itself */
	for (team = 0; team < 2; team++) {
		setRECT(&rect, 0x70 + team * 16, KAR_TEAM_CLUT_Y, 16, 1);
		StoreImage(&rect, (u_long *)clut);
		DrawSync(0);
		for (i = 0; i < 8; i++) {
			c = tint[KAR_CUP.format != KAR_FMT_CLASSIC ? team : 2][i];
			clut[i + 5] = (clut[i + 5] & 0x8000) | c[0] | (c[1] << 5) | (c[2] << 10);
		}
		if (team == 1) {
			rect.x = KAR_RED_FILL_CLUT_X;
		}
		LoadImage(&rect, (u_long *)clut);
		DrawSync(0);
	}
}

/* The two standing spots painted on the ice go: every point of each takes the
 * ice mirrored from just below it, in the background as it sits in video memory. */
static void KAR_clearSpots(void)
{
	static const int16_t centre[2] = { 143, 236 };
	uint16_t to[20], from[20];
	RECT rect;
	int32_t n, y, x, dx, dy, x0;

	for (n = 0; n < 2; n++) {
		x0 = (centre[n] - 20) & ~1;
		for (y = 152 - 13; y <= 152 + 13; y++) {
			setRECT(&rect, 768 + x0 / 2, y, 20, 1);
			StoreImage(&rect, (u_long *)to);
			DrawSync(0);
			rect.y = 2 * (152 + 13) + 1 - y;
			StoreImage(&rect, (u_long *)from);
			DrawSync(0);
			dy = y - 152;
			for (x = 0; x < 40; x++) {
				dx = x0 + x - centre[n];
				if (dx * dx * 169 + dy * dy * 400 <= 169 * 400) {
					((uint8_t *)to)[x] = ((uint8_t *)from)[x];
				}
			}
			rect.y = y;
			LoadImage(&rect, (u_long *)to);
			DrawSync(0);
		}
	}
}

/* Where a team's board group starts, the groups side by side along the top
 * (the face at this corner, the board ten right and twenty-five down). */
static int32_t KAR_boardX(int32_t row)
{
	int32_t pitch = KAR_CUP.teams == 2 ? 0xd2 : KAR_CUP.teams == 3 ? 0x64 : 0x4c;
	int32_t slot = row < 2 ? row : row - 1;

	return -(pitch * KAR_CUP.teams) / 2 + (pitch - KAR_GROUP_W) / 2 + slot * pitch;
}

static void KAR_setupFormat(void)
{
	static const uint8_t teams[5] = { 2, 2, 2, 3, 4 };
	static const uint8_t stones[5] = { 5, 5, 4, 4, 3 };
	int32_t r, i, cup;

	KAR_CUP.format = readPStat(KAR_FORMAT_PSTAT);
	if (KAR_CUP.format > KAR_FMT_FOUR) {
		KAR_CUP.format = KAR_FMT_CLASSIC;
	}
	KAR_CUP.teams = teams[KAR_CUP.format];
	KAR_CUP.stones = stones[KAR_CUP.format];
	KAR_CUP.rows = KAR_CUP.teams + 1;
	KAR_CUP.shown = 1;
	KAR_CUP.playerActor = 0;
	KAR_CUP.turnOrder[0] = 0;
	KAR_CUP.turnOrder[1] = 1;
	KAR_CUP.turnOrder[2] = 3;
	KAR_CUP.turnOrder[3] = 4;
	KAR_CUP.orderFlash = 0;
	KAR_CUP.roundShown = 0;
	KAR_CUP.lunge = 0;
	KAR_CUP.endTimer = 0;
	for (i = 0; i < 16; i++) {
		KAR_CUP.order[i] = KAR_row(0)->stones;
	}
	for (i = 0; i < 8; i++) {
		KAR_CUP.rival[i] = KAR_RIVALS;
	}
	for (r = 0; r < 5; r++) {
		KAR_CUP.actor[r][0] = KAR_CUP.actor[r][1] = 0;
	}
	if (KAR_CUP.format == KAR_FMT_CLASSIC) {
		KAR_CUP.actor[1][0] = KAR_CUP.actor[1][1] = MAIN_D_80135248;
		return;
	}
	/* A cup names the player's rival by round: whoever its draw puts across. */
	cup = readPStat(KAR_CUP_PSTAT);
	if (readPStat(KAR_RIVAL_PSTAT) == 0 && cup >= 1 && cup <= 5) {
		uint8_t draw[14];

		r = readPStat(KAR_CUP_PSTAT + 1);
		if (r >= 1 && r <= 3 && KAR_CUP.teams == 2) {
			KAR_cupDraw(draw);
			writePStat(KAR_RIVAL_PSTAT, r == 1 ? draw[1] : r == 2 ? draw[9] : draw[13]);
		}
	}
	/* in doubles the player's mate is another of the rivals' cup, in the third place */
	r = readPStat(KAR_RIVAL_PSTAT);
	if (KAR_CUP.format == KAR_FMT_DOUBLES && r > 0 && readPStat(KAR_RIVAL_PSTAT + 2) == 0) {
		int32_t tries, second = readPStat(KAR_RIVAL_PSTAT + 1), other;

		cup = (r - 1) / 4 * 4 + 1;
		/* and the rival's own mate when the story named none: three models that fit together */
		for (tries = 0; tries < 32; tries++) {
			other = second;
			while (other == 0 || other == r) {
				other = cup + rand() % 4;
			}
			do {
				i = cup + rand() % 4;
			} while (i == r || i == other);
			if (KAR_RIVALS[r].room + KAR_RIVALS[other].room + KAR_RIVALS[i].room <= KAR_ROOM_MAX) {
				break;
			}
		}
		writePStat(KAR_RIVAL_PSTAT + 1, other);
		writePStat(KAR_RIVAL_PSTAT + 2, i);
	}
	/* the rivals the story named take the map's first places */
	for (i = 0; i < 3; i++) {
		r = readPStat(KAR_RIVAL_PSTAT + i);
		if (r > 0 && r < KAR_RIVAL_ROWS && KAR_RIVALS[r].species >= 0) {
			if (i > 0) {
				MAP_DIGIMON_TABLE[i] = MAP_DIGIMON_TABLE[0];
			}
			MAP_DIGIMON_TABLE[i].typeId = KAR_RIVALS[r].species;
			MAP_DIGIMON_TABLE[i].followMode = 0;
			scriptLoadModel(KAR_RIVALS[r].species);
			scriptSetDigimon(KAR_RIVALS[r].species, i, 0);
			KAR_CUP.rival[i + 2] = &KAR_RIVALS[r];
		}
	}
	/* the rivals are the map's first Digimon, in order */
	MAIN_D_80135248 = 2;
	KAR_CUP.actor[1][0] = KAR_CUP.actor[1][1] = 2;
	KAR_CUP.actor[3][0] = KAR_CUP.actor[3][1] = 3;
	KAR_CUP.actor[4][0] = KAR_CUP.actor[4][1] = 4;
	if (KAR_CUP.format == KAR_FMT_DOUBLES) {
		KAR_CUP.actor[0][1] = ENTITY_TABLE[4] != NULL ? 4 : 1;
		KAR_CUP.actor[1][1] = 3;
	}
	for (i = 1; i < 5; i++) {
		if (ENTITY_TABLE[i] != NULL && KAR_CUP.rival[i] == KAR_RIVALS) {
			KAR_CUP.rival[i] = KAR_findRival(ENTITY_TABLE[i]->type);
		}
	}
	KAR_makeTeamColours();
	KAR_makeBoardColours();
	KAR_tintFills();
	if (KAR_CUP.format != KAR_FMT_CLASSIC && ENTITY_TABLE[1] != NULL && KAR_CUP.actor[0][1] != 1) {
		ENTITY_TABLE[1]->isOnMap = 0;
	}
	if (KAR_CUP.teams > 2 || KAR_CUP.format == KAR_FMT_DOUBLES) {
		KAR_clearSpots();
	}
	/* a partner with no chart icon brings its own */
	if (KAR_CUP.actor[0][1] == 1 && ENTITY_TABLE[1] != NULL && ENTITY_TABLE[1]->type > KAR_CHART_ICONS) {
		loadSpeciesPortrait(ENTITY_TABLE[1]->type, KAR_OWN_ICON_X, KAR_OWN_ICON_Y, 16,
		                    KAR_OWN_ICON_CLUT_X, KAR_TEAM_CLUT_Y);
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
	MAIN_D_80135248 = readPStat(0x7a);
	MAIN_D_80135248 += 2;
	KAR_LOBBY.page = KAR_LOBBY_OFF;
	KAR_LOBBY.written = 0;
	KAR_LOBBY.note = 0;
	KAR_LOBBY.cursor = 0;
	{
		int32_t cup = readPStat(KAR_CUP_PSTAT), round = readPStat(KAR_CUP_PSTAT + 1);

		if (readPStat(KAR_FORMAT_PSTAT) == KAR_FMT_LOBBY) {
			KAR_LOBBY.page = KAR_LOBBY_MODE;
		} else if (readPStat(KAR_FORMAT_PSTAT) != KAR_FMT_CLASSIC && cup >= 1 && cup <= 5 && round >= 1 && round <= 3) {
			/* a round of a cup: its draw first, and the round names its own players */
			writePStat(KAR_RIVAL_PSTAT, 0);
			writePStat(KAR_RIVAL_PSTAT + 1, 0);
			writePStat(KAR_RIVAL_PSTAT + 2, 0);
			KAR_LOBBY.page = KAR_LOBBY_DRAW;
		}
	}
	if (KAR_LOBBY.page != KAR_LOBBY_OFF) {
		/* nothing of the match exists until the menu is through */
		KAR_CUP.format = KAR_FMT_DUEL;
		KAR_CUP.rows = 0;
		KAR_CUP.teams = 2;
		KAR_CUP.playerActor = 0;
		KAR_MOD.valid = 0;
		MAIN_D_80135244 = 0x20;
		clearTextArea();
		KAR_loadBanners();
	} else {
		KAR_setupFormat();
		KAR_setupMatch(0);
	}
	addObjectLate(0x1388, 0xf, KAR_tickStones, KAR_lateStones, KAR_renderScene);
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

	for (p = 0; p < KAR_CUP.rows; p++) {
		stone = KAR_row(p)->stones;
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
					KAR_linkStone(stone, p, models.id[p < 3 ? p : 1]);
					playSound2(8, 4);
				}
			}
		}
	}

	KAR_updateCollisions();
}

static void KAR_heldSpot(int32_t idx, VECTOR *pos)
{
	int32_t style = KAR_style(idx);
	MATRIX world;

	if (style == KAR_ACT_PENGUINMON) {
		KAR_copyVector(pos, &ENTITY_TABLE[idx]->posData->location);
		pos->vx -= 0x50;
	} else if (style == KAR_ACT_GENERIC) {
		/* on the ice beside it, on the side of the middle of the rink */
		KAR_copyVector(pos, &ENTITY_TABLE[idx]->posData->location);
		pos->vx += idx == 1 ? KAR_CUP.rival[idx]->hold : -KAR_CUP.rival[idx]->hold;
	} else {
		/* Evaluate the hand from this pass's skeleton, not a stale workm.
		 * The late tick calls this before interpolation for the logic pose. */
		GsGetLw(&ENTITY_TABLE[idx]->posData[idx == 0 ? 9 : 4].posMatrix, &world);
		pos->vx = world.t[0];
		pos->vy = world.t[1] + 0x78;
		pos->vz = world.t[2];
	}
}

static void KAR_heldPosition(VECTOR *pos)
{
	KAR_heldSpot(KAR_actor(), pos);
}

/* Everyone to their places: who throws next on the two throwing spots, the
 * others of a doubles pair or of a crowded match waiting behind them. */
static void KAR_placeActors(int32_t face, int32_t idle)
{
	/* each team's place on the line, by how many play */
	static const int16_t homeX[3][4] = {
		{ -0x190, 0x190, 0, 0 }, { -0x1f4, 0, 0x1f4, 0 }, { -0x258, -0xc8, 0xc8, 0x258 },
	};
	int32_t r, k, ent, x, thrower, slot = 0;
	PositionData *pos;

	for (r = 0; r < KAR_CUP.rows; r++) {
		if (r == 2) {
			continue;
		}
		x = homeX[KAR_CUP.teams - 2][slot++];
		thrower = r == 0 ? KAR_CUP.playerActor : r == KAR_CUP.shown ? MAIN_D_80135248 : KAR_CUP.actor[r][0];
		for (k = 0; k < 2; k++) {
			ent = KAR_CUP.actor[r][k];
			if ((k == 1 && ent == KAR_CUP.actor[r][0]) || ENTITY_TABLE[ent] == NULL) {
				break;
			}
			pos = ENTITY_TABLE[ent]->posData;
			/* a pair's other half waits beside the thrower, a step back */
			pos->location.vx = x;
			pos->location.vz = 0x578;
			if (KAR_CUP.format == KAR_FMT_DOUBLES) {
				/* a pair side by side, each in its own place whoever throws */
				pos->location.vx = (x < 0 ? -0x190 : 0x190) + (k == (x < 0) ? 0x96 : -0x96);
			} else if (ent != thrower) {
				pos->location.vx = x + (x < 0 ? 0xb4 : -0xb4);
				pos->location.vz = 0x610;
			}
			if (face) {
				pos->rotation.vy = 0;
			}
			if (idle) {
				startAnimation(ENTITY_TABLE[ent], 0);
			}
		}
	}
}

/* The clip of a phase of the throw: 0 taking aim, 1 winding up, 2 letting go. */
static void KAR_throwClip(int32_t phase)
{
	static const uint8_t clips[3][3] = { { 0x23, 0x24, 0x25 }, { 0x1c, 0x1d, 0x1e }, { 0xff, 0x1f, 0x20 } };
	int32_t ent = KAR_actor();
	int32_t style = KAR_style(ent);
	int32_t clip;

	if (style == KAR_ACT_GENERIC) {
		clip = (&KAR_CUP.rival[ent]->aimClip)[phase];
		if (phase == 2) {
			KAR_CUP.lunge = 6;
		}
	} else {
		clip = clips[style][phase];
	}
	if (clip != 0xff) {
		startAnimation(ENTITY_TABLE[ent], clip);
	}
}

static void KAR_setThrower(int32_t row)
{
	int32_t turn = KAR_row(row)->thrown & 1;

	MAIN_D_8013523C = row;
	if (row == 0) {
		KAR_CUP.playerActor = KAR_CUP.actor[0][turn];
	} else {
		MAIN_D_80135248 = KAR_CUP.actor[row][turn];
		KAR_CUP.shown = row;
	}
}

/* In the classic match, fewest stones thrown goes first; level on stones, whoever is behind on
 * points; level on both, the later row (the original's rule for two). */
static int32_t KAR_nextRow(void)
{
	KarStoneRow *row, *best;
	int32_t r, pick = 0;

	if (KAR_CUP.format != KAR_FMT_CLASSIC) {
		/* The new formats throw in rounds of one stone each. The first goes by the
		 * boards, left to right; every later one by the score as it stands when the
		 * round begins, whoever leads first and whoever trails last, level teams
		 * left to right. */
		int32_t least = KAR_CUP.stones, most = 0, n = 0, k;

		for (r = 0; r < KAR_CUP.rows; r++) {
			if (r != 2) {
				if (KAR_row(r)->thrown < least) {
					least = KAR_row(r)->thrown;
				}
				if (KAR_row(r)->thrown > most) {
					most = KAR_row(r)->thrown;
				}
			}
		}
		if (least == most) {
			uint8_t before[4];

			for (k = 0; k < 4; k++) {
				before[k] = KAR_CUP.turnOrder[k];
			}
			for (r = 0; r < KAR_CUP.rows; r++) {
				if (r == 2) {
					continue;
				}
				for (k = n++; k > 0 && KAR_row(KAR_CUP.turnOrder[k - 1])->score < KAR_row(r)->score; k--) {
					KAR_CUP.turnOrder[k] = KAR_CUP.turnOrder[k - 1];
				}
				KAR_CUP.turnOrder[k] = r;
			}
			for (k = 0; k < KAR_CUP.teams; k++) {
				if (before[k] != KAR_CUP.turnOrder[k]) {
					KAR_CUP.orderFlash = 30;
				}
			}
		}
		for (k = 0; k < KAR_CUP.teams; k++) {
			if (KAR_row(KAR_CUP.turnOrder[k])->thrown == least) {
				return KAR_CUP.turnOrder[k];
			}
		}
		return 0;
	}
	for (r = 1; r < KAR_CUP.rows; r++) {
		if (r == 2) {
			continue;
		}
		row = KAR_row(r);
		best = KAR_row(pick);
		if (row->thrown < best->thrown ||
		    (row->thrown == best->thrown && (row->thrown == KAR_CUP.stones || row->score <= best->score))) {
			pick = r;
		}
	}
	return pick;
}

static int32_t KAR_allThrown(void)
{
	int32_t r;

	for (r = 0; r < KAR_CUP.rows; r++) {
		if (r != 2 && KAR_row(r)->thrown < KAR_CUP.stones) {
			return 0;
		}
	}
	return 1;
}

static int32_t KAR_nearestToButton(int32_t r)
{
	KarStone *stone = KAR_row(r)->stones;
	int32_t i, dist, best = 0x7fff;

	for (i = 0; i < 5; i++, stone++) {
		if (stone->state > 0) {
			dist = KAR_distance(stone->pos.vx + KAR_D_8005B438.ring[0].dx, stone->pos.vz + KAR_D_8005B438.ring[0].dz);
			if (dist < best) {
				best = dist;
			}
		}
	}
	return best;
}

/* 0 lost, 1 won, 2 won by a lot, 3 nobody ahead. Level on points, a cup's match is
 * played again; a quick match goes to the stone nearest the button. */
static int32_t KAR_result(void)
{
	int32_t r, dist, mine = KAR_row(0)->score, best = -128, nearest = 0x7fff;

	for (r = 1; r < KAR_CUP.rows; r++) {
		if (r != 2 && KAR_row(r)->score > best) {
			best = KAR_row(r)->score;
		}
	}
	if (mine > best) {
		return mine >= BK(BIG_WIN) ? 2 : 1;
	}
	if (mine < best || KAR_CUP.format == KAR_FMT_CLASSIC) {
		return 0;
	}
	if (readPStat(KAR_CUP_PSTAT) != 0) {
		return 3;
	}
	for (r = 1; r < KAR_CUP.rows; r++) {
		if (r != 2 && KAR_row(r)->score == best) {
			dist = KAR_nearestToButton(r);
			if (dist < nearest) {
				nearest = dist;
			}
		}
	}
	dist = KAR_nearestToButton(0);
	if (dist == nearest) {
		return 3;
	}
	return dist < nearest ? 1 : 0;
}

static void KAR_lateStones(int32_t instanceId)
{
	int32_t p, i, n = 0;
	KarStone *stone;
	KarDrawStone *old, *now;

	(void)instanceId;
	for (p = 0; p < KAR_CUP.rows; p++) {
		stone = KAR_row(p)->stones;
		for (i = 0; i < 5; i++, stone++, n++) {
			if (stone->state == 0) {
				KAR_heldPosition(&stone->pos);
				if (KAR_style(KAR_actor()) != KAR_ACT_PENGUINMON && KAR_style(KAR_actor()) != KAR_ACT_GENERIC) {
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
	if (KAR_LOBBY.page != KAR_LOBBY_OFF) {
		KAR_renderLobby();
		return;
	}
	/* These primitives used to be emitted from KAR_tick, so they vanished
	 * on extra passes. Their animation counters now advance only in the tick. */
	KAR_renderAimArrow();
	KAR_renderStoneCursor();
	KAR_renderScores();
	KAR_renderPowerMeter();
	KAR_renderReadyPrompt();
	KAR_renderNamePlates();
	GsClearOt(0, 0xa, &KAR_D_800638CC[ACTIVE_FRAMEBUFFER]);

	for (p = 0; p < KAR_CUP.rows; p++) {
		stone = KAR_row(p)->stones;
		for (i = 0; i < 5; i++, stone++, n++) {
			if (p != 0 && p != 2 && MAIN_D_80135244 != 3 && MAIN_D_80135244 < 0x10 &&
			    (MAIN_D_80135250 != 0 || MAIN_D_80135244 < 5)) {
				continue;
			}
			/* one rival's waiting stones at a time on the right */
			if (KAR_CUP.format == KAR_FMT_CLASSIC && p != 0 && p != 2 && p != KAR_CUP.shown && stone->state < 0) {
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
				GsSetProjection(preview ? (KAR_CUP.format == KAR_FMT_CLASSIC ? 0x200 : KAR_RACK_PROJ) : VIEWPORT_DISTANCE);
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
			if (KAR_CUP.format != KAR_FMT_CLASSIC && stone->state < 0 && MAIN_D_80135244 == 5 &&
			    p == MAIN_D_8013523C) {
				/* whoever is choosing has its stones large under its board as well, while it chooses */
				pos.vx = (KAR_boardX(p) + 26) * 0xcf8 / 0x200;
				pos.vy = i * 0x8e - 0xc1; /* closer together: the turn queue sits below */
				pos.vz = 0;
				TransMatrix(&stone->coord.coord, &pos);
				stone->coord.flg = 0;
				GsSetProjection(0x200);
				view = 2;
				GsGetLws(stone->obj.coord2, &lw, &ls);
				GsSetLightMatrix(&lw);
				GsSetLsMatrix(&ls);
				GsSortObject4(&stone->obj, &KAR_D_800638CC[ACTIVE_FRAMEBUFFER], 5, getScratchAddr(0));
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
	if (KAR_CUP.format != KAR_FMT_CLASSIC) {
		int32_t r, k;

		for (r = 0; r < KAR_CUP.rows; r++) {
			for (k = 0; r != 2 && k < 2; k++) {
				if (KAR_CUP.actor[r][k] != MAIN_D_80135248) {
					ENTITY_TABLE[KAR_CUP.actor[r][k]]->posData->rotation.vy = 0x800;
					startAnimation(ENTITY_TABLE[KAR_CUP.actor[r][k]], 2);
				}
			}
		}
	}
	if (ENTITY_TABLE[1] != NULL) {
		ENTITY_TABLE[1]->isOnMap = 1;
	}
	SOME_SCRIPT_SYNC_BIT = 1;
	removeObject(0x1388, 0xf);
	setTextColor(1);
	writePStat(0x79, KAR_result());
	{
		/* the last round of a cup won: the cup is the player's, in the saved count */
		int32_t cup = readPStat(KAR_CUP_PSTAT);

		if (cup >= 1 && cup <= 5 && readPStat(KAR_CUP_PSTAT + 1) == 3 && readPStat(0x79) != 0 && readPStat(0x79) != 3 &&
		    readPStat(KAR_PROGRESS_PSTAT) < cup) {
			writePStat(KAR_PROGRESS_PSTAT, cup);
		}
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

	KAR_MOD.tickPrims = 0;
	if (KAR_LOBBY.page != KAR_LOBBY_OFF) {
		KAR_tickLobby();
		return;
	}
	if (KAR_CUP.orderFlash != 0) {
		KAR_CUP.orderFlash--;
	}
	if (KAR_CUP.lunge != 0) {
		/* a Digimon with no throwing clip shoves the stone off with its body */
		KarStone *held = &KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A];
		int32_t step = KAR_CUP.lunge > 3 ? 0x1e : -0x1e;

		ENTITY_TABLE[KAR_actor()]->posData->location.vx += step * rcos(held->angle) / 4096;
		ENTITY_TABLE[KAR_actor()]->posData->location.vz += step * rsin(held->angle) / 4096;
		KAR_CUP.lunge--;
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
		player = KAR_actor();
		KAR_copyVector(&KAR_MOD.previousArrowOrigin, &KAR_MOD.arrowOrigin);
		KAR_MOD.previousArrowRotation = KAR_MOD.arrowRotation;
		KAR_copyVector(&KAR_MOD.arrowOrigin, &KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].pos);
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
		if (KAR_CUP.format != KAR_FMT_CLASSIC) {
			static const uint16_t teamClut[5] = { 0x70, KAR_RED_FILL_CLUT_X, 0, KAR_BOARD_CLUT_X, KAR_BOARD_CLUT_X + 16 };

			setClut(prim, teamClut[MAIN_D_8013523C], 0x1e6);
		}
		prim->tpage = getTPage(0, 0, 640, 0);
		if (i != (MAIN_D_80135238 / 3)) {
			AddPrim(ot + otz, prim);
		}
		prim++;
		GsSetWorkBase((PACKET *)prim);
		prev = out;
	}
}

/* A sprite drawn at another size than its picture's. */
static void KAR_renderSpriteSized(KarSprite *sp, int32_t w, int32_t h)
{
	POLY_FT4 *p;

	p = (POLY_FT4 *)GsGetWorkBase();
	SetPolyFT4(p);
	p->tpage = getTPage(0, 0, 640, 0);
	setRGB0(p, sp->r, sp->g, sp->b);
	setClut(p, sp->clutX, sp->clutY);
	setUVWH(p, sp->u, sp->v, sp->w, sp->h);
	setXYWH(p, sp->x, sp->y, w, h);
	AddPrim(ACTIVE_ORDERING_TABLE->org + KAR_ICON_LAYER, p++);
	GsSetWorkBase((PACKET *)p);
}

void KAR_renderStoneCursor(void)
{
	KarSprite sprite;
	KarStone *stone;

	sprite = KAR_D_8005B478;
	stone = KAR_row(0)->stones;
	if (KAR_MOD.hudState == 5) {
		sprite.x = (MAIN_D_8013523C != 0) ? 0x64 : -0x78;
		if (KAR_CUP.format != KAR_FMT_CLASSIC) {
			sprite.x = KAR_boardX(MAIN_D_8013523C) + 26 - 12;
			sprite.y = (MAIN_D_8013523A * 25) - 65;
		} else {
			sprite.y = (MAIN_D_8013523A * 30) - 0x2d;
		}
		KAR_renderSprite(&sprite);
	}
}

void KAR_renderScores(void)
{
	KarSprite spr;
	KarDigits dig;
	int32_t i;
	int32_t small = 0;
	int16_t lo;
	int16_t hi;
	int16_t x;
	int16_t y;

	spr = KAR_D_8005B48C;
	dig = KAR_D_8005B4A0;
	for (i = 0; i < KAR_CUP.rows; i++) {
		if (i == 2) {
			continue;
		}
		lo = KAR_row(i)->score % 10;
		hi = KAR_row(i)->score / 10;
		x = (i != 0) ? 0x68 : -0x69;
		y = -0x3e;
		if (KAR_CUP.format != KAR_FMT_CLASSIC) {
			x = KAR_boardX(i) + 27;
			y = KAR_BOARD_Y + 25;
			small = 1;
		}
		if (KAR_row(i)->score < 0) {
			hi = 10;
		}
		if (lo < 0) {
			lo *= -1;
		}
		spr.u = dig.glyph[lo].u;
		spr.v = dig.glyph[lo].v;
		spr.x = x;
		spr.y = y;
		KAR_renderSpriteSized(&spr, small ? 12 : 16, small ? 12 : 16);

		spr.u = dig.glyph[hi].u;
		spr.v = dig.glyph[hi].v;
		spr.x = x - (small ? 12 : 0x10);
		spr.y = y;
		KAR_renderSpriteSized(&spr, small ? 12 : 16, small ? 12 : 16);
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
		if (KAR_CUP.format != KAR_FMT_CLASSIC && ENTITY_TABLE[KAR_actor()] != NULL) {
			/* over whoever is throwing, in its team's colour */
			static const uint16_t teamClut[5] = { 0x70, KAR_RED_FILL_CLUT_X, 0, KAR_BOARD_CLUT_X, KAR_BOARD_CLUT_X + 16 };
			VECTOR *loc = &ENTITY_TABLE[KAR_actor()]->posData->location;
			SVECTOR at;
			DVECTOR sxy;

			at.vx = loc->vx;
			at.vy = 0;
			at.vz = loc->vz;
			GsSetLsMatrix(&GsWSMATRIX);
			gte_ldv0(&at);
			gte_rtps();
			gte_stsxy(&sxy);
			arr.sprite[1].x = sxy.vx - 15;
			arr.sprite[1].clutX = teamClut[MAIN_D_8013523C];
		}

		for (i = 10; i > 0; i--) {
			if ((step * i) <= MAIN_D_8013523E) {
				arr.sprite[0].x = (arr.sprite[1].x + i * 3) - 3;
				if (i == 10) {
					arr.sprite[0].clutX = 0x30;
				} else {
					/* filled in the thrower's colour when there are more than two colours */
					arr.sprite[0].clutX = KAR_CUP.format != KAR_FMT_CLASSIC ? arr.sprite[1].clutX : 0x70;
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

/* A Digimon's icon on its two idle frames, `size` wide: the chart's for a
 * chart id, the partner's own for 0. */
static void KAR_renderIcon(int32_t icon, int32_t x, int32_t y, int32_t size, int32_t light)
{
	POLY_FT4 *p;
	KarChartIcon *box;

	p = (POLY_FT4 *)GsGetWorkBase();
	SetPolyFT4(p);
	setRGB0(p, light, light, light);
	if (icon > 0) {
		box = &EVO_CHART_BOXES[icon - 1];
		p->tpage = KAR_CHART_TPAGE;
		p->clut = EVO_CHART_VIEW_COLORS[box->clut];
		setUVWH(p, box->u + ((PLAYTIME_FRAMES % 20) < 10 ? 0 : 0x10), box->v, 15, 15);
	} else {
		p->tpage = getTPage(0, 0, 640, 0);
		setClut(p, KAR_OWN_ICON_CLUT_X, KAR_TEAM_CLUT_Y);
		setUVWH(p, 0, KAR_OWN_ICON_Y, 15, 15);
	}
	setXYWH(p, x, y, size, size);
	AddPrim(ACTIVE_ORDERING_TABLE->org + KAR_ICON_LAYER, p++);
	GsSetWorkBase((PACKET *)p);
}

/* The icon of whoever plays in an entity's place. */
static int32_t KAR_iconOf(int32_t ent)
{
	if (ent == 1) {
		return ENTITY_TABLE[1] != NULL && ENTITY_TABLE[1]->type <= KAR_CHART_ICONS ? ENTITY_TABLE[1]->type : 0;
	}
	return KAR_CUP.rival[ent]->icon;
}

/* MOD (curling cups): the lobby. The room's script sends the player in with the
 * format variable at KAR_FMT_LOBBY and the rink opens on a menu: the kind of
 * match (one against one, doubles, three or four teams), then a quick match
 * against whoever of the city turns up or one of the five cups. A cup shows its
 * draw before each of its rounds. */
static const uint8_t KAR_CUP_FISH[5] = { 62, 63, 64, 66, 67 }; /* what each cup asks for at the door */
static const char *const KAR_MODE_NAMES[3] = { KAR_STR_MODE_SINGLES, KAR_STR_MODE_DOUBLES, KAR_STR_MODE_CROWD };
static const char *const KAR_MODE_HINTS[3] = { KAR_STR_HINT_SINGLES, KAR_STR_HINT_DOUBLES, KAR_STR_HINT_CROWD };
static const char *const KAR_CUP_NAMES[5] = { KAR_STR_CUP_1, KAR_STR_CUP_2, KAR_STR_CUP_3, KAR_STR_CUP_4,
	                                      KAR_STR_CUP_5 };
static const char *const KAR_ROUND_NAMES[3] = { KAR_STR_DRAW_ROUND_1, KAR_STR_DRAW_ROUND_2, KAR_STR_DRAW_ROUND_3 };

enum { KAR_HINT_MODE, KAR_HINT_QUICK = 3, KAR_HINT_FEW, KAR_HINT_ENTRY, KAR_HINT_EARLIER, KAR_HINT_PLAYERS,
       KAR_HINT_NO_FISH, KAR_HINT_NO_ANY_FISH, KAR_HINT_DRAW };

static int32_t KAR_inCity(int32_t row)
{
	return isDigimonRecruited(KAR_RIVALS[row].icon) != 0;
}

/* how many players a quick match of this kind takes besides the tamer, and whether the city has them */
static int32_t KAR_quickOpen(int32_t mode)
{
	int32_t i, n = 0;

	for (i = 1; i <= 20; i++) {
		n += KAR_inCity(i);
	}
	return n >= (mode == 0 ? 1 : mode == 1 ? 3 : 2) ? n : 0;
}

/* 0 when the cup can be entered; else the hint that says why not */
static int32_t KAR_cupClosed(int32_t mode, int32_t cup)
{
	int32_t i;

	if (mode != 0 || readPStat(KAR_PROGRESS_PSTAT) < cup - 1) {
		return KAR_HINT_EARLIER;
	}
	for (i = 0; i < 7; i++) {
		if (!KAR_inCity(KAR_CUP_FIRST[cup - 1] + i)) {
			return KAR_HINT_PLAYERS;
		}
	}
	return 0;
}

/* n of the city's players, all different, whose models fit in the heap together */
static void KAR_drawQuick(int32_t n)
{
	uint8_t pool[20], pick[3];
	int32_t count = 0, i, j, k, tries, room;

	for (i = 1; i <= 20; i++) {
		if (KAR_inCity(i)) {
			pool[count++] = i;
		}
	}
	for (tries = 0; tries < 64; tries++) {
		room = 0;
		for (i = 0; i < n; i++) {
			do {
				k = pool[rand() % count];
				for (j = 0; j < i && pick[j] != k; j++) {
				}
			} while (j < i);
			pick[i] = k;
			room += KAR_RIVALS[k].room;
		}
		if (room <= KAR_ROOM_MAX) {
			break;
		}
	}
	for (i = 0; i < 3; i++) {
		writePStat(KAR_RIVAL_PSTAT + i, i < n ? pick[i] : 0);
	}
}

static void KAR_lobbyText(int32_t row, const char *str, int32_t colour)
{
	RECT area;

	setRECT(&area, 0, row * 13, 0x100, 13);
	clearTextSubArea(&area);
	setTextColor(colour);
	drawString((char *)str, 0, row * 13);
}

static int32_t KAR_lobbyHint(void)
{
	int32_t closed;

	if (KAR_LOBBY.page == KAR_LOBBY_DRAW) {
		return KAR_HINT_DRAW;
	}
	if (KAR_LOBBY.page == KAR_LOBBY_MODE) {
		return KAR_HINT_MODE + KAR_LOBBY.cursor;
	}
	if (KAR_LOBBY.note != 0) {
		return KAR_LOBBY.note;
	}
	if (KAR_LOBBY.cursor == 0) {
		return KAR_quickOpen(KAR_LOBBY.mode) ? KAR_HINT_QUICK : KAR_HINT_FEW;
	}
	closed = KAR_cupClosed(KAR_LOBBY.mode, KAR_LOBBY.cursor);
	return closed != 0 ? closed : KAR_HINT_ENTRY;
}

/* The page's words go to the text area once, and its hint line when it changes. */
/* the rows of the page: the three kinds, or a kind's quick match and, alone against one rival, the five cups */
static int32_t KAR_lobbyRows(void)
{
	if (KAR_LOBBY.page == KAR_LOBBY_MODE) {
		return 3;
	}
	return KAR_LOBBY.mode == 0 ? 6 : 1;
}

static void KAR_writeLobby(void)
{
	char line[48];
	int32_t i, hint;

	if (!KAR_LOBBY.written) {
		clearTextArea();
		if (KAR_LOBBY.page == KAR_LOBBY_MODE) {
			KAR_lobbyText(0, KAR_STR_LOBBY_TITLE, 7);
			for (i = 0; i < 3; i++) {
				KAR_lobbyText(1 + i, KAR_MODE_NAMES[i], 1);
			}
		} else if (KAR_LOBBY.page == KAR_LOBBY_LIST) {
			KAR_lobbyText(0, KAR_MODE_NAMES[KAR_LOBBY.mode], 7);
			KAR_lobbyText(1, KAR_STR_QUICK, KAR_quickOpen(KAR_LOBBY.mode) ? 1 : 9);
			for (i = 1; i < KAR_lobbyRows(); i++) {
				KAR_lobbyText(1 + i, KAR_CUP_NAMES[i - 1], KAR_cupClosed(KAR_LOBBY.mode, i) == 0 ? 1 : 9);
			}
		} else {
			int32_t cup = readPStat(KAR_CUP_PSTAT), round = readPStat(KAR_CUP_PSTAT + 1);

			KAR_lobbyText(0, KAR_CUP_NAMES[cup - 1], 7);
			KAR_lobbyText(1, KAR_ROUND_NAMES[round - 1], 1);
			KAR_cupDraw(KAR_LOBBY.slot);
		}
		KAR_LOBBY.written = 1;
		KAR_LOBBY.hint = 0xff;
	}
	hint = KAR_lobbyHint();
	if (hint != KAR_LOBBY.hint) {
		const char *str;

		KAR_LOBBY.hint = hint;
		switch (hint) {
		case KAR_HINT_QUICK: str = KAR_STR_HINT_QUICK; break;
		case KAR_HINT_FEW: str = KAR_STR_HINT_FEW; break;
		case KAR_HINT_EARLIER: str = KAR_STR_HINT_EARLIER; break;
		case KAR_HINT_PLAYERS: str = KAR_STR_HINT_PLAYERS; break;
		case KAR_HINT_NO_ANY_FISH: str = KAR_STR_HINT_NO_ANY_FISH; break;
		case KAR_HINT_DRAW: str = KAR_STR_HINT_DRAW; break;
		case KAR_HINT_ENTRY:
		case KAR_HINT_NO_FISH:
			sprintf(line, hint == KAR_HINT_ENTRY ? KAR_STR_HINT_ENTRY : KAR_STR_HINT_NO_FISH,
			        ITEM_NAME(KAR_CUP_FISH[KAR_LOBBY.cursor - 1]));
			str = line;
			break;
		default: str = KAR_MODE_HINTS[hint]; break;
		}
		KAR_lobbyText(8, str, hint == KAR_HINT_NO_FISH || hint == KAR_HINT_NO_ANY_FISH ? 3 : 1);
	}
}

static void KAR_lobbyBox(int32_t x, int32_t y, int32_t w, int32_t h, int32_t rgb)
{
	GsBOXF box;

	box.attribute = 0;
	box.r = rgb >> 16;
	box.g = rgb >> 8;
	box.b = rgb;
	box.x = x;
	box.y = y;
	setWH(&box, w, h);
	GsSortBoxFill(&box, ACTIVE_ORDERING_TABLE, 6);
}

/*
 * The cups' banners: KARCUP.DAT holds, for each language (in MOD_LANG's
 * order), five rows of 128 colours and five 256 x 40 pictures of 8 bits
 * (tools/curling_banners.py). The menu's language goes into the video memory
 * the rink leaves free beside the text, read through the world's scratch
 * area; a disc without the file shows the menu without them.
 */
#define KAR_BANNER_FILE    "KARCUP.DAT"
#define KAR_BANNER_SECTORS 26
#define KAR_BANNER_X       576
#define KAR_BANNER_Y       256
#define KAR_BANNER_W       256
#define KAR_BANNER_H       40
#define KAR_BANNER_COLOURS 128

static uint8_t KAR_BANNERS;

extern int32_t modFileOnDisc(char *name);

static void KAR_loadBanners(void)
{
	uint8_t *buf = (uint8_t *)WORLD_SCRATCH_START;
	RECT rect;

	KAR_BANNERS = 0;
	if (!modFileOnDisc(KAR_BANNER_FILE)) {
		return;
	}
	readFileSectors(KAR_BANNER_FILE, buf, MOD_LANG * KAR_BANNER_SECTORS, KAR_BANNER_SECTORS);
	setRECT(&rect, KAR_BANNER_X, KAR_BANNER_Y + 5 * KAR_BANNER_H, KAR_BANNER_COLOURS, 5);
	LoadImage(&rect, (u_long *)buf);
	setRECT(&rect, KAR_BANNER_X, KAR_BANNER_Y, KAR_BANNER_W / 2, 5 * KAR_BANNER_H);
	LoadImage(&rect, (u_long *)(buf + 5 * KAR_BANNER_COLOURS * 2));
	DrawSync(0);
	KAR_BANNERS = 1;
}

/* cup 1..5's banner, its top left at (-128, y) */
static void KAR_renderBanner(int32_t cup, int32_t y)
{
	SPRT *p;
	DR_TPAGE *tp;

	if (!KAR_BANNERS || cup < 1 || cup > 5) {
		return;
	}
	p = (SPRT *)GsGetWorkBase();
	setSprt(p);
	setRGB0(p, 0x80, 0x80, 0x80);
	setXY0(p, -KAR_BANNER_W / 2, y);
	setUV0(p, 0, (cup - 1) * KAR_BANNER_H);
	setWH(p, KAR_BANNER_W, KAR_BANNER_H);
	p->clut = getClut(KAR_BANNER_X, KAR_BANNER_Y + 5 * KAR_BANNER_H + cup - 1);
	AddPrim(ACTIVE_ORDERING_TABLE->org + 6, p++);
	/* linked after the sprite: drawn before it */
	tp = (DR_TPAGE *)p;
	setDrawTPage(tp, 1, 0, getTPage(1, 0, KAR_BANNER_X, KAR_BANNER_Y));
	AddPrim(ACTIVE_ORDERING_TABLE->org + 6, tp++);
	GsSetWorkBase((PACKET *)tp);
}

/* a small gold cup: the mark of a cup already won */
static void KAR_renderTrophy(int32_t x, int32_t y)
{
	KAR_lobbyBox(x + 1, y, 9, 1, 0xfff0a0);
	KAR_lobbyBox(x + 1, y + 1, 9, 1, 0xf0c030);
	KAR_lobbyBox(x + 2, y + 2, 7, 3, 0xf0c030);
	KAR_lobbyBox(x + 3, y + 5, 5, 1, 0xc89018);
	KAR_lobbyBox(x + 5, y + 6, 1, 2, 0xc89018);
	KAR_lobbyBox(x + 3, y + 8, 5, 2, 0xf0c030);
	KAR_lobbyBox(x, y + 1, 1, 3, 0xc89018);
	KAR_lobbyBox(x + 10, y + 1, 1, 3, 0xc89018);
}

static void KAR_lobbyFace(int32_t row, int32_t x, int32_t y, int32_t light)
{
	if (row == 0) {
		KarSpriteSet set;

		set = KAR_D_8005B4F4;
		set.sprite[0].x = x;
		set.sprite[0].y = y;
		set.sprite[0].r = set.sprite[0].g = set.sprite[0].b = light;
		KAR_renderSpriteSized(&set.sprite[0], 16, 16);
	} else {
		KAR_renderIcon(KAR_RIVALS[row].icon, x, y, 16, light);
	}
}

/* one side of a tie of the draw to the next column */
static void KAR_drawLink(int32_t x, int32_t y0, int32_t y1)
{
	drawLine2P(0xa0a0a0, x + 20, y0 + 8, x + 42, y0 + 8, 6, 0);
	drawLine2P(0xa0a0a0, x + 20, y1 + 8, x + 42, y1 + 8, 6, 0);
	drawLine2P(0xa0a0a0, x + 42, y0 + 8, x + 42, y1 + 8, 6, 0);
	drawLine2P(0xa0a0a0, x + 42, (y0 + y1) / 2 + 8, x + 64, (y0 + y1) / 2 + 8, 6, 0);
}

static void KAR_renderDraw(void)
{
	RECT box;
	int32_t round = readPStat(KAR_CUP_PSTAT + 1), i, top = -0x50;
	uint8_t *s = KAR_LOBBY.slot;

	KAR_ICON_LAYER = 6;
	if (KAR_BANNERS) {
		/* the banner names the cup: the round's name beside the final */
		top = -0x44;
		KAR_renderBanner(readPStat(KAR_CUP_PSTAT), -0x74);
		renderString(0, 0x18, -0x46, 0x78, 13, 0, 13, 6, 1);
	} else {
		renderString(0, -0x8c, -0x68, 0x90, 13, 0, 0, 6, 1);
		renderString(0, 0x10, -0x68, 0x80, 13, 0, 13, 6, 1);
	}
	renderString(0, -0x80, top + 0xa8, 0xfc, 13, 0, 104, 6, 1);
	{
		const uint8_t *won = s + 8;

		for (i = 0; i < 8; i++) {
			KAR_lobbyFace(s[i], -0x70, top + i * 20,
			              round == 1 ? (i < 2 ? 0x80 : 0x58) : s[i] == won[i / 2] ? 0x58 : 0x28);
		}
		for (i = 0; i < 4; i++) {
			KAR_drawLink(-0x70, top + i * 40, top + i * 40 + 20);
			if (round >= 2) {
				KAR_lobbyFace(won[i], -0x70 + 0x44, top + 10 + i * 40,
				              round == 2 ? (i < 2 ? 0x80 : 0x58) : won[i] == s[12 + i / 2] ? 0x58 : 0x28);
			} else {
				/* nobody there yet: the line goes on to the next tie */
				drawLine2P(0xa0a0a0, -0x70 + 64, top + 18 + i * 40, -0x70 + 0x44 + 20, top + 18 + i * 40, 6, 0);
			}
		}
		for (i = 0; i < 2; i++) {
			KAR_drawLink(-0x70 + 0x44, top + 10 + i * 80, top + 10 + i * 80 + 40);
			if (round >= 3) {
				KAR_lobbyFace(s[12 + i], -0x70 + 0x88, top + 30 + i * 80, 0x80);
			} else {
				drawLine2P(0xa0a0a0, -0x70 + 0x44 + 64, top + 38 + i * 80, -0x70 + 0x88 + 20, top + 38 + i * 80, 6, 0);
			}
		}
		KAR_drawLink(-0x70 + 0x88, top + 30, top + 30 + 80);
		KAR_renderTrophy(-0x70 + 0xd0, top + 72);
	}
	KAR_ICON_LAYER = 0xa;
	if (KAR_BANNERS) {
		setRECT(&box, -0x94, -0x78, 0x128, 0xf0);
	} else {
		setRECT(&box, -0x94, -0x70, 0x128, 0xe4);
	}
	renderUIBoxPlain(&box, 1, 6);
}

static void KAR_renderLobby(void)
{
	RECT box;
	int32_t i, n;

	if (KAR_LOBBY.page == KAR_LOBBY_LEAVING || !KAR_LOBBY.written) {
		return;
	}
	if (KAR_LOBBY.page == KAR_LOBBY_DRAW) {
		KAR_renderDraw();
		return;
	}
	n = KAR_lobbyRows();
	if (KAR_LOBBY.page == KAR_LOBBY_LIST) {
		KAR_renderBanner(KAR_LOBBY.cursor, -0x74);
		if (KAR_LOBBY.mode == 0 && KAR_LOBBY.cursor >= 1) {
			/* the cup's seven players: the ones the city still lacks stay dark */
			KAR_ICON_LAYER = 6;
			for (i = 0; i < 7; i++) {
				int32_t row = KAR_CUP_FIRST[KAR_LOBBY.cursor - 1] + i;

				KAR_lobbyFace(row, -0x46 + i * 20, 0x56, KAR_inCity(row) ? 0x80 : 0x18);
			}
			KAR_ICON_LAYER = 0xa;
			setRECT(&box, -0x50, 0x52, 0xa0, 0x18);
			renderUIBoxPlain(&box, 1, 6);
		}
	}
	renderString(0, -0x56, -0x40, 0xa0, 13, 0, 0, 6, 1);
	for (i = 0; i < n; i++) {
		renderString(0, -0x50, -0x2c + i * 13, 0xa0, 13, 0, (i + 1) * 13, 6, 1);
		if (KAR_LOBBY.page == KAR_LOBBY_LIST && i > 0 && readPStat(KAR_PROGRESS_PSTAT) >= i) {
			KAR_renderTrophy(0x50, -0x2b + i * 13);
		}
	}
	renderSelectionCursor(-0x54, -0x2c + KAR_LOBBY.cursor * 13, 0xa8, 13, 6);
	renderString(0, -0x80, 0x3e, 0xfc, 13, 0, 104, 6, 1);
	setRECT(&box, -0x60, -0x48, 0xc0, 0x26 + n * 13);
	renderUIBoxPlain(&box, 1, 6);
	setRECT(&box, -0x88, 0x36, 0x110, 0x1a);
	renderUIBoxPlain(&box, 1, 6);
}

static void KAR_lobbyPage(int32_t page, int32_t cursor)
{
	KAR_LOBBY.page = page;
	KAR_LOBBY.cursor = cursor;
	KAR_LOBBY.written = 0;
	KAR_LOBBY.note = 0;
}

static void KAR_lobbyPlay(void)
{
	KAR_LOBBY.page = KAR_LOBBY_OFF;
	setTextColor(1);
	clearTextArea();
	KAR_setupFormat();
	KAR_setupMatch(0);
	MAIN_D_80135244 = 0;
}

static void KAR_lobbyEnter(void)
{
	static const uint8_t formats[3] = { KAR_FMT_DUEL, KAR_FMT_DOUBLES, KAR_FMT_FOUR };
	int32_t mode = KAR_LOBBY.mode, cup = KAR_LOBBY.cursor, fish, i;

	if (cup == 0) {
		int32_t city = KAR_quickOpen(mode);

		if (city == 0) {
			playSound(0, 4);
			return;
		}
		for (fish = 62; fish <= 67 && getItemCount(fish) == 0; fish++) {
		}
		if (fish > 67) {
			KAR_LOBBY.note = KAR_HINT_NO_ANY_FISH;
			playSound(0, 4);
			return;
		}
		removeItem(fish, 1);
		i = formats[mode];
		if (mode == 2 && (city < 3 || (rand() & 1) != 0)) {
			i = KAR_FMT_THREE;
		}
		writePStat(KAR_FORMAT_PSTAT, i);
		writePStat(KAR_CUP_PSTAT, 0);
		writePStat(KAR_CUP_PSTAT + 1, 0);
		KAR_drawQuick(i == KAR_FMT_DUEL ? 1 : i == KAR_FMT_THREE ? 2 : 3);
		playSound(0, 3);
		KAR_lobbyPlay();
		return;
	}
	if (KAR_cupClosed(mode, cup) != 0) {
		playSound(0, 4);
		return;
	}
	if (getItemCount(KAR_CUP_FISH[cup - 1]) == 0) {
		KAR_LOBBY.note = KAR_HINT_NO_FISH;
		playSound(0, 4);
		return;
	}
	removeItem(KAR_CUP_FISH[cup - 1], 1);
	writePStat(KAR_FORMAT_PSTAT, formats[mode]);
	writePStat(KAR_CUP_PSTAT, cup);
	writePStat(KAR_CUP_PSTAT + 1, 1);
	writePStat(KAR_SEED_PSTAT, rand() & 0xff);
	for (i = 0; i < 3; i++) {
		writePStat(KAR_RIVAL_PSTAT + i, 0);
	}
	playSound(0, 3);
	KAR_lobbyPage(KAR_LOBBY_DRAW, 0);
}

static void KAR_tickLobby(void)
{
	int32_t pressed = POLLED_INPUT & ~POLLED_INPUT_PREVIOUS;
	int32_t n = KAR_lobbyRows();

	if (KAR_LOBBY.page == KAR_LOBBY_LEAVING) {
		return;
	}
	if (!KAR_LOBBY.written) {
		/* the page appears with the buttons that chose it still down */
		KAR_writeLobby();
		return;
	}
	if (KAR_LOBBY.page == KAR_LOBBY_DRAW) {
		if (pressed & CONFIRM_BUTTON) {
			playSound(0, 3);
			KAR_lobbyPlay();
		}
		return;
	}
	if (pressed & 0x1000) {
		KAR_LOBBY.cursor = (KAR_LOBBY.cursor + n - 1) % n;
		KAR_LOBBY.note = 0;
		playSound(0, 2);
	} else if (pressed & 0x4000) {
		KAR_LOBBY.cursor = (KAR_LOBBY.cursor + 1) % n;
		KAR_LOBBY.note = 0;
		playSound(0, 2);
	} else if (pressed & CONFIRM_BUTTON) {
		if (KAR_LOBBY.page == KAR_LOBBY_MODE) {
			playSound(0, 3);
			KAR_LOBBY.mode = KAR_LOBBY.cursor;
			KAR_lobbyPage(KAR_LOBBY_LIST, 0);
			return;
		}
		KAR_lobbyEnter();
		if (KAR_LOBBY.page == KAR_LOBBY_OFF || !KAR_LOBBY.written) {
			return;
		}
	} else if (pressed & CANCEL_BUTTON) {
		playSound(0, 4);
		if (KAR_LOBBY.page == KAR_LOBBY_LIST) {
			KAR_lobbyPage(KAR_LOBBY_MODE, KAR_LOBBY.mode);
			return;
		}
		/* out of the rink with nothing played */
		KAR_LOBBY.page = KAR_LOBBY_LEAVING;
		setTextColor(1);
		clearTextArea();
		writePStat(0x79, KAR_RESULT_LEFT);
		MAIN_D_80135244 = 0x14;
		SOME_SCRIPT_SYNC_BIT = 1;
		removeObject(0x1388, 0xf);
		return;
	}
	KAR_writeLobby();
}

void KAR_renderNamePlates(void)
{
	static const uint8_t boardClut[5] = { 0x70, 0x80, 0, KAR_BOARD_CLUT_X, KAR_BOARD_CLUT_X + 16 };
	KarSpriteSet set;
	KarSprite extra;
	KarSprite *p;
	int32_t i, x, light, a, b, k, least = 99;

	set = KAR_D_8005B4F4;
	extra = KAR_D_8005B558;
	p = set.sprite;
	if (KAR_CUP.format == KAR_FMT_CLASSIC) {
		if (MAIN_D_80135248 == 3) {
			p[2] = extra;
		}
		for (i = 0; i < 4; i++, p++) {
			KAR_renderSprite(p);
		}
		return;
	}
	/* one board a team, side by side: its faces, then the board in its stones' colour; whoever is not
	 * throwing a shade darker */
	for (i = 0; i < KAR_CUP.rows; i++) {
		if (i == 2) {
			continue;
		}
		x = KAR_boardX(i);
		/* whoever throws now at full light, whoever has thrown this round darkest */
		if (least == 99) {
			for (k = 0; k < KAR_CUP.rows; k++) {
				if (k != 2 && KAR_row(k)->thrown < least) {
					least = KAR_row(k)->thrown;
				}
			}
		}
		light = i == MAIN_D_8013523C ? 0x80 : KAR_row(i)->thrown > least ? 0x38 : 0x60;
		a = KAR_CUP.actor[i][0];
		b = KAR_CUP.actor[i][1];
		if (i == 0) {
			extra = p[0];
			extra.x = x;
			extra.y = KAR_BOARD_Y;
			extra.r = extra.g = extra.b = light;
			if (b != a) {
				extra.x = x + 2;
				extra.y = KAR_BOARD_Y + 3;
			}
			KAR_renderSpriteSized(&extra, b != a ? 18 : 22, b != a ? 18 : 22);
			if (b != a) {
				KAR_renderIcon(KAR_iconOf(b), x + 25, KAR_BOARD_Y + 3, 18, light);
			}
		} else if (b != a) {
			KAR_renderIcon(KAR_iconOf(a), x + 2, KAR_BOARD_Y + 3, 18, light);
			KAR_renderIcon(KAR_iconOf(b), x + 23, KAR_BOARD_Y + 3, 18, light);
		} else {
			KAR_renderIcon(KAR_iconOf(a), x, KAR_BOARD_Y, 24, light);
		}
		extra = p[1];
		extra.x = x + 8;
		extra.y = KAR_BOARD_Y + 19;
		extra.clutX = boardClut[i];
		extra.r = extra.g = extra.b = light;
		KAR_renderSpriteSized(&extra, 36, 24);
	}
	/* who is still to throw this round, in order, in a corner: the thrower first and larger, each over
	 * a bar of its colour; the line blinks when a new round has changed the order */
	if (KAR_MOD.hudState >= 3 && KAR_MOD.hudState < 0x13) {
		/* "Round N" over the line, written once a round on a row of the text area nothing else uses */
		if (KAR_CUP.roundShown != least + 1 && least < KAR_CUP.stones) {
			char title[32];
			RECT area;

			setRECT(&area, 0, 0xea, 0x70, 13);
			clearTextSubArea(&area);
			sprintf(title, KAR_STR_ROUND, least + 1);
			setTextColor(1);
			drawString(title, 0, 0xea);
			KAR_CUP.roundShown = least + 1;
		}
		renderString(0, -0x94, 0x60 - 22 - 15, 0x70, 13, 0, 0xea, 6, 1);
	}
	x = -0x94;
	for (k = 0; k < KAR_CUP.teams; k++) {
		int32_t size;

		i = KAR_CUP.turnOrder[k];
		if (KAR_row(i)->thrown > least && i != MAIN_D_8013523C) {
			continue;
		}
		size = i == MAIN_D_8013523C ? 22 : 15;
		light = i == MAIN_D_8013523C ? 0x80 : 0x60;
		if (i == MAIN_D_8013523C || ((KAR_CUP.orderFlash / 3) & 1) == 0) {
			/* the face of whoever throws that stone: in doubles the two of a team take turns */
			int32_t who = i == MAIN_D_8013523C ? KAR_actor() : KAR_CUP.actor[i][KAR_row(i)->thrown & 1];

			if (who == 0) {
				extra = p[0];
				extra.x = x;
				extra.y = 0x60 - size;
				extra.r = extra.g = extra.b = light;
				KAR_renderSpriteSized(&extra, size, size);
			} else {
				KAR_renderIcon(KAR_iconOf(who), x, 0x60 - size, size, light);
			}
			extra = p[1];
			extra.x = x;
			extra.y = 0x61;
			extra.clutX = boardClut[i];
			extra.r = extra.g = extra.b = light;
			KAR_renderSpriteSized(&extra, size, 5);
		}
		x += size + 5;
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
	if (KAR_AUTO()) {
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
		for (p = 0; p < KAR_CUP.rows; p++) {
			if (p == 2) {
				continue;
			}
			stone = KAR_row(p)->stones;
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

		if ((KAR_CUP.order[MAIN_D_80135256]->ring != KAR_CUP.order[MAIN_D_80135256]->prevRing) &&
		    (KAR_CUP.order[MAIN_D_80135256]->state < 0x65)) {
			ring = KAR_CUP.order[MAIN_D_80135256]->ring & 0xF;
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
	for (p = 0; p < KAR_CUP.rows; p++) {
		stone = KAR_row(p)->stones;
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
	for (p = 0; p < KAR_CUP.rows; p++) {
		stone2 = KAR_row(p)->stones;
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
	int32_t reruns = 0;

	KAR_bounceOffWall();
	if (MAIN_D_80135244 != 0xb) {
		return;
	}
	/* Collision resolution only changes positive states to other positive
	 * states (the pegged stone becomes 3). Keep all positive stones, even
	 * stationary ones, in the original row/slot order. No pairs are pruned. */
	for (p = 0; p < KAR_CUP.rows; p++) {
		stone = KAR_row(p)->stones;
		for (i = 0; i < 5; i++, stone++) {
			if (stone->state > 0 && count < 15) {
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
		if (collided != 0 && step == 0xa && reruns < 8) { /* MOD (curling collisions): the last step is not repeated without end */
			step--;
			reruns++;
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

	for (p = 0; p < KAR_CUP.rows; p++) {
		stone = KAR_row(p)->stones;
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

/* MOD (curling collisions): two free stones exchange the part of their speed that runs along the line between
 * their centres and keep the rest, and only when they are closing in. Both then stay at the contact positions for
 * the rest of the tick. The original shared the speed by the angle alone and moved both stones by guessed amounts: a
 * stone hit on its side left along the thrower's line, the thrown stone stopped dead, and a grazing touch could
 * leave the pair inside each other, colliding without end. Returns 0 when the stones were moving apart. */
static int32_t KAR_exchangeAlongCentres(KarStone *stoneA, VECTOR a, KarStone *stoneB, VECTOR b)
{
	KarStone *stone[2];
	int32_t along, ang, i;
	int32_t n[2], t[2];

	stone[0] = stoneA;
	stone[1] = stoneB;
	along = ratan2(b.vz - a.vz, b.vx - a.vx);
	for (i = 0; i < 2; i++) {
		n[i] = t[i] = 0;
		if (stone[i]->speed > 0) {
			ang = (stone[i]->angle - along) & 0xfff;
			n[i] = stone[i]->speed * rcos(ang) / 4096;
			t[i] = stone[i]->speed * rsin(ang) / 4096;
		}
	}
	/* Stones moving apart are left alone: rounding can still find them a hair inside each other. */
	if (n[0] * stoneB->weight - n[1] * stoneA->weight <= 0) {
		return 0;
	}
	stoneA->pos.vx = stoneA->target.vx = a.vx;
	stoneA->pos.vz = stoneA->target.vz = a.vz;
	stoneB->pos.vx = stoneB->target.vx = b.vx;
	stoneB->pos.vz = stoneB->target.vz = b.vz;
	for (i = 0; i < 2; i++) {
		stone[i]->speed = KAR_distance(n[1 - i], t[i]);
		stone[i]->angle = (along + ratan2(t[i], n[1 - i])) & 0xfff;
	}
	return 1;
}

void KAR_resolveStoneCollision(KarStone *stoneA, VECTOR a, KarStone *stoneB, VECTOR b)
{
	int16_t speed;

	speed = stoneA->speed + stoneB->speed;
	if (stoneA->state == 2 || stoneA->state == 3 || stoneB->state == 2 || stoneB->state == 3) {
		if (speed >= 0xbb8) {
			playSound2(8, 1);
		} else {
			playSound2(8, 2);
		}
		if (stoneA->state == 2 || stoneA->state == 3) {
			KAR_collidePeggedStone(stoneA, a, stoneB, b);
		} else {
			KAR_collidePeggedStone(stoneB, b, stoneA, a);
		}
	} else if (KAR_exchangeAlongCentres(stoneA, a, stoneB, b)) { /* MOD (curling collisions) */
		if (speed >= 0xbb8) {
			playSound2(8, 1);
		} else {
			playSound2(8, 2);
		}
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

	idx = KAR_actor();

	MAIN_D_80135244 = 6;
	KAR_throwClip(0);

	KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].state = 0;
	if (KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].type == 3) {
		n = (MAIN_D_8013523C != 0) ? 1 : 2;
		KAR_linkStone(&KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A], MAIN_D_8013523C, n);
	}

	MAIN_D_8013524C = -0x400;
	ENTITY_TABLE[idx]->posData->rotation.vy = 0;
}

void KAR_selectPreviousStone(void)
{
	int32_t row;

	if (MAIN_D_8013523A > 0) {
		for (row = MAIN_D_8013523A - 1; row >= 0; row--) {
			if (KAR_row(MAIN_D_8013523C)->stones[row].state == -1) {
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
			if (KAR_row(MAIN_D_8013523C)->stones[row].state == -1) {
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

	idx = KAR_actor();
	ENTITY_TABLE[idx]->posData->rotation.vy -= delta;
	MAIN_D_8013524C += delta;
}

// clang-format off
void KAR_turnAimRight(delta)
	int16_t delta;
// clang-format on
{
	int8_t idx;

	idx = KAR_actor();
	ENTITY_TABLE[idx]->posData->rotation.vy -= delta;
	MAIN_D_8013524C += delta;
}

void KAR_beginThrow(void)
{
	MAIN_D_80135244 = 7;
	KAR_throwClip(1);
}

/* The prompts draw from the tick, so an extra pass replays them before its
 * renders (modReplayTickPrims): same OT order as the first pass. */
enum { KAR_PRIMS_NONE, KAR_PRIMS_YES_NO, KAR_PRIMS_HINT };

static void KAR_drawYesNoPrims(int32_t selection)
{
	RECT finalPos;

	finalPos = MAIN_D_80134A10;
#if !VERSION_IS(US)
	renderString(0, UI_BOX_DATA[0].finalPos.x + 6, UI_BOX_DATA[0].finalPos.y + 6, 0xfc, 0x1a, 0, 0xdc, 6, 1);
	renderSelectionCursor(finalPos.x + 6, finalPos.y + 7 + ((selection - 3) * 13), 0x80, 0xd, 6);
#else
	renderString(0, -0x7c, 0x30, 0x52, 0xd, 0, 0xdc, 6, 1);
	renderString(0, -0x7c, 0x3d, 0x5c, 0xd, 0x53, 0xdc, 6, 1);
	renderSelectionCursor(finalPos.x + 6, finalPos.y + 7 + ((selection - 3) * 13), 0x5e, 0xd, 6);
#endif
}

static void KAR_drawHintPrims(int32_t page)
{
	renderString(0, UI_BOX_DATA[0].finalPos.x + 4, UI_BOX_DATA[0].finalPos.y + 2, 0xfc, 0x34, 0,
	             (page - 1) * 52, 6, 1);
	renderUIBox(0);
}

void KAR_replayTickPrims(void)
{
	if (KAR_MOD.tickPrims == KAR_PRIMS_YES_NO) {
		KAR_drawYesNoPrims(KAR_MOD.tickPrimArg);
	} else if (KAR_MOD.tickPrims == KAR_PRIMS_HINT) {
		KAR_drawHintPrims(KAR_MOD.tickPrimArg);
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
			KAR_MOD.tickPrims = KAR_PRIMS_YES_NO;
			KAR_MOD.tickPrimArg = MAIN_D_80135250;
			KAR_drawYesNoPrims(MAIN_D_80135250);
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

	if (KAR_style(MAIN_D_80135248) == KAR_ACT_PENGUINMON) {
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
			KAR_MOD.tickPrims = KAR_PRIMS_HINT;
			KAR_MOD.tickPrimArg = MAIN_D_80135250;
			KAR_drawHintPrims(MAIN_D_80135250);
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

	for (p = 0; p < KAR_CUP.rows; p++) {
		if (p == 2) {
			continue;
		}
		stone = KAR_row(p)->stones;
		KAR_row(p)->score = 0;
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
				stone->ring |= p << 4;
				break;
			}
		}
	}

	KAR_registerThrownStone();
}

static int32_t KAR_thrownStones(void)
{
	int32_t r, n = 0;

	for (r = 0; r < KAR_CUP.rows; r++) {
		if (r != 2) {
			n += KAR_row(r)->thrown;
		}
	}
	return n;
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

		MAIN_D_80135256 = KAR_thrownStones();
		MAIN_D_80135256--;
		for (i = 0; i < KAR_CUP.rows; i++) {
			KAR_row(i)->score = 0;
		}
		while (MAIN_D_80135256 >= 0) {
			player = KAR_CUP.order[MAIN_D_80135256]->ring >> 4;
			ring = KAR_CUP.order[MAIN_D_80135256]->ring & 0xf;
			if (KAR_CUP.order[MAIN_D_80135256]->state > 0 && ring != 0) {
				KAR_row(player)->score += values.value[ring - 1];
			}
			MAIN_D_80135256--;
		}

		for (i = 0; i < KAR_CUP.rows; i++) {
			rowStone = KAR_row(i)->stones;
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
			ring = KAR_CUP.order[MAIN_D_80135256]->ring & 0xf;
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
		if (KAR_CUP.order[MAIN_D_80135256]->ring != KAR_CUP.order[MAIN_D_80135256]->prevRing) {
			done = tickCameraMoveTo(KAR_CUP.order[MAIN_D_80135256]->pos.vx,
			                        KAR_CUP.order[MAIN_D_80135256]->pos.vz, 5);
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

		player = KAR_CUP.order[MAIN_D_80135256]->ring >> 4;
		ring = KAR_CUP.order[MAIN_D_80135256]->ring & 0xf;
		KAR_row(player)->score += values.value[ring - 1];
		MAIN_D_8013522E = 2;
		if (KAR_CUP.order[MAIN_D_80135256]->ring != KAR_CUP.order[MAIN_D_80135256]->prevRing &&
		    (KAR_CUP.order[MAIN_D_80135256]->ring & 0xf) != 0) {
			if (ring == 3) {
				playSound2(8, 6);
			} else {
				playSound2(8, 5);
			}
		}
	}

	if (MAIN_D_8013522E == 2 && MAIN_D_80135256 >= 0) {
		if (KAR_CUP.order[MAIN_D_80135256]->ring != KAR_CUP.order[MAIN_D_80135256]->prevRing &&
		    (KAR_CUP.order[MAIN_D_80135256]->ring & 0xf) != 0) {
			if (MAIN_D_80135234++ >= 0xb) {
				if (KAR_CUP.order[MAIN_D_80135256]->state < 100) {
					KAR_CUP.order[MAIN_D_80135256]->state += 100;
				} else {
					KAR_CUP.order[MAIN_D_80135256]->state -= 100;
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

/* Nothing within a stone's width of the straight line there (the stone aimed
 * at aside). The original's own test stops at the stone it aims for. */
static int32_t KAR_pathClear(const VECTOR *from, int32_t x, int32_t z, const KarStone *target)
{
	KarStone *stone;
	int32_t ang = ratan2(z - from->vz, x - from->vx);
	int32_t steps = KAR_distance(x - from->vx, z - from->vz) / 30;
	int32_t n, p, i, px, pz;

	for (n = 1; n < steps; n++) {
		px = from->vx + n * 30 * rcos(ang) / 4096;
		pz = from->vz + n * 30 * rsin(ang) / 4096;
		for (p = 0; p < KAR_CUP.rows; p++) {
			stone = KAR_row(p)->stones;
			for (i = 0; i < 5; i++, stone++) {
				if (stone->state > 0 && stone != target &&
				    KAR_distance(px - stone->pos.vx, pz - stone->pos.vz) < 0x97) {
					return 0;
				}
			}
		}
	}
	return 1;
}

/* The speed a stone of this weight leaves with to stop that far away. */
static int32_t KAR_launchSpeed(int32_t weight, int32_t dist)
{
	uint32_t acc = 0, cur = weight, value = weight * 100;

	if (weight <= 0 || dist <= 0) {
		return 0;
	}
	while (acc < (uint32_t)dist) {
		acc += cur / weight;
		value = value * 100 / BKS(FRICTION);
		cur = value / 100;
	}
	return cur;
}

/* A rival of the new formats: by its row of curlingRivals it knocks away the
 * best stone of another team, or draws to the button, or to the far ring when
 * the button is covered. Its aim and its power miss by its own spreads. */
static void KAR_chooseCupShot(void)
{
	const KarRival *me = KAR_CUP.rival[KAR_actor()];
	KarStoneRow *row = KAR_row(MAIN_D_8013523C);
	KarStone *stone, *target = NULL;
	VECTOR from;
	int32_t r, i, n, points, value, best = -1000, plan = 1, tx, tz, angle, power, noise;

	KAR_heldSpot(KAR_actor(), &from);
	if (rand() % 100 < me->hitPercent) {
		for (r = 0; r < KAR_CUP.rows; r++) {
			if (r == 2 || r == MAIN_D_8013523C) {
				continue;
			}
			stone = KAR_row(r)->stones;
			for (i = 0; i < 5; i++, stone++) {
				if (stone->state != 1 || (stone->ring & 0xf) == 0) {
					continue;
				}
				points = MAIN_D_80134A44.value[(stone->ring & 0xf) - 1];
				value = points * 16 + KAR_row(r)->score;
				if (points > 0 && value > best &&
				    KAR_pathClear(&from, stone->pos.vx, stone->pos.vz, stone)) {
					best = value;
					target = stone;
				}
			}
		}
	}
	if (target != NULL) {
		plan = 0;
		tx = target->pos.vx;
		tz = target->pos.vz;
	} else {
		tx = -KAR_D_8005B438.ring[0].dx;
		tz = -KAR_D_8005B438.ring[0].dz;
		if (!KAR_pathClear(&from, tx, tz, NULL)) {
			if (KAR_pathClear(&from, -KAR_D_8005B438.ring[3].dx, -KAR_D_8005B438.ring[3].dz, NULL)) {
				tx = -KAR_D_8005B438.ring[3].dx;
				tz = -KAR_D_8005B438.ring[3].dz;
			} else {
				plan = 0; /* nothing open: through the middle, hard */
			}
		}
	}
	n = -1;
	for (i = 0; i < 4 && n < 0; i++) {
		n = KAR_findUnusedStoneOfType(KAR_D_8005B418.type[plan][i]);
	}
	if (n < 0) {
		n = MAIN_D_8013523A;
	}
	row->unk4 = n;
	stone = &row->stones[n];
	angle = ratan2(tz - from.vz, tx - from.vx);
	noise = me->aimNoise;
	if (noise != 0) {
		angle += rand() % (noise * 2 + 1) - noise;
	}
	r = KAR_distance(tx - from.vx, tz - from.vz);
	if (KAR_style(KAR_actor()) == KAR_ACT_GENERIC) {
		r -= 0x5a; /* its shove carries the stone this far first */
	}
	power = KAR_launchSpeed(stone->weight, r) - 0x258;
	if (plan == 0) {
		power += 0x12c;
	}
	noise = me->powerNoise * 4;
	if (noise != 0) {
		power += rand() % (noise * 2 + 1) - noise;
	}
	if (power < 0x64) {
		power = 0x64;
	}
	if (power > 0xa8c) {
		power = 0xa8c;
	}
	stone->angle = angle;
	row->unk2 = power;
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
			if (KAR_row(0)->score > KAR_row(1)->score) {
				stone = KAR_row(1)->stones;
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
							KAR_row(1)->unk2 = 0x960;
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
						KAR_row(1)->unk2 = 0x960;
					}
				}
			}
		} else {
			KAR_setOpponentShot(1, angle, 0, -0x6a4);
		}
	} else {
		px = ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[0];
		pz = ENTITY_TABLE[MAIN_D_80135248]->posData[4].posMatrix.workm.t[2];

		if (KAR_row(1)->thrown == 0) {
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
					if ((KAR_row(1)->thrown % 2) != 0) {
						angle += rand() % BKS(AI_NOISE);
					} else {
						angle -= rand() % BKS(AI_NOISE);
					}
					KAR_setOpponentShot(0, angle, 0, -0x6a4);
				}
			}
		} else {
			for (i = 0; i < 5; i++) {
				if (KAR_row(1)->stones[(uint32_t)i].state == -1) {
					if (KAR_row(1)->stones[(uint32_t)i].type == 3) {
						KAR_row(1)->unk4 = i;
						angle = ratan2(-0x6a4 - pz, -px);
						clear = KAR_findClearShotAngle(0, -0x6a4);
						if (angle == clear) {
							KAR_setOpponentShot(1, angle, 0, -0x6a4);
						} else {
							KAR_row(1)->unk4 = 6;
						}
						break;
					}
					KAR_row(1)->unk4 = 6;
				}
			}

			if (KAR_row(1)->unk4 == 6) {
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
						if ((KAR_row(1)->thrown % 2) != 0) {
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
						KAR_row(1)->unk2 = 0x960;
					}
					break;
				case 3:
					angle = ratan2(-0x6a4 - pz, -px);
					if ((KAR_row(1)->thrown % 2) != 0) {
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
		for (p = 0; p < KAR_CUP.rows; p++) {
			stone = KAR_row(p)->stones;
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

	stone = KAR_row(player)->stones;
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
			KAR_row(1)->unk4 = n;
			stone = &KAR_row(1)->stones[n];
			break;
		}
	}

	stone->angle = val;
	power = KAR_computeThrowPower(stone->weight, b, c);
	KAR_row(1)->unk2 = power;
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

	stone = KAR_row(0)->stones;
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

	/* the player's own team plans too when its mate throws */
	stone = KAR_row(MAIN_D_8013523C != 0 || KAR_CUP.format != KAR_FMT_CLASSIC ? MAIN_D_8013523C : 1)->stones;
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
	MAIN_D_80135256 = KAR_thrownStones();
	KAR_CUP.order[MAIN_D_80135256] =
		&KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A];
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

	player = KAR_actor();
	switch (MAIN_D_80135244) {
	case 0:
		MAIN_D_80135252 = 0;
		KAR_placeActors(1, 1);
		/* the new formats go straight to the match: no demonstration offered */
		MAIN_D_80135244 = KAR_CUP.format == KAR_FMT_CLASSIC ? 1 : 3;
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
		KAR_row(0)->unk4 = rand() % 5;
		KAR_row(0)->unk2 = 0x7D0;
		KAR_row(0)->stones[KAR_row(0)->unk4].angle = ratan2(-0xC1C, 0x190);
		break;
	case 5:
		if (KAR_AUTO() || (MAIN_D_80135250 != 0)) {
			if (KAR_tickHintBox(0) != 0) {
				break;
			}
			if (MAIN_D_80135252++ >= 0x10) {
				MAIN_D_80135252 -= 0xF;
				if (MAIN_D_8013523A == KAR_row(MAIN_D_8013523C)->unk4) {
					KAR_row(2)->stones[4] =
						KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A];
					KAR_row(2)->stones[4].state = -0x65;
					KAR_beginAiming();
					MAIN_D_80135252 = 0;
					if (MAIN_D_80135250 != 0) {
						MAIN_D_80135250 = 1;
					}
				}
				if (KAR_row(MAIN_D_8013523C)->unk4 > MAIN_D_8013523A) {
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
			if ((POLLED_INPUT & KAR_PICK_PREV) && !(POLLED_INPUT_PREVIOUS & KAR_PICK_PREV)) {
				KAR_selectPreviousStone();
			}
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			else
#endif
			if ((POLLED_INPUT & KAR_PICK_NEXT) && !(POLLED_INPUT_PREVIOUS & KAR_PICK_NEXT)) {
				KAR_selectNextStone();
			}
		}
		MAIN_D_80135254 = 0;
		break;
	case 6:
		if (KAR_AUTO() || (MAIN_D_80135250 != 0)) {
			int32_t targetAngle;

			targetAngle = KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].angle;
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
			KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].pos.vx,
			KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].pos.vz, 0x14);
		if (MAIN_D_80135224 == 1) {
			MAIN_D_80135244 = 8;
		}
		MAIN_D_80135254 = 0;
		break;
	case 8:
		if ((MAIN_D_8013523E += 0x64) >= 0xBB8) {
			MAIN_D_8013523E = 0;
			MAIN_D_80135244 = 6;
			KAR_placeActors(0, 0);
			startAnimation(ENTITY_TABLE[KAR_actor()], 0);
		} else if (KAR_AUTO() || (MAIN_D_80135250 != 0)) {
			if (KAR_tickHintBox(2) != 0) {
				if (KAR_row(MAIN_D_8013523C)->unk2 <= MAIN_D_8013523E) {
					MAIN_D_8013523E = KAR_row(MAIN_D_8013523C)->unk2;
				}
			} else if (KAR_row(MAIN_D_8013523C != 0 || KAR_CUP.format != KAR_FMT_CLASSIC ? MAIN_D_8013523C : 1)->unk2 <= MAIN_D_8013523E) {
				if (KAR_CUP.format != KAR_FMT_CLASSIC) {
					/* its own spread decides the miss, not the meter's steps */
					MAIN_D_8013523E = KAR_row(MAIN_D_8013523C)->unk2;
				}
				MAIN_D_80135244 = 9;
				KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].rotation.vy =
					ENTITY_TABLE[KAR_CUP.format == KAR_FMT_CLASSIC ? MAIN_D_80135248 : KAR_actor()]->posData->rotation.vy;
				if (MAIN_D_80135250 != 0) {
					MAIN_D_80135250 = 1;
				}
			}
		} else if ((POLLED_INPUT & CONFIRM_BUTTON) && !(POLLED_INPUT_PREVIOUS & CONFIRM_BUTTON)) {
			MAIN_D_80135244 = 9;
			KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].angle =
				MAIN_D_8013524C;
			KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].rotation.vy =
				ENTITY_TABLE[KAR_actor()]->posData->rotation.vy;
		}
		break;
	case 9:
		KAR_throwClip(2);
		KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].speed =
			MAIN_D_8013523E + 0x258;
		MAIN_D_80135244 = 0xA;
		MAIN_D_80135252 = 0;
		MAIN_D_80135228 = playSound2(8, 0);
		KAR_D_800638F4 = KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].pos;
		break;
	case 0xA:
		if (MAIN_D_80135252++ >= 3) {
			MAIN_D_80135252 = 0;
			KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].state = 1;
			KAR_D_800638F4 =
				KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].pos;
			MAIN_D_80135244 = 0xB;
		}
		break;
	case 0xB:
		KAR_D_80063904 = KAR_D_800638F4;
		KAR_D_800638F4 =
			KAR_row(MAIN_D_8013523C)->stones[MAIN_D_8013523A].pos;
		moveCameraByDiff(&KAR_D_80063904, &KAR_D_800638F4);
		break;
	case 0xC: {
		KarStone *stone;

		thunkStopSoundMask(MAIN_D_80135228);
		KAR_classifyStoneRings();
		KAR_row(MAIN_D_8013523C)->thrown++;
		MAIN_D_80135244 = 0xD;
		for (i = 0; i < KAR_CUP.rows; i++) {
			if (i == 2) {
				continue;
			}
			stone = KAR_row(i)->stones;
			KAR_row(i)->thrown = 0;
			for (j = 0; j < KAR_CUP.stones; j++, stone++) {
				if (stone->state != -1) {
					KAR_row(i)->thrown++;
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
			KAR_placeActors(0, 1);
			if (MAIN_D_8013523C == 0) {
				KAR_D_80063914.stone =
					KAR_row(0)->stones[MAIN_D_8013523A];
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
				KAR_setThrower(KAR_nextRow());
				KAR_placeActors(0, 1);
				for (j = 0; j < 5; j++) {
					if (KAR_row(MAIN_D_8013523C)->stones[j].state == -1) {
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
			if (KAR_allThrown() && KAR_CUP.format != KAR_FMT_CLASSIC) {
				/* the script does the talking after these matches */
				i = KAR_result();
				playSound2(8, (i == 1 || i == 2) ? 7 : 8);
				KAR_CUP.endTimer = 50;
				MAIN_D_80135244 = 0x15;
			} else if (KAR_allThrown()) {
				MAIN_D_80135250 = 1;
				clearTextArea();
				MAIN_D_80135244 = 0x13;
				if (KAR_row(0)->score > KAR_row(1)->score) {
					playSound2(8, 7);
					MAIN_D_80135252 = 8;
				} else {
					playSound2(8, 8);
					MAIN_D_80135252 = 7;
				}
			} else if (KAR_AUTO()) {
				MAIN_D_80135244 = 0x11;
			} else {
				MAIN_D_80135244 = 5;
			}
		}
		break;
	case 0x11:
		if (KAR_CUP.format != KAR_FMT_CLASSIC) {
			KAR_chooseCupShot();
			MAIN_D_80135244 = 5;
			MAIN_D_80135250 = 0;
			MAIN_D_80135252 = 0;
			MAIN_D_8013523E = 0;
			break;
		}
		KAR_chooseOpponentShot();
		MAIN_D_80135244 = 0x13;
		MAIN_D_80135250 = 1;
		MAIN_D_80135252 = 0;
		MAIN_D_8013523E = 0;
		if (KAR_row(0)->score > KAR_row(1)->score) {
			MAIN_D_80135252 = 5;
		} else if (KAR_row(0)->score == KAR_row(1)->score) {
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
	case 0x15:
		if (KAR_CUP.endTimer == 0) {
			KAR_finishMatch();
		} else {
			KAR_CUP.endTimer--;
		}
		break;
	}
}
