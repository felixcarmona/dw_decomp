#include <malloc.h>
#include <stdlib.h>

#include <libetc.h>
#include <libgpu.h>
#include <libgs.h>
#include <libgte.h>

#include <dw/aabb.h>
#include <dw/clock.h>
#include <dw/combat.h>
#include <dw/entity.h>
#include <dw/graphics.h>
#include <dw/ids.h>
#include <dw/main.h>
#include <dw/math.h>
#include <dw/mod_present.h>
#include <dw/mod_qol.h>
#include <dw/mod_hard.h>
#include <dw/model.h>
#include <dw/move.h>
#include <dw/params.h>
#include <dw/sound.h>
#include <dw/std.h>
#include <dw/types.h>
#include <dw/version.h>
#include <dw/world_object.h>
#include <dw/level.h>
#include <dw/mod_options.h>
#include <dw/cup_rules.h>
#include <dw/mod_rules.h>
#include <dw/tournament.h>

#include "common.h"

#define STD_FINISHER_TIM	((char *)BATTLE_FINISHER_TIM_ADDR)
#define STD_FINISHER_MODEL	((char *)BATTLE_FINISHER_MODEL_ADDR)
#define STD_CONFUSION_MODEL	((char *)BATTLE_CONFUSION_MODEL_ADDR)
#define STD_STUN_MODEL		((char *)BATTLE_STUN_MODEL_ADDR)
#define STD_BUFF_MODEL		((TMDModel *)BATTLE_BUFF_MODEL_ADDR)

extern TMDModel *STD_ARENA_MODEL;
extern int8_t GAME_STATE;
extern int32_t VIEWPORT_DISTANCE;
extern GsOT *ACTIVE_ORDERING_TABLE;
extern int32_t ACTIVE_FRAMEBUFFER;
extern int32_t DRAWING_OFFSET_X;
extern int32_t DRAWING_OFFSET_Y;
extern char *STD_ARENA_TIMS[];
extern char *STD_ARENA_MODELS[];
extern char *STD_ARENA_COLLISIONS[];
extern int8_t MAP_COLLISION_DATA[];

void STD_removeArenaRenderer(void);
void STD_initializeOpponent(int16_t type, int16_t slot, uint8_t tier);
void initializeDigimonObject(int32_t type, int32_t instanceId, void (*tick)(int32_t));
int32_t entityGetTechFromAnim(Entity *entity, int32_t anim);
void scriptLoadModel(int32_t modelId);
void swapByte(uint8_t *a, uint8_t *b);
void STD_loadTIMToVRAM(char *path);
void STD_setupParticipants(uint16_t *out, uint16_t *arg);
void STD_initializeCamera(void);
void STD_renderArena1(void);
void STD_freeArenaModel(void);
void STD_removeOverworldObjects(void);
#if !VERSION_IS(US)
void STD_addArenaRenderer(int32_t which);
#else
void STD_addArenaRenderer(int16_t which);
#endif
int16_t STD_tournamentMain(uint16_t *arg);
int16_t STD_runMatch(Entity *opponent, int16_t b);
void STD_runIntro(int32_t arg);
int32_t STD_combatMain(Entity *entity, Entity *other);
void STD_removeCameraIntro(void);
void GsGetTimInfo(unsigned long *tim, GsIMAGE *img);
void STD_loadArenaTIMToVRAM(char *path, int32_t count);
void STD_initializeLighting();
int32_t STD_isBracketFinished(void);
void STD_renderArena0(void);
void STD_loadArenaAssets(int32_t id);
void STD_initializeFinisherAuraModel(char *tim, char *base);
void STD_initializePoisonBubble(void);
void STD_initializeConfusionEffect(char *base);
void STD_initializeStunEffect(char *base);
void initializeBuffModel(TMDModel *model);
void STD_battleTickFrame(void);
void *allocateArray(uint32_t size);
void freeArray(void *array);
void removeMapEntities(void);
void STD_loadWinLoseModel(void);
void STD_initializeTournament(int32_t arena, uint16_t *arg);
void STD_removeVSPhase(void);
void fadeToBlack(int16_t mode);
void STD_deinitializeTournament(void);
void STD_addPodiumRenderer(void);
void STD_initializeBracket(void);
void STD_addBracket(int32_t track);
void STD_removeBracket(int32_t mode);
void STD_addBracketIntro(void);
void STD_removeBracketIntro(void);
void STD_tickFrames(unsigned short count);
void STD_loadChampionModels(void);
void STD_initializeChampionScene(void);
void STD_addChampionScene(void);
void STD_removeChampionScene(void);
void STD_removePodiumRenderer(void);
void STD_removeLoseScene(void);
void STD_removeWinScene(void);
void STD_setVSPhase(int32_t arg);
void fadeFromBlack(int16_t frames);
int32_t loadMapSounds(int32_t mapSoundId);
uint32_t lookupFileSize(char *path);

static void *std_setup_functions[] = {
	STD_removeArenaRenderer,
	STD_renderArena1,
	STD_renderArena0,
	STD_addArenaRenderer,
	STD_freeArenaModel,
	STD_loadArenaAssets,
	STD_loadArenaTIMToVRAM,
	STD_tournamentMain,
	STD_loadTIMToVRAM,
	STD_initializeLighting,
	STD_removeOverworldObjects,
	STD_deinitializeTournament,
	STD_initializeCamera,
	STD_runMatch,
	STD_initializeOpponent,
	STD_initializeTournament,
};

StdArenaCfg STD_ARENA_COUNTS = { { 3, 4 }, { 4, 1 } };

int16_t STD_MATCH_RESULT;
Entity *STD_OPPONENT_ENTITY;
int32_t STD_CAMERA_STATE;
int32_t MAIN_D_801350F0;
int32_t STD_CHAMPION_SCENE_DONE;
uint8_t STD_MUSIC;

static void *std_setup_sbss_order[] = {
	&STD_MUSIC,
	&STD_CHAMPION_SCENE_DONE,
	&MAIN_D_801350F0,
	&STD_CAMERA_STATE,
	&STD_OPPONENT_ENTITY,
	&STD_MATCH_RESULT,
};

// clang-format off
/* MOD (cup rules): 38 rows, was 24: the cups 23..37 (tools/balance.py writes them from content/tables/tournaments.json) */
int16_t STD_OPPONENT_BASE_STATS[24] = {
	0x0064, 0x0096, 0x00c8, 0x00fa, 0x012c, 0x0000, 0x00c8, 0x00c8,
	0x00c8, 0x00c8, 0x00c8, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa,
	0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x015e, 0x0000,
};
/* MOD (cup rules): the same number for the cups after the original's, 23 onwards (content/tables/tournament-rules.json,
 * newCupStats; balance.py writes it here). */
int16_t STD_NEW_CUP_BASE_STATS[18] = {
	150, 200, 250, 220, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250,
};
#define STD_NEW_CUPS		18
#define STD_NEW_CUP_SECTORS	17
static char STD_PATH_NEW_CUPS[] = "\\STDDAT\\GRADE\\NEWCUPS.BIN";

char STD_PATH_GRADE_STDDAT_GRADE_GRADED_TIM[] = "\\STDDAT\\GRADE\\GRADED.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_GRADEC_TIM[] = "\\STDDAT\\GRADE\\GRADEC.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_GRADEB_TIM[] = "\\STDDAT\\GRADE\\GRADEB.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_GRADEA_TIM[] = "\\STDDAT\\GRADE\\GRADEA.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_GRADES_TIM[] = "\\STDDAT\\GRADE\\GRADES.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_GRADER_TIM[] = "\\STDDAT\\GRADE\\GRADER.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_CAPV1_TIM[24] = "\\STDDAT\\GRADE\\CAPV1.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_CAPV2_TIM[24] = "\\STDDAT\\GRADE\\CAPV2.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_CAPV3_TIM[24] = "\\STDDAT\\GRADE\\CAPV3.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_CAPV4_TIM[24] = "\\STDDAT\\GRADE\\CAPV4.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_CAPVO_TIM[24] = "\\STDDAT\\GRADE\\CAPVO.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_FR_TIM[] = "\\STDDAT\\GRADE\\FR.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_GP_TIM[] = "\\STDDAT\\GRADE\\GP.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_TW_TIM[] = "\\STDDAT\\GRADE\\TW.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_CO_TIM[] = "\\STDDAT\\GRADE\\CO.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_NT_TIM[] = "\\STDDAT\\GRADE\\NT.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_MT_TIM[] = "\\STDDAT\\GRADE\\MT.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_DT_TIM[] = "\\STDDAT\\GRADE\\DT.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_DY_TIM[] = "\\STDDAT\\GRADE\\DY.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_WI_TIM[] = "\\STDDAT\\GRADE\\WI.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_AN_TIM[] = "\\STDDAT\\GRADE\\AN.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_HU_TIM[] = "\\STDDAT\\GRADE\\HU.TIM";

char STD_PATH_GRADE_STDDAT_GRADE_BT_TIM[] = "\\STDDAT\\GRADE\\BT.TIM";

char *STD_GRADES[23] = {
	STD_PATH_GRADE_STDDAT_GRADE_GRADED_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_GRADEC_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_GRADEB_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_GRADEA_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_GRADES_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_GRADER_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_CAPV1_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_CAPV2_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_CAPV3_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_CAPV4_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_CAPVO_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_FR_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_GP_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_TW_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_CO_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_NT_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_MT_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_DT_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_DY_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_WI_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_AN_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_HU_TIM,
	STD_PATH_GRADE_STDDAT_GRADE_BT_TIM,
};

char STD_PATH_STDDAT_STDTIM_BIN[] = "\\STDDAT\\STDTIM.BIN";

char STD_PATH_STDDAT_TIME_TIM[] = "\\STDDAT\\TIME.TIM";

char STD_PATH_ETCDAT_SBOY_TIM[] = "\\ETCDAT\\SBOY.TIM";
// clang-format on

void STD_initializeTournament(int32_t arena, uint16_t *arg)
{
	if (arena == 0) {
		STD_MUSIC = 0x1e;
	} else {
		STD_MUSIC = 0x20;
	}

	GAME_STATE = 5;
	DRAWING_OFFSET_X = 0xa0;
	DRAWING_OFFSET_Y = 0x78;
	STD_D_8007B684[0].length = 2;
	STD_D_8007B684[0].org = STD_D_8007B664;
	STD_D_8007B684[1].length = 2;
	STD_D_8007B684[1].org = STD_D_8007B674;
	STD_D_8007B6AC[0].length = 2;
	STD_D_8007B6AC[0].org = STD_D_8007B6D4;
	STD_D_8007B6AC[1].length = 2;
	STD_D_8007B6AC[1].org = STD_D_8007B6E4;
	removeMapEntities();
	ENTITY_TABLE[0]->isOnScreen = 0;
	STD_removeOverworldObjects();
	STD_initializeCamera();
	STD_initializeLighting(arena);
	STD_loadArenaAssets(arena);
	STD_loadWinLoseModel();
	if (arg[0] < 23) {
		loadTIMFile(STD_GRADES[arg[0]], GENERAL_BUFFER_PTR);
	} else {
		/* MOD (cup rules): the cups after the original's share one file: a banner per language and cup, each
		 * STD_NEW_CUP_SECTORS long (tools/cup_banners.py) */
		loadStackedTIMEntry(STD_PATH_NEW_CUPS, GENERAL_BUFFER_PTR,
				    (MOD_LANG * STD_NEW_CUPS + (arg[0] - 23) % STD_NEW_CUPS) * STD_NEW_CUP_SECTORS, STD_NEW_CUP_SECTORS);
	}
	STD_loadTIMToVRAM(STD_PATH_STDDAT_STDTIM_BIN);
	loadTIMFile(STD_PATH_STDDAT_TIME_TIM, GENERAL_BUFFER);
	STD_initializeFinisherAuraModel(STD_FINISHER_TIM, STD_FINISHER_MODEL);
	STD_initializePoisonBubble();
	STD_initializeConfusionEffect(STD_CONFUSION_MODEL);
	STD_initializeStunEffect(STD_STUN_MODEL);
	initializeBuffModel(STD_BUFF_MODEL);
}

void STD_initializeOpponent(int16_t type, int16_t slot, uint8_t tier)
{
	int32_t nb;
	int32_t na;
	int32_t div;
	int32_t mul;
	uint8_t listB[16];
	uint8_t listA[16];
	int16_t tech;
	Stats *stats;
	int32_t i;
	int16_t base;

	base = tier < 23 ? STD_OPPONENT_BASE_STATS[tier] : STD_NEW_CUP_BASE_STATS[(tier - 23) % STD_NEW_CUPS];
	scriptLoadModel(type);
	ENTITY_TABLE[slot + 2] = (Entity *)&NPC_ENTITIES[slot];
	initializeDigimonObject(type, slot + 2, STD_tickNPCTournament);
	switch (DIGI_PARA(ENTITY_TABLE[slot + 2]->type).level) {
	case 3:
		div = 1;
		mul = 1;
		break;
	case 4:
		div = 1;
		mul = 2;
		break;
	case 5:
		div = 10;
		mul = 25;
		break;
	case 6: /* MOD (mega): x3.5; without it div and mul were left unset */
		div = 10;
		mul = 35;
		break;
	}
	stats = &NPC_ENTITIES[slot].digimonEntity.stats;
	stats->base.off = mul * (base + base * (0x1e - randomLimit(0x3d)) / 100) / div;
	stats->base.def = mul * (base + base * (0x1e - randomLimit(0x3d)) / 100) / div;
	stats->base.speed = mul * (base + base * (0x1e - randomLimit(0x3d)) / 100) / div;
	stats->base.brain = mul * (base + base * (0x1e - randomLimit(0x3d)) / 100) / div;
	stats->base.hp = mul * (base * 10 + base * 10 * (0x1e - randomLimit(0x3d)) / 100) / div;
	stats->base.mp = mul * (base * 10 + base * 10 * (0x1e - randomLimit(0x3d)) / 100) / div;
#if VERSION_IS(JP)
	stats->current.currentHP = stats->base.hp;
	stats->current.currentMP = stats->base.mp;
#endif
	if (stats->base.off >= 0x3e8) {
		stats->base.off = 0x3e7;
	}
	if (stats->base.def >= 0x3e8) {
		stats->base.def = 0x3e7;
	}
	if (stats->base.speed >= 0x3e8) {
		stats->base.speed = 0x3e7;
	}
	if (stats->base.brain >= 0x3e8) {
		stats->base.brain = 0x3e7;
	}
	if (stats->base.hp >= 0x2710) {
		stats->base.hp = 0x270f;
	}
	if (stats->base.mp >= 0x2710) {
		stats->base.mp = 0x270f;
	}

#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	stats->current.currentHP = stats->base.hp;
	stats->current.currentMP = stats->base.mp;
#endif
	modHardEnemy(stats);
	stats->current.chargeMode = 0;
	ENTITY_TABLE[slot + 2]->isOnMap = 1;
	ENTITY_TABLE[slot + 2]->isOnScreen = 1;
	nb = 0;
	na = 0;
	for (i = 0; i < 0xf; i++) {
		tech = entityGetTechFromAnim(ENTITY_TABLE[slot + 2], i + 0x2e);
		if (tech == 0xff) {
			continue;
		}
		if (tech >= 0x3a && tech < 0x71) {
			continue;
		}
		if (stats->base.mp / 10 < MOVE_DATA[tech].mpCost * 3) {
			continue;
		}
		if (MOVE_DATA[tech].range == 4 || tech == 0x2d) {
			listA[na++] = i;
		}
		listB[nb++] = i;
	}
	if (nb != 0) {
		for (i = 0; i < nb; i++) {
			swapByte(&listB[i], &listB[randomLimit(nb)]);
		}
		for (i = 0; i < 3; i++) {
			stats->base.moves[i] = 0xff;
			if (i <= nb - 1) {
				stats->base.moves[i] = listB[i] + 0x2e;
			}
		}
	} else {
		stats->base.moves[0] = 0x2e;
		stats->base.moves[1] = 0xff;
		stats->base.moves[2] = 0xff;
	}
	if (na != 0) {
		for (i = 0; i < na; i++) {
			swapByte(&listA[i], &listA[randomLimit(na)]);
		}
		if (randomLimit(0xa) == 0) {
			stats->base.moves[3] = listA[0] + 0x2e;
		}
	}
	stats->base.moves[3] = 0xff;
	for (i = 0; i < 0x10; i++) {
		if (DIGI_PARA(ENTITY_TABLE[slot + 2]->type).moves[i] >= 0x3a &&
		    DIGI_PARA(ENTITY_TABLE[slot + 2]->type).moves[i] < 0x71) {
			stats->base.moves[3] = i + 0x2e;
			return;
		}
	}
}

int16_t STD_runMatch(Entity *opponent, int16_t b)
{
	int16_t result;

	ENTITY_TABLE[1]->isOnScreen = 1;
	GAME_STATE = 5;
	modSpeedPlateAdd(); /* MOD (battle speed): R2 x1 / x2 from the intro to the end of the match */
	STD_runIntro(b);
	STD_CAMERA_STATE = 1;
	result = STD_combatMain(ENTITY_TABLE[0], opponent);
	modSpeedPlateRemove();
	STD_removeCameraIntro();
	STD_CAMERA_STATE = 10;
	return result;
}

void STD_initializeCamera(void)
{
	VIEWPORT_DISTANCE = 500;
	GsSetProjection(500);
	STDVS_CAMERA.rotation.vx = 100;
	STDVS_CAMERA.rotation.vy = 0;
	STDVS_CAMERA.rotation.vz = 0;
	STDVS_CAMERA.translation.vx = 0;
	STDVS_CAMERA.translation.vy = 500;
	STDVS_CAMERA.translation.vz = 3000;
	STDVS_CAMERA.view.super = NULL;
	RotMatrix(&STDVS_CAMERA.rotation, &STDVS_CAMERA.view.view);
	TransMatrix(&STDVS_CAMERA.view.view, &STDVS_CAMERA.translation);
	STDVS_CAMERA.coord.flg = 0;
	GsSetView2(&STDVS_CAMERA.view);
}

void STD_deinitializeTournament(void)
{
	RECT rect;
	int32_t i;

	i = 0;
	stopBGM();
	stopSound();
	STD_removeArenaRenderer();
	STD_removeVSPhase();
	STD_freeArenaModel();
	setRECT(&rect, 0, 0, 0x140, 0x1e0);
	ClearImage(&rect, 0, 0, 0);
	setRECT(&rect, 0x300, 0, 0xff, 0x180);
	ClearImage(&rect, 0, 0, 0);
	setRECT(&rect, 0x180, 0, 0x180, 0x100);
	ClearImage(&rect, 0, 0, 0);
	DrawSync(0);
	ENTITY_TABLE[1]->isOnScreen = 0;
	loadTIMFile(STD_PATH_ETCDAT_SBOY_TIM, GENERAL_BUFFER_PTR);
	fadeToBlack(1);

	for (; i < 0xb; i++) {
		STD_battleTickFrame();
	}

	ENTITY_TABLE[1]->posData->location.vy = 0;
	ENTITY_TABLE[1]->anim.locY = 0;
	ENTITY_TABLE[0]->isOnScreen = 1;
	GAME_STATE = 0;
}

void STD_removeOverworldObjects(void)
{
	removeObject(0xfa2, 0);
	removeObject(0xfa0, 0);
#if VERSION_IS(JP)
	removeObject(0xfa6, 0);
#endif
	removeObject(0xfa8, 0);
}

void STD_initializeLighting(void)
{
	LIGHT_DATA.light[0].vx = 1000;
	LIGHT_DATA.light[0].vy = 1000;
	LIGHT_DATA.light[0].vz = 1000;
	LIGHT_DATA.light[0].r = 255;
	LIGHT_DATA.light[0].g = 255;
	LIGHT_DATA.light[0].b = 255;
	GsSetFlatLight(0, &LIGHT_DATA.light[0]);
	LIGHT_DATA.light[1].vx = -1000;
	LIGHT_DATA.light[1].vy = 1000;
	LIGHT_DATA.light[1].vz = -1000;
	LIGHT_DATA.light[1].r = 160;
	LIGHT_DATA.light[1].g = 160;
	LIGHT_DATA.light[1].b = 160;
	GsSetFlatLight(1, &LIGHT_DATA.light[1]);
	LIGHT_DATA.light[2].vx = -20;
	LIGHT_DATA.light[2].vy = 20;
	LIGHT_DATA.light[2].vz = 200;
	LIGHT_DATA.light[2].r = 96;
	LIGHT_DATA.light[2].g = 96;
	LIGHT_DATA.light[2].b = 96;
	GsSetFlatLight(2, &LIGHT_DATA.light[2]);
	GsSetAmbient(0x400, 0x400, 0x400);
	GsSetLightMode(0);
}

void STD_loadTIMToVRAM(char *path)
{
	GsIMAGE img;
	int32_t *p;
	int32_t i;

	p = (int32_t *)GENERAL_BUFFER;
	readFile(path, p);
	for (i = 0; i < 6; i++) {
		p = (int32_t *)((char *)p + 4);
		GsGetTimInfo((u_long *)p, &img);
		p += ((img.pw * img.ph) / 2) + 4;
		LoadImage((RECT *)&img.px, img.pixel);
		if ((img.pmode >> 3) & 1) {
			LoadImage((RECT *)&img.cx, img.clut);
			p += ((img.cw * img.ch) / 2) + 3;
		}
	}
}

/* MOD (rules as data): written by tools/balance.py build from content/tables/tournament-rules.json; -1 = does not exist */
#define STD_REMAP_MAX 4
int16_t STD_REMAP[STD_REMAP_MAX][2] = { { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 } };
int16_t STD_ARENA_SCREEN = -1;

/*
 * MOD (cup rules): the seven rivals of the cup arg[0], written to arg[1..7]. It was startTournament's work, in the main
 * executable; here it costs the field no memory.
 *
 * A cup of the original's lists: the recruited originals of its list (all the recruited ones when the list is empty), and
 * species of DIGI.DAT whose level lies in the levels of that list (a Mega is not drawn), up to as many as half the pool, at
 * least seven. The special cup 0x16 fields its own list and nothing else.
 *
 * A cup with a rule (CUP_RULE_MARK in its entry list): CUP_RULE_EXACT draws only the recruited species of its rival list
 * (the whole list while none is recruited). Any other rule adds every species that meets it, an original once it is
 * recruited and a species of DIGI.DAT always. The cups that mix levels (attribute, Field) keep to the partner's level and
 * the ones next to it while seven such rivals exist.
 */
#define STD_POOL_MAX	640

static void STD_pickOpponents(uint16_t *arg)
{
	uint8_t *jumpTable;
	uint8_t *rule;
	uint8_t *p;
	uint16_t *pool;
	uint32_t *tags;
	uint32_t ref;
	uint32_t tag;
	int32_t kind;
	int32_t value;
	int32_t partner;
	int32_t own;
	int32_t count;
	int32_t listed;
	int32_t lo;
	int32_t hi;
	int32_t level;
	int32_t pass;
	int32_t id;
	int32_t i;
	int32_t k;
	int32_t n;
	uint16_t t;

	partner = PARTNER_ENTITY.digimonEntity.entity.type;
	own = DIGI_PARA(partner).level;
	jumpTable = getCupDataJumpTable(10, arg[0]);
	rule = getCupDataJumpTableEntry(jumpTable, 3) + 2;
	p = getCupDataJumpTableEntry(jumpTable, 4) + 2;
	kind = -1;
	value = 0;
	if (*rule == CUP_RULE_MARK) {
		kind = rule[1];
		value = rule[2];
	}
	pool = (uint16_t *)allocateArray(STD_POOL_MAX * 2);
	tags = (uint32_t *)allocateArray(0x800);
	count = 0;
	lo = MOD_TOURNAMENT[MOD_TR_POOL_LOW];
	hi = MOD_TOURNAMENT[MOD_TR_POOL_HIGH];
	if (*p < 0xfe) {
		lo = 9;
		hi = 0;
		while ((i = *p++) < 0xfe) {
			k = DIGI_PARA(i).level > LEVEL_ULTIMATE ? LEVEL_ULTIMATE : DIGI_PARA(i).level;
			if (k < lo) {
				lo = k;
			}
			if (k > hi) {
				hi = k;
			}
			if (arg[0] == 0x16 || isDigimonRecruited(i)) {
				pool[count++] = i;
			}
		}
		if (count == 0 && kind == CUP_RULE_EXACT) {
			p = getCupDataJumpTableEntry(jumpTable, 4) + 2;
			while ((i = *p++) < 0xfe) {
				pool[count++] = i;
			}
		}
	} else if (kind < 0) {
		for (i = 0; i < NUM_RECRUITABLE_DIGIMON; i++) {
			if (isDigimonRecruited(i)) {
				pool[count++] = i;
			}
		}
	}
	if (lo > MOD_TOURNAMENT[MOD_TR_POOL_HIGH]) {
		lo = MOD_TOURNAMENT[MOD_TR_POOL_HIGH];
	}
	if (hi > MOD_TOURNAMENT[MOD_TR_POOL_HIGH]) {
		hi = MOD_TOURNAMENT[MOD_TR_POOL_HIGH];
	}
	listed = count;
	if (arg[0] != 0x16 && kind != CUP_RULE_EXACT) {
		while (isFileReadQueueBusy()) {
			tickFileReadQueue(0);
		}
		readFileSectors(SPECIES_FILE_NAME, tags, 0, 1);
		ref = tags[SPECIES_HEADER_CUP_TAGS / 4];
		if (tags[0] != 0x49474944) {
			ref = 0;
		}
		/* pass 0: the mixed cups keep near the partner's level; pass 1: anyone, when that left fewer than seven */
		for (pass = kind == CUP_RULE_ATTRIBUTE || kind == CUP_RULE_FIELD ? 0 : 1; pass < 2; pass++) {
			count = listed;
			for (k = 0; k < (int32_t)(ref >> 24); k++) {
				readFileSectors(SPECIES_FILE_NAME, tags, (ref & 0xffffff) + k, 1);
				for (i = 0; i < CUP_TAGS_PER_SECTOR && count < STD_POOL_MAX; i++) {
					id = k * CUP_TAGS_PER_SECTOR + i;
					tag = tags[i];
					if (tag == 0 || id == partner) {
						continue;
					}
					level = id < NUM_ORIGINAL_DIGIMON ? DIGI_PARA(id).level : (int32_t)CUP_TAG_LEVEL(tag);
					if (kind < 0) {
						if (!(tag & CUP_TAG_RIVAL) || level < lo || level > hi) {
							continue;
						}
					} else {
						if (!cupRuleHolds(kind, value, tag, level) || level < LEVEL_ROOKIE) {
							continue;
						}
						if (!(tag & CUP_TAG_RIVAL) && !(id < NUM_RECRUITABLE_DIGIMON && isDigimonRecruited(id))) {
							continue;
						}
						if (pass == 0 && (level < own - 1 || level > own + 1)) {
							continue;
						}
						for (n = 0; n < listed && pool[n] != id; n++) {
						}
						if (n < listed) {
							continue;
						}
					}
					pool[count++] = id;
				}
			}
			if (count >= 7) {
				break;
			}
		}
		if (kind < 0) {
			/* MOD (economy as data): MOD_TOURNAMENT's pool rules: how many of them join */
			n = count - listed;
			k = listed > MOD_TOURNAMENT[MOD_TR_POOL_EXTRA_MIN] && MOD_TOURNAMENT[MOD_TR_POOL_EXTRA_DIV] > 0 ? listed / MOD_TOURNAMENT[MOD_TR_POOL_EXTRA_DIV] : MOD_TOURNAMENT[MOD_TR_POOL_EXTRA_MIN];
			if (k < 0) {
				k = 0;
			}
			if (k > 64) {
				k = 64;
			}
			if (k > n) {
				k = n;
			}
			for (i = 0; i < k; i++) {
				id = listed + i + randomLimit(n - i);
				t = pool[id];
				pool[id] = pool[listed + i];
				pool[listed + i] = t;
			}
			count = listed + k;
		}
	}
	freeArray(tags);
	if (count == 0) {
		pool[count++] = partner;
	}
	arg++;
	if (count <= 7) {
		for (k = 0; k < count; k++) {
			*arg++ = pool[k];
		}
		for (; k < 7; k++) {
			*arg++ = pool[randomLimit(count)];
		}
	} else {
		for (k = 0; k < count; k++) {
			if (pool[k] == partner) {
				pool[k] = pool[--count];
				break;
			}
		}
		for (k = 0; k < 7; k++) {
			n = randomLimit(count);
			*arg++ = pool[n];
			pool[n] = pool[--count];
		}
	}
	freeArray(pool);
}

int16_t STD_tournamentMain(uint16_t *arg)
{
	int32_t j;
	int32_t m;
	uint16_t local[4]; /* MOD (portraits): was u8 */
	int16_t arena;
	long n;
	long i;

	STD_pickOpponents(arg);
	/* MOD (rules as data): the opponents whose models do not exist here are swapped for another species: STD_REMAP */
	for (n = 1; n < 8; n++) {
		for (i = 0; i < STD_REMAP_MAX; i++) {
			if (STD_REMAP[i][0] >= 0 && arg[n] == STD_REMAP[i][0]) {
				arg[n] = STD_REMAP[i][1];
				break;
			}
		}
	}
	fadeToBlack(5);
	STD_tickFrames(5);
	loadMapSounds(0x10);
	if (STD_ARENA_SCREEN >= 0 && CURRENT_SCREEN == STD_ARENA_SCREEN) { /* MOD (rules as data) */
		arena = 1;
	} else {
		arena = 0;
	}
	STD_initializeTournament(arena, arg);
	STD_initializeBracket();
	STD_setupParticipants(local, arg);
	STD_setVSPhase(0xa);
	STD_addArenaRenderer(arena);
	STD_addBracketIntro();
	fadeFromBlack(5);
	STD_tickFrames(0x78);
	STD_removeBracketIntro();
	STD_MATCH_RESULT = 0;
	i = 0;
	j = 0;
	while (STD_MATCH_RESULT != -1 && i != 3) {
		STD_addBracket(1);
		loadMapSounds(0x10);
		while (STD_isBracketFinished() == 0) {
			STD_battleTickFrame();
		}
		STD_removeBracket(1);
		STD_initializeOpponent(local[i], i, arg[0]);
		STD_OPPONENT_ENTITY = ENTITY_TABLE[i + 2];
		STD_MATCH_RESULT = STD_runMatch(ENTITY_TABLE[i + 2], arena);
		j++;
		if (STD_MATCH_RESULT == 1) {
			loadMapSounds(0x10);
			STD_removeWinScene();
			STD_initializeCamera();
			STD_CAMERA_STATE = 0xa;
		} else {
			STD_removeLoseScene();
		}
		removeMapEntities();
		GAME_STATE = 5;
		i++;
	}
	if (STD_MATCH_RESULT == 1) {
		STD_loadChampionModels();
		STD_addBracket(2);
		while (STD_isBracketFinished() == 0) {
			STD_battleTickFrame();
		}
		fadeToBlack(0xa);
		for (m = 0; m < 0xb; m++) {
			STD_battleTickFrame();
		}
		STD_removeBracket(2);
		STD_addPodiumRenderer();
		STD_initializeChampionScene();
		STD_battleTickFrame();
		STD_battleTickFrame();
		fadeFromBlack(0xf);
		for (m = 0; m < 0x10 || MAIN_D_801350F0 > 0; m++) {
			STD_battleTickFrame();
		}
		STD_addChampionScene();
		while (STD_CHAMPION_SCENE_DONE == 0) {
			STD_battleTickFrame();
		}
		STD_removePodiumRenderer();
		STD_removeChampionScene();
	}
	STD_deinitializeTournament();
	if (STD_MATCH_RESULT == 1) {
		return i;
	}

	return i - 1;
}

void STD_loadArenaTIMToVRAM(char *path, int32_t count)
{
	GsIMAGE img;
	int32_t *p;
	int32_t i;

	p = (int32_t *)GENERAL_BUFFER;
	readFile(path, p);
	for (i = 0; i < count; i++) {
		p = (int32_t *)((char *)p + 4);
		GsGetTimInfo((u_long *)p, &img);
		p += ((img.pw * img.ph) / 2) + 4;
		LoadImage((RECT *)&img.px, img.pixel);
		if ((img.pmode >> 3) & 1) {
			LoadImage((RECT *)&img.cx, img.clut);
			p += ((img.cw * img.ch) / 2) + 3;
		}
	}
}

// clang-format off
void STD_loadArenaAssets(id)
	uint8_t id;
// clang-format on
{
	int32_t i;

	STD_loadArenaTIMToVRAM(STD_ARENA_TIMS[id], ((uint8_t *)STD_ARENA_COUNTS.timCount)[id]);
	STD_ARENA_MODEL = malloc3(((int32_t)lookupFileSize(STD_ARENA_MODELS[id]) + 0x7ff) & ~0x7ff);
	readFile(STD_ARENA_MODELS[id], STD_ARENA_MODEL);
	GsMapModelingData((u_long *)&STD_ARENA_MODEL->flags);
	for (i = 0; i < STD_ARENA_COUNTS.modelCount[id]; i++) {
		GsLinkObject4((u_long)STD_ARENA_MODEL->obj, &STD_ARENA_OBJECTS[i], i);
		GsInitCoordinate2(NULL, &STD_ARENA_COORDS[i].coord);
		STD_ARENA_OBJECTS[i].attribute = 0;
		STD_ARENA_OBJECTS[i].coord2 = &STD_ARENA_COORDS[i].coord;
	}
	readFile(STD_ARENA_COLLISIONS[id], MAP_COLLISION_DATA);
}

void STD_freeArenaModel(void)
{
	free3(STD_ARENA_MODEL);
}

// clang-format off
#if !VERSION_IS(US)
void STD_addArenaRenderer(which)
	uint8_t which;
#else
void STD_addArenaRenderer(int16_t which)
#endif
// clang-format on
{
	switch (which) {
	case 0:
		addObject(0x1a7, 0, (TickFunction)0, (RenderFunction)STD_renderArena0);
		break;
	case 1:
		addObject(0x1a7, 0, (TickFunction)0, (RenderFunction)STD_renderArena1);
		break;
	}
}

void STD_renderArena0(void)
{
	MATRIX m;
	int32_t i;

	if (MOD_TICK_FAST && !MOD_REAL_TICK) {
		return; /* MOD (battle speed): this frame is thrown away; sorting the arena only cost time */
	}
	GsGetLw(&STD_ARENA_COORDS[1].coord, &m);
	GsSetLightMatrix(&m);
	GsGetLs(&STD_ARENA_COORDS[1].coord, &m);
	GsSetLsMatrix(&m);
	GsSortObject4(&STD_ARENA_OBJECTS[1], ACTIVE_ORDERING_TABLE, 2, getScratchAddr(0));
	if (STD_isBracketFinished() == 1) {
		STD_ARENA_OBJECTS[0].attribute |= 0x200;
	} else {
		STD_ARENA_OBJECTS[0].attribute = 0;
	}
	GsClearOt(0, 0xfff, &STD_D_8007B684[ACTIVE_FRAMEBUFFER]);
	GsClearOt(0, 0xffe, &STD_D_8007B6AC[ACTIVE_FRAMEBUFFER]);
	for (i = 2; i >= 0; i--) {
		if (i == 1) {
			continue;
		}
		GsGetLw(&STD_ARENA_COORDS[i].coord, &m);
		GsSetLightMatrix(&m);
		GsGetLs(&STD_ARENA_COORDS[i].coord, &m);
		GsSetLsMatrix(&m);
		switch (i) {
		case 0:
			GsSortObject4(&STD_ARENA_OBJECTS[i], &STD_D_8007B6AC[ACTIVE_FRAMEBUFFER], 0xc, getScratchAddr(0));
			break;
		case 2:
			GsSortObject4(&STD_ARENA_OBJECTS[i], &STD_D_8007B684[ACTIVE_FRAMEBUFFER], 0xc, getScratchAddr(0));
			break;
		}
	}
	GsSortOt(&STD_D_8007B684[ACTIVE_FRAMEBUFFER], ACTIVE_ORDERING_TABLE);
	GsSortOt(&STD_D_8007B6AC[ACTIVE_FRAMEBUFFER], ACTIVE_ORDERING_TABLE);
}

void STD_renderArena1(void)
{
	MATRIX m;
	int32_t i;

	if (MOD_TICK_FAST && !MOD_REAL_TICK) {
		return; /* MOD (battle speed): this frame is thrown away; sorting the arena only cost time */
	}
	if (STD_isBracketFinished() == 1) {
		GsGetLw(&STD_ARENA_COORDS[3].coord, &m);
		GsSetLightMatrix(&m);
		GsGetLs(&STD_ARENA_COORDS[3].coord, &m);
		GsSetLsMatrix(&m);
		GsSortObject4(&STD_ARENA_OBJECTS[3], ACTIVE_ORDERING_TABLE, 2, getScratchAddr(0));
	}
	if (STD_isBracketFinished() == 1) {
		STD_ARENA_OBJECTS[0].attribute |= 0x200;
	} else {
		STD_ARENA_OBJECTS[0].attribute = 0;
	}
	GsClearOt(0, 0xfff, &STD_D_8007B684[ACTIVE_FRAMEBUFFER]);
	GsClearOt(0, 0xffe, &STD_D_8007B6AC[ACTIVE_FRAMEBUFFER]);
	for (i = 2; i >= 0; i--) {
		GsGetLw(&STD_ARENA_COORDS[i].coord, &m);
		GsSetLightMatrix(&m);
		GsGetLs(&STD_ARENA_COORDS[i].coord, &m);
		GsSetLsMatrix(&m);
		switch (i) {
		case 0:
			GsSortObject4(&STD_ARENA_OBJECTS[i], &STD_D_8007B6AC[ACTIVE_FRAMEBUFFER], 0xc, getScratchAddr(0));
			break;
		case 1:
		case 2:
			GsSortObject4(&STD_ARENA_OBJECTS[i], &STD_D_8007B684[ACTIVE_FRAMEBUFFER], 0xc, getScratchAddr(0));
			break;
		}
	}
	GsSortOt(&STD_D_8007B684[ACTIVE_FRAMEBUFFER], ACTIVE_ORDERING_TABLE);
	GsSortOt(&STD_D_8007B6AC[ACTIVE_FRAMEBUFFER], ACTIVE_ORDERING_TABLE);
}

void STD_removeArenaRenderer(void)
{
	removeObject(0x1a7, 0);
}
