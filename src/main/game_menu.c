#include <stdio.h>
#include <string.h>

#include <libetc.h>
#include <libgpu.h>
#include <libgs.h>
#include <libgte.h>

#include <dw/mod_font.h>
#include <dw/mod_options.h>
#include <dw/caps.h> /* MOD (caps) */
#include <dw/mod_fps.h>
#include <dw/clock.h>
#include <dw/entity.h>
#include <dw/evl.h>
#include <dw/fish.h>
#include <dw/font.h>
#include <dw/garbage.h>
#include <dw/graphics.h>
#include <dw/input.h>
#include <dw/item.h>
#include <dw/level.h>
#include <dw/main.h>
#include <dw/model.h>
#include <dw/move.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/sound.h>
#include <dw/species.h>
#include <dw/tamer.h>
#include <dw/ui.h>
#include <dw/lang.h>
#include <dw/version.h>
#include <text/main/game_menu.h>

/* Colors of the two button glyphs in the medal and card view footers */
#if !VERSION_IS(US)
#define FOOTER_GLYPH1_COLOR 0xe
#define FOOTER_GLYPH2_COLOR 0xf
#else
#define FOOTER_GLYPH1_COLOR 0xf
#define FOOTER_GLYPH2_COLOR 7
#endif

typedef struct {
	int8_t tab[2];
} DigimonTabs;

typedef struct {
	int8_t tab[5];
} PlayerTabs;

typedef struct {
	uint8_t data[8];
} TriangleCursorUVData;

typedef struct {
	int8_t data[8];
} TriangleCursorOffsetData;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t unknown;
	int8_t disabled;
	int8_t clutY;
	uint8_t width;
	uint8_t height;
	uint8_t texX;
	uint8_t texY;
} GameMenuSprite;

typedef struct {
	int16_t x;
	int16_t y;
	uint8_t width;
	uint8_t height;
	uint8_t texX;
	uint8_t texY;
} GameMenuLabel;

/* MOD (care numbers): happiness runs -100..100 in the rules; players see it as 0..100, as its bar shows it */
#define HAPPINESS_SHOWN(h) (((h) + 100) / 2)

typedef struct {
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
	uint8_t color;
	uint8_t pad;
} Line;

typedef struct {
	int16_t posX;
	int16_t posY;
	uint8_t u;
	uint8_t v;
	uint8_t clut;
	uint8_t pad;
} EvoChartEntry;

typedef struct {
	int16_t posX;
	int16_t posY;
	uint8_t width;
	uint8_t height;
	uint8_t texX;
	uint8_t texY;
} IconRect;

typedef struct {
	int16_t posX;
	int16_t posY;
	int16_t uvWidth;
	uint8_t uvX;
	uint8_t uvY;
} StringRect;

typedef struct {
	uint8_t v[21];
} StatsIconClutTable;

typedef struct {
	int16_t m[5];
} EvoClutTable;

typedef struct {
	int16_t x1;
	int16_t x2;
	int16_t x3;
	int16_t x4;
	int16_t y1;
	int16_t y2;
	int16_t y3;
	int16_t y4;
} Line4Points;

typedef struct {
	int16_t posX;
	int16_t posY;
	int16_t unk4;
	int16_t unk6;
} ChartSprite;

typedef struct {
	int16_t posX;
	int16_t posY;
	int16_t width;
	int16_t height;
} Inset;

typedef struct {
	int8_t v[66];
} CardSprites;

typedef struct {
	int32_t v[6];
} ConditionMaskTable;

extern GsDOBJ2 MEDAL_OBJECT;
extern GsCOORDINATE2 MEDAL_COORDINATES;
extern int32_t IS_IN_MENU;
extern GsOT *ACTIVE_ORDERING_TABLE;
extern int32_t CHANGED_INPUT;
extern uint8_t INVENTORY_POINTER;
extern uint32_t POLLED_INPUT;
extern uint32_t POLLED_INPUT_PREVIOUS;
extern int32_t MONEY;
extern uint16_t PLAYTIME_FRAMES;
extern int32_t ACTIVE_FRAMEBUFFER;
extern GsOT_TAG *FRAMEBUFFER0_ORIGIN;
extern GsOT_TAG *FRAMEBUFFER1_ORIGIN;
extern GsOT *FRAMEBUFFER_OT[2];
extern GsRVIEW2 GS_VIEWPOINT;
extern int32_t VIEWPORT_DISTANCE;
extern char MAIN_D_80124C0C[][12];
extern char MAIN_D_80124C54[];
extern char *MOVE_NAMES[];

void drawInventoryText(void);
void closeTriangleMenu(void);
void closeInventoryBoxes2();
void renderDigimonMovesView(void);
void renderDigimonStatsView(void);
void renderCardsView(void);
void renderMedalView(void);
void renderEvoChartView(void);
static void chartDetailOpen(int32_t id); /* MOD (portraits) */
static int32_t chartDetailInput(void);
static void chartOverviewInput(int32_t grid); /* MOD (chart) */
static void chartBoxXY(int32_t col, int32_t row, int16_t *x, int16_t *y);
void loadMenuOverlay(void);
void closeMenuOverlay(void);
static void chartCardRelease(void); /* MOD (chart criteria) */
static void chartVanillaInput(void); /* MOD (options): NEW DIGIMON off */
void renderPlayerInfoView(void);
void renderString();
void renderMenuTab(int16_t x, int8_t w, int8_t layer);
void setCameraFollowPlayer(void);
void handleGameMenuSelection(int32_t selection);
int32_t createMenuBox(int16_t id, int16_t x, int16_t y, int16_t width,
                      int16_t height, int8_t features, void (*tick)(void),
                      void (*render)(void));
void closeUIBoxIfOpen(int32_t arg);
void getEntityScreenPos(Entity *entity, int32_t flag, DVECTOR *outPos);
void addInventoryUI(void);
void tickGameMenu(void);
void renderGameMenu(void);
void tickDigimonMenu(void);
void renderDigimonMenu(void);
void tickPlayerMenu(void);
void renderPlayerMenu(void);
void tickTriangleMenu(void);
void renderRectPolyFT4(int16_t posX, int16_t posY, uint8_t width,
                       uint8_t height, uint8_t texX, uint8_t texY,
                       int16_t texturePage, int16_t clut, int32_t zIndex,
                       int8_t flag);
void renderSeparatorLines(Line *lines, int8_t count, int32_t zIndex);
void setPosDataPolyFT4(POLY_FT4 *prim, int32_t posX, int32_t posY,
                       int32_t width, int32_t height);
void setUVDataPolyFT4(POLY_FT4 *prim, int32_t uPos, int32_t vPos,
                      int32_t width, int32_t height);
void removeTriangleMenu(void);
void removeUIBox1(void);
void addGameMenu(void);
void renderDateDigits(void);
void renderTriangleCursor(int8_t selection, int16_t yOffset);
int32_t isUIBoxAvailable(int32_t id);
void setSleepDisabled(int16_t arg);
void startFeedingItem(ItemId arg);
void removeOneSelectedItem(void);
void renderFeedingItem(int16_t arg);
int8_t getEquippedSlot(void);
void equipMove(void);
int32_t isKeyDown(int32_t mask);
void convertValueToDigits(int32_t digits, int32_t value, int32_t *outCount,
                          int32_t *outDigits);
void drawLine2P(int32_t color, int32_t x0, int32_t y0, int32_t x1,
                int32_t y1, int32_t zIndex, int32_t flag);
void drawLine3P(int32_t color, int32_t x0, int32_t y0,
                int32_t x1, int32_t y1, int32_t x2,
                int32_t y2, int32_t zIndex, int32_t flag);
int32_t hasDigimonRaised(int32_t digimonId);
int32_t hasMedal(uint16_t medal);
void activateMedalTexture(int32_t medalId, int32_t previousMedalId);
int32_t loadCardImage(int32_t id);
void renderEvoChartDetail(void);
void renderCardImage(void);
void renderCardCount(void);
int32_t hasMove(int32_t moveId);
int32_t drawPlayerInfoStrings(void);
int32_t isTriggerSet(uint16_t trigger);
void renderInsetBox(int16_t a, int16_t b, int16_t c, int16_t d, int32_t otz);
void renderDigiviceEntity(Entity *entity, int32_t entityId);
int32_t drawDigimonMovesText(void);
u_short GetTPage(int32_t tp, int32_t abr, int32_t x, int32_t y);
u_short GetClut(int32_t x, int32_t y);
void renderBox(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t r,
               uint8_t g, uint8_t b, uint8_t flags, int32_t otz);
void renderDigimonMoveBox(void);
int32_t getCardAmount(uint8_t card);
int32_t loadStackedTIMEntry(char *path, u_long buffer, int32_t offset,
                            int32_t sectors);
void renderNumber(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e,
                  int32_t f);
void renderDigimonMovesSelected(int16_t panel);
int32_t drawMoveViewHelpStrings(void);
int32_t drawDigimonStatsStrings(void);
void renderDigimonStatusConditions(int32_t condition);
void clearTextSubArea(RECT *rect);
void sortArray(int16_t *arr, int8_t count);
void removeStaticUIBox(int16_t id);
void drawObject(GsDOBJ2 *obj, GsOT *ot, int32_t flag);
void renderDigiviceMedals(void);
void drawString(char *str, int32_t x, int32_t y);
void renderBorderBox(int16_t x, int16_t y, int16_t w, int16_t h,
                     int32_t c1, int32_t c2, uint8_t r, uint8_t g, uint8_t b,
                     int32_t a10);
int32_t strlen(char *s);
void startAnimation(Entity *entity, uint8_t animId);
void tickAnimation(Entity *entity);
int32_t drawCardViewStrings(void);
int32_t drawMedalViewStrings(void);
void renderDigimonStatsBar(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e);
int32_t entityGetTechFromAnim(Entity *entity, int32_t anim);
int32_t drawEvoChartStrings(int32_t arg); /* MOD (portraits): was int8_t, a species can be > 127 */

static void *game_menu_functions[] = {
	drawCardViewStrings,
	drawMedalViewStrings,
	drawEvoChartStrings,
	drawPlayerInfoStrings,
	removeUIBox1,
	removeTriangleMenu,
	sortArray,
	renderDigiviceMedals,
	drawMoveViewHelpStrings,
	renderDigimonMoveBox,
	renderDigimonMovesSelected,
	drawDigimonMovesText,
	renderDigiviceEntity,
	renderDigimonStatusConditions,
	renderDigimonStatsBar,
	drawDigimonStatsStrings,
	renderBorderBox,
	renderBox,
	renderInsetBox,
	renderCardsView,
	renderMedalView,
	renderEvoChartView,
	renderPlayerInfoView,
	renderMenuTab,
	renderDigimonMovesView,
	renderDigimonStatsView,
	renderCardCount,
	renderCardImage,
	loadCardImage,
	renderEvoChartDetail,
	equipMove,
	getEquippedSlot,
	renderFeedingItem,
	removeOneSelectedItem,
	startFeedingItem,
	setSleepDisabled,
	handleGameMenuSelection,
	isUIBoxAvailable,
	renderPlayerMenu,
	tickPlayerMenu,
	renderDigimonMenu,
	tickDigimonMenu,
	createMenuBox,
	tickGameMenu,
	renderRectPolyFT4,
	renderTriangleCursor,
	renderDateDigits,
	renderSeparatorLines,
	renderGameMenu,
	closeUIBoxIfOpen,
	closeTriangleMenu,
	tickTriangleMenu,
	addGameMenu,
};

// clang-format off
uint8_t EQUIPPED_MOVES[4] = {
	0xff, 0xff, 0xff, 0xff,
};

SVECTOR MEDAL_ROTATION = { 0x0000, 0x0000, 0x0000, 0x0000 };

GsRVIEW2 DIGIVICE_ENTITY_VIEW = {
	1300, 0, -3280, 0, 0, 0, 0, NULL,
};

GsRVIEW2 MEDAL_VIEW = {
	-1050, 220, -10000, -1050, 220, 0, 0, NULL,
};

MENU_LABELS_TEXT

StatsIconClutTable STATS_VIEW_ELEMENT_CLUT = { {
	0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0b, 0x04, 0x04, 0x04, 0x0a,
} };

ConditionMaskTable MAIN_D_80123DD0 = { {
	0x00000040, 0x00000020, 0x00000004, 0x00000001,
	0x00000010, 0x00000002,
} };

char STR_OVERWORLD_EMPTY[] = "";

TriangleCursorUVData SELECTION_CURSOR_U_MIN = { { 0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x08, 0x08 } };

TriangleCursorUVData SELECTION_CURSOR_U_MAX = { { 0x04, 0x00, 0x04, 0x00, 0x08, 0x08, 0x0c, 0x0c } };

TriangleCursorUVData SELECTION_CURSOR_V_MIN = { { 0xfb, 0xfb, 0xff, 0xff, 0xfb, 0xfb, 0xfb, 0xfb } };

TriangleCursorUVData SELECTION_CURSOR_V_MAX = { { 0xff, 0xff, 0xfb, 0xfb, 0xff, 0xff, 0xff, 0xff } };

TriangleCursorOffsetData SELECTION_CURSOR_X = { { 0x00, 0x18, 0x00, 0x18, 0x04, 0x04, 0x00, 0x19 } };

TriangleCursorOffsetData SELECTION_CURSOR_Y = { { 0x00, 0x00, 0x16, 0x16, 0x00, 0x16, 0x00, 0x00 } };

TriangleCursorOffsetData SELECTION_CURSOR_WIDTH = { { 0x04, 0x04, 0x04, 0x04, 0x18, 0x18, 0x04, 0x04 } };

TriangleCursorOffsetData SELECTION_CURSOR_HEIGHT = { { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x16, 0x16 } };

/* MOD (font): the whole row the header copies (the original cleared 24 pixels; its cells overwrote the rest, the mod's
 * glyphs leave a piece's leading spaces undrawn) */
RECT MENU_TEXT_AREA = { 0, 232, 64, 12 };

YEAR_DAY_TEXT

DigimonTabs DIGIMON_MENU_VIEWS = { { 0x01, 0x01 } };

PlayerTabs MAIN_D_801342A4 = { { 1, 1, 1, 1, 1 } };

LEVEL_TITLES_TEXT

RECT DIGIMON_STATS_TEXT_AREA = { 0, 24, 256, 200 };

RECT MOVES_VIEW_TEXT_AREA = { 0, 24, 256, 200 };

/* Keep four pixels clear before the power column (x = 16). */
#define MOVE_NAME_WIDTH 120
#define FINISHER_NAME_WIDTH 136

RECT MAIN_D_801342F0 = { 0, 24, 256, 200 };

RECT PLAYER_INFO_TEXT_AREA = { 0, 12, 256, 200 };

#if VERSION_IS(US)
char FMT_PLAYTIME[8] = "%d:%02d"; /* MOD: the minutes keep two digits; the array holds no longer format */
#endif

RECT EVO_CHART_TEXT_AREA = { 0, 12, 256, 200 };

RECT EVO_CHART_NAME_AREA = { 0, 48, 200, 12 }; /* MOD (long names): was 120 wide (a name of 19 characters is ~160) */
/* MOD (long names): the status screen's name field and the chart's name, in pixels (font.c drawStringMode) */
#define STATUS_NAME_W	0x78
#define CHART_NAME_W	0x88

#if VERSION_IS(US)
/* MOD (long names): the width copied from the text area for a name drawn into the status screen's field: a name that
 * fits is copied as the original did (len * 10 pixels, cut at 0x78); a longer one was drawn tighter and is copied whole */
static int32_t modStatusNameWidth(char *name)
{
	int32_t fit;
	int32_t w;

	fit = fitStringMode(name, STATUS_NAME_W);
	if (fit == FIT_VANILLA) {
		w = strlen(name) * 10;
		return w >= 0x79 ? 0x78 : w;
	}
	w = measureStringMode(name, fit);
	return w > STATUS_NAME_W ? STATUS_NAME_W : w; /* never over the SPECIAL icons */
}
#endif

/* MOD (mega): the level name of the Megas, drawn into the text area with the
 * chart's other level names (row 0x3C holds "Ultimate"; 0x48 is free) */
#define EVO_CHART_MEGA_ROW	0x48
#define EVO_CHART_MEGA_WIDTH	0x22
static char EVO_CHART_MEGA_LABEL[] = CHART_STR_MEGA; /* MOD (language): keyed, text/main/game_menu.yaml */

RECT MEDAL_VIEW_TEXT_AREA = { 0, 12, 256, 200 };

RECT MEDAL_DETAIL_AREA = { 0, 24, 252, 48 };

RECT CARD_VIEW_TEXT_AREA = { 0, 12, 256, 200 };

EvoClutTable EVO_CHART_VIEW_COLORS = { {
	0x7a07, 0x7a47, 0x7a87, 0x7ac7, 0x7b07,
} };

CardSprites CARD_SPRITE_CLUT = { {
	0x00, 0x00, 0x04, 0x01, 0x00, 0x03, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x01, 0x04, 0x04, 0x03, 0x03, 0x04,
	0x03, 0x06, 0x01, 0x03, 0x01, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
	0x02, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x00, 0x03, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x00,
} };

EvoClutTable EVO_CHART_DETAIL_COLORS = { {
	0x7a07, 0x7a47, 0x7a87, 0x7ac7, 0x7b07,
} };

char MAIN_D_80123E78[] = "\\CARD\\CARD.ALL";

GameMenuSprite MENU_OPTIONS[8] = {
	{ 0x10, 0x1c, 1, 1, 0xc, 0x18, 0x18, 0x0, 0x30 },
	{ 0x14, 0x47, 1, 0, 0xc, 0x14, 0x14, 0x0, 0x0 },
	{ 0x38, 0x47, 2, 0, 0xd, 0x14, 0x14, 0x28, 0x0 },
	{ 0x5b, 0x47, 3, 0, 0xc, 0x14, 0x14, 0x0, 0x14 },
	{ 0x14, 0x6f, 4, 0, 0xd, 0x14, 0x14, 0x28, 0x14 },
	{ 0x38, 0x6f, 5, 0, 0xd, 0x14, 0x14, 0x0, 0x28 },
	{ 0x5b, 0x6f, 6, 0, 0xd, 0x14, 0x14, 0x28, 0x28 },
	{ 0x14, 0x1f, 7, 0, 0xd, 0x14, 0x13, 0x50, 0x0 },
};

/* MOD (language): translated labels use FI_INFO's free cells, excluding all five clock
 * frames. tools/localize/textures/fi-info/build.py prints these rows; V is TIM row + 1.
 * DIGIMON keeps the disc's label. Chinese uses SYSTEM_W's unused cells for
 * native 12px labels; its V values are direct rows and its palette uses white. */
GameMenuLabel GAME_MENU_TEXT_SPRITES[8] = {
#if MOD_LANG_BUILD == MOD_LANG_ZH
	{ 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0012, 0x005e, 0x18, 0x0d, 0x38, 0x5f },
	{ 0x0036, 0x005e, 0x18, 0x0d, 0x50, 0x60 },
	{ 0x0059, 0x005e, 0x18, 0x0d, 0x68, 0x60 },
	{ 0x0012, 0x0086, 0x18, 0x0d, 0xd0, 0x60 },
	{ 0x0036, 0x0086, 0x18, 0x0d, 0xe8, 0x60 },
	{ 0x0059, 0x0086, 0x18, 0x0d, 0xe8, 0x6e },
	{ 0x0012, 0x0036, 0x18, 0x0d, 0xb0, 0x46 },
#elif MOD_LANG_BUILD == MOD_LANG_PT_BR
	{ 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0015, 0x0060, 0x13, 0x07, 0x80, 0x31 },
	{ 0x0035, 0x0060, 0x1c, 0x07, 0x50, 0x1b },
	{ 0x0057, 0x0060, 0x1c, 0x07, 0xa0, 0x21 },
	{ 0x0012, 0x0088, 0x19, 0x07, 0x80, 0x38 },
	{ 0x0036, 0x0088, 0x19, 0x07, 0xa0, 0x2f },
	{ 0x005a, 0x0088, 0x17, 0x07, 0xa0, 0x28 },
	{ 0x0012, 0x0038, 0x18, 0x07, 0xa0, 0x38 },
#elif MOD_LANG_BUILD != MOD_LANG_EN
	{ 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 },
	{ 0x000e, 0x0060, 0x20, 0x07, 0x80, 0x31 },
	{ 0x0035, 0x0060, 0x1c, 0x07, 0x50, 0x1b },
	{ 0x0055, 0x0060, 0x20, 0x07, 0xa0, 0x21 },
	{ 0x000e, 0x0088, 0x20, 0x07, 0x80, 0x38 },
	{ 0x0032, 0x0086, 0x20, 0x09, 0xa0, 0x2f },
	{ 0x0055, 0x0088, 0x20, 0x07, 0xa0, 0x28 },
	{ 0x000e, 0x0038, 0x20, 0x07, 0xa0, 0x38 },
#else
	{ 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 },
	{ 0x0013, 0x0060, 0x18, 0x07, 0x50, 0x14 },
	{ 0x0035, 0x0060, 0x1c, 0x07, 0x50, 0x1b },
	{ 0x005a, 0x0060, 0x1c, 0x07, 0x50, 0x22 },
	{ 0x0015, 0x0088, 0x14, 0x07, 0x6c, 0x14 },
	{ 0x0039, 0x0088, 0x14, 0x07, 0x6c, 0x1b },
	{ 0x0060, 0x0088, 0x14, 0x07, 0x6c, 0x22 },
	{ 0x0017, 0x0038, 0x10, 0x07, 0x50, 0x29 },
#endif
};

Line GAME_MENU_LINES[3] = {
	{ 0xffc2, 0xffc5, 0x003e, 0xffc5, 0x00, 0x00 },
	{ 0xffc1, 0xffc6, 0x003f, 0xffc6, 0x01, 0x00 },
	{ 0xffc2, 0xffc7, 0x003e, 0xffc7, 0x00, 0x00 },
};

RGB8 UI_LINES_BORDER_COLOR[4] = {
	{ 0x02, 0x02, 0x02 },
	{ 0x90, 0xd9, 0xfa },
	{ 0x4a, 0x9f, 0xc5 },
	{ 0xb4, 0x96, 0x69 },
};

Line STATS_VIEW_LINES[6] = {
	{ 0xffe5, 0xfff1, 0xffe5, 0x0063, 0x00, 0x00 },
	{ 0xffe6, 0xfff0, 0xffe6, 0x0064, 0x01, 0x00 },
	{ 0xffe5, 0xfff1, 0xffe7, 0x0063, 0x00, 0x00 },
	{ 0xff6e, 0xffee, 0x0092, 0xffee, 0x00, 0x00 },
	{ 0xff6d, 0xffef, 0x0093, 0xffef, 0x01, 0x00 },
	{ 0xff6e, 0xfff0, 0x0092, 0xfff0, 0x00, 0x00 },
};

Line MOVES_TECHSET_LINES[6] = {
	{ 0xff6e, 0xffeb, 0x0092, 0xffeb, 0x00, 0x00 },
	{ 0xff6d, 0xffec, 0x0093, 0xffec, 0x01, 0x00 },
	{ 0xff6e, 0xffed, 0x0092, 0xffed, 0x00, 0x00 },
	{ 0xff6e, 0x0010, 0x0092, 0x0010, 0x00, 0x00 },
	{ 0xff6d, 0x0011, 0x0093, 0x0011, 0x01, 0x00 },
	{ 0xff6e, 0x0012, 0x0093, 0x0012, 0x00, 0x00 },
};

Line MOVES_VIEW_HELP_LINES[12] = {
	{ 0xff76, 0x001a, 0xffad, 0x001a, 0x00, 0x00 },
	{ 0xff76, 0x001b, 0xffad, 0x001b, 0x01, 0x00 },
	{ 0xff76, 0x001c, 0xffad, 0x001c, 0x00, 0x00 },
	{ 0xff72, 0x001e, 0xff72, 0x0059, 0x00, 0x00 },
	{ 0xff73, 0x001e, 0xff73, 0x0059, 0x01, 0x00 },
	{ 0xff74, 0x001e, 0xff74, 0x0059, 0x00, 0x00 },
	{ 0xffae, 0x001e, 0xffae, 0x0059, 0x00, 0x00 },
	{ 0xffaf, 0x001e, 0xffaf, 0x0059, 0x01, 0x00 },
	{ 0xffb0, 0x001e, 0xffb0, 0x0059, 0x00, 0x00 },
	{ 0xff76, 0x005b, 0xffad, 0x005b, 0x00, 0x00 },
	{ 0xff76, 0x005c, 0xffad, 0x005c, 0x01, 0x00 },
	{ 0xff76, 0x005d, 0xffad, 0x005d, 0x00, 0x00 },
};

IconRect MOVES_VIEW_HELP_CORNERS[4] = {
	{ 0xff72, 0x001a, 0x04, 0x04, 0x78, 0x10 },
	{ 0xffad, 0x001a, 0x04, 0x04, 0x7c, 0x10 },
	{ 0xff72, 0x0059, 0x04, 0x04, 0x78, 0x14 },
	{ 0xffad, 0x0059, 0x04, 0x04, 0x7c, 0x14 },
};

Line DIGIMON_MOVE_INFO_LINES_LEFT[18] = {
	{ 0xff9a, 0x0001, 0xffef, 0x0001, 0x00, 0x00 },
	{ 0xff9a, 0x0002, 0xffef, 0x0002, 0x01, 0x00 },
	{ 0xff9a, 0x0003, 0xffef, 0x0003, 0x00, 0x00 },
	{ 0xff6f, 0x0005, 0xff6f, 0x005c, 0x00, 0x00 },
	{ 0xff70, 0x0005, 0xff70, 0x005c, 0x01, 0x00 },
	{ 0xff71, 0x0005, 0xff71, 0x005c, 0x00, 0x00 },
	{ 0xfff1, 0x0005, 0xfff1, 0x0015, 0x00, 0x00 },
	{ 0xfff2, 0x0005, 0xfff2, 0x0015, 0x01, 0x00 },
	{ 0xfff3, 0x0005, 0xfff3, 0x0015, 0x00, 0x00 },
	{ 0xff72, 0x0016, 0xfff0, 0x0016, 0x00, 0x00 },
	{ 0xff71, 0x0017, 0xfff0, 0x0017, 0x01, 0x00 },
	{ 0xff72, 0x0018, 0xfff0, 0x0018, 0x00, 0x00 },
	{ 0xffc3, 0x0019, 0xffc3, 0x005c, 0x00, 0x00 },
	{ 0xffc4, 0x0018, 0xffc4, 0x005c, 0x01, 0x00 },
	{ 0xffc5, 0x0019, 0xffc5, 0x005c, 0x00, 0x00 },
	{ 0xff73, 0x005d, 0xffc2, 0x005d, 0x00, 0x00 },
	{ 0xff73, 0x005e, 0xffc2, 0x005e, 0x01, 0x00 },
	{ 0xff73, 0x005f, 0xffc2, 0x005f, 0x00, 0x00 },
};

Line DIGIMON_MOVE_INFO_LINES_RIGHT[18] = {
	{ 0x0037, 0x0001, 0x008d, 0x0001, 0x00, 0x00 },
	{ 0x0037, 0x0002, 0x008d, 0x0002, 0x01, 0x00 },
	{ 0x0037, 0x0003, 0x008d, 0x0003, 0x00, 0x00 },
	{ 0x000c, 0x0005, 0x000c, 0x0015, 0x00, 0x00 },
	{ 0x000d, 0x0005, 0x000d, 0x0015, 0x01, 0x00 },
	{ 0x000e, 0x0005, 0x000e, 0x0015, 0x00, 0x00 },
	{ 0x008e, 0x0005, 0x008e, 0x005c, 0x00, 0x00 },
	{ 0x008f, 0x0005, 0x008f, 0x005c, 0x01, 0x00 },
	{ 0x0090, 0x0005, 0x0090, 0x005c, 0x00, 0x00 },
	{ 0x0010, 0x0016, 0x008e, 0x0016, 0x00, 0x00 },
	{ 0x000f, 0x0017, 0x008e, 0x0017, 0x01, 0x00 },
	{ 0x0010, 0x0018, 0x008e, 0x0018, 0x00, 0x00 },
	{ 0x003a, 0x0019, 0x003a, 0x005c, 0x00, 0x00 },
	{ 0x003b, 0x0018, 0x003b, 0x005c, 0x01, 0x00 },
	{ 0x003c, 0x0019, 0x003c, 0x005c, 0x00, 0x00 },
	{ 0x003e, 0x005d, 0x008d, 0x005d, 0x00, 0x00 },
	{ 0x003e, 0x005e, 0x008d, 0x005e, 0x01, 0x00 },
	{ 0x003e, 0x005f, 0x008d, 0x005f, 0x00, 0x00 },
};

/* MOD (language): POTENCIA, ALC., EFECTO in the Spanish SYSTEM_W.TIM (tools/localize/textures/systemw/NOTES.md) */
IconRect MOVES_VIEW_TECHSET_TEXT[13] = {
#if MOD_LANG_BUILD == MOD_LANG_ES
	{ 0x000e, 0xffb4, 0x0020, 0x0007, 0x0000, 0x0029 },
	{ 0x0062, 0xffb4, 0x000f, 0x0007, 0x0020, 0x0029 },
	{ 0x0075, 0xffb4, 0x0019, 0x0007, 0x004d, 0x0029 },

#elif MOD_LANG_BUILD == MOD_LANG_PT_BR
	{ 0x000e, 0xffb4, 0x000f, 0x0007, 0x0000, 0x0029 },
	{ 0x0062, 0xffb4, 0x000f, 0x0007, 0x0020, 0x0029 },
	{ 0x0075, 0xffb4, 0x0017, 0x0007, 0x004d, 0x0029 },

#elif MOD_LANG_BUILD == MOD_LANG_FR
	{ 0x000e, 0xffb0, 0x0026, 0x000c, 0x0080, 0x006e },
	{ 0x0052, 0xffb0, 0x001c, 0x000c, 0x00a6, 0x006e },
	{ 0x0070, 0xffb0, 0x0022, 0x000c, 0x00c2, 0x006e },

#elif MOD_LANG_BUILD == MOD_LANG_DE
	{ 0x000e, 0xffb0, 0x0026, 0x000c, 0x0080, 0x006e },
	{ 0x0052, 0xffb0, 0x001c, 0x000c, 0x00a6, 0x006e },
	{ 0x0070, 0xffb0, 0x0022, 0x000c, 0x00c2, 0x006e },

#elif MOD_LANG_BUILD == MOD_LANG_IT
	{ 0x000e, 0xffb0, 0x0026, 0x000c, 0x0080, 0x006e },
	{ 0x0052, 0xffb0, 0x001c, 0x000c, 0x00a6, 0x006e },
	{ 0x0070, 0xffb0, 0x0022, 0x000c, 0x00c2, 0x006e },

#elif MOD_LANG_BUILD == MOD_LANG_JA
	{ 0x000e, 0xffb0, 0x0026, 0x000c, 0x0080, 0x006e },
	{ 0x0052, 0xffb0, 0x001c, 0x000c, 0x00a6, 0x006e },
	{ 0x0070, 0xffb0, 0x0022, 0x000c, 0x00c2, 0x006e },

#elif MOD_LANG_BUILD == MOD_LANG_CA
	{ 0x000e, 0xffb0, 0x0026, 0x000c, 0x0080, 0x006e },
	{ 0x0052, 0xffb0, 0x001c, 0x000c, 0x00a6, 0x006e },
	{ 0x0070, 0xffb0, 0x0022, 0x000c, 0x00c2, 0x006e },

#elif MOD_LANG_BUILD == MOD_LANG_ZH
	{ 0x000e, 0xffb0, 0x0026, 0x000c, 0x0080, 0x006e },
	{ 0x0052, 0xffb0, 0x001c, 0x000c, 0x00a6, 0x006e },
	{ 0x0070, 0xffb0, 0x0022, 0x000c, 0x00c2, 0x006e },

#else
	{ 0x0016, 0xffb4, 0x17, 0x07, 0x5c, 0x22 },
	{ 0x0062, 0xffb4, 0x13, 0x07, 0x4c, 0x29 },
	{ 0x007a, 0xffb4, 0x10, 0x07, 0x00, 0x30 },
#endif
	{ 0x0044, 0xffb4, 0x0b, 0x07, 0x74, 0x22 },
	{ 0x0035, 0xffc1, 0x04, 0x04, 0x78, 0x0c },
	{ 0x0035, 0xffd0, 0x04, 0x04, 0x78, 0x0c },
	{ 0x0035, 0xffe0, 0x04, 0x04, 0x78, 0x0c },
	{ 0x005e, 0xffc1, 0x04, 0x04, 0x78, 0x0c },
	{ 0x005e, 0xffd0, 0x04, 0x04, 0x78, 0x0c },
	{ 0x005e, 0xffe0, 0x04, 0x04, 0x78, 0x0c },
	{ 0x0075, 0xffc1, 0x04, 0x04, 0x78, 0x0c },
	{ 0x0075, 0xffd0, 0x04, 0x04, 0x78, 0x0c },
	{ 0x0075, 0xffe0, 0x04, 0x04, 0x78, 0x0c },
};

#if !VERSION_IS(US)
StringRect MOVES_VIEW_STRING_SPRITES[12] = {
	{ 0xff8e, 0xffbf, 0x009c, 0x00, 0x18 },
	{ 0x0042, 0xffbf, 0x0030, 0xa8, 0x18 },
	{ 0xff8e, 0xffcd, 0x0048, 0x84, 0x30 },
	{ 0xff8e, 0xffde, 0x0054, 0x00, 0x24 },
	{ 0xfffa, 0xffde, 0x0084, 0x54, 0x24 },
	{ 0xffa6, 0xffec, 0x0078, 0x54, 0x30 },
	{ 0xff9a, 0x0002, 0x00cc, 0x00, 0x3c },
	{ 0xff9a, 0x0012, 0x0078, 0x00, 0x48 },
	{ 0xff9a, 0x0022, 0x0084, 0x00, 0x54 },
	{ 0xff9a, 0x0032, 0x00a8, 0x00, 0x60 },
	{ 0xff9a, 0x0042, 0x00a8, 0x00, 0x60 },
	{ 0xff9a, 0x0052, 0x00d8, 0x00, 0x6c },
};

IconRect MOVES_VIEW_SPRITES[7] = {
	{ 0x0030, 0xffbf, 0x0c, 0x0c, 0x24, 0x00 },
	{ 0xffe8, 0xffde, 0x0c, 0x0c, 0x54, 0x00 },
	{ 0xff94, 0xffec, 0x0c, 0x0c, 0x24, 0x00 },
	{ 0xff88, 0x0002, 0x0c, 0x0c, 0x54, 0x00 },
	{ 0xff88, 0x0012, 0x0c, 0x0c, 0x24, 0x00 },
	{ 0xff88, 0x0032, 0x0c, 0x0c, 0x6c, 0x00 },
	{ 0xff88, 0x0052, 0x0c, 0x0c, 0xb4, 0x00 },
};
#else
StringRect MOVES_VIEW_STRING_SPRITES[12] = {
	{ 0xff8e, 0xffbf, 0x0073, 0x00, 0x18 },
	{ 0x000f, 0xffbf, 0x006e, 0x71, 0x18 },
	{ 0xff8e, 0xffcd, 0x0000, 0xdf, 0x18 },
	{ 0xff8e, 0xffde, 0x000c, 0x00, 0x24 },
	{ 0xff9a, 0xffde, 0x00be, 0x0c, 0x24 },
	{ 0xffa6, 0xffec, 0x0000, 0x54, 0x30 },
	{ 0xff9a, 0x0002, 0x0064, 0x00, 0x3c },
	{ 0xff9a, 0x0012, 0x0064, 0x00, 0x48 },
	{ 0xff9a, 0x0022, 0x0064, 0x00, 0x54 },
	{ 0xff9a, 0x0032, 0x0064, 0x00, 0x60 },
	{ 0xff9a, 0x0042, 0x0064, 0x00, 0x60 },
	{ 0xff9a, 0x0052, 0x0064, 0x00, 0x6c },
};

IconRect MOVES_VIEW_SPRITES[7] = {
	{ 0x0005, 0xffbf, 0x0c, 0x0c, 0x24, 0x00 },
	{ 0x0052, 0xffde, 0x0c, 0x0c, 0x54, 0x00 },
	{ 0xffbe, 0xffec, 0x0c, 0x0c, 0x24, 0x00 },
	{ 0xff88, 0x0002, 0x0c, 0x0c, 0x54, 0x00 },
	{ 0xff88, 0x0012, 0x0c, 0x0c, 0x24, 0x00 },
	{ 0xff88, 0x0032, 0x0c, 0x0c, 0x6c, 0x00 },
	{ 0xff88, 0x0052, 0x0c, 0x0c, 0xb4, 0x00 },
};
#endif

RECT STATS_VIEW_INSETS[13] = {
	{ 50, 52, 98, 14 },
	{ 50, 68, 244, 14 },
	{ 50, 84, 26, 14 },
	{ 122, 84, 26, 14 },
	{ 75, 171, 50, 8 },
	{ 75, 186, 50, 8 },
	{ 75, 201, 50, 8 },
	{ 194, 130, 104, 5 },
	{ 194, 145, 104, 5 },
	{ 194, 160, 54, 5 },
	{ 194, 175, 54, 5 },
	{ 194, 190, 54, 5 },
	{ 194, 205, 54, 5 },
};

#if !VERSION_IS(US)
StringRect STATS_VIEW_TEXT[9] = {
	{ 0xff76, 0x0022, 0x0030, 0x48, 0x30 },
	{ 0xff82, 0x0040, 0x0024, 0x18, 0x30 },
	{ 0xff82, 0x004f, 0x0024, 0x78, 0x30 },
	{ 0xfffc, 0x0004, 0x0018, 0x9c, 0x30 },
	{ 0xfffc, 0x0013, 0x0018, 0xb4, 0x30 },
	{ 0xfffc, 0x0022, 0x0024, 0x00, 0x24 },
	{ 0xfffc, 0x0031, 0x0024, 0x24, 0x24 },
	{ 0xffaf, 0xffdd, 0x000c, 0x00, 0x30 },
	{ 0xfff6, 0xffdd, 0x000c, 0x0c, 0x30 },
};
#else
StringRect STATS_VIEW_TEXT[9] = {
	{ 0xff76, 0x0022, 0x0030, 0x48, 0x30 },
	{ 0xff82, 0x0040, 0x0024, 0x18, 0x30 },
	{ 0xff82, 0x004f, 0x0024, 0x78, 0x30 },
	{ 0xfffc, 0x0004, 0x0018, 0x9c, 0x30 },
	{ 0xfffc, 0x0013, 0x0018, 0xb4, 0x30 },
	{ 0xfffc, 0x0022, 0x0024, 0x00, 0x24 },
	{ 0xfffc, 0x0031, 0x0024, 0x24, 0x24 },
	{ 0xffaf, 0xffdd, 0x0000, 0x00, 0x30 },
	{ 0xfff6, 0xffdd, 0x0000, 0x00, 0x30 },
};
#endif

/* MOD (language): the Spanish labels in SYSTEM_W.TIM's Spanish copy (tools/localize/textures/systemw/NOTES.md):
 * NOMBRE, ESPECIE, PESO, EDAD, ESPECIAL, FELICIDAD, VELOCIDAD, INTELECTO */
IconRect STATS_VIEW_ELEMENTS[21] = {
#if MOD_LANG_BUILD == MOD_LANG_ES
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x001f, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x001e, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0014, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x0024, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x0027, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#elif MOD_LANG_BUILD == MOD_LANG_PT_BR
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x0015, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x000f, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0012, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x001c, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x001f, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#elif MOD_LANG_BUILD == MOD_LANG_FR
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x001f, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x001e, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0014, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x0024, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x0027, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#elif MOD_LANG_BUILD == MOD_LANG_DE
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x001f, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x001e, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0014, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x0024, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x0027, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#elif MOD_LANG_BUILD == MOD_LANG_IT
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x001f, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x001e, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0014, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x0024, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x0027, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#elif MOD_LANG_BUILD == MOD_LANG_JA
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x001f, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x001e, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0014, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x0024, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x0027, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#elif MOD_LANG_BUILD == MOD_LANG_CA
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x001f, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x001e, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0014, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x0024, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x0027, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#elif MOD_LANG_BUILD == MOD_LANG_ZH
	{ 0xff74, 0xffaf, 0x0035, 0x000b, 0x0000, 0x004b },
	{ 0xff74, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0056 },
	{ 0xffee, 0xfff4, 0x0035, 0x000b, 0x0000, 0x0061 },
	{ 0xff71, 0xffbe, 0x001f, 0x000a, 0x0080, 0x0030 },
	{ 0xff71, 0xffce, 0x001e, 0x000a, 0x009f, 0x0030 },
	{ 0xffc1, 0xffde, 0x0014, 0x000a, 0x00bd, 0x0030 },
	{ 0xff79, 0xffde, 0x0014, 0x000a, 0x00d1, 0x0030 },
	{ 0x0034, 0xffc2, 0x0019, 0x0008, 0x0000, 0x0038 },
	{ 0x0009, 0xffc2, 0x001d, 0x0008, 0x0090, 0x0048 },
	{ 0x0055, 0xffc2, 0x0019, 0x0008, 0x0032, 0x0038 },
	{ 0xff77, 0x004f, 0x0009, 0x000b, 0x00f0, 0x0025 },
	{ 0xffef, 0x0004, 0x000b, 0x000b, 0x0093, 0x003a },
	{ 0xffef, 0x0013, 0x000b, 0x000b, 0x009e, 0x003a },
	{ 0xffef, 0x0022, 0x000b, 0x000b, 0x00a9, 0x003a },
	{ 0xffef, 0x0031, 0x000b, 0x000b, 0x00b4, 0x003a },
	{ 0xffef, 0x0040, 0x000b, 0x000b, 0x00bf, 0x003a },
	{ 0xffef, 0x004f, 0x000a, 0x000b, 0x00ca, 0x003a },
	{ 0xff83, 0x0031, 0x0024, 0x000c, 0x00d4, 0x003a },
	{ 0xfffb, 0x0040, 0x0027, 0x000c, 0x00d4, 0x0046 },
	{ 0xfffb, 0x004f, 0x0025, 0x000c, 0x00d4, 0x0052 },
	{ 0xff76, 0x004f, 0x0009, 0x000b, 0x002c, 0x0074 },

#else
	{ 0xff74, 0xffaf, 0x35, 0x0b, 0x00, 0x4b },
	{ 0xff74, 0xfff4, 0x35, 0x0b, 0x00, 0x56 },
	{ 0xffee, 0xfff4, 0x35, 0x0b, 0x00, 0x61 },
	{ 0xff76, 0xffbe, 0x1a, 0x0a, 0x80, 0x30 },
	{ 0xff76, 0xffce, 0x1a, 0x0a, 0x9a, 0x30 },
	{ 0xffbd, 0xffde, 0x1c, 0x0a, 0xb4, 0x30 },
	{ 0xff7b, 0xffde, 0x13, 0x0a, 0x80, 0x3a },
	{ 0x0034, 0xffc2, 0x19, 0x08, 0x00, 0x38 },
	{ 0x0009, 0xffc2, 0x19, 0x08, 0x19, 0x38 },
	{ 0x0055, 0xffc2, 0x19, 0x08, 0x32, 0x38 },
	{ 0xff77, 0x004f, 0x09, 0x0b, 0xf0, 0x25 },
	{ 0xffef, 0x0004, 0x0b, 0x0b, 0x93, 0x3a },
	{ 0xffef, 0x0013, 0x0b, 0x0b, 0x9e, 0x3a },
	{ 0xffef, 0x0022, 0x0b, 0x0b, 0xa9, 0x3a },
	{ 0xffef, 0x0031, 0x0b, 0x0b, 0xb4, 0x3a },
	{ 0xffef, 0x0040, 0x0b, 0x0b, 0xbf, 0x3a },
	{ 0xffef, 0x004f, 0x0a, 0x0b, 0xca, 0x3a },
	/* MOD (original bug): HAPPINESS is 41 px in SYSTEM_W.TIM (u 214..254); the original drew u 212..247 at -126: a
	   stray mark at u 212 before the H and the last S cut off */
	{ 0xff81, 0x0031, 0x29, 0x0c, 0xd6, 0x3a },
	{ 0xfffc, 0x0040, 0x24, 0x0c, 0xd4, 0x46 },
	{ 0xfffc, 0x004f, 0x24, 0x0c, 0xd4, 0x52 },
	{ 0xff76, 0x004f, 0x09, 0x0b, 0x2c, 0x74 },
#endif
};

/* MOD (language): the move-help screen's element names, one per row (posX: the left column's x, the right column is at
 * 100; posY unused; texY with the 0x80 of the texture's v). The Spanish words are not all 20 wide (COMBATE, TIERRA,
 * MUGRE): each has its own cell and the left column is right-aligned, 3 px before the grid */
IconRect MOVES_VIEW_ELEMENT_NAMES[7] = {
#if MOD_LANG_BUILD == MOD_LANG_ES
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#elif MOD_LANG_BUILD == MOD_LANG_PT_BR
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#elif MOD_LANG_BUILD == MOD_LANG_FR
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#elif MOD_LANG_BUILD == MOD_LANG_DE
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#elif MOD_LANG_BUILD == MOD_LANG_IT
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#elif MOD_LANG_BUILD == MOD_LANG_JA
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#elif MOD_LANG_BUILD == MOD_LANG_CA
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#elif MOD_LANG_BUILD == MOD_LANG_ZH
	{ 0xffbf, 0x0000, 0x0014, 0x000a, 0x0000, 0x0098 },
	{ 0xffb5, 0x0000, 0x001e, 0x000a, 0x0014, 0x0098 },
	{ 0xffc5, 0x0000, 0x000e, 0x000a, 0x0032, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x0040, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0056, 0x0098 },
	{ 0xffc1, 0x0000, 0x0012, 0x000a, 0x0068, 0x0098 },
	{ 0xffbd, 0x0000, 0x0016, 0x000a, 0x003c, 0x00e0 },

#else
	{ 0xffbd, 0x0000, 0x14, 0x0a, 0x00, 0x98 },
	{ 0xffbd, 0x0000, 0x14, 0x0a, 0x14, 0x98 },
	{ 0xffbd, 0x0000, 0x14, 0x0a, 0x28, 0x98 },
	{ 0xffbd, 0x0000, 0x14, 0x0a, 0x3c, 0x98 },
	{ 0xffbd, 0x0000, 0x14, 0x0a, 0x50, 0x98 },
	{ 0xffbd, 0x0000, 0x14, 0x0a, 0x64, 0x98 },
	{ 0xffbd, 0x0000, 0x14, 0x0a, 0x78, 0x98 },
#endif
};

/* MOD (language): the move panel's labels (TECH. DATA, POWER, MP, RANG, SPEC; posX is added to the panel's x, texY
 * with the 0x80 of the texture's v); Spanish DATOS TEC. (9 rows for the accent, 2 px higher), POTENCIA, MP, ALCANCE,
 * EFECTO */
IconRect MOVES_SELECTED_LABELS[5] = {
#if MOD_LANG_BUILD == MOD_LANG_ES
	{ 0xff74, 0xfffc, 0x0025, 0x0009, 0x0090, 0x00e0 },
	{ 0xff74, 0x001d, 0x0020, 0x0007, 0x0000, 0x00a9 },
	{ 0xff74, 0x002e, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003f, 0x001e, 0x0007, 0x002f, 0x00a9 },
	{ 0xff74, 0x0050, 0x0019, 0x0007, 0x004d, 0x00a9 },

#elif MOD_LANG_BUILD == MOD_LANG_PT_BR
	{ 0xff74, 0xfffc, 0x0025, 0x0009, 0x0090, 0x00e0 },
	{ 0xff74, 0x001d, 0x000f, 0x0007, 0x0000, 0x00a9 },
	{ 0xff74, 0x002e, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003f, 0x001e, 0x0007, 0x002f, 0x00a9 },
	{ 0xff74, 0x0050, 0x0017, 0x0007, 0x004d, 0x00a9 },

#elif MOD_LANG_BUILD == MOD_LANG_FR
	{ 0xff74, 0xfffa, 0x0040, 0x000c, 0x0060, 0x00d4 },
	{ 0xff74, 0x001b, 0x0026, 0x000c, 0x0080, 0x00ee },
	{ 0xff74, 0x002c, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003d, 0x0026, 0x000c, 0x00a0, 0x00d4 },
	{ 0xff74, 0x004e, 0x0022, 0x000c, 0x00c2, 0x00ee },

#elif MOD_LANG_BUILD == MOD_LANG_DE
	{ 0xff74, 0xfffa, 0x0040, 0x000c, 0x0060, 0x00d4 },
	{ 0xff74, 0x001b, 0x0026, 0x000c, 0x0080, 0x00ee },
	{ 0xff74, 0x002c, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003d, 0x0026, 0x000c, 0x00a0, 0x00d4 },
	{ 0xff74, 0x004e, 0x0022, 0x000c, 0x00c2, 0x00ee },

#elif MOD_LANG_BUILD == MOD_LANG_IT
	{ 0xff74, 0xfffa, 0x0040, 0x000c, 0x0060, 0x00d4 },
	{ 0xff74, 0x001b, 0x0026, 0x000c, 0x0080, 0x00ee },
	{ 0xff74, 0x002c, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003d, 0x0026, 0x000c, 0x00a0, 0x00d4 },
	{ 0xff74, 0x004e, 0x0022, 0x000c, 0x00c2, 0x00ee },

#elif MOD_LANG_BUILD == MOD_LANG_JA
	{ 0xff74, 0xfffa, 0x0040, 0x000c, 0x0060, 0x00d4 },
	{ 0xff74, 0x001b, 0x0026, 0x000c, 0x0080, 0x00ee },
	{ 0xff74, 0x002c, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003d, 0x0026, 0x000c, 0x00a0, 0x00d4 },
	{ 0xff74, 0x004e, 0x0022, 0x000c, 0x00c2, 0x00ee },

#elif MOD_LANG_BUILD == MOD_LANG_CA
	{ 0xff74, 0xfffa, 0x0040, 0x000c, 0x0060, 0x00d4 },
	{ 0xff74, 0x001b, 0x0026, 0x000c, 0x0080, 0x00ee },
	{ 0xff74, 0x002c, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003d, 0x0026, 0x000c, 0x00a0, 0x00d4 },
	{ 0xff74, 0x004e, 0x0022, 0x000c, 0x00c2, 0x00ee },

#elif MOD_LANG_BUILD == MOD_LANG_ZH
	{ 0xff74, 0xfffa, 0x0040, 0x000c, 0x0060, 0x00d4 },
	{ 0xff74, 0x001b, 0x0026, 0x000c, 0x0080, 0x00ee },
	{ 0xff74, 0x002c, 0x000b, 0x0007, 0x0074, 0x00a2 },
	{ 0xff74, 0x003d, 0x0026, 0x000c, 0x00a0, 0x00d4 },
	{ 0xff74, 0x004e, 0x0022, 0x000c, 0x00c2, 0x00ee },

#else
	{ 0xff74, 0xfffe, 0x25, 0x07, 0x11, 0xb0 },
	{ 0xff74, 0x001d, 0x17, 0x07, 0x5c, 0xa2 },
	{ 0xff74, 0x002e, 0x0b, 0x07, 0x74, 0xa2 },
	{ 0xff74, 0x003f, 0x13, 0x07, 0x4c, 0xa9 },
	{ 0xff74, 0x0050, 0x10, 0x07, 0x00, 0xb0 },
#endif
};

Line TAMER_VIEW_PLAYER_LINES[11] = {
	{ 0xff6e, 0xffbf, 0x0092, 0xffbf, 0x00, 0x00 },
	{ 0xff6d, 0xffc0, 0x0093, 0xffc0, 0x01, 0x00 },
	{ 0xff6e, 0xffc1, 0x0092, 0xffc1, 0x00, 0x00 },
	{ 0xffd3, 0xffc1, 0x0092, 0xffc1, 0x00, 0x00 },
	{ 0xffd1, 0xffc1, 0xffd1, 0x0062, 0x00, 0x00 },
	{ 0xffd2, 0xffc0, 0xffd2, 0x0063, 0x01, 0x00 },
	{ 0xffd3, 0xffc1, 0xffd3, 0x002b, 0x00, 0x00 },
	{ 0xffd2, 0x002c, 0x0091, 0x002c, 0x00, 0x00 },
	{ 0xffd3, 0x002d, 0x0093, 0x002d, 0x01, 0x00 },
	{ 0xffd3, 0x002e, 0x0091, 0x002e, 0x00, 0x00 },
	{ 0xffd3, 0x002f, 0xffd3, 0x0064, 0x00, 0x00 },
};

RECT TAMER_WINDOW_BOXES[11] = {
	{ 74, 38, 75, 14 },
	{ 211, 64, 27, 14 },
	{ 211, 101, 27, 14 },
	{ 211, 195, 27, 14 },
	{ 198, 82, 99, 14 },
	{ 199, 120, 75, 14 },
	{ 199, 140, 75, 14 },
	{ 211, 174, 18, 18 },
	{ 231, 174, 18, 18 },
	{ 251, 174, 18, 18 },
	{ 271, 174, 18, 18 },
};

#if !VERSION_IS(US)
StringRect TAMER_WINDOW_NAME_STRING[11] = {
	{ 0xff88, 0xffaf, 0x0018, 0x00, 0x0c },
	{ 0xffdb, 0xffca, 0x0054, 0x24, 0x0c },
	{ 0xffdb, 0xffef, 0x0054, 0x78, 0x0c },
	{ 0xffdb, 0x0002, 0x0048, 0x00, 0x18 },
	{ 0xffdb, 0x0016, 0x0048, 0x60, 0x18 },
	{ 0xffdb, 0x0039, 0x0054, 0x00, 0x24 },
	{ 0xffdb, 0x004d, 0x0048, 0x54, 0x24 },
	{ 0x0050, 0xffef, 0x0018, 0xcc, 0x0c },
	{ 0x0074, 0x0002, 0x0018, 0x48, 0x18 },
	{ 0x004d, 0x0016, 0x000c, 0xa8, 0x18 },
	{ 0x0050, 0x004d, 0x000c, 0x9c, 0x24 },
};
#else
/* MOD (language): Spanish Nombre is wider than Name (taken further left) and Nivel starts further (PLAYER_LABEL_PIECES) */
StringRect TAMER_WINDOW_NAME_STRING[11] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ 0xff76, 0xffaf, 0x0034, 0x00, 0x0c },
	{ 0xffdb, 0xffca, 0x0040, 0x38, 0x0c },
#else
	{ 0xff84, 0xffaf, 0x0024, 0x00, 0x0c },
	{ 0xffdb, 0xffca, 0x0054, 0x24, 0x0c },
#endif
	{ 0xffdb, 0xffef, 0x0054, 0x78, 0x0c },
	{ 0xffdb, 0x0002, 0x0048, 0x00, 0x18 },
	{ 0xffdb, 0x0016, 0x0048, 0x60, 0x18 },
	{ 0xffdb, 0x0039, 0x0054, 0x00, 0x24 },
	{ 0xffdb, 0x004d, 0x0048, 0x54, 0x24 },
	{ 0x0050, 0xffef, 0x0018, 0xcc, 0x0c },
	{ 0x0074, 0x0002, 0x0018, 0x48, 0x18 },
	{ 0x004d, 0x0016, 0x000c, 0xa8, 0x18 },
	{ 0x0050, 0x004d, 0x000c, 0x9c, 0x24 },
};
#endif

EvoChartEntry EVO_CHART_BOXES[NUM_EVO_CHART_DIGIMON] = {
	{ 0x001e, 0x002b, 0x40, 0x90, 0x00, 0x00 },
	{ 0x0043, 0x002b, 0x60, 0x90, 0x00, 0x00 },
	{ 0x0068, 0x002b, 0x40, 0x80, 0x00, 0x00 },
	{ 0x0068, 0x00b0, 0x60, 0x80, 0x01, 0x00 },
	{ 0x008d, 0x002b, 0x40, 0x30, 0x01, 0x00 },
	{ 0x00d5, 0x003e, 0x60, 0x30, 0x03, 0x00 },
	{ 0x008d, 0x0064, 0x80, 0x30, 0x01, 0x00 },
	{ 0x00bd, 0x003e, 0xa0, 0x30, 0x00, 0x00 },
	{ 0x00a5, 0x002b, 0xc0, 0x30, 0x00, 0x00 },
	{ 0x00d5, 0x0077, 0xe0, 0x30, 0x01, 0x00 },
	{ 0x00bd, 0x009d, 0xa0, 0x70, 0x01, 0x00 },
	{ 0x00fa, 0x002b, 0xc0, 0x00, 0x00, 0x00 },
	{ 0x0112, 0x0077, 0xe0, 0x00, 0x00, 0x00 },
	{ 0x0112, 0x008a, 0x00, 0x10, 0x00, 0x00 },
	{ 0x001e, 0x0077, 0x80, 0x90, 0x00, 0x00 },
	{ 0x0043, 0x0077, 0xa0, 0x90, 0x00, 0x00 },
	{ 0x0068, 0x003e, 0x80, 0x80, 0x02, 0x00 },
	{ 0x0068, 0x0064, 0xa0, 0x80, 0x00, 0x00 },
	{ 0x00d5, 0x0064, 0x00, 0x40, 0x00, 0x00 },
	{ 0x00bd, 0x0051, 0x20, 0x40, 0x00, 0x00 },
	{ 0x00bd, 0x002b, 0x40, 0x40, 0x00, 0x00 },
	{ 0x00d5, 0x008a, 0x60, 0x40, 0x02, 0x00 },
	{ 0x008d, 0x009d, 0x80, 0x40, 0x00, 0x00 },
	{ 0x008d, 0x008a, 0xa0, 0x40, 0x00, 0x00 },
	{ 0x00a5, 0x0077, 0xc0, 0x70, 0x01, 0x00 },
	{ 0x00fa, 0x003e, 0x20, 0x10, 0x00, 0x00 },
	{ 0x00fa, 0x0077, 0x40, 0x10, 0x00, 0x00 },
	{ 0x00fa, 0x009d, 0x60, 0x10, 0x00, 0x00 },
	{ 0x001e, 0x0051, 0xc0, 0x90, 0x02, 0x00 },
	{ 0x0043, 0x0051, 0xe0, 0x90, 0x00, 0x00 },
	{ 0x0068, 0x0051, 0xc0, 0x80, 0x00, 0x00 },
	{ 0x0068, 0x008a, 0xe0, 0x80, 0x00, 0x00 },
	{ 0x00bd, 0x0064, 0xc0, 0x40, 0x00, 0x00 },
	{ 0x008d, 0x0051, 0xe0, 0x40, 0x01, 0x00 },
	{ 0x00a5, 0x008a, 0x00, 0x50, 0x02, 0x00 },
	{ 0x00d5, 0x002b, 0x20, 0x50, 0x00, 0x00 },
	{ 0x00d5, 0x0051, 0x40, 0x50, 0x00, 0x00 },
	{ 0x00a5, 0x003e, 0x60, 0x50, 0x00, 0x00 },
	{ 0x00d5, 0x009d, 0xe0, 0x70, 0x00, 0x00 },
	{ 0x0112, 0x002b, 0x80, 0x10, 0x03, 0x00 },
	{ 0x00fa, 0x0051, 0xa0, 0x10, 0x00, 0x00 },
	{ 0x00fa, 0x00b0, 0xc0, 0x10, 0x00, 0x00 },
	{ 0x001e, 0x009d, 0x00, 0xa0, 0x00, 0x00 },
	{ 0x0043, 0x009d, 0x20, 0xa0, 0x01, 0x00 },
	{ 0x0068, 0x0077, 0x00, 0x90, 0x02, 0x00 },
	{ 0x0068, 0x009d, 0x20, 0x90, 0x02, 0x00 },
	{ 0x008d, 0x003e, 0x80, 0x50, 0x03, 0x00 },
	{ 0x00a5, 0x0051, 0xa0, 0x50, 0x03, 0x00 },
	{ 0x00bd, 0x008a, 0xc0, 0x50, 0x03, 0x00 },
	{ 0x00a5, 0x0064, 0xe0, 0x50, 0x00, 0x00 },
	{ 0x008d, 0x0077, 0x00, 0x60, 0x00, 0x00 },
	{ 0x00a5, 0x009d, 0x20, 0x60, 0x00, 0x00 },
	{ 0x008d, 0x00b0, 0x00, 0x80, 0x00, 0x00 },
	{ 0x0112, 0x003e, 0xe0, 0x10, 0x00, 0x00 },
	{ 0x0112, 0x0064, 0x00, 0x20, 0x00, 0x00 },
	{ 0x0112, 0x009d, 0x20, 0x20, 0x01, 0x00 },
	{ 0x0068, 0x00c3, 0x60, 0x60, 0x00, 0x00 },
	{ 0x00bd, 0x0077, 0x40, 0x60, 0x00, 0x00 },
	{ 0x0112, 0x0051, 0x20, 0x00, 0x00, 0x00 },
	{ 0x00fa, 0x0064, 0x40, 0x00, 0x04, 0x00 },
	{ 0x00fa, 0x008a, 0x60, 0x00, 0x03, 0x00 },
};

Line MAIN_D_8012472C[14] = {
	{ 0xff6e, 0xffbd, 0x0092, 0xffbd, 0x00, 0x00 },
	{ 0xff6d, 0xffbe, 0x0093, 0xffbe, 0x01, 0x00 },
	{ 0xff6e, 0xffbf, 0x0092, 0xffbf, 0x00, 0x00 },
	{ 0x0047, 0xffbf, 0x0092, 0xffbf, 0x00, 0x00 },
	{ 0x0045, 0xffc0, 0x0045, 0x000c, 0x00, 0x00 },
	{ 0x0046, 0xffbf, 0x0046, 0x000f, 0x01, 0x00 },
	{ 0x0047, 0xffc0, 0x0047, 0x000e, 0x00, 0x00 },
	{ 0xff6e, 0x000e, 0x0046, 0x000e, 0x00, 0x00 },
	{ 0x0047, 0x000e, 0x0092, 0x000e, 0x00, 0x00 },
	{ 0xff6d, 0x000f, 0x0093, 0x000f, 0x01, 0x00 },
	{ 0xff6e, 0x0010, 0x0092, 0x0010, 0x00, 0x00 },
	{ 0xff6d, 0x004f, 0x0091, 0x004f, 0x00, 0x00 },
	{ 0xff6c, 0x0050, 0x0092, 0x0050, 0x01, 0x00 },
	{ 0xff6d, 0x0051, 0x0091, 0x0051, 0x00, 0x00 },
};

char *STATUS_VIEW_LABELS[8] = {
	MAIN_D_801238A0,
	MAIN_D_801238B0,
	MAIN_D_801238D8,
	MAIN_D_80123900,
	MAIN_D_80123914,
	MAIN_D_80123920,
	MAIN_D_80123938,
	MAIN_D_8012394C,
};

char *TECH_VIEW_LABELS[10] = {
	MAIN_D_80123958,
	MAIN_D_80123970,
	MAIN_D_80123990,
	MAIN_D_801239AC,
	MAIN_D_80134240,
	MAIN_D_801239C4,
	MAIN_D_801239D0,
	MAIN_D_801239DC,
	MAIN_D_801239E8,
	MAIN_D_801239F4,
};

char *PLAYER_VIEW_LABELS[5] = {
	MAIN_D_80123A18,
	MAIN_D_80123A3C,
	MAIN_D_80123A60,
	MAIN_D_80123A7C,
	MAIN_D_80123A98,
};

char *CARD_CHART_LABELS[5] = {
	MAIN_D_80123AB0,
	MAIN_D_80123AD0,
	MAIN_D_80123AE4,
	MAIN_D_80123B10,
	MAIN_D_80123B20,
};

#if FONT_SWITCH
/* MOD (font): the crops of the player screen's label rows ("NameLevel  Raised", "Bits  Time", "Items  Medals"):
 * drawStringPieces lays each piece out in the font's spacing (dw/mod_font.h). MOD (language): Spanish "Nombre" is
 * wider than Name's crop, so Nivel starts further (TAMER_WINDOW_NAME_STRING) */
uint16_t PLAYER_LABEL_PIECES[3][6] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ 0, 0x38, 0x78, 0xcc, PIECES_END },
#else
	{ 0, 0x24, 0x78, 0xcc, PIECES_END },
#endif
	{ 0, 0x48, 0x60, 0xa8, 0xb4, PIECES_END },
	{ 0, 0x54, 0x9c, PIECES_END },
};
#endif

/*
 * MOD (language): the label rows drawn once and cut at fixed pixels (text/ours/README.md "Left as built") whose words
 * do not fit the US crops in Spanish: each language's crops (x on the screen, the piece's u and width in the text area)
 * and pieces (DRAW_STRING_PIECES_AT). The text of each row is laid out for them (text/es/names.json: every word starts
 * at its piece's offset in the original font's cells). The English values are the original's literals.
 */
typedef struct {
	int16_t x;
	uint8_t u;
	uint8_t w;
} LabelCrop;

/* The localized technique tab keeps its text inside the tab border. */
LabelCrop DIGIMON_TAB_CROPS[2] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ -0x8a, 0x00, 0x3c },
	{ -0x44, 0x3c, 0x3c },
#else
	{ -0x8a, 0x00, 0x3c },
	{ -0x3f, 0x3c, 0x30 },
#endif
};

/* Five Player tabs, measured in both fonts in every language. */
LabelCrop PLAYER_TAB_CROPS[5] = {
#if MOD_LANG_BUILD == MOD_LANG_EN
	{ -134, 0, 48 },
	{ -69, 48, 44 },
	{ -12, 92, 32 },
	{ 37, 124, 44 },
	{ 102, 168, 36 },
#elif MOD_LANG_BUILD == MOD_LANG_ES
	{ -140, 0, 60 },
	{ -67, 60, 40 },
	{ -12, 100, 32 },
	{ 33, 132, 52 },
	{ 94, 184, 52 },
#elif MOD_LANG_BUILD == MOD_LANG_PT_BR
	{ -140, 0, 60 },
	{ -63, 60, 32 },
	{ -12, 92, 32 },
	{ 33, 124, 52 },
	{ 94, 176, 52 },
#elif MOD_LANG_BUILD == MOD_LANG_FR
	{ -136, 0, 52 },
	{ -67, 52, 40 },
	{ -12, 92, 32 },
	{ 33, 124, 52 },
	{ 102, 176, 36 },
#elif MOD_LANG_BUILD == MOD_LANG_DE
	{ -136, 0, 52 },
	{ -67, 52, 40 },
	{ -12, 92, 32 },
	{ 33, 124, 52 },
	{ 96, 176, 48 },
#elif MOD_LANG_BUILD == MOD_LANG_IT
	{ -136, 0, 52 },
	{ -67, 52, 40 },
	{ -12, 92, 32 },
	{ 37, 124, 44 },
	{ 98, 168, 44 },
#elif MOD_LANG_BUILD == MOD_LANG_JA
	{ -122, 0, 24 },
	{ -65, 24, 36 },
	{ -14, 60, 36 },
	{ 41, 96, 36 },
	{ 108, 132, 24 },
#elif MOD_LANG_BUILD == MOD_LANG_ZH
	{ -122, 0, 24 },
	{ -65, 24, 36 },
	{ -14, 60, 36 },
	{ 41, 96, 36 },
	{ 108, 132, 24 },
#elif MOD_LANG_BUILD == MOD_LANG_CA
	{ -140, 0, 60 },
	{ -67, 60, 40 },
	{ -12, 100, 32 },
	{ 33, 132, 52 },
	{ 94, 184, 52 },
#endif
};

uint16_t PLAYER_TAB_PIECES[6] = {
#if MOD_LANG_BUILD == MOD_LANG_EN
	0 | PIECE_CENTRE, 48 | PIECE_CENTRE, 92 | PIECE_CENTRE, 124 | PIECE_CENTRE, 168 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_ES
	0 | PIECE_CENTRE, 60 | PIECE_CENTRE, 100 | PIECE_CENTRE, 132 | PIECE_CENTRE, 184 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_PT_BR
	0 | PIECE_CENTRE, 60 | PIECE_CENTRE, 92 | PIECE_CENTRE, 124 | PIECE_CENTRE, 176 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_FR
	0 | PIECE_CENTRE, 52 | PIECE_CENTRE, 92 | PIECE_CENTRE, 124 | PIECE_CENTRE, 176 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_DE
	0 | PIECE_CENTRE, 52 | PIECE_CENTRE, 92 | PIECE_CENTRE, 124 | PIECE_CENTRE, 176 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_IT
	0 | PIECE_CENTRE, 52 | PIECE_CENTRE, 92 | PIECE_CENTRE, 124 | PIECE_CENTRE, 168 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_JA
	0 | PIECE_CENTRE, 24 | PIECE_CENTRE, 60 | PIECE_CENTRE, 96 | PIECE_CENTRE, 132 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_ZH
	0 | PIECE_CENTRE, 24 | PIECE_CENTRE, 60 | PIECE_CENTRE, 96 | PIECE_CENTRE, 132 | PIECE_CENTRE, PIECES_END,
#elif MOD_LANG_BUILD == MOD_LANG_CA
	0 | PIECE_CENTRE, 60 | PIECE_CENTRE, 100 | PIECE_CENTRE, 132 | PIECE_CENTRE, 184 | PIECE_CENTRE, PIECES_END,
#endif
};

/* the technique screen's titles ("TechsetFinal  Techs"): Equipadas, Final, Técnicas */
LabelCrop TECH_TITLE_CROPS[3] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ -0x8e, 0x00, 0x48 },
	{ -0x8e, 0x48, 0x24 },
	{ -0x8e, 0x6c, 0x40 },
#else
	{ -0x8e, 0x00, 0x3c },
	{ -0x8e, 0x3c, 0x24 },
	{ -0x8e, 0x60, 0x3c },
#endif
};

uint16_t TECH_TITLE_PIECES[4] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	0, 0x48, 0x6c, PIECES_END,
#else
	0, 0x3c, 0x60, PIECES_END,
#endif
};

/* the technique grid's "Tech" over "help" (the Digimon tab's word): Técnicas is wider, centred over ayuda */
LabelCrop TECH_GRID_TITLE_CROP = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	-0x95, 0x3c, 0x40,
#else
	-0x87, 0x3c, 0x30,
#endif
};

/* the technique grid's "help" label: ayuda is wider */
LabelCrop TECH_HELP_CROP = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	-0x81, 0x00, 0x28,
#else
	-0x81, 0x00, 0x24,
#endif
};

/* the technique panel's status effects ("poisconfstunflat", the row drawn at MOVE_STATUS_ROW_X; x unused): the Spanish
 * words need the row to start further left (its last glyph must start before 244, drawGlyph's limit) */
LabelCrop MOVE_STATUS_CROPS[4] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ 0, 0x40, 0x1c },
	{ 0, 0x60, 0x2c },
	{ 0, 0x8c, 0x30 },
	{ 0, 0xbc, 0x28 },
#else
	{ 0, 0x84, 0x1a },
	{ 0, 0xa0, 0x1c },
	{ 0, 0xbc, 0x20 },
	{ 0, 0xdb, 0x1e },
#endif
};

int16_t MOVE_STATUS_ROW_X = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	0x40,
#else
	0x84,
#endif
};

uint16_t MOVE_STATUS_PIECES[5] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	0, 0x20, 0x4c, 0x7c, PIECES_END,
#else
	0, 0x1c, 0x38, 0x57, PIECES_END,
#endif
};

/* the card list's row ("Card List×Select   △Cancel": the title, ×, Select, △, Cancel; 0xd8 is the card count's
 * suffix, renderCardCount): Cartas centred, Cancelar wider, so × and Cancelar start further left */
LabelCrop CARD_LIST_CROPS[5] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ -0x1a, 0x00, 0x34 },
	{ -0x23, 0x48, 0x0c },
	{ -0x17, 0x54, 0x30 },
	{ 0x2b, 0x84, 0x0c },
	{ 0x37, 0x90, 0x40 },
#else
	{ -0x24, 0x00, 0x48 },
	{ -0x23, 0x48, 0x0c },
	{ -0x17, 0x54, 0x3c },
	{ 0x2b, 0x90, 0x0c },
	{ 0x37, 0x9c, 0x3c },
#endif
};

uint16_t CARD_LIST_PIECES[7] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	0 | PIECE_CENTRE, 0x48, 0x54, 0x84, 0x90, 0xd8, PIECES_END,
#else
	0 | PIECE_CENTRE, 0x48, 0x54, 0x90, 0x9c, 0xd8, PIECES_END,
#endif
};

/* the medal screen's row ("Medals Chart   ×△ Select  Cancel": the title, ×, Select in two crops, △, Cancel):
 * Medallas centred, Cancelar wider and further left (its last glyph must start before 244, drawGlyph's limit) */
LabelCrop MEDAL_VIEW_CROPS[6] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ -0x1e, 0x00, 0x3c },
	{ -0x23, 0x6c, 0x0c },
	{ -0x17, 0x84, 0x18 },
	{ 0x01, 0x9c, 0x14 },
	{ 0x2b, 0x78, 0x0c },
	{ 0x37, 0xb4, 0x40 },
#else
	{ -0x36, 0x00, 0x6c },
	{ -0x23, 0x6c, 0x0c },
	{ -0x17, 0x84, 0x18 },
	{ 0x01, 0x9c, 0x24 },
	{ 0x2b, 0x78, 0x0c },
	{ 0x37, 0xc0, 0x30 },
#endif
};

uint16_t MEDAL_VIEW_PIECES[6] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	0 | PIECE_CENTRE, 0x6c, 0x78, 0x84, 0xb4, PIECES_END,
#else
	0 | PIECE_CENTRE, 0x6c, 0x78, 0x84, 0xc0, PIECES_END,
#endif
};

/* the original evolution chart's keys ("×Select   △Cancel", NEW DIGIMON off): Cancelar is wider */
LabelCrop EVO_CHART_KEY_CROPS[4] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	{ -0x1a, 0x00, 0x0c },
	{ -0x0e, 0x0c, 0x3c },
	{ 0x3a, 0x48, 0x0c },
	{ 0x46, 0x54, 0x40 },
#else
	{ -0x1a, 0x00, 0x0c },
	{ -0x0e, 0x0c, 0x3c },
	{ 0x3a, 0x48, 0x0c },
	{ 0x46, 0x54, 0x3c },
#endif
};

/* The chart detail's "Tree" and level names (each centred at -0x44), rows 0x18
 * ("Tree FreshTraining"), 0x24 ("RookieChampion"), 0x3c ("Ultimate"): Árbol, Novato and Perfecto are wider */
LabelCrop CHART_TREE_CROP = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	-0x16, 0x00, 0x28,
#else
	-0x14, 0x00, 0x24,
#endif
};

LabelCrop CHART_LEVEL_CROPS[5] = {
#if MOD_LANG_BUILD == MOD_LANG_ZH
    { -92,40,48 }, { -98,88,60 }, { -94,0,52 }, { -98,52,60 }, { -101,0,66 },
#elif MOD_LANG_BUILD != MOD_LANG_EN
	{ -0x58, 0x28, 0x28 },
	{ -0x61, 0x50, 0x3a },
	{ -0x5e, 0x00, 0x34 },
	{ -0x62, 0x34, 0x3c },
	{ -0x65, 0x00, 0x42 },
#else
	{ -0x5b, 0x24, 0x2e },
	{ -0x63, 0x52, 0x3e },
	{ -0x5b, 0x00, 0x2f },
	{ -0x64, 0x2f, 0x41 },
	{ -0x64, 0x00, 0x41 },
#endif
};

uint16_t CHART_TREE_ROW_PIECES[5] = {
#if MOD_LANG_BUILD == MOD_LANG_ZH
    0 | PIECE_CENTRE, 40 | PIECE_CENTRE, 88 | PIECE_CENTRE, 204, PIECES_END,
#elif MOD_LANG_BUILD != MOD_LANG_EN
	0 | PIECE_CENTRE, 0x28 | PIECE_CENTRE, 0x50 | PIECE_CENTRE, 0xcc, PIECES_END,
#else
	0 | PIECE_CENTRE, 0x24 | PIECE_CENTRE, 0x52 | PIECE_CENTRE, 0xcc, PIECES_END,
#endif
};

uint16_t CHART_LEVEL_ROW_PIECES[3] = {
#if MOD_LANG_BUILD != MOD_LANG_EN
	0 | PIECE_CENTRE, 0x34 | PIECE_CENTRE, PIECES_END,
#else
	0 | PIECE_CENTRE, 0x2f | PIECE_CENTRE, PIECES_END,
#endif
};

char *MEDAL_NAMES[15] = {
	STR_MEDAL_NAME_GRADE_CUP,
	STR_MEDAL_NAME_VERSION_CUP,
	STR_MEDAL_NAME_TYPE_CUP,
	STR_MEDAL_NAME_SPECIAL_CUP,
	STR_MEDAL_NAME_100_TIMES,
	STR_MEDAL_NAME_TECHNIQUE_MASTER,
	STR_MEDAL_NAME_DIGIMON_MASTER,
	STR_MEDAL_NAME_MAX_ABILITIES,
	STR_MEDAL_NAME_PERFECT_CURLING,
	STR_MEDAL_NAME_100_FISH,
	MAIN_D_80134248,
	STR_MEDAL_NAME_TOWN_FLOURISHING,
	STR_MEDAL_NAME_CARD_COMPLETE,
	STR_MEDAL_NAME_BITS_MAXED,
	STR_MEDAL_NAME_10_YEARS,
};

#if !VERSION_IS(US)
char *MEDAL_DESCRIPTIONS[45] = {
	STR_MEDAL_DESCRIPTION_CUP_D_C_B_A_S,
	STR_MEDAL_DESCRIPTION_WIN_IN_ALL,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_WIN_IN_ALL_VER_1_2,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_FIRE_GRAPPLE_THUNDER_WIND,
	STR_MEDAL_DESCRIPTION_NATURE_COOL_METALLIC_FILTH_CUP,
	STR_MEDAL_DESCRIPTION_TYPE_CUP_WIN_IN_ALL,
	STR_MEDAL_DESCRIPTION_DINO_WING_ANIMAL_HUMAN_CUP,
	STR_MEDAL_DESCRIPTION_TYPE_CUP_WIN_IN_ALL,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_WON_CHAMPIONSHIP_100_TIMES,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_MASTERED_56_SWITCH_TECHNIQUES,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_RAISED_ALL_61_DIGIMON,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_MAXED_ALL_OF_THE_DIGIMONS,
	STR_MEDAL_DESCRIPTION_PARAMETERS,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_GOT_A_PERFECT_SCORE_IN_CURLING,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_100_FISH_CAUGHT,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_FINISHED_THE_GAME,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_JIJIMON_SAID_THE_TOWN,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_COLLECTED_ALL_DIGIMON_CARDS,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_COLLECTED_999999_BITS,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_SURVIVED_FOR_300_DAYS,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
};
#else
char *MEDAL_DESCRIPTIONS[45] = {
	STR_MEDAL_DESCRIPTION_CUP_D_C_B_A_S,
	STR_MEDAL_DESCRIPTION_WIN_IN_ALL,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_WIN_IN_ALL_VER_1_2,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_FIRE_GRAPPLE_THUNDER_WIND,
	STR_MEDAL_DESCRIPTION_NATURE_COOL_METALLIC_FILTH_CUP,
	STR_MEDAL_DESCRIPTION_WIN_IN_ALL,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_DINO_WING_ANIMAL_HUMAN_CUP,
	STR_MEDAL_DESCRIPTION_WIN_IN_ALL,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_WON_CHAMPIONSHIP_100_TIMES,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_MASTERED_56_SWITCH_TECHNIQUES,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_RAISED_ALL_61_DIGIMON,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_MAXED_ALL_OF_THE_DIGIMONS,
	STR_MEDAL_DESCRIPTION_PARAMETERS,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_GOT_A_PERFECT_SCORE_IN_CURLING,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_100_FISH_CAUGHT,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_FINISHED_THE_GAME,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_JIJIMON_SAID_THE_TOWN,
	STR_MEDAL_DESCRIPTION_IS_FLOURISHING,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_COLLECTED_ALL_DIGIMON_CARDS,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_COLLECTED_999999_BITS,
	STR_OVERWORLD_EMPTY,
	STR_OVERWORLD_EMPTY,
	STR_MEDAL_DESCRIPTION_SURVIVED_FOR_300_DAYS,
	STR_OVERWORLD_EMPTY,
};
#endif

char *TAMER_LEVEL_TITLES[11] = {
	MAIN_D_801342A8,
	MAIN_D_801342B0,
	MAIN_D_80123DE8,
	MAIN_D_80123DF4,
	MAIN_D_80123E04,
	MAIN_D_801342B8,
	MAIN_D_80123E10,
	MAIN_D_801342C0,
	MAIN_D_801342C8,
	MAIN_D_801342D0,
	MAIN_D_801342D8,
};

Line4Points CHART_FROM_LINES_EVEN[4] = {
	{ 0xffac, 0xffbc, 0xffdb, 0xfff6, 0xffd0, 0xffd0, 0xffef, 0xffef },
	{ 0xff9a, 0xffb6, 0xffc0, 0xfff6, 0xffe8, 0xffe8, 0xfff2, 0xfff2 },
	{ 0xff9a, 0xffb6, 0xffc0, 0xfff6, 0xffff, 0xffff, 0xfff5, 0xfff5 },
	{ 0xffac, 0xffbc, 0xffdb, 0xfff6, 0x0017, 0x0017, 0xfff8, 0xfff8 },
};

Line4Points CHART_FROM_LINES_ODD[5] = {
	{ 0xffbe, 0xffcc, 0xfff6, 0xfff6, 0xffc4, 0xffc4, 0xffee, 0xffee },
	{ 0xffac, 0xffc6, 0xffdb, 0xfff6, 0xffdc, 0xffdc, 0xfff1, 0xfff1 },
	{ 0xff9a, 0xfff6, 0xfff6, 0xfff6, 0xfff4, 0xfff4, 0xfff4, 0xfff4 },
	{ 0xffac, 0xffc6, 0xffdb, 0xfff6, 0x000c, 0x000c, 0xfff7, 0xfff7 },
	{ 0xffbe, 0xffcc, 0xfff6, 0xfff6, 0x0024, 0x0024, 0xfffa, 0xfffa },
};

Line4Points CHART_TO_LINES_EVEN[6] = {
	{ 0x0009, 0x003d, 0x0040, 0x0040, 0xffec, 0xffb8, 0xffb8, 0xffb8 },
	{ 0x0009, 0x0024, 0x0043, 0x0052, 0xffef, 0xffef, 0xffd0, 0xffd0 },
	{ 0x0009, 0x003f, 0x0049, 0x0064, 0xfff2, 0xfff2, 0xffe8, 0xffe8 },
	{ 0x0009, 0x003f, 0x0049, 0x0064, 0xfff5, 0xfff5, 0xffff, 0xffff },
	{ 0x0009, 0x0024, 0x0043, 0x0052, 0xfff8, 0xfff8, 0x0017, 0x0017 },
	{ 0x0009, 0x003d, 0x0040, 0x0040, 0xfffb, 0x002f, 0x002f, 0x002f },
};

Line4Points CHART_TO_LINES_ODD[5] = {
	{ 0x0009, 0x0033, 0x0040, 0x0040, 0xffee, 0xffc4, 0xffc4, 0xffc4 },
	{ 0x0009, 0x0024, 0x0039, 0x0052, 0xfff1, 0xfff1, 0xffdc, 0xffdc },
	{ 0x0009, 0x0064, 0x0064, 0x0064, 0xfff4, 0xfff4, 0xfff4, 0xfff4 },
	{ 0x0009, 0x0024, 0x0039, 0x0052, 0xfff7, 0xfff7, 0x000c, 0x000c },
	{ 0x0009, 0x0033, 0x0040, 0x0040, 0xfffa, 0x0024, 0x0024, 0x0024 },
};

RGB8 MAIN_D_80124A84[12] = {
	{ 0xfb, 0xe8, 0x02 },
	{ 0xec, 0x9a, 0x00 },
	{ 0x00, 0xb6, 0x00 },
	{ 0x00, 0x64, 0x00 },
	{ 0x00, 0xae, 0xb8 },
	{ 0x00, 0x6f, 0x8e },
	{ 0x00, 0x3c, 0xc8 },
	{ 0x00, 0x1e, 0x8c },
	{ 0xbe, 0x00, 0xa0 },
	{ 0x6e, 0x00, 0x6e },
	{ 0xbe, 0x00, 0x1e },
	{ 0x82, 0x1e, 0x1e },
};

ChartSprite CHART_FROM_SPRITES_EVEN[4] = {
	{ 0xff9a, 0xffc7, 0x0000, 0x0000 },
	{ 0xff88, 0xffdf, 0x0000, 0x0000 },
	{ 0xff88, 0xfff7, 0x0000, 0x0000 },
	{ 0xff9a, 0x000f, 0x0000, 0x0000 },
};

ChartSprite CHART_FROM_SPRITES_ODD[5] = {
	{ 0xffac, 0xffbb, 0x0000, 0x0000 },
	{ 0xff9a, 0xffd3, 0x0000, 0x0000 },
	{ 0xff88, 0xffeb, 0x0000, 0x0000 },
	{ 0xff9a, 0x0003, 0x0000, 0x0000 },
	{ 0xffac, 0x001b, 0x0000, 0x0000 },
};

ChartSprite CHART_TO_SPRITES_EVEN[6] = {
	{ 0x0042, 0xffaf, 0x0000, 0x0000 },
	{ 0x0054, 0xffc7, 0x0000, 0x0000 },
	{ 0x0066, 0xffdf, 0x0000, 0x0000 },
	{ 0x0066, 0xfff7, 0x0000, 0x0000 },
	{ 0x0054, 0x0010, 0x0000, 0x0000 },
	{ 0x0042, 0x0027, 0x0000, 0x0000 },
};

ChartSprite CHART_TO_SPRITES_ODD[5] = {
	{ 0x0042, 0xffbb, 0x0000, 0x0000 },
	{ 0x0054, 0xffd3, 0x0000, 0x0000 },
	{ 0x0066, 0xffeb, 0x0000, 0x0000 },
	{ 0x0054, 0x0003, 0x0000, 0x0000 },
	{ 0x0042, 0x001b, 0x0000, 0x0000 },
};

#if !VERSION_IS(US)
Inset MAIN_D_80124B48[6] = {
	{ 0x001c, 0x002b, 0x004e, 0x0082 },
	{ 0x0081, 0x0032, 0x003c, 0x0004 },
	{ 0x0088, 0x0055, 0x0030, 0x002e },
	{ 0x00d6, 0x002a, 0x004e, 0x0082 },
	{ 0x0044, 0x00bb, 0x0030, 0x0004 },
	{ 0x001a, 0x00ce, 0x0084, 0x0004 },
};
#else
Inset MAIN_D_80124B48[6] = {
	{ 0x001c, 0x002b, 0x004e, 0x0082 },
	{ 0x0081, 0x0032, 0x003c, 0x0004 },
	{ 0x0088, 0x0055, 0x0030, 0x002e },
	{ 0x00d6, 0x002a, 0x004e, 0x0082 },
	{ 0x0026, 0x00bb, 0x006c, 0x0004 },
	{ 0x001a, 0x00ce, 0x0084, 0x0004 },
};
#endif
// clang-format on

int32_t MENU_OPTION_COUNT;
int32_t HAS_FISHING_ROD;
int32_t TRIANGLE_MENU_STATE;
int8_t MENU_STATE;
int8_t MENU_SUB_STATE;
int8_t DIGIMON_MENU_STATE;
int8_t PLAYER_MENU_STATE;
int16_t MOVE_SELECT_BOX_Y;
int16_t MOVE_SELECT_BOX_X;
int8_t SELECTED_MEDAL;
int8_t MEDAL_SELECTOR_INDEX;
int8_t SELECTED_CARD;
int16_t CHART_SELECTED_DIGIMON;
int16_t CHART_SELECTED_COLUMN;
int16_t CHART_SELECTED_ROW;
int8_t MAIN_D_80134D46;
int16_t MAIN_D_80134D48;

static void *game_menu_sbss_order[] = {
	&MAIN_D_80134D48,
	&MAIN_D_80134D46,
	&CHART_SELECTED_ROW,
	&CHART_SELECTED_COLUMN,
	&CHART_SELECTED_DIGIMON,
	&SELECTED_CARD,
	&MEDAL_SELECTOR_INDEX,
	&SELECTED_MEDAL,
#if !VERSION_IS(US)
	&MOVE_SELECT_BOX_Y,
	&MOVE_SELECT_BOX_X,
#else
	&MOVE_SELECT_BOX_X,
	&MOVE_SELECT_BOX_Y,
#endif
	&PLAYER_MENU_STATE,
	&DIGIMON_MENU_STATE,
	&MENU_SUB_STATE,
	&MENU_STATE,
	&TRIANGLE_MENU_STATE,
	&HAS_FISHING_ROD,
	&MENU_OPTION_COUNT,
};

GARBAGE(addGameMenu, 14);

void addGameMenu(void)
{
	MENU_OPTIONS[0].unknown = 1;
	MENU_OPTION_COUNT = 7;
	MENU_OPTIONS[7].disabled = 0;
	HAS_FISHING_ROD = getFishingAvailability();
	if (HAS_FISHING_ROD != 0) {
		MENU_OPTION_COUNT++;
		if (HAS_FISHING_ROD == 1) {
			MENU_OPTIONS[7].disabled = 1;
		}
		MENU_OPTIONS[0].unknown = 7;
	}
	if (PARTNER_PARA.condition & 1) {
		MENU_OPTIONS[6].disabled = 0;
	} else {
		MENU_OPTIONS[6].disabled = 1;
	}
	TRIANGLE_MENU_STATE = 0;
	addObject(0xfa4, 0, (void (*)(int32_t))tickTriangleMenu, NULL);
}

/* MOD (centred): the triangle menu's box centred on the screen (the original's top was -0x50 with or without the
   fishing row: 25 pixels high without it, which put it against the HP and MP over it); everything in it moves down
   by this */
static int16_t triangleMenuShift(void)
{
	return HAS_FISHING_ROD != 0 ? 0x50 - 0x95 / 2 : 0x50 - 0x6e / 2;
}

void tickTriangleMenu(void)
{
	RECT rect;

	rect = MENU_TEXT_AREA;
	switch (TRIANGLE_MENU_STATE) {
	case 0:
		if (HAS_FISHING_ROD != 0) {
			createMenuBox(0, -0x42, -0x50 + triangleMenuShift(), 0x84, 0x95, 2,
			              tickGameMenu, renderGameMenu);
		} else {
			createMenuBox(0, -0x42, -0x50 + triangleMenuShift(), 0x84, 0x6e, 2,
			              tickGameMenu, renderGameMenu);
		}
		if (UI_BOX_DATA[0].frame == 4) {
			TRIANGLE_MENU_STATE = -1;
		}
		clearTextSubArea(&rect);
		DRAW_STRING_PIECES(STR_YEAR_DAY, 0, 0xe8, 0, 0x24);
		break;
	case 1:
		closeUIBoxIfOpen(0);
		removeObject(0xfa4, 0);
		break;
	case 2:
		closeUIBoxIfOpen(0);
		if (UI_BOX_DATA[0].frame == 0) {
			addInventoryUI();
			TRIANGLE_MENU_STATE = -1;
		}
		break;
	case 3:
		closeUIBoxIfOpen(0);
		if (UI_BOX_DATA[0].frame == 0) {
			createMenuBox(1, -0x96, -0x59, 300, 0xbe, 0,
			              tickDigimonMenu, renderDigimonMenu);
			clearTextArea();
			DRAW_STRING_PIECES(STATUS_VIEW_LABELS[0], 0, 0, 0 | PIECE_CENTRE, 0x3c | PIECE_CENTRE);
			MENU_STATE = 0;
			MENU_SUB_STATE = 0;
			TRIANGLE_MENU_STATE = -1;
			DIGIMON_MENU_STATE = 0;
		}
		break;
	case 4:
		TAMER_ENTITY.entity.isOnScreen = 1;
		PARTNER_ENTITY.digimonEntity.entity.isOnScreen = 1;
		closeUIBoxIfOpen(1);
		if (UI_BOX_DATA[1].frame == 0) {
			TRIANGLE_MENU_STATE = 0;
		}
		break;
	case 5:
		closeUIBoxIfOpen(0);
		if (UI_BOX_DATA[0].frame == 0) {
			createMenuBox(1, -0x96, -0x59, 300, 0xbe, 0,
			              tickPlayerMenu, renderPlayerMenu);
			clearTextArea();
			DRAW_STRING_PIECES_AT(PLAYER_VIEW_LABELS[0], 0, 0, PLAYER_TAB_PIECES); /* MOD (language) */
			MENU_STATE = 0;
			MENU_SUB_STATE = 0;
			TRIANGLE_MENU_STATE = -1;
			PLAYER_MENU_STATE = 0;
		}
		break;
	case 6:
		TAMER_ENTITY.entity.isOnScreen = 1;
		PARTNER_ENTITY.digimonEntity.entity.isOnScreen = 1;
		closeUIBoxIfOpen(1);
		if (UI_BOX_DATA[1].frame == 0) {
			closeMenuOverlay();
			TRIANGLE_MENU_STATE = 0;
		}
	}
}

void closeTriangleMenu(void)
{
	closeUIBoxIfOpen(1);
	closeInventoryBoxes2();
	closeUIBoxIfOpen(0);
	removeObject(0xfa4, 0);
}

void closeUIBoxIfOpen(id)
	int16_t id;
{
	RECT rect;
	DVECTOR pos;

	if (UI_BOX_DATA[id].frame >= 5 && UI_BOX_DATA[id].state == 1) {
		getEntityScreenPos(ENTITY_TABLE[0], 0, &pos);
		setRECT(&rect, pos.vx - 5, pos.vy - 5, 10, 10);
		removeAnimatedUIBox(id, 0);
	}
}

/* MOD (vitals): while the triangle menu is open, the partner's HP and MP at the top right of the screen, outside the
   box, as the battle HUD draws them (its sprites in the ETC texture, tpage 0x1e, are resident on the field) */
static void renderVitalsSprite(int32_t x, int32_t y, int32_t w, int32_t h, int32_t u, int32_t v, int32_t uw,
                               int32_t vh, int32_t clutY, int32_t z)
{
	POLY_FT4 *prim;

	prim = (POLY_FT4 *)GsGetWorkBase();
	SetPolyFT4(prim);
	prim->tpage = GetTPage(0, 0, 896, 256);
	prim->clut = GetClut(0x100, clutY);
	setRGB0(prim, 0x80, 0x80, 0x80);
	setUVWH(prim, u, v, uw, vh);
	setXYWH(prim, x, y, w, h);
	AddPrim(ACTIVE_ORDERING_TABLE->org + z, prim++);
	GsSetWorkBase((PACKET *)prim);
}

/* the battle HUD's 7 x 11 digits (STD_renderNumber), the last ending at `right` */
static void renderVitalsDigits(int32_t right, int32_t y, int32_t value, int32_t z)
{
	int32_t digits[6];
	int32_t count;
	int32_t i;

	convertValueToDigits(5, value, &count, digits);
	for (i = 0; i < count; i++) {
		right -= 7;
		renderVitalsSprite(right, y, 7, 11, digits[i] * 7, 172, 7, 11, 492, z);
	}
}

/* one battle HUD bar (STD_renderFighterStatusBars): frame, icon, the fill stretched to 50 px at most and the
   current value in its box */
static void renderVitalsBattleBar(int32_t x0, int32_t y0, int32_t mp, int32_t current, int32_t max)
{
	renderVitalsDigits(x0 + 0x49 + 28, y0 - 3, current, 8);
	/* fill first: a later sprite on the same layer is drawn under it */
	if (current > 0) {
		renderVitalsSprite(x0 + 0x12, y0 + 3, current * 50 / max, 2, mp ? 0x84 : 0x80, 0xb0, 2, 2, 0x1eb, 8);
	}
	renderVitalsSprite(x0 + 3, y0 - 2, 11, 11, mp ? 0x9b : 0x90, 0xb0, 11, 11, 0x1eb, 8);
	renderVitalsSprite(x0, y0, 0x68, 8, 0x80, 0xa8, 0x68, 8, 0x1ec, 8);
}

static void renderMenuVitals(void)
{
	int32_t hp;
	int32_t maxHp;
	int32_t mpv;
	int32_t maxMp;

	hp = PARTNER_ENTITY.digimonEntity.stats.current.currentHP;
	maxHp = PARTNER_ENTITY.digimonEntity.stats.base.hp;
	mpv = PARTNER_ENTITY.digimonEntity.stats.current.currentMP;
	maxMp = PARTNER_ENTITY.digimonEntity.stats.base.mp;
	if (maxHp <= 0 || maxMp <= 0)
		return;
	if (hp < 0)
		hp = 0;
	if (hp > maxHp)
		hp = maxHp;
	if (mpv < 0)
		mpv = 0;
	if (mpv > maxMp)
		mpv = maxMp;
	renderVitalsBattleBar(0x24, -0x64, 0, hp, maxHp);
	renderVitalsBattleBar(0x24, -0x58, 1, mpv, maxMp);
}

void renderGameMenu(void)
{
	Line lines[2];
	GameMenuSprite *sprite;
	int32_t digitCount;
	int32_t digits[6];
	int8_t disabled;
	int8_t highlight;
	int16_t yOffset;
	int32_t i;

	/* MOD (centred): moved down by triangleMenuShift */
	lines[0] = GAME_MENU_LINES[0];
	lines[1] = GAME_MENU_LINES[1];
	lines[0].y1 += triangleMenuShift();
	lines[0].y2 += triangleMenuShift();
	lines[1].y1 += triangleMenuShift();
	lines[1].y2 += triangleMenuShift();
	renderSeparatorLines(lines, 2, 5);
	renderDateDigits();
	renderMenuVitals(); /* MOD (vitals) */
	yOffset = triangleMenuShift();
	if (MENU_OPTION_COUNT == 7) {
		yOffset -= 0x28;
	}
	sprite = &MENU_OPTIONS[1];
	renderTriangleCursor((int8_t)MENU_OPTIONS[0].unknown, yOffset);
	for (i = 1; i < MENU_OPTION_COUNT; sprite++, i++) {
		disabled = 0;
		if (sprite->disabled == 1) {
			disabled = 1;
		}
		highlight = 0;
		if ((i == MENU_OPTIONS[0].unknown) &&
		    ((PLAYTIME_FRAMES % 10) < 5)) {
			highlight = 0x14;
		}
		renderRectPolyFT4(sprite->x - 0x42,
		                  sprite->y - 0x50 + yOffset,
		                  sprite->width, sprite->height,
		                  sprite->texX + highlight,
		                  sprite->texY + 0xc0, 0x1e,
		                  GetClut(0x100, sprite->clutY + 0x1f0), 6,
		                  disabled);
		renderRectPolyFT4(GAME_MENU_TEXT_SPRITES[i].x - 0x42,
		                  yOffset + (GAME_MENU_TEXT_SPRITES[i].y - 0x50),
		                  GAME_MENU_TEXT_SPRITES[i].width,
		                  GAME_MENU_TEXT_SPRITES[i].height,
		                  GAME_MENU_TEXT_SPRITES[i].texX,
                          GAME_MENU_TEXT_SPRITES[i].texY + (MOD_LANG == MOD_LANG_ZH ? 0x80 : 0xbf),
                          MOD_LANG == MOD_LANG_ZH ? 5 : 0x1e,
                          MOD_LANG == MOD_LANG_ZH ? 0x7b06 : 0x7f50, 6,
		                  disabled);
	}
#if !VERSION_IS(US)
	renderString(3, 7, -0x49 + triangleMenuShift(), 0xc, 0xc, 0, 0xe8, 6, 1);
	renderString(3, 0x2c, -0x49 + triangleMenuShift(), 0xc, 0xc, 0xc, 0xe8, 6, 1);
#else
	/* MOD (font): from the row the text is drawn on (0xe8), not the one above, which no one clears: the player screen's
	 * play time (0xe4) left its pixels there over "Year" (the same place on screen, one row shorter) */
	renderString(3, -0x3c, -0x48 + triangleMenuShift(), 0x24, 0xb, 0, 0xe8, 6, 1);
	renderString(3, 6, -0x48 + triangleMenuShift(), 0x1c, 0xb, 0x24, 0xe8, 6, 1);
#endif
	convertValueToDigits(3, DAY, &digitCount, digits);
}

#if !VERSION_IS(US)
void renderDateDigits(void)
{
	int32_t digitCount;
	int32_t digits[6];
	int32_t i;
	int16_t texX;
	int16_t texY;

	convertValueToDigits(2, DAY + 1, &digitCount, digits);
	for (i = digitCount - 1; i >= 0; i--) {
		texY = 0x34;
		if (digits[i] == 0) {
			texX = 0x78;
		} else if (digits[i] < 5) {
			texX = (digits[i] - 1) * 8 + 0x60;
			texY = 0x28;
		} else {
			texX = (digits[i] - 5) * 8 + 0x50;
		}
		renderRectPolyFT4((2 - i) * 9 + 0xf, -0x4a + triangleMenuShift(), 8, 0xc, texX,
		                  texY + 0xc0, 0x1e, 0x7f10, 5, 0);
	}

	convertValueToDigits(3, YEAR + 1, &digitCount, digits);
	for (i = digitCount - 1; i >= 0; i--) {
		texY = 0x34;
		if (digits[i] == 0) {
			texX = 0x78;
		} else if (digits[i] < 5) {
			texX = (digits[i] - 1) * 8 + 0x60;
			texY = 0x28;
		} else {
			texX = (digits[i] - 5) * 8 + 0x50;
		}
		renderRectPolyFT4((2 - i) * 9 - 0x16, -0x4a + triangleMenuShift(), 8, 0xc, texX,
		                  texY + 0xc0, 0x1e, 0x7f10, 5, 0);
	}
}
#else
void renderDateDigits(void)
{
	int32_t digitCount;
	int32_t digits[6];
	int32_t digit;
	int32_t i;
	int32_t j;
	int16_t texX;
	int16_t texY;

	convertValueToDigits(2, DAY + 1, &digitCount, digits);
	for (i = digitCount - 1, j = 0; i >= 0; i--, j++) {
		texY = 0x34;
		if ((digit = *(int32_t *)&digits[i]) == 0) {
			texX = 0x78;
		} else if (digit < 5) {
			texX = (digits[i] - 1) * 8 + 0x60;
			texY = 0x28;
		} else {
			texX = (digits[i] - 5) * 8 + 0x50;
		}
		renderRectPolyFT4(0x25 + j * 9, -0x4a + triangleMenuShift(), 7, 0xc, texX + 1,
		                  texY + 0xc0, 0x1e, 0x7f10, 5, 0);
	}

	convertValueToDigits(3, YEAR + 1, &digitCount, digits);
	for (i = digitCount - 1, j = 0; i >= 0; i--, j++) {
		texY = 0x34;
		if ((digit = *(int32_t *)&digits[i]) == 0) {
			texX = 0x78;
		} else if (digit < 5) {
			texX = (digits[i] - 1) * 8 + 0x60;
			texY = 0x28;
		} else {
			texX = (digits[i] - 5) * 8 + 0x50;
		}
		renderRectPolyFT4(-0x16 + j * 9, -0x4a + triangleMenuShift(), 7, 0xc, texX + 1,
		                  texY + 0xc0, 0x1e, 0x7f10, 5, 0);
	}
}
#endif

void renderTriangleCursor(int8_t selection, int16_t yOffset)
{
	TriangleCursorUVData u0;
	TriangleCursorUVData u1;
	TriangleCursorUVData v0;
	TriangleCursorUVData v1;
	TriangleCursorOffsetData xOffset;
	TriangleCursorOffsetData yOffsetData;
	TriangleCursorOffsetData width;
	TriangleCursorOffsetData height;
	POLY_FT4 *prim;
	GsOT_TAG *tags;
	int16_t baseX;
	int16_t baseY;
	int32_t i;

	u0 = SELECTION_CURSOR_U_MIN;
	u1 = SELECTION_CURSOR_U_MAX;
	v0 = SELECTION_CURSOR_V_MIN;
	v1 = SELECTION_CURSOR_V_MAX;
	xOffset = SELECTION_CURSOR_X;
	yOffsetData = SELECTION_CURSOR_Y;
	width = SELECTION_CURSOR_WIDTH;
	height = SELECTION_CURSOR_HEIGHT;
	baseX = MENU_OPTIONS[selection].x -
	        0x46;
	baseY = MENU_OPTIONS[selection].y -
	        0x53;
	tags = ACTIVE_ORDERING_TABLE->org;
	for (i = 0; i < 8; i++) {
		prim = (POLY_FT4 *)GsGetWorkBase();
		SetPolyFT4(prim);
		setUV4(prim, (uint8_t)u0.data[i], (uint8_t)v0.data[i], (uint8_t)u1.data[i], (uint8_t)v0.data[i], (uint8_t)u0.data[i], (uint8_t)v1.data[i], (uint8_t)u1.data[i], (uint8_t)v1.data[i]);
		setPosDataPolyFT4(prim, baseX + xOffset.data[i],
		                  yOffset + (baseY + yOffsetData.data[i]),
		                  width.data[i], height.data[i]);
		setRGB0(prim, 0x80, 0x80, 0x80);
		prim->tpage = GetTPage(0, 0, 0x380, 0x1c0);
		prim->clut = GetClut(0x100, 0x1fc);
		AddPrim(&tags[5], prim);
		prim++;
		GsSetWorkBase((PACKET *)prim);
	}
}

void tickGameMenu(void)
{
	int32_t selection;

	if (PARTNER_PARA.condition & 1) {
		MENU_OPTIONS[6].disabled = 0;
	}
	selection = MENU_OPTIONS[0].unknown;
	if ((POLLED_INPUT & ~POLLED_INPUT_PREVIOUS) & 0x1000) {
		if ((selection -= 3) <= 0) {
			selection += ((MENU_OPTION_COUNT + 1) / 3) * 3;
		}
		if (selection >= MENU_OPTION_COUNT) {
			selection -= 3;
		}
	} else if ((POLLED_INPUT & ~POLLED_INPUT_PREVIOUS) & 0x4000) {
		if ((selection += 3) >= MENU_OPTION_COUNT) {
			selection -= ((MENU_OPTION_COUNT + 1) / 3) * 3;
		}
		if (selection <= 0) {
			selection += 3;
		}
	} else if ((POLLED_INPUT & ~POLLED_INPUT_PREVIOUS) & 0x8000) {
		if ((selection -= 1) <= 0) {
			selection = MENU_OPTION_COUNT - 1;
		}
	} else if ((POLLED_INPUT & ~POLLED_INPUT_PREVIOUS) & 0x2000) {
		if ((selection += 1) >= MENU_OPTION_COUNT) {
			selection = 1;
		}
	}
	if (selection != MENU_OPTIONS[0].unknown) {
		MENU_OPTIONS[0].unknown = selection;
		playSound(0, 2);
	}
	if (TRIANGLE_MENU_STATE == -1) {
		if (isKeyDown(CONFIRM_BUTTON) != 0) {
			if (MENU_OPTIONS[MENU_OPTIONS[0].unknown].disabled & 1) {
				playSound(0, 4);
			} else {
				playSound(0, 3);
			}
			handleGameMenuSelection(MENU_OPTIONS[0].unknown);
		}
		if ((isKeyDown(CANCEL_BUTTON) != 0) &&
		    ((UI_BOX_DATA[0].state == 1) ||
		     (UI_BOX_DATA[0].frame == 0))) {
			playSound(0, 4);
			closeTriangleMenu();
			tamerSetState(0);
			setCameraFollowPlayer();
			IS_IN_MENU = 0;
			startGameTime();
		}
	}
}

int32_t createMenuBox(int16_t id, int16_t x, int16_t y, int16_t width,
                      int16_t height, int8_t features, void (*tick)(void),
                      void (*render)(void))
{
	RECT finalPos;
	RECT startPos;
	DVECTOR entityPos;

	if (UI_BOX_DATA[id].state == 1) {
		return 1;
	}
	if (UI_BOX_DATA[id].frame == 0) {
		setRECT(&finalPos, x, y, width, height);
		getEntityScreenPos(ENTITY_TABLE[0], 1, &entityPos);
		setRECT(&startPos, entityPos.vx - 5, entityPos.vy - 5, 10, 10);
		createAnimatedUIBox(id, 1, features, &finalPos, &startPos,
		                    (TickFunction)tick, (RenderFunction)render);
	}
	return 0;
}

void tickDigimonMenu(void)
{
	int8_t equippedSlot;
	int16_t previousX;
	int16_t previousY;
	int32_t i;

	if ((DIGIMON_MENU_STATE != 1) ||
	    ((DIGIMON_MENU_STATE == 1) && (MENU_STATE == 1))) {
		if ((CHANGED_INPUT & 0x2000) && (MENU_STATE != 0)) {
			DIGIMON_MENU_STATE++;
			if (DIGIMON_MENU_STATE >= 2) {
				DIGIMON_MENU_STATE = 1;
			} else {
				MENU_STATE = 0;
				MENU_SUB_STATE = 0;
				playSound(0, 2);
			}
		}
		if ((CHANGED_INPUT & 0x8000) && (MENU_STATE != 0)) {
			DIGIMON_MENU_STATE--;
			if (DIGIMON_MENU_STATE < 0) {
				DIGIMON_MENU_STATE = 0;
			} else {
				MENU_STATE = 0;
				MENU_SUB_STATE = 0;
				playSound(0, 2);
			}
		}
		if (isKeyDown(CANCEL_BUTTON) != 0) {
			if (MENU_STATE == 1) {
				TRIANGLE_MENU_STATE = 4;
			}
			playSound(0, 4);
		}
		if ((DIGIMON_MENU_STATE == 1) && (isKeyDown(CONFIRM_BUTTON) != 0)) {
			if (MENU_STATE == 1) {
				MENU_STATE = 2;
			}
			playSound(0, 3);
		}
	} else if (MENU_STATE == 6) {
		if (isKeyDown(CANCEL_BUTTON) != 0) {
			for (i = 0; i < 3; i++) {
				if ((PARTNER_ENTITY.digimonEntity.stats.base.moves[i] !=
				     0xff) &&
				    (MOVE_DATA[entityGetTechFromAnim(
						       &PARTNER_ENTITY.digimonEntity.entity,
						       PARTNER_ENTITY.digimonEntity.stats.base
							       .moves[i])]
				             .power != 0)) {
					break;
				}
			}
			if (((PARTNER_ENTITY.digimonEntity.stats.base.moves[0] !=
			      0xff) ||
			     (PARTNER_ENTITY.digimonEntity.stats.base.moves[1] !=
			      0xff) ||
			     (PARTNER_ENTITY.digimonEntity.stats.base.moves[2] !=
			      0xff)) &&
			    (i != 3)) {
				MENU_STATE = 4;
			}
			playSound(0, 4);
		} else if (isKeyDown(0x80) != 0) {
			MENU_STATE = 7;
			MENU_SUB_STATE = 0;
			playSound(0, 3);
		} else if (isKeyDown(CONFIRM_BUTTON) != 0) {
			if ((equippedSlot = getEquippedSlot()) != -1) {
				EQUIPPED_MOVES[equippedSlot] = 0xff;
				PARTNER_ENTITY.digimonEntity.stats.base
					.moves[equippedSlot] = 0xff;
				playSound(0, 3);
			} else {
				equipMove();
			}
		}
		previousX = MOVE_SELECT_BOX_X;
		previousY = MOVE_SELECT_BOX_Y;
		if (CHANGED_INPUT & 0x1000) {
			MOVE_SELECT_BOX_Y -= 0xf;
		}
		if (CHANGED_INPUT & 0x4000) {
			MOVE_SELECT_BOX_Y += 0xf;
		}
		if (CHANGED_INPUT & 0x8000) {
			MOVE_SELECT_BOX_X -= 0x12;
		}
		if (CHANGED_INPUT & 0x2000) {
			MOVE_SELECT_BOX_X += 0x12;
		}
		if (MOVE_SELECT_BOX_X < 0x73) {
			MOVE_SELECT_BOX_X = 0x73;
		}
		if (MOVE_SELECT_BOX_X >= 0xf2) {
			MOVE_SELECT_BOX_X = 0xf1;
		}
		if (MOVE_SELECT_BOX_Y < 0x6f) {
			MOVE_SELECT_BOX_Y = 0x6f;
		}
		if (MOVE_SELECT_BOX_Y >= 0xca) {
			MOVE_SELECT_BOX_Y = 0xc9;
		}
		if ((previousX != MOVE_SELECT_BOX_X) ||
		    (previousY != MOVE_SELECT_BOX_Y)) {
			playSound(0, 2);
		}
	} else if ((MENU_STATE == 8) && (isKeyDown(CANCEL_BUTTON) != 0)) {
		MENU_STATE = 9;
		MENU_SUB_STATE = 0;
		playSound(0, 4);
	}
	TAMER_ENTITY.entity.isOnScreen = 0;
	PARTNER_ENTITY.digimonEntity.entity.isOnScreen = 0;
}

void renderDigimonMenu(void)
{
	DigimonTabs tabs;
	int32_t i;

	tabs = DIGIMON_MENU_VIEWS;
	switch (DIGIMON_MENU_STATE) {
	case 0:
		renderDigimonStatsView();
		break;
	case 1:
		renderDigimonMovesView();
		break;
	}
	tabs.tab[DIGIMON_MENU_STATE] = 0;
	for (i = 0; i < 2; i++) { /* MOD (language): DIGIMON_TAB_CROPS */
		renderString(tabs.tab[i], DIGIMON_TAB_CROPS[i].x, -0x65, DIGIMON_TAB_CROPS[i].w, 0xc, DIGIMON_TAB_CROPS[i].u, 0, 5, 1);
	}
	renderMenuTab(-0x91, 0x4c, tabs.tab[0]);
	renderMenuTab(-0x46, 0x40, tabs.tab[1]);
}

int32_t isUIBoxAvailable(int32_t id)
{
	if (UI_BOX_DATA[id].state == 1) {
		return 1;
	}

	if (UI_BOX_DATA[id].frame == 0) {
		return 1;
	}

	return 0;
}

void tickPlayerMenu(void)
{
	RECT finalPos;
	RECT secondFinalPos;
	RECT startPos;
	int16_t cardX;
	int16_t boxX; /* MOD (chart): the selected box (chartBoxXY) */
	int16_t boxY;
	uint8_t previousCard;

	if (MENU_STATE < 2) {
		/* MOD (chart): L1/R1 turn the chart's tab with the focus on the menu's tab too, before the grid is entered */
		if (PLAYER_MENU_STATE == 1 && MENU_STATE == 1 && MOD_OPT_NEW_DIGIMON && (CHANGED_INPUT & 0xC)) {
			chartOverviewInput(0);
		}
		if (CHANGED_INPUT & 0x2000) {
			PLAYER_MENU_STATE++;
			if (PLAYER_MENU_STATE >= 5) {
				PLAYER_MENU_STATE = 4;
			} else {
				MENU_STATE = 0;
				MENU_SUB_STATE = 0;
				playSound(0, 2);
			}
		}
		if (CHANGED_INPUT & 0x8000) {
			PLAYER_MENU_STATE--;
			if (PLAYER_MENU_STATE < 0) {
				PLAYER_MENU_STATE = 0;
			} else {
				MENU_STATE = 0;
				MENU_SUB_STATE = 0;
				playSound(0, 2);
			}
		}
		if (isKeyDown(CANCEL_BUTTON) != 0) {
			TRIANGLE_MENU_STATE = 6;
			playSound(0, 4);
		}
		if ((isKeyDown(CONFIRM_BUTTON) != 0) && (MENU_STATE == 1) &&
		    (PLAYER_MENU_STATE != 0)) {
			MENU_STATE = 2;
			playSound(0, 3);
			MEDAL_SELECTOR_INDEX = SELECTED_MEDAL = 0;
			SELECTED_CARD = 0;
		}
	} else {
		if ((isKeyDown(CANCEL_BUTTON) != 0) && (MENU_STATE == 2)) {
			playSound(0, 4);
			MENU_STATE = 1;
			CHART_SELECTED_DIGIMON = -1;
		}

		if (PLAYER_MENU_STATE == 4 && MENU_STATE == 2) {
			chartOverviewInput(1);
		}
		if (PLAYER_MENU_STATE == 1) {
			if (MENU_STATE == 2) {
				/* MOD (chart): the tabs, pages and grid of every partner species (chartOverviewInput) */
				if (MOD_OPT_NEW_DIGIMON) {
					chartOverviewInput(1);
				} else {
					chartVanillaInput(); /* MOD (options): NEW DIGIMON off, the original sheet */
				}
				if (isKeyDown(CONFIRM_BUTTON) != 0) {
					if (CHART_SELECTED_DIGIMON > 0 &&
					    hasDigimonRaised((uint16_t)CHART_SELECTED_DIGIMON) != 0) {
						chartBoxXY(CHART_SELECTED_COLUMN, CHART_SELECTED_ROW, &boxX, &boxY);
						if (isUIBoxAvailable(2) == 1) {
							setRECT(&finalPos, -0x96, -0x59, 0x12c, 190);
							startPos.x = boxX - 0x9b;
							startPos.y = boxY - 0x70;
							setWH(&startPos, 10, 10);
							MAIN_D_80134D46 = 0;
							chartDetailOpen(CHART_SELECTED_DIGIMON); /* MOD (portraits) */
							createAnimatedUIBox(
								2, 1, 0, &finalPos, &startPos,
								NULL,
								(RenderFunction)renderEvoChartDetail);
							playSound(0, 3);
							MENU_STATE = 3;
						}
					} else {
						playSound(0, 4);
					}
				}
			} else if ((MENU_STATE == 3) && chartDetailInput()) {
				/* MOD (portraits): moved the cursor, or went to another species of the tree */
			} else if ((MENU_STATE == 3) &&
			           (isKeyDown(CANCEL_BUTTON) != 0)) {
				if (MENU_STATE == 3) {
					chartBoxXY(CHART_SELECTED_COLUMN, CHART_SELECTED_ROW, &boxX, &boxY);
					setRECT(&finalPos, boxX - 0x9b, boxY - 0x70, 10, 10);
					removeAnimatedUIBox(2, &finalPos);
					playSound(0, 3);
					MENU_STATE = 2;
					chartCardRelease(); /* MOD (chart criteria) */
				}
			}
		}

#if !VERSION_IS(US)
		if (PLAYER_MENU_STATE == 2 && MENU_STATE == 2) {
#else
		if (PLAYER_MENU_STATE == 2) {
#endif
			if ((CHANGED_INPUT & 0x8000) &&
			    ((MEDAL_SELECTOR_INDEX % 5) != 0)) {
				MEDAL_SELECTOR_INDEX--;
			}
			if ((CHANGED_INPUT & 0x2000) &&
			    ((MEDAL_SELECTOR_INDEX % 5) != 4)) {
				MEDAL_SELECTOR_INDEX++;
			}
			if ((CHANGED_INPUT & 0x1000) &&
			    (MEDAL_SELECTOR_INDEX >= 5)) {
				MEDAL_SELECTOR_INDEX -= 5;
			}
			if ((CHANGED_INPUT & 0x4000) &&
			    (MEDAL_SELECTOR_INDEX < 10)) {
				MEDAL_SELECTOR_INDEX += 5;
			}
			if (SELECTED_MEDAL != MEDAL_SELECTOR_INDEX) {
				if (hasMedal(MEDAL_SELECTOR_INDEX) != 0) {
					activateMedalTexture(MEDAL_SELECTOR_INDEX,
					                     SELECTED_MEDAL);
					MENU_STATE = 3;
				}
				playSound(0, 2);
			}
			SELECTED_MEDAL = MEDAL_SELECTOR_INDEX;
		}

		if (PLAYER_MENU_STATE == 3) {
			previousCard = SELECTED_CARD;
			if (MENU_STATE == 2) {
				if ((isKeyDown(CONFIRM_BUTTON) != 0) &&
				    (getCardAmount((uint8_t)SELECTED_CARD) != 0)) {
					playSound(0, 3);
					MENU_STATE = 3;
					loadCardImage(SELECTED_CARD);
					cardX = (SELECTED_CARD % 11) < 6 ? 0xa6 : -0xcc;
					setRECT(&startPos, (SELECTED_CARD % 11) * 0x18 - 0x79, (SELECTED_CARD / 11) * 0x18 - 0x36, 1, 1);
					setRECT(&finalPos, -0x4b, -0x53, 0x96, 0xb4);
					setRECT(&secondFinalPos, 0x4a, 0x45, 0x36, 0x18);
					createAnimatedUIBox(
						2, 1, 0, &finalPos, &startPos, NULL,
						(RenderFunction)renderCardImage);
					createAnimatedUIBox(
						3, 1, 0, &secondFinalPos, &startPos,
						NULL, (RenderFunction)renderCardCount);
				}
				if (MENU_STATE == 2) {
					if ((CHANGED_INPUT & 0x8000) &&
					    ((SELECTED_CARD % 11) != 0)) {
						SELECTED_CARD--;
					}
					if ((CHANGED_INPUT & 0x2000) &&
					    ((SELECTED_CARD % 11) != 10)) {
						SELECTED_CARD++;
					}
					if ((CHANGED_INPUT & 0x1000) &&
					    (SELECTED_CARD >= 11)) {
						SELECTED_CARD -= 11;
					}
					if ((CHANGED_INPUT & 0x4000) &&
					    (SELECTED_CARD < 0x37)) {
						SELECTED_CARD += 11;
					}
					if (previousCard != SELECTED_CARD) {
						playSound(0, 2);
					}
				}
			}
			if ((isKeyDown(CANCEL_BUTTON) != 0) && (MENU_STATE == 3)) {
				playSound(0, 4);
				MENU_STATE = 2;
				setRECT(&startPos, (SELECTED_CARD % 11) * 0x18 - 0x79, (SELECTED_CARD / 11) * 0x18 - 0x36, 1, 1);
				removeAnimatedUIBox(2, &startPos);
				removeAnimatedUIBox(3, &startPos);
			}
		}
	}

	if (POLLED_INPUT == 0) {
		MAIN_D_80134D48 = 0;
	} else {
		MAIN_D_80134D48++;
	}
	TAMER_ENTITY.entity.isOnScreen = 0;
	PARTNER_ENTITY.digimonEntity.entity.isOnScreen = 0;
}

void renderPlayerMenu(void)
{
	PlayerTabs tabs;
	int32_t i;

	tabs = MAIN_D_801342A4;
	switch (PLAYER_MENU_STATE) {
	case 0:
		renderPlayerInfoView();
		break;
	case 1:
	case 4:
		renderEvoChartView();
		break;
	case 2:
		renderMedalView();
		break;
	case 3:
		renderCardsView();
		break;
	}
	tabs.tab[PLAYER_MENU_STATE] = 0;
	for (i = 0; i < 5; i++) {
		renderString(tabs.tab[i], PLAYER_TAB_CROPS[i].x, -0x65, PLAYER_TAB_CROPS[i].w, 0xc, PLAYER_TAB_CROPS[i].u, 0, 5, 1);
	}
	renderMenuTab(-145, 70, tabs.tab[0]);
	renderMenuTab(-76, 58, tabs.tab[1]);
	renderMenuTab(-19, 46, tabs.tab[2]);
	renderMenuTab(26, 66, tabs.tab[3]);
	renderMenuTab(91, 58, tabs.tab[4]);

}

void handleGameMenuSelection(int32_t selection)
{
	switch (selection) {
	case 1:
		if (MENU_OPTIONS[selection].disabled & 1) {
			return;
		}
		TRIANGLE_MENU_STATE = 2;
		drawInventoryText();
		break;
	case 6:
		if (MENU_OPTIONS[selection].disabled & 1) {
			return;
		}
		closeTriangleMenu();
		partnerSetState(3);
		IS_IN_MENU = 0;
		startGameTime();
		break;
	case 5:
		if (MENU_OPTIONS[selection].disabled & 1) {
			return;
		}
		closeTriangleMenu();
		partnerSetState(4);
		IS_IN_MENU = 0;
		startGameTime();
		break;
	case 2:
		TRIANGLE_MENU_STATE = 3;
		break;
	case 3:
		loadMenuOverlay();
		TRIANGLE_MENU_STATE = 5;
		break;
	case 4:
		if (MENU_OPTIONS[selection].disabled & 1) {
			return;
		}
		closeTriangleMenu();
		partnerSetState(0xf);
		IS_IN_MENU = 0;
		startGameTime();
		break;
	case 7:
		if (MENU_OPTIONS[selection].disabled & 1) {
			return;
		}
		closeTriangleMenu();
		setCameraFollowPlayer();
		initializeFishing();
		tamerSetState(0xb);
		IS_IN_MENU = 0;
	}
}

void setSleepDisabled(int16_t arg)
{
	MENU_OPTIONS[6].disabled = arg;
}

void startFeedingItem(ItemId arg) /* MOD: was u8 */
{
	if (TAMER_ITEM.worldItem.type == ITEM_NONE) {
		TAMER_ITEM.worldItem.type = arg;
		tamerSetState(6);
		partnerSetState(5);
		removeObject(0xfa4, 0);
		IS_IN_MENU = 0;
		startGameTime();
	}
}

void removeOneSelectedItem(void)
{
	removeItem(INVENTORY.types.array[INVENTORY_POINTER], 1);
}

void renderFeedingItem(int16_t arg)
{
	MATRIX *m;

	if (arg == 0) {
		m = &TAMER_ENTITY.entity.posData[9].posMatrix.workm;
	}

	TAMER_ITEM.worldItem.spriteLocation.vx = m->t[0];
	TAMER_ITEM.worldItem.spriteLocation.vy = m->t[1];
	TAMER_ITEM.worldItem.spriteLocation.vz = m->t[2];

	renderOverworldItem(&TAMER_ITEM.worldItem);
}

int8_t getEquippedSlot(void)
{
	uint8_t moveId;
	uint8_t column;
	uint8_t row;
	int32_t slot;

	column = (MOVE_SELECT_BOX_X - 0x73) / 18;
	row = (MOVE_SELECT_BOX_Y - 0x6f) / 15;
	if (row == 1) {
		row = 5;
	} else if (row == 2) {
		row = 1;
	} else if (row == 3) {
		row = 4;
	} else if (row == 4) {
		row = 2;
	} else if (row == 5) {
		row = 3;
	}
	moveId = column + row * 8;
	if (moveId >= 0x30) {
		moveId++;
	}
	for (slot = 0; slot < 3; slot++) {
		if ((moveId == 0x2c) && (EQUIPPED_MOVES[slot] == 0x30)) {
			return slot;
		}
		if ((moveId == 0x37) && (EQUIPPED_MOVES[slot] == 0x39)) {
			return slot;
		}
		if (EQUIPPED_MOVES[slot] == moveId) {
			return slot;
		}
	}
	return -1;
}

void equipMove(void)
{
	RECT textArea;
	uint8_t column;
	uint8_t row;
	uint8_t moveId;
	int32_t slot;
	int32_t animation;

	column = (MOVE_SELECT_BOX_X - 0x73) / 18;
	row = (MOVE_SELECT_BOX_Y - 0x6f) / 15;
	if (row == 1) {
		row = 5;
	} else if (row == 2) {
		row = 1;
	} else if (row == 3) {
		row = 4;
	} else if (row == 4) {
		row = 2;
	} else if (row == 5) {
		row = 3;
	}
	moveId = column + row * 8;
	if (row == 6) {
		moveId++;
	}
	if (hasMove(moveId) == 0) {
		playSound(0, 4);
		return;
	}

	for (animation = 0; animation < 16; animation++) {
		if (moveId == 0x2c) {
			if (DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type)
			            .moves[animation] == 0x2c) {
				break;
			}
			if (DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type)
			            .moves[animation] == 0x30) {
				moveId = 0x30;
				break;
			}
		}
		if (moveId == 0x37) {
			if (DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type)
			            .moves[animation] == 0x37) {
				break;
			}
			if (DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type)
			            .moves[animation] == 0x39) {
				moveId = 0x39;
				break;
			}
		}
		if (moveId ==
		    DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type)
		            .moves[animation]) {
			break;
		}
		if (animation == 15) {
			playSound(0, 4);
			return;
		}
	}

	for (slot = 0; slot < 3; slot++) {
		if (EQUIPPED_MOVES[slot] == 0xff) {
			break;
		}
		if (slot == 2) {
			playSound(0, 4);
			return;
		}
	}
	EQUIPPED_MOVES[slot] = moveId;
	PARTNER_ENTITY.digimonEntity.stats.base.moves[slot] = animation + 0x2e;
	setRECT(&textArea, 0, slot * 12 + 0x18, 0x84, 0xc);
	clearTextSubArea(&textArea);
	drawStringMode(MOVE_NAMES[moveId], 0, slot * 12 + 0x18,
		              fitStringMode(MOVE_NAMES[moveId], MOVE_NAME_WIDTH));
	playSound(0, 3);
}

int32_t loadCardImage(id)
int8_t id;
{
	return loadStackedTIMEntry(MAIN_D_80123E78, (u_long)TEXTURE_BUFFER,
	                           id * 0xe, 0xe);
}

void renderCardImage(void)
{
	renderRectPolyFT4(-0x4b, -0x54, 0x96, 0xb4, 0, 0, GetTPage(1, 0, 576, 256), GetClut(448, 511), 4, 0);
}

void renderCardCount(void)
{
	renderNumber(0, 0x59, 0x4b, 1, getCardAmount(SELECTED_CARD), 3);
	renderString(0, 0x6b, 0x4c, 0xc, 0xc, 0xd8, 0xc, 0);
}

/* MOD (status bars): the stats view's happiness or discipline bar (50 x 6 at x -0x54) drawn like the field's
   (renderStatusBarFill, partner_impl.c): one fill over the whole range on a grey track, red below the midpoint and
   green from it, where the icons change, the top row lighter. Sorted fill first: the later box is drawn under it.
   A bar where more is worse (tiredness) has the colours the other way round: red from the midpoint */
static void renderMenuStatusBar(int32_t y, int32_t value, int32_t max, int32_t moreIsWorse)
{
	int32_t w;
	uint8_t r;
	uint8_t g;

	if (value < 0)
		value = 0;
	if (value > max)
		value = max;
	w = value * 0x32 / max;
	if ((value * 2 < max) != (moreIsWorse != 0)) {
		r = 0xE0;
		g = 0x30;
	} else {
		r = 0x30;
		g = 0xC8;
	}
	if (w > 0) {
		renderBox(-0x54, y, w, 1, r + (0xFF - r) / 3, g + (0xFF - g) / 3, 0x60, 0, 5);
		renderBox(-0x54, y + 1, w, 5, r, g, 0x28, 0, 5);
	}
	renderBox(-0x54, y, 0x32, 1, 0x70, 0x70, 0x70, 0, 5);
	renderBox(-0x54, y + 1, 0x32, 5, 0x58, 0x58, 0x58, 0, 5);
}

/* MOD (care numbers): with NEW STATUS BARS the stats view's left block holds four rows instead of three, tiredness third,
   each with its number beside the bar, and the row of the age and weight adds the care mistakes. The divider moves to the
   middle of the screen to make the room, and ATRIBUTOS keeps all its numbers in the rest. Off, the screen is the original's.
   The labels are drawn by drawDigimonStatsStrings into a free row of the text area */
#define STATS_CARE_TEXT_V   0x54
#define STATS_PANEL_DX      26
#define STATS_DIVIDER_X     (-0x1b)
#define STATS_ATTR_TOP      (-0x12)
#define STATS_HP_W          0x64
#define STATS_HP_W_NARROW   78
#define STATS_MAX_X         0x5f
#define STATS_MAX_DX        15
#define STATS_NUM_BOX_X     127
#define STATS_NUM_BOX_W     30
static int16_t STATS_CARE_LABEL_W[2];
static char *STATS_CARE_LABELS[2] = { STATUS_STR_TIRED, CHART_STR_MISTAKES };

/* where an element of the tables goes: their entries differ per language, the panel they lie in does not */
static int32_t statsPanelX(int32_t x, int32_t y)
{
	return MOD_OPT_BARS && x > STATS_DIVIDER_X && y > STATS_ATTR_TOP ? x + STATS_PANEL_DX : x;
}

/* the left block's rows, in the tables' own entries (the same in every language): once, when the screen opens */
static void statsCareLayout(int32_t bars)
{
	int32_t i;
	int32_t x;

	STATS_VIEW_ELEMENTS[17].posY = bars ? 0x30 : 0x31;
	STATS_VIEW_TEXT[1].posY = bars ? 0x3c : 0x40;
	STATS_VIEW_TEXT[2].posY = STATS_VIEW_ELEMENTS[10].posY = STATS_VIEW_ELEMENTS[20].posY = bars ? 0x54 : 0x4f;
	STATS_VIEW_INSETS[4].y = bars ? 170 : 171;
	STATS_VIEW_INSETS[5].y = bars ? 182 : 186;
	STATS_VIEW_INSETS[6].y = bars ? 206 : 201;
	x = STATS_DIVIDER_X + (bars ? STATS_PANEL_DX : 0);
	STATS_VIEW_LINES[0].x1 = STATS_VIEW_LINES[0].x2 = STATS_VIEW_LINES[2].x1 = x;
	STATS_VIEW_LINES[1].x1 = STATS_VIEW_LINES[1].x2 = x + 1;
	STATS_VIEW_LINES[2].x2 = x + 2;
	for (i = 7; i < 13; i++) {
		STATS_VIEW_INSETS[i].x = bars ? 194 + STATS_PANEL_DX : 194;
	}
	STATS_VIEW_INSETS[7].w = STATS_VIEW_INSETS[8].w = bars ? STATS_HP_W_NARROW + 4 : STATS_HP_W + 4;
}

/* a row's number in its box beside the bar. Sorted: what is drawn later lies under */
static void renderStatsCareNumber(int32_t row, int32_t value)
{
	renderNumber(0, STATS_NUM_BOX_X + STATS_NUM_BOX_W - 25 - 0xa0, 0x31 + row * 12, 3, value, 5);
	renderInsetBox(STATS_NUM_BOX_X, 168 + row * 12, STATS_NUM_BOX_W, 12, 5);
}

/* the original bar: above the midpoint a fill over its track, below it the fill alone in the track's colours */
static void renderOriginalStatsBar(int32_t y, int32_t w, int32_t high)
{
	renderRectPolyFT4(-0x54, y, (uint8_t)w, 6, high ? 0x18 : 0x4a, 0xf0, 0x18, GetClut(0x70, high ? 0x1f6 : 0x1f5), 5, 0);
	if (high) {
		renderRectPolyFT4(-0x54, y, 0x32, 6, 0x4a, 0xf0, 0x18, GetClut(0x70, 0x1f5), 5, 0);
	}
}

void renderDigimonStatsView(void)
{
	RECT *r;
	StringRect *sr;
	StatsIconClutTable cluts;
	IconRect *icon;
	int32_t i;
	int32_t j;
	int16_t w;
	int16_t k;
	int16_t special;
	int16_t clut;
	int32_t attrX;
	int32_t maxX;
	int32_t hpW;
	uint8_t frame;

	cluts = STATS_VIEW_ELEMENT_CLUT;
	switch (MENU_STATE) {
	case 0:
		if (drawDigimonStatsStrings() == 1) {
			MENU_STATE = 1;
		}
		break;
	case 1:
		renderSeparatorLines(STATS_VIEW_LINES, 6, 5);
		for (j = 0; j < 9; j++) {
			sr = &STATS_VIEW_TEXT[j];
			renderString(3, statsPanelX(sr->posX, sr->posY), sr->posY + 1, sr->uvWidth, 0xc, sr->uvX, sr->uvY, 5, 1);
		}
		for (j = 0; j < 0x15; j++) {
			icon = &STATS_VIEW_ELEMENTS[j];
			renderRectPolyFT4(statsPanelX(icon->posX, icon->posY), icon->posY, icon->width, icon->height, icon->texX,
			                  icon->texY + 0x80, 5, GetClut(0x60, cluts.v[j] + 0x1e8), 5, 0);
		}
#if !VERSION_IS(US)
		renderString(0, -0x6d, -0x42, 0x48, 0xc, 0, 0x3c, 5, 0);
		w = strlen(DIGIMON_NAME(ENTITY_TABLE[1]->type));
		renderString(0, -0x6d, -0x33, (w / 2) * 12, 0xc, 0x9c, 0x24, 5, 0);
#else
		/* MOD (optional name): an unnamed partner's name is its species', as long as the species row's */
		renderString(0, -0x6d, -0x42, modPartnerNameIsSpecies(ENTITY_TABLE[1]->type) ? modStatusNameWidth(PARTNER_ENTITY.name) : 0x48,
			     0xc, 0, 0x3c, 5, 0);
		/* MOD (long names): drawn tighter by drawDigimonStatsStrings when it does not fit */
		renderString(0, -0x6c, -0x32, modStatusNameWidth(DIGIMON_NAME(ENTITY_TABLE[1]->type)), 0xc, 0, 0x48, 5, 0);
#endif
		attrX = 0x23 + (MOD_OPT_BARS ? STATS_PANEL_DX : 0);
		maxX = STATS_MAX_X + (MOD_OPT_BARS ? STATS_MAX_DX : 0);
		hpW = MOD_OPT_BARS ? STATS_HP_W_NARROW : STATS_HP_W;
		renderNumber(0, -0x6d, -0x22, 2, PARTNER_PARA.age, 5);
#if !VERSION_IS(US)
		renderNumber(0, -0x26, -0x22, 2, PARTNER_PARA.weight, 5);
		renderNumber(0, 0x23, 3, 4, PARTNER_ENTITY.digimonEntity.stats.current.currentHP, 5);
		renderString(0, 0x53, 4, 0xc, 0xc, 0x3c, 0x30, 5, 0);
		renderNumber(0, 0x5f, 3, 4, PARTNER_ENTITY.digimonEntity.stats.base.hp, 5);
		renderNumber(0, 0x23, 0x12, 4, PARTNER_ENTITY.digimonEntity.stats.current.currentMP, 5);
		renderString(0, 0x53, 0x13, 0xc, 0xc, 0x3c, 0x30, 5, 0);
		renderNumber(0, 0x5f, 0x12, 4, PARTNER_ENTITY.digimonEntity.stats.base.mp, 5);
		renderNumber(0, 0x23, 0x20, 4, PARTNER_ENTITY.digimonEntity.stats.base.off, 5);
		renderNumber(0, 0x23, 0x30, 4, PARTNER_ENTITY.digimonEntity.stats.base.def, 5);
		renderNumber(0, 0x23, 0x3f, 4, PARTNER_ENTITY.digimonEntity.stats.base.speed, 5);
		renderNumber(0, 0x23, 0x4e, 4, PARTNER_ENTITY.digimonEntity.stats.base.brain, 5);
#else
		renderNumber(0, -0x23, -0x22, 2, PARTNER_PARA.weight, 5);
		renderNumber(0, attrX, 1, 4, PARTNER_ENTITY.digimonEntity.stats.current.currentHP, 5);
		renderNumber(0, maxX, 1, 4, PARTNER_ENTITY.digimonEntity.stats.base.hp, 5);
		renderNumber(0, attrX, 0x10, 4, PARTNER_ENTITY.digimonEntity.stats.current.currentMP, 5);
		renderNumber(0, maxX, 0x10, 4, PARTNER_ENTITY.digimonEntity.stats.base.mp, 5);
		renderNumber(0, attrX, 0x1f, 4, PARTNER_ENTITY.digimonEntity.stats.base.off, 5);
		renderNumber(0, attrX, 0x2e, 4, PARTNER_ENTITY.digimonEntity.stats.base.def, 5);
		renderNumber(0, attrX, 0x3d, 4, PARTNER_ENTITY.digimonEntity.stats.base.speed, 5);
		renderNumber(0, attrX, 0x4c, 4, PARTNER_ENTITY.digimonEntity.stats.base.brain, 5);
#endif
		renderDigimonStatsBar(PARTNER_ENTITY.digimonEntity.stats.base.hp, 0x270f, hpW, attrX + 1, 0xc);
		renderDigimonStatsBar(PARTNER_ENTITY.digimonEntity.stats.base.mp, 0x270f, hpW, attrX + 1, 0x1b);
		renderDigimonStatsBar(PARTNER_ENTITY.digimonEntity.stats.base.off, 0x3e7, 0x32, attrX + 1, 0x2a);
		renderDigimonStatsBar(PARTNER_ENTITY.digimonEntity.stats.base.def, 0x3e7, 0x32, attrX + 1, 0x39);
		renderDigimonStatsBar(PARTNER_ENTITY.digimonEntity.stats.base.speed, 0x3e7, 0x32, attrX + 1, 0x48);
		renderDigimonStatsBar(PARTNER_ENTITY.digimonEntity.stats.base.brain, 0x3e7, 0x32, attrX + 1, 0x57);
		for (i = 0, k = 0; i < 3; i++) {
			special = DIGI_PARA(ENTITY_TABLE[1]->type).special[i];
			if (special != 0xff) {
				clut = 0x7a06;
				if ((special == 2) || (special == 3) || (special == 4)) {
					clut = 0x7a46;
				} else if (special == 5) {
					clut = 0x7a86;
				}
				if (PLAYTIME_FRAMES % 10 < 5) {
					frame = 0;
				} else {
					frame = 0xc;
				}
				renderRectPolyFT4(k + 9, -0x33, 0xc, 0xc, frame + (special * 0x18 + 0x24), 0x80, 5, clut, 5, 0);
				k += 0xc;
			}
		}
		if (DIGI_PARA(ENTITY_TABLE[1]->type).type != 0) {
			renderRectPolyFT4(0x3b, -0x33, 0xc, 0xc, (DIGI_PARA(ENTITY_TABLE[1]->type).type - 1) * 0xc, 0x80, 5, 0x7a06, 5, 0);
		}
		if (RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).sleepCycle == 5) {
			renderString(0, 0x55, -0x33, 0x30, 0xc, 0xcc, 0x30, 5, 0);
		}
		if (RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).sleepCycle < 5) {
			renderString(0, 0x55, -0x33, 0x30, 0xc, RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).sleepCycle * 0x30, 0x18, 5, 0);
		}
		for (i = 0; i < 3; i++) {
			if (i < PARTNER_ENTITY.lives) {
				renderRectPolyFT4(i * 0xe - 0x54, 0x22, 0xc, 0xb, 0xe8, 0x8c, 5, 0x7b86, 5, 0);
			}
			renderRectPolyFT4(i * 0xe - 0x54, 0x22, 0xc, 0xb, 0xf4, 0x8c, 5, 0x7b86, 5, 0);
		}
		renderDigimonStatusConditions(PARTNER_PARA.condition);
		/* MOD (care numbers): each icon and original bar in one call, the choice in its arguments (heap) */
		renderRectPolyFT4(-0x8a, STATS_VIEW_ELEMENTS[17].posY, 0xb, 0xb, PARTNER_PARA.happiness >= 0 ? 0 : 0xb, 0xf4, 5,
		                  GetClut(0x60, 0x1f2), 5, 0);
		renderRectPolyFT4(-0x8a, STATS_VIEW_TEXT[1].posY, 0xb, 0xb, PARTNER_PARA.discipline >= 0x32 ? 0x16 : 0x21, 0xf4, 5,
		                  GetClut(0x60, 0x1f2), 5, 0);
		if (MOD_OPT_BARS) { /* MOD (options): NEW STATUS BARS, as on the field (renderStatusBars) */
			renderMenuStatusBar(0x34, PARTNER_PARA.happiness + 100, 200, 0);
			renderMenuStatusBar(0x40, PARTNER_PARA.discipline, 100, 0);
		} else {
			renderOriginalStatsBar(0x35, PARTNER_PARA.happiness >= 0 ? PARTNER_PARA.happiness / 2 : (PARTNER_PARA.happiness + 0x64) / 2,
			                       PARTNER_PARA.happiness >= 0);
			renderOriginalStatsBar(0x44, PARTNER_PARA.discipline >= 0x32 ? PARTNER_PARA.discipline - 0x32 : PARTNER_PARA.discipline,
			                       PARTNER_PARA.discipline >= 0x32);
		}
		renderBox(-0x54, MOD_OPT_BARS ? 0x57 : 0x52, PARTNER_PARA.virusBar * 3, 6, 0xc8, 0xc8, 0x3c, 0, 5);
		if (MOD_OPT_BARS) { /* MOD (care numbers): the new row, the condition bubble's own icon and the fields */
			renderRectPolyFT4(-0x8a, 0x48, 0xc, 0xc, 5 * 0xc + 0x30, 0x8c, 5, 0x7a46, 5, 0);
			renderString(3, -0x7c, 0x49, STATS_CARE_LABEL_W[0], 0xc, 0, STATS_CARE_TEXT_V, 5, 1);
			renderMenuStatusBar(0x4c, PARTNER_PARA.tiredness, 100, 1);
			renderInsetBox(75, 194, 50, 8, 5);
			renderStatsCareNumber(0, HAPPINESS_SHOWN(PARTNER_PARA.happiness));
			renderStatsCareNumber(1, PARTNER_PARA.discipline);
			renderStatsCareNumber(2, PARTNER_PARA.tiredness);
			renderStatsCareNumber(3, PARTNER_PARA.virusBar);
			/* the care mistakes, in a field like the weight's on the same cyan */
			w = STATS_CARE_LABEL_W[1];
			renderNumber(0, 268 - 0xa0 + 1, -0x22, 3, PARTNER_PARA.careMistakes, 5);
			renderString(17, 268 - 0xa0 - w - 3, -0x23, w, 0xc, 1 << 6, STATS_CARE_TEXT_V, 5, 0);
			renderBox(268 - 0xa0 - w - 4, -0x22, w + 2, 10, 0x40, 0xe0, 0xff, 0, 5);
			renderInsetBox(268, 84, 0x1a, 14, 5);
		}
		for (j = 0; j < 0xd; j++) {
			r = &STATS_VIEW_INSETS[j];
			renderInsetBox(r->x, r->y, r->w, r->h, 5);
		}
		renderDigiviceEntity(ENTITY_TABLE[1], 1);
		break;
	}
}

void renderDigimonMovesView(void)
{
	Move *mv;
	StringRect *sr;
	int32_t row;
	IconRect *icon;
	int32_t i;
	int32_t j;
	if (MENU_STATE > 0 && MENU_STATE < 7) {
		for (i = 0; i < 0xd; i++) {
			icon = &MOVES_VIEW_TECHSET_TEXT[i];
			renderRectPolyFT4(icon->posX,
			                  icon->posY,
			                  icon->width,
			                  icon->height,
			                  icon->texX,
			                  icon->texY + 0x80, 5, 0x7b06, 5, 0);
		}
		renderString(3, TECH_TITLE_CROPS[0].x, -0x57, TECH_TITLE_CROPS[0].w, 0xc, TECH_TITLE_CROPS[0].u, 0x48, 5, 1); /* MOD (language) */
		j = 0;
		while (j < 3) {
			if (EQUIPPED_MOVES[j] != 0xff) {
				mv = &MOVE_DATA[EQUIPPED_MOVES[j]];
				renderString(0, -0x7c, j * 0xf - 0x42, 0x78, 0xc, 0, j * 0xc + 0x18, 5, 1);
				renderNumber(0, 0x10, j * 0xf - 0x43, 3, mv->power, 5);
				renderNumber(0, 0x39, j * 0xf - 0x43, 3, mv->mpCost * 3, 5);
				if (mv->range != 0) {
#if !VERSION_IS(US)
					renderString(0, 0x65, j * 0xf - 0x42, 0xc, 0xc,
					             (mv->range - 1) * 0xc + 0x48, 0x6c, 5, 1);
#else
					renderString(0, 0x65, j * 0xf - 0x42, 0xc, 0xc,
					             (mv->range - 1) * 0x24, 0x78, 5, 1);
#endif
				}
				if (mv->status != 0) {
					renderRectPolyFT4(0x7d, j * 0xf - 0x43, 0xc, 0xc,
					                  (mv->status - 1) * 0xc, 0x8c, 5, 0x7a06, 5, 0);
				}
			}
			j++;
		}
		renderSeparatorLines(MOVES_TECHSET_LINES, 3, 3);
	}
	switch (MENU_STATE) {
	case 0:
		if (drawDigimonMovesText() == 1) {
			MENU_STATE = 1;
		}
		break;
	case 1:
		renderString(3, TECH_TITLE_CROPS[1].x, -0xf, TECH_TITLE_CROPS[1].w, 0xc, TECH_TITLE_CROPS[1].u, 0x48, 5, 1); /* MOD (language) */
		for (i = 0; i < 3; i++) {
			icon = &MOVES_VIEW_TECHSET_TEXT[i];
			renderRectPolyFT4(icon->posX, -9,
			                  icon->width,
			                  icon->height,
			                  icon->texX,
			                  icon->texY + 0x80, 5, 0x7b06, 5, 0);
		}
		renderRectPolyFT4(0x75, 4, 4, 4, 0x78, 0x8c, 5, 0x7b06, 5, 0);
		renderSeparatorLines(&MOVES_TECHSET_LINES[3], 3, 5);
		if (EQUIPPED_MOVES[3] != 0xff) {
			renderString(3, TECH_TITLE_CROPS[1].x, -0xf, TECH_TITLE_CROPS[1].w, 0xc, TECH_TITLE_CROPS[1].u, 0x48, 5, 1);
			renderString(0, -0x7c, 1, FINISHER_NAME_WIDTH, 0xc, 0, 0x3c, 5, 1);
			renderNumber(0, 0x10, 0, 3, modSpeciesMove(PARTNER_ENTITY.digimonEntity.entity.type, EQUIPPED_MOVES[3])->power, 5);
			renderString(0, 0x65, 1, 0xc, 0xc,
			             (MOVE_DATA[EQUIPPED_MOVES[3]].range - 1) * 0xc + 0x48, 0x6c, 5, 1);
			if (MOVE_DATA[EQUIPPED_MOVES[3]].status != 0) {
				renderRectPolyFT4(0x7d, 0, 0xc, 0xc,
				                  (MOVE_DATA[EQUIPPED_MOVES[3]].status - 1) * 0xc, 0x8c, 5,
				                  0x7a06, 5, 0);
			}
		}
#if !VERSION_IS(US)
		renderString(0xe, -0x82, 0x19, 0xc, 0xc, 0, 0x54, 5, 1);
		renderString(0, -0x76, 0x19, 0xa8, 0xc, 0xc, 0x54, 5, 1);
		renderString(0, -0x82, 0x27, 0x84, 0xc, 0, 0x60, 5, 1);
		renderString(0xf, 2, 0x27, 0xc, 0xc, 0x84, 0x60, 5, 1);
#else
		renderString(0xf, -0x82, 0x19, 0xc, 0xc, 0, 0x54, 5, 1);
		renderString(0, -0x76, 0x19, 0xa8, 0xc, 0xc, 0x54, 5, 1);
		renderString(0, -0x76, 0x27, 0x84, 0xc, 0xc, 0x60, 5, 1);
		renderString(7, -0x82, 0x27, 0xc, 0xc, 0, 0x60, 5, 1);
#endif
		renderString(0, 0xe, 0x27, 0x18, 0xc, 0x90, 0x60, 5, 1);
		break;
	case 2:
	case 4:
		UI_BOX_DATA[1].finalPos.h -= 0x27;
		if (UI_BOX_DATA[1].finalPos.h < 0x4a) {
			++MENU_STATE;
		}
		break;
	case 3:
	case 5:
		UI_BOX_DATA[1].finalPos.h += 0x27;
		if (0xbd < UI_BOX_DATA[1].finalPos.h) {
			MENU_STATE = (MENU_STATE == 3) ? 6 : 1;
		}
		break;
	case 6:
		renderString(3, TECH_TITLE_CROPS[2].x, -0xf, TECH_TITLE_CROPS[2].w, 0xc, TECH_TITLE_CROPS[2].u, 0x48, 5, 1); /* MOD (language) */
		renderSeparatorLines(MOVES_VIEW_HELP_LINES, 0xc, 5);
		for (i = 0; i < 4; i++) {
			icon = &MOVES_VIEW_HELP_CORNERS[i];
			renderRectPolyFT4(icon->posX,
			                  icon->posY,
			                  icon->width,
			                  icon->height,
			                  icon->texX,
			                  icon->texY + 0x80, 5, 0x7b06, 5, 0);
		}
		renderString(0, TECH_GRID_TITLE_CROP.x, 0x20, TECH_GRID_TITLE_CROP.w, 0xc, TECH_GRID_TITLE_CROP.u, 0, 5, 1); /* MOD (language) */
		renderString(0, TECH_HELP_CROP.x, 0x2e, TECH_HELP_CROP.w, 0xc, TECH_HELP_CROP.u, 0x6c, 5, 1); /* MOD (language) */
#if !VERSION_IS(US)
		renderString(0x10, -0x80, 0x4c, 0xc, 0xc, 0x24, 0x6c, 5, 1);
		renderString(0, -0x74, 0x4c, 0x18, 0xc, 0x30, 0x6c, 5, 1);
#else
		renderString(0xc, -0x74, 0x4c, 0x18, 0xc, 0x30, 0x6c, 5, 1);
#endif
		renderRectPolyFT4(-0x74, 0x3c, 10, 10, 0x80, 0x8c, 5, 0x7b06, 5, 0);
		j = 0;
		while (j < 7) {
			icon = &MOVES_VIEW_ELEMENT_NAMES[j];
			renderRectPolyFT4(icon->posX, j * 0xf - 8, icon->width, icon->height, icon->texX, icon->texY, 5, 0x7b06, 5, 0);
			renderRectPolyFT4(100, j * 0xf - 8, icon->width, icon->height, icon->texX, icon->texY, 5, 0x7b06, 5, 0);
			j++;
		}
		j = 0;
		while (j < 8) {
			renderRectPolyFT4(j * 0x12 - 0x26, -0xf, 4, 5, j * 4 + 0x94, 0x8c, 5, 0x7b06, 5, 0);
			j++;
		}
		renderRectPolyFT4(MOVE_SELECT_BOX_X - 0xa0,
		                  MOVE_SELECT_BOX_Y - 0x78, 0x12, 0x10, 0xc0, 0x8c, 5, 0x7b06, 5, 0);
		for (row = 0; row < 7; row++) {
			for (j = 0; j < 8; j++) {
				renderBox(j * 0x12 - 0x2a, row * 0xf - 7, 0xc, 0xc,
				          0x4e, 0x60, 0x6e, 0x80, 5);
			}
		}
		if (0xa8 < MOVE_SELECT_BOX_X) {
			renderDigimonMovesSelected(0);
		} else {
			renderDigimonMovesSelected(1);
		}
		renderDigimonMoveBox();
		break;
	case 7:
		if (drawMoveViewHelpStrings() == 1) {
			MENU_STATE = 8;
		}
		break;
	case 8:
		drawLine2P(0x20202, -0x92, -4, 0x92, -4, 5, 0);
		drawLine2P(0xfad990, -0x93, -3, 0x93, -3, 5, 0);
		drawLine2P(0x20202, -0x92, -2, 0x92, -2, 5, 0);
#if !VERSION_IS(US)
		renderString(3, -0x8e, -0x52, 0x54, 0xc, 0, 0x30, 5, 1);
#else
		renderString(3, -0x8e, -0x52, 0xa0, 0xc, 0, 0x30, 5, 1);
#endif
		renderString(0xe, -0x72, -0x32, 0xc, 0xc, 0xcc, 0x30, 5, 1);
		renderString(0xe, 6, -0x21, 0xc, 0xc, 0xcc, 0x30, 5, 1);
		renderString(0xe, -0x2a, -0x13, 0xc, 0xc, 0xcc, 0x30, 5, 1);
		for (i = 0; i < 0xc; i++) {
			sr = &MOVES_VIEW_STRING_SPRITES[i];
			renderString(0, sr->posX,
			             sr->posY + 1,
			             sr->uvWidth, 0xc,
			             sr->uvX,
			             sr->uvY, 5, 1);
		}
		renderString(0, 0x42, 0x33, 0x3c, 0xc, 0x78, 0x48, 5, 1);
		renderString(0, 0x42, 0x43, 0x3c, 0xc, 0x84, 0x54, 5, 1);
		renderRectPolyFT4(-0x81, -0x41, 0xc, 0xc, 0xb4, 0x8c, 5, 0x7b06, 5, 0);
		renderRectPolyFT4(-0x81, -0x22, 0xc, 0xc, 0xb4, 0x8c, 5, 0x7b06, 5, 0);
		renderRectPolyFT4(-0x7b, 0x50, 0x12, 0x10, 0xc0, 0x8c, 5, 0x7b06, 5, 0);
		for (i = 0; i < 7; i++) {
			icon = &MOVES_VIEW_SPRITES[i];
#if VERSION_IS(US)
			if (i != 2) {
#endif
				renderRectPolyFT4(icon->posX, icon->posY, icon->width, icon->height,
				                  icon->texX, icon->texY + 0x80, 5,
				                  (i == 5) ? getClut(96, 489) : getClut(96, 488), 5, 0x80);
#if VERSION_IS(US)
			}
#endif
		}
#if !VERSION_IS(US)
		renderBox(-0x18, -0x22, 0xc, 0xc, 200, 0, 0x28, 0x81, 4);
#else
		renderBox(0x52, -0x22, 0xc, 0xc, 200, 0, 0x28, 0x81, 4);
#endif
		renderBox(-0x78, 2, 0xc, 0xc, 200, 0, 0x28, 0x81, 4);
		renderBox(-0x78, 0x22, 0xc, 0xc, 0x4e, 0x60, 0x6e, 0x80, 4);
		renderBox(-0x78, 0x32, 0xc, 0xc, 0x68, 0x68, 0x68, 0x83, 4);
		renderBox(-0x78, 0x42, 0xc, 0xc, 0x68, 0x68, 0x68, 0x83, 4);
		break;
	case 9:
		if (drawDigimonMovesText() == 1) {
			MENU_STATE = 6;
		}
	}
}

void renderMenuTab(int16_t x, int8_t w, int8_t layer)
{
	int32_t i;
	int8_t h;

	h = 0x10;
	if (layer == 1) {
		h--;
	}
	renderRectPolyFT4(x, -0x68, 7, h, 0xd4, 0x8c, 5, GetClut(0x60, 0x1ec), 5, layer);
	for (i = 0; i < (w - 0xe) / 4; i++) {
		renderRectPolyFT4(x + 7 + i * 4, -0x68, 4, h, 0xe2, 0x8c, 5, GetClut(0x60, 0x1ec), 5, layer);
	}
	renderRectPolyFT4(x + w - 9, -0x68, 7, h, 0xdb, 0x8c, 5, GetClut(0x60, 0x1ec), 5, layer);
}

void renderPlayerInfoView(void)
{
	int32_t i;
	StringRect *e;
	RECT *r;
	int32_t j;
	int32_t k;
	int32_t n;
	int8_t count;

	switch (MENU_STATE) {
	case 0:
		if (drawPlayerInfoStrings() == 1) {
			MENU_STATE = 1;
		}
		break;
	case 1:
		renderSeparatorLines(TAMER_VIEW_PLAYER_LINES, 0xb, 5);

		for (i = 0; i < 0xb; i++) {
			e = &TAMER_WINDOW_NAME_STRING[i];
			renderString(3 - ((i / 7) * 3), e->posX, e->posY + 1, e->uvWidth, 0xc, e->uvX,
			             e->uvY, 5, 1);
		}

		renderString(0, -0x54, -0x50, 0x48, 0xc, 0, 0x30, 5, 1);
		renderNumber(0, 0x35, -0x36, 2, TAMER_ENTITY.tamerLevel, 5);
#if !VERSION_IS(US)
		renderString(0, 0x28, -0x25, 0x30, 0xc, 0, (TAMER_ENTITY.tamerLevel * 12) + 0x40, 5, 1);
#else
		renderString(0, 0x28, -0x24, 0x64, 0xc, 0, (TAMER_ENTITY.tamerLevel * 12) + 0x40, 5, 1);
#endif
		n = strlen(TAMER_LEVEL_TITLES[TAMER_ENTITY.tamerLevel]) / 2;
		renderString(0, (n * 12) + 0x28, -0x25, 0x30, 0xc, 0xb4, 0x18, 5, 1);
		renderNumber(0, 0x35, -0x11, 2, TAMER_ENTITY.raisedCount, 5);
		renderNumber(0, 0x29, 2, 6, MONEY, 5);
#if !VERSION_IS(US)
		renderNumber(0, 0x29, 0x16, 3, PLAYTIME_HOURS, 5);
		renderNumber(0, 0x59, 0x16, 2, PLAYTIME_MINUTES, 5);
#else
		renderString(0, 0x32, 0x16, 0x4c, 0xc, 0, 0xe4, 5, 1);
#endif

		for (i = 0; i < 2; i++) {
			if (isTriggerSet(i + 0x2d) == 1) {
				renderRectPolyFT4(i * 0x15 + 0x34, 0x37, 0x10, 0x10, i * 0x10 + 0xb0, 0xa0, 5, 0x7bc6, 5, 0);
			}
		}

		if (isTriggerSet(0x2f) == 1) {
			renderRectPolyFT4(0x5e, 0x37, 0x10, 0x10, 0xe0, 0xa0, 5, 0x7bc6, 5, 0);
		}

		if (isTriggerSet(0x30) == 1) {
			renderRectPolyFT4(0x73, 0x37, 0x10, 0x10, 0xd0, 0xa0, 5, 0x7bc6, 5, 0);
		}

		count = 0;
		for (j = 0; j < 0x12; j++) {
			if (hasMedal(j) != 0) {
				count++;
			}
		}

		renderNumber(0, 0x35, 0x4d, 2, count, 5);
		renderDigiviceEntity(ENTITY_TABLE[0], 0);

		for (k = 0; k < 0xb; k++) {
			r = &TAMER_WINDOW_BOXES[k];
			renderInsetBox(r->x, r->y, r->w, r->h, 5);
		}
		break;
	}
}

void renderDigimonMovesSelected(int16_t panel)
{
	RECT rect;
	IconRect *label;
	Move *move;
	int32_t i;
	uint8_t row;
	uint8_t col;
#if !VERSION_IS(US)
	uint8_t status;
#endif
	uint8_t moveId;

	if (panel == 0) {
		renderSeparatorLines(DIGIMON_MOVE_INFO_LINES_LEFT, 0x12, 4);
	} else {
		renderSeparatorLines(DIGIMON_MOVE_INFO_LINES_RIGHT, 0x12, 4);
	}

	renderRectPolyFT4((panel * 0x9d) - 0x91, 1, 4, 4, 0x78, 0x90, 5, 0x7b06, 4, 0);
	renderRectPolyFT4((panel * 0x9d) - 0x10, 1, 4, 4, 0x7c, 0x90, 5, 0x7b06, 4, 0);
	renderRectPolyFT4((panel * 0xcb) - 0x91, 0x5c, 4, 4, 0x78, 0x94, 5, 0x7b06, 4, 0);
	renderRectPolyFT4((panel * 0xcb) - 0x3e, 0x5c, 4, 4, 0x7c, 0x94, 5, 0x7b06, 4, 0);
	if (panel == 0) {
		renderRectPolyFT4(-0x10, 0x15, 4, 4, 0x7c, 0x94, 5, 0x7b06, 4, 0);
	} else {
		renderRectPolyFT4(0xc, 0x15, 4, 4, 0x78, 0x94, 5, 0x7b06, 4, 0);
	}
	renderBox((panel * 0x9d) - 0x8f, 3, 0x80, 0x13, 0x32, 0x32, 0x80, 0, 4);
	renderBox((panel * 0xcb) - 0x8f, 0x18, 0x57, 0x45, 0x32, 0x32, 0x80, 0, 4);
	for (i = 0; i < 5; i++) {
		label = &MOVES_SELECTED_LABELS[i];
		renderRectPolyFT4(panel * (i == 0 ? 0x9d : 0xcb) + label->posX,
		                  label->posY,
		                  label->width,
		                  label->height,
		                  label->texX,
		                  label->texY, 5, 0x7b06, 3, 0);
	}

	for (i = 0; i < 4; i++) {
		renderRectPolyFT4((panel * 0xcb) - 0x71, i * 0x11 + 0x1b, 0xa, 0xa, 0x8a, 0x8c, 5, 0x7b06, 3, 0);
	}

	row = (MOVE_SELECT_BOX_X - 0x73) / 18;
	col = (MOVE_SELECT_BOX_Y - 0x6f) / 15;
	if (col == 1) {
		col = 5;
	} else if (col == 2) {
		col = 1;
	} else if (col == 3) {
		col = 4;
	} else if (col == 4) {
		col = 2;
	} else if (col == 5) {
		col = 3;
	}
	moveId = col * 8 + row;
	if (col == 6) {
		moveId++;
	}

	if (hasMove(moveId)) {
		move = modSpeciesMove(PARTNER_ENTITY.digimonEntity.entity.type, moveId);
		setRECT(&rect, 0, 0x84, 0x78, 0xc);
		clearTextSubArea(&rect);
		drawStringMode(MOVE_NAMES[moveId], 0, 0x84,
		              fitStringMode(MOVE_NAMES[moveId], MOVE_NAME_WIDTH));
		renderString(0, (panel * 0x9d) - 0x8a, 9, 0x78, 0xc, 0, 0x84, 3, 1);
		renderNumber(0, (panel * 0xcb) - 0x64, 0x1b, 3, move->power, 3);
		renderNumber(0, (panel * 0xcb) - 0x64, 0x2c, 3, move->mpCost * 3, 3);
		renderString(0, (panel * 0xcb) - 0x64, 0x3e, 0x24, 0xc, (move->range - 1) * 0x24, 0x78, 3, 1);
#if !VERSION_IS(US)
		status = move->status;
		if (status == 1) {
			renderString(0, (panel * 0xcb) - 0x64, 0x4f, 0xc, 0xc, 0x84, 0x6c, 3, 1);
		} else if (status == 4) {
			renderString(0, (panel * 0xcb) - 0x64, 0x4f, 0x24, 0xc, 0xc0, 0x6c, 3, 1);
		} else if (status != 0) {
			renderString(0, (panel * 0xcb) - 0x64, 0x4f, 0x18, 0xc, (status - 2) * 0x18 + 0x90, 0x6c, 3, 1);
		}
#else
		/* MOD (language): MOVE_STATUS_CROPS */
		if (move->status >= 1 && move->status <= 4) {
			renderString(0, (panel * 0xcb) - 0x64, 0x4f, MOVE_STATUS_CROPS[move->status - 1].w, 0xc,
			             MOVE_STATUS_CROPS[move->status - 1].u, 0x6c, 3, 1);
		}
#endif
	}
}

void renderDigimonMoveBox(void)
{
	int16_t moveId;
	int16_t moves[16];
	int32_t col;
	int32_t row;
	int32_t j;
	int16_t clut;
	uint8_t x;
	uint8_t special;
	int8_t blink;
	int32_t i;
	uint8_t y;

	for (i = 0; i < 3; i++) {
		if (EQUIPPED_MOVES[i] < 0x3a) {
			x = EQUIPPED_MOVES[i] % 8;
			if (EQUIPPED_MOVES[i] < 8) {
				y = 0;
			} else if (EQUIPPED_MOVES[i] < 0x10) {
				y = 2;
			} else if (EQUIPPED_MOVES[i] < 0x18) {
				y = 4;
			} else if (EQUIPPED_MOVES[i] < 0x20) {
				y = 5;
			} else if (EQUIPPED_MOVES[i] < 0x28) {
				y = 3;
			} else if (EQUIPPED_MOVES[i] < 0x30) {
				y = 1;
			} else {
				x = (EQUIPPED_MOVES[i] - 1) % 8;
				y = 6;
			}
			if (EQUIPPED_MOVES[i] == 0x30) {
				x = 4;
				y = 1;
			}
			if (EQUIPPED_MOVES[i] == 0x39) {
				x = 6;
				y = 6;
			}
			renderBox(x * 0x12 - 0x2a, y * 0xf - 7, 0xc, 0xc, 200, 0, 0x28, 1, 4);
		}
	}

	for (i = 0; i < 0x10; i++) {
		moves[i] = DIGI_PARA(ENTITY_TABLE[1]->type).moves[i];
	}
	sortArray(moves, 0x10);

	i = 0;
	for (row = 0; row < 7; row++) {
		for (col = 0; col < 8; col++) {
			moveId = row * 8 + col;
			if (moveId >= 0x30) {
				moveId++;
			}
			j = 0;
			if (moveId == 0x2c) {
				for (j = 0; j < 0x10; j++) {
					if (moves[j] == 0x2c || moves[j] == 0x30) {
						break;
					}
					if (j == 0xf) {
						renderBox(0x1e, 8, 0xc, 0xc, 0x68, 0x68, 0x68, 3, 4);
					}
				}
			}
			if (moveId == 0x37) {
				for (j = 0; j < 0x10; j++) {
					if (moves[j] == 0x37 || moves[j] == 0x39) {
						break;
					}
					if (j == 0xf) {
						renderBox(0x42, 0x53, 0xc, 0xc, 0x68, 0x68, 0x68, 3, 4);
					}
				}
			}
			if (moveId == moves[i]) {
				i++;
			} else if (moveId != 0x2c && moveId != 0x37 && moveId != 0x30 &&
			           moveId != 0x39) {
				if (moveId < 8) {
					y = 0;
				} else if (moveId < 0x10) {
					y = 2;
				} else if (moveId < 0x18) {
					y = 4;
				} else if (moveId < 0x20) {
					y = 5;
				} else if (moveId < 0x28) {
					y = 3;
				} else if (moveId < 0x30) {
					y = 1;
				} else {
					y = 6;
				}
				renderBox(col * 0x12 - 0x2a, y * 0xf - 7, 0xc, 0xc, 0x68, 0x68, 0x68, 3, 4);
			}
			if (hasMove(moveId) == 1) {
				special = modSpeciesMove(PARTNER_ENTITY.digimonEntity.entity.type, moveId)->special;
				switch (special) {
				case 0:
					clut = 0x7a06;
					break;
				case 1:
					clut = 0x7a06;
					break;
				case 2:
					clut = 0x7a46;
					break;
				case 3:
					clut = 0x7a46;
					break;
				case 4:
					clut = 0x7a46;
					break;
				case 5:
					clut = 0x7a86;
					break;
				case 6:
					clut = 0x7a06;
				}
				if (PLAYTIME_FRAMES % 0x14 < 10) {
					blink = 0;
				} else {
					blink = 0xc;
				}
				renderRectPolyFT4(col * 0x12 - 0x2a, special * 0xf - 7, 0xc, 0xc,
				                  blink + (special * 0x18 + 0x24), 0x80, 5, clut, 4, 0);
			}
		}
	}
}

int32_t drawMoveViewHelpStrings(void)
{
	RECT rect;

	rect = MAIN_D_801342F0;
	if (MENU_SUB_STATE == 0) {
		clearTextSubArea(&rect);
	}

	/* The original indexed STATUS_VIEW_LABELS[MENU_SUB_STATE + 9], past its 8 entries: the
	 * help lines are TECH_VIEW_LABELS[1..8], which follow it in the original's data. GCC
	 * places TECH_VIEW_LABELS elsewhere, so the help read garbage pointers and hung. */
	/* MOD (font): only the first help line is cut (around the button icon); the others are copied whole */
#if FONT_SWITCH
	if (MENU_SUB_STATE == 0) {
		DRAW_STRING_PIECES(TECH_VIEW_LABELS[MENU_SUB_STATE + 1], 0, MENU_SUB_STATE * 0xc + 0x18, 0, 0x71);
	} else {
		DRAW_STRING_PIECES(TECH_VIEW_LABELS[MENU_SUB_STATE + 1], 0, MENU_SUB_STATE * 0xc + 0x18, 0);
	}
#else
	drawString(TECH_VIEW_LABELS[MENU_SUB_STATE + 1], 0, MENU_SUB_STATE * 0xc + 0x18);
#endif
	MENU_SUB_STATE++;

	if (MENU_SUB_STATE == 8) {
		return 1;
	}

	return 0;
}

void sortArray(int16_t *arr, int8_t count)
{
	int32_t i;
	int32_t j;
	int16_t tmp;

	for (i = 0; i < count - 1; ++i) {
		for (j = i + 1; j < count; ++j) {
			if (arr[i] > arr[j]) {
				tmp = arr[i];
				arr[i] = arr[j];
				arr[j] = tmp;
			}
		}
	}
}

void removeTriangleMenu(void)
{
	removeStaticUIBox(0);
	removeObject(0xfa4, 0);
}

void removeUIBox1(void)
{
	removeStaticUIBox(1);
}

/*
 * MOD (portraits): the chart's sheet (SBOY.TIM at 512,256) has portraits for the original 61 only. The
 * detail view of a species shows up to 11 others (its "from" and "to" species of DIGI.DAT); their icons
 * are loaded from DIGI.DAT when the detail opens, into the VRAM of the battle effect slots (576,256 on,
 * idle outside battle): 12 pairs of 16x16 frames, each with a CLUT below.
 */
#define CHART_EXTRA_SLOTS	12
#define CHART_EXTRA_X		576
#define CHART_EXTRA_Y		256
#define CHART_EXTRA_CLUT_Y	496
#define CHART_EXTRA_TPAGE	((CHART_EXTRA_Y / 256) * 16 + CHART_EXTRA_X / 64)
static int16_t CHART_EXTRA_ID[CHART_EXTRA_SLOTS];

/*
 * MOD (portraits): the detail view can go from a species to one of its "from" or "to" boxes (D-pad moves a
 * cursor between them, X opens the species, TRIANGLE comes back), so it shows any species of the tree:
 * CHART_DETAIL_ID, the one in the middle (the overview's CHART_SELECTED_DIGIMON is where the box opened from).
 */
#define CHART_DETAIL_DEPTH_MAX	8
static int16_t CHART_DETAIL_ID;
static int16_t CHART_DETAIL_STACK[CHART_DETAIL_DEPTH_MAX];
static int8_t CHART_DETAIL_DEPTH;
static int8_t CHART_CURSOR_SIDE; /* 0 the "from" boxes, 1 the "to" boxes */
static int8_t CHART_CURSOR_J;	 /* the slot in that side's array */

/*
 * MOD (chart lists): the boxes of the detail view, every evolution the game can take into or out of the species in
 * the middle, not only its path row's slots: the row's "to" slots, a Baby II's Rookies past them
 * (getEvolutionExtraTargets), the Fresh and Ultimate pairs (MOD_EVO_FRESH, MOD_EVO_ULTIMATE); the row's "from" slots,
 * the pairs and extra lists into it, and the parents DIGI.DAT's chart block lists for a species without a path row.
 * In order, without repeats; with NEW DIGIMON off only the original's species. The "to" side shows CHART_TO_PAGE boxes
 * at a time: the cursor turns the page past the first or last box, a bar at the right shows where it is.
 */
#define CHART_TO_MAX	24
#define CHART_FROM_MAX	5
#define CHART_TO_PAGE	6
static int16_t CHART_TO_IDS[CHART_TO_MAX];
static int16_t CHART_FROM_IDS[CHART_FROM_MAX];
static int8_t CHART_TO_N;
static int8_t CHART_FROM_N;
static int8_t CHART_TO_TOP;	/* the first "to" box of the page shown */

static void chartListAdd(int16_t *list, int8_t *n, int32_t max, int32_t id)
{
	int32_t i;

	if (id <= 0 || id >= NUM_DIGIMON || !modSpeciesAllowed(id)) {
		return;
	}
	for (i = 0; i < *n; i++) {
		if (list[i] == id) {
			return;
		}
	}
	if (*n < max) {
		list[(*n)++] = id;
	}
}

static void chartBuildLists(int32_t id)
{
	EvolutionPath *path;
	DigimonId *extra;
	uint16_t *block;
	int32_t n;
	int32_t i;
	int32_t k;

	CHART_TO_N = CHART_FROM_N = CHART_TO_TOP = 0;
	path = getEvolutionPath(id);
	for (i = 0; i < 6; i++) {
		chartListAdd(CHART_TO_IDS, &CHART_TO_N, CHART_TO_MAX, path->to[i]);
	}
	for (i = 0; i < 5; i++) {
		chartListAdd(CHART_FROM_IDS, &CHART_FROM_N, CHART_FROM_MAX, path->from[i]);
	}
	n = getEvolutionExtraTargets(id, &extra);
	for (i = 0; i < n; i++) {
		chartListAdd(CHART_TO_IDS, &CHART_TO_N, CHART_TO_MAX, extra[i]);
	}
	for (k = 0; k < SPECIES_EXTRA_PARENTS; k++) {
		n = getEvolutionExtraParent(k, &extra);
		for (i = 0; n > 0 && i < SPECIES_EXTRA_TARGETS && extra[i] > 0; i++) {
			if (extra[i] == id) {
				chartListAdd(CHART_FROM_IDS, &CHART_FROM_N, CHART_FROM_MAX, n);
			}
		}
	}
	for (i = 0; i < MOD_EVO_PAIR_MAX * 2; i++) {
		int16_t *pair = i < MOD_EVO_PAIR_MAX ? MOD_EVO_FRESH[i] : MOD_EVO_ULTIMATE[i - MOD_EVO_PAIR_MAX];

		if (pair[0] == id) {
			chartListAdd(CHART_TO_IDS, &CHART_TO_N, CHART_TO_MAX, pair[1]);
		} else if (pair[1] == id) {
			chartListAdd(CHART_FROM_IDS, &CHART_FROM_N, CHART_FROM_MAX, pair[0]);
		}
	}
	/* the block's {child, parent} pairs after the tabs' counts and ids (tools/species.py chart_parents) */
	block = (uint16_t *)readSpeciesChart(0, 0);
	if (block != NULL) {
		k = SPECIES_CHART_TABS;
		for (i = 0; i < SPECIES_CHART_TABS; i++) {
			k += block[i];
		}
		n = block[k];
		for (i = 0; i < n && k + 2 + i * 2 < 0x400; i++) {
			if (block[k + 1 + i * 2] == id) {
				chartListAdd(CHART_FROM_IDS, &CHART_FROM_N, CHART_FROM_MAX, block[k + 2 + i * 2]);
			}
		}
	}
}

/* the boxes of a side as drawn: the first index of its list, how many, and the layout's first slot (centred) */
static int32_t chartSide(int32_t side, int32_t *first, int32_t *slot)
{
	int32_t n;

	if (side == 0) {
		n = CHART_FROM_N;
		*first = 0;
		*slot = ((n % 2) == 0 ? 4 - n : 5 - n) / 2;
	} else {
		*first = CHART_TO_TOP;
		n = CHART_TO_N - CHART_TO_TOP;
		if (n > CHART_TO_PAGE) {
			n = CHART_TO_PAGE;
		}
		*slot = ((n % 2) == 0 ? 6 - n : 5 - n) / 2;
	}
	return n;
}

/* the icons of the species in the middle and of the boxes shown (at most 1 + 5 + 6 = CHART_EXTRA_SLOTS) */
static void loadChartPortraits(int32_t id)
{
	int32_t n;
	int32_t i;
	int32_t other;
	int32_t first;
	int32_t slot;
	int32_t to;

	for (i = 0; i < CHART_EXTRA_SLOTS; i++) {
		CHART_EXTRA_ID[i] = 0;
	}
	to = chartSide(1, &first, &slot);
	n = 0;
	for (i = -1; i < CHART_FROM_N + to && n < CHART_EXTRA_SLOTS; i++) {
		other = i < 0 ? id : (i < CHART_FROM_N ? CHART_FROM_IDS[i] : CHART_TO_IDS[first + i - CHART_FROM_N]);
		if (other <= NUM_EVO_CHART_DIGIMON || other >= NUM_DIGIMON) {
			continue;
		}
		CHART_EXTRA_ID[n] = other;
		loadSpeciesPortrait(other, CHART_EXTRA_X + (n % 8) * 8, CHART_EXTRA_Y + (n / 8) * 16, 16,
		                    CHART_EXTRA_X + (n % 8) * 16, CHART_EXTRA_CLUT_Y + n / 8);
		n++;
	}
}

/* draws the icon of a species outside the chart's 61 at (x, y) */
static void renderChartPortrait(int32_t id, int16_t x, int16_t y)
{
	int32_t i;

	for (i = 0; i < CHART_EXTRA_SLOTS; i++) {
		if (CHART_EXTRA_ID[i] == id) {
			renderRectPolyFT4(x, y, 0x10, 0x10, (i % 8) * 32, (i / 8) * 16, CHART_EXTRA_TPAGE,
			                  GetClut(CHART_EXTRA_X + (i % 8) * 16, CHART_EXTRA_CLUT_Y + i / 8), 4, 0);
			return;
		}
	}
}

/* the box j of a side's page: its species (0 for none) and position on the detail view, as renderEvoChartDetail lays them out */
static int32_t chartDetailBox(int32_t side, int32_t j, int16_t *x, int16_t *y)
{
	ChartSprite *sprites;
	int32_t n;
	int32_t first;
	int32_t slot;

	n = chartSide(side, &first, &slot);
	if (j < 0 || j >= n) {
		return 0;
	}
	if (side == 0) {
		sprites = (n % 2) == 0 ? CHART_FROM_SPRITES_EVEN : CHART_FROM_SPRITES_ODD;
	} else {
		sprites = (n % 2) == 0 ? CHART_TO_SPRITES_EVEN : CHART_TO_SPRITES_ODD;
	}
	*x = sprites[slot + j].posX;
	*y = sprites[slot + j].posY;
	return side == 0 ? CHART_FROM_IDS[j] : CHART_TO_IDS[first + j];
}

static void chartDetailMoveCursor(int32_t side, int32_t j)
{
	CHART_CURSOR_SIDE = side;
	CHART_CURSOR_J = j;
	playSound(0, 2);
}

/* the cursor on the first box of the species in the middle: the "to" side first */
static void chartDetailPlaceCursor(void)
{
	int16_t x;
	int16_t y;
	int32_t side;
	int32_t j;

	CHART_CURSOR_SIDE = 1;
	CHART_CURSOR_J = 0;
	for (side = 1; side >= 0; side--) {
		for (j = 0; j < CHART_TO_PAGE; j++) {
			if (chartDetailBox(side, j, &x, &y) != 0) {
				CHART_CURSOR_SIDE = side;
				CHART_CURSOR_J = j;
				return;
			}
		}
	}
}

static int8_t CHART_POPUP;
static int8_t CHART_CARD_SCROLL;
static int8_t CHART_CARD_COUNT;
#define CHART_CARD_VISIBLE 10
static int16_t CHART_CARD_ID;	/* MOD (chart criteria): the evolution whose card is drawn, -1 none */
static int16_t CHART_LIT_FOR;	/* the centre whose lit lines are in CHART_LIT, -1 none */

static void chartDetailOpen(int32_t id)
{
	CHART_POPUP = 0;
	CHART_CARD_ID = CHART_LIT_FOR = -1; /* MOD (chart criteria): the partner may have changed since the last time */
	CHART_DETAIL_ID = id;
	CHART_DETAIL_DEPTH = 0;
	chartBuildLists(id); /* MOD (chart lists) */
	chartDetailPlaceCursor();
	loadChartPortraits(id);
}

static void chartDetailGoto(int32_t id)
{
	CHART_DETAIL_ID = id;
	chartBuildLists(id); /* MOD (chart lists) */
	chartDetailPlaceCursor();
	loadChartPortraits(id);
	playSound(0, 3);
}

/* the input of the detail view; 1 when it took the key (the caller then does not close the view) */
static int32_t chartDetailInput(void)
{
	int16_t x;
	int16_t y;
	int16_t ox;
	int16_t oy;
	int32_t j;
	int32_t best;
	int32_t d;
	int32_t bestd;
	int32_t id;
	int32_t side;

	/* Modal input is consumed before navigation, confirmation and history. */
	if (CHART_POPUP) {
		if (isKeyDown(CANCEL_BUTTON) || isKeyDown(0x80)) {
			CHART_POPUP = 0;
			CHART_CARD_ID = -1;
			playSound(0, 4);
		}
		else if ((CHANGED_INPUT & 0x1000) && CHART_CARD_SCROLL > 0) CHART_CARD_SCROLL--;
		else if ((CHANGED_INPUT & 0x4000) && CHART_CARD_SCROLL + CHART_CARD_VISIBLE < CHART_CARD_COUNT - 1) CHART_CARD_SCROLL++;
		return 1;
	}
	if (MOD_OPT_EVO_GUIDE && CHART_CURSOR_SIDE == 1 && isKeyDown(0x80) &&
	    chartDetailBox(1, CHART_CURSOR_J, &x, &y) > 0) {
		CHART_POPUP = 1;
		CHART_CARD_SCROLL = 0;
		CHART_CARD_ID = -1;
		playSound(0, 3);
		return 1;
	}
	side = CHART_CURSOR_SIDE;
	if (chartDetailBox(side, CHART_CURSOR_J, &x, &y) == 0) {
		chartDetailPlaceCursor(); /* no box under the cursor (a species without boxes on that side) */
		side = CHART_CURSOR_SIDE;
		if (chartDetailBox(side, CHART_CURSOR_J, &x, &y) == 0) {
			return 0; /* no box at all: TRIANGLE closes */
		}
	}
	if (CHANGED_INPUT & 0x1000) { /* up; MOD (chart lists): past the first "to" box, the page before */
		if (CHART_CURSOR_J > 0) {
			chartDetailMoveCursor(side, CHART_CURSOR_J - 1);
		} else if (side == 1 && CHART_TO_TOP > 0) {
			CHART_TO_TOP -= CHART_TO_PAGE;
			chartDetailMoveCursor(1, CHART_TO_PAGE - 1);
			loadChartPortraits(CHART_DETAIL_ID);
		}
		return 1;
	}
	if (CHANGED_INPUT & 0x4000) { /* down; past the last, the next page */
		if (chartDetailBox(side, CHART_CURSOR_J + 1, &ox, &oy) != 0) {
			chartDetailMoveCursor(side, CHART_CURSOR_J + 1);
		} else if (side == 1 && CHART_TO_TOP + CHART_TO_PAGE < CHART_TO_N) {
			CHART_TO_TOP += CHART_TO_PAGE;
			chartDetailMoveCursor(1, 0);
			loadChartPortraits(CHART_DETAIL_ID);
		}
		return 1;
	}
	if (CHANGED_INPUT & (0x8000 | 0x2000)) { /* left, right: the other side, the box nearest in height */
		best = -1;
		bestd = 0x7fff;
		for (j = 0; j < CHART_TO_PAGE; j++) {
			if (chartDetailBox(1 - side, j, &ox, &oy) != 0) {
				d = oy > y ? oy - y : y - oy;
				if (d < bestd) {
					bestd = d;
					best = j;
				}
			}
		}
		if (best >= 0) {
			chartDetailMoveCursor(1 - side, best);
		}
		return 1;
	}
	if (isKeyDown(CONFIRM_BUTTON) != 0) {
		id = chartDetailBox(side, CHART_CURSOR_J, &x, &y);
		if (id > 0 && id < NUM_DIGIMON && hasDigimonRaised(id & 0xffff) == 1 &&
		    CHART_DETAIL_DEPTH < CHART_DETAIL_DEPTH_MAX) {
			CHART_DETAIL_STACK[(int32_t)CHART_DETAIL_DEPTH++] = CHART_DETAIL_ID;
			chartDetailGoto(id);
		} else {
			playSound(0, 4);
		}
		return 1;
	}
	if (isKeyDown(CANCEL_BUTTON) != 0 && CHART_DETAIL_DEPTH > 0) {
		chartDetailGoto(CHART_DETAIL_STACK[(int32_t)--CHART_DETAIL_DEPTH]);
		return 1;
	}
	return 0;
}

/*
 * MOD (chart): the overview shows every partner species of the build, not the sheet's 61: DIGI.DAT's chart block
 * (tools/species.py chart_tabs) lists them by tab (Fresh and In-Training, Rookie, Champion, Ultimate, Mega), each level
 * in the order of its parents. One tab is shown at a time (L1/R1 change it), CHART_ROWS rows of CHART_COLS boxes that
 * scroll a row at a time when the cursor goes past the top or bottom row, with a scroll bar at the band's right edge
 * (it was pages). The species of the sheet (1..61) keep their hand-drawn icon; the others are read from
 * the block into the battle effect VRAM (idle outside battle; the detail view's icons are in its first 32 rows, the
 * card viewer reloads over it and the rows are read again when the tab comes back), CHART_PAGE_SIZE slots used as a
 * ring by the species' place in the tab, so a scroll reads only the row that comes in. The bottom row names the raised
 * species under the cursor and counts the tab's raised species.
 */
void recalculateProsperity(void);
#define CITY_VIEW (PLAYER_MENU_STATE == 4)
/* a chart cell: the species, and for City the level and the hidden flag the disc packs with it, so drawing never reads a record */
#define CHART_CELL_ID(cell)	((cell) & 0x0fff)
#define CHART_CELL_LEVEL(cell)	(((cell) >> 12) & 7)

#define CHART_COLS		14
#define CHART_ROWS		7
#define CHART_PAGE_SIZE		(CHART_COLS * CHART_ROWS)
#define CHART_GRID_X		27	/* the first box's border, screen pixels */
#define CHART_GRID_Y		57
#define CHART_BAND_W		279	/* the tab's band, from x 20 */
#define CHART_PITCH		0x13
#define CHART_SCROLL_X		294	/* the scroll bar: past the last column's border (292), inside the band (299) */
#define CHART_SCROLL_W		3
#define CHART_SCROLL_H		(CHART_ROWS * CHART_PITCH - 1)
#define CHART_ICON_X		576	/* the page's icons: 16 a row, 4 halfwords each; each row's second idle frames below it */
#define CHART_ICON_Y		288
#define CHART_ICON_TPAGE	((CHART_ICON_Y / 256) * 16 + CHART_ICON_X / 64)
#define CHART_CLUT_X		640	/* their CLUTs: 4 a row */
#define CHART_CLUT_Y		288
#define CHART_LABEL_ROW		0x60	/* text area rows: the tab names, the name under the cursor, the counters */
#define CHART_NAME_ROW		0x6c
#define CHART_COUNT_ROW		0x78
#define CHART_KEYS_ROW		0x84

static char *CHART_TAB_NAMES[SPECIES_CHART_TABS] = { CHART_STR_BABY, CHART_STR_ROOKIE, CHART_STR_CHAMPION, CHART_STR_ULTIMATE,
						      CHART_STR_MEGA };
/* the band of each tab: light and dark edges (0xBBGGRR) and fill, the original's level colours (Mega: gold) */
static int32_t CHART_TAB_EDGES[SPECIES_CHART_TABS][2] = {
	{ 0xaaa0c8, 0x5a3c8c }, { 0xc88250, 0x5f370a }, { 0x6eaf5a, 0x285a00 }, { 0x6996d2, 0x1e4178 }, { 0x60d8f8, 0x105878 },
};
static uint8_t CHART_TAB_FILL[SPECIES_CHART_TABS][3] = {
	{ 0xa5, 0x5a, 0x73 }, { 0x0f, 0x50, 0x82 }, { 0x00, 0x87, 0x41 }, { 0xaf, 0x64, 0x2d }, { 0xb4, 0x8c, 0x18 },
};
static int8_t CHART_TAB;
static int16_t CHART_TOP;		/* the tab's row shown first */
static int16_t CHART_TAB_COUNT[SPECIES_CHART_TABS];
static int16_t CHART_RAISED;		/* raised species of the tab */
static int16_t CHART_PAGE_IDS[CHART_PAGE_SIZE];	/* the shown species, box by box */
static int16_t CHART_PAGE_COUNT;
static int16_t CHART_TAB_LABEL_X[SPECIES_CHART_TABS]; /* where each tab name is in the text area, and its width */
static int16_t CHART_TAB_LABEL_W[SPECIES_CHART_TABS];
static int16_t CHART_TAB_LABEL_ROW[SPECIES_CHART_TABS];
static int16_t CHART_KEY_W[2];		/* "L1" and "R1" */
static int8_t CHART_CHIP_PAD;		/* a chip's width past its name */
static int16_t CHART_TEXT_ID;		/* the species whose name is in the text area (-2: none drawn yet) */
static int16_t CHART_SHOWN_NAME_W;
static int16_t CHART_COUNT_W;

/* the tab's rows of boxes */
static int32_t chartRows(void)
{
	return (CHART_TAB_COUNT[(int32_t)CHART_TAB] + CHART_COLS - 1) / CHART_COLS;
}

/* the VRAM slot of the shown box k's icon: its place in the tab, modulo the slots */
static int32_t chartSlot(int32_t k)
{
	return (CHART_TOP * CHART_COLS + k) % CHART_PAGE_SIZE;
}

/* the icon's top-left corner of a grid box, screen pixels (its border is one pixel out) */
static void chartBoxXY(int32_t col, int32_t row, int16_t *x, int16_t *y)
{
	if (!CITY_VIEW && !MOD_OPT_NEW_DIGIMON) { /* MOD (options): the original sheet's columns (chartVanillaInput) */
		*x = (col < 3 ? col * 0x25 + 0x1c : col < 7 ? (col - 3) * 0x18 + 0x8b : (col - 7) * 0x18 + 0xf8) + 2;
		*y = row * 0x13 + 0x2b;
		return;
	}
	*x = CHART_GRID_X + 1 + col * CHART_PITCH;
	*y = CHART_GRID_Y + 1 + row * CHART_PITCH;
}

/* City uses luminance CLUTs, preserving transparent entries and opaque dark outlines. */
static void cityGreyClut(uint16_t *palette, int32_t slot)
{
	RECT rect;
	int32_t i;
	int32_t c;
	int32_t grey;

	for (i = 0; i < 16; i++) {
		c = palette[i];
		if (c != 0) {
			grey = ((c & 31) * 77 + ((c >> 5) & 31) * 150 + ((c >> 10) & 31) * 29) >> 8;
			if (grey == 0) grey = 1;
			palette[i] = (c & 0x8000) | grey | (grey << 5) | (grey << 10);
		}
	}
	setRECT(&rect, CHART_CLUT_X + (slot % 4) * 16, CHART_CLUT_Y + slot / 4, 16, 1);
	LoadImage(&rect, (u_long *)palette);
	DrawSync(0);
}

/* reads the shown rows: the tab's counts and ids, the raised count, and the icons of the species past the sheet, of
 * every shown row (row < 0) or of shown row `row` only (a scroll) */
static void chartLoadPage(int32_t row)
{
	uint8_t *s;
	uint16_t *ids;
	int32_t first;
	int32_t g;
	int32_t k;
	int32_t sector;
	int32_t last;
	int32_t slot;
	RECT rect;
	uint16_t palette[16];
	int32_t id;
	int32_t clut;
	uint8_t *icon;

	CHART_PAGE_COUNT = 0;
	CHART_RAISED = 0;
	CHART_TEXT_ID = -2;
	s = readSpeciesChart(0, CITY_VIEW);
	if (s == NULL) {
		return;
	}
	ids = (uint16_t *)s;
	first = 0;
	for (k = 0; k < (CITY_VIEW ? 1 : SPECIES_CHART_TABS); k++) {
		CHART_TAB_COUNT[k] = ids[k];
		if (k < CHART_TAB) {
			first += ids[k];
		}
	}
	ids += CITY_VIEW ? 1 : SPECIES_CHART_TABS;
	if (CITY_VIEW && !MOD_OPT_STORY) {
		g = 0;
		for (k = 0; k < CHART_TAB_COUNT[0]; k++) {
			if (CHART_CELL_ID(ids[k]) < 59) {
				ids[g++] = ids[k];
			}
		}
		CHART_TAB_COUNT[0] = g;
	}
	for (k = 0; k < CHART_TAB_COUNT[(int32_t)CHART_TAB]; k++) {
		g = CHART_CELL_ID(ids[first + k]);
		if (CITY_VIEW ? isDigimonRecruited(g) : hasDigimonRaised(g)) {
			CHART_RAISED++;
		}
	}
	if (CHART_TOP > chartRows() - CHART_ROWS) {
		CHART_TOP = chartRows() > CHART_ROWS ? chartRows() - CHART_ROWS : 0;
	}
	first += CHART_TOP * CHART_COLS;
	CHART_PAGE_COUNT = CHART_TAB_COUNT[(int32_t)CHART_TAB] - CHART_TOP * CHART_COLS;
	if (CHART_PAGE_COUNT > CHART_PAGE_SIZE) {
		CHART_PAGE_COUNT = CHART_PAGE_SIZE;
	}
	for (k = 0; k < CHART_PAGE_COUNT; k++) {
		CHART_PAGE_IDS[k] = ids[first + k];
	}
	last = -1;
	for (k = 0; k < CHART_PAGE_COUNT; k++) {
		id = CHART_CELL_ID(CHART_PAGE_IDS[k]);
		if (row >= 0 && k / CHART_COLS != row) continue;
		slot = chartSlot(k);
		if (id <= NUM_EVO_CHART_DIGIMON) {
			if (CITY_VIEW && !isDigimonRecruited(id)) {
				clut = EVO_CHART_VIEW_COLORS.m[EVO_CHART_BOXES[id - 1].clut];
				setRECT(&rect, (clut & 63) * 16, clut >> 6, 16, 1);
				StoreImage(&rect, (u_long *)palette);
				DrawSync(0);
				cityGreyClut(palette, slot);
			}
			continue;
		}
		g = first + k;
		sector = 1 + g / SPECIES_CHART_ICONS_PER_SECTOR;
		if (sector != last) {
			DrawSync(0); /* the uploads read the sector buffer */
			s = readSpeciesChart(sector, CITY_VIEW);
			last = sector;
		}
		slot = chartSlot(k);
		icon = s + (g % SPECIES_CHART_ICONS_PER_SECTOR) * SPECIES_CHART_ICON_SIZE;
		setRECT(&rect, CHART_CLUT_X + (slot % 4) * 16, CHART_CLUT_Y + slot / 4, 16, 1);
		if (CITY_VIEW && !isDigimonRecruited(id)) {
			cityGreyClut((uint16_t *)icon, slot);
		} else {
			LoadImage(&rect, (u_long *)icon);
		}
		setRECT(&rect, CHART_ICON_X + (slot % 16) * 4, CHART_ICON_Y + (slot / 16) * 32, 4, 16);
		LoadImage(&rect, (u_long *)(icon + 0x20));
		rect.y += 16;
		LoadImage(&rect, (u_long *)(icon + 0xA0));
	}
	DrawSync(0);
}

/* the mod font's solidus (tools/font/extra.txt F061): an ASCII '/' draws the original font's triangle */
#define CHART_SLASH_1	((char)0xF0)
#define CHART_SLASH_2	((char)0x61)

/* writes a number's digits at `p`, returns the end */
static char *chartDigits(char *p, int32_t n)
{
	char tmp[6];
	int32_t i;

	i = 0;
	do {
		tmp[i++] = '0' + n % 10;
		n /= 10;
	} while (n > 0 && i < 6);
	while (i > 0) {
		*p++ = tmp[--i];
	}
	*p = 0;
	return p;
}

/* the tab names and keys, once when the view opens (after drawEvoChartStrings cleared the text area) */
static void chartDrawLabels(void)
{
	int32_t k;
	int32_t x;
	int32_t row;
	int32_t sum;
	int32_t room;
	int32_t pad;
	int32_t mode;
	int32_t w;
	char buf[16];

	if (CITY_VIEW) {
		CHART_TAB_LABEL_W[0] = drawStringMode(CITY_STR_PROSPERITY, 0, CHART_LABEL_ROW, FIT_VANILLA);
		CHART_TAB_LABEL_W[1] = drawStringMode(CITY_STR_RESIDENTS, 128, CHART_LABEL_ROW, FIT_VANILLA);
		chartDigits(buf, readPStat(1));
		CHART_KEY_W[0] = drawStringMode(buf, 0, CHART_KEYS_ROW, FIT_VANILLA);
		return;
	}
	CHART_KEY_W[0] = measureStringMode("L1", FIT_VANILLA);
	CHART_KEY_W[1] = measureStringMode("R1", FIT_VANILLA);
	drawString("L1", 0, CHART_KEYS_ROW);
	drawString("R1", 0x20, CHART_KEYS_ROW);
	/* the chips share the band's width with the keys: a wider font (OPTIONS' FONT) narrows their margins first, then
	 * draws the names tighter */
	room = CHART_BAND_W - CHART_KEY_W[0] - CHART_KEY_W[1] - 8 - (SPECIES_CHART_TABS - 1) * 3;
	sum = 0;
	for (k = 0; k < SPECIES_CHART_TABS; k++) {
		sum += measureStringMode(CHART_TAB_NAMES[k], FIT_VANILLA);
	}
	pad = (room - sum) / SPECIES_CHART_TABS;
	pad = pad > 10 ? 10 : (pad < 4 ? 4 : pad);
	CHART_CHIP_PAD = pad;
	room -= pad * SPECIES_CHART_TABS;
	/* the names in the text area: past its width, on the keys' row after them */
	x = 0;
	row = CHART_LABEL_ROW;
	for (k = 0; k < SPECIES_CHART_TABS; k++) {
		mode = FIT_VANILLA;
		CHART_TAB_LABEL_W[k] = measureStringMode(CHART_TAB_NAMES[k], FIT_VANILLA);
		if (sum > room) {
			CHART_TAB_LABEL_W[k] = CHART_TAB_LABEL_W[k] * room / sum;
			mode = fitStringMode(CHART_TAB_NAMES[k], CHART_TAB_LABEL_W[k]);
		}
		if (x + CHART_TAB_LABEL_W[k] + 12 > 256) {
			row = CHART_KEYS_ROW;
			x = 0x40;
		}
		CHART_TAB_LABEL_X[k] = x;
		CHART_TAB_LABEL_ROW[k] = row;
		w = drawStringMode(CHART_TAB_NAMES[k], x, row, mode);
		if (mode == FIT_VANILLA || w < CHART_TAB_LABEL_W[k]) {
			CHART_TAB_LABEL_W[k] = w;
		}
		x += CHART_TAB_LABEL_W[k] + 12;
	}
}

/* the name under the cursor (raised species only) and the counters, when they change */
static void chartDrawStatus(void)
{
	RECT rect;
	char buf[16];
	char *p;
	int32_t id;

	id = MENU_STATE >= 2 ? CHART_SELECTED_DIGIMON : -1;
	if (!CITY_VIEW && (id <= 0 || !hasDigimonRaised(id))) id = -1;
	if (id == CHART_TEXT_ID) {
		return;
	}
	if (CHART_TEXT_ID == -2) { /* a new page: the counters */
		setRECT(&rect, 0, CHART_COUNT_ROW, 256, 12);
		clearTextSubArea(&rect);
		p = chartDigits(buf, CHART_RAISED);
		*p++ = CHART_SLASH_1;
		*p++ = CHART_SLASH_2;
		chartDigits(p, CHART_TAB_COUNT[(int32_t)CHART_TAB]);
		CHART_COUNT_W = measureStringMode(buf, FIT_VANILLA);
		drawString(buf, 0, CHART_COUNT_ROW);
	}
	setRECT(&rect, 0, CHART_NAME_ROW, 256, 12);
	clearTextSubArea(&rect);
	CHART_SHOWN_NAME_W = 0;
	if (id > 0) {
		CHART_SHOWN_NAME_W = drawStringMode(CITY_VIEW &&
			(CHART_PAGE_IDS[CHART_SELECTED_ROW * CHART_COLS + CHART_SELECTED_COLUMN] & 0x8000) && !isDigimonRecruited(id)
			? "?????" : (char *)digimonPara(id), 0, CHART_NAME_ROW, FIT_VANILLA);
	}
	if (CITY_VIEW) {
		setRECT(&rect, 80, CHART_KEYS_ROW, 64, 12);
		clearTextSubArea(&rect);
		buf[0] = '+';
		chartDigits(buf + 1, id == 11 || id == 39 || id == 53 ? 1 : id > 0 ? DIGI_PARA(id).level - 2 : 0);
		CHART_KEY_W[1] = drawStringMode(buf, 80, CHART_KEYS_ROW, FIT_VANILLA);
	}
	CHART_TEXT_ID = id;
}

/* the species under the cursor (-1: an empty box) */
static void chartSelect(void)
{
	int32_t k;

	k = CHART_SELECTED_ROW * CHART_COLS + CHART_SELECTED_COLUMN;
	if (k >= CHART_PAGE_COUNT && CHART_PAGE_COUNT > 0) { /* past the last species: onto it */
		k = CHART_PAGE_COUNT - 1;
		CHART_SELECTED_ROW = k / CHART_COLS;
		CHART_SELECTED_COLUMN = k % CHART_COLS;
	}
	CHART_SELECTED_DIGIMON = k < CHART_PAGE_COUNT ? CHART_CELL_ID(CHART_PAGE_IDS[k]) : -1;
}

static void chartOverviewInput(int32_t grid)
{
	int32_t tab;
	int32_t top;
	int32_t row;
	int32_t col;

	tab = CHART_TAB;
	top = CHART_TOP;
	row = CHART_SELECTED_ROW;
	col = CHART_SELECTED_COLUMN;
	if (!CITY_VIEW && (CHANGED_INPUT & 0x4)) { /* L1, R1: the tab before or after, wrapping */
		CHART_TAB = (CHART_TAB + SPECIES_CHART_TABS - 1) % SPECIES_CHART_TABS;
	} else if (!CITY_VIEW && (CHANGED_INPUT & 0x8)) {
		CHART_TAB = (CHART_TAB + 1) % SPECIES_CHART_TABS;
	} else if (!grid) { /* the focus on the menu's tab: the D-pad is the menu's, the grid stays */
	} else if (CHANGED_INPUT & 0x8000) { /* left, right: within the row */
		if (CHART_SELECTED_COLUMN > 0) {
			CHART_SELECTED_COLUMN--;
		}
	} else if (CHANGED_INPUT & 0x2000) {
		if (CHART_SELECTED_COLUMN < CHART_COLS - 1 &&
		    CHART_SELECTED_ROW * CHART_COLS + CHART_SELECTED_COLUMN + 1 < CHART_PAGE_COUNT) {
			CHART_SELECTED_COLUMN++;
		}
	} else if (CHANGED_INPUT & 0x1000) { /* up, down: past the top or bottom row, the rows scroll by one */
		if (CHART_SELECTED_ROW > 0) {
			CHART_SELECTED_ROW--;
		} else if (CHART_TOP > 0) {
			CHART_TOP--;
		}
	} else if ((CHANGED_INPUT & 0x4000) && (CHART_TOP + CHART_SELECTED_ROW + 1) < chartRows()) {
		if (CHART_SELECTED_ROW < CHART_ROWS - 1) {
			CHART_SELECTED_ROW++;
		} else {
			CHART_TOP++;
		}
	}
	if (CHART_TAB != tab) {
		CHART_TOP = 0;
		CHART_SELECTED_ROW = CHART_SELECTED_COLUMN = 0;
		chartLoadPage(-1);
	} else if (CHART_TOP != top) {
		chartLoadPage(CHART_TOP > top ? CHART_ROWS - 1 : 0); /* only the row that comes in */
	}
	chartSelect();
	if (CHART_TAB != tab || CHART_TOP != top || CHART_SELECTED_ROW != row || CHART_SELECTED_COLUMN != col) {
		playSound(0, 2);
	}
}

/*
 * MOD (options): with NEW DIGIMON off the overview is the original one: the sheet of the original's species in its level
 * columns, its cursor and its input (the code before patch 0170).
 */
static void chartVanillaInput(void)
{
	int16_t previousRow;
	int16_t previousColumn;
	uint8_t maxColumn;
	int16_t selectorX;
	int32_t i;

	previousRow = CHART_SELECTED_COLUMN;
	previousColumn = CHART_SELECTED_ROW;
	if (CHANGED_INPUT & 0x8000) {
		CHART_SELECTED_COLUMN--;
	}
	if (CHANGED_INPUT & 0x2000) {
		CHART_SELECTED_COLUMN++;
	}
	if (CHART_SELECTED_COLUMN < 0) {
		CHART_SELECTED_COLUMN = 0;
	}
	if (CHART_SELECTED_COLUMN >= 9) {
		CHART_SELECTED_COLUMN = 8;
	}
	if ((CHART_SELECTED_COLUMN == 0) || (CHART_SELECTED_COLUMN == 1)) {
		if (CHANGED_INPUT & 0x1000) {
			CHART_SELECTED_ROW -= 2;
		}
		if (CHANGED_INPUT & 0x4000) {
			CHART_SELECTED_ROW += 2;
		}
		CHART_SELECTED_ROW = (CHART_SELECTED_ROW / 2) * 2;
		maxColumn = 6;
	} else {
		if (CHANGED_INPUT & 0x1000) {
			CHART_SELECTED_ROW--;
		}
		if (CHANGED_INPUT & 0x4000) {
			CHART_SELECTED_ROW++;
		}
		if (CHART_SELECTED_COLUMN == 2) {
			maxColumn = 8;
		} else if (((CHART_SELECTED_COLUMN >= 4) && (CHART_SELECTED_COLUMN < 7)) || (CHART_SELECTED_COLUMN == 8)) {
			maxColumn = 6;
		} else {
			maxColumn = 7;
		}
	}
	if (CHART_SELECTED_ROW < 0) {
		CHART_SELECTED_ROW = 0;
	}
	if (maxColumn < CHART_SELECTED_ROW) {
		CHART_SELECTED_ROW = maxColumn;
	}
	if ((previousRow != CHART_SELECTED_COLUMN) || (previousColumn != CHART_SELECTED_ROW)) {
		playSound(0, 2);
	}
	if (CHART_SELECTED_COLUMN < 3) {
		selectorX = CHART_SELECTED_COLUMN * 0x25 + 0x1c;
	} else if (CHART_SELECTED_COLUMN < 7) {
		selectorX = (CHART_SELECTED_COLUMN - 3) * 0x18 + 0x8b;
	} else {
		selectorX = (CHART_SELECTED_COLUMN - 7) * 0x18 + 0xf8;
	}
	for (i = 0; i < NUM_EVO_CHART_DIGIMON; i++) {
		if (((selectorX + 2) == EVO_CHART_BOXES[i].posX) &&
		    ((CHART_SELECTED_ROW * 0x13 + 0x2b) == EVO_CHART_BOXES[i].posY)) {
			break;
		}
	}
	CHART_SELECTED_DIGIMON = i < NUM_EVO_CHART_DIGIMON ? i + 1 : -1;
}

static void renderEvoChartViewVanilla(void)
{
	int32_t i;
	int16_t x;
	int8_t shift;
	EvoClutTable cluts;

	cluts = EVO_CHART_VIEW_COLORS;
	switch (MENU_STATE) {
	case 0:
		if (drawEvoChartStrings(0) == 1) {
			MENU_STATE = 1;
		}
		break;
	case 2:
	case 3:
	case 4:
		if (CHART_SELECTED_COLUMN < 3) {
			x = (int16_t)(CHART_SELECTED_COLUMN * 0x25 + 0x1c);
		} else if (CHART_SELECTED_COLUMN < 7) {
			x = (int16_t)((CHART_SELECTED_COLUMN - 3) * 0x18 + 0x8b);
		} else {
			x = (int16_t)((CHART_SELECTED_COLUMN - 7) * 0x18 + 0xf8);
		}
		renderRectPolyFT4((int16_t)(x - 0xa2), (int16_t)(CHART_SELECTED_ROW * 0x13 - 0x4e), 0x18, 0x14, 0, 0xe8, 0x18,
		                  0x7dc7, 5, 0);
		/* fall through */
	case 1:
		for (i = 0; i < NUM_EVO_CHART_DIGIMON; i++) {
			if (hasDigimonRaised((i + 1) & 0xffff)) {
				shift = 0;
				if ((i + 1 == CHART_SELECTED_DIGIMON) && (1 < MENU_STATE) && ((PLAYTIME_FRAMES % 10) < 5)) {
					shift = 0x10;
				}
				renderRectPolyFT4((int16_t)(EVO_CHART_BOXES[i].posX - 0xa0), (int16_t)(EVO_CHART_BOXES[i].posY - 0x78),
				                  0x10, 0x10, (uint8_t)(shift + EVO_CHART_BOXES[i].u), EVO_CHART_BOXES[i].v, 0x18,
				                  cluts.m[EVO_CHART_BOXES[i].clut], 5, 0);
			}
		}
		for (i = 0; i < NUM_EVO_CHART_DIGIMON; i++) {
			renderBorderBox((EVO_CHART_BOXES[i].posX - 1), (EVO_CHART_BOXES[i].posY - 1), 0x12, 0x12, 0xbebebe,
			                0x3c3c3c, 0x87, 0x87, 0x87, 5);
		}
		renderBorderBox(0x15, 0x27, 0x22, 0x9f, 0xaaa0c8, 0x5a3c8c, 0xa5, 0x5a, 0x73, 5);
		renderBorderBox(0x3a, 0x27, 0x22, 0x9f, 0xa078a5, 0x46144b, 0x6e, 0x2d, 0x5a, 5);
		renderBorderBox(0x5f, 0x27, 0x22, 0xb0, 0xc88250, 0x5f370a, 0xf, 0x50, 0x82, 5);
		renderBorderBox(0x84, 0x27, 0x6a, 0x9f, 0x6eaf5a, 0x285a00, 0, 0x87, 0x41, 5);
		renderBorderBox(0xf1, 0x27, 0x38, 0x9f, 0x6996d2, 0x1e4178, 0xaf, 0x64, 0x2d, 5);
		/* MOD (language): each language's crops (EVO_CHART_KEY_CROPS) */
		for (i = 0; i < 4; i++) {
			renderString(i == 0 ? 0xf : i == 2 ? 7 : 0, EVO_CHART_KEY_CROPS[i].x, 0x54, EVO_CHART_KEY_CROPS[i].w, 0xc,
			             EVO_CHART_KEY_CROPS[i].u, 0xc, 5, 1);
		}
	}
}

void renderEvoChartView(void)
{
	int32_t k;
	int32_t x;
	int32_t w;
	int16_t bx;
	int16_t by;
	int16_t id;
	int8_t shift;
	int32_t y;
	int32_t slot;
	EvoClutTable cluts;

	if (!CITY_VIEW && !MOD_OPT_NEW_DIGIMON) {
		renderEvoChartViewVanilla();
		return;
	}
	cluts = EVO_CHART_VIEW_COLORS;
	switch (MENU_STATE) {
	case 0:
		if (drawEvoChartStrings(0) == 1) {
			if (CITY_VIEW) {
				CHART_TAB = CHART_TOP = 0;
				/* Wild recruits add nothing at once, and some scripts add more than the rule:
				 * show what the next save would write, without tournament cleanup. */
				recalculateProsperity();
			}
			chartDrawLabels();
			chartLoadPage(-1);
			chartSelect();
			MENU_STATE = 1;
		}
		break;
	case 2:
	case 3:
	case 4:
		chartBoxXY(CHART_SELECTED_COLUMN, CHART_SELECTED_ROW, &bx, &by);
		renderRectPolyFT4(bx - 0xa4, by - 0x79, 0x18, 0x14, 0, 0xe8, 0x18, 0x7dc7, 5, 0);
		/* fall through */
	case 1:
		chartDrawStatus();
		for (k = 0; k < CHART_PAGE_COUNT; k++) {
			id = CHART_CELL_ID(CHART_PAGE_IDS[k]);
			if (CITY_VIEW ? ((CHART_PAGE_IDS[k] & 0x8000) && !isDigimonRecruited(id)) : !hasDigimonRaised(id)) {
				continue;
			}
			chartBoxXY(k % CHART_COLS, k / CHART_COLS, &bx, &by);
			shift = ((PLAYTIME_FRAMES + k * 3) % 20) < 10; /* two idle frames, half a second each, out of step */
			slot = chartSlot(k);
			if (id <= NUM_EVO_CHART_DIGIMON) {
				renderRectPolyFT4(bx - 0xa0, by - 0x78, 0x10, 0x10,
				                  (uint8_t)(shift * 0x10 + EVO_CHART_BOXES[id - 1].u),
				                  EVO_CHART_BOXES[id - 1].v, 0x18, (CITY_VIEW && !isDigimonRecruited(id))
				                  ? GetClut(CHART_CLUT_X + (slot % 4) * 16, CHART_CLUT_Y + slot / 4)
				                  : cluts.m[EVO_CHART_BOXES[id - 1].clut], 5, 0);
			} else {
				renderRectPolyFT4(bx - 0xa0, by - 0x78, 0x10, 0x10, (slot % 16) * 16,
				                  CHART_ICON_Y - 256 + shift * 16 + (slot / 16) * 32, CHART_ICON_TPAGE,
				                  GetClut(CHART_CLUT_X + (slot % 4) * 16, CHART_CLUT_Y + slot / 4), 5, 0);
			}
		}
		if (CITY_VIEW) {
			x = 22 + CHART_TAB_LABEL_W[0] + 6;
			renderString(4, 22 - 160, 40 - 120, CHART_TAB_LABEL_W[0], 12, 0, CHART_LABEL_ROW, 5, 1);
			renderString(0, x + 27 - CHART_KEY_W[0] - 160, 40 - 120, CHART_KEY_W[0], 12, 0, CHART_KEYS_ROW, 5, 1);
			renderInsetBox(x, 38, 30, 12, 5);
			renderString(4, 293 - CHART_COUNT_W - CHART_TAB_LABEL_W[1] - 5 - 160, 40 - 120,
			             CHART_TAB_LABEL_W[1], 12, 128, CHART_LABEL_ROW, 5, 1);
			renderString(0, 297 - CHART_COUNT_W - 160, 40 - 120, CHART_COUNT_W, 12, 0, CHART_COUNT_ROW, 5, 1);
			if (CHART_SHOWN_NAME_W > 0) {
				id = CHART_SELECTED_DIGIMON;
				renderString(isDigimonRecruited(id) ? 0 : 9, 22 - 160, 0x52, CHART_SHOWN_NAME_W, 12, 0, CHART_NAME_ROW, 5, 1);
				if (isDigimonRecruited(id)) renderString(7, 297 - CHART_KEY_W[1] - 160, 0x52, CHART_KEY_W[1], 12, 80, CHART_KEYS_ROW, 5, 1);
			}
		} else {
		/* the texts: the tab names on their chips, the keys, the name and the counters on the bottom row */
		renderString(9, 20 - 0xa0, 40 - 0x78, CHART_KEY_W[0], 0xc, 0, CHART_KEYS_ROW, 5, 1);
		x = 20 + CHART_KEY_W[0] + 4;
		for (k = 0; k < SPECIES_CHART_TABS; k++) {
			w = CHART_TAB_LABEL_W[k];
			renderString(k == CHART_TAB ? 0 : 9, x + CHART_CHIP_PAD / 2 - 0xa0, 40 - 0x78, w, 0xc,
			             CHART_TAB_LABEL_X[k], CHART_TAB_LABEL_ROW[k], 5, 1);
			x += w + CHART_CHIP_PAD + 3;
		}
		renderString(9, x + 1 - 0xa0, 40 - 0x78, CHART_KEY_W[1], 0xc, 0x20, CHART_KEYS_ROW, 5, 1);
		if (CHART_SHOWN_NAME_W > 0) {
			renderString(0, 22 - 0xa0, 0x52, CHART_SHOWN_NAME_W, 0xc, 0, CHART_NAME_ROW, 5, 1);
		}
		renderString(0, 297 - CHART_COUNT_W - 0xa0, 0x52, CHART_COUNT_W, 0xc, 0, CHART_COUNT_ROW, 5, 1);
		}
		/* the scroll bar when the tab has more rows than are shown: its thumb, then its track */
		if (chartRows() > CHART_ROWS) {
			w = CHART_SCROLL_H * CHART_ROWS / chartRows();
			y = CHART_GRID_Y + (CHART_SCROLL_H - w) * CHART_TOP / (chartRows() - CHART_ROWS);
			renderBox(CHART_SCROLL_X - 0xa0, y - 0x78, CHART_SCROLL_W, w, 0xe6, 0xe6, 0xe6, 0, 5);
			k = CHART_TAB;
			renderBox(CHART_SCROLL_X - 0xa0, CHART_GRID_Y - 0x78, CHART_SCROLL_W, CHART_SCROLL_H,
			          CHART_TAB_FILL[k][0] / 2, CHART_TAB_FILL[k][1] / 2, CHART_TAB_FILL[k][2] / 2, 0, 5);
		}
		/* the boxes, the tab's band, the chips */
		for (k = 0; k < CHART_PAGE_COUNT; k++) {
			chartBoxXY(k % CHART_COLS, k / CHART_COLS, &bx, &by);
			id = CHART_CELL_ID(CHART_PAGE_IDS[k]);
			x = CITY_VIEW ? CHART_CELL_LEVEL(CHART_PAGE_IDS[k]) - 2 : 0;
			w = CITY_VIEW && !isDigimonRecruited(id) ? 2 : 1;
			renderBorderBox(bx - 1, by - 1, 0x12, 0x12, 0xbebebe, 0x3c3c3c,
			                CITY_VIEW ? CHART_TAB_FILL[x][0] / w : 0x87,
			                CITY_VIEW ? CHART_TAB_FILL[x][1] / w : 0x87,
			                CITY_VIEW ? CHART_TAB_FILL[x][2] / w : 0x87, 5);
		}
		k = CHART_TAB;
		if (CITY_VIEW) {
			renderBorderBox(20, 53, CHART_BAND_W, 143, 0x3c3c3c, 0x788c96, 0x28, 0x3c, 0x46, 5);
			break;
		}
		renderBorderBox(20, 53, CHART_BAND_W, 143, CHART_TAB_EDGES[k][0], CHART_TAB_EDGES[k][1], CHART_TAB_FILL[k][0],
		                CHART_TAB_FILL[k][1], CHART_TAB_FILL[k][2], 5);
		x = 20 + CHART_KEY_W[0] + 4;
		for (k = 0; k < SPECIES_CHART_TABS; k++) {
			w = CHART_TAB_LABEL_W[k] + CHART_CHIP_PAD;
			if (k == CHART_TAB) {
				renderBorderBox(x, 37, w - 1, 16, CHART_TAB_EDGES[k][0], CHART_TAB_EDGES[k][1],
				                CHART_TAB_FILL[k][0], CHART_TAB_FILL[k][1], CHART_TAB_FILL[k][2], 5);
			} else {
				renderBox(x - 0xa0, 39 - 0x78, w, 14, CHART_TAB_FILL[k][0] * 9 / 16, CHART_TAB_FILL[k][1] * 9 / 16,
				          CHART_TAB_FILL[k][2] * 9 / 16, 0, 5);
			}
			x += w + 3;
		}
	}
}

/*
 * MOD (chart criteria): with the cursor on one of the "to" boxes the detail view opens
 * a modal card of that evolution's requirements: care mistakes, weight, stats (a Rookie: the highest stat) and one bonus,
 * the four evoRequirementsMet scores. In the tree of the partner's own species the card adds the partner's values and a
 * mark per requirement, and the lines go green to the evolutions whose requirements reach evoPassPoints (special
 * evolutions, the time each level waits and the preference for species not yet raised are left out), dim to the rest;
 * other trees keep the original colours. A Baby II reached from a Fresh has no requirements ("Automatic").
 * The popup's text takes the overview's rows of the text area (CHART_CARD_ROW, two lines a row); chartCardRelease
 * gives them back when the detail closes.
 */
#define CHART_CARD_ROW		0x60
#define CHART_CARD_LINES	18
#define CHART_CARD_TITLE_ROW (CHART_CARD_ROW + (CHART_CARD_LINES / 2) * 12)
#define CHART_CARD_VERDICT_ROW (CHART_CARD_TITLE_ROW + 12)
#define CHART_CARD_FOOTER_ROW (CHART_CARD_VERDICT_ROW + 12)
#define CHART_CARD_HEADER_ROW (CHART_CARD_FOOTER_ROW + 12)
#define CHART_CARD_X 18
#define CHART_CARD_Y 26
#define CHART_CARD_W 284
#define CHART_CARD_H 190
#define CHART_LIT_EDGE		0x50be28 /* the lines of a met evolution, BGR */
#define CHART_LIT_CORE		0xb4ffaa
#define CHART_DIM_LINE		0x6a5c48
#define CHART_FROM_LINE		0xb4a596
#define CHART_MARK_YES		0x6ee65a
#define CHART_MARK_NO		0x505af0

extern int32_t MOD_RENDER_PASS;
int8_t getNumMasteredMoves(void);

/* Packed immutable text cells: only their screen positions change while scrolling. */
typedef struct {
	int16_t w;
	uint8_t u, v;
} ChartCell;
typedef struct {
	ChartCell label, need, value;
	int8_t valueColor, mark, indent;
} ChartCardLine;
static ChartCardLine CHART_CARD[CHART_CARD_LINES];
static int16_t CHART_PACK_U, CHART_PACK_V;
static int8_t CHART_CARD_MINE;		/* the card is of the partner's tree */
static int8_t CHART_TEXT_TAKEN;		/* the card wrote over the overview's rows */
static int16_t CHART_CARD_VERDICT_W;
static int8_t CHART_CARD_VERDICT_COLOR;
static int16_t CHART_CARD_FOOTER_W;
static int16_t CHART_PROMPT_W;
static int16_t CHART_NEEDS_W;
static int16_t CHART_HAVE_W;
static uint32_t CHART_LIT;		/* bit k: the evolution into CHART_TO_IDS[k] is met */
/* Only MENU code reads these immutable translated strings. */
static const char CHART_POPUP_AUTOMATIC[] __attribute__((section(".rodata.CHART_POPUP_AUTOMATIC"))) = CHART_STR_AUTOMATIC;
#define CHART_TEXT_AUTOMATIC ((char *)CHART_POPUP_AUTOMATIC)
static const char CHART_POPUP_BATTLES[] __attribute__((section(".rodata.CHART_POPUP_BATTLES"))) = CHART_STR_BATTLES;
#define CHART_TEXT_BATTLES ((char *)CHART_POPUP_BATTLES)
static const char CHART_POPUP_BONUS_ANY[] __attribute__((section(".rodata.CHART_POPUP_BONUS_ANY"))) = CHART_STR_BONUS_ANY;
#define CHART_TEXT_BONUS_ANY ((char *)CHART_POPUP_BONUS_ANY)
static const char CHART_POPUP_BONUS_ONE_OF[] __attribute__((section(".rodata.CHART_POPUP_BONUS_ONE_OF"))) = CHART_STR_BONUS_ONE_OF;
#define CHART_TEXT_BONUS_ONE_OF ((char *)CHART_POPUP_BONUS_ONE_OF)
static const char CHART_POPUP_CLOSE[] __attribute__((section(".rodata.CHART_POPUP_CLOSE"))) = CHART_STR_CLOSE;
#define CHART_TEXT_CLOSE ((char *)CHART_POPUP_CLOSE)
static const char CHART_POPUP_DISCIPLINE[] __attribute__((section(".rodata.CHART_POPUP_DISCIPLINE"))) = CHART_STR_DISCIPLINE;
#define CHART_TEXT_DISCIPLINE ((char *)CHART_POPUP_DISCIPLINE)
static const char CHART_POPUP_EVOLVE[] __attribute__((section(".rodata.CHART_POPUP_EVOLVE"))) = CHART_STR_EVOLVE;
#define CHART_TEXT_EVOLVE ((char *)CHART_POPUP_EVOLVE)
static const char CHART_POPUP_FROM[] __attribute__((section(".rodata.CHART_POPUP_FROM"))) = CHART_STR_FROM;
#define CHART_TEXT_FROM ((char *)CHART_POPUP_FROM)
static const char CHART_POPUP_HAPPINESS[] __attribute__((section(".rodata.CHART_POPUP_HAPPINESS"))) = CHART_STR_HAPPINESS;
#define CHART_TEXT_HAPPINESS ((char *)CHART_POPUP_HAPPINESS)
static const char CHART_POPUP_HAVE[] __attribute__((section(".rodata.CHART_POPUP_HAVE"))) = CHART_STR_HAVE;
#define CHART_TEXT_HAVE ((char *)CHART_POPUP_HAVE)
static const char CHART_POPUP_MISTAKES[] __attribute__((section(".rodata.CHART_POPUP_MISTAKES"))) = CHART_STR_MISTAKES;
#define CHART_TEXT_MISTAKES ((char *)CHART_POPUP_MISTAKES)
static const char CHART_POPUP_MISTAKES_ANY[] __attribute__((section(".rodata.CHART_POPUP_MISTAKES_ANY"))) = CHART_STR_MISTAKES_ANY;
#define CHART_TEXT_MISTAKES_ANY ((char *)CHART_POPUP_MISTAKES_ANY)
static const char CHART_POPUP_NEEDS[] __attribute__((section(".rodata.CHART_POPUP_NEEDS"))) = CHART_STR_NEEDS;
#define CHART_TEXT_NEEDS ((char *)CHART_POPUP_NEEDS)
static const char CHART_POPUP_NO[] __attribute__((section(".rodata.CHART_POPUP_NO"))) = CHART_STR_NO;
#define CHART_TEXT_NO ((char *)CHART_POPUP_NO)
static const char CHART_POPUP_READY[] __attribute__((section(".rodata.CHART_POPUP_READY"))) = CHART_STR_READY;
#define CHART_TEXT_READY ((char *)CHART_POPUP_READY)
static const char CHART_POPUP_SHORT[] __attribute__((section(".rodata.CHART_POPUP_SHORT"))) = CHART_STR_SHORT;
#define CHART_TEXT_SHORT ((char *)CHART_POPUP_SHORT)
static const char CHART_POPUP_STATS[] __attribute__((section(".rodata.CHART_POPUP_STATS"))) = CHART_STR_STATS;
#define CHART_TEXT_STATS ((char *)CHART_POPUP_STATS)
static const char CHART_POPUP_STATS_ANY[] __attribute__((section(".rodata.CHART_POPUP_STATS_ANY"))) = CHART_STR_STATS_ANY;
#define CHART_TEXT_STATS_ANY ((char *)CHART_POPUP_STATS_ANY)
static const char CHART_POPUP_STAT_BRN[] __attribute__((section(".rodata.CHART_POPUP_STAT_BRN"))) = CHART_STR_STAT_BRN;
#define CHART_TEXT_STAT_BRN ((char *)CHART_POPUP_STAT_BRN)
static const char CHART_POPUP_STAT_DEF[] __attribute__((section(".rodata.CHART_POPUP_STAT_DEF"))) = CHART_STR_STAT_DEF;
#define CHART_TEXT_STAT_DEF ((char *)CHART_POPUP_STAT_DEF)
static const char CHART_POPUP_STAT_HP[] __attribute__((section(".rodata.CHART_POPUP_STAT_HP"))) = CHART_STR_STAT_HP;
#define CHART_TEXT_STAT_HP ((char *)CHART_POPUP_STAT_HP)
static const char CHART_POPUP_STAT_MP[] __attribute__((section(".rodata.CHART_POPUP_STAT_MP"))) = CHART_STR_STAT_MP;
#define CHART_TEXT_STAT_MP ((char *)CHART_POPUP_STAT_MP)
static const char CHART_POPUP_STAT_OFF[] __attribute__((section(".rodata.CHART_POPUP_STAT_OFF"))) = CHART_STR_STAT_OFF;
#define CHART_TEXT_STAT_OFF ((char *)CHART_POPUP_STAT_OFF)
static const char CHART_POPUP_STAT_SPD[] __attribute__((section(".rodata.CHART_POPUP_STAT_SPD"))) = CHART_STR_STAT_SPD;
#define CHART_TEXT_STAT_SPD ((char *)CHART_POPUP_STAT_SPD)
static const char CHART_POPUP_TECHS[] __attribute__((section(".rodata.CHART_POPUP_TECHS"))) = CHART_STR_TECHS;
#define CHART_TEXT_TECHS ((char *)CHART_POPUP_TECHS)
static const char CHART_POPUP_TOP[] __attribute__((section(".rodata.CHART_POPUP_TOP"))) = CHART_STR_TOP;
#define CHART_TEXT_TOP ((char *)CHART_POPUP_TOP)
static const char CHART_POPUP_VERDICT[] __attribute__((section(".rodata.CHART_POPUP_VERDICT"))) = CHART_STR_VERDICT;
#define CHART_TEXT_VERDICT ((char *)CHART_POPUP_VERDICT)
static const char CHART_POPUP_VIEW[] __attribute__((section(".rodata.CHART_POPUP_VIEW"))) = CHART_STR_VIEW;
#define CHART_TEXT_VIEW ((char *)CHART_POPUP_VIEW)
static const char CHART_POPUP_WEIGHT[] __attribute__((section(".rodata.CHART_POPUP_WEIGHT"))) = CHART_STR_WEIGHT;
#define CHART_TEXT_WEIGHT ((char *)CHART_POPUP_WEIGHT)
static const char CHART_POPUP_YES[] __attribute__((section(".rodata.CHART_POPUP_YES"))) = CHART_STR_YES;
#define CHART_TEXT_YES ((char *)CHART_POPUP_YES)
static char *CHART_STAT_NAMES[6] = { CHART_TEXT_STAT_HP,  CHART_TEXT_STAT_MP,  CHART_TEXT_STAT_OFF,
				      CHART_TEXT_STAT_DEF, CHART_TEXT_STAT_SPD, CHART_TEXT_STAT_BRN };

static int32_t chartIsPartnerTree(void)
{
	return CHART_DETAIL_ID == PARTNER_ENTITY.digimonEntity.entity.type;
}

/* a Baby II's row: no requirement at all (the Fresh's evolution is fixed, MOD_EVO_FRESH) */
static int32_t chartIsAutomatic(EvoRequirements *r)
{
	return r->care == -1 && r->weight == -1;
}

/* 1 when the partner (the centre of the tree) meets the requirements of evolving into `target` */
static int32_t chartEvoMet(int32_t target)
{
	EvoRequirements r;
	int32_t i;

	r = EVO_REQ_ROW(target);
	if (chartIsAutomatic(&r)) {
		for (i = 0; i < MOD_EVO_PAIR_MAX; i++) {
			if (MOD_EVO_FRESH[i][0] == CHART_DETAIL_ID && MOD_EVO_FRESH[i][1] == target) {
				return 1;
			}
		}
		return 0;
	}
	if (!isPartnerModelFit(target)) {
		return 0;
	}
	return evoRequirementPoints(evoRequirementsMet(CHART_DETAIL_ID, target, (r.flags & 0x10) >> 4, r.flags & 1)) >=
	       evoPassPoints();
}

/* the met evolutions of the partner's tree, once per centre */
static void chartUpdateLit(void)
{
	int32_t k;

	if (CHART_LIT_FOR == CHART_DETAIL_ID) {
		return;
	}
	CHART_LIT = 0;
	for (k = 0; k < CHART_TO_N; k++) {
		if (chartEvoMet(CHART_TO_IDS[k])) {
			CHART_LIT |= 1 << k;
		}
	}
	CHART_LIT_FOR = CHART_DETAIL_ID;
}

/* a number, with its sign */
static __attribute__((noinline)) char *chartNumber(char *p, int32_t n)
{
	if (n < 0) {
		*p++ = '-';
		n = -n;
	}
	return chartDigits(p, n);
}

/* appends a "<label> <n>+" or "<label> <lo>-<hi>" requirement to `p` */
static __attribute__((noinline)) char *chartRange(char *p, char *label, int32_t lo, int32_t hi)
{
	while (*label != 0) {
		*p++ = *label++;
	}
	*p++ = ' ';
	p = chartNumber(p, lo);
	if (hi == lo) {
		*p++ = '+';
	} else {
		*p++ = '-';
		p = chartNumber(p, hi);
	}
	*p = 0;
	return p;
}

#define CHART_NO_VALUE	-32768	/* a line without a number */

/* copies `str` to `p`, returns the end */
static __attribute__((noinline)) char *chartAppend(char *p, char *str)
{
	while (*str != 0) {
		*p++ = *str++;
	}
	*p = 0;
	return p;
}

/* Pack normal-width strings into the borrowed rows once, before any sprite is submitted.
 * Fixed title/headers/footer use the rows above CHART_CARD_TITLE_ROW. */
static ChartCell chartCardText(char *text)
{
	ChartCell cell;
	cell.w = measureStringMode(text, FIT_VANILLA);
	if (CHART_PACK_U + cell.w > 244) { CHART_PACK_U = 0; CHART_PACK_V += 12; }
	cell.u = CHART_PACK_U;
	cell.v = CHART_PACK_V;
	drawStringMode(text, cell.u, cell.v, FIT_VANILLA);
	CHART_PACK_U += (cell.w + 7) & ~3;
	return cell;
}

static __attribute__((noinline)) void chartCardLine(char *label, int32_t have, int32_t ok, int32_t mark, int32_t indent, char *word)
{
	ChartCardLine *l;
	char buf[16], name[160];
	char *split;
	int32_t k;
	if (CHART_CARD_COUNT >= CHART_CARD_LINES) return;
	k = CHART_CARD_COUNT++;
	l = &CHART_CARD[k];
	l->indent = indent;
	l->mark = CHART_CARD_MINE && mark ? (ok ? 1 : 2) : 0;
	l->valueColor = ok ? 7 : 6;
	l->value.w = l->need.w = 0;
	strcpy(name, label);
	split = strrchr(name, ' ');
	if (k && split && ((split[1] >= '0' && split[1] <= '9') || split[1] == '-')) {
		*split = 0;
		l->need = chartCardText(split + 1);
	}
	if (k == 0) {
		l->label.w = drawStringMode(name, 0, CHART_CARD_TITLE_ROW, FIT_VANILLA);
		l->label.u = 0; l->label.v = CHART_CARD_TITLE_ROW;
	} else l->label = chartCardText(name);
	if (CHART_CARD_MINE && (word != NULL || have != CHART_NO_VALUE)) {
		if (word == NULL) { chartNumber(buf, have); word = buf; }
		l->value = chartCardText(word);
	}
}

/* the card of evolving into `target`: drawn into the text area when the target or the centre changes */
static __attribute__((noinline)) void chartCardBuild(int32_t target)
{
	EvoRequirements r;
	BaseStats *st;
	RECT rect;
	char a[160];
	int32_t met;
	int32_t isMaxCM;
	int32_t isMaxBattles;
	int32_t div;
	int32_t tol;
	int32_t k;
	int32_t need;
	int32_t have;
	int32_t top;
	int32_t statCount;
	int16_t req[6];

	setRECT(&rect, 0, CHART_CARD_ROW, 256, CHART_CARD_HEADER_ROW + 12 - CHART_CARD_ROW);
	clearTextSubArea(&rect);
	CHART_TEXT_TAKEN = 1;
	CHART_CARD_COUNT = 0;
	CHART_PACK_U = 0; CHART_PACK_V = CHART_CARD_ROW;
	CHART_CARD_VERDICT_W = CHART_CARD_FOOTER_W = 0;
	CHART_CARD_MINE = chartIsPartnerTree();
	r = EVO_REQ_ROW(target);
	st = &PARTNER_ENTITY.digimonEntity.stats.base;
	isMaxCM = (r.flags & 0x10) >> 4;
	isMaxBattles = r.flags & 1;
	met = CHART_CARD_MINE ? evoRequirementsMet(CHART_DETAIL_ID, target, isMaxCM, isMaxBattles) : 0;

	/* The title and all wording use the same card text rows. */
	CHART_CARD_MINE = 0;
	sprintf(a, CHART_TEXT_EVOLVE, hasDigimonRaised((uint16_t)target) ? (char *)digimonPara(target) : "?????");
	chartCardLine(a, CHART_NO_VALUE, 1, 0, 0, NULL);
	CHART_CARD_MINE = chartIsPartnerTree();
	if (chartIsAutomatic(&r)) {
		chartCardLine(CHART_TEXT_AUTOMATIC, CHART_NO_VALUE, 1, 1, 0, NULL);
		return;
	}

	/* care mistakes */
	if (!isMaxCM && r.care <= 0) {
		chartCardLine(CHART_TEXT_MISTAKES_ANY, CHART_NO_VALUE, 1, 1, 0, NULL);
	} else {
		chartRange(a, CHART_TEXT_MISTAKES, isMaxCM ? 0 : r.care, r.care);
		chartCardLine(a, PARTNER_PARA.careMistakes, met & EVO_MET_CARE, 1, 0, NULL);
	}

	/* weight */
	tol = evoAmt(MOD_EVF_SCORE_WEIGHT_TOL);
	r.weight = evoTargetWeight(CHART_DETAIL_ID, target);
	chartRange(a, CHART_TEXT_WEIGHT, r.weight - tol, r.weight + tol);
	chartCardLine(a, PARTNER_PARA.weight, met & EVO_MET_WEIGHT, 1, 0, NULL);

	/* stats: a Rookie's is the highest stat; the others, every stat asked for */
	req[0] = r.hp;
	req[1] = r.mp;
	req[2] = r.offense;
	req[3] = r.defense;
	req[4] = r.speed;
	req[5] = r.brain;
	if (DIGI_PARA(target).level == 3U) {
		top = CHART_CARD_MINE ? evoPartnerTopStat() : 0;
		chartCardLine(CHART_TEXT_TOP, CHART_NO_VALUE, met & EVO_MET_STATS, 1, 0, CHART_STAT_NAMES[top]);
		for (k = 0; k < 6; k++) if (req[k] == 1)
			chartCardLine(CHART_STAT_NAMES[k], CHART_NO_VALUE, 1, 0, 8, NULL);
	} else {
		div = evoAmt(MOD_EVF_SCORE_HPMP_DIV);
		statCount = 0;
		for (k = 0; k < 6; k++) if (req[k] >= 0) statCount++;
		if (statCount != 1)
			chartCardLine(statCount == 0 ? CHART_TEXT_STATS_ANY : CHART_TEXT_STATS, CHART_NO_VALUE,
			              met & EVO_MET_STATS, 1, 0, NULL);
		for (k = 0; k < 6; k++) {
			if (req[k] < 0) {
				continue;
			}
			need = req[k];
			switch (k) {
			case 0: have = st->hp; need *= div; break;
			case 1: have = st->mp; need *= div; break;
			case 2: have = st->off; break;
			case 3: have = st->def; break;
			case 4: have = st->speed; break;
			default: have = st->brain; break;
			}
			chartRange(a, CHART_STAT_NAMES[k], need, need);
			chartCardLine(a, have, statCount == 1 ? (met & EVO_MET_STATS) : have >= need,
			              statCount == 1, statCount == 1 ? 0 : 8, NULL);
		}
	}

	/* the bonus: one of them is enough; one that always holds (a Rookie's battles -2 or techniques 0) makes it "any" */
	if ((r.battles != -1 && !isMaxBattles && r.battles <= 0) || (r.techs != -1 && r.techs <= 0) ||
	    (r.discipline != -1 && r.discipline <= 0)) {
		chartCardLine(CHART_TEXT_BONUS_ANY, CHART_NO_VALUE, 1, 1, 0, NULL);
	} else {
		chartCardLine(CHART_TEXT_BONUS_ONE_OF, CHART_NO_VALUE, met & EVO_MET_BONUS, 1, 0, NULL);
		if (r.digimon != -1) {
			sprintf(a, CHART_TEXT_FROM, hasDigimonRaised((uint16_t)r.digimon) ? (char *)digimonPara(r.digimon) : "?????");
			chartCardLine(a, CHART_NO_VALUE, CHART_DETAIL_ID == r.digimon, 0, 8,
			              CHART_DETAIL_ID == r.digimon ? CHART_TEXT_YES : CHART_TEXT_NO);
		}
		if (r.discipline != -1) {
			chartRange(a, CHART_TEXT_DISCIPLINE, r.discipline, r.discipline);
			chartCardLine(a, PARTNER_PARA.discipline, PARTNER_PARA.discipline >= r.discipline, 0, 8, NULL);
		}
		if (r.happiness != -1) {
			/* MOD (care numbers): the same 0..100 as the status screen; the rules keep -100..100 */
			chartRange(a, CHART_TEXT_HAPPINESS, HAPPINESS_SHOWN(r.happiness), HAPPINESS_SHOWN(r.happiness));
			chartCardLine(a, HAPPINESS_SHOWN(PARTNER_PARA.happiness), PARTNER_PARA.happiness >= r.happiness, 0, 8, NULL);
		}
		if (r.battles != -1) {
			chartRange(a, CHART_TEXT_BATTLES, isMaxBattles ? 0 : r.battles, r.battles);
			chartCardLine(a, PARTNER_PARA.battles,
			              isMaxBattles ? PARTNER_PARA.battles <= r.battles : PARTNER_PARA.battles >= r.battles, 0, 8,
			              NULL);
		}
		if (r.techs != -1) {
			chartRange(a, CHART_TEXT_TECHS, r.techs, r.techs);
			have = getNumMasteredMoves();
			chartCardLine(a, have, have >= r.techs, 0, 8, NULL);
		}
	}

	/* Count the same four marks as the game; the configured score decides readiness. */
	if (CHART_CARD_MINE) {
		k = ((met & EVO_MET_CARE) != 0) + ((met & EVO_MET_WEIGHT) != 0) + ((met & EVO_MET_STATS) != 0) +
		    ((met & EVO_MET_BONUS) != 0);
		sprintf(a, CHART_TEXT_VERDICT, k, evoPassPoints());
		if (evoRequirementPoints(met) >= evoPassPoints() && isPartnerModelFit(target)) {
			chartAppend(a + strlen(a), CHART_TEXT_READY);
			CHART_CARD_VERDICT_COLOR = 7;
		} else {
			CHART_CARD_VERDICT_COLOR = 6;
		}
		CHART_CARD_VERDICT_W = drawStringMode(a, 0, CHART_CARD_VERDICT_ROW, FIT_VANILLA);
		if (k < evoPassPoints()) {
			sprintf(a, CHART_TEXT_SHORT, evoPassPoints() - k);
			CHART_CARD_FOOTER_W = drawStringMode(a, 0, CHART_CARD_FOOTER_ROW, FIT_VANILLA);
		}
	}
}

/* a mark at screen (x, y): a tick or a cross */
static __attribute__((noinline)) void chartMark(int32_t x, int32_t y, int32_t yes)
{
	int32_t i;

	x -= 0xa0;
	y -= 0x78;
	for (i = 0; i < 2; i++) {
		if (yes) {
			drawLine3P(CHART_MARK_YES, x, y + 3 + i, x + 2, y + 5 + i, x + 6, y - 1 + i, 2, 0);
		} else {
			drawLine2P(CHART_MARK_NO, x + i, y, x + 5 + i, y + 5, 2, 0);
			drawLine2P(CHART_MARK_NO, x + 5 + i, y, x + i, y + 5, 2, 0);
		}
	}
}

/* Small pad outlines share the menu's line primitives; Square remains square at 1x. */
static __attribute__((noinline)) void chartButton(int32_t x, int32_t y, int32_t button)
{
	x -= 160;
	y -= 120;
	if (button == 0x80) {
		drawLine3P(0xc080ff, x, y, x + 8, y, x + 8, y + 8, 2, 0);
		drawLine3P(0xc080ff, x + 8, y + 8, x, y + 8, x, y, 2, 0);
	} else if (button == 0x10) {
		drawLine3P(0x80ff80, x, y + 8, x + 4, y, x + 8, y + 8, 2, 0);
		drawLine2P(0x80ff80, x, y + 8, x + 8, y + 8, 2, 0);
	} else {
		drawLine2P(0xffb080, x, y, x + 8, y + 8, 2, 0);
		drawLine2P(0xffb080, x + 8, y, x, y + 8, 2, 0);
	}
}

static __attribute__((noinline)) void chartCardPrompt(int32_t popup)
{
	int32_t x;
	int32_t u = popup ? (CHART_CARD_FOOTER_W + 7) & ~3 : 0;
	if (MOD_RENDER_PASS == 0 && CHART_CARD_ID == -1) {
		CHART_PROMPT_W = drawStringMode(popup ? CHART_TEXT_CLOSE : CHART_TEXT_VIEW, u, CHART_CARD_FOOTER_ROW,
		                              FIT_VANILLA);
		CHART_TEXT_TAKEN = 1;
		if (!popup) CHART_CARD_ID = -2;
	}
	x = (popup ? 286 : 294) - CHART_PROMPT_W;
	renderString(0, x - 160, (popup ? 197 : 205) - 120, CHART_PROMPT_W, 12, u, CHART_CARD_FOOTER_ROW, 2, 1);
	chartButton(x - 13, popup ? 198 : 206, popup ? CANCEL_BUTTON : 0x80);
}

static void chartCellRender(ChartCell *cell, int32_t x, int32_t y, int32_t color)
{
	if (cell->w) renderString(color, x - 160, y - 120, cell->w, 12, cell->u, cell->v, 2, 1);
}

static void chartCardScrollBar(void)
{
	int32_t top = CHART_CARD_SCROLL, count = CHART_CARD_COUNT - 1, i, h, y;
	if (count <= CHART_CARD_VISIBLE) return;
	for (i = 0; i < 4; i++) {
		drawLine2P(top ? 0xffffff : 0x7d6e5a, 126 + i, -44 - i, 132 - i, -44 - i, 2, 0);
		drawLine2P(top + CHART_CARD_VISIBLE < count ? 0xffffff : 0x7d6e5a,
		           126 + i, 66 + i, 132 - i, 66 + i, 2, 0);
	}
	h = 104 * CHART_CARD_VISIBLE / count;
	y = 80 + (104 - h) * top / (count - CHART_CARD_VISIBLE);
	renderBox(128, y - 120, 3, h, 120, 220, 255, 0, 2);
	renderBox(128, -40, 3, 104, 20, 30, 38, 0, 2);
}

static __attribute__((noinline)) void chartCardRender(int32_t target)
{
	ChartCardLine *l;
	int32_t k, y;
	if (CHART_CARD_ID != target) {
		if (MOD_RENDER_PASS != 0) return;
		chartCardBuild(target);
		CHART_NEEDS_W = drawStringMode(CHART_TEXT_NEEDS, 0, CHART_CARD_HEADER_ROW, FIT_VANILLA);
		CHART_HAVE_W = drawStringMode(CHART_TEXT_HAVE, 128, CHART_CARD_HEADER_ROW, FIT_VANILLA);
		chartCardPrompt(1);
		CHART_CARD_ID = target;
	}
	chartCellRender(&CHART_CARD[0].label, 160 - CHART_CARD[0].label.w / 2, 30, 4);
	if (CHART_CARD_VERDICT_W)
		renderString(CHART_CARD_VERDICT_COLOR, -CHART_CARD_VERDICT_W / 2, 45 - 120,
		             CHART_CARD_VERDICT_W, 12, 0, CHART_CARD_VERDICT_ROW, 2, 1);
	if (CHART_CARD_COUNT > 2) {
		renderString(1, 194 - CHART_NEEDS_W - 160, 59 - 120, CHART_NEEDS_W, 12, 0, CHART_CARD_HEADER_ROW, 2, 1);
		if (CHART_CARD_MINE)
			renderString(1, 280 - CHART_HAVE_W - 160, 59 - 120, CHART_HAVE_W, 12, 128, CHART_CARD_HEADER_ROW, 2, 1);
	}
	for (k = 1 + CHART_CARD_SCROLL; k < CHART_CARD_COUNT && k <= CHART_CARD_SCROLL + CHART_CARD_VISIBLE; k++) {
		l = &CHART_CARD[k];
		y = 72 + (k - 1 - CHART_CARD_SCROLL) * 12;
		chartCellRender(&l->label, 41 + l->indent, y, 0);
		chartCellRender(&l->need, 194 - l->need.w, y, 0);
		chartCellRender(&l->value, 280 - l->value.w, y, l->valueColor);
		if (l->mark) chartMark(30, y + 3, l->mark == 1);
	}
	if (CHART_CARD_FOOTER_W)
		renderString(0, 28 - 160, 197 - 120, CHART_CARD_FOOTER_W, 12, 0, CHART_CARD_FOOTER_ROW, 2, 1);
	chartCardPrompt(1);
	chartCardScrollBar();
	/* LIFO within each bucket: text, inner/outer frames, then fill. */
	renderBorderBox(19, 27, 281, 187, 0xaa823c, 0xaa823c, 44, 62, 74, 2);
	renderBorderBox(18, 26, 283, 189, 0xffebaa, 0xffebaa, 44, 62, 74, 2);
	/* All chart/menu/tabs use OT >=4. OT3 is after the field and before OT2.
	 * PSX average-black plus a small subtraction approximates 40% brightness. */
	renderBox(-160, -120, 320, 240, 12, 12, 12, 3, 3);
	renderBox(-160, -120, 320, 240, 0, 0, 0, 1, 3);
}

/* the detail closed: the overview's rows of the text area are drawn again */
static void chartCardRelease(void)
{
	RECT rect;

	CHART_POPUP = 0;
	CHART_CARD_ID = -1;
	if (!CHART_TEXT_TAKEN) {
		return;
	}
	setRECT(&rect, 0, CHART_CARD_ROW, 256, CHART_CARD_HEADER_ROW + 12 - CHART_CARD_ROW);
	clearTextSubArea(&rect);
	chartDrawLabels();
	CHART_TEXT_ID = -2;
	CHART_TEXT_TAKEN = 0;
}

GARBAGE_ARRAY(renderEvoChartDetail, (&MOVES_TECHSET_LINES[0].x1), 30, 9);

/* MOD (chart lists): when the "to" boxes take more than a page, arrows and a bar right of them (as the requirements
 * table's): an arrow is bright while there is a page that way */
static void chartToScrollBar(void)
{
	int32_t i;
	int32_t h;
	int32_t y;

	if (CHART_TO_N <= CHART_TO_PAGE) {
		return;
	}
	for (i = 0; i < 4; i++) {
		drawLine2P(CHART_TO_TOP > 0 ? 0xffffff : 0x7d6e5a, 131 + i, -76 - i, 137 - i, -76 - i, 4, 0);
		drawLine2P(CHART_TO_TOP + CHART_TO_PAGE < CHART_TO_N ? 0xffffff : 0x7d6e5a, 131 + i, 47 + i, 137 - i, 47 + i,
		           4, 0);
	}
	h = 116 * CHART_TO_PAGE / CHART_TO_N;
	y = -72 + (116 - h) * CHART_TO_TOP / (CHART_TO_N - CHART_TO_PAGE);
	if (y > 44 - h) {
		y = 44 - h;
	}
	renderBox(133, y, 3, h, 120, 220, 255, 0, 4);
	renderBox(133, -72, 3, 116, 20, 30, 38, 0, 4);
}

void renderEvoChartDetail(void)
{
	int32_t w; /* MOD (long names) */
	Line4Points *fromLines;
	Line4Points *toLines;
	int32_t j;
	ChartSprite *fromSprites;
	ChartSprite *toSprites;
	uint32_t color1;
	uint32_t color2;
	int32_t id;
	int32_t len;
	EvoClutTable clut;
	int32_t i;
	int32_t fromCount; /* MOD (chart lists): the boxes of each side, from its list */
	int32_t toCount;
	int32_t toFirst;
	int32_t fromSlot;
	int32_t toSlot;
	int16_t cx;
	int16_t cy;
	int32_t mine; /* MOD (chart criteria) */
	int32_t cardId;

	clut = EVO_CHART_DETAIL_COLORS;
	MENU_SUB_STATE = 2;
	drawEvoChartStrings(CHART_DETAIL_ID);
	fromCount = chartSide(0, &i, &fromSlot);
	toCount = chartSide(1, &toFirst, &toSlot);

	/* The partner's tree lights met evolutions; only a "to" box offers the modal. */
	mine = MOD_OPT_EVO_GUIDE && chartIsPartnerTree(); /* OPTIONS' EVOLUTION GUIDE off: the original chart */
	if (mine) {
		chartUpdateLit();
	}
	cardId = MOD_OPT_EVO_GUIDE && CHART_CURSOR_SIDE == 1 ? chartDetailBox(1, CHART_CURSOR_J, &cx, &cy) : 0;

	/* MOD (chart lists): an even count on the 4-box (from) or 6-box (to) layout, an odd one on the 5-box one, centred
	 * as the original rows were; the line colours stay those of the layout's slot */
	fromLines = ((fromCount % 2) == 0 ? CHART_FROM_LINES_EVEN : CHART_FROM_LINES_ODD) + fromSlot;
	fromSprites = ((fromCount % 2) == 0 ? CHART_FROM_SPRITES_EVEN : CHART_FROM_SPRITES_ODD) + fromSlot;
	toLines = ((toCount % 2) == 0 ? CHART_TO_LINES_EVEN : CHART_TO_LINES_ODD) + toSlot;
	toSprites = ((toCount % 2) == 0 ? CHART_TO_SPRITES_EVEN : CHART_TO_SPRITES_ODD) + toSlot;

	for (j = 0; j < fromCount; j++) {
		if (mine) {
			drawLine3P(CHART_FROM_LINE, fromLines->x1, fromLines->y1, fromLines->x2, fromLines->y2, fromLines->x3,
			           fromLines->y3, 4, 0);
			drawLine2P(CHART_FROM_LINE, fromLines->x3, fromLines->y3, fromLines->x4, fromLines->y4, 4, 0);
		} else {
			for (i = -1; i <= 1; i++) {
				color1 = i == 0 ? 0x794e3 : 0x65db;
				drawLine3P(color1, fromLines->x1, fromLines->y1 + i, fromLines->x2, fromLines->y2 + i,
				           fromLines->x3, fromLines->y3 + i, 4, 0);
				drawLine2P(color1, fromLines->x3, fromLines->y3 + i, fromLines->x4, fromLines->y4 + i, 4, 0);
			}
		}
		fromLines++;
	}

	for (j = 0; j < toCount; j++) {
		if (mine && !(CHART_LIT & (1 << (toFirst + j)))) {
			/* MOD (chart criteria): an evolution the partner does not meet, dim */
			drawLine3P(CHART_DIM_LINE, toLines->x1, toLines->y1, toLines->x2, toLines->y2, toLines->x3, toLines->y3, 4, 0);
			drawLine2P(CHART_DIM_LINE, toLines->x3, toLines->y3, toLines->x4, toLines->y4, 4, 0);
		} else {
			color1 = (MAIN_D_80124A84[(toSlot + j) * 2].r & 0xff) |
			         ((MAIN_D_80124A84[(toSlot + j) * 2].g & 0xff) << 8) |
			         ((MAIN_D_80124A84[(toSlot + j) * 2].b & 0xff) << 16);
			color2 = (MAIN_D_80124A84[((toSlot + j) * 2) + 1].r & 0xff) |
			         ((MAIN_D_80124A84[((toSlot + j) * 2) + 1].g & 0xff) << 8) |
			         ((MAIN_D_80124A84[((toSlot + j) * 2) + 1].b & 0xff) << 16);
			if (mine) { /* MOD (chart criteria): one it meets, green */
				color1 = CHART_LIT_CORE;
				color2 = CHART_LIT_EDGE;
			}
			for (i = -1; i <= 1; i++) {
				drawLine3P(i == 0 ? color1 : color2, toLines->x1, toLines->y1 + i, toLines->x2, toLines->y2 + i,
				           toLines->x3, toLines->y3 + i, 4, 0);
				drawLine2P(i == 0 ? color1 : color2, toLines->x3, toLines->y3 + i, toLines->x4, toLines->y4 + i, 4, 0);
			}
		}
		toLines++;
	}
	chartToScrollBar(); /* MOD (chart lists) */

	if (CHART_DETAIL_ID <= NUM_EVO_CHART_DIGIMON) {
		renderRectPolyFT4(-8, -0x14, 0x10, 0x10,
		                  EVO_CHART_BOXES[CHART_DETAIL_ID - 1].u,
		                  EVO_CHART_BOXES[CHART_DETAIL_ID - 1].v, 0x18,
		                  clut.m[EVO_CHART_BOXES[CHART_DETAIL_ID - 1].clut],
		                  4, 0);
	} else {
		renderChartPortrait(CHART_DETAIL_ID, -8, -0x14);
	}
	renderBorderBox(0x97, 99, 0x12, 0x12, 0xbebebe, 0x3c3c3c, 0x87, 0x87,
	                0x87, 4);

	for (j = 0; j < fromCount + toCount; j++) {
		id = j < fromCount ? CHART_FROM_IDS[j] : CHART_TO_IDS[toFirst + j - fromCount];
		if (j == fromCount) {
			fromSprites = toSprites;
		}
		/* MOD (portraits): beyond the chart's 61 the icon comes from DIGI.DAT */
		if (id <= NUM_EVO_CHART_DIGIMON && hasDigimonRaised(id & 0xffff) == 1) {
			renderRectPolyFT4(fromSprites->posX, fromSprites->posY,
			                  0x10, 0x10,
			                  EVO_CHART_BOXES[id - 1].u,
			                  EVO_CHART_BOXES[id - 1].v, 0x18,
			                  clut.m[EVO_CHART_BOXES[id - 1].clut],
			                  4, 0);
		} else if (id > NUM_EVO_CHART_DIGIMON && hasDigimonRaised(id & 0xffff) == 1) {
			renderChartPortrait(id, fromSprites->posX, fromSprites->posY);
		}
		if (j >= fromCount && DIGI_PARA(id).level == LEVEL_MEGA) {
			/* MOD (mega): the chart has no Mega column and no Mega portraits: a
			 * gold box with the level name under it */
			renderBorderBox(fromSprites->posX + 0x9f,
			                fromSprites->posY + 0x77, 0x12, 0x12,
			                0x60e0ff, 0x104878, 0xb0, 0x88, 0x28, 4);
			renderString(0, fromSprites->posX + 8 - EVO_CHART_MEGA_WIDTH / 2,
			             fromSprites->posY + 0x13, EVO_CHART_MEGA_WIDTH, 0xc, 0,
			             EVO_CHART_MEGA_ROW, 4);
		} else {
			renderBorderBox(fromSprites->posX + 0x9f,
			                fromSprites->posY + 0x77, 0x12, 0x12,
			                0xbebebe, 0x3c3c3c, 0x87, 0x87, 0x87, 4);
		}
		fromSprites++;
	}

#if !VERSION_IS(US)
	renderString(3, -0x19, -0x4f, 0x30, 0xc, 0, 0x18, 4);
	renderString(0, -0x56, 0x3a, 0x24, 0xc,
	             ((DIGI_PARA(CHART_DETAIL_ID).level - 1) * 36) + 0x30, 0x18,
	             4);
#else
	/* MOD (language): each language's crops (CHART_TREE_CROP, CHART_LEVEL_CROPS) */
	renderString(3, CHART_TREE_CROP.x, -0x4f, CHART_TREE_CROP.w, 0xc,
	             CHART_TREE_CROP.u, 0x18, 4);
	switch (DIGI_PARA(CHART_DETAIL_ID).level) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		i = DIGI_PARA(CHART_DETAIL_ID).level - 1;
		renderString(0, CHART_LEVEL_CROPS[i].x, 0x3a, CHART_LEVEL_CROPS[i].w, 0xc, CHART_LEVEL_CROPS[i].u,
		             i < 2 ? 0x18 : i < 4 ? 0x24 : 0x3c, 4);
		break;
	case LEVEL_MEGA: /* MOD (mega) */
		renderString(0, -0x44 - EVO_CHART_MEGA_WIDTH / 2, 0x3a, EVO_CHART_MEGA_WIDTH, 0xc, 0,
		             EVO_CHART_MEGA_ROW, 4);
		break;
	}
#endif
#if VERSION_IS(US)
	/* MOD (long names): a name that fits the original 0x78 is placed and copied as the original did; a longer
	 * one (drawn tighter if it must, drawEvoChartStrings) is centred in the level's panel by its width */
	if (measureStringMode(DIGIMON_NAME(CHART_DETAIL_ID), FIT_VANILLA) <= 0x78) {
		len = strlen(DIGIMON_NAME(CHART_DETAIL_ID)) / 2;
		renderString(0, -0x5c - ((len - 4) * 6), 0x4d, 0x78, 0xc, 0, 0x30, 4);
	} else {
		w = measureStringMode(DIGIMON_NAME(CHART_DETAIL_ID),
				      fitStringMode(DIGIMON_NAME(CHART_DETAIL_ID), CHART_NAME_W));
		renderString(0, -0x42 - w / 2, 0x4d, w, 0xc, 0, 0x30, 4);
	}
#else
	len = strlen(DIGIMON_NAME(CHART_DETAIL_ID)) / 2;
	renderString(0, -0x5c - ((len - 4) * 6), 0x4d, 0x78, 0xc, 0, 0x30, 4);
#endif
	/* MOD (portraits): the cursor of the navigation on the box it points at; MOD (chart criteria): always shown (the
	 * card is that box's), pulsing between two golds */
	if (chartDetailBox(CHART_CURSOR_SIDE, CHART_CURSOR_J, &cx, &cy) != 0) {
		if ((PLAYTIME_FRAMES % 10) < 7) {
			renderBorderBox(cx + 0x9f - 2, cy + 0x77 - 2, 0x16, 0x16, 0x20e0ff, 0x1070a0, 0xd0, 0xc0, 0x20, 4);
		} else {
			renderBorderBox(cx + 0x9f - 2, cy + 0x77 - 2, 0x16, 0x16, 0x1890b8, 0x0c4870, 0x90, 0x84, 0x16, 4);
		}
	}
	for (i = 0; i < 6; i++) {
		renderInsetBox(MAIN_D_80124B48[i].posX, MAIN_D_80124B48[i].posY,
		               MAIN_D_80124B48[i].width, MAIN_D_80124B48[i].height, 4);
	}
	if (cardId > 0) {
		if (CHART_POPUP) chartCardRender(cardId);
		else chartCardPrompt(0);
	}
}

void renderMedalView(void)
{
	int32_t i;
	int32_t j;

	switch (MENU_STATE) {
	case 0:
		if (drawMedalViewStrings() == 1) {
			MENU_STATE = 1;
		}
		return;
	case 3:
		if (drawMedalViewStrings() == 1) {
			MENU_STATE = 2;
		}
	case 2:
		renderRectPolyFT4(((MEDAL_SELECTOR_INDEX % 5) * 38) - 0x7e,
		                  ((MEDAL_SELECTOR_INDEX / 5) * 24) - 0x3d, 0x18, 0x18, 0, 0xd0,
		                  0x18, 0x7dc7, 1, 0);
	case 1:
		renderSeparatorLines(MAIN_D_8012472C, 0xe, 5);
		renderString(3, MEDAL_VIEW_CROPS[0].x, -0x51, MEDAL_VIEW_CROPS[0].w, 0xc, MEDAL_VIEW_CROPS[0].u, 0xc, 5, 1); /* MOD (language) */

		for (i = 0; i < 0xf; i++) {
			if (hasMedal(i) != 0) {
				if (MEDAL_SELECTOR_INDEX == i) {
					if (MENU_STATE != 3) {
#if !VERSION_IS(US)
						renderString(0, -0x42, 0x17, 0x84, 0xc, 0, 0x18, 5, 1);
#else
						renderString(0, -0x5d, 0x17, 0xac, 0xc, 0, 0x18, 5, 1);
#endif
						for (j = 0; j < 3; j++) {
							renderString(0, -0x7e, j * 13 + 0x27, 0xfc, 0xc, 0,
							             j * 12 + 0x24, 5, 1);
						}
					}
					renderDigiviceMedals();
				}
			} else {
#if !VERSION_IS(US)
				if (MEDAL_SELECTOR_INDEX == i) {
					renderString(0, -0x42, 0x17, 0xc, 0xc, 0xe4, 0xc, 5, 1);
					renderString(0, -0x7e, 0x27, 0xc, 0xc, 0xe4, 0xc, 5, 1);
				}
#endif
				renderRectPolyFT4(((i % 5) * 38) - 0x7c, ((i / 5) * 24) - 0x3d, 0xf,
				                  0x18, 0xf0, 0xc8, 0x18, 0x7dc7, 5, 0);
			}

			renderRectPolyFT4(((i % 5) * 38) - 0x7a, ((i / 5) * 24) - 0x3d, 0xf, 0x18,
			                  0xf0, 0xb0, 0x18, 0x7dc7, 5, 0);
		}

#if !VERSION_IS(US)
		renderInsetBox(0x5c, 0x8d, 0x88, 0xe, 5);
#else
		renderInsetBox(0x3e, 0x8d, 0xb0, 0xe, 5);
#endif
#if !VERSION_IS(US)
		renderString(FOOTER_GLYPH1_COLOR, -0x23, 0x55, 0xc, 0xc, 0x6c, 0xc, 5, 1);
		renderString(0, -0x17, 0x55, 0x18, 0xc, 0x84, 0xc, 5, 1);
		renderString(0, 1, 0x55, 0x24, 0xc, 0x9c, 0xc, 5, 1);
		renderString(FOOTER_GLYPH2_COLOR, 0x2b, 0x55, 0xc, 0xc, 0x78, 0xc, 5, 1);
		renderString(0, 0x37, 0x55, 0x18, 0xc, 0x84, 0xc, 5, 1);
		renderString(0, 0x4f, 0x55, 0x24, 0xc, 0xc0, 0xc, 5, 1);
#else
		/* MOD (language): each language's crops (MEDAL_VIEW_CROPS) */
		for (j = 1; j < 6; j++) {
			renderString(j == 1 ? FOOTER_GLYPH1_COLOR : j == 4 ? FOOTER_GLYPH2_COLOR : 0, MEDAL_VIEW_CROPS[j].x, 0x55,
			             MEDAL_VIEW_CROPS[j].w, 0xc, MEDAL_VIEW_CROPS[j].u, 0xc, 5, 1);
		}
#endif
	}
}

void renderCardsView(void)
{
	CardSprites sprites;
	int32_t j;
	int32_t i;
	int8_t card;
	int8_t amount;

	sprites = CARD_SPRITE_CLUT;

	switch (MENU_STATE) {
	case 0:
		if (drawCardViewStrings() == 1) {
			MENU_STATE = 1;
		}
		return;
	case 2:
	case 3:
		renderRectPolyFT4(((SELECTED_CARD % 11) * 24) - 0x84,
		                  ((SELECTED_CARD / 11) * 24) - 0x40, 0x18, 0x14, 0, 0xe8,
		                  0x18, 0x7dc7, 5, 0);
	case 1:
		for (i = 0; i < 6; i++) {
			for (j = 0; j < 11; j++) {
				card = j + i * 11;
				amount = getCardAmount(card);
				if (amount > 0) {
					renderRectPolyFT4(j * 24 - 0x81, i * 24 - 0x3e, 0x10, 0x10,
					                  (card % 8) * 32, (card / 8) * 16, 0x18,
					                  sprites.v[card] * 64 + 0x7a07, 5, 0);
				}
				renderBorderBox(j * 24 + 0x1e, i * 24 + 0x39, 0x12, 0x12,
				                0xbebebe, 0x3c3c3c, 0x69, 0x69, 0x69, 5);
			}
		}

		/* MOD (language): each language's crops (CARD_LIST_CROPS) */
		renderString(3, CARD_LIST_CROPS[0].x, -0x50, CARD_LIST_CROPS[0].w, 0xc, CARD_LIST_CROPS[0].u, 0xc, 5, 1);
		renderString(FOOTER_GLYPH1_COLOR, CARD_LIST_CROPS[1].x, 0x53, CARD_LIST_CROPS[1].w, 0xc, CARD_LIST_CROPS[1].u, 0xc, 5, 1);
		renderString(0, CARD_LIST_CROPS[2].x, 0x53, CARD_LIST_CROPS[2].w, 0xc, CARD_LIST_CROPS[2].u, 0xc, 5, 1);
		renderString(FOOTER_GLYPH2_COLOR, CARD_LIST_CROPS[3].x, 0x53, CARD_LIST_CROPS[3].w, 0xc, CARD_LIST_CROPS[3].u, 0xc, 5, 1);
		renderString(0, CARD_LIST_CROPS[4].x, 0x53, CARD_LIST_CROPS[4].w, 0xc, CARD_LIST_CROPS[4].u, 0xc, 5, 1);
	}
}

void renderInsetBox(int16_t x, int16_t y, int16_t w, int16_t h, int32_t otz)
{
	x -= 0xa0;
	y -= 0x78;
	drawLine3P(0x20202, x, y + h, x, y, x + w, y, otz, 0);
	drawLine3P(0xa08769, x, y + h, x + w, y + h, x + w, y, otz, 0);
	renderBox(x, y, w, h, 0x35, 0x4b, 0x5c, 0, otz);
}

void renderBox(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t r,
               uint8_t g, uint8_t b, uint8_t flags, int32_t otz)
{
	GsBOXF box;

	if ((flags & 1) != 0) {
		if ((flags & 2) != 0) {
			box.attribute = 0x60000000;
		} else {
			box.attribute = 0x40000000;
		}
	} else {
		box.attribute = 0;
	}

	box.x = x;
	box.y = y;
	setWH(&box, w, h);
	box.r = r;
	box.g = g;
	box.b = b;

	GsSortBoxFill(&box, ACTIVE_ORDERING_TABLE, otz);

	if ((flags & 0x80) != 0) {
		drawLine3P(0x20202,
		           x - 1, y - 1,
		           x + 0xd, y - 1,
		           x + 0xd, y + 0xc,
		           otz, 0);
		drawLine3P(0x20202,
		           x - 1, y - 1,
		           x - 1, y + 0xc,
		           x + 0xd,
		           y + 0xc,
		           otz, 0);
	}
}

void renderBorderBox(int16_t x, int16_t y, int16_t w, int16_t h, int32_t c1,
                     int32_t c2, uint8_t r, uint8_t g, uint8_t b, int32_t a10)
{
	x -= 0xa0;
	y -= 0x78;
	drawLine3P(c1, x, y + h, x, y, x + w, y, a10, 0);
	drawLine3P(c2, x, y + h, x + w, y + h, x + w, y, a10, 0);
	renderBox(x, y, w, h, r, g, b, 0, a10);
}

int32_t drawDigimonStatsStrings(void)
{
	RECT rect;
	int32_t i;

	rect = DIGIMON_STATS_TEXT_AREA;

	switch (MENU_SUB_STATE) {
	case 0:
		statsCareLayout(MOD_OPT_BARS);
		clearTextSubArea(&rect);
		DRAW_STRING_CELLS(MAIN_D_80124C54, 0, 0xf0);
		DRAW_STRING_PIECES(MAIN_D_80124C0C[2], 0, 0x24, 0);
		DRAW_STRING_PIECES(MAIN_D_80124C0C[3], 0x24, 0x24, 0);
		MENU_SUB_STATE = 1;
		break;
	case 1:
#if !VERSION_IS(US)
		drawString(DIGIMON_NAME(ENTITY_TABLE[1]->type), 0x9c, 0x24);
#else
		/* MOD (long names): drawn tighter when it does not fit the field */
		drawStringMode(DIGIMON_NAME(ENTITY_TABLE[1]->type), 0, 0x48,
			       fitStringMode(DIGIMON_NAME(ENTITY_TABLE[1]->type), STATUS_NAME_W));
		if (modPartnerNameIsSpecies(ENTITY_TABLE[1]->type)) { /* MOD (optional name): the species' name, as above */
			drawStringMode(PARTNER_ENTITY.name, 0, 0x3c, fitStringMode(PARTNER_ENTITY.name, STATUS_NAME_W));
		} else
#endif
		drawString(PARTNER_ENTITY.name, 0, 0x3c);
		MENU_SUB_STATE = 2;
		break;
	case 2:
		DRAW_STRING_PIECES(STATUS_VIEW_LABELS[1], 0, 0x30, 0, 0x18, 0x48, 0x78, 0x9c, 0xb4, 0xcc);
		MENU_SUB_STATE = 3;
		break;
	case 3:
		DRAW_STRING_PIECES(STATUS_VIEW_LABELS[2], 0, 0x18, 0, 0x30, 0x60, 0x90, 0xc0);
		if (MOD_OPT_BARS) { /* MOD (care numbers): the three labels renderDigimonStatsView adds */
			for (i = 0; i < 2; i++) {
				STATS_CARE_LABEL_W[i] = drawStringMode(STATS_CARE_LABELS[i], i << 6, STATS_CARE_TEXT_V,
				                                       fitStringMode(STATS_CARE_LABELS[i], i == 1 ? 0x2c : 0x24));
			}
		}

		return 1;
	}

	return 0;
}

void renderDigimonStatsBar(a, b, c, d, e)
	int16_t a;
int16_t b;
uint8_t c;
int16_t d;
int16_t e;
{
	uint8_t w;

	/* MOD (caps): full at the old limit, which the caller passes (the
	 * bar has no room past it, and uint8_t w would wrap) */
	if (a > b) {
		a = b;
	}
	w = c * a / b;
	renderBox(d, e, w, 2, 0x32, 0xc8, 0xc8, 0, 5);
}

void renderDigimonStatusConditions(int32_t condition)
{
	ConditionMaskTable masks;
	int32_t i;
	uint8_t bobY;
	int16_t clut;

	masks = MAIN_D_80123DD0;
	if (PLAYTIME_FRAMES % 16 < 3) {
		bobY = 0x82;
	} else if (PLAYTIME_FRAMES % 16 < 5) {
		bobY = 0x80;
	} else if (PLAYTIME_FRAMES % 16 < 7) {
		bobY = 0x7d;
	} else if (PLAYTIME_FRAMES % 16 < 9) {
		bobY = 0x79;
	} else if (PLAYTIME_FRAMES % 16 < 11) {
		bobY = 0x7d;
	} else if (PLAYTIME_FRAMES % 16 < 13) {
		bobY = 0x80;
	} else {
		bobY = 0x82;
	}
	for (i = 0; i < 6; i++) {
		if (condition & masks.v[i]) {
			clut = 0x7a06;
			if ((i == 0) || (i == 4)) {
				clut = 0x7a86;
			}
			if (i == 5) {
				clut = 0x7a46;
			}
			renderRectPolyFT4(i * 0xf - 0x8a, bobY - 0x78, 0xc, 0xc, i * 0xc + 0x30, 0x8c, 5, clut, 5, 0);
		}
	}

	if (condition & 8) {
		renderRectPolyFT4(-0x30, bobY - 0x78, 0xc, 0xc, 0x60, 0xc8, 5, 0x7a06, 5, 0);
	}
}

static VECTOR modDigiviceSavedPos;
static SVECTOR modDigiviceSavedRot;

void renderDigiviceEntity(entity, entityId)
	Entity *entity;
int8_t entityId;
{
	MATRIX m;
	VECTOR savedPos;
	SVECTOR savedRot;
	GsF_LIGHT lights[3];
	PositionData *pos;
	int32_t type;
	int32_t bone;
	int32_t count;
	int32_t i;
	uint8_t anim;

	FRAMEBUFFER_OT[0]->length = 9;
	FRAMEBUFFER_OT[0]->org = FRAMEBUFFER0_ORIGIN;
	FRAMEBUFFER_OT[1]->length = 9;
	FRAMEBUFFER_OT[1]->org = FRAMEBUFFER1_ORIGIN;

	GsSetProjection(0x200);
	GsSetRefView2(&DIGIVICE_ENTITY_VIEW);
	GsClearOt(0, 5, FRAMEBUFFER_OT[ACTIVE_FRAMEBUFFER]);

	for (i = 0; i < 3; i++) {
		lights[i].vx = lights[i].vy = lights[i].vz = (i == 0) ? 100 : 0;
		lights[i].r = lights[i].g = lights[i].b = 0x80;
		GsSetFlatLight(i, &lights[i]);
	}

	type = entity->type;
	anim = entity->anim.animId;
	pos = entity->posData;
	count = DIGI_PARA(type).boneCount;
	modInterpUnapplyEntity(entityId); /* MOD (fps): its own position */
	savedPos = pos->location;
	savedRot = pos->rotation;
	/* MOD (fps): this moves the live entity to the menu view, poses it and
	 * puts it back (restarting its animation), on every render pass. The
	 * extra passes put it back where it was before the first pass, so they
	 * leave the entity exactly as the first one did. */
	if (MOD_RENDER_PASS == 0) {
		modDigiviceSavedPos = savedPos;
		modDigiviceSavedRot = savedRot;
	} else {
		savedPos = modDigiviceSavedPos;
		savedRot = modDigiviceSavedRot;
	}
	setEntityPosition(entityId, RAISE_ROW(type).viewX,
	                  RAISE_ROW(type).viewY,
	                  RAISE_ROW(type).viewZ);
	setEntityRotation(entityId, 0, 0, 0);
	setupEntityMatrix(entityId);
	startAnimation(entity, 0);
	tickAnimation(entity);
	m = GsWSMATRIX;

	for (bone = 0; bone < count; pos++, bone++) {
		if (pos->obj.tmd != NULL) {
			GsGetLw(pos->obj.coord2, &m);
			GsSetLightMatrix(&m);
			GsGetLs(pos->obj.coord2, &m);
			GsSetLsMatrix(&m);
			GsSortObject4(&pos->obj, FRAMEBUFFER_OT[ACTIVE_FRAMEBUFFER], 5,
			              getScratchAddr(0));
		}
	}

	GsSortOt(FRAMEBUFFER_OT[ACTIVE_FRAMEBUFFER], ACTIVE_ORDERING_TABLE);
	GsSetProjection(VIEWPORT_DISTANCE);
	GsSetRefView2(&GS_VIEWPOINT);

	for (i = 0; i < 3; i++) {
		GsSetFlatLight(i, &LIGHT_DATA.light[i]);
	}

	setEntityPosition(entityId, savedPos.vx, savedPos.vy, savedPos.vz);
	setEntityRotation(entityId, savedRot.vx, savedRot.vy, savedRot.vz);
	setupEntityMatrix(entityId);
	startAnimation(entity, anim);
	tickAnimation(entity);
}

void renderDigiviceMedals(void)
{
	MATRIX m;

	m = GsWSMATRIX;
	FRAMEBUFFER_OT[0]->length = 9;
	FRAMEBUFFER_OT[0]->org = FRAMEBUFFER0_ORIGIN;
	FRAMEBUFFER_OT[1]->length = 9;
	FRAMEBUFFER_OT[1]->org = FRAMEBUFFER1_ORIGIN;

	GsSetProjection(0x400);
	GsSetRefView2(&MEDAL_VIEW);
	GsClearOt(0, 1, FRAMEBUFFER_OT[ACTIVE_FRAMEBUFFER]);

	MEDAL_ROTATION.vy += 0x64;
	RotMatrix(&MEDAL_ROTATION, &MEDAL_COORDINATES.coord);
	MEDAL_COORDINATES.flg = 0;
	drawObject(&MEDAL_OBJECT, FRAMEBUFFER_OT[ACTIVE_FRAMEBUFFER], 5);
	GsSortOt(FRAMEBUFFER_OT[ACTIVE_FRAMEBUFFER], ACTIVE_ORDERING_TABLE);

	GsSetProjection(VIEWPORT_DISTANCE);
	GsSetRefView2(&GS_VIEWPOINT);
}

int32_t drawDigimonMovesText(void)
{
	RECT rect;

	rect = MOVES_VIEW_TEXT_AREA;
	switch (MENU_SUB_STATE) {
	case 0:
		clearTextSubArea(&rect);
		DRAW_STRING_CELLS(MAIN_D_80124C54, 0, 0xf0);
		if (PARTNER_ENTITY.digimonEntity.stats.base.moves[0] != 0xff) {
			EQUIPPED_MOVES[0] = entityGetTechFromAnim(ENTITY_TABLE[1],
			                                          PARTNER_ENTITY.digimonEntity.stats.base.moves[0]);
			drawStringMode(MOVE_NAMES[EQUIPPED_MOVES[0]], 0, 0x18,
		              fitStringMode(MOVE_NAMES[EQUIPPED_MOVES[0]], MOVE_NAME_WIDTH));
		} else {
			EQUIPPED_MOVES[0] = 0xff;
		}
		MENU_SUB_STATE = 1;
		break;
	case 1:
		DRAW_STRING_PIECES_AT(STATUS_VIEW_LABELS[3], 0, 0x48, TECH_TITLE_PIECES); /* MOD (language) */
		if (PARTNER_ENTITY.digimonEntity.stats.base.moves[1] != 0xff) {
			EQUIPPED_MOVES[1] = entityGetTechFromAnim(ENTITY_TABLE[1],
			                                          PARTNER_ENTITY.digimonEntity.stats.base.moves[1]);
			drawStringMode(MOVE_NAMES[EQUIPPED_MOVES[1]], 0, 0x24,
		              fitStringMode(MOVE_NAMES[EQUIPPED_MOVES[1]], MOVE_NAME_WIDTH));
		} else {
			EQUIPPED_MOVES[1] = 0xff;
		}
		MENU_SUB_STATE = 2;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
		DrawSync(0);
#endif
		break;
	case 2:
		DRAW_STRING_PIECES_AT(STATUS_VIEW_LABELS[6], MOVE_STATUS_ROW_X, 0x6c, MOVE_STATUS_PIECES); /* MOD (language) */
		if (PARTNER_ENTITY.digimonEntity.stats.base.moves[2] != 0xff) {
			EQUIPPED_MOVES[2] = entityGetTechFromAnim(ENTITY_TABLE[1],
			                                          PARTNER_ENTITY.digimonEntity.stats.base.moves[2]);
			drawStringMode(MOVE_NAMES[EQUIPPED_MOVES[2]], 0, 0x30,
		              fitStringMode(MOVE_NAMES[EQUIPPED_MOVES[2]], MOVE_NAME_WIDTH));
		} else {
			EQUIPPED_MOVES[2] = 0xff;
		}
		MENU_SUB_STATE = 3;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
		DrawSync(0);
#endif
		break;
	case 3:
		if (PARTNER_ENTITY.digimonEntity.stats.base.moves[3] != 0xff) {
			EQUIPPED_MOVES[3] = entityGetTechFromAnim(ENTITY_TABLE[1],
			                                          PARTNER_ENTITY.digimonEntity.stats.base.moves[3]);
			drawStringMode(MOVE_NAMES[EQUIPPED_MOVES[3]], 0, 0x3c,
		              fitStringMode(MOVE_NAMES[EQUIPPED_MOVES[3]], FINISHER_NAME_WIDTH));
		} else {
			EQUIPPED_MOVES[3] = 0xff;
		}
		DRAW_STRING_PIECES(STATUS_VIEW_LABELS[4], 0, 0x6c, 0, 0x30);
		MENU_SUB_STATE = 4;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
		DrawSync(0);
#endif
		break;
	case 4:
		DRAW_STRING_PIECES(STATUS_VIEW_LABELS[5], 0, 0x54, 0, 0xc);
		MENU_SUB_STATE = 5;
		break;
	case 5:
		DRAW_STRING_PIECES(STATUS_VIEW_LABELS[7], 0, 0x60, 0, 0xc, 0x90);
		DRAW_STRING_PIECES(TECH_VIEW_LABELS[0], 0, 0x78, 0, 0x24, 0x48, 0x6c);
		MOVE_SELECT_BOX_X = 0x73;
		MOVE_SELECT_BOX_Y = 0x6f;
		return 1;
	}
	return 0;
}

int32_t drawPlayerInfoStrings(void)
{
	RECT rect;
#if VERSION_IS(US)
	char buf[8];
#endif
	int32_t i;

	rect = PLAYER_INFO_TEXT_AREA;
	switch (MENU_SUB_STATE) {
	case 0:
		clearTextSubArea(&rect);
		DRAW_STRING_CELLS(MAIN_D_80124C54, 0, 0xf0);
		MENU_SUB_STATE = 1;
		break;
	case 1:
#if VERSION_IS(US)
		sprintf(buf, FMT_PLAYTIME, (int)PLAYTIME_HOURS, (int)PLAYTIME_MINUTES);
		drawString(buf, 0, 0xe4);
#endif
	case 2:
	case 3:
#if FONT_SWITCH
		if (FONT_PROPORTIONAL) {
			drawStringPieces(PLAYER_VIEW_LABELS[MENU_SUB_STATE + 1], 0, (MENU_SUB_STATE - 1) * 0xc + 0xc,
			                 PLAYER_LABEL_PIECES[MENU_SUB_STATE - 1]);
		} else {
			drawString(PLAYER_VIEW_LABELS[MENU_SUB_STATE + 1], 0, (MENU_SUB_STATE - 1) * 0xc + 0xc);
		}
#else
		drawString(PLAYER_VIEW_LABELS[MENU_SUB_STATE + 1], 0,
		           (MENU_SUB_STATE - 1) * 0xc + 0xc);
#endif
		MENU_SUB_STATE++;
		break;
	case 4:
		drawString((char *)digimonPara(0), 0, 0x30);
		for (i = 0; i < 4; i++) {
			drawString(TAMER_LEVEL_TITLES[i], 0, i * 0xc + 0x40);
		}
		MENU_SUB_STATE = 5;
		break;
	case 5:
		for (i = 4; i < 0xb; i++) {
			drawString(TAMER_LEVEL_TITLES[i], 0, i * 0xc + 0x40);
		}
		return 1;
	}
	return 0;
}

int32_t drawEvoChartStrings(int32_t arg)
{
	RECT rect1;
	RECT rect2;

	rect1 = EVO_CHART_TEXT_AREA;
	rect2 = EVO_CHART_NAME_AREA;
	switch (MENU_SUB_STATE) {
	case 0:
		clearTextSubArea(&rect1);
		DRAW_STRING_CELLS(MAIN_D_80124C54, 0, 0xf0);
		DRAW_STRING_PIECES(CARD_CHART_LABELS[1], 0, 0xc, 0, 0xc, 0x48, 0x54);
		CHART_SELECTED_COLUMN = CHART_SELECTED_ROW = 0;
		MENU_SUB_STATE = 1;
		/* fall through */
	case 1:
		DRAW_STRING_PIECES_AT(CARD_CHART_LABELS[2], 0, 0x18, CHART_TREE_ROW_PIECES); /* MOD (language) */
#if VERSION_IS(US)
		DRAW_STRING_PIECES_AT(CARD_CHART_LABELS[3], 0, 0x24, CHART_LEVEL_ROW_PIECES); /* MOD (language) */
		DRAW_STRING_PIECES(CARD_CHART_LABELS[4], 0, 0x3c, 0 | PIECE_CENTRE);
		drawString(EVO_CHART_MEGA_LABEL, 0, EVO_CHART_MEGA_ROW); /* MOD (mega) */
#endif
		return 1;
	case 2:
		clearTextSubArea(&rect2);
#if VERSION_IS(US)
		/* MOD (long names): drawn tighter when it does not fit the panel (renderEvoChartDetail) */
		drawStringMode((char *)digimonPara(arg), 0, 0x30, fitStringMode((char *)digimonPara(arg), CHART_NAME_W));
#else
		drawString((char *)digimonPara(arg), 0, 0x30);
#endif
	}
	return 0;
}

int32_t drawMedalViewStrings(void)
{
	RECT rect1;
	RECT rect2;

	rect1 = MEDAL_VIEW_TEXT_AREA;
	rect2 = MEDAL_DETAIL_AREA;
	switch (MENU_SUB_STATE) {
	case 0:
		clearTextSubArea(&rect1);
		DRAW_STRING_CELLS(MAIN_D_80124C54, 0, 0xf0);
		MENU_SUB_STATE = 1;
		break;
	case 1:
		DRAW_STRING_PIECES_AT(PLAYER_VIEW_LABELS[1], 0, 0xc, MEDAL_VIEW_PIECES); /* MOD (language) */
		MENU_SUB_STATE = 2;
		break;
	case 2:
		drawString(MEDAL_NAMES[MEDAL_SELECTOR_INDEX], 0, 0x18);
		MENU_SUB_STATE = 3;
		break;
	case 3:
	case 4:
	case 5:
		drawString(MEDAL_DESCRIPTIONS[(MENU_SUB_STATE - 3) + MEDAL_SELECTOR_INDEX * 3], 0,
		           (MENU_SUB_STATE - 3) * 0xc + 0x24);
		MENU_SUB_STATE++;
		if (MENU_SUB_STATE == 6) {
			return 1;
		}
		break;
	case 6:
		clearTextSubArea(&rect2);
		MENU_SUB_STATE = 2;
	}
	return 0;
}

int32_t drawCardViewStrings(void)
{
	RECT rect;

	rect = CARD_VIEW_TEXT_AREA;
	switch (MENU_SUB_STATE) {
	case 0:
		clearTextSubArea(&rect);
		DRAW_STRING_CELLS(MAIN_D_80124C54, 0, 0xF0);
		MENU_SUB_STATE = 1;
		break;
	case 1:
		DRAW_STRING_PIECES_AT(CARD_CHART_LABELS[0], 0, 0xC, CARD_LIST_PIECES); /* MOD (language) */
		SELECTED_CARD = 0;
		return 1;
	}
	return 0;
}

void renderSeparatorLines(Line *lines, int8_t count, int32_t zIndex)
{
	RGB8 *color;
	int32_t i;

	for (i = 0; i < count; lines++, i++) {
		color = &UI_LINES_BORDER_COLOR[lines->color];
		drawLine2P((color->r & 0xff) | ((color->g & 0xff) << 8) |
		                            ((color->b & 0xff) << 16),
		           lines->x1, lines->y1, lines->x2, lines->y2,
		           zIndex, 0);
	}
}

void renderRectPolyFT4(int16_t posX, int16_t posY, uint8_t width,
                       uint8_t height, uint8_t texX, uint8_t texY,
                       int16_t texturePage, int16_t clut, int32_t zIndex,
                       int8_t flag)
{
	POLY_FT4 *prim;
	GsOT_TAG *tags;

	tags = ACTIVE_ORDERING_TABLE->org;
	prim = (POLY_FT4 *)GsGetWorkBase();
	SetPolyFT4(prim);
	if (flag & 0x40) {
		SetSemiTrans(prim, 1);
	}
	if ((texY + height) >= 0x100) {
		height--;
	}
	if ((texX + width) >= 0x100) {
		width--;
	}
	setUVDataPolyFT4(prim, texX, texY, width, height);
	if (flag & 2) {
		setPosDataPolyFT4(prim, posX, posY, width * 2, height * 2);
	} else if (flag & 4) {
		setPosDataPolyFT4(prim, posX, posY, width * 2, height * 2);
		prim->x0 += width * 2;
		prim->x1 -= width * 2;
		prim->x2 += width * 2;
		prim->x3 -= width * 2;
	} else {
		setPosDataPolyFT4(prim, posX, posY, width, height);
	}
	if (flag & 1) {
		setRGB0(prim, 0x32, 0x32, 0x32);
	} else {
		setRGB0(prim, 0x80, 0x80, 0x80);
	}
	prim->tpage = texturePage;
	prim->clut = clut;
	AddPrim(&tags[zIndex], prim);
	prim++;
	GsSetWorkBase((PACKET *)prim);
	if (flag & 0x80) {
		drawLine3P(0x20202, posX - 1, posY - 1, posX + 0xd,
		           posY - 1, posX + 0xd, posY + 0xc, 2, 0);
		drawLine3P(0x20202, posX - 1, posY - 1, posX - 1,
		           posY + 0xc, posX + 0xd, posY + 0xc, 2, 0);
	}
}
