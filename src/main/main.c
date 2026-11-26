#include <stdlib.h>

#include <__rts_info_t__.h>

#include <libcd.h>
#include <libetc.h>
#include <libgpu.h>
#include <libgs.h>

#include <dw/anim.h>
#include <dw/attack_object.h>
#include <dw/bubble.h>
#include <dw/butterfly.h>
#include <dw/clock.h>
#include <dw/combat.h>
#include <dw/efe.h>
#include <dw/entity.h>
#include <dw/fade.h>
#include <dw/font.h>
#include <dw/garbage.h>
#include <dw/ids.h>
#include <dw/item.h>
#include <dw/main.h>
#include <dw/map.h>
#include <dw/map_object.h>
#include <dw/mod_fps.h>
#include <dw/model.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/tamer.h>
#include <dw/types.h>
#include <dw/ui.h>
#include <dw/utils.h>
#include <dw/version.h>
#include <dw/vs.h>
#include <dw/world_object.h>
#include <text/main/main.h>

#include "common.h"

int32_t isTriggerSet(uint16_t trigger);
void unsetTrigger(uint16_t trigger);

extern uint8_t INVENTORY_POINTER;

extern StatsGains INITIAL_COMBAT_STATS[];

typedef struct {
	int16_t spawnX[10];
	int16_t spawnY[10];
	int16_t spawnZ[10];
	int16_t rotation[10];
	int16_t targetMap[10];
	int16_t targetExit[10];
} MapWarps;

extern MapWarps MAP_WARPS;
extern uint8_t CURRENT_EXIT;
extern int8_t TAMER_START_TILE_X;
extern int8_t TAMER_START_TILE_Y;
extern int8_t TAMER_WAYPOINT_COUNT;
extern int8_t TAMER_WAYPOINT_X[];
extern int8_t TAMER_WAYPOINT_Y[];
extern int16_t FADE_IN_CURRENT;
extern int32_t CHECKED_MEMORY_CARD;
extern int32_t CURRENT_MENU;
extern int32_t TARGET_MENU;
extern int32_t IS_SCRIPT_PAUSED;
extern int16_t FADE_OUT_CURRENT;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
extern int32_t HAS_BUTTERFLY;
extern int32_t BUTTERFLY_ID;
extern int32_t CONDITION_BUBBLE_TYPE;
extern int32_t CONDITION_BUBBLE_ID;
extern int32_t CONDITION_BUBBLE_TIMER;
#endif

void GsSetNearClip(long clip);
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
void checkFishingMap(int32_t mapId, int32_t arg1);
int32_t isUIBoxAvailable(int32_t id);
void renderString(uint8_t color, int16_t x, int16_t y, int16_t w, int16_t h,
		  uint8_t u, uint8_t v, long layer, int32_t shadow);
#endif

uint16_t MAIN_func_800F19B0(int32_t param_1, int32_t param_2);

void renderMainMenuBackground(void);
void checkShopMap(int32_t mapId);
uint32_t playSound(int32_t vabId, int32_t val);
void tickMainMenu(void);
void renderMainMenu(void);
void handlePause(void);
int32_t tickScript(void);
void updateTournamentRegistration(void);
void initializeNamingBuffer(uint8_t flags);
void initializeTextbox(void);
#if !VERSION_IS(US)
void tickTextboxHandling(void);
#else
void tickTextboxHandling(int32_t flag);
#endif
int32_t newGameStateMachine(void);
void processInput(void);
void loadNewGameScene(void);
void writePStat(int32_t stat, int32_t value);

void initializeHeap(void);
void initializeFramebuffer(void);
void view_init(void);
void initializeEffectData(void);
void addThrownItem(int32_t type);
void startThrowingItem(void);
int32_t BTL_addItemParticles(Entity *e);
void tickThrownItem(int32_t instanceId);
void renderThrownItem(int32_t instanceId);
void unloadNewGameScene(void);

void initializeInventoryUI(void);
void initializeEntityText(void);
void initializeInventoryModules(void);

void initializeEFE(void);
void initializeParticleFX(void);
void initializeEntityParticleFX(void);
void initializeCloudFXData(void);
void initializeHealingParticles(void);

void BTL_buffStats(Entity *entity, int32_t entityId, int32_t amount,
		   int16_t* valuePtr, uint8_t color, uint8_t icon);
void BTL_addBuffDiskEffect(Entity *entity);

void SsInit(void);
void PadInit(int32_t mode);
long MemCardInit(long val);
long MemCardStart(void);
void initializeGsTMDMap(void);
void initializeScripts(void);
void initializeStatusObjects(void);
void initializeMusic(void);
void finalizeMusic(void);
void playMovie(int32_t movieId, int32_t shouldPlay);
void loadStackedTIMFile(char *path);
void runLandingScreen(void);
void runMainMenu(void);
void newGameScene(void);
void initializeLoadedMap(void);
void recalculatePPandArena(void);
void gameLoop(void);
void cleanupGame(void);
void initializePartner(int32_t type, int32_t posX, int32_t posY,
		       int32_t posZ, int32_t rotX, int32_t rotY,
		       int32_t rotZ);
void setDigimonRaised(uint16_t type);
void initializeMap(void);
void runMapHeadScript(int32_t scriptId);
uint8_t readPStat(int32_t id);
void initializeUIBoxData(void);
void initializeMedalModel(void);
void initializeChest(void);
void initializeFileReadQueue(void);
void fillEFEXTable(void);
void initializeFadeData(void);
int32_t loadTIMFile(char *path, void *buffer);

int32_t main(void);
void applyDrawOffset(int32_t offset);
void pollInputGame(void);
void pollInputMenu(void);
void renderPressStartToContinue(void);
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
int32_t tickTrialEnd(void);
void renderTrialEndBox(void);
#endif
int32_t entityIsOffScreen(Entity *entity, int32_t width, int32_t height);

/* Order anchor (reversed): pins symtab/section order to address order.
 * Unreferenced; discarded by --gc-sections. */
void *main_order_anchor[] = {
	handleBuffDisks,
	renderThrownItem,
	tickThrownItem,
	startThrowingItem,
	addThrownItem,
	initializeInventoryModules,
	initializeEffectData,
	view_init,
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	renderTrialEndBox,
#endif
	renderMainMenuBackground,
	renderPressStartToContinue,
	pollInputMenu,
	pollInputGame,
	applyDrawOffset,
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	tickTrialEnd,
#endif
	gameLoop,
#if VERSION_EQUAL_OR_NEWER(US)
	recalculatePPandArena,
#endif
	initializeLoadedMap,
	newGameScene,
	runMainMenu,
	runLandingScreen,
	initializeFramebuffer,
	initializeHeap,
	main,
};

// clang-format off
#if VERSION_IS(US)
DVECTOR DRAW_OFFSET_0 = { 0x00a0, 0x0078 };

DVECTOR DRAW_OFFSET_1 = { 0x00a0, 0x0168 };
#endif

GsOT_TAG *FRAMEBUFFER0_ORIGIN = (GsOT_TAG *)(TEXTURE_BUFFER + 0x28);

GsOT_TAG *FRAMEBUFFER1_ORIGIN = (GsOT_TAG *)(TEXTURE_BUFFER + 0x2028);

GsOT *FRAMEBUFFER_OT[2] = {
	(GsOT *)TEXTURE_BUFFER,
	(GsOT *)(TEXTURE_BUFFER + 0x14),
};

uint8_t *GENERAL_BUFFER_PTR = GENERAL_BUFFER;

#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
int16_t SECRET_CODE_CANDIDATE = -1;

int16_t SECRET_CODE = -1;
#endif

char MAIN_D_8012CE64[20] = "\\ETCDAT\\FI_INFO.TIM";

char MAIN_D_8012CE78[20] = "\\ETCNA\\TITLE256.TIM";

char MAIN_D_8012CE8C[] = "\\ETCDAT\\ETCTIM.BIN";

char MAIN_D_8012CEA0[] = "\\ETCNA\\TITLE2.TIM";

TRIAL_END_TEXT

DigimonPara DIGIMON_DATA[180] = {
	{
		DIGIMON_PARA_NAME(TAMER)
		0x00000011,
		0x0064,
		0x012c,
		0x00,
		0x00,
		{ 0x00, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(BOTAMON)
		0x00000003,
		0x005f,
		0x0078,
		0x01,
		0x01,
		{ 0x00, 0xff, 0xff },
		0x00,
		0x00,
		{
			0x71, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(KOROMON)
		0x00000007,
		0x0064,
		0x0096,
		0x01,
		0x02,
		{ 0x00, 0x01, 0xff },
		0x00,
		0x00,
		{
			0x75, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(AGUMON)
		0x00000016,
		0x0078,
		0x00fa,
		0x02,
		0x03,
		{ 0x00, 0xff, 0x01 },
		0x00,
		0x32,
		{
			0x02, 0x00, 0x04, 0x05, 0x03, 0x2b, 0x29, 0x3b,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(BETAMON)
		0x00000018,
		0x00a0,
		0x00a0,
		0x03,
		0x03,
		{ 0x04, 0xff, 0x02 },
		0x04,
		0x1e,
		{
			0x0c, 0x0a, 0x10, 0x13, 0x14, 0x12, 0x15, 0x42,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GREYMON)
		0x0000001a,
		0x00e6,
		0x0190,
		0x02,
		0x04,
		{ 0x00, 0x01, 0x02 },
		0x27,
		0x1e,
		{
			0x02, 0x00, 0x04, 0x01, 0x05, 0x03, 0x07, 0x2c,
			0x2d, 0x29, 0x0b, 0x09, 0x3f, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(DEVIMON)
		0x0000001c,
		0x00a0,
		0x0190,
		0x03,
		0x04,
		{ 0x01, 0x02, 0x04 },
		0x05,
		0x1e,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2a, 0x29, 0x0b, 0x0a,
			0x09, 0x08, 0x10, 0x11, 0x3e, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(AIRDRAMON)
		0x00000014,
		0x00b4,
		0x0190,
		0x02,
		0x04,
		{ 0x02, 0x00, 0xff },
		0x00,
		0x00,
		{
			0x02, 0x01, 0x05, 0x0c, 0x0b, 0x0a, 0x0d, 0x09,
			0x0e, 0x0f, 0x45, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TYRANNOMON)
		0x0000001b,
		0x00e6,
		0x0190,
		0x01,
		0x04,
		{ 0x01, 0xff, 0x00 },
		0x00,
		0x00,
		{
			0x02, 0x01, 0x03, 0x2b, 0x2c, 0x2e, 0x2f, 0x28,
			0x2a, 0x29, 0x3a, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MERAMON)
		0x00000016,
		0x0096,
		0x0190,
		0x01,
		0x04,
		{ 0x00, 0xff, 0x01 },
		0x0f,
		0x14,
		{
			0x02, 0x00, 0x04, 0x01, 0x05, 0x03, 0x06, 0x2c,
			0x2d, 0x2a, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SEADRAMON)
		0x00000011,
		0x00c8,
		0x0154,
		0x01,
		0x04,
		{ 0x04, 0x03, 0x00 },
		0x00,
		0x00,
		{
			0x02, 0x04, 0x26, 0x24, 0x20, 0x17, 0x10, 0x13,
			0x14, 0x12, 0x15, 0x58, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NUMEMON)
		0x00000016,
		0x00b4,
		0x010e,
		0x03,
		0x04,
		{ 0x06, 0xff, 0xff },
		0x00,
		0x00,
		{
			0x39, 0x31, 0x32, 0x33, 0x35, 0x36, 0x34, 0x38,
			0x6c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(METALGREYMON)
		0x0000001e,
		0x0118,
		0x01ae,
		0x03,
		0x05,
		{ 0x05, 0x00, 0x01 },
		0x0b,
		0x14,
		{
			0x05, 0x06, 0x07, 0x2e, 0x2f, 0x28, 0x18, 0x1c,
			0x1b, 0x1f, 0x1a, 0x19, 0x1d, 0x1e, 0x4f, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MAMEMON)
		0x00000013,
		0x0064,
		0x00c8,
		0x01,
		0x05,
		{ 0x01, 0x05, 0xff },
		0x00,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x18, 0x1b, 0x1f, 0x1a, 0x1e, 0x44, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MONZAEMON)
		0x0000000e,
		0x0082,
		0x014a,
		0x02,
		0x05,
		{ 0x01, 0x02, 0xff },
		0x22,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x0c, 0x0b, 0x0a, 0x08, 0x0e, 0x3c, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PUNIMON)
		0x00000003,
		0x005f,
		0x0078,
		0x01,
		0x01,
		{ 0x02, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x72, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TSUNOMON)
		0x00000003,
		0x0064,
		0x00dc,
		0x01,
		0x02,
		{ 0x02, 0x04, 0xff },
		0x00,
		0x0a,
		{
			0x76, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GABUMON)
		0x00000016,
		0x0082,
		0x0104,
		0x01,
		0x03,
		{ 0x01, 0xff, 0x00 },
		0x00,
		0x0a,
		{
			0x00, 0x05, 0x2b, 0x2c, 0x2e, 0x28, 0x2a, 0x63,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ELECMON)
		0x00000019,
		0x00be,
		0x00e6,
		0x01,
		0x03,
		{ 0x02, 0xff, 0x01 },
		0x00,
		0x0a,
		{
			0x2c, 0x2d, 0x29, 0x0c, 0x0b, 0x0a, 0x0d, 0x65,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(KABUTERIMON)
		0x0000001c,
		0x00f0,
		0x01ae,
		0x02,
		0x04,
		{ 0x03, 0x00, 0x02 },
		0x00,
		0x0a,
		{
			0x02, 0x01, 0x03, 0x0d, 0x0f, 0x25, 0x26, 0x27,
			0x24, 0x20, 0x22, 0x5d, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ANGEMON)
		0x0000001a,
		0x0096,
		0x0190,
		0x02,
		0x04,
		{ 0x02, 0xff, 0x01 },
		0x00,
		0x0a,
		{
			0x2c, 0x2d, 0x2a, 0x0c, 0x0b, 0x0a, 0x0d, 0x09,
			0x0e, 0x0f, 0x52, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(BIRDRAMON)
		0x00000018,
		0x00e6,
		0x01c2,
		0x02,
		0x04,
		{ 0x00, 0x02, 0xff },
		0x00,
		0x0a,
		{
			0x02, 0x00, 0x04, 0x01, 0x05, 0x03, 0x07, 0x0d,
			0x09, 0x0f, 0x5e, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GARURUMON)
		0x0000001a,
		0x00dc,
		0x0122,
		0x02,
		0x04,
		{ 0x04, 0x00, 0x01 },
		0x00,
		0x0a,
		{
			0x02, 0x00, 0x04, 0x2e, 0x2f, 0x2a, 0x10, 0x13,
			0x11, 0x12, 0x15, 0x40, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(FRIGIMON)
		0x00000013,
		0x00a0,
		0x017c,
		0x02,
		0x04,
		{ 0x04, 0xff, 0x01 },
		0x08,
		0x1e,
		{
			0x2b, 0x29, 0x17, 0x10, 0x13, 0x14, 0x11, 0x12,
			0x16, 0x15, 0x69, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(WHAMON)
		0x0000000c,
		0x00c8,
		0x0190,
		0x02,
		0x04,
		{ 0x04, 0xff, 0x03 },
		0x00,
		0x0a,
		{
			0x24, 0x20, 0x17, 0x10, 0x13, 0x14, 0x11, 0x12,
			0x16, 0x15, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(VEGIEMON)
		0x00000014,
		0x0096,
		0x010e,
		0x03,
		0x04,
		{ 0x03, 0xff, 0x04 },
		0x00,
		0x0a,
		{
			0x25, 0x26, 0x27, 0x24, 0x20, 0x22, 0x14, 0x15,
			0x48, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SKULLGREYMON)
		0x0000001a,
		0x0104,
		0x01e0,
		0x03,
		0x05,
		{ 0x01, 0x04, 0x05 },
		0x00,
		0x0a,
		{
			0x2b, 0x30, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x10, 0x11, 0x12, 0x1b, 0x19, 0x50, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(METALMAMEMON)
		0x00000013,
		0x0064,
		0x00c8,
		0x01,
		0x05,
		{ 0x01, 0x05, 0xff },
		0x00,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x1c, 0x1b, 0x1f, 0x1a, 0x19, 0x4d, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(VADEMON)
		0x00000010,
		0x006e,
		0x0190,
		0x03,
		0x05,
		{ 0x05, 0xff, 0x03 },
		0x00,
		0x0a,
		{
			0x27, 0x21, 0x24, 0x18, 0x1c, 0x1b, 0x1f, 0x1a,
			0x19, 0x1d, 0x1e, 0x43, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(POYOMON)
		0x00000003,
		0x005f,
		0x012c,
		0x01,
		0x01,
		{ 0x02, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x73, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TOKOMON)
		0x0000000b,
		0x0064,
		0x00a0,
		0x01,
		0x02,
		{ 0x02, 0x01, 0xff },
		0x00,
		0x0a,
		{
			0x77, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PATAMON)
		0x00000017,
		0x0082,
		0x00b4,
		0x01,
		0x03,
		{ 0x01, 0xff, 0x02 },
		0x00,
		0x0a,
		{
			0x2b, 0x2c, 0x2f, 0x2a, 0x0d, 0x09, 0x0e, 0x47,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(KUNEMON)
		0x0000001c,
		0x0078,
		0x00d2,
		0x03,
		0x03,
		{ 0x03, 0xff, 0x02 },
		0x00,
		0x0a,
		{
			0x0c, 0x0b, 0x0a, 0x25, 0x26, 0x20, 0x22, 0x4c,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(UNIMON)
		0x0000001b,
		0x00b4,
		0x0190,
		0x02,
		0x04,
		{ 0x02, 0x01, 0xff },
		0x00,
		0x0a,
		{
			0x30, 0x2d, 0x2a, 0x0c, 0x0b, 0x0a, 0x0d, 0x09,
			0x0e, 0x0f, 0x54, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(OGREMON)
		0x00000012,
		0x00b4,
		0x0186,
		0x03,
		0x04,
		{ 0x01, 0x00, 0x03 },
		0x00,
		0x0a,
		{
			0x02, 0x04, 0x03, 0x2b, 0x2c, 0x2e, 0x2f, 0x28,
			0x2a, 0x25, 0x23, 0x51, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SHELLMON)
		0x0000000f,
		0x012c,
		0x0190,
		0x01,
		0x04,
		{ 0x04, 0xff, 0x02 },
		0x00,
		0x0a,
		{
			0x24, 0x20, 0x17, 0x10, 0x13, 0x14, 0x11, 0x12,
			0x16, 0x15, 0x57, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(CENTARUMON)
		0x0000001b,
		0x00a5,
		0x01ae,
		0x01,
		0x04,
		{ 0x00, 0x01, 0xff },
		0x00,
		0x0a,
		{
			0x02, 0x00, 0x04, 0x01, 0x05, 0x03, 0x07, 0x30,
			0x2d, 0x29, 0x56, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(BAKEMON)
		0x0000000a,
		0x0096,
		0x0172,
		0x03,
		0x04,
		{ 0x02, 0xff, 0x04 },
		0x00,
		0x0a,
		{
			0x0c, 0x0b, 0x0a, 0x0d, 0x09, 0x08, 0x0f, 0x10,
			0x12, 0x15, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(DRIMOGEMON)
		0x0000001c,
		0x00c8,
		0x0168,
		0x01,
		0x04,
		{ 0x01, 0xff, 0x03 },
		0x00,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x27, 0x24, 0x4b, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SUKAMON)
		0x00000015,
		0x00af,
		0x0122,
		0x03,
		0x04,
		{ 0x06, 0xff, 0xff },
		0x23,
		0x0a,
		{
			0x37, 0x31, 0x32, 0x33, 0x35, 0x36, 0x34, 0x38,
			0x41, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ANDROMON)
		0x00000015,
		0x0096,
		0x0190,
		0x02,
		0x05,
		{ 0x05, 0x01, 0x02 },
		0x15,
		0x1e,
		{
			0x2e, 0x2d, 0x2f, 0x28, 0x0c, 0x0b, 0x18, 0x1c,
			0x1b, 0x1f, 0x1a, 0x19, 0x1d, 0x1e, 0x66, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GIROMON)
		0x0000000b,
		0x0082,
		0x012c,
		0x02,
		0x05,
		{ 0x05, 0x02, 0x01 },
		0x13,
		0x1e,
		{
			0x2e, 0x2f, 0x0b, 0x0a, 0x08, 0x18, 0x1c, 0x1b,
			0x1f, 0x1a, 0x19, 0x1d, 0x1e, 0x4a, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ETEMON)
		0x0000001b,
		0x0082,
		0x0109,
		0x03,
		0x05,
		{ 0x01, 0x02, 0x06 },
		0x12,
		0x1e,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x0b, 0x09, 0x08, 0x37, 0x38, 0x5c, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(YURAMON)
		0x00000009,
		0x005a,
		0x00e6,
		0x01,
		0x01,
		{ 0x03, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x74, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TANEMON)
		0x0000000a,
		0x0064,
		0x00e6,
		0x01,
		0x02,
		{ 0x03, 0x04, 0xff },
		0x00,
		0x0a,
		{
			0x78, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(BIYOMON)
		0x00000016,
		0x0064,
		0x00eb,
		0x02,
		0x03,
		{ 0x02, 0xff, 0x00 },
		0x00,
		0x0a,
		{
			0x02, 0x05, 0x0a, 0x0d, 0x09, 0x0e, 0x0f, 0x46,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PALMON)
		0x0000001f,
		0x0064,
		0x0104,
		0x02,
		0x03,
		{ 0x03, 0xff, 0x04 },
		0x00,
		0x0a,
		{
			0x25, 0x24, 0x20, 0x22, 0x17, 0x14, 0x15, 0x62,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MONOCHROMON)
		0x00000018,
		0x0104,
		0x014a,
		0x01,
		0x04,
		{ 0x00, 0x01, 0x03 },
		0x00,
		0x0a,
		{
			0x02, 0x01, 0x05, 0x03, 0x07, 0x2e, 0x2d, 0x28,
			0x27, 0x23, 0x22, 0x68, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(LEOMON)
		0x00000017,
		0x00a0,
		0x0190,
		0x02,
		0x04,
		{ 0x01, 0xff, 0x02 },
		0x00,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x0c, 0x0b, 0x5b, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(COELAMON)
		0x00000014,
		0x0096,
		0x019a,
		0x01,
		0x04,
		{ 0x04, 0xff, 0x03 },
		0x00,
		0x0a,
		{
			0x17, 0x10, 0x13, 0x14, 0x11, 0x12, 0x15, 0x25,
			0x26, 0x23, 0x67, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(KOKATORIMON)
		0x00000015,
		0x00be,
		0x0140,
		0x02,
		0x04,
		{ 0x02, 0xff, 0x01 },
		0x00,
		0x0a,
		{
			0x2c, 0x28, 0x2a, 0x0c, 0x0b, 0x0a, 0x0d, 0x09,
			0x0e, 0x0f, 0x61, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(KUWAGAMON)
		0x0000001e,
		0x00f0,
		0x019f,
		0x03,
		0x04,
		{ 0x03, 0x01, 0x02 },
		0x00,
		0x0a,
		{
			0x2b, 0x29, 0x0d, 0x09, 0x25, 0x26, 0x27, 0x24,
			0x20, 0x22, 0x64, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MOJYAMON)
		0x00000012,
		0x00aa,
		0x012c,
		0x02,
		0x04,
		{ 0x04, 0x01, 0x03 },
		0x00,
		0x0a,
		{
			0x2c, 0x2e, 0x10, 0x13, 0x14, 0x12, 0x16, 0x15,
			0x27, 0x22, 0x55, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NANIMON)
		0x00000014,
		0x0078,
		0x00f0,
		0x03,
		0x04,
		{ 0x06, 0x01, 0xff },
		0x00,
		0x0a,
		{
			0x2c, 0x2e, 0x2d, 0x37, 0x31, 0x32, 0x33, 0x35,
			0x36, 0x34, 0x6b, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MEGADRAMON)
		0x0000001d,
		0x00dc,
		0x01f4,
		0x03,
		0x05,
		{ 0x05, 0x04, 0x01 },
		0x00,
		0x0a,
		{
			0x2c, 0x2e, 0x10, 0x12, 0x11, 0x18, 0x1c, 0x1b,
			0x1f, 0x1a, 0x19, 0x1d, 0x1e, 0x4e, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PIXIMON)
		0x0000001a,
		0x006e,
		0x014a,
		0x01,
		0x05,
		{ 0x03, 0x02, 0xff },
		0x00,
		0x0a,
		{
			0x25, 0x26, 0x27, 0x23, 0x21, 0x24, 0x20, 0x22,
			0x0d, 0x09, 0x0e, 0x0f, 0x49, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(DIGITAMAMON)
		0x0000000c,
		0x0064,
		0x00fa,
		0x01,
		0x05,
		{ 0x00, 0x02, 0x04 },
		0x00,
		0x0a,
		{
			0x02, 0x00, 0x04, 0x01, 0x05, 0x03, 0x06, 0x07,
			0x0b, 0x09, 0x08, 0x0e, 0x0f, 0x17, 0x15, 0x60,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PENGUINMON)
		0x00000019,
		0x0078,
		0x00fa,
		0x01,
		0x03,
		{ 0x04, 0xff, 0x03 },
		0x00,
		0x0a,
		{
			0x25, 0x24, 0x10, 0x13, 0x14, 0x12, 0x15, 0x5f,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NINJAMON)
		0x00000013,
		0x0064,
		0x00c8,
		0x01,
		0x04,
		{ 0x03, 0x01, 0x00 },
		0x00,
		0x0a,
		{
			0x00, 0x04, 0x2b, 0x2c, 0x2d, 0x2a, 0x26, 0x24,
			0x20, 0x22, 0x59, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PHOENIXMON)
		0x0000001f,
		0x00fa,
		0x01c2,
		0x02,
		0x05,
		{ 0x02, 0x00, 0xff },
		0x00,
		0x0a,
		{
			0x04, 0x01, 0x03, 0x06, 0x07, 0x0c, 0x0b, 0x0a,
			0x0d, 0x09, 0x08, 0x0e, 0x0f, 0x6d, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(HERCULESKABUTERIMON)
		0x0000001a,
		0x00fa,
		0x01b8,
		0x01,
		0x05,
		{ 0x03, 0x01, 0x00 },
		0x00,
		0x0a,
		{
			0x01, 0x03, 0x2e, 0x2d, 0x2f, 0x29, 0x25, 0x26,
			0x27, 0x23, 0x21, 0x24, 0x20, 0x22, 0x5d, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MEGASEADRAMON)
		0x0000000f,
		0x00c8,
		0x0154,
		0x01,
		0x05,
		{ 0x04, 0x02, 0xff },
		0x00,
		0x0a,
		{
			0x17, 0x10, 0x13, 0x14, 0x11, 0x12, 0x16, 0x15,
			0x0d, 0x0e, 0x0f, 0x6f, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(WEREGARURUMON)
		0x00000000,
		0x0000,
		0x0000,
		0x02,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PANJYAMON)
		0x00000017,
		0x00a0,
		0x0190,
		0x01,
		0x04,
		{ 0x01, 0x02, 0xff },
		0x00,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x0c, 0x0b, 0x5b, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GIGADRAMON)
		0x0000001d,
		0x00dc,
		0x01f4,
		0x01,
		0x05,
		{ 0x05, 0x04, 0x01 },
		0x00,
		0x0a,
		{
			0x2c, 0x2e, 0x10, 0x12, 0x11, 0x18, 0x1c, 0x1b,
			0x1f, 0x1a, 0x19, 0x1d, 0x1e, 0x4e, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(METALETEMON)
		0x0000001b,
		0x0082,
		0x0109,
		0x02,
		0x05,
		{ 0x01, 0x02, 0x06 },
		0x00,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x2d, 0x2f, 0x28, 0x2a, 0x29,
			0x0b, 0x09, 0x08, 0x37, 0x38, 0x5c, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MYOTISMON)
		0x00000019,
		0x00a0,
		0x01ae,
		0x03,
		0x05,
		{ 0x02, 0x01, 0xff },
		0x00,
		0x00,
		{
			0x2e, 0x08, 0x0f, 0x10, 0x16, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(YANMAMON)
		0x0000001e,
		0x00a0,
		0x0168,
		0x03,
		0x03,
		{ 0x02, 0xff, 0xff },
		0x04,
		0x0a,
		{
			0x0a, 0x0d, 0x09, 0x26, 0x23, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GOTSUMON)
		0x00000016,
		0x006e,
		0x0118,
		0x01,
		0x03,
		{ 0x01, 0xff, 0xff },
		0x01,
		0x0a,
		{
			0x2e, 0x2f, 0x28, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(FLAREIZAMON)
		0x00000016,
		0x0082,
		0x0118,
		0x01,
		0x03,
		{ 0x00, 0xff, 0xff },
		0x26,
		0x0a,
		{
			0x02, 0x00, 0x04, 0x05, 0x03, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(WARUMONZAEMON)
		0x0000000c,
		0x0096,
		0x019a,
		0x03,
		0x05,
		{ 0x01, 0x02, 0xff },
		0x1b,
		0x32,
		{
			0x2e, 0x2f, 0x29, 0x08, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SNOWAGUMON)
		0x00000016,
		0x008c,
		0x00fa,
		0x02,
		0x03,
		{ 0x04, 0xff, 0x01 },
		0x05,
		0x0a,
		{
			0x13, 0x11, 0x12, 0x2b, 0x29, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(HYOGAMON)
		0x00000012,
		0x00b4,
		0x0186,
		0x03,
		0x04,
		{ 0x04, 0x01, 0xff },
		0x28,
		0x0a,
		{
			0x10, 0x13, 0x2b, 0x2e, 0x2f, 0x2a, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PLATINUMSUKAMON)
		0x00000015,
		0x00af,
		0x0122,
		0x02,
		0x04,
		{ 0x06, 0xff, 0xff },
		0x0e,
		0x0a,
		{
			0x31, 0x33, 0x36, 0x34, 0x38, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(DOKUNEMON)
		0x0000001c,
		0x0078,
		0x00d2,
		0x03,
		0x03,
		{ 0x03, 0x02, 0xff },
		0x08,
		0x19,
		{
			0x25, 0x26, 0x20, 0x22, 0x4c, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SHIMAUNIMON)
		0x0000001b,
		0x00b4,
		0x0190,
		0x01,
		0x04,
		{ 0x02, 0x01, 0xff },
		0x01,
		0x0a,
		{
			0x30, 0x2d, 0x2a, 0x0c, 0x0b, 0x0a, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TANKMON)
		0x00000011,
		0x00c8,
		0x01c2,
		0x01,
		0x04,
		{ 0x00, 0x01, 0xff },
		0x02,
		0x14,
		{
			0x02, 0x03, 0x2e, 0x2f, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(REDVEGIMON)
		0x00000014,
		0x0096,
		0x010e,
		0x03,
		0x04,
		{ 0x03, 0x00, 0xff },
		0x29,
		0x0a,
		{
			0x25, 0x26, 0x27, 0x24, 0x20, 0x22, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(JMOJYAMON)
		0x00000012,
		0x00aa,
		0x012c,
		0x02,
		0x04,
		{ 0x01, 0x03, 0xff },
		0x27,
		0x0a,
		{
			0x2c, 0x2e, 0x14, 0x27, 0x22, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NISEDRIMOGEMON)
		0x0000001c,
		0x00c8,
		0x0168,
		0x02,
		0x04,
		{ 0x01, 0xff, 0xff },
		0x16,
		0x0a,
		{
			0x2b, 0x2c, 0x2d, 0x2f, 0x28, 0x29, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GOBURIMON)
		0x00000017,
		0x007d,
		0x0118,
		0x03,
		0x03,
		{ 0x01, 0xff, 0xff },
		0x26,
		0x1e,
		{
			0x02, 0x04, 0x2b, 0x2e, 0x2a, 0x10, 0x12, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MUDFRIGIMON)
		0x00000013,
		0x00a0,
		0x017c,
		0x02,
		0x04,
		{ 0x01, 0xff, 0xff },
		0x01,
		0x0a,
		{
			0x2b, 0x29, 0x17, 0x14, 0x15, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PSYCHEMON)
		0x00000016,
		0x0082,
		0x0104,
		0x02,
		0x03,
		{ 0x01, 0xff, 0xff },
		0x0d,
		0x0a,
		{
			0x2b, 0x2c, 0x2e, 0x28, 0x2a, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MODOKIBETAMON)
		0x00000018,
		0x00a0,
		0x00a0,
		0x02,
		0x03,
		{ 0x04, 0xff, 0x02 },
		0x00,
		0x28,
		{
			0x0c, 0x0a, 0x14, 0x15, 0x42, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TOYAGUMON)
		0x00000016,
		0x008c,
		0x00fa,
		0x01,
		0x03,
		{ 0x00, 0xff, 0x01 },
		0x20,
		0x0a,
		{
			0x02, 0x00, 0x04, 0x05, 0x03, 0x2b, 0x29, 0x3b,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(PIDDOMON)
		0x0000001a,
		0x0096,
		0x0190,
		0x02,
		0x04,
		{ 0x02, 0xff, 0xff },
		0x3b,
		0x0a,
		{
			0x2c, 0x2d, 0x0b, 0x09, 0x0e, 0x0f, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ARURAUMON)
		0x0000001f,
		0x0064,
		0x0104,
		0x03,
		0x03,
		{ 0x03, 0x04, 0xff },
		0x07,
		0x05,
		{
			0x25, 0x24, 0x20, 0x17, 0x14, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GEREMON)
		0x00000016,
		0x00b4,
		0x010e,
		0x03,
		0x04,
		{ 0x06, 0xff, 0xff },
		0x20,
		0x0a,
		{
			0x39, 0x31, 0x32, 0x35, 0x36, 0x6c, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(VERMILIMON)
		0x00000018,
		0x0104,
		0x014a,
		0x02,
		0x04,
		{ 0x00, 0x01, 0xff },
		0x38,
		0x01,
		{
			0x01, 0x05, 0x03, 0x07, 0x2e, 0x2d, 0x28, 0x68,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(FUGAMON)
		0x00000012,
		0x00b4,
		0x0186,
		0x03,
		0x04,
		{ 0x02, 0x01, 0x00 },
		0x11,
		0x0a,
		{
			0x02, 0x04, 0x2e, 0x0c, 0x0d, 0x0f, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TEKKAMON)
		0x0000000b,
		0x0082,
		0x012c,
		0x03,
		0x05,
		{ 0x05, 0xff, 0x01 },
		0x14,
		0x64,
		{
			0x2e, 0x2f, 0x1f, 0x1e, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MORISHELLMON)
		0x0000000f,
		0x012c,
		0x0190,
		0x01,
		0x04,
		{ 0x03, 0x04, 0xff },
		0x3c,
		0x01,
		{
			0x24, 0x20, 0x17, 0x14, 0x11, 0x15, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GUARDROMON)
		0x00000013,
		0x0096,
		0x0140,
		0x02,
		0x04,
		{ 0x05, 0x01, 0x00 },
		0x0f,
		0x0a,
		{
			0x01, 0x2e, 0x18, 0x19, 0x1e, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MUCHOMON)
		0x00000019,
		0x0078,
		0x00fa,
		0x01,
		0x03,
		{ 0x03, 0x02, 0xff },
		0x08,
		0x0a,
		{
			0x25, 0x10, 0x13, 0x5f, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ICEMON)
		0x00000016,
		0x0069,
		0x0136,
		0x02,
		0x03,
		{ 0x04, 0xff, 0x01 },
		0x05,
		0x0a,
		{
			0x2e, 0x10, 0x13, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(AKATORIMON)
		0x00000015,
		0x00be,
		0x0140,
		0x01,
		0x04,
		{ 0x02, 0x04, 0xff },
		0x0f,
		0x0a,
		{
			0x2c, 0x0c, 0x0a, 0x0d, 0x09, 0x0e, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TSUKAIMON)
		0x00000017,
		0x0082,
		0x00b4,
		0x03,
		0x03,
		{ 0x01, 0x02, 0xff },
		0x26,
		0x1e,
		{
			0x2b, 0x2c, 0x2f, 0x2a, 0x0d, 0x09, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SHARMAMON)
		0x00000017,
		0x007d,
		0x0118,
		0x03,
		0x03,
		{ 0x00, 0x01, 0xff },
		0x26,
		0x0a,
		{
			0x02, 0x04, 0x2b, 0x2e, 0x2a, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(CLEARAGUMON)
		0x00000016,
		0x008c,
		0x00fa,
		0x01,
		0x03,
		{ 0x03, 0x01, 0xff },
		0x09,
		0x05,
		{
			0x02, 0x00, 0x04, 0x05, 0x03, 0x2b, 0x29, 0x3b,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(WEEDMON)
		0x00000014,
		0x0096,
		0x010e,
		0x03,
		0x04,
		{ 0x03, 0xff, 0xff },
		0x2a,
		0x0a,
		{
			0x25, 0x26, 0x27, 0x24, 0x20, 0x22, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ICEDEVIMON)
		0x0000001c,
		0x00a0,
		0x0190,
		0x03,
		0x04,
		{ 0x04, 0x01, 0xff },
		0x3a,
		0x01,
		{
			0x2b, 0x2d, 0x2a, 0x10, 0x11, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(DARKRIZAMON)
		0x00000016,
		0x0082,
		0x0118,
		0x03,
		0x03,
		{ 0x00, 0x02, 0xff },
		0x05,
		0x0a,
		{
			0x04, 0x05, 0x03, 0x10, 0x13, 0x12, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SANDYANMAMON)
		0x0000001e,
		0x00a0,
		0x0168,
		0x03,
		0x03,
		{ 0x02, 0x00, 0xff },
		0x0a,
		0x05,
		{
			0x02, 0x09, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SNOWGOBURIMON)
		0x00000017,
		0x007d,
		0x0118,
		0x03,
		0x03,
		{ 0x04, 0x01, 0xff },
		0x26,
		0x0a,
		{
			0x2b, 0x2e, 0x2a, 0x10, 0x12, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(BLUEMERAMON)
		0x00000016,
		0x0096,
		0x0190,
		0x02,
		0x04,
		{ 0x04, 0x04, 0xff },
		0x3d,
		0x01,
		{
			0x00, 0x01, 0x03, 0x2c, 0x2d, 0x10, 0x13, 0x11,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GURURUMON)
		0x0000001a,
		0x00dc,
		0x0122,
		0x03,
		0x04,
		{ 0x01, 0x04, 0xff },
#if !VERSION_IS(JP)
		0x01,
#else
		0x00,
#endif
		0x0a,
		{
			0x2e, 0x2f, 0x2a, 0x10, 0x11, 0x12, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SABERDRAMON)
		0x00000018,
		0x00e6,
		0x01c2,
		0x03,
		0x04,
		{ 0x04, 0x02, 0xff },
#if !VERSION_IS(JP)
		0x08,
#else
		0x00,
#endif
		0x0a,
		{
			0x02, 0x01, 0x03, 0x0d, 0x09, 0x0f, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SOULMON)
		0x0000000a,
		0x0096,
		0x0172,
		0x03,
		0x04,
		{ 0x02, 0x04, 0xff },
#if !VERSION_IS(JP)
		0x05,
#else
		0x00,
#endif
		0x0a,
		{
			0x0c, 0x0b, 0x0a, 0x08, 0x10, 0x53, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ROCKMON)
		0x00000013,
		0x00a0,
		0x017c,
		0x03,
		0x04,
		{ 0x04, 0x01, 0xff },
		0x39,
		0x01,
		{
			0x2b, 0x29, 0x13, 0x11, 0x12, 0x16, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(OTAMAMON)
		0x0000000f,
		0x0091,
		0x00be,
		0x01,
		0x03,
		{ 0x04, 0xff, 0xff },
		0x04,
		0x00,
		{
			0x17, 0x14, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(GEKOMON)
		0x00000016,
		0x0069,
		0x0140,
		0x01,
		0x04,
		{ 0x04, 0xff, 0xff },
		0x01,
		0x0a,
		{
			0x17, 0x10, 0x14, 0x12, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TENTOMON)
		0x0000001b,
		0x0096,
		0x00dc,
		0x01,
		0x03,
		{ 0x03, 0xff, 0xff },
#if !VERSION_IS(JP)
		0x16,
		0x28,
#else
		0x04,
		0x0a,
#endif
		{
			0x25, 0x23, 0x21, 0x24, 0x22, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(WARUSEADRAMON)
		0x0000000f,
		0x00c8,
		0x0186,
		0x03,
		0x05,
		{ 0x04, 0x02, 0xff },
		0x00,
		0x00,
		{
			0x10, 0x16, 0x12, 0x0e, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(METEORMON)
		0x00000016,
		0x00a0,
		0x0190,
		0x01,
		0x03,
		{ 0x00, 0x01, 0xff },
		0x1b,
		0x64,
		{
			0x04, 0x07, 0x2e, 0x28, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(UNKNOWN)
		0x00000000,
		0x0000,
		0x0000,
		0x03,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MACHINEDRAMON)
		0x00000020,
		0x0190,
		0x0258,
		0x00,
		0x05,
		{ 0x05, 0x02, 0x01 },
		0x46,
		0x00,
		{
			0x2e, 0x0b, 0x08, 0x16, 0x19, 0x1e, 0x6a, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(ANALOGMAN)
		0x00000012,
		0x0064,
		0x015e,
		0x00,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x00,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(JIJIMON)
		0x00000011,
		0x0078,
		0x00fa,
		0x02,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MARKET_MANAGER)
		0x00000010,
		0x0078,
		0x00fa,
		0x01,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(SHOGUNGEKOMON)
		0x0000001c,
		0x0190,
		0x0320,
		0x01,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(KING_SUKAMON)
		0x00000015,
		0x01c2,
		0x0320,
		0x03,
		0x04,
		{ 0x06, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(CHERRYMON)
		0x0000001a,
		0x0190,
		0x03e8,
		0x02,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(HAGURUMON)
		0x00000005,
		0x0078,
		0x0140,
		0x02,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(TINMON)
		0x00000007,
		0x00aa,
		0x0190,
		0x02,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(MASTER_TYRANNOMON)
		0x0000001b,
		0x017c,
		0x024e,
		0x01,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_GOBIROMON)
		0x00000017,
		0x007d,
		0x0118,
		0x03,
		0x03,
		{ 0x01, 0xff, 0xff },
		0x26,
		0x14,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(BRACHIOMON)
		0x00000015,
		0x0190,
		0x04b0,
		0x01,
		0x00,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(DEMIMERAMON)
		0x00000003,
		0x0064,
		0x00e6,
		0x01,
		0x01,
		{ 0xff, 0xff, 0xff },
		0x00,
		0x0a,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_BETAMON)
		0x00000018,
		0x00a0,
		0x00a0,
		0x03,
		0x03,
		{ 0x04, 0xff, 0x02 },
		0x00,
		0x00,
		{
			0x0c, 0x0a, 0x14, 0x15, 0x42, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_GREYMON)
		0x0000001a,
		0x00e6,
		0x0190,
		0x02,
		0x04,
		{ 0x00, 0x01, 0x02 },
		0x17,
		0x64,
		{
			0x04, 0x03, 0x2c, 0x3f, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_DEVIMON)
		0x0000001c,
		0x00a0,
		0x0190,
		0x03,
		0x04,
		{ 0x01, 0x02, 0x04 },
		0x1a,
		0x64,
		{
			0x2e, 0x2a, 0x29, 0x0b, 0x08, 0x10, 0x11, 0x3e,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_AIRDRAMON)
		0x00000014,
		0x00b4,
		0x0190,
		0x02,
		0x04,
		{ 0x02, 0x00, 0xff },
		0x19,
		0x00,
		{
			0x0b, 0x0e, 0x0f, 0x45, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_TYRANNOMON)
		0x0000001b,
		0x00e6,
		0x0190,
		0x01,
		0x04,
		{ 0x01, 0xff, 0x00 },
		0x1b,
		0x64,
		{
			0x2e, 0x2f, 0x28, 0x2a, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_MERAMON)
		0x00000016,
		0x0096,
		0x0190,
		0x01,
		0x04,
		{ 0x00, 0xff, 0x01 },
		0x17,
		0x64,
		{
			0x04, 0x05, 0x2c, 0x3d, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_SEADRAMON)
		0x00000011,
		0x00c8,
		0x0154,
		0x01,
		0x04,
		{ 0x04, 0x03, 0x00 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_NUMEMON)
		0x00000016,
		0x00b4,
		0x010e,
		0x03,
		0x04,
		{ 0x06, 0xff, 0xff },
		0x1c,
		0x64,
		{
			0x31, 0x33, 0x38, 0x6c, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_METALGREYMON)
		0x0000001e,
		0x0118,
		0x01ae,
		0x03,
		0x05,
		{ 0x05, 0x00, 0x01 },
		0x18,
		0x64,
		{
			0x06, 0x07, 0x2e, 0x2f, 0x1f, 0x1d, 0x1e, 0x4f,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_MAMEMON)
		0x00000013,
		0x0064,
		0x00c8,
		0x01,
		0x05,
		{ 0x01, 0x05, 0xff },
		0x1a,
		0x64,
		{
			0x2e, 0x2f, 0x1a, 0x1e, 0x44, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_MONZAEMON)
		0x0000000e,
		0x0082,
		0x014a,
		0x02,
		0x05,
		{ 0x01, 0x02, 0xff },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_GABUMON)
		0x00000016,
		0x0082,
		0x0104,
		0x01,
		0x03,
		{ 0x01, 0xff, 0x00 },
		0x1c,
		0x00,
		{
			0x2c, 0x2e, 0x28, 0x2a, 0x63, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_ELECMON)
		0x00000019,
		0x00be,
		0x00e6,
		0x01,
		0x03,
		{ 0x02, 0xff, 0x01 },
		0x00,
		0x00,
		{
			0x0c, 0x0b, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_KABUTERIMON)
		0x0000001c,
		0x00f0,
		0x01ae,
		0x02,
		0x04,
		{ 0x03, 0xff, 0x02 },
		0x00,
		0x00,
		{
			0x01, 0x26, 0x24, 0x22, 0x5d, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_ANGEMON)
		0x0000001a,
		0x0096,
		0x0190,
		0x02,
		0x04,
		{ 0x02, 0xff, 0x01 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_BIRDRAMON)
		0x00000018,
		0x00e6,
		0x01c2,
		0x02,
		0x04,
		{ 0x00, 0x02, 0xff },
		0x00,
		0x00,
		{
			0x01, 0x03, 0x0f, 0x5e, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_GARURUMON)
		0x0000001a,
		0x00dc,
		0x0122,
		0x02,
		0x04,
		{ 0x04, 0x02, 0x01 },
		0x1b,
		0x64,
		{
			0x2f, 0x2a, 0x10, 0x11, 0x40, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_FRIGIMON)
		0x00000013,
		0x00a0,
		0x017c,
		0x02,
		0x04,
		{ 0x04, 0xff, 0x01 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_WHAMON)
		0x0000000c,
		0x0190,
		0x0226,
		0x02,
		0x04,
		{ 0x04, 0xff, 0x03 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_VEGIMON)
		0x00000014,
		0x0096,
		0x010e,
		0x01,
		0x04,
		{ 0x03, 0xff, 0x04 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_SKULLGREYMON)
		0x0000001a,
		0x0104,
		0x01e0,
		0x03,
		0x05,
		{ 0x01, 0x04, 0x05 },
		0x17,
		0x64,
		{
#if !VERSION_IS(JP)
			0x30, 0x2e, 0x2f, 0x2a, 0x19, 0x50, 0xff, 0xff,
#else
			0x2c, 0x2e, 0x2f, 0x2a, 0x19, 0x50, 0xff, 0xff,
#endif
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_METALMAMEMON)
		0x00000013,
		0x0064,
		0x00c8,
		0x01,
		0x05,
		{ 0x01, 0x05, 0xff },
		0x18,
		0x64,
		{
			0x2f, 0x1b, 0x1a, 0x19, 0x4d, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_VADEMON)
		0x00000010,
		0x006e,
		0x0190,
		0x03,
		0x05,
		{ 0x05, 0xff, 0x03 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_PATAMON)
		0x00000017,
		0x0082,
		0x00b4,
		0x01,
		0x03,
		{ 0x01, 0xff, 0x02 },
		0x04,
		0x14,
		{
			0x2c, 0x2f, 0x0d, 0x09, 0x47, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_KUNEMON)
		0x0000001c,
		0x0078,
		0x00d2,
		0x03,
		0x03,
		{ 0x03, 0xff, 0x02 },
		0x1c,
		0x64,
		{
			0x0c, 0x0a, 0x25, 0x26, 0x4c, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_UNIMON)
		0x0000001b,
		0x00b4,
		0x0190,
		0x02,
		0x04,
		{ 0x02, 0x01, 0xff },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_OGREMON)
		0x00000012,
		0x00b4,
		0x0186,
		0x03,
		0x04,
		{ 0x01, 0x00, 0x03 },
		0x1b,
		0x64,
		{
			0x04, 0x2c, 0x28, 0x51, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_SHELLMON)
		0x0000000f,
		0x012c,
		0x0190,
		0x01,
		0x04,
		{ 0x04, 0xff, 0x02 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_CENTARUMON)
		0x0000001b,
		0x00a5,
		0x01ae,
		0x01,
		0x04,
		{ 0x00, 0x01, 0xff },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_BAKEMON)
		0x0000000a,
		0x0096,
		0x0172,
		0x03,
		0x04,
		{ 0x02, 0xff, 0x04 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_DRIMOGEMON)
		0x0000001c,
		0x00d7,
		0x0168,
		0x01,
		0x04,
		{ 0x01, 0xff, 0x03 },
		0x00,
		0x00,
		{
			0x2b, 0x2c, 0x2e, 0x28, 0x4b, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_SUKAMON)
		0x00000015,
		0x00af,
		0x0122,
		0x03,
		0x04,
		{ 0x06, 0xff, 0xff },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_ANDROMON)
		0x00000015,
		0x0096,
		0x0190,
		0x02,
		0x05,
		{ 0x05, 0x01, 0x02 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_GIROMON)
		0x0000000b,
		0x0082,
		0x012c,
		0x02,
		0x05,
		{ 0x05, 0x02, 0x01 },
		0x18,
		0x64,
		{
			0x18, 0x1c, 0x1f, 0x1d, 0x4a, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_ETEMON)
		0x0000001b,
		0x0082,
		0x0109,
		0x03,
		0x05,
		{ 0x01, 0x03, 0x06 },
		0x19,
		0x64,
		{
			0x2c, 0x2f, 0x2a, 0x09, 0x5c, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_BIYOMON)
		0x00000016,
		0x0064,
		0x00eb,
		0x02,
		0x03,
		{ 0x02, 0xff, 0x00 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_PALMON)
		0x0000001f,
		0x0064,
		0x0104,
		0x01,
		0x03,
		{ 0x03, 0xff, 0x04 },
		0x19,
		0x64,
		{
			0x25, 0x20, 0x14, 0x62, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_MONOCHROMON)
		0x00000018,
		0x0104,
		0x014a,
		0x01,
		0x04,
		{ 0x00, 0x01, 0x03 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_LEOMON)
		0x00000017,
		0x00a0,
		0x0190,
		0x02,
		0x04,
		{ 0x01, 0xff, 0x02 },
		0x00,
		0x00,
		{
			0x2b, 0x2c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_COELAMON)
		0x00000014,
		0x0096,
		0x019a,
		0x01,
		0x04,
		{ 0x04, 0xff, 0x03 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_KOKATORIMON)
		0x00000015,
		0x00be,
		0x0140,
		0x01,
		0x04,
		{ 0x02, 0xff, 0x01 },
		0x18,
		0x64,
		{
			0x2c, 0x2a, 0x0b, 0x0e, 0x61, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_KUWAGAMON)
		0x0000001e,
		0x00f0,
		0x019f,
		0x03,
		0x04,
		{ 0x03, 0x01, 0x02 },
		0x00,
		0x00,
		{
			0x2b, 0x29, 0x09, 0x25, 0x20, 0x64, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_MOJYAMON)
		0x00000012,
		0x00aa,
		0x012c,
		0x01,
		0x04,
		{ 0x04, 0x01, 0x03 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_NANIMON)
		0x00000014,
		0x0078,
		0x00f0,
		0x03,
		0x04,
		{ 0x06, 0x01, 0xff },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_MEGADRAMON)
		0x0000001d,
		0x00dc,
		0x01f4,
		0x03,
		0x05,
		{ 0x05, 0x04, 0x01 },
		0x17,
		0x64,
		{
			0x2e, 0x10, 0x1d, 0x4e, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_PIXIMON)
		0x0000001a,
		0x006e,
		0x014a,
		0x02,
		0x05,
		{ 0x03, 0x02, 0xff },
		0x1c,
		0x64,
		{
			0x26, 0x21, 0x0e, 0x49, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_DIGITAMAMON)
		0x0000000c,
		0x0064,
		0x00fa,
		0x01,
		0x05,
		{ 0x00, 0x02, 0x04 },
		0x1a,
		0x64,
		{
			0x07, 0x08, 0x0e, 0x60, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_NINJAMON)
		0x00000013,
		0x0064,
		0x00c8,
		0x01,
		0x04,
		{ 0x03, 0x01, 0x00 },
		0x1a,
		0x64,
		{
			0x04, 0x2c, 0x24, 0x59, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_PENGUINMON)
		0x00000019,
		0x0064,
		0x00fa,
		0x01,
		0x03,
		{ 0x04, 0xff, 0x03 },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC_MYOTISMON)
		0x00000019,
		0x00fa,
		0x0258,
		0x03,
		0x05,
		{ 0x02, 0x01, 0xff },
		0x00,
		0x00,
		{
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC2_GREYMON)
		0x0000001a,
		0x00e6,
		0x0190,
		0x03,
		0x04,
		{ 0x00, 0x01, 0x02 },
		0x00,
		0x00,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
	{
		DIGIMON_PARA_NAME(NPC2_METALGREYMON)
		0x0000001e,
		0x0118,
		0x01ae,
		0x03,
		0x05,
		{ 0x05, 0x00, 0x01 },
		0x00,
		0x00,
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		0x00,
	},
};

DIGIMON_NAMES_TEXT

Entity *ENTITY_TABLE[10] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
int16_t SECRET_CODES[3][12] = {
	{
		0x1000, 0x1000, 0x4000, 0x4000, 0x8000, 0x2000, 0x8000, 0x2000,
		0x0020, 0x0040, 0x0080, 0x0010,
	},
	{
		0x2000, 0x0020, 0x0020, 0x4000, 0x0040, 0x0040, 0x8000, 0x0080,
		0x0080, 0x1000, 0x0010, 0x0010,
	},
	{
		0x4000, 0x8000, 0x1000, 0x2000, 0x4000, 0x8000, 0x1000, 0x2000,
		0x4000, 0x8000, 0x1000, 0x2000,
	},
};
#endif

int16_t ITEM_THROW_HEIGHT[24] = {
	0xff10, 0xfea7, 0xfe48, 0xfdf3, 0xfda8, 0xfd67, 0xfd30, 0xfd03,
	0xfce0, 0xfcc7, 0xfcb8, 0xfcb3, 0xfcb8, 0xfcc7, 0xfce0, 0xfd03,
	0xfd30, 0xfd67, 0xfda8, 0xfdf3, 0xfe48, 0xfea7, 0xff10, 0xff83,
};
// clang-format on

int32_t GAME_INPUT_REPEAT_TIMER;
int32_t MENU_INPUT_REPEAT_TIMER;
int32_t FIRST_SCREEN_FRAME_COUNTER;
int32_t FIRST_SCREEN_PRESSED_START;
int8_t MAIN_STATE;
int32_t MONEY;
int16_t HOUR;
int16_t MINUTE;
int32_t DRAWING_OFFSET_X;
int32_t DRAWING_OFFSET_Y;
uint8_t MAP_LAYER_ENABLED;
int32_t ACTIVE_FRAMEBUFFER;
GsOT *ACTIVE_ORDERING_TABLE;
int32_t VIEWPORT_DISTANCE;
int32_t VIEWPORT_DISTANCE_COPY;
int32_t DRAWING_OFFSET_X_COPY;
int32_t DRAWING_OFFSET_Y_COPY;
int32_t POLLED_INPUT;
int32_t POLLED_INPUT_PREVIOUS;
int32_t CHANGED_INPUT;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
int16_t SECRET_CODE_PROGRESS;
int8_t TRIAL_END_STATE;
int8_t TRIAL_END_TIMER;
#endif
int32_t IS_IN_MENU;
DigimonEntity *BATTLE_TARGETED_DIGIMON;
DigimonEntity *BATTLE_ATTACKING_DIGIMON;
uint16_t PLAYTIME_MINUTES;
uint16_t PLAYTIME_HOURS;
uint16_t PLAYTIME_FRAMES;
uint8_t YEAR;
int16_t DAY;
uint16_t LAST_HANDLED_FRAME;
uint16_t CURRENT_FRAME;
int8_t GAME_STATE;

static void *main_sbss_order[] = {
	&GAME_STATE,
	&CURRENT_FRAME,
	&LAST_HANDLED_FRAME,
#if !VERSION_IS(JP_TRIAL) && !VERSION_IS(JP_BOMBOM)
	&DAY,
#endif
	&YEAR,
	&PLAYTIME_FRAMES,
	&PLAYTIME_HOURS,
	&PLAYTIME_MINUTES,
	&BATTLE_ATTACKING_DIGIMON,
	&BATTLE_TARGETED_DIGIMON,
	&IS_IN_MENU,
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	&TRIAL_END_TIMER,
	&TRIAL_END_STATE,
	&SECRET_CODE_PROGRESS,
#endif
	&CHANGED_INPUT,
	&POLLED_INPUT_PREVIOUS,
	&POLLED_INPUT,
	&DRAWING_OFFSET_Y_COPY,
	&DRAWING_OFFSET_X_COPY,
	&VIEWPORT_DISTANCE_COPY,
	&VIEWPORT_DISTANCE,
	&ACTIVE_ORDERING_TABLE,
	&ACTIVE_FRAMEBUFFER,
	&MAP_LAYER_ENABLED,
	&DRAWING_OFFSET_Y,
	&DRAWING_OFFSET_X,
	&MINUTE,
	&HOUR,
	&MONEY,
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	&DAY,
#endif
	&MAIN_STATE,
	&FIRST_SCREEN_PRESSED_START,
	&FIRST_SCREEN_FRAME_COUNTER,
	&MENU_INPUT_REPEAT_TIMER,
	&GAME_INPUT_REPEAT_TIMER,
};

SavedState SAVED_STATE;
TamerEntity TAMER_ENTITY;
PartnerEntity PARTNER_ENTITY;
NPCEntity NPC_ENTITIES[8];
GsOT_TAG GS_OT_TAGS_0[4096];
GsOT_TAG GS_OT_TAGS_1[4096];
GsOT GS_ORDERING_TABLE[2];
DR_OFFSET DR_OFFSETS[2];
PACKET GS_WORK_BASES[2][0x14000];
GsRVIEW2 GS_VIEWPOINT;
GsRVIEW2 GS_VIEWPOINT_COPY;
MomentumData PARTNER_MOMENTUM_DATA[34];
PositionData PARTNER_POSITION_DATA[34];
uint8_t PARTNER_MODEL_BUFFER[0x18800];
MomentumData TAMER_MOMENTUM_DATA[22];
PositionData TAMER_POSITION_DATA[22];
#ifdef __MWERKS__
uint8_t TAMER_MODEL_BUFFER[0x7800];
char MAIN_D_801A8B98[0x6800];
#else
/*
 * BOYS.MMD (the tamer model, 57036 bytes) does not fit in the 0x7800 bytes of
 * TAMER_MODEL_BUFFER: the original relies on MAIN_D_801A8B98 coming right
 * after it (0x7800 + 0x6800 = 0xE000). GCC lays out the .bss sections in a
 * different order, so make it one buffer and alias the second half.
 */
uint8_t TAMER_MODEL_BUFFER[0x7800 + 0x6800];
extern char MAIN_D_801A8B98[];
__asm__(".globl MAIN_D_801A8B98\n"
	"\t.set MAIN_D_801A8B98, TAMER_MODEL_BUFFER + 0x7800");
#endif
int8_t MAP_COLLISION_DATA[0x2800];
StdVsCamera STDVS_CAMERA;

static void *main_bss_order[] = {
	&STDVS_CAMERA,
	MAP_COLLISION_DATA,
	MAIN_D_801A8B98,
	TAMER_MODEL_BUFFER,
	TAMER_POSITION_DATA,
	TAMER_MOMENTUM_DATA,
	PARTNER_MODEL_BUFFER,
	PARTNER_POSITION_DATA,
	PARTNER_MOMENTUM_DATA,
	&GS_VIEWPOINT_COPY,
	&GS_VIEWPOINT,
	GS_WORK_BASES,
	DR_OFFSETS,
#if !VERSION_IS(US)
	GS_OT_TAGS_1,
	GS_OT_TAGS_0,
	GS_ORDERING_TABLE,
#else
	GS_ORDERING_TABLE,
	GS_OT_TAGS_1,
	GS_OT_TAGS_0,
#endif
	NPC_ENTITIES,
	&PARTNER_ENTITY,
	&TAMER_ENTITY,
	&SAVED_STATE,
};

int32_t main(void)
{
	int32_t partnerId;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	uint8_t showDemo;

	showDemo = 0;
#endif

	initializeHeap();
	SsInit();
	ResetCallback();
	ResetGraph(0);
	SetGraphDebug(0);
	initializeFramebuffer();
	PadInit(0);
	CdInit();
	MemCardInit(1);
	MemCardStart();
	initializeModelComponents();
	initializeWorldObjects();
	initializeGsTMDMap();
	initializeFileReadQueue();
	initializeAttackObjects();
	initializeFontCLUT();
	fillEFEXTable();
	initializeFadeData();
	initializeScripts();
	view_init();
	initializeUIBoxData();
	initializeEffectData();
	initializeInventoryModules();
	initializeMedalModel();
	initializeClockData();
	initializeStatusObjects();
	loadTIMFile(MAIN_D_8012CE64, GENERAL_BUFFER_PTR);

	for (;;) {
		loadTIMFile(MAIN_D_8012CE78, GENERAL_BUFFER_PTR);
		FIRST_SCREEN_PRESSED_START = 0;
		while (FIRST_SCREEN_PRESSED_START == 0) {
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
			switch (showDemo) {
			case 0:
				playMovie(0, 1);
				break;
			case 1:
				loadDynamicLibrary(VS_REL, NULL, 0, 0, 0);
				VS_playDemo();
				break;
			}
			showDemo = (showDemo + 1) & 1;
#else
			playMovie(0, 1);
#endif
			initializeMusic();
			runLandingScreen();
			finalizeMusic();
		}

		loadStackedTIMFile(MAIN_D_8012CE8C);
		initializeMusic();
		runMainMenu();

		switch (MAIN_STATE) {
		case 0:
			newGameScene();
			finalizeMusic();
			playMovie(1, 1);
			initializeMusic();
			loadStackedTIMFile(MAIN_D_8012CE8C);
			initializeTamer(0, 0, 0, 0, 0, 0, 0);
			if (readPStat(0xfe) == 0) {
				partnerId = 0x3;
			} else {
				partnerId = 0x11;
			}
			initializePartner(partnerId, 0, 0, 0, 0, 0, 0);
			setDigimonRaised(partnerId);
			initializeMap();
			initializeChest();
			runMapHeadScript(0xcc);
			addClock();
			stopGameTime();
			break;
		case 1:
			loadStackedTIMFile(MAIN_D_8012CE8C);
			initializeTamer(0, 0, 0, 0, 0, 0, 0);
			initializePartner(SAVED_STATE.partnerType, 0, 0, 0, 0, 0, 0);
			initializeMap();
			initializeChest();
			runMapHeadScript(SAVED_STATE.currentScreen);
			initializeLoadedMap();
			addClock();
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			if (tamerGetState() != 0) {
				tamerSetState(0);
			}
#endif
			break;
		case 2:
			finalizeMusic();
			playMovie(2, 1);
			initializeMusic();
			loadStackedTIMFile(MAIN_D_8012CE8C);
			initializeTamer(0, 0, 0, 0, 0, 0, 0);
			initializePartner(SAVED_STATE.partnerType, 0, 0, 0, 0, 0, 0);
			initializeMap();
			initializeChest();
			runMapHeadScript(SAVED_STATE.currentScreen);
			initializeLoadedMap();
			addClock();
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			if (tamerGetState() != 0) {
				tamerSetState(0);
			}
#endif
			break;
		}

		fadeFromBlack(0x28);
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
		while (MAIN_STATE != 3) {
			if ((DAY >= 5) && (tamerGetState() == 0)) {
				fadeToBlack(0x28);
				tamerSetState(6);
				partnerSetState(0xb);
				break;
			}
			gameLoop();
		}
		while (FADE_OUT_CURRENT < 0x1f) {
			gameLoop();
		}
		unloadMapParts();
		clearDroppedItems();
		checkFishingMap(0, 0);
		cleanupGame();
		finalizeMusic();
		initializeMusic();
		while (tickTrialEnd() != 1) {
			gameLoop();
		}
		finalizeMusic();
#else
#if VERSION_EQUAL_OR_NEWER(US)
		recalculatePPandArena();
#endif
		while (MAIN_STATE != 3) {
			gameLoop();
		}
		cleanupGame();
		finalizeMusic();
		playMovie(3, 1);
#endif
	}
}

void initializeHeap(void)
{
	unsigned long heap;
	unsigned long size;

	heap = (_end & 0xfffffff0) + 0x10;
	size = ((_stack_addr - (_stack_size << 10)) & 0xfffffff0) - heap;
	InitHeap3((unsigned long *)heap, size);
}

void initializeFramebuffer(void)
{
#if !VERSION_IS(US)
	RECT rect;
	DVECTOR ofs0;
	DVECTOR ofs1;

	ofs0.vx = 160;
	ofs0.vy = 120;
	ofs1.vx = 160;
	ofs1.vy = 360;
#else
	DVECTOR ofs0;
	DVECTOR ofs1;
	RECT rect;

	ofs0 = DRAW_OFFSET_0;
	ofs1 = DRAW_OFFSET_1;
#endif

	SetDispMask(0);
	GsInitGraph(320, 240, 4, 0, 0);
	GsDefDispBuff(0, 0, 0, 240);

	setRECT(&rect, 0, 0, 320, 480);
	ClearImage(&rect, 0, 0, 0);

	GS_ORDERING_TABLE[0].length = 12;
	GS_ORDERING_TABLE[0].org = GS_OT_TAGS_0;
	GS_ORDERING_TABLE[1].length = 12;
	GS_ORDERING_TABLE[1].org = GS_OT_TAGS_1;

	GsInit3D();

	DRAWING_OFFSET_X = 160;
	DRAWING_OFFSET_Y = 120;

	SetDrawOffset(&DR_OFFSETS[0], (u_short *)&ofs1);
	SetDrawOffset(&DR_OFFSETS[1], (u_short *)&ofs0);

	MAP_LAYER_ENABLED = 1;

	SetDispMask(1);
}

void runLandingScreen(void)
{
	int32_t done;
	int32_t timer;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	int32_t i;
#endif

	done = 0;
	timer = 0x258;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	SECRET_CODE = -1;
#endif
	addObject(0xfa3, 0, NULL, (RenderFunction)renderPressStartToContinue);
	FADE_OUT_CURRENT = 0;
	fadeFromBlack(0x28);

	while (--timer > 0) {
		pollInputMenu();

#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
		if ((!(CHANGED_INPUT != 0) == 0) && (SECRET_CODE == -1)) {
			switch (SECRET_CODE_CANDIDATE) {
			case -1:
#if VERSION_IS(JP_TRIAL)
				for (i = 0; i < 3; i++) {
#else
				for (i = 0; i < 1; i++) {
#endif
					if (CHANGED_INPUT & SECRET_CODES[i][0]) {
						SECRET_CODE_CANDIDATE = i;
					}
				}
				SECRET_CODE_PROGRESS = 1;
				break;
			case 0:
			case 1:
			case 2:
				if (CHANGED_INPUT & SECRET_CODES[SECRET_CODE_CANDIDATE][SECRET_CODE_PROGRESS]) {
					SECRET_CODE_PROGRESS++;
					if (SECRET_CODE_PROGRESS == 12) {
						SECRET_CODE = SECRET_CODE_CANDIDATE;
						playSound(0, 0x18);
					}
				} else {
					SECRET_CODE_CANDIDATE = -1;
					SECRET_CODE_PROGRESS = 0;
				}
				break;
			}
		}

#endif
		if ((POLLED_INPUT & 0x800) != 0) {
			if ((FADE_OUT_CURRENT == 0) && (FADE_IN_CURRENT == 0)) {
				playSound(0, 3);
				FIRST_SCREEN_PRESSED_START = 1;
				goto fade;
			}
		}

		ACTIVE_FRAMEBUFFER = GsGetActiveBuff();
		GsSetWorkBase(GS_WORK_BASES[ACTIVE_FRAMEBUFFER]);
		GsClearOt(0, 0, &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
		ACTIVE_ORDERING_TABLE = &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER];
		tickObjects();
		renderObjects();
		applyDrawOffset(ACTIVE_FRAMEBUFFER);
		DrawSync(0);
		VSync(0);
		ResetGraph(1);
		GsSetOrign(DRAWING_OFFSET_X, DRAWING_OFFSET_Y);
		GsSwapDispBuff();
		GsDrawOt(&GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
	}

fade:
	fadeToBlack(0x28);

	while (FADE_OUT_CURRENT < 0x28) {
		ACTIVE_FRAMEBUFFER = GsGetActiveBuff();
		GsSetWorkBase(GS_WORK_BASES[ACTIVE_FRAMEBUFFER]);
		GsClearOt(0, 0, &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
		ACTIVE_ORDERING_TABLE = &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER];
		tickObjects();
		renderObjects();
		applyDrawOffset(ACTIVE_FRAMEBUFFER);
		DrawSync(0);
		VSync(0);
		ResetGraph(1);
		GsSetOrign(DRAWING_OFFSET_X, DRAWING_OFFSET_Y);
		GsSwapDispBuff();
		GsDrawOt(&GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
	}

	removeObject(0xfa3, 0);
}

void runMainMenu(void)
{
	int32_t done;

	done = 0;
	CHECKED_MEMORY_CARD = 0x10;
	CURRENT_MENU = -1;
	TARGET_MENU = 0;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	TARGET_MENU = 0x47;
#endif
	loadTIMFile(MAIN_D_8012CEA0, GENERAL_BUFFER_PTR);
	addObject(0x1388, 0, (TickFunction)tickMainMenu, (RenderFunction)renderMainMenu);
	addObject(0xfa3, 0, NULL, (RenderFunction)renderMainMenuBackground);
	fadeFromBlack(0x28);

	do {
		ACTIVE_FRAMEBUFFER = GsGetActiveBuff();
		ACTIVE_ORDERING_TABLE = &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER];
		GsSetWorkBase(GS_WORK_BASES[ACTIVE_FRAMEBUFFER]);
		GsClearOt(0, 0, &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
		AddPrim(ACTIVE_ORDERING_TABLE->org + 0x20,
			&DR_OFFSETS[ACTIVE_FRAMEBUFFER]);
		pollInputMenu();
		tickObjects();
		renderObjects();
		DrawSync(0);
		VSync(0);
		ResetGraph(1);
		GsSwapDispBuff();
		GsSortClear(0, 0, 0, &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
		GsDrawOt(&GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);

		if ((CURRENT_MENU == -1) && (FADE_OUT_CURRENT == 0)) {
			fadeToBlack(0x28);
			removeObject(0x1388, 0);
		}
	} while ((CURRENT_MENU != -1) && (FADE_OUT_CURRENT < 0x28));

	removeObject(0xfa3, 0);
}

void newGameScene(void)
{
	int32_t done;

	done = 0;
	checkShopMap(0xda);
	initializeNamingBuffer(0);
	initializeTextbox();
	fadeFromBlack(0x14);
	writePStat(0xfe, 0);
	writePStat(0xf3, 0xff);
	loadNewGameScene();

	do {
		/* MOD (fps): rendered by the shared tick / present helper */
		modTickStart(MOD_PRESENT_BATTLE);
		pollInputGame();
		ACTIVE_FRAMEBUFFER = GsGetActiveBuff();
		ACTIVE_ORDERING_TABLE = &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER];
		GsSetWorkBase(GS_WORK_BASES[ACTIVE_FRAMEBUFFER]);
		GsClearOt(0, 0, ACTIVE_ORDERING_TABLE);
		processInput();
#if !VERSION_IS(US)
		tickTextboxHandling();
#else
		tickTextboxHandling(1);
#endif

		if (readPStat(0xf3) == 0) {
			done = newGameStateMachine();
		}

		tickObjects();
		modPresent(MOD_PRESENT_BATTLE, NULL);

		if ((done == 1) && (FADE_OUT_CURRENT == 0)) {
			fadeToBlack(0x28);
		}
	} while ((done == 0) || (FADE_OUT_CURRENT < 0x28));

	unloadNewGameScene();
}

extern VECTOR STORED_TAMER_POS;
int32_t entityCheckCollision(Entity *source, Entity *entity, int32_t arg2, int32_t arg3);
extern uint8_t PREVIOUS_EXIT;
extern int8_t TAMER_PREVIOUS_TILE_X;
extern int8_t TAMER_PREVIOUS_TILE_Y;
extern int8_t TAMER_WAYPOINT_CURRENT;
extern int8_t TAMER_WAYPOINT_ACTIVE;
extern int32_t CAMERA_UPDATE_TILES;
void createCameraMovement(VECTOR *target, int32_t instanceId);
void setImmortalHour(void);
extern int8_t PREV_TILE_X;
extern int8_t PREV_TILE_Y;
void uploadMapTileImages(MapTileData *tiles, int32_t index);
void memcpy(uint8_t *dst, uint8_t *src, uint32_t size);

void initializeLoadedMap(void)
{
	int32_t type;
	int16_t x;
	int32_t i;
	int16_t y;
	int16_t z;
	int16_t rot;
	int16_t idx;
	int16_t exit;

	STORED_TAMER_POS = TAMER_ENTITY.entity.posData->location = SAVED_STATE.playerPos;
	PARTNER_ENTITY.digimonEntity.entity.posData->location = SAVED_STATE.partnerPos;
	initializeDrawingOffsets(MAP_TILE_DATA.tiles);

	for (i = 0; i < 8; i++) {
#if VERSION_EQUAL_OR_OLDER(JP_BOMBOM)
		NPC_ENTITIES[i].digimonEntity.entity.isOnScreen =
			entityIsOffScreen(&NPC_ENTITIES[i].digimonEntity.entity, 320, 240) ^ 1;
#else
		if ((NPC_ENTITIES[i].digimonEntity.entity.type != -1) &&
		    (NPC_ENTITIES[i].digimonEntity.entity.type != 0)) {
			NPC_ENTITIES[i].digimonEntity.entity.isOnScreen = 1;
		}
#endif
	}

	startAnimation(ENTITY_TABLE[0], 0);
	startAnimation(ENTITY_TABLE[1], 0);

	idx = entityCheckCollision(&PARTNER_ENTITY.digimonEntity.entity, &TAMER_ENTITY.entity, 0, 0);
	if ((idx >= 2) && (idx < 10)) {
		type = ENTITY_TABLE[idx]->type;
		if ((type == 0x6d) || (type >= 0x75) ||
		    ((type == 0x5c) && (SAVED_STATE.currentScreen == 0x9b)) ||
		    ((type == 0x6a) && (SAVED_STATE.currentScreen == 0x55)) ||
		    (type == 1) || (type == 2) || (type == 0xf) || (type == 0x10) ||
		    (type == 0x1d) || (type == 0x1e) || (type == 0x2b) || (type == 0x2c)) {
			exit = SAVED_STATE.currentExit;
			x = MAP_WARPS.spawnX[exit];
			y = MAP_WARPS.spawnY[exit];
			z = MAP_WARPS.spawnZ[exit];
			rot = MAP_WARPS.rotation[exit];
			TAMER_ENTITY.entity.posData->location.vx = x;
			TAMER_ENTITY.entity.posData->location.vy = y;
			TAMER_ENTITY.entity.posData->location.vz = z;
			STORED_TAMER_POS = TAMER_ENTITY.entity.posData->location;
			TAMER_ENTITY.entity.posData->rotation.vy = rot;

			if ((rot <= 0x200) || (rot > 0xe00)) {
				z += 0xc8;
			} else if ((rot > 0x200) && (rot <= 0x600)) {
				x += 0xc8;
			} else if ((rot > 0x600) && (rot <= 0xa00)) {
				z -= 0xc8;
			} else if ((rot > 0xa00) && (rot <= 0xe00)) {
				x -= 0xc8;
			}

			PARTNER_ENTITY.digimonEntity.entity.posData->location.vx = x;
			PARTNER_ENTITY.digimonEntity.entity.posData->location.vy = y;
			PARTNER_ENTITY.digimonEntity.entity.posData->location.vz = z;
			startAnimation(ENTITY_TABLE[0], 0);
			startAnimation(ENTITY_TABLE[1], 0);
		} else {
			ENTITY_TABLE[idx]->isOnMap = 0;
		}
	}

	PARTNER_ENTITY.digimonEntity.stats = SAVED_STATE.partnerStats;
	PARTNER_ENTITY.lives = SAVED_STATE.lives;
	MONEY = SAVED_STATE.money;
	PARTNER_PARA = SAVED_STATE.partnerPara;
	PREVIOUS_SCREEN = SAVED_STATE.previousScreen;
	CURRENT_EXIT = SAVED_STATE.currentExit;
	PREVIOUS_EXIT = SAVED_STATE.previousExit;
	memcpy((uint8_t *)TAMER_WAYPOINT_X, (uint8_t *)SAVED_STATE.tamerWaypointX, 0x1e);
	memcpy((uint8_t *)TAMER_WAYPOINT_Y, (uint8_t *)SAVED_STATE.tamerWaypointY, 0x1e);
	TAMER_PREVIOUS_TILE_X = SAVED_STATE.tamerPreviousTileX;
	TAMER_PREVIOUS_TILE_Y = SAVED_STATE.tamerPreviousTileY;
	TAMER_WAYPOINT_CURRENT = SAVED_STATE.tamerWaypointCurrent;
	TAMER_WAYPOINT_COUNT = SAVED_STATE.tamerWaypointCount;
	TAMER_START_TILE_X = SAVED_STATE.tamerStartTileX;
	TAMER_START_TILE_Y = SAVED_STATE.tamerStartTileY;
	TAMER_WAYPOINT_ACTIVE = SAVED_STATE.tamerWaypointActive;
	CAMERA_UPDATE_TILES = 1;
	createCameraMovement(&TAMER_ENTITY.entity.posData->location, 2);
	setImmortalHour();
	initializeDrawingOffsets(MAP_TILE_DATA.tiles);

	MAP_TILE_X = MAP_TILE_DATA.cameraX / 128;
	if (MAP_TILE_DATA.width < 5) {
		MAP_TILE_X = 0;
	} else if ((MAP_TILE_X + 4) > MAP_TILE_DATA.width) {
		MAP_TILE_X -= (int8_t)((MAP_TILE_X + 4) - MAP_TILE_DATA.width);
	}
	PREV_TILE_X = MAP_TILE_X;

	MAP_TILE_Y = MAP_TILE_DATA.cameraY / 128;
	if (MAP_TILE_DATA.height < 4) {
		MAP_TILE_Y = 0;
	} else if ((MAP_TILE_Y + 3) > MAP_TILE_DATA.height) {
		MAP_TILE_Y -= (int8_t)((MAP_TILE_Y + 3) - MAP_TILE_DATA.height);
	}
	PREV_TILE_Y = MAP_TILE_Y;

	uploadMapTileImages(MAP_TILE_DATA.tiles, MAP_TILE_X + (MAP_TILE_Y * (int16_t)MAP_TILE_DATA.width));
	updateMinuteHand(HOUR, MINUTE);
}

#if VERSION_EQUAL_OR_NEWER(US)
void recalculatePPandArena(void)
{
	uint8_t pp;
	int32_t i;

	pp = 0;
	for (i = 3; i < 0x3b; i++) {
		if ((DIGIMON_DATA[i].level >= 3) && (isTriggerSet((uint16_t)(TRIGGER_RECRUITED_BASE + i)) != 0)) {
			if ((i == 0xb) || (i == 0x27) || (i == 0x35)) {
				pp++;
			} else {
				pp += (uint8_t)(DIGIMON_DATA[i].level - 2);
			}
		}
	}
	writePStat(1, pp);
	pp = readPStat(3);
	if ((pp < 0) || (pp >= 0x17)) {
		if (isTriggerSet(0x25) != 0) {
			unsetTrigger(0x25);
		}
		if (isTriggerSet(0x26) != 0) {
			unsetTrigger(0x26);
		}
		if (isTriggerSet(0x27) != 0) {
			unsetTrigger(0x27);
		}
	}
}
#endif

/* MOD (perf): timing of the overworld loop, in horizontal lines (NTSC: ~263
 * per vertical refresh, ~2146 CPU cycles per line). Root counter 1 counts
 * hblanks and runs free (16 bits), so differences of it are elapsed lines
 * even across VSync waits. Set MOD_PERF_RESET to clear everything.
 *   TICK    loop start .. after lateTickObjects (input, script, tickObjects,
 *           late tick): CPU of the game logic
 *   RENDER  renderObjects + applyDrawOffset of the first pass: render CPU
 *   GPU     DrawSync(0) after that: wait for the GPU to finish the previous
 *           displayed frame
 *   XRENDER, XGPU  the same for the extra passes (2nd, 3rd render of a tick)
 *   LAST/MAX/SUM   TICK + RENDER + GPU of each tick (the old busy counter)
 *   PERIOD  lines between the starts of two ticks (789 = 3 refreshes) */
int32_t MOD_PERF_LAST;
int32_t MOD_PERF_MAX;
int32_t MOD_PERF_SUM;
int32_t MOD_PERF_FRAMES;
int32_t MOD_PERF_TICK_SUM;
int32_t MOD_PERF_TICK_MAX;
int32_t MOD_PERF_RENDER_SUM;
int32_t MOD_PERF_RENDER_MAX;
int32_t MOD_PERF_GPU_SUM;
int32_t MOD_PERF_GPU_MAX;
int32_t MOD_PERF_XRENDER_SUM;
int32_t MOD_PERF_XRENDER_MAX;
int32_t MOD_PERF_XGPU_SUM;
int32_t MOD_PERF_XGPU_MAX;
int32_t MOD_PERF_XPASSES;
int32_t MOD_PERF_PERIOD;
int32_t MOD_PERF_LATE;      /* passes swapped after their refresh */
int32_t MOD_PERF_XSTART;    /* lines since tick start when the last extra pass began */
int32_t MOD_PERF_XEND;      /* ... and when it ended (after DrawSync) */
int32_t MOD_PERF_XSTARTV;   /* refreshes since tick start at those two points */
int32_t MOD_PERF_XENDV;
int32_t MOD_PERF_RESET;
int32_t MOD_PERF_SNAP;      /* lines of the interpolation snapshot */
int32_t MOD_PERF_INTERP[4]; /* lines in modInterpApply, modInterpRestore (sums), passes, snapshot sum */
int32_t MOD_PERF_P0END;     /* lines since tick start when the first pass ended */
int32_t MOD_PERF_P0ENDV;    /* ... and refreshes */
static int32_t modPerfPrevStart = -1;
static int32_t modPassSetupLines = 40;

static int32_t modLines(void)
{
	return *(volatile uint16_t *)0x1F801110;
}

/* MOD (fps): struct assignments and memcpy() here call the BIOS memcpy, which
 * is slow (it runs from ROM). The per-pass code copies words or bytes itself;
 * modCopy needs sizes and addresses that are multiples of 4. */
static void modCopy(void *dst, const void *src, int32_t size)
{
	uint32_t *d = dst;
	const uint32_t *s = src;

	for (size >>= 2; size > 0; size--) {
		*d++ = *s++;
	}
}

static void modCopyBytes(uint8_t *dst, const uint8_t *src, int32_t size)
{
	while (size-- > 0) {
		*dst++ = *src++;
	}
}

static void modPerfAdd(int32_t *sum, int32_t *max, int32_t value)
{
	*sum += value;
	if (value > *max) {
		*max = value;
	}
}

static void modPerfReset(void)
{
	MOD_PERF_RESET = 0;
	MOD_PERF_LAST = MOD_PERF_MAX = MOD_PERF_SUM = MOD_PERF_FRAMES = 0;
	MOD_PERF_TICK_SUM = MOD_PERF_TICK_MAX = 0;
	MOD_PERF_RENDER_SUM = MOD_PERF_RENDER_MAX = 0;
	MOD_PERF_GPU_SUM = MOD_PERF_GPU_MAX = 0;
	MOD_PERF_XRENDER_SUM = MOD_PERF_XRENDER_MAX = 0;
	MOD_PERF_XGPU_SUM = MOD_PERF_XGPU_MAX = 0;
	MOD_PERF_XPASSES = 0;
	MOD_PERF_LATE = 0;
	MOD_PERF_INTERP[0] = MOD_PERF_INTERP[1] = MOD_PERF_INTERP[2] = MOD_PERF_INTERP[3] = 0;
}

/* MOD (fps): fixed-timestep loop. The game logic (input, scripts,
 * tickObjects: movement, clock, timers, animation, then the late tick) runs
 * once per tick, and a tick always lasts 3 vertical refreshes (20 Hz) as in
 * the original. The scene is rendered MOD_FPS_RENDERS_PER_TICK times per tick
 * (3 = 60 fps, 1 = original 20 fps), pass r being shown at refresh
 * tickStart + (r + 1) * 3 / n. A pass that would be shown too late, or would
 * not end before the next tick is due (judging by the previous pass's cost),
 * is skipped; the logic never is. See dw/mod_fps.h for how renders stay idempotent. */
int32_t MOD_FPS_RENDERS_PER_TICK = 3;
/* test switch (state checks with the slow interpreter CPU, where the extra
 * passes rarely fit): with N > 0 every tick lasts N refreshes instead of 3,
 * with any number of renders, and every pass is rendered, shown at refresh
 * tickStart + (r + 1) * N / n with that fraction of the tick. Comparing 1 and
 * 3 renders with the same N (large enough for 3 passes, e.g. 8) keeps the
 * emulated time per tick equal, so CD reads end on the same tick in both. */
int32_t MOD_FPS_TEST_TICK;
static int32_t modTickRefreshes(void);
int32_t MOD_FPS_TICKS;
int32_t MOD_FPS_RENDERS;
int32_t MOD_RENDER_PASS;
/* effect render scripts not rolled back on an extra pass for lack of room
 * (BTL_renderEFEEngine): expected to stay 0 */
int32_t MOD_FPS_EFE_NO_ROOM;

static void modFpsBeginFrame(void)
{
	ACTIVE_FRAMEBUFFER = GsGetActiveBuff();
	GsSetWorkBase(GS_WORK_BASES[ACTIVE_FRAMEBUFFER]);
	GsClearOt(0, 0, &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
	ACTIVE_ORDERING_TABLE = &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER];
}

static void modFpsSetView(void)
{
	GsSetOrign(DRAWING_OFFSET_X, DRAWING_OFFSET_Y);
	GsSetRefView2(&GS_VIEWPOINT);
	GsSetProjection(VIEWPORT_DISTANCE);
}

/* The libgs view state the first pass rendered with: the camera set before
 * tickObjects (DRAWING_OFFSET as it was before the tick, the one-tick lag the
 * tiles rely on) plus whatever the tick changed. The extra passes render with
 * exactly the same, not with the post-tick DRAWING_OFFSET/GS_VIEWPOINT. */
long ReadGeomScreen(void);
static _GsPOSITION modPass0Position;
static MATRIX modPass0WsMatrix;
static MATRIX modPass0WsMatrixOrg;
static MATRIX modPass0LightWsMatrix;
static long modPass0Projection;

static void modSavePass0View(void)
{
	modPass0Position = POSITION;
	modCopy(&modPass0WsMatrix, &GsWSMATRIX, sizeof(MATRIX));
	modCopy(&modPass0WsMatrixOrg, &GsWSMATRIX_ORG, sizeof(MATRIX));
	modCopy(&modPass0LightWsMatrix, &GsLIGHTWSMATRIX, sizeof(MATRIX));
	modPass0Projection = ReadGeomScreen();
}

static void modRestorePass0View(void)
{
	POSITION = modPass0Position;
	modCopy(&GsWSMATRIX, &modPass0WsMatrix, sizeof(MATRIX));
	modCopy(&GsWSMATRIX_ORG, &modPass0WsMatrixOrg, sizeof(MATRIX));
	modCopy(&GsLIGHTWSMATRIX, &modPass0LightWsMatrix, sizeof(MATRIX));
	GsSetProjection(modPass0Projection);
}

/* Prims some ticks add straight into the first pass's ordering table, before
 * any render: they are added again, in the same order, at the start of the
 * extra passes. */
int32_t MOD_TICK_PRIM_MAIN_MENU; /* scriptTickMainMenu, tickSaveMachine */
int32_t MOD_TICK_PRIM_MERAMON;   /* tickMeramonShake */
void modReplayMainMenu(void);
void addMeramonShakePrim(void);

static void modReplayTickPrims(void)
{
	if (MOD_TICK_PRIM_MAIN_MENU) {
		modReplayMainMenu();
	}
	if (MOD_TICK_PRIM_MERAMON) {
		addMeramonShakePrim();
	}
}

/* Render-only counters: extra passes start from the value they had before the
 * first pass, and get the first pass's result back afterwards. */
extern int8_t MENU_STATE;
extern int8_t MENU_SUB_STATE;
extern SVECTOR MEDAL_ROTATION;
extern int16_t MONOCHROMON_BUBBLE_TIMER;
extern uint8_t TEXTBOX_OPEN_TIMER;
extern int16_t STATS_GAINS[6];   /* renderPostBattleStatsBox counts them down */
extern uint8_t FLEE_BUBBLE_DATA[8 * 12]; /* renderFleeBubble: frame++ */

static ModRollbackVar MOD_ROLLBACK_VARS[] = {
	{ &MENU_STATE, sizeof(MENU_STATE) },
	{ &MENU_SUB_STATE, sizeof(MENU_SUB_STATE) },
	{ &MEDAL_ROTATION, sizeof(MEDAL_ROTATION) },
	{ &MONOCHROMON_BUBBLE_TIMER, sizeof(MONOCHROMON_BUBBLE_TIMER) },
	{ &TEXTBOX_OPEN_TIMER, sizeof(TEXTBOX_OPEN_TIMER) },
	{ UI_BOX_DATA, sizeof(UIBoxData) * 6 },
	{ STATS_GAINS, sizeof(int16_t) * 6 },
	{ INITIAL_COMBAT_STATS, sizeof(StatsGains) },
	{ FLEE_BUBBLE_DATA, sizeof(FLEE_BUBBLE_DATA) },
};
#define MOD_NUM_ROLLBACK_VARS (sizeof(MOD_ROLLBACK_VARS) / sizeof(MOD_ROLLBACK_VARS[0]))
#define MOD_ROLLBACK_SIZE (1 + 1 + sizeof(SVECTOR) + 2 + 1 + sizeof(UIBoxData) * 6 + \
			   12 + sizeof(StatsGains) + 96)
static uint8_t modRollbackPre[MOD_ROLLBACK_SIZE];
static uint8_t modRollbackPost[MOD_ROLLBACK_SIZE];

static void modRollbackCopy(uint8_t *buf, int32_t toVars)
{
	uint32_t i;

	for (i = 0; i < MOD_NUM_ROLLBACK_VARS; i++) {
		if (toVars) {
			modCopyBytes(MOD_ROLLBACK_VARS[i].ptr, buf, MOD_ROLLBACK_VARS[i].size);
		} else {
			modCopyBytes(buf, MOD_ROLLBACK_VARS[i].ptr, MOD_ROLLBACK_VARS[i].size);
		}
		buf += MOD_ROLLBACK_VARS[i].size;
	}
}

/* the caller's list (modPresent's extra argument) */
static ModRollbackSet *modExtraRollback;

static void modCopyAny(void *dst, const void *src, int32_t size)
{
	if ((((uint32_t)dst | (uint32_t)src | (uint32_t)size) & 3) == 0) {
		modCopy(dst, src, size);
	} else {
		modCopyBytes(dst, src, size);
	}
}

static void modExtraRollbackCopy(uint8_t *buf, int32_t toVars)
{
	ModRollbackSet *set = modExtraRollback;
	int32_t i;
	int32_t size;

	for (i = 0; i < set->count; i++) {
		size = (set->vars[i].size + 3) & ~3; /* keeps the buffer aligned */
		if (toVars) {
			modCopyAny(set->vars[i].ptr, buf, set->vars[i].size);
		} else {
			modCopyAny(buf, set->vars[i].ptr, set->vars[i].size);
		}
		buf += size;
	}
}

void modRenderRollbackSave(void)
{
	modRollbackCopy(modRollbackPre, 0);
	if (modExtraRollback != NULL) {
		modExtraRollbackCopy((uint8_t *)modExtraRollback->pre, 0);
	}
}

void modRenderRollbackBegin(void)
{
	modRollbackCopy(modRollbackPost, 0);
	modRollbackCopy(modRollbackPre, 1);
	if (modExtraRollback != NULL) {
		modExtraRollbackCopy((uint8_t *)modExtraRollback->post, 0);
		modExtraRollbackCopy((uint8_t *)modExtraRollback->pre, 1);
	}
}

void modRenderRollbackEnd(void)
{
	modRollbackCopy(modRollbackPost, 1);
	if (modExtraRollback != NULL) {
		modExtraRollbackCopy((uint8_t *)modExtraRollback->post, 1);
	}
}

int32_t modEfeAreaSave(int32_t data, uint32_t top, uint32_t **buf)
{
	int32_t size = ((EfeFileHeader *)(data - 0x34))->tmdStart & ~3;
	uint32_t *b;

	b = (uint32_t *)((top - (uint32_t)size) & ~3u);
	if (size <= 0 || size > 0x10000 ||
	    (uint32_t)b < (uint32_t)EFE_HEAP_POINTER + 0x5000) {
		MOD_FPS_EFE_NO_ROOM++;
		return 0;
	}
	modCopy(b, (void *)data, size);
	*buf = b;
	return size;
}

void modEfeAreaRestore(int32_t data, uint32_t *buf, int32_t size)
{
	modCopy((void *)data, buf, size);
}

static uint32_t modRenderLcg = 0x1234567;

/* The random numbers the first render pass of a tick drew (modRenderRand,
 * modRenderCustomRandom) are logged and handed out again, in the same order,
 * to the extra passes: what render code places at random (sparkles, the
 * wireframe colours) then changes once per tick as before, not on every
 * frame. Past the log (or if a pass draws more) a private generator is used;
 * the game's generators never advance on an extra pass. */
#define MOD_RAND_LOG_MAX 256
static int32_t modRandLog[MOD_RAND_LOG_MAX];
static int32_t modRandLogCount;
static int32_t modRandLogPos;
static int8_t modRandLogging;   /* inside the first pass's renderObjects */

int32_t modRenderRandRecord(int32_t value)
{
	if (modRandLogging && modRandLogCount < MOD_RAND_LOG_MAX) {
		modRandLog[modRandLogCount++] = value;
	}
	return value;
}

int32_t modRenderRandReplay(int32_t *value)
{
	if (modRandLogPos < modRandLogCount) {
		*value = modRandLog[modRandLogPos++];
		return 1;
	}
	return 0;
}

int32_t modRenderRand(void)
{
	int32_t value;

	if (MOD_RENDER_PASS == 0) {
		return modRenderRandRecord(rand());
	}
	if (modRenderRandReplay(&value)) {
		return value;
	}
	modRenderLcg = modRenderLcg * 1103515245 + 12345;
	return (modRenderLcg >> 16) & 0x7FFF;
}

/* MOD (fps): interpolation of the extra render passes (docs/mods/60fps.md
 * 2.3-2.4). After each tick the state the renders read is snapshotted
 * ("cur", the previous snapshot becomes "prev"); each render pass then
 * temporarily writes values between prev and cur, by the fraction of the tick
 * elapsed at the refresh where the pass is shown, and puts the tick's values
 * back after the pass, so the logic only ever sees its own state.
 * - World camera: the 2D origin the 3D is drawn with (POSITION, which is the
 *   DRAWING_OFFSET of before the tick) and the pair cameraX/Y +
 *   DRAWING_OFFSET_X/Y renderMap places the tiles and map sprites with. Each
 *   is interpolated between its own values of the two ticks, so the one-tick
 *   lag between them is kept and the tiles stay where the 3D is.
 * - Entity roots (bone 0 of ENTITY_TABLE): the translation of the built
 *   matrix, the location field (drop shadow) and the rotation (shortest arc),
 *   rebuilt like setupModelMatrix. Only entities whose matrix was rebuilt
 *   during the tick and still matches its rotation/scale (anything else shows
 *   the matrix as the tick left it), drawn in both ticks, of the same type.
 * - Snaps (render the tick's own state): map load, a change of the 3D view,
 *   game state or the interpolation switch, a camera step over 64 px, an
 *   entity step over 256 units; and a pass whose interpolated camera would
 *   need background tiles outside the VRAM tile window gets the smallest
 *   later fraction that does not. */
int32_t MOD_FPS_INTERP = 1;
int32_t MOD_INTERP_MAP_GEN;   /* bumped on every map setup/unload */
int32_t MOD_INTERP_STATS[6];  /* passes with alpha < 1, entity roots written,
				 VRAM window clamps, snapped ticks, entity
				 skeletons written, bone matrices built */
int32_t MOD_MIST_SHIFT[3];    /* renderMist: x, y and x of the far layer */
int32_t isMistActive(void);

typedef struct {
	Entity *entity;
	int32_t type;
	MATRIX coord;
	VECTOR location;
	VECTOR scale;
	SVECTOR rotation;
	int8_t valid;
} ModInterpRoot;

typedef struct {
	int16_t ox, oy;  /* POSITION of the first pass */
	int16_t cx, cy;  /* MAP_TILE_DATA.cameraX/Y */
	int32_t dx, dy;  /* DRAWING_OFFSET_X/Y */
} ModInterpCam;

/* Bones 1..n-1 (step 5): only the previous tick's values are kept, the
 * current ones are the live fields, which the passes never write (they only
 * build the bone's coord from interpolated values, and rebuild it from the
 * live fields afterwards). boneCount is at most 32. */
#define MOD_BONES_MAX 31
typedef struct {
	int16_t rot[3];
	int16_t loc[3];
	int16_t scale[3];
} ModInterpBone;

/* 0 = bones at 20 Hz (step 3), 1 = tamer and partner, 2 = every entity */
int32_t MOD_FPS_INTERP_BONES = 2;
static ModInterpBone modBonePrev[ENTITY_MAX][MOD_BONES_MAX];
static Entity *modBonePrevEntity[ENTITY_MAX]; /* NULL: modBonePrev not usable */
static int32_t modBonePrevType[ENTITY_MAX];
static uint8_t modBonePrevGen[ENTITY_MAX];
static uint8_t modBoneStartGen[ENTITY_MAX];   /* bumped by startAnimation */
static uint32_t modBoneMask[ENTITY_MAX];      /* bones to interpolate (bit b-1) */
static int8_t modBoneApplied[ENTITY_MAX];
static uint8_t modBonePrevAnimFlag[ENTITY_MAX]; /* anim.animFlag when kept */
/* coords replaced by a pass, put back after it; skeletons that would not fit
 * are not interpolated that tick */
#define MOD_BONE_SAVE_MAX 128
static MATRIX modBoneSave[MOD_BONE_SAVE_MAX];
static int16_t modBoneSaveBase[ENTITY_MAX];
/* bones that move less than this in a tick are drawn as the tick left them
 * (rotation 4096 = 360 degrees; location in model units; scale 4096 = 1) */
#define MOD_BONE_MIN_ROT 4
#define MOD_BONE_MIN_LOC 2
#define MOD_BONE_MIN_SCALE 16
static int32_t modInterpLastTickV;
static int32_t modInterpTickGap;      /* refreshes since the previous tick */

static ModInterpRoot modRootPrev[ENTITY_MAX];
static ModInterpRoot modRootCur[ENTITY_MAX];
static int8_t modRootLerp[ENTITY_MAX];   /* interpolate this tick */
static int8_t modRootApplied[ENTITY_MAX]; /* written for the current pass */
static int8_t modRootOwnRender[ENTITY_MAX]; /* moved by its own render this tick */
static MATRIX modRootSaveCoord[ENTITY_MAX];
static VECTOR modRootSaveLoc[ENTITY_MAX];
static SVECTOR modRootSaveRot[ENTITY_MAX];
static ModInterpCam modCamPrev;
static ModInterpCam modCamCur;
static int8_t modCamLerp;
static int8_t modInterpValid;     /* the previous tick was snapshotted */
static int32_t modInterpMapGen;
static GsRVIEW2 modInterpView;
static int8_t modInterpGameState;
static int16_t modCamSaveCX, modCamSaveCY;
static int32_t modCamSaveDX, modCamSaveDY;
static int8_t modInterpLastPassLerped;
static int8_t modCamApplied;
static int8_t modInterpBattle;     /* modPresent with MOD_PRESENT_BATTLE */
static int8_t modInterpLastBattle;
static MATRIX modInterpWsMatrix;   /* GsWSMATRIX of the previous tick's first pass */

static int32_t modLerp(int32_t a, int32_t b, int32_t alpha)
{
	return a + (((b - a) * alpha + 2048) >> 12);
}

static int32_t modLerpAngle(int32_t a, int32_t b, int32_t alpha)
{
	int32_t d = ((b - a) << 20) >> 20; /* shortest arc, -2048..2047 */

	return a + ((d * alpha + 2048) >> 12);
}

/* main.c has its own memcpy, and no memory compare */
static int32_t modSame(const void *a, const void *b, int32_t size)
{
	const uint8_t *pa = a;
	const uint8_t *pb = b;

	while (size-- > 0) {
		if (*pa++ != *pb++) {
			return 0;
		}
	}
	return 1;
}

static int32_t modAbs(int32_t v)
{
	return v < 0 ? -v : v;
}

static void modClearBoneFlags(Entity *entity)
{
	PositionData *pd = entity->posData;
	int32_t n = DIGIMON_DATA[entity->type].boneCount;
	int32_t i;

	for (i = 0; i < n; i++, pd++) {
		pd->posMatrix.flg = 0;
	}
}

static int32_t modEntityDrawn(Entity *e)
{
	return e->isOnMap == 2 || (e->isOnMap != 0 && e->isOnScreen != 0);
}

/* startAnimation snaps every bone to the new animation's first pose: the
 * skeleton of that entity is not interpolated on that tick */
void modInterpAnimStarted(void *entity)
{
	int32_t i;

	for (i = 0; i < ENTITY_MAX; i++) {
		if (ENTITY_TABLE[i] == entity) {
			modBoneStartGen[i]++;
		}
	}
}

static int32_t modAngleDiff(int32_t a, int32_t b)
{
	return ((b - a) << 20) >> 20; /* shortest arc, -2048..2047 */
}

static int32_t modFits16(int32_t v)
{
	return v >= -32768 && v <= 32767;
}

/* which bones of each entity to interpolate this tick: those that moved by
 * more than a hair, if the entity's animation ran during the tick (so
 * tickMomentum rebuilt every bone's coord from its fields; a bone that changed
 * otherwise was written by other code, and the skeleton is drawn as the tick
 * left it), was not restarted (startAnimation jumps to a new pose), and the
 * entity is the same and drawn in both ticks. Each pass saves the coords it
 * replaces in modBoneSave and puts them back after the pass, so the tick only
 * ever sees its own matrices whatever this check decides. */
static void modInterpSnapshotBones(int32_t snap)
{
	int32_t i;
	int32_t b;
	int32_t n;
	int32_t k;
	int32_t used = 0;
	uint32_t mask;
	Entity *e;
	PositionData *pd;
	ModInterpBone *pb;

	for (i = 0; i < ENTITY_MAX; i++) {
		modBoneMask[i] = 0;
		e = ENTITY_TABLE[i];
		if (snap || MOD_FPS_INTERP_BONES == 0 ||
		    (MOD_FPS_INTERP_BONES == 1 && i > 1) || e == NULL ||
		    !modRootCur[i].valid || !modRootPrev[i].valid ||
		    modRootPrev[i].entity != e || modBonePrevEntity[i] != e ||
		    modBonePrevType[i] != e->type ||
		    modBonePrevGen[i] != modBoneStartGen[i] ||
		    !(modBonePrevAnimFlag[i] & 1)) {
			continue;
		}
		n = DIGIMON_DATA[e->type].boneCount;
		pd = e->posData + 1;
		pb = modBonePrev[i];
		mask = 0;
		k = 0;
		for (b = 1; b < n; b++, pd++, pb++) {
			if (modAbs(modAngleDiff(pb->rot[0], pd->rotation.vx)) < MOD_BONE_MIN_ROT &&
			    modAbs(modAngleDiff(pb->rot[1], pd->rotation.vy)) < MOD_BONE_MIN_ROT &&
			    modAbs(modAngleDiff(pb->rot[2], pd->rotation.vz)) < MOD_BONE_MIN_ROT &&
			    modAbs(pd->location.vx - pb->loc[0]) < MOD_BONE_MIN_LOC &&
			    modAbs(pd->location.vy - pb->loc[1]) < MOD_BONE_MIN_LOC &&
			    modAbs(pd->location.vz - pb->loc[2]) < MOD_BONE_MIN_LOC &&
			    modAbs(pd->scale.vx - pb->scale[0]) < MOD_BONE_MIN_SCALE &&
			    modAbs(pd->scale.vy - pb->scale[1]) < MOD_BONE_MIN_SCALE &&
			    modAbs(pd->scale.vz - pb->scale[2]) < MOD_BONE_MIN_SCALE) {
				continue; /* not moved, or too little to see */
			}
			/* tickMomentum leaves the bone's translation at its location */
			if (pd->posMatrix.coord.t[0] != pd->location.vx ||
			    pd->posMatrix.coord.t[1] != pd->location.vy ||
			    pd->posMatrix.coord.t[2] != pd->location.vz) {
				mask = 0;
				break;
			}
			mask |= 1u << (b - 1);
			k++;
		}
		if (mask == 0 || used + k > MOD_BONE_SAVE_MAX) {
			continue;
		}
		modBoneSaveBase[i] = used;
		used += k;
		modBoneMask[i] = mask;
	}
}

/* write the bone coords for the pass (the fields stay the tick's) */
static void modInterpApplyBones(int32_t i, int32_t alpha)
{
	Entity *e = ENTITY_TABLE[i];
	uint32_t mask = modBoneMask[i];
	PositionData *pd = e->posData + 1;
	ModInterpBone *pb = modBonePrev[i];
	MATRIX *save = &modBoneSave[modBoneSaveBase[i]];
	SVECTOR rot;
	VECTOR loc;
	VECTOR scale;

	for (; mask != 0; mask >>= 1, pd++, pb++) {
		if (!(mask & 1)) {
			continue;
		}
		modCopy(save++, &pd->posMatrix.coord, sizeof(MATRIX));
		rot.vx = modLerpAngle(pb->rot[0], pd->rotation.vx, alpha);
		rot.vy = modLerpAngle(pb->rot[1], pd->rotation.vy, alpha);
		rot.vz = modLerpAngle(pb->rot[2], pd->rotation.vz, alpha);
		loc.vx = modLerp(pb->loc[0], pd->location.vx, alpha);
		loc.vy = modLerp(pb->loc[1], pd->location.vy, alpha);
		loc.vz = modLerp(pb->loc[2], pd->location.vz, alpha);
		scale.vx = modLerp(pb->scale[0], pd->scale.vx, alpha);
		scale.vy = modLerp(pb->scale[1], pd->scale.vy, alpha);
		scale.vz = modLerp(pb->scale[2], pd->scale.vz, alpha);
		/* as calculatePosMatrix/setupModelMatrix build it */
		TransMatrix(&pd->posMatrix.coord, &loc);
		RotMatrix(&rot, &pd->posMatrix.coord);
		ScaleMatrix(&pd->posMatrix.coord, &scale);
		MOD_INTERP_STATS[5]++;
	}
	modClearBoneFlags(e);
	modBoneApplied[i] = 1;
	MOD_INTERP_STATS[4]++;
}

/* the tick's bone coords back */
static void modInterpRestoreBones(int32_t i)
{
	Entity *e = ENTITY_TABLE[i];
	uint32_t mask = modBoneMask[i];
	PositionData *pd = e->posData + 1;
	MATRIX *save = &modBoneSave[modBoneSaveBase[i]];

	for (; mask != 0; mask >>= 1, pd++) {
		if (mask & 1) {
			modCopy(&pd->posMatrix.coord, save++, sizeof(MATRIX));
		}
	}
	modClearBoneFlags(e);
	modBoneApplied[i] = 0;
}

/* after the passes: the tick's bone values become the previous ones */
static void modInterpKeepBones(void)
{
	int32_t i;
	int32_t b;
	int32_t n;
	Entity *e;
	PositionData *pd;
	ModInterpBone *pb;

	for (i = 0; i < ENTITY_MAX; i++) {
		e = ENTITY_TABLE[i];
		modBonePrevEntity[i] = NULL;
		/* the Digivice model poses the live partner from render */
		if (e == NULL || e->posData == NULL || !modEntityDrawn(e) ||
		    modRootOwnRender[i]) {
			continue;
		}
		n = DIGIMON_DATA[e->type].boneCount;
		if (n - 1 > MOD_BONES_MAX) {
			continue;
		}
		pd = e->posData + 1;
		pb = modBonePrev[i];
		for (b = 1; b < n; b++, pd++, pb++) {
			if (!modFits16(pd->location.vx) || !modFits16(pd->location.vy) ||
			    !modFits16(pd->location.vz) || !modFits16(pd->scale.vx) ||
			    !modFits16(pd->scale.vy) || !modFits16(pd->scale.vz)) {
				break;
			}
			pb->rot[0] = pd->rotation.vx;
			pb->rot[1] = pd->rotation.vy;
			pb->rot[2] = pd->rotation.vz;
			pb->loc[0] = pd->location.vx;
			pb->loc[1] = pd->location.vy;
			pb->loc[2] = pd->location.vz;
			pb->scale[0] = pd->scale.vx;
			pb->scale[1] = pd->scale.vy;
			pb->scale[2] = pd->scale.vz;
		}
		if (b == n) {
			modBonePrevEntity[i] = e;
			modBonePrevType[i] = e->type;
			modBonePrevGen[i] = modBoneStartGen[i];
			modBonePrevAnimFlag[i] = e->anim.animFlag;
		}
	}
}

/* forget the previous tick: the next tick renders its own state */
void modInterpInvalidate(void)
{
	modInterpValid = 0;
}

/* after the tick (and late tick), before the first pass */
static void modInterpSnapshot(void)
{
	int32_t i;
	int32_t snap;
	Entity *e;
	PositionData *pd;
	ModInterpRoot *cur;
	ModInterpRoot *prev;
	MATRIX m;

	/* a tick much longer than 3 refreshes: a battle or a load ran inside it */
	snap = !modInterpValid || modInterpMapGen != MOD_INTERP_MAP_GEN ||
	       modInterpGameState != GAME_STATE || modInterpTickGap > 2 * modTickRefreshes() ||
	       modInterpLastBattle != modInterpBattle ||
	       /* a camera cut that does not go through GS_VIEWPOINT (the battle
		* effects' fixed camera, EFE opcodes 63/64) */
	       !modSame(&modInterpWsMatrix, &modPass0WsMatrix, sizeof(MATRIX)) ||
	       !modSame(&modInterpView, &GS_VIEWPOINT, sizeof(GsRVIEW2));
	modInterpMapGen = MOD_INTERP_MAP_GEN;
	modInterpGameState = GAME_STATE;
	modInterpLastBattle = modInterpBattle;
	modCopy(&modInterpWsMatrix, &modPass0WsMatrix, sizeof(MATRIX));
	modCopy(&modInterpView, &GS_VIEWPOINT, sizeof(GsRVIEW2));

	modCopy(&modCamPrev, &modCamCur, sizeof(ModInterpCam));
	if (modInterpBattle) {
		/* the origin is set from DRAWING_OFFSET right before the swap */
		modCamCur.ox = DRAWING_OFFSET_X;
		modCamCur.oy = DRAWING_OFFSET_Y;
	} else {
		modCamCur.ox = modPass0Position.offx;
		modCamCur.oy = modPass0Position.offy;
	}
	modCamCur.cx = MAP_TILE_DATA.cameraX;
	modCamCur.cy = MAP_TILE_DATA.cameraY;
	modCamCur.dx = DRAWING_OFFSET_X;
	modCamCur.dy = DRAWING_OFFSET_Y;
	if (!snap &&
	    (modAbs(modCamCur.ox - modCamPrev.ox) > 64 || modAbs(modCamCur.oy - modCamPrev.oy) > 64 ||
	     modAbs(modCamCur.cx - modCamPrev.cx) > 64 || modAbs(modCamCur.cy - modCamPrev.cy) > 64 ||
	     modAbs(modCamCur.dx - modCamPrev.dx) > 64 || modAbs(modCamCur.dy - modCamPrev.dy) > 64)) {
		snap = 1; /* a camera cut: the entities would not match it either */
	}
	modCamLerp = !snap &&
		     (modCamCur.ox != modCamPrev.ox || modCamCur.oy != modCamPrev.oy ||
		      modCamCur.cx != modCamPrev.cx || modCamCur.cy != modCamPrev.cy ||
		      modCamCur.dx != modCamPrev.dx || modCamCur.dy != modCamPrev.dy);
	if (snap) {
		MOD_INTERP_STATS[3]++;
	}

	for (i = 0; i < ENTITY_MAX; i++) {
		cur = &modRootCur[i];
		prev = &modRootPrev[i];
		modCopy(prev, cur, sizeof(ModInterpRoot));
		modRootLerp[i] = 0;
		modRootOwnRender[i] = 0;
		e = ENTITY_TABLE[i];
		if (e == NULL || e->posData == NULL) {
			cur->valid = 0;
			continue;
		}
		pd = e->posData;
		cur->entity = e;
		cur->type = e->type;
		modCopy(&cur->coord, &pd->posMatrix.coord, sizeof(MATRIX));
		modCopy(&cur->location, &pd->location, sizeof(VECTOR));
		modCopy(&cur->scale, &pd->scale, sizeof(VECTOR));
		modCopy(&cur->rotation, &pd->rotation, sizeof(SVECTOR));
		/* the matrix must be what rotation/scale build, or interpolating
		 * them would show something the tick did not */
		RotMatrix(&cur->rotation, &m);
		ScaleMatrix(&m, &cur->scale);
		cur->valid = modEntityDrawn(e) &&
			     modSame(m.m, cur->coord.m, sizeof(m.m));
		if (snap || !cur->valid || !prev->valid || prev->entity != e ||
		    prev->type != cur->type) {
			continue;
		}
		if (modSame(&prev->coord, &cur->coord, sizeof(MATRIX)) &&
		    prev->location.vx == cur->location.vx &&
		    prev->location.vy == cur->location.vy &&
		    prev->location.vz == cur->location.vz) {
			continue; /* not moved: nothing to interpolate */
		}
		if (modAbs(cur->coord.t[0] - prev->coord.t[0]) > 256 ||
		    modAbs(cur->coord.t[1] - prev->coord.t[1]) > 256 ||
		    modAbs(cur->coord.t[2] - prev->coord.t[2]) > 256 ||
		    modAbs(cur->location.vx - prev->location.vx) > 256 ||
		    modAbs(cur->location.vy - prev->location.vy) > 256 ||
		    modAbs(cur->location.vz - prev->location.vz) > 256) {
			continue; /* a teleport */
		}
		modRootLerp[i] = 1;
	}
	modInterpSnapshotBones(snap);
	modInterpValid = 1;
}

/* is the background at tile camera (ex, ey) inside the VRAM tile window,
 * or at least no further out than the tick's own camera (exCur, eyCur)? */
static int32_t modTilesCovered(int32_t ex, int32_t ey, int32_t exCur, int32_t eyCur)
{
	int32_t lo;
	int32_t hi;
	int32_t cols;

	cols = MAP_TILE_DATA.width > 3 ? 4 : MAP_TILE_DATA.width;
	lo = MAP_TILE_X * 128;
	hi = lo + cols * 128;
	if (exCur < lo) lo = exCur;
	if (exCur + 320 > hi) hi = exCur + 320;
	if (ex < lo || ex + 320 > hi) {
		return 0;
	}
	lo = MAP_TILE_Y * 128;
	hi = lo + 3 * 128;
	if (eyCur < lo) lo = eyCur;
	if (eyCur + 240 > hi) hi = eyCur + 240;
	if (ey < lo || ey + 240 > hi) {
		return 0;
	}
	return 1;
}

/* before renderObjects of a pass shown at fraction alpha (0..4096) of the tick */
static void modInterpApply(int32_t alpha)
{
	int32_t i;
	int32_t cx, cy, dx, dy, ox, oy;
	Entity *e;
	PositionData *pd;
	ModInterpRoot *cur;
	ModInterpRoot *prev;
	SVECTOR rot;

	modInterpLastPassLerped = 0;
	MOD_MIST_SHIFT[0] = MOD_MIST_SHIFT[1] = MOD_MIST_SHIFT[2] = 0;
	if (alpha >= 4096) {
		return;
	}
	if (modCamLerp) {
		/* the background drawn at x is at tile x - (cameraX + DRAWING_OFFSET_X
		 * - POSITION.offx); keep that inside the tiles in VRAM */
		for (;;) {
			cx = modLerp(modCamPrev.cx, modCamCur.cx, alpha);
			cy = modLerp(modCamPrev.cy, modCamCur.cy, alpha);
			dx = modLerp(modCamPrev.dx, modCamCur.dx, alpha);
			dy = modLerp(modCamPrev.dy, modCamCur.dy, alpha);
			ox = modLerp(modCamPrev.ox, modCamCur.ox, alpha);
			oy = modLerp(modCamPrev.oy, modCamCur.oy, alpha);
			if (modTilesCovered(cx + dx - ox, cy + dy - oy,
					    modCamCur.cx + modCamCur.dx - modCamCur.ox,
					    modCamCur.cy + modCamCur.dy - modCamCur.oy)) {
				break;
			}
			MOD_INTERP_STATS[2]++;
			alpha += 512;
			if (alpha >= 4096) {
				return;
			}
		}
		modCamSaveCX = MAP_TILE_DATA.cameraX;
		modCamSaveCY = MAP_TILE_DATA.cameraY;
		modCamSaveDX = DRAWING_OFFSET_X;
		modCamSaveDY = DRAWING_OFFSET_Y;
		MAP_TILE_DATA.cameraX = cx;
		MAP_TILE_DATA.cameraY = cy;
		DRAWING_OFFSET_X = dx;
		DRAWING_OFFSET_Y = dy;
		POSITION.offx = ox;
		POSITION.offy = oy;
		if (isMistActive()) {
			/* the mist moves by the camera step of each tick (tickMist) */
			MOD_MIST_SHIFT[0] = modCamCur.cx - cx;
			MOD_MIST_SHIFT[1] = modCamCur.cy - cy;
			MOD_MIST_SHIFT[2] = (modCamCur.cx - cx) * 12 / 10;
		}
		modCamApplied = 1;
		modInterpLastPassLerped = 1;
	}

	for (i = 0; i < ENTITY_MAX; i++) {
		if (!modRootLerp[i]) {
			continue;
		}
		e = ENTITY_TABLE[i];
		pd = e->posData;
		cur = &modRootCur[i];
		prev = &modRootPrev[i];
		modCopy(&modRootSaveCoord[i], &pd->posMatrix.coord, sizeof(MATRIX));
		modCopy(&modRootSaveLoc[i], &pd->location, sizeof(VECTOR));
		modCopy(&modRootSaveRot[i], &pd->rotation, sizeof(SVECTOR));
		rot.vx = modLerpAngle(prev->rotation.vx, cur->rotation.vx, alpha);
		rot.vy = modLerpAngle(prev->rotation.vy, cur->rotation.vy, alpha);
		rot.vz = modLerpAngle(prev->rotation.vz, cur->rotation.vz, alpha);
		RotMatrix(&rot, &pd->posMatrix.coord);
		ScaleMatrix(&pd->posMatrix.coord, &cur->scale);
		pd->posMatrix.coord.t[0] = modLerp(prev->coord.t[0], cur->coord.t[0], alpha);
		pd->posMatrix.coord.t[1] = modLerp(prev->coord.t[1], cur->coord.t[1], alpha);
		pd->posMatrix.coord.t[2] = modLerp(prev->coord.t[2], cur->coord.t[2], alpha);
		pd->location.vx = modLerp(prev->location.vx, cur->location.vx, alpha);
		pd->location.vy = modLerp(prev->location.vy, cur->location.vy, alpha);
		pd->location.vz = modLerp(prev->location.vz, cur->location.vz, alpha);
		modCopy(&pd->rotation, &rot, sizeof(SVECTOR));
		modClearBoneFlags(e);
		modRootApplied[i] = 1;
		modInterpLastPassLerped = 1;
		MOD_INTERP_STATS[1]++;
	}
	for (i = 0; i < ENTITY_MAX; i++) {
		if (modBoneMask[i] != 0) {
			modInterpApplyBones(i, alpha);
			modInterpLastPassLerped = 1;
		}
	}
	MOD_INTERP_STATS[0]++;
}

static void modInterpRestoreRoot(int32_t i)
{
	Entity *e = ENTITY_TABLE[i];
	PositionData *pd = e->posData;

	modCopy(&pd->posMatrix.coord, &modRootSaveCoord[i], sizeof(MATRIX));
	modCopy(&pd->location, &modRootSaveLoc[i], sizeof(VECTOR));
	modCopy(&pd->rotation, &modRootSaveRot[i], sizeof(SVECTOR));
	modClearBoneFlags(e);
	modRootApplied[i] = 0;
}

/* for render code that moves an entity itself (the Digivice model): give it
 * the tick's values back first */
void modInterpUnapplyEntity(int32_t entityId)
{
	if (entityId >= 0 && entityId < ENTITY_MAX) {
		modRootOwnRender[entityId] = 1;
		if (modRootApplied[entityId]) {
			modInterpRestoreRoot(entityId);
		}
		if (modBoneApplied[entityId]) {
			modInterpRestoreBones(entityId);
		}
	}
}

/* after renderObjects of a pass (the 2D origin stays until the swap) */
static void modInterpRestore(void)
{
	int32_t i;

	if (!modInterpLastPassLerped) {
		return;
	}
	if (modCamApplied) {
		MAP_TILE_DATA.cameraX = modCamSaveCX;
		MAP_TILE_DATA.cameraY = modCamSaveCY;
		DRAWING_OFFSET_X = modCamSaveDX;
		DRAWING_OFFSET_Y = modCamSaveDY;
		MOD_MIST_SHIFT[0] = MOD_MIST_SHIFT[1] = MOD_MIST_SHIFT[2] = 0;
		modCamApplied = 0;
	}
	for (i = 0; i < ENTITY_MAX; i++) {
		if (modRootApplied[i]) {
			modInterpRestoreRoot(i);
		}
		if (modBoneApplied[i]) {
			modInterpRestoreBones(i);
		}
	}
}

/* after the last pass of a tick: if it was interpolated, the bone world
 * matrices (workm, read by some ticks) are recomputed from the tick's own
 * matrices, as the original's single render left them */
static void modInterpEndTick(void)
{
	int32_t i;
	int32_t n;
	int32_t b;
	Entity *e;
	PositionData *pd;
	MATRIX lw;
	MATRIX ls;

	POSITION = modPass0Position;
	modInterpKeepBones();
	if (!modInterpLastPassLerped) {
		return;
	}
	modInterpLastPassLerped = 0;
	for (i = 0; i < ENTITY_MAX; i++) {
		if ((!modRootLerp[i] && modBoneMask[i] == 0) || modRootOwnRender[i]) {
			continue;
		}
		e = ENTITY_TABLE[i];
		pd = e->posData;
		n = DIGIMON_DATA[e->type].boneCount;
		for (b = 0; b < n; b++, pd++) {
			if (pd->obj.tmd != NULL) {
				GsGetLws(pd->obj.coord2, &lw, &ls);
			}
		}
	}
}

/* MOD (fps): the tick / present helper shared by the loops (gameLoop, the
 * battle loops, the new-game scene). A loop calls modTickStart() before its
 * logic, runs the logic (tickObjects included) and then modPresent(), which
 * runs the late tick and renders the tick MOD_FPS_RENDERS_PER_TICK times
 * (pass r shown at refresh tickStart + (r + 1) * 3 / n, interpolated), and
 * waits until the tick has lasted 3 refreshes, as VSync(3) did.
 * MOD_PRESENT_BATTLE: loops that set the 2D origin after VSync from the
 * post-tick DRAWING_OFFSET (battle, battle data load, new-game scene); each
 * pass does that right before its swap. gameLoop sets it before the tick.
 * extra: render-side state of the caller to roll back around the extra
 * passes, like the built-in list (dw/mod_fps.h). */
static int32_t modTickV;
static int32_t modTickL0;
static int32_t modLastEndV = -1000;
static int32_t modLastEndLines;

static int32_t modTickRefreshes(void)
{
	return MOD_FPS_TEST_TICK > 0 ? MOD_FPS_TEST_TICK : 3;
}

void modTickStart(int32_t flags)
{
	int32_t now;

	if (MOD_PERF_RESET != 0) {
		modPerfReset();
	}
	modFlushRemovals();
	MOD_RENDER_PASS = 0;
	now = VSync(-1);
	if ((flags & MOD_PRESENT_BATTLE) && now - modLastEndV < modTickRefreshes()) {
		/* VSync(3) counts from the previous tick's last VSync: the battle
		 * AI, which runs before the loop function, is part of this tick */
		modTickV = modLastEndV;
		modTickL0 = modLastEndLines;
	} else {
		modTickV = now;
		modTickL0 = modLines();
	}
	if (modPerfPrevStart >= 0) {
		MOD_PERF_PERIOD = (modTickL0 - modPerfPrevStart) & 0xFFFF;
	}
	modPerfPrevStart = modTickL0;
	MOD_TICK_PRIM_MAIN_MENU = 0;
	MOD_TICK_PRIM_MERAMON = 0;
}

void modPresent(int32_t flags, ModRollbackSet *extra)
{
	int32_t n;
	int32_t r;
	int32_t tickV;
	int32_t endV;
	int32_t l0;
	int32_t l1;
	int32_t l2;
	int32_t l3;
	int32_t tickLines;
	int32_t passLines = 0;
	int32_t lc = 0;
	int32_t vLast = 0;
	int32_t interp;
	int32_t shownV;
	int32_t battle = flags & MOD_PRESENT_BATTLE;
	int32_t period = modTickRefreshes(); /* refreshes per tick: 3 */
	int32_t force = MOD_FPS_TEST_TICK > 0;

	tickV = modTickV;
	l0 = modTickL0;
	n = MOD_FPS_RENDERS_PER_TICK;
	if (n < 1) {
		n = 1;
	}
	if (n > 3) {
		n = 3;
	}
	/* with one render per tick nothing below touches the interpolation */
	interp = (n > 1) && (MOD_FPS_INTERP != 0);
	if (!interp) {
		modInterpInvalidate();
	}
	modInterpBattle = battle;
	modExtraRollback = n > 1 ? extra : NULL;
	lateTickObjects();
	MOD_FPS_TICKS++;
	l1 = modLines();
	tickLines = (l1 - l0) & 0xFFFF;
	modPerfAdd(&MOD_PERF_TICK_SUM, &MOD_PERF_TICK_MAX, tickLines);

	MOD_RENDER_MULTIPASS = 1;
	for (r = 0; r < n; r++) {
		endV = tickV + (r + 1) * period / n;
		if (r > 0) {
			lc = modLines();
			if (force) {
				/* test switch: every pass, however late (the tick then
				 * lasts longer, the logic per tick is the same) */
			} else if (VSync(-1) >= endV) {
				continue; /* this pass would be shown late: skip it */
			} else if ((tickV + period - vLast) * 263 - VSync(1) <
				   passLines + modPassSetupLines + 4) {
				/* lines left before the vblank that ends the tick,
				 * counted from the last vblank waited for; the pass costs
				 * about what the first one did plus its setup (OT clear,
				 * rollback, view) */
				break; /* it would not end before the next tick is due */
			}
			MOD_RENDER_PASS = r;
			modRenderRollbackBegin();
			modFpsBeginFrame();
			modRestorePass0View();
			modReplayTickPrims();
			if (modExtraRollback != NULL && modExtraRollback->replay != NULL) {
				modExtraRollback->replay();
			}
		} else {
			modSavePass0View();
			if (interp) {
				l2 = modLines();
				modInterpTickGap = tickV - modInterpLastTickV;
				modInterpLastTickV = tickV;
				modInterpSnapshot();
				MOD_PERF_SNAP = (modLines() - l2) & 0xFFFF;
				MOD_PERF_INTERP[3] += MOD_PERF_SNAP;
			}
		}
		l1 = modLines();
		if (r > 0) {
			modPassSetupLines = (l1 - lc) & 0xFFFF;
			MOD_PERF_XSTART = (l1 - l0) & 0xFFFF;
			MOD_PERF_XSTARTV = VSync(-1) - tickV;
		}
		if (interp) {
			/* the fraction of the tick at the refresh this pass is shown */
			shownV = VSync(-1) + 1;
			if (shownV < endV || force) {
				shownV = endV; /* forced: the fraction it would have had */
			}
			if (shownV > tickV + period) {
				shownV = tickV + period;
			}
			l2 = modLines();
			modInterpApply((shownV - tickV) * 4096 / period);
			MOD_PERF_INTERP[0] += (modLines() - l2) & 0xFFFF;
			MOD_PERF_INTERP[2]++;
		}
		/* the first pass logs its render random numbers, the others
		 * replay them */
		modRandLogging = r == 0 && n > 1;
		if (r == 0) {
			modRandLogCount = 0;
		}
		modRandLogPos = 0;
		renderObjects();
		modRandLogging = 0;
		applyDrawOffset(ACTIVE_FRAMEBUFFER);
		if (battle) {
			/* the origin this frame is drawn with (the interpolated one if
			 * the camera is), as these loops set it after VSync */
			GsSetOrign(DRAWING_OFFSET_X, DRAWING_OFFSET_Y);
		}
		if (interp) {
			l2 = modLines();
			modInterpRestore();
			MOD_PERF_INTERP[1] += (modLines() - l2) & 0xFFFF;
		}
		l2 = modLines();
		DrawSync(0);
		l3 = modLines();
		if (r == 0) {
			passLines = (l3 - l1) & 0xFFFF;
			modPerfAdd(&MOD_PERF_RENDER_SUM, &MOD_PERF_RENDER_MAX, (l2 - l1) & 0xFFFF);
			modPerfAdd(&MOD_PERF_GPU_SUM, &MOD_PERF_GPU_MAX, (l3 - l2) & 0xFFFF);
			MOD_PERF_LAST = tickLines + ((l3 - l1) & 0xFFFF);
			MOD_PERF_P0END = (l3 - l0) & 0xFFFF;
			MOD_PERF_P0ENDV = VSync(-1) - tickV;
			modPerfAdd(&MOD_PERF_SUM, &MOD_PERF_MAX, MOD_PERF_LAST);
			MOD_PERF_FRAMES++;
		} else {
			modRenderRollbackEnd();
			modPerfAdd(&MOD_PERF_XRENDER_SUM, &MOD_PERF_XRENDER_MAX, (l2 - l1) & 0xFFFF);
			modPerfAdd(&MOD_PERF_XGPU_SUM, &MOD_PERF_XGPU_MAX, (l3 - l2) & 0xFFFF);
			MOD_PERF_XPASSES++;
			MOD_PERF_XEND = (l3 - l0) & 0xFFFF;
			MOD_PERF_XENDV = VSync(-1) - tickV;
		}
		if (VSync(-1) >= endV) {
			MOD_PERF_LATE++;
		}
		do {
			VSync(0); /* like VSync(3): always swap on a vertical blank */
		} while (VSync(-1) < endV);
		vLast = VSync(-1);
		GsSwapDispBuff();
		GsSortClear(0, 0, 0, &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
		GsDrawOt(&GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER]);
		MOD_FPS_RENDERS++;
	}
	if (interp) {
		modInterpEndTick();
	}
	if (battle) {
		GsSetOrign(DRAWING_OFFSET_X, DRAWING_OFFSET_Y);
	}
	while (VSync(-1) < tickV + period) {
		VSync(0); /* the tick lasts 3 refreshes, as VSync(3) did */
	}
	MOD_RENDER_PASS = 0;
	MOD_RENDER_MULTIPASS = 0;
	modExtraRollback = NULL;
	modFlushRemovals();
	modLastEndV = VSync(-1);
	modLastEndLines = modLines();
}

void gameLoop(void)
{
	modTickStart(0);
	pollInputGame();
	modFpsBeginFrame();
	processInput();
	updateTournamentRegistration();

	if (IS_SCRIPT_PAUSED == 0) {
		tickScript();
	}

	modFpsSetView();
	GS_VIEWPOINT_COPY = GS_VIEWPOINT;
	VIEWPORT_DISTANCE_COPY = VIEWPORT_DISTANCE;
	DRAWING_OFFSET_X_COPY = DRAWING_OFFSET_X;
	DRAWING_OFFSET_Y_COPY = DRAWING_OFFSET_Y;
	tickObjects();
	modPresent(0, NULL);
	handlePause();
}

#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
int32_t tickTrialEnd(void)
{
	RECT targetRect;
	RECT sourceRect;

	switch (TRIAL_END_STATE) {
	case 0:
		if (FADE_OUT_CURRENT >= 0x28) {
			if (CONDITION_BUBBLE_TYPE != -1) {
				removeConditionBubble(CONDITION_BUBBLE_ID);
				CONDITION_BUBBLE_TIMER = 0;
				CONDITION_BUBBLE_TYPE = -1;
			}
			if (HAS_BUTTERFLY == 0) {
				unsetButterfly(BUTTERFLY_ID);
				HAS_BUTTERFLY = -1;
			}
			clearTextArea();
			drawString(STR_THATS_ALL_FOR_THIS_TIME, 0, 0xc);
			TRIAL_END_STATE = 1;
		}
		break;
	case 1:
		drawString(STR_PLEASE_ENJOY_THE_REST_IN_THE, 0, 0x18);
		fadeFromBlack(0x28);
		TRIAL_END_STATE = 2;
		break;
	case 2:
		if (FADE_IN_CURRENT >= 0x28) {
			TRIAL_END_STATE = 3;
		}
		break;
	case 3:
		if (isUIBoxAvailable(1) == 1) {
			setRECT(&targetRect, -130, 42, 262, 59);
			setRECT(&sourceRect, -5, -5, 10, 10);
			createAnimatedUIBox(1, 0, 2, &targetRect, &sourceRect, NULL, (RenderFunction)renderTrialEndBox);
			TRIAL_END_TIMER = 0;
			TRIAL_END_STATE = 4;
		}
		break;
	case 4:
		if (TRIAL_END_TIMER >= 0x78) {
			setRECT(&targetRect, -5, -5, 10, 10);
			removeAnimatedUIBox(1, &targetRect);
			TRIAL_END_TIMER = 0;
			TRIAL_END_STATE = 5;
		}
		TRIAL_END_TIMER++;
		break;
	case 5:
		if ((isUIBoxAvailable(1) == 1) && (TRIAL_END_TIMER >= 0x64)) {
			TRIAL_END_TIMER = 0;
			TRIAL_END_STATE = 0;
			return 1;
		}
		TRIAL_END_TIMER++;
		break;
	}
	return 0;
}
#endif

void applyDrawOffset(int32_t offset)
{
	AddPrim((char *)GS_ORDERING_TABLE[offset].org + 0x80,
		&DR_OFFSETS[offset]);
}

void pollInputGame(void)
{
	int32_t held;

	POLLED_INPUT_PREVIOUS = POLLED_INPUT;
	POLLED_INPUT = PadRead(0);
	CHANGED_INPUT = POLLED_INPUT & ~POLLED_INPUT_PREVIOUS;
	held = POLLED_INPUT & POLLED_INPUT_PREVIOUS;
	held &= 0xf000f000;
	if (held != 0) {
		if (++GAME_INPUT_REPEAT_TIMER >= 5) {
			GAME_INPUT_REPEAT_TIMER -= 2;
		} else {
			held = 0;
		}
	} else {
		GAME_INPUT_REPEAT_TIMER = 0;
	}
	CHANGED_INPUT |= held;
}

void pollInputMenu(void)
{
	int32_t held;

	POLLED_INPUT_PREVIOUS = POLLED_INPUT;
	POLLED_INPUT = PadRead(0);
	CHANGED_INPUT = POLLED_INPUT & ~POLLED_INPUT_PREVIOUS;
	held = POLLED_INPUT & POLLED_INPUT_PREVIOUS;
	held &= 0xf000f000;
	if (held != 0) {
		if (++MENU_INPUT_REPEAT_TIMER >= 0xf) {
			MENU_INPUT_REPEAT_TIMER -= 6;
		} else {
			held = 0;
		}
	} else {
		MENU_INPUT_REPEAT_TIMER = 0;
	}
	CHANGED_INPUT |= held;
}

void renderPressStartToContinue(void)
{
	GsOT_TAG *ot;
	POLY_FT4 *prim;

	ot = ACTIVE_ORDERING_TABLE->org;
	prim = (POLY_FT4 *)GsGetWorkBase();
	FIRST_SCREEN_FRAME_COUNTER = FIRST_SCREEN_FRAME_COUNTER + 1;

	if (FIRST_SCREEN_PRESSED_START == 1) {
		FIRST_SCREEN_FRAME_COUNTER = FIRST_SCREEN_FRAME_COUNTER + 0x1d;
	}

	FIRST_SCREEN_FRAME_COUNTER = FIRST_SCREEN_FRAME_COUNTER % 0x3c;

	if (FIRST_SCREEN_FRAME_COUNTER < 0x1e) {
		SetPolyFT4(prim);
		setXYWH(prim, -0x36, 0x32, 121, 10);
		setUVWH(prim, 0, 0xf1, 121, 10);
#if VERSION_IS(JP)
		setRGB0(prim, 0x80, 0x80, 0x80);
#elif VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
		setRGB0(prim, 0x80, 0x80, 0x80);
		switch (SECRET_CODE) {
		case -1:
			break;
		case 0:
			setRGB0(prim, 0x80, 0, 0);
			break;
		case 1:
			setRGB0(prim, 0, 0x80, 0);
			break;
		case 2:
			setRGB0(prim, 0, 0, 0x80);
			break;
		}
#else
		setRGB0(prim, 0, 0x80, 0);
#endif
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
		if (SECRET_CODE != -1) {
			setTPage(prim, 1, 0, 768, 0);
		}
#else
		setTPage(prim, 1, 0, 768, 0);
#endif
		setClut(prim, 0, 480);
		AddPrim(ot + 30, prim++);
	}

	SetPolyFT4(prim);
	setXYWH(prim, -0xa0, -0x78, 256, 240);
	setUVWH(prim, 0, 0, 255, 240);
	setRGB0(prim, 0x80, 0x80, 0x80);
	setTPage(prim, 1, 0, 768, 0);
	setClut(prim, 0, 480);
	AddPrim(ot + 30, prim++);

	SetPolyFT4(prim);
	setXYWH(prim, 0x60, -0x78, 64, 240);
	setUVWH(prim, 0, 0, 64, 240);
	setRGB0(prim, 0x80, 0x80, 0x80);
	setTPage(prim, 1, 0, 896, 0);
	setClut(prim, 0, 480);
	AddPrim(ot + 30, prim++);
	GsSetWorkBase((PACKET *)prim);
}

void renderMainMenuBackground(void)
{
	GsOT_TAG *ot;
	POLY_FT4 *prim;

	ot = ACTIVE_ORDERING_TABLE->org;
	prim = (POLY_FT4 *)GsGetWorkBase();

	SetPolyFT4(prim);
	setXYWH(prim, -0xa0, -0x78, 256, 240);
	setUVWH(prim, 0, 0, 255, 240);
	setRGB0(prim, 0x80, 0x80, 0x80);
	setTPage(prim, 1, 0, 768, 0);
	setClut(prim, 0, 480);
	AddPrim(ot + 30, prim++);

	SetPolyFT4(prim);
	setXYWH(prim, 0x60, -0x78, 64, 240);
	setUVWH(prim, 0, 0, 64, 240);
	setRGB0(prim, 0x80, 0x80, 0x80);
	setTPage(prim, 1, 0, 896, 0);
	setClut(prim, 0, 480);
	AddPrim(ot + 30, prim++);
	GsSetWorkBase((PACKET *)prim);
}

#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
void renderTrialEndBox(void)
{
	renderString(0, -0x7d, 0x2d, 0xf0, 0x18, 0, 0xc, 5, 0);
}
#endif

GARBAGE_ARRAY(view_init, (&SAVED_STATE.partnerType), 1, 1);

void view_init(void)
{
	GsSetProjection(0x400);
	GS_VIEWPOINT.vpx = 0;
	GS_VIEWPOINT.vpy = -0xb73;
	GS_VIEWPOINT.vpz = -0xb73;
	GS_VIEWPOINT.vpy = 0;
	GS_VIEWPOINT.vpz = -0x7d0;
	GS_VIEWPOINT.vrx = 0;
	GS_VIEWPOINT.vry = 0;
	GS_VIEWPOINT.vrz = 0;
	GS_VIEWPOINT.rz = 0;
	GS_VIEWPOINT.super = 0;
	GsSetRefView2(&GS_VIEWPOINT);
	GsSetNearClip(0x64);
}

void initializeEffectData(void)
{
	initializeEFE();
	initializeParticleFX();
	initializeEntityParticleFX();
	initializeCloudFXData();
	initializeHealingParticles();
}

void initializeInventoryModules(void)
{
	COMBAT_DATA_PTR = &COMBAT_DATA;
	resetFlattenGlobal();
	initializeDroppedItems();
	initializeInventoryUI();
	initializeInventory();
	initializeEntityText();
}

void addThrownItem(type)
	int16_t type;
{
	MATRIX *workm;

	if (TAMER_ITEM.worldItem.type == ITEM_NONE) {
		workm = &TAMER_ENTITY.entity.posData[9].posMatrix.workm;
		TAMER_ITEM.worldItem.spriteLocation.vx = workm->t[0];
		TAMER_ITEM.worldItem.spriteLocation.vy = workm->t[1];
		TAMER_ITEM.worldItem.spriteLocation.vz = workm->t[2];
		TAMER_ITEM.time = 0;
		TAMER_ITEM.worldItem.type = type;
		addObject(0x194, 0, tickThrownItem, renderThrownItem);
	}
}

void startThrowingItem(void)
{
	startAnimation(ENTITY_TABLE[0], 6);
	addThrownItem(INVENTORY.types.array[INVENTORY_POINTER]);
	removeItem(INVENTORY.types.array[INVENTORY_POINTER], 1);
}

void tickThrownItem(int32_t instanceId)
{
	SVECTOR step;
	VECTOR diff;
	MATRIX *workm;
	VECTOR *loc;

	if ((TAMER_ENTITY.entity.anim.animId == 6) && (TAMER_ENTITY.entity.anim.animFrame < 7)) {
		workm = &TAMER_ENTITY.entity.posData[9].posMatrix.workm;
		TAMER_ITEM.worldItem.spriteLocation.vx = workm->t[0];
		TAMER_ITEM.worldItem.spriteLocation.vy = workm->t[1];
		TAMER_ITEM.worldItem.spriteLocation.vz = workm->t[2];
		return;
	}

	loc = &PARTNER_ENTITY.digimonEntity.entity.posData->location;
	diff.vx = loc->vx - TAMER_ITEM.worldItem.spriteLocation.vx;
	diff.vy = 0;
	diff.vz = loc->vz - TAMER_ITEM.worldItem.spriteLocation.vz;
	if (TAMER_ITEM.time == 0) {
		step.vx = 0;
		step.vy = ITEM_THROW_HEIGHT[TAMER_ITEM.time];
		step.vz = 0;
	} else {
		step.vx = diff.vx / (0x19 - TAMER_ITEM.time);
		step.vy = ITEM_THROW_HEIGHT[TAMER_ITEM.time];
		step.vz = diff.vz / (0x19 - TAMER_ITEM.time);
	}

	TAMER_ITEM.worldItem.spriteLocation.vy = 0;
	TAMER_ITEM.worldItem.spriteLocation.vx += step.vx;
	TAMER_ITEM.worldItem.spriteLocation.vy += step.vy;
	TAMER_ITEM.worldItem.spriteLocation.vz += step.vz;
	TAMER_ITEM.time++;

	if (TAMER_ITEM.time >= 0x18) {
		if (ITEM_FUNCTIONS[TAMER_ITEM.worldItem.type] != NULL) {
			ITEM_FUNCTIONS[TAMER_ITEM.worldItem.type](TAMER_ITEM.worldItem.type);
		}
		BTL_addItemParticles(ENTITY_TABLE[1]);
		removeTamerItem();
	}
}

void renderThrownItem(int32_t instanceId)
{
	renderOverworldItem(&TAMER_ITEM.worldItem);
}

void handleBuffDisks(type)
	int16_t type;
{
	BaseStats *stats;
	int16_t offLimit;
	int16_t defLimit;
	int16_t speedLimit;
	int16_t gain;

	if (PARTNER_ENTITY.digimonEntity.stats.current.currentHP == 0) {
		return;
	}

	offLimit = (INITIAL_COMBAT_STATS[0].offense +
		    ((INITIAL_COMBAT_STATS[0].offense * 3) / 10));
	if (offLimit > 999) {
		offLimit = 999;
	}

	defLimit = (INITIAL_COMBAT_STATS[0].defense +
		    ((INITIAL_COMBAT_STATS[0].defense * 3) / 10));
	if (defLimit > 999) {
		defLimit = 999;
	}

	speedLimit = (INITIAL_COMBAT_STATS[0].speed +
		      ((INITIAL_COMBAT_STATS[0].speed * 3) / 10));
	if (speedLimit > 999) {
		speedLimit = 999;
	}

	stats = &PARTNER_ENTITY.digimonEntity.stats.base;

	switch(type) {
	case 0xf:
		if (offLimit < (stats->off + 20)) {
			gain = offLimit - stats->off;
		}
		else {
			gain = 20;
		}
		BTL_buffStats(ENTITY_TABLE[1], 0, gain, &stats->off, 0xb, 3);
		break;
	case 0x10:
		if ((stats->def + 20) > defLimit) {
			gain = defLimit - stats->def;
		}
		else {
			gain = 20;
		}
		BTL_buffStats(ENTITY_TABLE[1], 0, gain, &stats->def, 0xb, 4);
		break;
	case 0x11:
		if ((stats->speed + 20) > speedLimit) {
			gain = speedLimit - stats->speed;
		}
		else {
			gain = 20;
		}
		BTL_buffStats(ENTITY_TABLE[1], 0, gain, &stats->speed, 0xb, 5);
		break;
	case 0x12:
#if !VERSION_IS(US)
		if (offLimit < (stats->off + 20)) {
			stats = stats;
		} else {
			stats = stats;
		}
		if ((stats->def + 20) > defLimit) {
			stats = stats;
		} else {
			stats = stats;
		}
		if ((stats->speed + 20) > speedLimit) {
			stats = stats;
		} else {
			stats = stats;
		}
#endif
		BTL_buffStats(ENTITY_TABLE[1], 0, 20, &stats->off, 0xb, 3);
		BTL_buffStats(ENTITY_TABLE[1], 0, 20, &stats->def, 0xb, 4);
		BTL_buffStats(ENTITY_TABLE[1], 0, 20, &stats->speed, 0xb, 5);
		break;
	case 0x13:
		if (offLimit < (stats->off + 50)) {
			gain = offLimit - stats->off;
		}
		else {
			gain = 50;
		}
		BTL_buffStats(ENTITY_TABLE[1], 0, gain, &stats->off, 0xb, 3);
		break;
	case 0x14:
		if ((stats->def + 50) > defLimit) {
			gain = defLimit - stats->def;
		}
		else {
			gain = 50;
		}
		BTL_buffStats(ENTITY_TABLE[1], 0, gain, &stats->def, 0xb, 4);
		break;
	case 0x15:
		if ((stats->speed + 50) > speedLimit) {
			gain = speedLimit - stats->speed;
		}
		else {
			gain = 50;
		}
		BTL_buffStats(ENTITY_TABLE[1], 0, gain, &stats->speed, 0xb, 5);
		break;
	}

	BTL_addBuffDiskEffect(ENTITY_TABLE[1]);
}
