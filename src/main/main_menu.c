extern int MOD_CHEATS_CARD_MODE, MOD_CHEATS_CARD_DONE;

#include <stdio.h>
#include <string.h>

#include <kernel.h>
#include <libgs.h>
#include <libmcrd.h>

#include <dw/entity.h>
#include <dw/font.h>
#include <dw/ids.h>
#include <dw/mod_options.h>
#include <dw/mod_fps.h>
#include <dw/mod_version.h>
#include <dw/mod_rules.h>
#include <dw/pstat.h>
#include <dw/save_tags.h>
#include <dw/input.h>
#include <dw/item.h>
#include <dw/kar.h>
#include <dw/main.h>
#include <dw/map.h>
#include <dw/move.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/script.h>
#include <dw/sjis.h>
#include <dw/sound.h>
#include <dw/species.h>
#include <dw/types.h>
#include <dw/utils.h>
#include <dw/version.h>
#include <dw/vs.h>
#include <dw/world_object.h>
#include <dw/screen.h>
#include <text/main/main_menu.h>

#include "common.h"

void modMenuVsBattle(int32_t trial); /* MOD (menu overlay) */

#if !VERSION_IS(US)
#define YES_TEXT_W 0x18
#define NO_TEXT_U 0xc
#define NO_TEXT_W 0x24
#else
/* Each answer owns a separate 48-pixel crop, in every language and font. */
#define YES_TEXT_W 0x30
#define NO_TEXT_U 0x30
#define NO_TEXT_W 0x30
#endif

uint16_t YES_NO_PIECES[3] = {
	PIECE_WORDS, 0x30 | PIECE_LEFT, PIECES_END,
};

#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
#define VS_RETURN_MENU 0x47
#else
#define VS_RETURN_MENU 0
#endif

#if VERSION_IS(JP_TRIAL)
#define SECRET_CODE_ITEM 0x7d
#elif VERSION_IS(JP_BOMBOM)
#define SECRET_CODE_ITEM 0x7e
#endif

/* MemCardSync command and result type */
#if !VERSION_IS(US)
typedef long MemCardSyncWord;
#else
typedef unsigned long MemCardSyncWord;
#endif

typedef struct {
	int8_t pos;
	int8_t count;
	int8_t defaultPos;
	int8_t scroll;
	int8_t max;
} MenuCursor;

typedef struct {
	int8_t pos;
	int8_t pad[5];
	int16_t rowHeight;
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
} MenuHighlight;

typedef struct {
	int32_t valid;
	char playerName[14];
	uint8_t pad12[6];
	char digimonName[20]; /* MOD (long names): was 18 and 2 bytes of padding (19 characters, title and tail) */
	char location[24];
} SaveSlotPreview;

/* a WORLD_POOP pile in the v1 save: the screen's low byte, 0xFF = none */
typedef struct {
	uint8_t map;
	uint8_t x;
	uint8_t y;
	int8_t size;
} SavedPoopPile;

typedef struct {
	int32_t mainState;
	VECTOR tamerPosition;
	VECTOR partnerPosition;
	SavedPoopPile worldPoop[100]; /* MOD (screens beyond 255): the v1 bytes; WORLD_POOP is wider */
	int32_t money;
	int32_t partnerType;
	uint8_t partnerData[16];
	uint16_t currentFrame;
	uint16_t lastHandledFrame;
	int16_t year;
	int16_t day;
	int16_t hour;
	int16_t minute;
	uint16_t playtimeFrames;
	uint16_t playtimeHours;
	uint16_t playtimeMinutes;
	uint8_t pad1DE[2];
	PartnerPara partnerPara;
	int16_t merit;
	int16_t battlesStarted;
	int16_t battlesFled;
	int16_t tournamentsWon;
	int16_t tournamentWins;
	int16_t tournamentsLost;
	Stats stats;
	uint8_t scriptState[0x17C];
	SavedInventory inventory; /* MOD: u8 ids; INVENTORY is ItemId */
	char playerName[20];
	char digimonName[20];
	int8_t partnerLives;
	int8_t tamerLevel;
	uint8_t raisedCount;
	int8_t npcMaps[8];
	uint8_t currentScreen;
	uint8_t previousScreen;
	uint8_t currentExit;
	uint8_t previousExit;
	int8_t tamerWaypointX[30];
	int8_t tamerWaypointY[30];
	int8_t tamerPreviousTileX;
	int8_t tamerPreviousTileY;
	int8_t tamerWaypointCurrent;
	int8_t tamerWaypointCount;
	int8_t tamerStartTileX;
	int8_t tamerStartTileY;
	int8_t tamerWaypointActive;
	uint8_t unknown4E1[0x1B];
	int32_t checksum;
	RegisteredDigimon battleRegistrationData[40];
} SavegamePayload;

/*
 * MOD: the v1 save layout must not change while the ids widen in RAM
 * (docs/mods/ids.md, step 3). The ids are written as their low byte.
 */
#define SAVE_LAYOUT_CHECK(name, cond) typedef char save_layout_##name[(cond) ? 1 : -1]
SAVE_LAYOUT_CHECK(size, sizeof(SavegamePayload) == 0xF00);
SAVE_LAYOUT_CHECK(inventory, __builtin_offsetof(SavegamePayload, inventory) == 0x40C);
SAVE_LAYOUT_CHECK(inventorySize, sizeof(((SavegamePayload *)0)->inventory) == 0x5B);
SAVE_LAYOUT_CHECK(playerName, __builtin_offsetof(SavegamePayload, playerName) == 0x467);
SAVE_LAYOUT_CHECK(currentScreen, __builtin_offsetof(SavegamePayload, currentScreen) == 0x49A);
SAVE_LAYOUT_CHECK(checksum, __builtin_offsetof(SavegamePayload, checksum) == 0x4FC);
SAVE_LAYOUT_CHECK(versionWord, __builtin_offsetof(SavegamePayload, unknown4E1) + 3 == 0x4E4);

/*
 * MOD (save v2, docs/mods/ids.md "Save format v2"): the file grows from one
 * memory card block to two. The first 0x2000 bytes stay a complete v1 file
 * (header and the two SavegamePayload copies, ids as their low byte), so the
 * original executable and VS mode read it as before; after them come two
 * extensions, one per copy, with the 16-bit ids and the wide registers.
 *
 * - The v1 payload carries a version word in the bytes its checksum skips:
 *   0x4E4 "DW2\0", 0x4E8 the format version. v1 saves have zeros there.
 *   A higher version than SAVE_FORMAT_VERSION is a "Data Error".
 * - An extension is used only if its magic, its checksum and its copy of its
 *   v1 payload's checksum all match. An older build that saved over a v2 file
 *   rewrote the v1 part only, so the stale extension fails the last test and
 *   the v1 data is migrated, as for a v1 save.
 * - Saving a v1 (one-block) file makes it two blocks (delete and create) when
 *   the card has a free block; otherwise it stays a v1 file.
 * - A save holding content the original executable does not have (a partner
 *   species >= NUM_ORIGINAL_DIGIMON, an item >= NUM_ITEMS, a screen >= 255)
 *   stores its v1 checksum inverted: the original reports "Data Error"
 *   instead of loading cut ids; this build accepts it through the extension.
 *   Without room for the extension (a one-block file on a full card) it is
 *   written as a plain v1 file instead, readable by both.
 */
#define SAVE_LEGACY_VERSION	2	/* the extension of v2 files: a fixed struct (SaveExtension), still read */
#define SAVE_FORMAT_VERSION	4	/* the extension is a list of tagged records (include/dw/save_tags.h, docs/mods/save-format.md) */
#define SAVE_V1_BYTES		0x2000
#define SAVE_V2_BYTES		0x4000
#define SAVE_EXT_SAVED_WREGS	16	/* w0..w15 are saved, w16..w63 are scratch */

typedef struct {
	char magic[4];			/* "DW2E" */
	uint16_t version;		/* SAVE_FORMAT_VERSION */
	uint16_t size;			/* sizeof(SaveExtension) */
	int32_t v1Checksum;		/* the checksum of the v1 payload saved with it */
	ScreenId currentScreen;		/* authoritative; the v1 bytes are the low byte */
	ScreenId previousScreen;
	ItemId inventoryTypes[30];	/* authoritative; ITEM_NONE = empty */
	int32_t partnerType;		/* the same as the v1 field (already 32-bit) */
	uint16_t wregs[SAVE_EXT_SAVED_WREGS];
	DigimonId registeredTypes[40];	/* battle registration ids (u8 in RAM for now) */
	ScreenId poopMaps[100];		/* WORLD_POOP screens (the v2 file: the v1 bytes) */
	uint8_t speciesRaised[NUM_DIGIMON / 8];	/* MOD (scale): SPECIES_RAISED_BITS, one bit per type */
	uint8_t speciesRecruited[NUM_DIGIMON / 8]; /* SPECIES_RECRUITED_BITS */
	uint8_t reserved[0x1000 - 0x188 - 4 - 2 * (NUM_DIGIMON / 8)];
	int32_t checksum;		/* byte sum of everything before it */
} SaveExtension;

SAVE_LAYOUT_CHECK(extension, sizeof(SaveExtension) == 0x1000);
SAVE_LAYOUT_CHECK(extensionTail, __builtin_offsetof(SaveExtension, reserved) == 0x188 + 2 * (NUM_DIGIMON / 8));

typedef struct {
	char id[2];
	uint8_t iconDisplayFlag;
	uint8_t blockNumber;
	char title[64];
	uint8_t reserved[12];
	uint8_t pocketStation[16];
	uint16_t iconClut[16];
	uint8_t iconFrames[3][0x80];
	SavegamePayload saves[2];
} SaveFile;

SAVE_LAYOUT_CHECK(file, sizeof(SaveFile) == SAVE_V1_BYTES);
/*
 * MOD (save extension area): the two extensions (file offsets 0x2000 and 0x3000) are read and written
 * at SAVE_EXT_ADDR (memmap.h), a part of the world scratch that is free whenever the card menu runs;
 * SAVE_FILE is the resident v1 file, as the original's (saveFileStart reads and writes the two parts).
 */
#define SAVE_EXT ((SaveExtension *)SAVE_EXT_ADDR)
SAVE_LAYOUT_CHECK(extArea, 2 * sizeof(SaveExtension) <= SAVE_EXT_MAX);

/*
 * MOD (save v4): a v4 extension is SaveExtHead, the records, a record of tag 0, and at the end of the extension's bytes
 * the byte sum of everything before it. Its size is the file layout's (SAVE_EXT_BYTES: 0x1000 in a two-block file).
 */
typedef struct {
	char magic[4];			/* "DW4E" */
	uint16_t version;		/* SAVE_FORMAT_VERSION */
	uint16_t size;			/* the extension's bytes, the checksum included */
	int32_t v1Checksum;		/* the checksum of the v1 payload saved with it */
} SaveExtHead;

#define SAVE_EXT_BYTES		0x1000
SAVE_LAYOUT_CHECK(extHead, sizeof(SaveExtHead) == 12);
/* the most a build writes: the head, the records (4 bytes each and their data), the end record and the checksum */
#define SAVE_RECORDS_MAX	(sizeof(SaveExtHead) + 9 * 4 + 4 + 2 * 30 + 4 + 2 * 16 + 2 * 40 + 2 * 100 + 8 + 2 * (NUM_DIGIMON / 8) + 4 + 4 + 4)
SAVE_LAYOUT_CHECK(recordsFit, SAVE_RECORDS_MAX <= SAVE_EXT_BYTES);

uint32_t CONNECTED_CARDS;
int32_t MAIN_MENU_ACTION;
int32_t SAVE_SLOT_SCROLL_TENTHS;
int32_t MEMORY_CARD_SLOT;
int32_t MEMORY_CARD_ERROR;
int32_t MEMORY_CARD_ID;
int32_t MAIN_MENU_TICKS;
static int32_t MOD_BOOT_LANGUAGE;
static int32_t MOD_BOOT_LANGUAGE_CURSOR;
static int32_t MOD_LANGUAGE_ERROR;
static void __attribute__((noinline)) modShowLanguageError(void);
static void __attribute__((noinline)) modRenderLanguageError(void);
static void __attribute__((noinline)) modDrawLanguageName(int32_t lang, int32_t x, int32_t y);
static char *MOD_LANGUAGE_NAMES[MOD_LANGS];
static __attribute__((noinline)) void modDrawLanguagePicker(void);
static __attribute__((noinline)) void modTickLanguagePicker(void);
static __attribute__((noinline)) void modRenderLanguagePicker(void);
static int32_t MOD_OPTIONS_BACK; /* MOD (options): the title menu comes back from OPTIONS */
#define MOD_OPT_BYTES	14 /* byte 13 reserved; zero keeps permanent enemy bars for older readers */
#define MOD_OPT_HEADER_BYTES 12
#define MOD_OPT_HEADER_CHEATS 0x80
#define MOD_OPT_HEADER_COUNT 0x1f
#define MOD_OPT_CHEATS_BIT 1 /* byte 12: CHEATS, and the Cheats menu's Select shortcuts in the bit above it */
#define MOD_OPT_SHORTCUTS_BIT 2
static int32_t MOD_SAVE_NUMBER; /* MOD (options): the highest save number seen in a save header (modTitleOptions) */
#define MOD_OPT_ROW_STORY	3 /* MOD (story): EXTENDED STORY, under NEW DIGIMON */
#define MOD_OPT_ROW_DIFF	4 /* MOD (difficulty): DIFFICULTY, under EXTENDED STORY */
#define MOD_OPT_ROW_QUICK	5 /* MOD (quick battle start): QUICK BATTLE START, under DIFFICULTY */
#if FONT_SWITCH
#define MOD_OPT_ROWS	10 /* MOD (options): and FONT, in a build with both fonts */
#define MOD_OPT_ROW_FONT	6
#define MOD_OPT_ROW_LANG	7
#else
#define MOD_OPT_ROWS	9
#define MOD_OPT_ROW_LANG	6
#endif
#define MOD_OPT_ROW_SIGNS	(MOD_OPT_ROW_LANG + 1) /* MOD (language): LANGUAGE and MAP SIGNS, the last two rows */
static void renderOptionsMenu(void);
static void drawOptionsText(void);
#define MOD_OPT_ROW_CHEATS (MOD_OPT_ROWS - 1)
static int32_t modOptionValue(int32_t row);
static int32_t modOptionCount(int32_t row);
static void modSetOption(int32_t row, int32_t value);
int32_t CURRENT_MENU;
int32_t NEW_CARDS;
int32_t MAIN_D_80135054;
int32_t MEMORY_CARD_OPERATION;
int32_t VS_PLAYER_INDEX;
int32_t BATTLE_REGISTRATION_SLOT;
int32_t CHECKED_MEMORY_CARD;
int32_t TARGET_MENU;
int32_t MEMORY_CARD_RETURN_MENU;
int32_t MEMORY_CARD_USED_BLOCKS;
long SAVE_RETRY_RETURN_MENU;

static void *main_menu_sbss_order[] = {
	&SAVE_RETRY_RETURN_MENU,
	&MEMORY_CARD_USED_BLOCKS,
	&MEMORY_CARD_RETURN_MENU,
	&TARGET_MENU,
	&CHECKED_MEMORY_CARD,
	&BATTLE_REGISTRATION_SLOT,
	&VS_PLAYER_INDEX,
	&MEMORY_CARD_OPERATION,
	&MAIN_D_80135054,
	&NEW_CARDS,
	&CURRENT_MENU,
	&MAIN_MENU_TICKS,
	&MEMORY_CARD_ID,
	&MEMORY_CARD_ERROR,
	&MEMORY_CARD_SLOT,
	&SAVE_SLOT_SCROLL_TENTHS,
	&MAIN_MENU_ACTION,
	&CONNECTED_CARDS,
};

SaveSlotPreview SAVEGAME_SLOT_INFO[15];

extern int8_t MAIN_STATE;
extern uint8_t CURRENT_EXIT;
extern uint8_t PREVIOUS_EXIT;
extern int8_t TAMER_WAYPOINT_X[];
extern int8_t TAMER_WAYPOINT_Y[];
extern int8_t TAMER_PREVIOUS_TILE_X;
extern int8_t TAMER_PREVIOUS_TILE_Y;
extern int8_t TAMER_WAYPOINT_CURRENT;
extern int8_t TAMER_WAYPOINT_COUNT;
extern int8_t TAMER_START_TILE_X;
extern int8_t TAMER_START_TILE_Y;
extern int8_t TAMER_WAYPOINT_ACTIVE;
extern int32_t MONEY;
extern uint16_t CURRENT_FRAME;
extern uint16_t LAST_HANDLED_FRAME;
extern uint8_t YEAR;
extern int16_t DAY;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
extern int16_t SECRET_CODE;
#endif
extern int16_t HOUR;
extern int16_t MINUTE;
extern uint16_t PLAYTIME_FRAMES;
extern uint16_t PLAYTIME_HOURS;
extern uint16_t PLAYTIME_MINUTES;
extern int32_t CHANGED_INPUT;
extern GsOT *ACTIVE_ORDERING_TABLE;
extern char *MOVE_NAMES[];
#if !VERSION_IS(US)
extern char *MAP_NAME_PTR[];
#endif

int32_t getFileCityTopMap(void);
void renderUIBoxBorder(RECT *rect, int32_t flag);
void SetPolyG4(POLY_G4 *prim);
void recalculatePPandArena(void);
void renderMainMenuBackground(void);

void renderInitialMenu();
void renderText(POLY_FT4 *prim, int32_t x, int32_t y, int32_t u, int32_t v,
		int32_t w, int32_t h, int32_t textColor);
void renderMenuBox(int32_t x, int32_t y, int32_t w, int32_t h);
void renderSelectNewGameCard();
void renderSelectCardSlot();
void renderCheckingCard(void);
void renderConfirmNoCard();
void renderSaveSlotBox(int32_t slot, int32_t x, int32_t y);
void renderSelectSlot();
void renderConfirmSlotSelection();
void renderFormatMemoryCard(void);
void renderMemoryCardReadError();
void renderSleepMenu(void);
void renderConfirmOverwrite();
void renderConfirmVSSlot();
void renderRegisterDigimon();
void renderSelectRegisterSlot(void);
void renderConfirmRegister();
void renderCantRegister(void);
void renderDoYouWantToSave(void);
void renderCantRegisterBaby(void);
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
void renderBattleModeMenu(void);
void renderInsertGameCardPrompt(void);
#endif
void drawMainMenuStrings();
void drawSaveSlotText(int32_t slot, int32_t row);
char *formatInteger(int32_t value, char *buf, int32_t digits);
void drawMoveName(long type, int32_t anim, int32_t color, int32_t pos);
/* MOD (long names): screens that show a species name of up to 19 characters whole (font.c drawStringMode) */
static int32_t modRegisterLong(void);
static void modTitleNameTail(void);
static void modDrawSlotName(char *name, int32_t y);
static void modDrawSlot2(char *str, int32_t x, int32_t y, int32_t slot2);
static int32_t MOD_REG_LIST_LONG;
static int32_t MOD_SAVE_SLOTS_LONG;
/* MOD (sleep menu): its rows as wide as the longer option measures, never narrower than the original's 0x54 */
#define SLEEP_TEXT_W		0x54
static int32_t MOD_SLEEP_W = SLEEP_TEXT_W;
static void modSleepMenuWidth(void);
#define REG_NAME_X		0x64 /* the registration screen's value column (text area x) */
#define REG_TEXT_W		0xD6 /* its text as the original copied it, to the box's edge */
#define REG_LONG_TEXT_W		0xEC /* long layout: the box as wide as the title's (0xF6), its text to the edge */
#define REG_LONG_NAME_W		(REG_LONG_TEXT_W - REG_NAME_X - 12) /* the name ends 12 px inside the box edge */
#define LIST_NAME_X		0x6C /* the list of registered Digimon: the species column */
#define LIST_CURSOR_END		0xCC /* where the original's cursor (and its cleared text) ends */
#define LIST_LONG_NAME_W	(0xE6 - LIST_NAME_X - 6) /* long layout: the cursor 0xE6 wide, the name 6 px inside it */
#define SLOT_NAME_X		0x6C /* the save slots' species column (text area x) */
#define SLOT_NAME_W		0x60 /* its width as the original copied it */
#define SLOT_LONG_NAME_W	0x88 /* long layout: the boxes 0x18 further left and 0x110 wide (SLOT_BOX_W) */
#if VERSION_IS(US)
#define SLOT_DX			(MOD_SAVE_SLOTS_LONG ? 0x18 : 0)
#define SLOT_BOX_W		(MOD_SAVE_SLOTS_LONG ? 0x110 : 0xE0)
#else
#define SLOT_DX			0
#define SLOT_BOX_W		0xE0
#endif
static void modSaveSlotsLayout(int32_t isLong);
void drawRegisteredDigimonSlots(int32_t slot);
void updateMemoryCardState();
void tickMainMenu(void);
int32_t tickMenuInput(MenuCursor *cursor, int32_t which);
int32_t getMenuOnCardChange(int32_t menu);
void setMemoryCardReadError(int32_t id, int32_t returnMenu);
int32_t isMemcardUnformatted(int32_t chan, MemCardSyncWord mode);
int32_t getCardUsedBlockCount(int32_t channel, int32_t returnMenu);
int32_t loadSaveSlotData(int32_t channel, char *filename, SaveSlotPreview *slots, int32_t unused);
int32_t tickSelectSlotInput(MenuCursor *cursor, int32_t which);
void initializeDefaultSavegame(void);
char *_strncpy(char *dst, char *src, int32_t n);
int32_t createSavegameChecksum(int32_t slot);
void loadSavegame(SavegamePayload *savegame);
void writeSavegame(SavegamePayload *savegame);
/* MOD (save v2) */
static int32_t SAVE_FILE_BYTES = SAVE_V1_BYTES;
static int32_t getSaveFileBytes(void);
static int32_t loadSaveCopy(int32_t copy);
static int32_t isSaveCopyValid(int32_t copy);
static void prepareSaveFile(int32_t fromRam);
static void modTitleOptions(void); /* MOD (options) */
static void modCardOptions(void);
void modNoticeProbe(void);
static int32_t modHeaderSaveNumber(const uint8_t *header);
static int32_t writeSaveFile(void);
static long saveFileStart(int32_t write);
static void saveFileSync(MemCardSyncWord *command, MemCardSyncWord *result);
static int32_t pickSaveCopy(void);
static void dropAbsentRegistered(RegisteredDigimon *p, int32_t n);
#define SAVE_IO_ERROR		1	/* MEMORY_CARD_ERROR_MESSAGES[1], "Not Inserted": a card part that did not start */
#define SAVE_IO_MESSAGES	9	/* the libmcrd codes the messages cover (0-8) */
int32_t countRegisteredDigimon(RegisteredDigimon *p);
void renderMainMenu();
void registerBattleData();
void renderMenuSelector(MenuHighlight *b);
int32_t createByteSum(uint8_t *data, int32_t len);
#if !VERSION_IS(US)
void appendWideDigit(int32_t digit, char **dst, int32_t *started);
#endif
void openSaveMachine(void);
int32_t tickSaveMachine(void);
void awardMachinedramonData();
void gameClearSave();
int32_t isSoundLoaded(int32_t mode, int32_t vabId);
int32_t scriptTickMainMenu(void);

void *main_menu_order_anchor[] = {
	scriptTickMainMenu,
	gameClearSave,
	awardMachinedramonData,
	tickSaveMachine,
	openSaveMachine,
#if !VERSION_IS(US)
	appendWideDigit,
#endif
	createByteSum,
	renderMenuSelector,
	registerBattleData,
	renderMainMenu,
	countRegisteredDigimon,
	writeSavegame,
	loadSavegame,
	createSavegameChecksum,
	_strncpy,
	initializeDefaultSavegame,
	tickSelectSlotInput,
	loadSaveSlotData,
	getCardUsedBlockCount,
	isMemcardUnformatted,
	setMemoryCardReadError,
	getMenuOnCardChange,
	tickMenuInput,
	tickMainMenu,
	updateMemoryCardState,
	drawRegisteredDigimonSlots,
	drawMoveName,
	formatInteger,
	drawSaveSlotText,
	drawMainMenuStrings,
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	renderInsertGameCardPrompt,
	renderBattleModeMenu,
#endif
	renderCantRegisterBaby,
	renderDoYouWantToSave,
	renderCantRegister,
	renderConfirmRegister,
	renderSelectRegisterSlot,
	renderRegisterDigimon,
	renderConfirmVSSlot,
	renderConfirmOverwrite,
	renderSleepMenu,
	renderMemoryCardReadError,
	renderFormatMemoryCard,
	renderConfirmSlotSelection,
	renderSelectSlot,
	renderSaveSlotBox,
	renderConfirmNoCard,
	renderCheckingCard,
	renderSelectCardSlot,
	renderSelectNewGameCard,
	renderMenuBox,
	renderText,
	renderInitialMenu,
};

// clang-format off
SAVE_LABELS_TEXT

#if !VERSION_IS(US)
char MAIN_D_801346B8[4] = "）は";
char MAIN_D_801346C0[2][2] = { "", "*" };
#else
char MAIN_D_801346B8[] = " is in";
char MAIN_D_801346C0[2][2] = { ".", "*" };
#endif

SAVE_PLAYER_LABEL_TEXT

#if VERSION_IS(US)
char FMT_NUMBER[] = "%i";
#endif

struct DIRENTRY *MEMCARD_DIRENTRIES = (struct DIRENTRY *)(TEXTURE_BUFFER + 0x5c00);

RegisteredDigimon *VS__REGISTERED_DIGIMON_BUFFER = (RegisteredDigimon *)(TEXTURE_BUFFER + 0x4800);

MEMCARD_MESSAGES_TEXT
SAVE_MESSAGES_TEXT

InventoryTable DEFAULT_INVENTORY_AMOUNTS = {
	{
		0x28, 0x2d, 0x32, 0x1e, 0x28, 0x1e, 0x1e, 0x1e,
		0x1e, 0x1e, 0x1e, 0x32, 0x1e, 0x28, 0x1e, 0x32,
		0x32, 0x50, 0x32, 0x32, 0x62, 0x32, 0x32, 0x32,
		0x32, 0x32, 0x32, 0x32, 0x32, 0x00,
	},
};

InventoryTable DEFAULT_INVENTORY_TYPES = {
	{
		0x02, 0x09, 0x0c, 0x13, 0x14, 0x15, 0x43, 0x46,
		0x0e, 0x54, 0x5a, 0x5e, 0x5f, 0x67, 0x68, 0x7d,
		0x7e, 0x7f, 0x61, 0x64, 0x6a, 0x6c, 0x6e, 0x6f,
		0x70, 0x71, 0x77, 0x20, 0x16, 0xff,
	},
};

LABELS_TEXT

CVECTOR MAIN_D_80131638[4] = {
	{ 0x80, 0x80, 0x80, 0x00 },
	{ 0x40, 0x40, 0x40, 0x00 },
	{ 0x80, 0x80, 0x00, 0x00 },
	{ 0xc0, 0x40, 0x40, 0x00 },
};

int8_t HEX_DIGITS[16] = "0123456789ABCDEF";

#if !VERSION_IS(US)
char SAVEGAME_ID_LABEL[16][6] = {
	"　０",
	"　１",
	"　２",
	"　３",
	"　４",
	"　５",
	"　６",
	"　７",
	"　８",
	"　９",
	"１０",
	"１１",
	"１２",
	"１３",
	"１４",
	"１５",
};

#if !VERSION_IS(JP)
MenuHighlight MENU_HIGHLIGHTS[22] = {
#else
MenuHighlight MENU_HIGHLIGHTS[19] = {
#endif
	{
		0x00,
		{ 0x04, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x005e,
		0x0037,
		0x0084,
		0x000c,
	},
	{
		0x00,
		{ 0x03, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0058,
		0x0037,
		0x0090,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0058,
		0x0037,
		0x0090,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0058,
		0x0037,
		0x0090,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0058,
		0x0037,
		0x0090,
		0x000c,
	},
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x006e,
		0x0037,
		0x0024,
		0x000c,
	},
	{
		0x00,
		{ 0x05, 0x00, 0x00, 0x00, 0x00 },
		0x0024,
		0x0036,
		0x002e,
		0x00d4,
		0x001b,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x003a,
		0x0052,
		0x0024,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x01, 0x00, 0x00, 0x00 },
		0x000c,
		0x004c,
		0x0065,
		0x0024,
		0x000c,
	},
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x0010,
		0x0076,
		0x006a,
		0x0054,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x01, 0x00, 0x00, 0x00 },
		0x000c,
		0x0040,
		0x005a,
		0x0024,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0046,
		0x0065,
		0x0024,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0046,
		0x00c3,
		0x0024,
		0x000c,
	},
	{
		0x00,
		{ 0x0a, 0x00, 0x00, 0x1d, 0x00 },
		0x000c,
		0x0046,
		0x0037,
		0x00cc,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0046,
		0x0059,
		0x0024,
		0x000c,
	},
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0046,
		0x0057,
		0x0024,
		0x000c,
	},
#if !VERSION_IS(JP)
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x03, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x005e,
		0x0037,
		0x0084,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0046,
		0x0065,
		0x0024,
		0x000c,
	},
#endif
};

#else
char SAVEGAME_ID_LABEL[16][6] = {
	" 0",
	" 1",
	" 2",
	" 3",
	" 4",
	" 5",
	" 6",
	" 7 ",
	" 8 ",
	" 9",
	"10",
	"11",
	"12",
	"13",
	"14",
	"15",
};

MenuHighlight MENU_HIGHLIGHTS[23] = { /* MOD: every frame of rows 12 apart 12 high (h 0x0a, was 0x0c), in the gaps: renderMenuSelector draws h + 2 from 2 above the row, so 0x0c put its bottom line on the next row's letters */ /* MOD (options): view 22 */
	{
		0x00,
		{ 0x05, 0x00, 0x00, 0x00, 0x00 }, /* MOD (options): 5 rows, OPTIONS the last (was 4) */
		0x000c,
		0x004f,
		0x0037,
		0x0097,
		0x000a,
	},
	{
		0x00,
		{ 0x03, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0048,
		0x0037,
		0x009c,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0043,
		0x0037,
		0x00a5,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0058,
		0x0037,
		0x0090,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0043,
		0x0037,
		0x0080,
		0x000a,
	},
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x006b,
		0x0037,
		0x0021,
		0x000a,
	},
	{
		0x00,
		{ 0x05, 0x00, 0x00, 0x00, 0x00 },
		0x0024,
		0x0036,
		0x002e,
		0x00d4,
		0x001b,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0037,
		0x0052,
		0x0021,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x01, 0x00, 0x00, 0x00 },
		0x000c,
		0x0049,
		0x0065,
		0x0021,
		0x000a,
	},
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x0010,
		0x0076,
		0x006a,
		0x0054,
		0x000c,
	},
	{
		0x00,
		{ 0x02, 0x01, 0x00, 0x00, 0x00 },
		0x000c,
		0x003d,
		0x005a,
		0x0021,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0043,
		0x0065,
		0x0021,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0043,
		0x00c3,
		0x0021,
		0x000a,
	},
	{
		0x00,
		{ 0x0a, 0x00, 0x00, 0x1d, 0x00 },
		0x000c,
		0x0046,
		0x0037,
		0x00cc,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0043,
		0x0059,
		0x0021,
		0x000a,
	},
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0043,
		0x0057,
		0x0021,
		0x000a,
	},
	{
		0xff,
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
	},
	{
		0x00,
		{ 0x03, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x005e,
		0x0037,
		0x0084,
		0x000a,
	},
	{
		0x00,
		{ 0x02, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0046,
		0x0065,
		0x0024,
		0x000a,
	},
	{ /* MOD (options): the OPTIONS box (renderOptionsMenu), its rows; x and w follow the font (drawOptionsText) */
		0x00,
		{ MOD_OPT_ROWS, 0x00, 0x00, 0x00, 0x00 },
		0x000c,
		0x0035,
		0x0037,
		0x00d6,
		0x000a, /* the frame 12 high, in the gaps of rows 12 apart (0x0c put its bottom on the next row's letters) */
	},
};

#endif

int8_t MENU_VIEWS[] = {
	0x00, 0x09, 0x05, 0x0a, 0x16 /* MOD (options) */, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x01, 0x05, 0x05, 0x05, 0x07, 0x08,
	0x05, 0x05, 0xff, 0x06, 0x02, 0x05, 0x05, 0x07,
	0x08, 0x05, 0xff, 0xff, 0xff, 0xff, 0x04, 0x05,
	0x05, 0x07, 0x08, 0x05, 0xff, 0xff, 0xff, 0xff,
	0x0b, 0x05, 0x05, 0x05, 0xff, 0xff, 0xff, 0xff,
	0x0c, 0x05, 0x0d, 0x05, 0x07, 0x08, 0x05, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x0e, 0x0f, 0x10, 0x05,
	0x05, 0x05, 0x0c, 0x05, 0x11, 0x12, 0x13,
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	0x14,
	0xff, 0x15, 0x05, 0x05, 0x07, 0x08, 0x05, 0x05,
#endif
};

char *TITLE_MENU_ITEMS[4] = {
	MAIN_D_80131008,
	MAIN_D_80131014,
	MAIN_D_80131024,
	MAIN_D_80131030,
};

char *SLOT_ACTION_TITLES[] = {
	MAIN_D_8013103C,
	MAIN_D_8013104C,
	MAIN_D_8013105C,
	MAIN_D_8013106C,
	MAIN_D_8013107C,
	MAIN_D_80131030,
	MAIN_D_80131090,
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	MAIN_D_801310A4,
#endif
};

char *MEMORY_CARD_OPERATION_MESSAGES[6] = {
	MAIN_D_801310B4,
	MAIN_D_801310CC,
	MAIN_D_801310E0,
	MAIN_D_801310F4,
	MAIN_D_80131108,
	MAIN_D_8013111C,
};

char *MEMORY_CARD_ERROR_MESSAGES[] = {
	STR_EMPTY,
	MAIN_D_80131134,
	MAIN_D_80131144,
	MAIN_D_80131150,
	MAIN_D_80131168,
	MAIN_D_80131178,
	STR_EMPTY,
	MAIN_D_8013118C,
	MAIN_D_8013119C,
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	MAIN_D_801311B4,
	MAIN_D_801311C0,
#endif
};

char *SLOT_ACTION_QUESTIONS[] = {
	MAIN_D_801311D4,
	MAIN_D_801311EC,
	MAIN_D_80131200,
	STR_EMPTY,
	STR_EMPTY,
	MAIN_D_80131218,
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	STR_EMPTY,
	MAIN_D_80131228,
#endif
};

#if !VERSION_IS(JP) && !VERSION_IS(US)
char *BATTLE_MODE_ITEMS[4] = {
	MAIN_D_80131234,
	MAIN_D_80131030,
	MAIN_D_80131254,
	MAIN_D_801310A4,
};
#elif VERSION_IS(US)
char *BATTLE_MODE_ITEMS[4] = {
	MAIN_D_80131234,
	MAIN_D_80131240,
	MAIN_D_80131254,
	MAIN_D_80131264,
};
#endif

#if !VERSION_IS(US)
char SAVE_FILE_NAME[32] = "BISLPS-01797DMR*";

SaveFile SAVE_FILE;

char MAIN_D_8013392C[68] = "デジモン　　　　　　　　　　　　　　　　　　　　　　　　　　　　";
#else
char SAVE_FILE_NAME[32] = "BASLUS-01032DMR*";

SaveFile SAVE_FILE = { 0 };

char MAIN_D_8013392C[68] = "Digimon";
#endif

uint16_t SAVE_ICON_CLUT[16] = {
	0x0000, 0x575f, 0x3a7a, 0x1575, 0x00cd, 0x0013, 0x8009, 0x96f7,
	0x8df0, 0xc631, 0x7ff7, 0x5294, 0x39ce, 0x2d6b, 0x1ce7, 0x8842,
};

uint8_t SAVE_ICON_FRAME_0[128] = {
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x0f, 0x00,
	0x00, 0x00, 0xff, 0xdc, 0xdd, 0xdd, 0xfd, 0x00,
	0x00, 0xf0, 0xdc, 0xed, 0xdd, 0xde, 0xee, 0x0f,
	0x00, 0xcf, 0xec, 0xde, 0xed, 0xdd, 0xde, 0x0f,
	0xf0, 0xff, 0xbe, 0xbb, 0xfb, 0xfe, 0xff, 0xfe,
	0xdf, 0xcf, 0xbd, 0xbd, 0xeb, 0xec, 0xfe, 0xff,
	0xcf, 0xce, 0xbd, 0xbb, 0xeb, 0xec, 0xec, 0xfe,
	0xdf, 0xee, 0xee, 0xff, 0xef, 0xee, 0xed, 0xfd,
	0xff, 0xff, 0x3f, 0xff, 0xf3, 0xff, 0xfe, 0xfe,
	0xff, 0x2f, 0x3f, 0x34, 0xf2, 0x3f, 0xff, 0xff,
	0xf0, 0x43, 0xf1, 0x1f, 0xa4, 0xff, 0xf3, 0x4f,
	0x00, 0x3f, 0x44, 0x13, 0x23, 0x4f, 0xf4, 0x42,
	0x00, 0x34, 0x32, 0x11, 0x21, 0x21, 0x43, 0x43,
	0x00, 0x24, 0x31, 0x32, 0x34, 0x32, 0x44, 0x04,
	0x00, 0x40, 0x24, 0x12, 0x21, 0x43, 0x04, 0x00,
	0x00, 0x00, 0x40, 0x44, 0x44, 0x04, 0x00, 0x00,
};

uint8_t SAVE_ICON_FRAME_1[128] = {
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x0f, 0x00,
	0x00, 0x00, 0xff, 0xdc, 0xdd, 0xdd, 0xff, 0x00,
	0x00, 0xf0, 0xdc, 0xdd, 0xdd, 0xde, 0xfe, 0x0f,
	0x00, 0xcf, 0xed, 0xde, 0xed, 0xde, 0xee, 0x0f,
	0xf0, 0xff, 0xbc, 0xbb, 0xfc, 0xfd, 0xff, 0xfe,
	0xff, 0xde, 0xc9, 0xcd, 0xec, 0xec, 0xfd, 0xff,
	0xdf, 0xde, 0xb9, 0xbb, 0xec, 0xec, 0xed, 0xfd,
	0xdf, 0xee, 0xff, 0xff, 0xef, 0xee, 0xed, 0xfd,
	0xef, 0xff, 0x4f, 0xff, 0xf3, 0xf4, 0xfe, 0xfe,
	0xf0, 0x3f, 0x3f, 0x34, 0x42, 0x41, 0xf2, 0xff,
	0xf0, 0x44, 0xfa, 0x13, 0x21, 0x11, 0xff, 0x4f,
	0x00, 0x3f, 0x43, 0x12, 0x32, 0xf4, 0xf2, 0x42,
	0x00, 0x34, 0x32, 0x11, 0x21, 0x21, 0xf3, 0x43,
	0x00, 0x24, 0x31, 0x55, 0x45, 0x32, 0x44, 0x04,
	0x00, 0x40, 0x24, 0x52, 0x25, 0x43, 0x04, 0x00,
	0x00, 0x00, 0x40, 0x44, 0x44, 0x04, 0x00, 0x00,
};

uint8_t SAVE_ICON_FRAME_2[128] = {
	0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00,
	0x00, 0xf0, 0xcf, 0xdd, 0xdd, 0xdd, 0x0f, 0x00,
	0x00, 0xcf, 0xdc, 0xed, 0xed, 0xde, 0xfe, 0x00,
	0x00, 0xdf, 0xbd, 0xba, 0xfc, 0xfe, 0xff, 0x0f,
	0xf0, 0xce, 0xdb, 0xbb, 0xec, 0xec, 0xfe, 0xff,
	0xf0, 0xcc, 0xbb, 0xcb, 0xec, 0xec, 0xed, 0xfe,
	0xef, 0xce, 0xfe, 0xff, 0xef, 0xed, 0xed, 0xfe,
	0xdf, 0xfe, 0xff, 0x4f, 0xf3, 0xff, 0xee, 0xfd,
	0xef, 0xff, 0x3f, 0x3f, 0xf2, 0xf1, 0xf3, 0xfe,
	0xff, 0x3f, 0x3f, 0x12, 0x21, 0x21, 0xff, 0xff,
	0xf0, 0x34, 0xfa, 0x12, 0x32, 0xf4, 0xf2, 0x42,
	0x00, 0x4f, 0x43, 0x11, 0x21, 0x21, 0xf3, 0x43,
	0x00, 0x34, 0x32, 0x55, 0x45, 0x22, 0x43, 0x04,
	0x00, 0x34, 0x21, 0x52, 0x55, 0x32, 0x44, 0x00,
	0x00, 0x40, 0x44, 0x52, 0x35, 0x43, 0x04, 0x00,
	0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x00, 0x00,
};

#if VERSION_IS(US)
uint8_t SAVE_TITLE_RESERVED[28] = { 0 };
#endif
// clang-format on

void renderText(POLY_FT4 *prim, int32_t x, int32_t y, int32_t u, int32_t v,
		int32_t w, int32_t h, int32_t textColor)
{
    if (MOD_LANG == MOD_LANG_ZH && h == 11) h = 12;
	x -= 0xA0;
	y -= 0x78;
	SetPolyFT4(prim);
	prim->tpage = getTPage(0, 0, 704, 256);
	prim->clut = GetClut(0xD0, 0x1E8);
	setRGB0(prim, MAIN_D_80131638[textColor].r, MAIN_D_80131638[textColor].g, MAIN_D_80131638[textColor].b);
#if !VERSION_IS(US)
	setUVWH(prim, u, v, w, h);
	setXYWH(prim, x, y, w, h);
#else
	setXYWH(prim, x, y, w, h);
	setUVWH(prim, u, v, w, h);
#endif
	AddPrim(ACTIVE_ORDERING_TABLE->org, prim);
}

void renderMenuBox(int32_t x, int32_t y, int32_t w, int32_t h)
{
	RECT rect;
	POLY_G4 *prim;
	x -= 0xA0;
	y -= 0x78;
	setRECT(&rect, x, y, w, h);
	renderUIBoxBorder(&rect, 0);
	prim = (POLY_G4 *)GsGetWorkBase();
	SetPolyG4(prim);
	setRGB0(prim, 0, 0, 0x70);
	setRGB1(prim, 0, 0, 0x50);
	setRGB2(prim, 0, 0, 0x30);
	setRGB3(prim, 0, 0, 0x10);
	setXYWH(prim, x + 3, y + 3, (w - 6), (h - 6));
	AddPrim(ACTIVE_ORDERING_TABLE->org, prim++);
	GsSetWorkBase((PACKET *)prim);
}

void renderSelectNewGameCard(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	if (CONNECTED_CARDS & 1) {
		renderText(cur++, 0x58, 0x37, 0, 0, 0x90, 0xc, 0);
	} else {
		renderText(cur++, 0x58, 0x37, 0, 0, 0x90, 0xc, 1);
	}
	if (CONNECTED_CARDS & 0x10) {
		renderText(cur++, 0x58, 0x43, 0, 0xc, 0x90, 0xc, 0);
	} else {
		renderText(cur++, 0x58, 0x43, 0, 0xc, 0x90, 0xc, 1);
	}
	renderText(cur++, 0x58, 0x4f, 0, 0x18, 0x90, 0xc, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x4e, 0x32, 0xa4, 0x2e);
#else
	if (CONNECTED_CARDS & 1) {
		renderText(cur++, 0x4B, 0x37, 0, 0, 0xBE, 0xC, 0);
	} else {
		renderText(cur++, 0x4B, 0x37, 0, 0, 0xBE, 0xC, 1);
	}
	if (CONNECTED_CARDS & 0x10) {
		renderText(cur++, 0x4B, 0x43, 0, 0xC, 0xBE, 0xC, 0);
	} else {
		renderText(cur++, 0x4B, 0x43, 0, 0xC, 0xBE, 0xC, 1);
	}
	renderText(cur++, 0x4B, 0x4F, 0, 0x18, 0xBE, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x41, 0x32, 0xBE, 0x2E);
#endif
}

void renderSelectCardSlot(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	if (CONNECTED_CARDS & 1) {
		renderText(cur++, 0x58, 0x37, 0, 0, 0x90, 0xc, 0);
	} else {
		renderText(cur++, 0x58, 0x37, 0, 0, 0x90, 0xc, 1);
	}
	if (CONNECTED_CARDS & 0x10) {
		renderText(cur++, 0x58, 0x43, 0, 0xc, 0x90, 0xc, 0);
	} else {
		renderText(cur++, 0x58, 0x43, 0, 0xc, 0x90, 0xc, 1);
	}
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x4e, 0x32, 0xa4, 0x22);
#else
	if (CONNECTED_CARDS & 1) {
		renderText(cur++, 0x46, 0x37, 0, 0, 0xBE, 0xC, 0);
	} else {
		renderText(cur++, 0x46, 0x37, 0, 0, 0xBE, 0xC, 1);
	}
	if (CONNECTED_CARDS & 0x10) {
		renderText(cur++, 0x46, 0x43, 0, 0xC, 0xBE, 0xC, 0);
	} else {
		renderText(cur++, 0x46, 0x43, 0, 0xC, 0xBE, 0xC, 1);
	}
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3C, 0x32, 0xB7, 0x22);
#endif
}

void renderCheckingCard(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x40, 0x37, 0, 0, 0xC0, 0x24, 2);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x36, 0x32, 0xD4, 0x2E);
}

void renderConfirmNoCard(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x3a, 0x18, 0, 0, 0x90, 0xc, 0);
#else
	renderText(cur++, 0x3A, 0x18, 0, 0, 0xBE, 0xC, 0);
#endif
	GsSetWorkBase((PACKET *)cur);
#if !VERSION_IS(US)
	renderMenuBox(0x30, 0x13, 0xb0, 0x16);
#else
	renderMenuBox(0x30, 0x13, 0xBE, 0x16);
#endif
	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x22, 0x5A, 0, 0xC, 0xFC, 0x3C, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x18, 0x55, 0x110, 0x46);
	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x6E, 0x37, 0, 0xF0, YES_TEXT_W, 0xC, 0);
	renderText(cur++, 0x6E, 0x43, NO_TEXT_U, 0xF0, NO_TEXT_W, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x64, 0x32, 0x38, 0x22);
}

void renderSaveSlotBox(int32_t slot, int32_t x, int32_t y)
{
	POLY_FT4 *cur;
	int32_t hl;
	int32_t v;
	v = ((slot % 7) * 0x18) + 0xC;
	cur = (POLY_FT4 *)GsGetWorkBase();
	if (((SAVEGAME_SLOT_INFO[slot].valid != 0) && (MAIN_MENU_ACTION != 0)) ||
	    ((SAVEGAME_SLOT_INFO[slot].valid == 0) && (MAIN_MENU_ACTION == 0))) {
		hl = 0;
	} else {
		hl = 1;
	}
	renderText(cur++, x + 0xA, y + 5, 0, v, 0x18, 0xC, hl);
	if (SAVEGAME_SLOT_INFO[slot].valid != 0) {
		renderText(cur++, x + 0x28, y + 5, 0x18, v, 0x48, 0xC, hl);
#if VERSION_IS(US)
		/* MOD (long names): the long layout's species column (drawSaveSlotText) */
		renderText(cur++, x + 0x76, y + 5, SLOT_NAME_X, v, MOD_SAVE_SLOTS_LONG ? SLOT_LONG_NAME_W : SLOT_NAME_W, 0xC,
			   hl);
#else
		renderText(cur++, x + 0x76, y + 5, 0x6C, v, 0x60, 0xC, hl);
#endif
#if !VERSION_IS(US)
		renderText(cur++, x + 0x28, y + 0x13, 0x18, v + 0xc, 0x6c, 0xc, hl);
	} else {
		renderText(cur++, x + 0x28, y + 5, 0x90, 0, 0x6c, 0xc, hl);
#else
		renderText(cur++, x + 0x28, y + 0x13, 0x18, v + 0xC, 0xE0, 0xC, hl);
#endif
	}
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(x, y, SLOT_BOX_W, 0x24); /* MOD (long names) */
}

void renderSelectSlot(void)
{
	POLY_FT4 *ft4;
	int32_t page;
	int32_t y;
	int32_t i;

	ft4 = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(ft4++, 0x3a, 0x18, 0, 0, 0x90, 0xc, 0);
	GsSetWorkBase((PACKET *)ft4);
	renderMenuBox(0x30, 0x13, 0xb0, 0x16);
#else
	/* MOD (long names): with a species name longer than the slot's column every box is 0xFC wide, 0x10 further
	 * left (loadSaveSlotData) */
	renderText(ft4++, 0x3A - SLOT_DX, 0x18, 0, 0, 0xD6, 0xC, 0);
	GsSetWorkBase((PACKET *)ft4);
	renderMenuBox(0x30 - SLOT_DX, 0x13, SLOT_BOX_W, 0x16);
#endif

	if (SAVE_SLOT_SCROLL_TENTHS < (MENU_HIGHLIGHTS[7].pad[2] * 10)) {
		SAVE_SLOT_SCROLL_TENTHS += 2;
	}

	if (SAVE_SLOT_SCROLL_TENTHS > (MENU_HIGHLIGHTS[7].pad[2] * 10)) {
		SAVE_SLOT_SCROLL_TENTHS -= 2;
	}

	page = SAVE_SLOT_SCROLL_TENTHS / 10;

	for (i = 1; i < 6; ++i) {
		y = (i * 0x24) + ((page * 10 - SAVE_SLOT_SCROLL_TENTHS) * 36 / 10 + 0x29);
		if (y < 0x29) {
			y = 0x29;
		}

		if (!(y < 0xBA)) {
			y = 0xB9;
		}

		renderSaveSlotBox(page + i, 0x30 - SLOT_DX, y); /* MOD (long names) */
	}

	y = (page * 10 - SAVE_SLOT_SCROLL_TENTHS) * 36 / 10 + 0x29;
	if (y < 0x29) {
		y = 0x29;
	}

	if (!(y < 0xBA)) {
		y = 0xB9;
	}

	renderSaveSlotBox(page, 0x30 - SLOT_DX, y); /* MOD (long names) */
}

void renderConfirmSlotSelection(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
	/* MOD (long names): the long layout of the slot list (SLOT_DX, loadSaveSlotData) */
	renderText(cur++, 0x3A - SLOT_DX, 0x18, 0, 0, 0x90, 0xC, 0);
	renderText(cur++, 0x3A - SLOT_DX, 0x2E, 0, 0xC, 0x18, 0xC, 0);
	if (SAVEGAME_SLOT_INFO[MEMORY_CARD_SLOT].valid != 0) {
		renderText(cur++, 0x58 - SLOT_DX, 0x2E, 0x18, 0xC, 0x48, 0xC, 0);
		renderText(cur++, 0xA6 - SLOT_DX, 0x2E, 0x6C, 0xC, MOD_SAVE_SLOTS_LONG ? SLOT_LONG_NAME_W : 0x60, 0xC, 0);
#if !VERSION_IS(US)
		renderText(cur++, 0x58, 0x3c, 0x18, 0x18, 0x6c, 0xc, 0);
#else
		renderText(cur++, 0x58 - SLOT_DX, 0x3C, 0, 0x18, 0xE0, 0xC, 0);
#endif
	} else {
		renderText(cur++, 0x58 - SLOT_DX, 0x2E, 0x90, 0, 0x6C, 0xC, 0);
	}
	renderText(cur++, 0x3A - SLOT_DX, 0x52, 0, 0xF0, YES_TEXT_W, 0xC, 0);
	renderText(cur++, 0x3A - SLOT_DX, 0x5E, NO_TEXT_U, 0xF0, NO_TEXT_W, 0xC, 0);
#if !VERSION_IS(US)
	renderText(cur++, 0x3a, 0x74, 0, 0x30, 0xcc, 0xc, 0);
#else
	renderText(cur++, 0x3A - SLOT_DX, 0x74, 0x14, 0x30, 0xCC, 0xC, 0);
#endif
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x30 - SLOT_DX, 0x13, 0xB0, 0x16);
	renderMenuBox(0x30 - SLOT_DX, 0x29, SLOT_BOX_W, 0x24);
	renderMenuBox(0x30 - SLOT_DX, 0x4D, 0x38, 0x22);
	renderMenuBox(0x30 - SLOT_DX, 0x6F, SLOT_BOX_W, 0x16);
}

void renderFormatMemoryCard(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x4C, 0x37, 0, 0, 0xA8, 0x24, 2);
	renderText(cur++, 0x4C, 0x65, 0, 0xF0, YES_TEXT_W, 0xC, 0);
	renderText(cur++, 0x4C, 0x71, NO_TEXT_U, 0xF0, NO_TEXT_W, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x42, 0x32, 0xBC, 0x2E);
	renderMenuBox(0x42, 0x60, 0x38, 0x22);
}

void renderMemoryCardReadError(void)
{
	POLY_FT4 *cur;

	if ((MEMORY_CARD_ERROR == 1) || (MEMORY_CARD_ERROR == 3) || (MEMORY_CARD_ERROR == 4) ||
	    (MEMORY_CARD_ERROR == 5) || (MEMORY_CARD_ERROR == 7)) {
		cur = (POLY_FT4 *)GsGetWorkBase();
		renderText(cur++, 0x4C, 0x37, 0, 0, 0xA8, 0x18, 3);
		GsSetWorkBase((PACKET *)cur);
		renderMenuBox(0x42, 0x32, 0xBC, 0x22);
		return;
	}
	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x40, 0x37, 0, 0xC, 0xC0, 0xC, 3);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x36, 0x32, 0xD4, 0x16);
}

void renderSleepMenu(void)
{
	POLY_FT4 *cur;

	int32_t x;

	/* centred on 0xA0 as the original (box 0x6C + 0x68 / 2), text 10 px inside it */
	x = 0xA0 - MOD_SLEEP_W / 2;
	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, x, 0x6A, 0, 0, MOD_SLEEP_W, 0xC, 0);
	renderText(cur++, x, 0x7A, 0, 0xC, MOD_SLEEP_W, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(x - 10, 0x65, MOD_SLEEP_W + 20, 0x26);
}

static void modSleepMenuWidth(void)
{
	int32_t w;
	int32_t w2;

	w = measureStringMode(MAIN_D_8013136C, FIT_VANILLA);
	w2 = measureStringMode(MAIN_D_80131378, FIT_VANILLA);
	if (w2 > w) {
		w = w2;
	}
	w = (w + 2) & ~1; /* 1 px of air after the last glyph, even so the box stays centred */
	MOD_SLEEP_W = w > SLEEP_TEXT_W ? w : SLEEP_TEXT_W;
	MENU_HIGHLIGHTS[11].x = 0xA0 - MOD_SLEEP_W / 2;
	MENU_HIGHLIGHTS[11].w = MOD_SLEEP_W;
}

void renderConfirmOverwrite(void)
{
	POLY_FT4 *cur;
	int32_t mask;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x40, 0x38, 0, 0, 0xc0, 0x18, 0);
#else
	renderText(cur++, 0x40, 0x38, 0, 0, 0xD4, 0x18, 0);
#endif
	if (MEMORY_CARD_ID == 0) {
		mask = 1;
	} else {
		mask = 0x10;
	}
	if (!(CONNECTED_CARDS & mask)) {
		MEMORY_CARD_ERROR = 1;
		renderText(cur++, 0x40, 0x5A, 0, 0xF0, YES_TEXT_W, 0xC, 1);
	} else {
		if (MEMORY_CARD_ERROR == 1) {
			MEMORY_CARD_ERROR = -1;
		}
		renderText(cur++, 0x40, 0x5A, 0, 0xF0, YES_TEXT_W, 0xC, 0);
	}
	renderText(cur++, 0x40, 0x66, NO_TEXT_U, 0xF0, NO_TEXT_W, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
#if !VERSION_IS(US)
	renderMenuBox(0x36, 0x33, 0xd4, 0x22);
#else
	renderMenuBox(0x36, 0x33, 0xE8, 0x22);
#endif
	renderMenuBox(0x36, 0x55, 0x38, 0x22);
	if (MEMORY_CARD_ERROR != -1) {
		cur = (POLY_FT4 *)GsGetWorkBase();
		renderText(cur++, 0x4C, 0x82, 0, 0x24, 0xA8, 0x18, 2);
		GsSetWorkBase((PACKET *)cur);
		renderMenuBox(0x42, 0x7D, 0xBC, 0x22);
	}
}

void renderConfirmVSSlot(void)
{
	POLY_FT4 *cur;
	int32_t hl;
	int32_t mask;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x46, 0x21, 0, 0, 0x6c, 0xc, 0);
	renderText(cur++, 0x46, 0x37, 0, 0xc, 0xb4, 0x24, 0);
#else
	renderText(cur++, 0x46, 0x21, 0, 0, 0xD6, 0xC, 0);
	renderText(cur++, 0x46, 0x37, 0, 0xC, 0xDC, 0x24, 0);
#endif
	if (MEMORY_CARD_ID == 0) {
		mask = 1;
	} else {
		mask = 0x10;
	}
	if (CONNECTED_CARDS & mask) {
		hl = 0;
	} else {
		hl = 1;
	}
#if !VERSION_IS(US)
	renderText(cur++, 0x46, 0x65, 0, 0xf0, 0x18, 0xc, hl);
	renderText(cur++, 0x46, 0x71, 0x18, 0xf0, 0x24, 0xc, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3c, 0x1c, 0x80, 0x16);
	renderMenuBox(0x3c, 0x32, 0xc8, 0x2e);
#else
	renderText(cur++, 0x46, 0x65, 0, 0xF0, YES_TEXT_W, 0xC, hl);
	renderText(cur++, 0x46, 0x71, NO_TEXT_U, 0xF0, NO_TEXT_W, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3C, 0x1C, 0xE0, 0x16);
	renderMenuBox(0x3C, 0x32, 0xE6, 0x2E);
#endif
	renderMenuBox(0x3C, 0x60, 0x38, 0x22);
}

void renderRegisterDigimon(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x46, 0x21, 0, 0, 0x6c, 0xc, 0);
	renderText(cur++, 0x46, 0x37, 0, 0xc, 0xa2, 0x6c, 0);
	renderText(cur++, 0x46, 0xad, 0, 0x78, 0xa8, 0xc, 0);
	renderText(cur++, 0x46, 0xc3, 0xa8, 0x78, 0x18, 0xc, 0);
	renderText(cur++, 0x46, 0xcf, 0xb4, 0x78, 0x24, 0xc, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3c, 0x1c, 0x80, 0x16);
	renderMenuBox(0x3c, 0x32, 0xb6, 0x76);
	renderMenuBox(0x3c, 0xa8, 0xbc, 0x16);
#else
	renderText(cur++, 0x32, 0x21, 0, 0, 0xE2, 0xC, 0);
	/* MOD (long names): a name that does not fit the box (MetalGreymonVaccine) widens it to the title's width */
	renderText(cur++, 0x32, 0x37, 0, 0xC, modRegisterLong() ? REG_LONG_TEXT_W : REG_TEXT_W, 0x6C, 0);
	renderText(cur++, 0x46, 0xAD, 0, 0x78, 0xB4, 0xC, 0);
	renderText(cur++, 0x46, 0xC3, 0, 0x84, 0x24, 0xC, 0);
	renderText(cur++, 0x46, 0xCF, 0, 0x90, 0x24, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x28, 0x1C, 0xF6, 0x16);
	renderMenuBox(0x28, 0x32, modRegisterLong() ? 0xF6 : 0xE0, 0x76); /* MOD (long names) */
	renderMenuBox(0x3C, 0xA8, 0xC8, 0x16);
#endif
	renderMenuBox(0x3C, 0xBE, 0x38, 0x22);
}

void renderSelectRegisterSlot(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x46, 0x21, 0, 0, 0x6c, 0xc, 0);
	renderText(cur++, 0x46, 0x37, 0, 0xc, 0xe0, 0x78, 0);
	renderText(cur++, 0x46, 0xb9, 0, 0x84, 0x90, 0xc, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3c, 0x1c, 0x80, 0x16);
	renderMenuBox(0x3c, 0x32, 0xe0, 0x82);
	renderMenuBox(0x3c, 0xb4, 0xa4, 0x16);
#else
	renderText(cur++, 0x32, 0x21, 0, 0, 0xE2, 0xC, 0);
	/* MOD (long names): with a name that would pass the cursor the list moves under the title box, as wide
	 * (drawRegisteredDigimonSlots) */
	if (MOD_REG_LIST_LONG) {
		renderText(cur++, 0x32, 0x37, 0, 0xC, 0xE6, 0x78, 0);
	} else {
		renderText(cur++, 0x46, 0x37, 0, 0xC, 0xE0, 0x78, 0);
	}
	renderText(cur++, 0x46, 0xB9, 0, 0x84, 0xCA, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x28, 0x1C, 0xF6, 0x16);
	if (MOD_REG_LIST_LONG) {
		renderMenuBox(0x28, 0x32, 0xF6, 0x82);
	} else {
		renderMenuBox(0x3C, 0x32, 0xE0, 0x82);
	}
	renderMenuBox(0x3C, 0xB4, 0xD4, 0x16);
#endif
}

void renderConfirmRegister(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x46, 0x21, 0, 0, 0x6c, 0xc, 0);
	renderText(cur++, 0x46, 0x37, 0, 0xc, 0xc0, 0x18, 0);
	renderText(cur++, 0x46, 0x59, 0xc0, 0x18, 0x18, 0xc, 0);
	renderText(cur++, 0x46, 0x65, 0xcc, 0x18, 0x24, 0xc, 0);
	renderText(cur++, 0x46, 0x7d, 0, 0x24, 0xc0, 0x18, 2);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3c, 0x1c, 0x80, 0x16);
	renderMenuBox(0x3c, 0x32, 0xd4, 0x22);
	renderMenuBox(0x3c, 0x54, 0x38, 0x22);
	renderMenuBox(0x3c, 0x78, 0xd4, 0x22);
#else
	renderText(cur++, 0x32, 0x21, 0, 0, 0xE2, 0xC, 0);
	renderText(cur++, 0x46, 0x37, 0, 0xC, 0xC0, 0x18, 0);
	renderText(cur++, 0x46, 0x59, 0, 0x24, 0x28, 0xC, 0);
	renderText(cur++, 0x46, 0x65, 0, 0x30, 0x28, 0xC, 0);
	renderText(cur++, 0x46, 0x7D, 0, 0x3C, 0xC0, 0x24, 2);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x28, 0x1C, 0xF6, 0x16);
	renderMenuBox(0x3C, 0x32, 0xD4, 0x22);
	renderMenuBox(0x3C, 0x54, 0x38, 0x22);
	renderMenuBox(0x3C, 0x78, 0xD4, 0x2E);
#endif
}

void renderCantRegister(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x22, 0x49, 0, 0, 0xfc, 0xc, 3);
	renderText(cur++, 0x22, 0x59, 0, 0xc, 0xfc, 0xc, 3);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x18, 0x40, 0x110, 0x2e);
#else
	renderText(cur++, 0x22, 0x46, 0, 0, 0xFC, 0x30, 3);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x18, 0x40, 0x110, 0x3A);
#endif
}

void renderDoYouWantToSave(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x46, 0x3C, 0, 0, 0xA8, 0xC, 0);
	renderText(cur++, 0x46, 0x57, 0, 0xC, YES_TEXT_W, 0xC, 0);
	renderText(cur++, 0x46, 0x63, NO_TEXT_U, 0xC, NO_TEXT_W, 0xC, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3C, 0x32, 0xBC, 0x20);
	renderMenuBox(0x3C, 0x52, 0x38, 0x22);
}

void renderCantRegisterBaby(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x2E, 0x45, 0, 0, 0xE4, 0x24, 3);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x24, 0x40, 0xF8, 0x2E);
}

#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
void renderBattleModeMenu(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x5e, 0x37, 0, 0, 0x84, 0xc, 0);
	renderText(cur++, 0x5e, 0x43, 0, 0xc, 0x84, 0xc, 0);
	renderText(cur++, 0x5e, 0x4f, 0, 0x18, 0x84, 0xc, 0);
	if (SECRET_CODE != -1) {
		renderText(cur++, 0x5e, 0x5b, 0, 0x24, 0x84, 0xc, 0);
	}
	GsSetWorkBase((PACKET *)cur);
	if (SECRET_CODE != -1) {
		renderMenuBox(0x54, 0x32, 0x98, 0x3a);
	} else {
		renderMenuBox(0x54, 0x32, 0x98, 0x2e);
	}
}

void renderInsertGameCardPrompt(void)
{
	POLY_FT4 *cur;
	int32_t disabled;
	int32_t slotMask;

	cur = (POLY_FT4 *)GsGetWorkBase();
	renderText(cur++, 0x46, 0x37, 0, 0, 0xb4, 0x24, 0);
	if (MEMORY_CARD_ID == 0) {
		slotMask = 1;
	} else {
		slotMask = 0x10;
	}
	if (CONNECTED_CARDS & slotMask) {
		disabled = 0;
	} else {
		disabled = 1;
	}
	renderText(cur++, 0x46, 0x65, 0, 0x24, YES_TEXT_W, 0xc, disabled);
	renderText(cur++, 0x46, 0x71, NO_TEXT_U, 0x24, NO_TEXT_W, 0xc, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x3c, 0x32, 0xc8, 0x2e);
	renderMenuBox(0x3c, 0x60, 0x38, 0x22);
}
#endif

void drawMainMenuStrings(int32_t menu)
{
#if !VERSION_IS(US)
	int8_t view;
	int32_t i;
	char buf[0x2c];
#else
	int32_t view;
	int32_t i;
	char buf[0x2C];
	int32_t type;
#endif

	MAIN_MENU_TICKS = 0;
	CURRENT_MENU = menu;
	if (CURRENT_MENU == -1) {
		return;
	}
	view = MENU_VIEWS[CURRENT_MENU];
	if (view == -1) {
		return;
	}
	MENU_HIGHLIGHTS[view].pos = MENU_HIGHLIGHTS[view].pad[1];
	MENU_HIGHLIGHTS[view].pad[2] = 0;
	if (view == 0 && MOD_OPTIONS_BACK) { /* MOD (options): back from OPTIONS, on its row */
		MENU_HIGHLIGHTS[view].pos = 4;
	}
	MOD_OPTIONS_BACK = 0;
	SAVE_SLOT_SCROLL_TENTHS = 0;
	NEW_CARDS = 0;
	clearTextArea();
	if (view == 0 && MOD_BOOT_LANGUAGE) {
		modDrawLanguagePicker();
		return;
	}
#if !VERSION_IS(US)
	switch (view) {
	case 0:
		drawString(TITLE_MENU_ITEMS[0], 0, 0);
		drawString(TITLE_MENU_ITEMS[1], 0, 0xc);
		DrawSync(0);
		drawString(TITLE_MENU_ITEMS[2], 0, 0x18);
		drawString(TITLE_MENU_ITEMS[3], 0, 0x24);
		break;
	case 1:
		drawString(SLOT_ACTION_TITLES[0], 0, 0);
		DrawSync(0);
		drawString(SLOT_ACTION_TITLES[0], 0, 0xc);
		drawString(MAIN_D_80134674, 0x30, 0xc);
		DrawSync(0);
		drawString(SLOT_ACTION_TITLES[3], 0, 0x18);
		break;
	case 2:
		drawString(SLOT_ACTION_TITLES[1], 0, 0);
		DrawSync(0);
		drawString(SLOT_ACTION_TITLES[1], 0, 0xc);
		drawString(MAIN_D_80134674, 0x30, 0xc);
		break;
	case 3:
		break;
	case 4:
		drawString(SLOT_ACTION_TITLES[2], 0, 0);
		DrawSync(0);
		drawString(SLOT_ACTION_TITLES[2], 0, 0xc);
		drawString(MAIN_D_80134674, 0x30, 0xc);
		break;
	case 5:
		drawString(MEMORY_CARD_OPERATION_MESSAGES[MEMORY_CARD_OPERATION], 0, 0);
		DrawSync(0);
		drawString(MAIN_D_80131278, 0, 0xc);
		DrawSync(0);
		drawString(MAIN_D_80131290, 0, 0x18);
		break;
	case 6:
		drawString(SLOT_ACTION_TITLES[3], 0, 0);
		DrawSync(0);
		drawString(MAIN_D_801312A0, 0, 0xc);
		DrawSync(0);
		drawString(MAIN_D_801312B8, 0, 0x18);
		DrawSync(0);
		drawString(MAIN_D_801312D0, 0, 0x24);
		DrawSync(0);
		drawString(MAIN_D_801312E4, 0, 0x30);
		DrawSync(0);
		drawString(MAIN_D_80139DF4, 0, 0x3c);
		DrawSync(0);
		DRAW_STRING_CELLS(MAIN_D_80139E14, 0, 0xf0);
		break;
	case 7:
		drawString(SLOT_ACTION_TITLES[MAIN_MENU_ACTION], 0, 0);
		DrawSync(0);
		if (MAIN_MENU_ACTION == 0 || MAIN_MENU_ACTION == 1 || MAIN_MENU_ACTION == 2) {
			if (MEMORY_CARD_ID != 0) {
				drawString(MAIN_D_80134674, 0x30, 0);
			}
		}
		drawString(MAIN_D_80131340, 0x90, 0);
		for (i = 0; i < 6; i++) {
			drawSaveSlotText(i, i);
		}
		break;
	case 9:
		drawString(MAIN_D_801312FC, 0, 0);
		DrawSync(0);
		if (MEMORY_CARD_ID != 0) {
			drawString(MAIN_D_80134674, 0x30, 0);
		}
		DrawSync(0);
		drawString(MAIN_D_80131318, 0, 0xc);
		DrawSync(0);
		drawString(MAIN_D_80131328, 0, 0x18);
		DrawSync(0);
		DRAW_STRING_CELLS(MAIN_D_80139E14, 0, 0xf0);
		break;
	case 8:
		drawString(SLOT_ACTION_TITLES[MAIN_MENU_ACTION], 0, 0);
		if (MAIN_MENU_ACTION == 0 || MAIN_MENU_ACTION == 1 || MAIN_MENU_ACTION == 2) {
			if (MEMORY_CARD_ID != 0) {
				drawString(MAIN_D_80134674, 0x30, 0);
			}
		}
		DrawSync(0);
		drawString(MAIN_D_80131340, 0x90, 0);
		DrawSync(0);
		drawString(SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 0, 0xc);
		DrawSync(0);
		drawString(SAVEGAME_SLOT_INFO[MEMORY_CARD_SLOT].playerName, 0x18, 0xc);
		DrawSync(0);
		drawString(SAVEGAME_SLOT_INFO[MEMORY_CARD_SLOT].digimonName, 0x6c, 0xc);
		DrawSync(0);
		drawString(SAVEGAME_SLOT_INFO[MEMORY_CARD_SLOT].location, 0x18, 0x18);
		DrawSync(0);
		drawString(SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 0, 0x30);
		DrawSync(0);
		drawString(SLOT_ACTION_QUESTIONS[MAIN_MENU_ACTION], 0x18, 0x30);
		DrawSync(0);
		DRAW_STRING_CELLS(MAIN_D_80139E14, 0, 0xf0);
		if (MAIN_MENU_ACTION == 2) {
			MENU_HIGHLIGHTS[view].pos = 1;
		}
		break;
	case 10:
		drawString(MAIN_D_8013134C, 0, 0);
		DrawSync(0);
		if (MEMORY_CARD_ID != 0) {
			drawString(MAIN_D_80134674, 0x30, 0);
		}
		if (MEMORY_CARD_ERROR == 1) {
			drawString(MAIN_D_80134678, 0x3c, 0);
		}
		if (MEMORY_CARD_ERROR == 1 || MEMORY_CARD_ERROR == 4) {
			drawString(MAIN_D_8013467C, 0x9c, 0);
		}
		if (MEMORY_CARD_ERROR == 3) {
			drawString(MAIN_D_80134680, 0x9c, 0);
		}
		DrawSync(0);
		drawString(MEMORY_CARD_ERROR_MESSAGES[MEMORY_CARD_ERROR], 0, 0xc);
		break;
	case 11:
		drawString(MAIN_D_8013136C, 0, 0);
		drawString(MAIN_D_80131378, 0, 0xc);
		modSleepMenuWidth();
		break;
	case 12:
		drawString(MAIN_D_80131390, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_801313B0, 0, 0xc);
		DrawSync(0);
		if (MEMORY_CARD_ERROR != -1) {
			drawString(MAIN_D_8013134C, 0, 0x24);
			DrawSync(0);
			if (MEMORY_CARD_ID != 0) {
				drawString(MAIN_D_80134674, 0x30, 0x24);
			}
			if (MEMORY_CARD_ERROR == 1) {
				drawString(MAIN_D_80134678, 0x3c, 0x24);
			}
			if (MEMORY_CARD_ERROR == 1 || MEMORY_CARD_ERROR == 4) {
				drawString(MAIN_D_8013467C, 0x9c, 0x24);
			}
			if (MEMORY_CARD_ERROR == 3) {
				drawString(MAIN_D_80134680, 0x9c, 0x24);
			}
			DrawSync(0);
			drawString(MEMORY_CARD_ERROR_MESSAGES[MEMORY_CARD_ERROR], 0, 0x30);
			DrawSync(0);
		}
		DRAW_STRING_CELLS(MAIN_D_80139E14, 0, 0xf0);
		break;
	case 13:
		drawString(SLOT_ACTION_TITLES[5], 0, 0);
		DrawSync(0);
		drawString(MAIN_D_801313E4, 0, 0xc);
		DrawSync(0);
		drawString(MAIN_D_80131400, 0, 0x18);
		DrawSync(0);
		drawString(MAIN_D_8013141C, 0, 0x24);
		DrawSync(0);
		DRAW_STRING_CELLS(MAIN_D_80139F6C, 0, 0xf0);
		if (VS_PLAYER_INDEX == 1) {
			drawString(MAIN_D_80134674, 0x30, 0xc);
			drawString(MAIN_D_80134674, 0x84, 0xc);
		}
		break;
	case 14:
		drawString(SLOT_ACTION_TITLES[6], 0, 0);
		DrawSync(0);
		drawString(MAIN_D_80134684, 0, 0xc);
		drawString(PARTNER_ENTITY.name, 0x2a, 0xc);
		DrawSync(0);
		drawString(MAIN_D_8013D814, 6, 0x18);
		drawString(DIGIMON_NAME(PARTNER_ENTITY.digimonEntity.entity.type), 0x2a, 0x18);
		DrawSync(0);
		drawString(MAIN_D_80134694, 6, 0x24);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.hp, buf, 4), 0x2a, 0x24);
		DrawSync(0);
		drawString(MAIN_D_80134698, 6, 0x30);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.mp, buf, 4), 0x2a, 0x30);
		DrawSync(0);
		drawString(MAIN_D_8013469C, 0, 0x3c);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.off, buf, 3), 0x36, 0x3c);
		DrawSync(0);
		drawString(MAIN_D_801346A4, 0, 0x48);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.def, buf, 3), 0x36, 0x48);
		DrawSync(0);
		drawString(MAIN_D_8013143C, 6, 0x54);
		drawMoveName(PARTNER_ENTITY.digimonEntity.entity.type, PARTNER_ENTITY.digimonEntity.stats.base.moves[0], 0x2a, 0x54);
		DrawSync(0);
		drawString(MAIN_D_80131448, 6, 0x60);
		drawMoveName(PARTNER_ENTITY.digimonEntity.entity.type, PARTNER_ENTITY.digimonEntity.stats.base.moves[1], 0x2a, 0x60);
		DrawSync(0);
		drawString(MAIN_D_80131454, 6, 0x6c);
		drawMoveName(PARTNER_ENTITY.digimonEntity.entity.type, PARTNER_ENTITY.digimonEntity.stats.base.moves[2], 0x2a, 0x6c);
		DrawSync(0);
		drawString(MAIN_D_80131460, 0, 0x78);
		break;
	case 15:
		drawString(SLOT_ACTION_TITLES[6], 0, 0);
		DrawSync(0);
		drawRegisteredDigimonSlots(0);
		drawString(MAIN_D_8013147C, 0, 0x84);
		break;
	case 16:
		drawString(SLOT_ACTION_TITLES[6], 0, 0);
		DrawSync(0);
		drawString(&SAVEGAME_ID_LABEL[(BATTLE_REGISTRATION_SLOT + 1) / 10][2], 0, 0xc);
		DrawSync(0);
		drawString(&SAVEGAME_ID_LABEL[(BATTLE_REGISTRATION_SLOT + 1) % 10][2], 0xc, 0xc);
		DrawSync(0);
		strcpy(buf, MAIN_D_8013D854);
		strcat(buf, PARTNER_ENTITY.name);
		strcat(buf, MAIN_D_80139FBC);
		drawString(buf, 0x18, 0xc);
		DrawSync(0);
		drawString(MAIN_D_80139FCC, 0, 0x18);
		DrawSync(0);
		drawString(MAIN_D_801314B0, 0, 0x24);
		DrawSync(0);
		drawString(MAIN_D_801314C8, 0, 0x30);
		break;
	case 17:
		drawString(MAIN_D_80131500, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_8013151C, 0, 0xc);
		break;
	case 18:
		drawString(MAIN_D_80131568, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_80139E14, 0, 0xc);
		break;
	case 19:
		strcpy(buf, PARTNER_ENTITY.name);
		strcat(buf, MAIN_D_801346B4);
		strcat(buf, DIGIMON_NAME(PARTNER_ENTITY.digimonEntity.entity.type));
		strcat(buf, MAIN_D_801346B8);
		drawString(buf, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_80131580, 0, 0xc);
		DrawSync(0);
		drawString(MAIN_D_801315A0, 0, 0x18);
		break;
#if !VERSION_IS(JP)
	case 20:
		drawString(BATTLE_MODE_ITEMS[0], 0, 0);
		DrawSync(0);
		drawString(BATTLE_MODE_ITEMS[1], 0, 0xc);
		DrawSync(0);
		drawString(BATTLE_MODE_ITEMS[2], 0, 0x18);
		drawString(BATTLE_MODE_ITEMS[3], 0, 0x24);
		break;
	case 21:
		drawString(MAIN_D_801315C0, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_801315D0, 0, 0xc);
		DrawSync(0);
		drawString(MAIN_D_801315EC, 0, 0x18);
		DrawSync(0);
		drawString(MAIN_D_80139E14, 0, 0x24);
		break;
#endif
	}
#else
	switch (view) {
	case 0:
		/* MOD (language): keep translated labels inside the title frame with either font (12 pixels of margin in its crop). */
		drawStringMode(TITLE_MENU_ITEMS[0], 0, 0, fitStringMode(TITLE_MENU_ITEMS[0], 0xA4));
		drawStringMode(TITLE_MENU_ITEMS[1], 0, 0xC, fitStringMode(TITLE_MENU_ITEMS[1], 0xA4));
		DrawSync(0);
		drawStringMode(TITLE_MENU_ITEMS[2], 0, 0x18, fitStringMode(TITLE_MENU_ITEMS[2], 0xA4));
		drawStringMode(TITLE_MENU_ITEMS[3], 0, 0x24, fitStringMode(TITLE_MENU_ITEMS[3], 0xA4));
		DrawSync(0);
		drawString(MOD_OPT_STR_OPTIONS, 0, 0x30); /* MOD (options) */
		DrawSync(0);
		modVersionDraw(); /* MOD (version) */
		break;
	case 22: /* MOD (options) */
		drawOptionsText();
		break;
	case 1:
		drawString(SLOT_ACTION_TITLES[0], 0, 0);
		DrawSync(0);
		modDrawSlot2(SLOT_ACTION_TITLES[0], 0, 0xC, 1); /* MOD (language) */
		DrawSync(0);
		drawString(SLOT_ACTION_TITLES[3], 0, 0x18);
		break;
	case 2:
		drawString(SLOT_ACTION_TITLES[1], 0, 0);
		DrawSync(0);
		modDrawSlot2(SLOT_ACTION_TITLES[1], 0, 0xC, 1); /* MOD (language) */
		MAIN_D_80135054 = 1;
		break;
	case 4:
		drawString(SLOT_ACTION_TITLES[2], 0, 0);
		DrawSync(0);
		modDrawSlot2(SLOT_ACTION_TITLES[2], 0, 0xC, 1); /* MOD (language) */
		MAIN_D_80135054 = 2;
		break;
	case 5:
		drawString(MEMORY_CARD_OPERATION_MESSAGES[MEMORY_CARD_OPERATION], 0, 0);
		DrawSync(0);
		drawString(MAIN_D_80131278, 0, 0xC);
		DrawSync(0);
		drawString(MAIN_D_80131290, 0, 0x18);
		break;
	case 6:
		drawString(SLOT_ACTION_TITLES[3], 0, 0);
		DrawSync(0);
		drawString(MAIN_D_801312A0, 0, 0xC);
		DrawSync(0);
		drawString(MAIN_D_801312B8, 0, 0x18);
		DrawSync(0);
		drawString(MAIN_D_801312D0, 0, 0x24);
		DrawSync(0);
		drawString(MAIN_D_801312E4, 0, 0x30);
		DrawSync(0);
		DRAW_STRING_PIECES_AT(STR_YES_NO_SPACED, 0, 0xF0, YES_NO_PIECES);
		break;
	case 7:
		/* MOD (language): the title of slot 2 drawn whole (the original drew a "2" at the x of the US title's "1") */
		modDrawSlot2(SLOT_ACTION_TITLES[MAIN_MENU_ACTION], 0, 0, MAIN_MENU_ACTION <= 2 && MEMORY_CARD_ID != 0);
		DrawSync(0);
		for (i = 0; i < 6; i++) {
			drawSaveSlotText(i, i);
		}
		break;
	case 9:
		modDrawSlot2(MAIN_D_801312FC, 0, 0, MEMORY_CARD_ID != 0); /* MOD (language) */
		DrawSync(0);
		drawString(MAIN_D_80131318, 0, 0xC);
		DrawSync(0);
		drawString(MAIN_D_80131328, 0, 0x18);
		DrawSync(0);
		DRAW_STRING_PIECES_AT(STR_YES_NO_SPACED, 0, 0xF0, YES_NO_PIECES);
		break;
	case 8:
		modDrawSlot2(SLOT_ACTION_TITLES[MAIN_MENU_ACTION], 0, 0, MAIN_MENU_ACTION <= 2 && MEMORY_CARD_ID != 0); /* MOD (language) */
		DrawSync(0);
		drawString(MAIN_D_80131340, 0x90, 0);
		DrawSync(0);
		drawString(SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 0, 0xC);
		DrawSync(0);
		drawString(SAVEGAME_SLOT_INFO[MEMORY_CARD_SLOT].playerName, 0x18, 0xC);
		DrawSync(0);
		modDrawSlotName(SAVEGAME_SLOT_INFO[MEMORY_CARD_SLOT].digimonName, 0xC); /* MOD (long names) */
		DrawSync(0);
		drawString(SAVEGAME_SLOT_INFO[MEMORY_CARD_SLOT].location, 0, 0x18);
		DrawSync(0);
		drawString(SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 0, 0x30);
		DrawSync(0);
		drawString(SLOT_ACTION_QUESTIONS[MAIN_MENU_ACTION], 0x18, 0x30);
		DrawSync(0);
		DRAW_STRING_PIECES_AT(STR_YES_NO_SPACED, 0, 0xF0, YES_NO_PIECES);
		if (MAIN_MENU_ACTION == 2) {
			MENU_HIGHLIGHTS[view].pos = 1;
		}
		break;
	case 10:
		drawString(MAIN_D_8013134C, 0, 0);
		DrawSync(0);
		if (MEMORY_CARD_ID != 0) {
			drawString(MAIN_D_80134674, 0x30, 0);
		}
		if (MEMORY_CARD_ERROR == 1) {
			drawString(MAIN_D_80134678, 0x3C, 0);
		}
		if (MEMORY_CARD_ERROR == 1 || MEMORY_CARD_ERROR == 4) {
			drawString(MAIN_D_8013467C, 0x9C, 0);
		}
		if (MEMORY_CARD_ERROR == 3) {
			drawString(MAIN_D_80134680, 0x9C, 0);
		}
		DrawSync(0);
		drawString(MEMORY_CARD_ERROR_MESSAGES[MEMORY_CARD_ERROR], 0, 0xC);
		break;
	case 11:
		drawString(MAIN_D_8013136C, 0, 0);
		drawString(MAIN_D_80131378, 0, 0xC);
		modSleepMenuWidth();
		break;
	case 12:
		drawString(MAIN_D_80131390, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_801313B0, 0, 0xC);
		DrawSync(0);
		if (MEMORY_CARD_ERROR != -1) {
			drawString(MAIN_D_801313D0, 0, 0x24);
			DrawSync(0);
			if (MEMORY_CARD_ID != 0) {
				drawString(&STR_EMPTY, 0x30, 0x24);
			}
			if (MEMORY_CARD_ERROR == 1) {
				drawString(&STR_EMPTY, 0x3C, 0x24);
			}
			if (MEMORY_CARD_ERROR == 1 || MEMORY_CARD_ERROR == 4) {
				drawString(&STR_EMPTY, 0x9C, 0x24);
			}
			if (MEMORY_CARD_ERROR == 3) {
				drawString(&STR_EMPTY, 0x9C, 0x24);
			}
			DrawSync(0);
			drawString(MEMORY_CARD_ERROR_MESSAGES[MEMORY_CARD_ERROR], 0, 0x30);
			DrawSync(0);
		}
		DRAW_STRING_PIECES_AT(STR_YES_NO_SPACED, 0, 0xF0, YES_NO_PIECES);
		break;
	case 13:
		drawString(SLOT_ACTION_TITLES[5], 0, 0);
		DrawSync(0);
		/* MOD (language): player 2's rows drawn with their own "2"s (the original drew two at the US row's x) */
		modDrawSlot2(MAIN_D_801313E4, 0, 0xC, VS_PLAYER_INDEX == 1);
		DrawSync(0);
		modDrawSlot2(MAIN_D_80131400, 0, 0x18, VS_PLAYER_INDEX == 1);
		DrawSync(0);
		modDrawSlot2(MAIN_D_8013141C, 0, 0x24, VS_PLAYER_INDEX == 1);
		DrawSync(0);
		DRAW_STRING_PIECES_AT(STR_YES_NO_SPACED, 0, 0xF0, YES_NO_PIECES);
		break;
	case 14:
		drawString(SLOT_ACTION_TITLES[6], 0, 0);
		DrawSync(0);
		drawString(MAIN_D_80134684, 0, 0xC);
		drawString(PARTNER_ENTITY.name, 0x64, 0xC);
		DrawSync(0);
		drawString(MAIN_D_8013468C, 0, 0x18);
		/* MOD (long names): a name that does not fit the box is drawn tighter in the wider box (modRegisterLong) */
		if (modRegisterLong()) {
			drawStringMode(DIGIMON_NAME(PARTNER_ENTITY.digimonEntity.entity.type), REG_NAME_X, 0x18,
				       fitStringMode(DIGIMON_NAME(PARTNER_ENTITY.digimonEntity.entity.type),
						     REG_LONG_NAME_W));
		} else {
			drawString(DIGIMON_NAME(PARTNER_ENTITY.digimonEntity.entity.type), REG_NAME_X, 0x18);
		}
		DrawSync(0);
		drawString(MAIN_D_80134694, 0, 0x24);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.hp, buf, 4), 0x64, 0x24);
		DrawSync(0);
		drawString(MAIN_D_80134698, 0, 0x30);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.mp, buf, 4), 0x64, 0x30);
		DrawSync(0);
		drawString(MAIN_D_8013469C, 0, 0x3C);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.off, buf, 3), 0x64, 0x3C);
		DrawSync(0);
		drawString(MAIN_D_801346A4, 0, 0x48);
		drawString(formatInteger(PARTNER_ENTITY.digimonEntity.stats.base.def, buf, 3), 0x64, 0x48);
		DrawSync(0);
		drawString(MAIN_D_8013143C, 0, 0x54);
		drawMoveName((type = PARTNER_ENTITY.digimonEntity.entity.type), PARTNER_ENTITY.digimonEntity.stats.base.moves[0], 0x64, 0x54);
		DrawSync(0);
		drawString(MAIN_D_80131448, 0, 0x60);
		drawMoveName((type = PARTNER_ENTITY.digimonEntity.entity.type), PARTNER_ENTITY.digimonEntity.stats.base.moves[1], 0x64, 0x60);
		DrawSync(0);
		drawString(MAIN_D_80131454, 0, 0x6C);
		drawMoveName((type = PARTNER_ENTITY.digimonEntity.entity.type), PARTNER_ENTITY.digimonEntity.stats.base.moves[2], 0x64, 0x6C);
		DrawSync(0);
		drawString(MAIN_D_80131460, 0, 0x78);
		drawString(MAIN_D_801346AC, 0, 0x84);
		drawString(MAIN_D_801346B0, 0, 0x90);
		break;
	case 15:
		drawString(SLOT_ACTION_TITLES[6], 0, 0);
		DrawSync(0);
		drawRegisteredDigimonSlots(0);
		drawString(MAIN_D_8013147C, 0, 0x84);
		break;
	case 16:
		drawString(SLOT_ACTION_TITLES[6], 0, 0);
		DrawSync(0);
		strcpy(buf, MAIN_D_80131498);
		drawString(buf, 0, 0xC);
		DrawSync(0);
		strcpy(buf, PARTNER_ENTITY.name);
		drawString(buf, 0, 0x18);
		DrawSync(0);
		drawString(&SAVEGAME_ID_LABEL[(BATTLE_REGISTRATION_SLOT + 1) / 10][2], 0, 0xC);
		DrawSync(0);
		drawString(&SAVEGAME_ID_LABEL[(BATTLE_REGISTRATION_SLOT + 1) % 10][2], 0xC, 0xC);
		DrawSync(0);
		drawString(MAIN_D_801346AC, 0, 0x24);
		DrawSync(0);
		drawString(MAIN_D_801346B0, 0, 0x30);
		DrawSync(0);
		drawString(MAIN_D_801314B0, 0, 0x3C);
		DrawSync(0);
		drawString(MAIN_D_801314C8, 0, 0x48);
		drawString(MAIN_D_801314E4, 0, 0x54);
		break;
	case 17:
		drawString(MAIN_D_80131500, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_8013151C, 0, 0xC);
		DrawSync(0);
		drawString(MAIN_D_80131538, 0, 0x18);
		drawString(MAIN_D_8013154C, 0, 0x24);
		break;
	case 18:
		drawString(MAIN_D_80131568, 0, 0);
		DrawSync(0);
		DRAW_STRING_PIECES_AT(STR_YES_NO_SPACED, 0, 0xC, YES_NO_PIECES); /* MOD (font): cut into Yes and No */
		break;
	case 19:
		strcpy(buf, PARTNER_ENTITY.name);
		if (!modPartnerNameIsSpecies(PARTNER_ENTITY.digimonEntity.entity.type)) { /* MOD (optional name): not "Agumon(Agumon)" */
			strcat(buf, MAIN_D_801346B4);
			strcat(buf, DIGIMON_NAME(PARTNER_ENTITY.digimonEntity.entity.type));
			strcat(buf, MAIN_D_801346B8);
		} else {
			strcat(buf, MAIN_D_801346B8);
		}
		drawString(buf, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_80131580, 0, 0xC);
		DrawSync(0);
		drawString(MAIN_D_801315A0, 0, 0x18);
		break;
	case 20:
		drawString(BATTLE_MODE_ITEMS[0], 0, 0);
		DrawSync(0);
		drawString(BATTLE_MODE_ITEMS[1], 0, 0xC);
		DrawSync(0);
		drawString(BATTLE_MODE_ITEMS[2], 0, 0x18);
		drawString(BATTLE_MODE_ITEMS[3], 0, 0x24);
		break;
	case 21:
		drawString(MAIN_D_801315C0, 0, 0);
		DrawSync(0);
		drawString(MAIN_D_801315D0, 0, 0xC);
		DrawSync(0);
		drawString(MAIN_D_801315EC, 0, 0x18);
		DrawSync(0);
		DRAW_STRING_PIECES_AT(STR_YES_NO_SPACED, 0, 0x24, YES_NO_PIECES); /* MOD (font): cut into Yes and No */
		break;
	}
#endif
}

/*
 * MOD (long names): the save slot screens' long layout, when a slot's species passes the original column (0x60
 * pixels: the original cut MetalGreymon to "MetalGreymo"): the boxes, their cursors and texts 0x18 further left, the
 * boxes 0x110 wide; the species column SLOT_LONG_NAME_W wide, a name tighter if it must.
 */
static void modSaveSlotsLayout(int32_t isLong)
{
	MOD_SAVE_SLOTS_LONG = isLong;
	MENU_HIGHLIGHTS[7].x = 0x36 - SLOT_DX;
	MENU_HIGHLIGHTS[7].w = 0xD4 + SLOT_BOX_W - 0xE0;
	MENU_HIGHLIGHTS[8].x = 0x37 - SLOT_DX;
}

/* MOD (long names): a save slot's species in its column, tighter in the long layout if it must */
/*
 * MOD (language): a row that names slot or player 1, drawn for 2 (`slot2`): every digit 1 of it becomes STR_2's. The
 * original drew the row and then a "2" at the x of the US text's "1", which lands elsewhere in another text or font
 * ("SEGUIR EN RANURA 1   2").
 */
static void modDrawSlot2(char *str, int32_t x, int32_t y, int32_t slot2)
{
	char buf[0x40];
	int32_t i;

	if (!slot2) {
		drawString(str, x, y);
		return;
	}
	for (i = 0; str[i] != '\0' && i < (int32_t)sizeof(buf) - 1; i++) {
		buf[i] = str[i] == '1' ? STR_2[0] : str[i]; /* (a two-byte code's second byte is never below 0x40) */
	}
	buf[i] = '\0';
	drawString(buf, x, y);
}

static void modDrawSlotName(char *name, int32_t y)
{
    if (MOD_LANG == MOD_LANG_ZH) name = zhCardName(name);
	if (MOD_SAVE_SLOTS_LONG) {
		drawStringMode(name, SLOT_NAME_X, y, fitStringMode(name, SLOT_LONG_NAME_W));
	} else {
		drawString(name, SLOT_NAME_X, y);
	}
}

void drawSaveSlotText(int32_t slot, int32_t row)
{
	RECT area;

	if (slot >= 0 && slot < 0xF) {
		row *= 0x18;
#if !VERSION_IS(US)
		setRECT(&area, 0, row + 0xc, 0xcc, 0x18);
#else
		setRECT(&area, 0, row + 0xC, MOD_SAVE_SLOTS_LONG ? 0xF0 : 0xE0, 0x18); /* MOD (long names) */
#endif
		clearTextSubArea(&area);
		drawString(SAVEGAME_ID_LABEL[slot + 1], 0, row + 0xC);
		drawString(SAVEGAME_SLOT_INFO[slot].playerName, 0x18, row + 0xC);
		DrawSync(0);
#if VERSION_IS(US)
		modDrawSlotName(SAVEGAME_SLOT_INFO[slot].digimonName, row + 0xC); /* MOD (long names) */
#else
		drawString(SAVEGAME_SLOT_INFO[slot].digimonName, 0x6C, row + 0xC);
#endif
		DrawSync(0);
		drawString(SAVEGAME_SLOT_INFO[slot].location, 0x18, row + 0x18);
		DrawSync(0);
	}
}

char *formatInteger(int32_t value, char *buf, int32_t digits)
{
#if !VERSION_IS(US)
	int32_t started;
	char *dst;

	started = 0;
	dst = buf;
	switch (digits) {
	case 4:
		appendWideDigit(value / 1000, &dst, &started);
		value %= 1000;
	case 3:
		appendWideDigit(value / 100, &dst, &started);
		value %= 100;
	case 2:
		appendWideDigit(value / 10, &dst, &started);
		value %= 10;
	case 1:
		appendWideDigit(value, &dst, &started);
	}
	*dst = 0;
	return buf;
#else
	sprintf(buf, FMT_NUMBER, value);
	return buf;
#endif
}

void drawMoveName(long type, int32_t anim, int32_t color, int32_t pos)
{
	int16_t move;

	if (anim >= 0x2E && anim < 0x3E) {
		move = DIGI_PARA(type).moves[anim - 0x2E];
		if (move >= 0 && move < NUM_MOVE_NAMES) {
			drawStringMode(MOVE_NAMES[move], color, pos, fitStringMode(MOVE_NAMES[move], 120));
		}
	}
}

/*
 * MOD (long names): the registration screen's long layout, when the partner's species name drawn by the font would
 * pass the box (REG_TEXT_W): the box as wide as the title's, the name tighter if it must (REG_LONG_NAME_W).
 */
static int32_t modRegisterLong(void)
{
	return REG_NAME_X + measureStringMode(DIGIMON_NAME(PARTNER_ENTITY.digimonEntity.entity.type), FIT_VANILLA) >
	       REG_TEXT_W;
}

/*
 * MOD (long names): the save title holds 17 characters of the species name (title + 0x1C); the rest of a longer
 * name goes to the header's reserved bytes after "LN", for the save slots (loadSaveSlotData). SAVE_FILE.reserved was
 * just cleared.
 */
static void modTitleNameTail(void)
{
	char *name;

	name = MOD_LANG == MOD_LANG_ZH ? zhLatinName(SAVE_FILE.saves[0].partnerType) : DIGIMON_NAME(SAVE_FILE.saves[0].partnerType);
	if (strlen(name) > 0x11) {
		SAVE_FILE.reserved[0] = 'L';
		SAVE_FILE.reserved[1] = 'N';
		strncpy((char *)&SAVE_FILE.reserved[2], name + 0x11, sizeof(SAVE_FILE.reserved) - 3);
	}
}

/*
 * MOD (language): the player's name in the save title, written by the memory card's own screens too, must be plain
 * Shift-JIS: a glyph of the mod font (F040-F087, the name keyboard) becomes its full-width plain letter there (a symbol:
 * the Shift-JIS one, ♥ a white star).
 * Which ones were accented goes to the header's reserved bytes after the long names' tail ("A" at [5], then each
 * letter's F0 code or 0), for the save slots (loadSaveSlotData). Run after modTitleNameTail.
 */
static const uint16_t PLAIN_LETTERS[71] = {
	0x8281, 0x8285, 0x828f, 0x8295, 0x8281, 0x8285, 0x828f, 0x8289, /* á é ó ú à è ò í */
	0x8289, 0x8295, 0x828e, 0x8283, 0x8260, 0x8264, 0x826e, 0x8274, /* ï ü ñ ç Á É Ó Ú */
	0x8260, 0x8264, 0x826e, 0x8268, 0x8268, 0x8274, 0x826d, 0x8262, /* À È Ò Í Ï Ü Ñ Ç */
	0x8148, 0x8149, 0x8145, 0x81e1, 0x81e2, 0x8169, 0x816a, 0x8195, /* ¿ ¡ · « » ( ) & */
	0x8193, 0x815e, 0x8163, 0x816d, 0x816e, 0x8194, 0x8196, 0x8160, /* % / … [ ] # * ~ */
	0x8197, 0x815c, 0x815c, 0x81f4, 0x8281, 0x8285, 0x8289, 0x828f, /* @ – — ♪ â ê î ô */
	0x8295, 0x8281, 0x8285, 0x828f, 0x8281, 0x828f, 0x8289, 0x8295, /* û ä ë ö ã õ ì ù */
	0x8293, 0x8260, 0x8264, 0x8268, 0x826e, 0x8274, 0x8260, 0x8264, /* ß Â Ê Î Ô Û Ä Ë */
	0x826e, 0x8260, 0x826e, 0x8268, 0x8274, 0x8199, 0x819a, /* Ö Ã Õ Ì Ù ♥ ★ */
};

static void modTitlePlayerName(void)
{
	uint8_t *p;
	int32_t i;
	uint16_t plain;
	int32_t n;

	p = (uint8_t *)SAVE_FILE.title + 0xe;
	for (i = 0; i < 6 && p[i * 2] != 0; i++) {
		if (p[i * 2] != 0xf0) {
			continue;
		}
		SAVE_FILE.reserved[5] = 'A';
		SAVE_FILE.reserved[6 + i] = p[i * 2 + 1];
		n = p[i * 2 + 1] - 0x40 - (p[i * 2 + 1] > 0x7f); /* F07F is skipped */
		plain = n >= 0 && n < (int32_t)(sizeof(PLAIN_LETTERS) / sizeof(PLAIN_LETTERS[0])) ? PLAIN_LETTERS[n] : 0x8140;
		p[i * 2] = plain >> 8;
		p[i * 2 + 1] = plain;
	}
}

/* Shared by the initial, sleep and save-machine card headers. */
static void modTitleNames(void)
{
	asciiToShiftJIS((uint8_t *)MAIN_D_8013392C,
	(uint16_t *)SAVE_FILE.title);
	_strncpy(SAVE_FILE.title + 8,
	SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 4);
	/* MOD: _strncpy does not terminate: clear old name bytes (original showed "Bayer" for "B" after "Player") */
	memset(SAVE_FILE.title + 0xe, 0, 0xe);
	_strncpy(SAVE_FILE.title + 0xe,
	SAVE_FILE.saves[0].playerName, 0xD);
	memset(SAVE_FILE.title + 0x1c, 0, 0x12);
	_strncpy(SAVE_FILE.title + 0x1c,
	MOD_LANG == MOD_LANG_ZH ? zhLatinName(SAVE_FILE.saves[0].partnerType) : DIGIMON_NAME(SAVE_FILE.saves[0].partnerType),
	0x11);
	memcpy(SAVE_FILE.reserved, SAVE_TITLE_RESERVED, 0x1C);
	modTitleNameTail(); /* MOD (long names) */
	modTitlePlayerName(); /* MOD (language) */
	modTitleOptions(); /* MOD (options) */
}

void drawRegisteredDigimonSlots(int32_t slot)
{
	RECT area;
	int32_t type; /* MOD (wide registered species): was u8 */
	int32_t i;
	int32_t y;
	int32_t currentSlot;

	/* MOD (long names): the long layout when a registered name drawn by the font would pass the cursor; decided
	 * over every slot, so the layout does not change while scrolling */
	MOD_REG_LIST_LONG = 0;
	for (i = 0; i < 40; i++) {
		type = registeredSpecies(&SAVE_FILE.saves[0].battleRegistrationData[i]);
		if (type != 0 && LIST_NAME_X + measureStringMode(DIGIMON_NAME(type), FIT_VANILLA) > LIST_CURSOR_END) {
			MOD_REG_LIST_LONG = 1;
		}
	}
	MENU_HIGHLIGHTS[15].x = MOD_REG_LIST_LONG ? 0x32 : 0x46;
	MENU_HIGHLIGHTS[15].w = MOD_REG_LIST_LONG ? 0xE6 : 0xCC;
	setRECT(&area, 0, 0xC, 0xF0, 0x78); /* MOD (long names): was 0xCC wide (a longer name was left behind) */
	clearTextSubArea(&area);
	for (i = 0; i < 10; i++) {
		currentSlot = slot + i;
		y = i * 0xc + 0xc;
		drawString(&SAVEGAME_ID_LABEL[(currentSlot + 1) / 10][2], 0, y);
		drawString(&SAVEGAME_ID_LABEL[(currentSlot + 1) % 10][2], 0xC, y);
		if ((type = registeredSpecies(&SAVE_FILE.saves[0].battleRegistrationData[currentSlot])) != 0) {
			drawString(SAVE_FILE.saves[0].battleRegistrationData[currentSlot].name, 0x1e, y);
			if (MOD_REG_LIST_LONG) { /* MOD (long names) */
				drawStringMode(DIGIMON_NAME(type), LIST_NAME_X, y,
					       fitStringMode(DIGIMON_NAME(type), LIST_LONG_NAME_W));
			} else {
				drawString(DIGIMON_NAME(type), LIST_NAME_X, y);
			}
		} else {
			setTextColor(9);
#if !VERSION_IS(US)
			drawString(MAIN_D_8013A0EC, 0x1e, y);
#else
			drawString(MAIN_D_801346C0[0], 0x1E, y);
#endif
			setTextColor(1);
		}
		DrawSync(0);
	}
}

void updateMemoryCardState(void)
{
	MemCardSyncWord result;
	MemCardSyncWord cmd;

	switch (MemCardSync(1, &cmd, &result)) {
	case -1:
		MemCardExist(CHECKED_MEMORY_CARD);
		return;
	case 0:
		return;
	case 1:
		if (cmd == 1) {
			switch (result) {
			case 3:
				NEW_CARDS |= (CHECKED_MEMORY_CARD == 0x10) ? 0x10 : 1;
			case 0:
				CONNECTED_CARDS |= (CHECKED_MEMORY_CARD == 0x10) ? 0x10 : 1;
				CHECKED_MEMORY_CARD ^= 0x10;
				break;
			case 2:
			case 1:
			default:
				CONNECTED_CARDS &= (CHECKED_MEMORY_CARD == 0x10) ? -0x11 : -2;
				CHECKED_MEMORY_CARD ^= 0x10;
				break;
			}
		}
		return;
	}
}

void tickMainMenu(void)
{
	int32_t view;
	MemCardSyncWord command;
	MemCardSyncWord result;
	long count;
	int32_t slot;
	uint32_t combinedInput;
	int32_t lo;
	int32_t hi;
	int32_t oldScroll;
	MenuCursor *cursor;
	int32_t input;
	int32_t value; /* MOD (options) */
	int32_t values;
	int32_t current;
	int32_t status;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	int32_t i;
	SavedInventory *inventory; /* MOD: the save layout */
#endif

	updateMemoryCardState();
	if (TARGET_MENU != CURRENT_MENU) {
		drawMainMenuStrings(TARGET_MENU);
	}
	MAIN_MENU_TICKS++;
    if (MOD_LANGUAGE_ERROR) {
        if (CHANGED_INPUT & (CONFIRM_BUTTON | CANCEL_BUTTON | 0x800)) {
            MOD_LANGUAGE_ERROR = 0;
            playSound(0, 3);
        }
        return;
    }
	if (MOD_BOOT_LANGUAGE && CURRENT_MENU == 0) {
		modTickLanguagePicker();
		return;
	}

	switch (CURRENT_MENU) {
	case 0:
#if !VERSION_IS(US)
		cursor = (MenuCursor *)MENU_HIGHLIGHTS;
		input = tickMenuInput(cursor, 1);
#else
		input = tickMenuInput(cursor = (MenuCursor *)MENU_HIGHLIGHTS, 1);
#endif
		switch (input) {
		case 1:
			switch (cursor->pos) {
			case 0:
				TARGET_MENU = 0xA;
				MAIN_MENU_ACTION = 0;
				break;
			case 1:
				if (CONNECTED_CARDS != 0) {
					TARGET_MENU = 0x14;
					MAIN_MENU_ACTION = 1;
				}
				break;
			case 2:
				if (CONNECTED_CARDS != 0) {
					TARGET_MENU = 0x1E;
					MAIN_MENU_ACTION = 2;
				}
				break;
			case 3:
				VS_PLAYER_INDEX = 0;
				TARGET_MENU = 0x32;
				MAIN_MENU_ACTION = 5;
				break;
			case 4: /* MOD (options): showing the newest save's */
				modCardOptions();
				TARGET_MENU = 4;
				break;
			}
			break;
		case 2:
			break;
		}
		break;

	case 4: /* MOD (options) */
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[22];
		input = tickMenuInput(cursor, 1);
		value = -1;
		values = modOptionCount(cursor->pos);
		current = modOptionValue(cursor->pos);
		if (input == 0 && CHANGED_INPUT == 0x8000) { /* left: the value before */
			value = current > 0 ? current - 1 : 0;
		} else if (input == 0 && CHANGED_INPUT == 0x2000) { /* right: the one after */
			value = current < 0 ? values - 1 : current < values - 1 ? current + 1 : current;
		} else if (input == 1) { /* confirm: the next one, round */
			value = (current + 1) % values;
		}
		if (value >= 0 && value != modOptionValue(cursor->pos)) {
			if (input == 0) {
				playSound(0, 2);
			}
			modSetOption(cursor->pos, value);
			clearTextArea();
			drawOptionsText();
		} else if (input == 2) {
			MOD_OPTIONS_BACK = 1;
			TARGET_MENU = 0;
		}
		break;

	case 1:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[9];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				TARGET_MENU = 2;
				MEMORY_CARD_OPERATION = 5;
			} else {
				TARGET_MENU = 0;
			}
			break;
		case 2:
			TARGET_MENU = 0;
			break;
		}
		TARGET_MENU = getMenuOnCardChange(0);
		break;

	case 2:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			status = MemCardFormat(MEMORY_CARD_ID);
			switch (status) {
			case 0:
				TARGET_MENU = MEMORY_CARD_RETURN_MENU;
				MEMORY_CARD_OPERATION = 0;
				break;
			case 1:
			case 2:
				setMemoryCardReadError(status, 0);
				break;
			default:
				setMemoryCardReadError(0, 0);
				break;
			}
		}
		break;

	case 3:
#if !VERSION_IS(JP) && !VERSION_IS(US)
		if (MAIN_MENU_TICKS >= 3) {
#elif VERSION_IS(US)
		if (((CHANGED_INPUT != CANCEL_BUTTON) && (CHANGED_INPUT != CONFIRM_BUTTON)) ||
		    (MAIN_MENU_TICKS >= 3)) {
#endif
			lo = CHANGED_INPUT & 0xFFFF;
			hi = (uint32_t)(CHANGED_INPUT & 0xFFFF0000) >> 16;
			combinedInput = lo | hi;
			if ((combinedInput == CANCEL_BUTTON) || (combinedInput == CONFIRM_BUTTON)) {
				playSound(0, 3);
				TARGET_MENU = MEMORY_CARD_RETURN_MENU;
			}
			if (MAIN_MENU_TICKS >= 0x12D) {
				TARGET_MENU = MEMORY_CARD_RETURN_MENU;
			}
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
		}
#endif
		break;

	case 9:
		if (MAIN_MENU_TICKS >= 3) {
			TARGET_MENU = -1;
		}
		break;

	case 0xA:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[1];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			switch (cursor->pos) {
			case 0:
				if (CONNECTED_CARDS & 1) {
					MEMORY_CARD_ID = 0;
					TARGET_MENU = 0xC;
					MEMORY_CARD_OPERATION = 0;
				}
				break;
			case 1:
				if (CONNECTED_CARDS & 0x10) {
					MEMORY_CARD_ID = 0x10;
					TARGET_MENU = 0xC;
					MEMORY_CARD_OPERATION = 0;
				}
				break;
			case 2:
				TARGET_MENU = 0x13;
				break;
			}
			break;
		case 2:
			TARGET_MENU = 0;
			break;
		}
		break;

	case 0xC:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardAccept(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, 0);
				break;
			}
			MemCardSync(0, &command, &result);
			result = isMemcardUnformatted(MEMORY_CARD_ID, result);
			switch (result) {
			case 0:
				TARGET_MENU = 0xD;
				break;
			case 4:
				TARGET_MENU = 1;
				MEMORY_CARD_RETURN_MENU = 0xC;
				break;
			default:
				setMemoryCardReadError(result, 0);
				break;
			}
		}
		break;

	case 0xD:
		TARGET_MENU = 0xE;
		if ((MEMORY_CARD_USED_BLOCKS = getCardUsedBlockCount(MEMORY_CARD_ID, 0)) != -1) {
			if (MEMORY_CARD_USED_BLOCKS >= 0xF) {
				setMemoryCardReadError(7, 0);
			} else {
				SAVE_FILE_NAME[0xF] = 0x3F;
				loadSaveSlotData(MEMORY_CARD_ID, SAVE_FILE_NAME,
						 SAVEGAME_SLOT_INFO, 0);
			}
		}
		break;

	case 0xE:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[7];
		input = tickSelectSlotInput(cursor, 1);
		slot = cursor->pos + cursor->scroll;
		if (input != 2) {
			if (input == 1) {
				if (SAVEGAME_SLOT_INFO[slot].valid == 0) {
					TARGET_MENU = 0xF;
					MEMORY_CARD_SLOT = slot;
				}
			}
		} else {
			TARGET_MENU = 0xA;
		}
		TARGET_MENU = getMenuOnCardChange(0);
		break;

	case 0xF:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[8];
		input = tickMenuInput(cursor, 1);
		if (input != 2) {
			if (input == 1) {
				if (cursor->pos == 0) {
					TARGET_MENU = 0x10;
					MEMORY_CARD_OPERATION = 1;
				} else {
					goto cancel_create;
				}
			}
		} else {
	cancel_create:
			TARGET_MENU = 0xE;
		}
		TARGET_MENU = getMenuOnCardChange(0);
		break;

	case 0x10:
		if ((MAIN_MENU_TICKS >= 3) && MemCardSync(1, &command, &result)) {
			SAVE_FILE_NAME[0xF] = HEX_DIGITS[MEMORY_CARD_SLOT];
			/* Horizon state always needs both blocks. */
			SAVE_FILE_BYTES = SAVE_V2_BYTES;
			status = MemCardCreateFile(MEMORY_CARD_ID, SAVE_FILE_NAME, SAVE_V2_BYTES / SAVE_V1_BYTES);
			switch (status) {
			case 0:
				TARGET_MENU = 0x11;
				break;
			case 7:
				setMemoryCardReadError(7, 0xA);
				break;
			case 4:
				TARGET_MENU = 1;
				MEMORY_CARD_RETURN_MENU = 0xC;
				break;
			default:
				setMemoryCardReadError(status, 0);
				break;
			}
		}
		break;

	case 0x11:
		MemCardSync(0, &command, &result);
		SAVE_FILE.id[0] = 0x53;
		SAVE_FILE.id[1] = 0x43;
		SAVE_FILE.iconDisplayFlag = 0x13;
		SAVE_FILE.blockNumber = 1;
		memcpy(SAVE_FILE.iconClut, SAVE_ICON_CLUT, 0x20);
		memcpy(SAVE_FILE.iconFrames[0], SAVE_ICON_FRAME_0, 0x80);
		memcpy(SAVE_FILE.iconFrames[1], SAVE_ICON_FRAME_1, 0x80);
		memcpy(SAVE_FILE.iconFrames[2], SAVE_ICON_FRAME_2, 0x80);
		if (MOD_CHEATS_CARD_MODE == 1) writeSavegame(&SAVE_FILE.saves[0]);
		else initializeDefaultSavegame();
#if !VERSION_IS(US)
		strncpy(SAVE_FILE.title, MAIN_D_8013392C, 0x40);
		_strncpy(SAVE_FILE.title + 8, SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 4);
		/* MOD: _strncpy does not terminate: clear old name bytes (original showed "Bayer" for "B" after "Player") */
		memset(SAVE_FILE.title + 0xe, 0, 0xe);
		_strncpy(SAVE_FILE.title + 0xe, SAVE_FILE.saves[0].playerName, 0xc);
		memset(SAVE_FILE.title + 0x1c, 0, 0x12);
		_strncpy(SAVE_FILE.title + 0x1c, DIGIMON_NAME(SAVE_FILE.saves[0].partnerType), 0x10);
		_strncpy(SAVE_FILE.title + 0x2e,
			 MAP_NAME_PTR[MAP_ENTRIES[SAVE_FILE.saves[0].currentScreen].loadingName], 0x12);
#else
		modTitleNames();
#endif
		prepareSaveFile(MOD_CHEATS_CARD_MODE == 1); /* MOD (save v2): checksums, extension, mirror */
		status = writeSaveFile();
		if (status != 1) {
			setMemoryCardReadError(0, 0);
		}
		saveFileSync(&command, &result); /* MOD (save extension area) */
		if (result == 0) {
			TARGET_MENU = 9;
			if (MOD_CHEATS_CARD_MODE == 1) MOD_CHEATS_CARD_DONE = 1;
			else { loadSaveCopy(0); MAIN_STATE = 0; }
		} else {
			setMemoryCardReadError(result, 0);
		}
		break;

	case 0x13:
		view = MENU_VIEWS[CURRENT_MENU];
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[view];
		input = tickMenuInput(cursor, 1);
		if (input != 2) {
			if (input == 1) {
				if (cursor->pos == 0) {
					TARGET_MENU = 9;
					MEMORY_CARD_ID = -1;
					MEMORY_CARD_SLOT = -1;
					initializeDefaultSavegame();
					prepareSaveFile(0); /* MOD (save v2) */
					loadSaveCopy(0);
				} else {
					TARGET_MENU = 0xA;
				}
			}
		} else {
			TARGET_MENU = 0xA;
		}
		break;

	case 0x14:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[2];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			switch (cursor->pos) {
			case 0:
				if (CONNECTED_CARDS & 1) {
					MEMORY_CARD_ID = 0;
					TARGET_MENU = 0x15;
					MEMORY_CARD_OPERATION = 0;
				}
				break;
			case 1:
				if (CONNECTED_CARDS & 0x10) {
					MEMORY_CARD_ID = 0x10;
					TARGET_MENU = 0x15;
					MEMORY_CARD_OPERATION = 0;
				}
				break;
			}
			break;
		case 2:
			TARGET_MENU = 0;
			break;
		}
		break;

	case 0x15:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardAccept(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, 0);
				break;
			}
			MemCardSync(0, &command, &result);
			result = isMemcardUnformatted(MEMORY_CARD_ID, result);
			if (result == 0) {
				TARGET_MENU = 0x16;
			} else {
				setMemoryCardReadError(result, 0);
			}
		}
		break;

	case 0x16:
		TARGET_MENU = 0x17;
		SAVE_FILE_NAME[0xF] = 0x3F;
		loadSaveSlotData(MEMORY_CARD_ID, SAVE_FILE_NAME,
				 SAVEGAME_SLOT_INFO, 0);
		break;

	case 0x17:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[7];
		input = tickSelectSlotInput(cursor, 1);
		slot = cursor->pos + cursor->scroll;
		if (input != 2) {
			if (input == 1) {
				if (SAVEGAME_SLOT_INFO[slot].valid == 1) {
					TARGET_MENU = 0x18;
					MEMORY_CARD_SLOT = slot;
				}
			}
		} else {
			TARGET_MENU = 0x14;
		}
		TARGET_MENU = getMenuOnCardChange(0);
		break;

	case 0x18:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[8];
		input = tickMenuInput(cursor, 1);
		if (input != 2) {
			if (input == 1) {
				if (cursor->pos == 0) {
					TARGET_MENU = 0x19;
					MEMORY_CARD_OPERATION = 2;
				} else {
					goto cancel_load;
				}
			}
		} else {
	cancel_load:
			TARGET_MENU = 0x17;
		}
		TARGET_MENU = getMenuOnCardChange(0);
		break;

	case 0x19:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			SAVE_FILE_NAME[0xF] = HEX_DIGITS[MEMORY_CARD_SLOT];
			/* MOD (save v2): a two-block file is read whole */
			SAVE_FILE_BYTES = getSaveFileBytes();
			status = saveFileStart(0); /* MOD (save extension area): the v1 file, then the extensions */
			if (status != 1) {
				setMemoryCardReadError(0, 0);
			}
			saveFileSync(&command, &result);
			if (result == 0) {
				/* MOD (save v2): loadSaveCopy also takes the extension, or
				 * refuses a newer format ("Data Error"); MOD (save robustness):
				 * the copy whose extension links first (pickSaveCopy) */
				slot = pickSaveCopy();
				if (slot >= 0 && loadSaveCopy(slot)) {
					if (MOD_CHEATS_CARD_MODE == 2) MOD_CHEATS_CARD_DONE = 2;
					TARGET_MENU = 9;
					break;
				}
				result = 2;
			}
			setMemoryCardReadError(result, 0);
		}
		break;

	case 0x1E:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[4];
		input = tickMenuInput(cursor, 1);
		if (input != 2) {
			if (input == 1) {
				switch (cursor->pos) {
				case 0:
					if (CONNECTED_CARDS & 1) {
						MEMORY_CARD_ID = 0;
						TARGET_MENU = 0x1F;
						MEMORY_CARD_OPERATION = 0;
					}
					break;
				case 1:
					if (CONNECTED_CARDS & 0x10) {
						MEMORY_CARD_ID = 0x10;
						TARGET_MENU = 0x1F;
						MEMORY_CARD_OPERATION = 0;
					}
					break;
				}
			}
		} else {
			TARGET_MENU = 0;
		}
		if (CONNECTED_CARDS == 0) {
			TARGET_MENU = 0;
		}
		break;

	case 0x1F:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardAccept(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, 0);
				break;
			}
			MemCardSync(0, &command, &result);
			result = isMemcardUnformatted(MEMORY_CARD_ID, result);
			if (result == 0) {
				TARGET_MENU = 0x20;
			} else {
				setMemoryCardReadError(result, 0);
			}
		}
		break;

	case 0x20:
		TARGET_MENU = 0x21;
		SAVE_FILE_NAME[0xF] = 0x3F;
		loadSaveSlotData(MEMORY_CARD_ID, SAVE_FILE_NAME,
				 SAVEGAME_SLOT_INFO, 0);
		break;

	case 0x21:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[7];
		input = tickSelectSlotInput(cursor, 1);
		slot = cursor->pos + cursor->scroll;
		if (input != 2) {
			if (input == 1) {
				if (SAVEGAME_SLOT_INFO[slot].valid == 1) {
					TARGET_MENU = 0x22;
					MEMORY_CARD_SLOT = slot;
				}
			}
		} else {
			TARGET_MENU = 0x1E;
		}
		TARGET_MENU = getMenuOnCardChange(0);
		break;

	case 0x22:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[8];
		input = tickMenuInput(cursor, 1);
		if (input != 2) {
			if (input == 1) {
				if (cursor->pos == 0) {
					TARGET_MENU = 0x23;
					MEMORY_CARD_OPERATION = 4;
				} else {
					goto cancel_delete;
				}
			}
		} else {
	cancel_delete:
			TARGET_MENU = 0x21;
		}
		TARGET_MENU = getMenuOnCardChange(0);
		break;

	case 0x23:
		if ((MAIN_MENU_TICKS >= 3) && MemCardSync(1, &command, &result)) {
			SAVE_FILE_NAME[0xF] = HEX_DIGITS[MEMORY_CARD_SLOT];
			status = MemCardDeleteFile(MEMORY_CARD_ID, SAVE_FILE_NAME);
			if (status == 0) {
				TARGET_MENU = 0x1F;
				MEMORY_CARD_OPERATION = 0;
			} else {
				setMemoryCardReadError(status, 0);
			}
		}
		break;

	case 0x28:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[11];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				TARGET_MENU = 9;
				MAIN_STATE = 0;
			} else {
				SAVE_RETRY_RETURN_MENU = 0x28;
				TARGET_MENU = 0x29;
				MEMORY_CARD_OPERATION = 0;
			}
			break;
		case 2:
			break;
		}
		break;

	case 0x29:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardExist(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, 0x28);
				break;
			}
			MemCardSync(0, &command, &result);
			switch (result) {
			case 0:
				TARGET_MENU = 0x2A;
				break;
			case 3:
				MEMORY_CARD_ERROR = -1;
				TARGET_MENU = 0x30;
				break;
			case 1:
			case 2:
			case 4:
				MEMORY_CARD_ERROR = result;
				TARGET_MENU = 0x30;
				break;
			default:
				setMemoryCardReadError(0, 0x28);
				break;
			}
		}
		break;

	case 0x2A:
		MemCardSync(0, &command, &result);
		SAVE_FILE_NAME[0xF] = HEX_DIGITS[MEMORY_CARD_SLOT];
		status = MemCardGetDirentry(MEMORY_CARD_ID, SAVE_FILE_NAME,
					MEMCARD_DIRENTRIES, &count, 0, 1);
		MEMORY_CARD_ERROR = -1;
		switch (status) {
		case -1:
			setMemoryCardReadError(0, 0x28);
			break;
		case 0:
			/* MOD (save v2): one or two blocks */
			SAVE_FILE_BYTES = MEMCARD_DIRENTRIES[0].size >= SAVE_V2_BYTES ? SAVE_V2_BYTES : SAVE_V1_BYTES;
			if (count == 0) {
				MEMORY_CARD_ERROR = 5;
				TARGET_MENU = 0x30;
			} else {
				TARGET_MENU = 0x2B;
				MEMORY_CARD_OPERATION = 1;
			}
			break;
		default:
			MEMORY_CARD_ERROR = status;
			TARGET_MENU = 0x30;
			break;
		}
		break;

	case 0x2B:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			SAVE_FILE.id[0] = 0x53;
			SAVE_FILE.id[1] = 0x43;
			SAVE_FILE.iconDisplayFlag = 0x13;
			SAVE_FILE.blockNumber = 1;
			memcpy(SAVE_FILE.iconClut, SAVE_ICON_CLUT, 0x20);
			memcpy(SAVE_FILE.iconFrames[0], SAVE_ICON_FRAME_0, 0x80);
			memcpy(SAVE_FILE.iconFrames[1], SAVE_ICON_FRAME_1, 0x80);
			memcpy(SAVE_FILE.iconFrames[2], SAVE_ICON_FRAME_2, 0x80);
			writeSavegame(&SAVE_FILE.saves[0]);
#if !VERSION_IS(US)
			strncpy(SAVE_FILE.title, MAIN_D_8013392C, 0x40);
			_strncpy(SAVE_FILE.title + 8, SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 4);
			/* MOD: _strncpy does not terminate: clear old name bytes (original showed "Bayer" for "B" after "Player") */
			memset(SAVE_FILE.title + 0xe, 0, 0xe);
			_strncpy(SAVE_FILE.title + 0xe, SAVE_FILE.saves[0].playerName, 0xc);
			memset(SAVE_FILE.title + 0x1c, 0, 0x12);
			_strncpy(SAVE_FILE.title + 0x1c, DIGIMON_NAME(SAVE_FILE.saves[0].partnerType), 0x10);
			_strncpy(SAVE_FILE.title + 0x2e,
				 MAP_NAME_PTR[MAP_ENTRIES[SAVE_FILE.saves[0].currentScreen].loadingName], 0x12);
#else
		modTitleNames();
#endif
			prepareSaveFile(1); /* MOD (save v2): checksums, extension, mirror */
			status = writeSaveFile();
			if (status != 1) {
				setMemoryCardReadError(0, 0x28);
			}
			saveFileSync(&command, &result); /* MOD (save extension area) */
			if (result == 0) {
				if (MOD_CHEATS_CARD_MODE == 1) MOD_CHEATS_CARD_DONE = 1;
				TARGET_MENU = 9;
			} else {
				setMemoryCardReadError(result, 0x28);
			}
		}
		break;

	case 0x30:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[12];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				if (CONNECTED_CARDS & (MEMORY_CARD_ID == 0 ? 1 : 0x10)) {
					TARGET_MENU = 0x31;
					MEMORY_CARD_OPERATION = 0;
				}
			} else {
				TARGET_MENU = 0x28;
			}
			break;
		case 2:
			break;
		}
		break;

	case 0x31:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardAccept(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, 0x28);
				break;
			}
			MemCardSync(0, &command, &result);
			result = isMemcardUnformatted(MEMORY_CARD_ID, result);
			switch (result) {
			case 0:
				CONNECTED_CARDS |= MEMORY_CARD_ID == 0x10 ? 0x10 : 1;
				TARGET_MENU = 0x2A;
				break;
			case 1:
				CONNECTED_CARDS &= MEMORY_CARD_ID == 0x10 ? -0x11 : -2;
				MEMORY_CARD_ERROR = 1;
				TARGET_MENU = 0x30;
				break;
			default:
				MEMORY_CARD_ERROR = result;
				TARGET_MENU = 0x30;
				break;
			}
		}
		break;

	case 0x32:
		if (VS_PLAYER_INDEX == 0) {
			MEMORY_CARD_ID = 0;
		} else {
			MEMORY_CARD_ID = 0x10;
		}
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[13];
		input = tickMenuInput(cursor,
				      VS_PLAYER_INDEX == 0 ? 1 : 2);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				if (CONNECTED_CARDS & (MEMORY_CARD_ID == 0 ? 1 : 0x10)) {
					TARGET_MENU = 0x33;
					MEMORY_CARD_OPERATION = 0;
				}
			} else {
				TARGET_MENU = VS_RETURN_MENU;
			}
			break;
		case 2:
			TARGET_MENU = VS_RETURN_MENU;
			break;
		}
		break;

	case 0x33:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardAccept(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, 0x32);
				break;
			}
			MemCardSync(0, &command, &result);
			result = isMemcardUnformatted(MEMORY_CARD_ID, result);
			if (result == 0) {
				TARGET_MENU = 0x34;
			} else {
				setMemoryCardReadError(result, 0x32);
				break;
			}
			SAVE_FILE_NAME[0xF] = 0x3F;
			if (loadSaveSlotData(MEMORY_CARD_ID, SAVE_FILE_NAME,
					     SAVEGAME_SLOT_INFO,
					     0x32) != 0) {
				TARGET_MENU = 0x34;
			}
		}
		break;

	case 0x34:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[7];
		input = tickSelectSlotInput(cursor,
					    VS_PLAYER_INDEX == 0 ? 1 : 2);
		slot = cursor->pos + cursor->scroll;
		if (input != 2) {
			if (input == 1) {
				if (SAVEGAME_SLOT_INFO[slot].valid == 1) {
					TARGET_MENU = 0x35;
					MEMORY_CARD_SLOT = slot;
				}
			}
		} else {
			TARGET_MENU = 0x32;
		}
		TARGET_MENU = getMenuOnCardChange(0x32);
		break;

	case 0x35:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[8];
		input = tickMenuInput(cursor,
				      VS_PLAYER_INDEX == 0 ? 1 : 2);
		if (input != 2) {
			if (input == 1) {
				if (cursor->pos == 0) {
					TARGET_MENU = 0x36;
					MEMORY_CARD_OPERATION = 2;
				} else {
					goto cancel_vs_load;
				}
			}
		} else {
	cancel_vs_load:
			TARGET_MENU = 0x34;
		}
		TARGET_MENU = getMenuOnCardChange(0x32);
		break;

	case 0x36:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			SAVE_FILE_NAME[0xF] = HEX_DIGITS[MEMORY_CARD_SLOT];
			SAVE_FILE_BYTES = getSaveFileBytes(); /* MOD (save v2) */
			status = saveFileStart(0); /* MOD (save extension area): the v1 file, then the extensions */
			if (status != 1) {
				setMemoryCardReadError(0, 0x32);
			}
			saveFileSync(&command, &result);
			if (result == 0) {
				/* MOD (save v2): a refusing v2 save needs its extension to be checked;
				 * MOD (save robustness): the copy whose extension links first (pickSaveCopy) */
				slot = pickSaveCopy();
				if (slot < 0) {
					setMemoryCardReadError(2, 0x32);
					break;
				}
				memcpy(&VS__REGISTERED_DIGIMON_BUFFER[VS_PLAYER_INDEX * 0x28],
				       SAVE_FILE.saves[slot].battleRegistrationData, 0xA00);
				dropAbsentRegistered(&VS__REGISTERED_DIGIMON_BUFFER[VS_PLAYER_INDEX * 0x28], 40);
				if (countRegisteredDigimon(&VS__REGISTERED_DIGIMON_BUFFER[VS_PLAYER_INDEX * 0x28])) {
					TARGET_MENU = 0x37;
				} else {
					setMemoryCardReadError(8, 0x32);
				}
			} else {
				setMemoryCardReadError(result, 0x32);
			}
		}
		break;

	case 0x37:
		if (VS_PLAYER_INDEX == 0) {
			VS_PLAYER_INDEX = 1;
			TARGET_MENU = 0x32;
		} else {
			TARGET_MENU = 0x38;
		}
		break;

	case 0x38:
		if (MAIN_MENU_TICKS >= 3) {
			modMenuVsBattle(0); /* MOD (menu overlay): VS_REL is read over this code */
			TARGET_MENU = VS_RETURN_MENU;
		}
		break;

	case 0x3C:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[14];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				TARGET_MENU = 0x3D;
				SAVE_FILE.saves[0] =
					SAVE_FILE.saves[1];
				SAVE_RETRY_RETURN_MENU = 0x3C;
				break;
			}
			/* fall through */
		case 2:
			TARGET_MENU = 0x45;
			break;
		}
		break;

	case 0x3D:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[15];
		oldScroll = cursor->scroll;
		input = tickMenuInput(cursor, 1);
		if (oldScroll != cursor->scroll) {
			drawRegisteredDigimonSlots(cursor->scroll);
		}
		if (input != 2) {
			if (input == 1) {
				BATTLE_REGISTRATION_SLOT = cursor->scroll + cursor->pos;
				TARGET_MENU = 0x3E;
			}
		} else {
			TARGET_MENU = 0x3C;
		}
		break;

	case 0x3E:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[16];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				TARGET_MENU = 0x3F;
				registerBattleData();
				MEMORY_CARD_OPERATION = 0;
			} else {
				TARGET_MENU = 0x3D;
			}
			break;
		case 2:
			TARGET_MENU = 0x3C;
			break;
		}
		break;

	case 0x3F:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardExist(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, SAVE_RETRY_RETURN_MENU);
				break;
			}
			MemCardSync(0, &command, &result);
			switch (result) {
			case 0:
				TARGET_MENU = 0x40;
				break;
			case 3:
				MEMORY_CARD_ERROR = -1;
				TARGET_MENU = 0x42;
				break;
			case 1:
			case 2:
			case 4:
				MEMORY_CARD_ERROR = result;
				TARGET_MENU = 0x42;
				break;
			default:
				setMemoryCardReadError(0, SAVE_RETRY_RETURN_MENU);
				break;
			}
		}
		break;

	case 0x40:
		MemCardSync(0, &command, &result);
		SAVE_FILE_NAME[0xF] = HEX_DIGITS[MEMORY_CARD_SLOT];
		status = MemCardGetDirentry(MEMORY_CARD_ID, SAVE_FILE_NAME,
					MEMCARD_DIRENTRIES, &count, 0, 1);
		MEMORY_CARD_ERROR = -1;
		switch (status) {
		case -1:
			setMemoryCardReadError(0, SAVE_RETRY_RETURN_MENU);
			break;
		case 0:
			/* MOD (save v2): one or two blocks */
			SAVE_FILE_BYTES = MEMCARD_DIRENTRIES[0].size >= SAVE_V2_BYTES ? SAVE_V2_BYTES : SAVE_V1_BYTES;
			if (count == 0) {
				MEMORY_CARD_ERROR = 5;
				TARGET_MENU = 0x42;
			} else {
				TARGET_MENU = 0x41;
				MEMORY_CARD_OPERATION = 1;
			}
			break;
		default:
			MEMORY_CARD_ERROR = status;
			TARGET_MENU = 0x42;
			break;
		}
		break;

	case 0x41:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			SAVE_FILE.id[0] = 0x53;
			SAVE_FILE.id[1] = 0x43;
			SAVE_FILE.iconDisplayFlag = 0x13;
			SAVE_FILE.blockNumber = 1;
			memcpy(SAVE_FILE.iconClut, SAVE_ICON_CLUT, 0x20);
			memcpy(SAVE_FILE.iconFrames[0], SAVE_ICON_FRAME_0, 0x80);
			memcpy(SAVE_FILE.iconFrames[1], SAVE_ICON_FRAME_1, 0x80);
			memcpy(SAVE_FILE.iconFrames[2], SAVE_ICON_FRAME_2, 0x80);
			writeSavegame(&SAVE_FILE.saves[0]);
#if !VERSION_IS(US)
			strncpy(SAVE_FILE.title, MAIN_D_8013392C, 0x40);
			_strncpy(SAVE_FILE.title + 8, SAVEGAME_ID_LABEL[MEMORY_CARD_SLOT + 1], 4);
			/* MOD: _strncpy does not terminate: clear old name bytes (original showed "Bayer" for "B" after "Player") */
			memset(SAVE_FILE.title + 0xe, 0, 0xe);
			_strncpy(SAVE_FILE.title + 0xe, SAVE_FILE.saves[0].playerName, 0xc);
			memset(SAVE_FILE.title + 0x1c, 0, 0x12);
			_strncpy(SAVE_FILE.title + 0x1c, DIGIMON_NAME(SAVE_FILE.saves[0].partnerType), 0x10);
			_strncpy(SAVE_FILE.title + 0x2e,
				 MAP_NAME_PTR[MAP_ENTRIES[SAVE_FILE.saves[0].currentScreen].loadingName], 0x12);
#else
		modTitleNames();
#endif
			prepareSaveFile(1); /* MOD (save v2): checksums, extension, mirror */
			status = writeSaveFile();
			if (status != 1) {
				setMemoryCardReadError(0, SAVE_RETRY_RETURN_MENU);
			}
			saveFileSync(&command, &result); /* MOD (save extension area) */
			if (result == 0) {
				TARGET_MENU = 9;
			} else {
				setMemoryCardReadError(result, SAVE_RETRY_RETURN_MENU);
			}
		}
		break;

	case 0x42:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[12];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				if (CONNECTED_CARDS & (MEMORY_CARD_ID == 0 ? 1 : 0x10)) {
					TARGET_MENU = 0x43;
					MEMORY_CARD_OPERATION = 0;
				}
			} else {
				TARGET_MENU = SAVE_RETRY_RETURN_MENU;
			}
			break;
		case 2:
			break;
		}
		break;

	case 0x43:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardAccept(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, SAVE_RETRY_RETURN_MENU);
				break;
			}
			MemCardSync(0, &command, &result);
			result = isMemcardUnformatted(MEMORY_CARD_ID, result);
			switch (result) {
			case 0:
				CONNECTED_CARDS |= MEMORY_CARD_ID == 0x10 ? 0x10 : 1;
				TARGET_MENU = 0x40;
				break;
			case 1:
				CONNECTED_CARDS &= MEMORY_CARD_ID == 0x10 ? -0x11 : -2;
				MEMORY_CARD_ERROR = 1;
				TARGET_MENU = 0x42;
				break;
			default:
				MEMORY_CARD_ERROR = result;
				TARGET_MENU = 0x42;
				break;
			}
		}
		break;

	case 0x44:
		if ((CHANGED_INPUT == CANCEL_BUTTON) || (CHANGED_INPUT == CONFIRM_BUTTON)) {
			playSound(0, 3);
			TARGET_MENU = 9;
		}
		if (MAIN_MENU_TICKS >= 0x79) {
			TARGET_MENU = 9;
		}
		break;

	case 0x45:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[18];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			if (cursor->pos == 0) {
				TARGET_MENU = 0x3F;
				SAVE_RETRY_RETURN_MENU = 0x45;
				MEMORY_CARD_OPERATION = 0;
			} else {
				TARGET_MENU = 9;
			}
			break;
		case 2:
			break;
		}
		break;

	case 0x46:
		if ((CHANGED_INPUT == CANCEL_BUTTON) || (CHANGED_INPUT == CONFIRM_BUTTON)) {
			playSound(0, 3);
			TARGET_MENU = 0x45;
		}
		break;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)

	case 0x47:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[20];
		if (SECRET_CODE != -1) {
			cursor->count = 4;
		} else {
			cursor->count = 3;
		}
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			switch (cursor->pos) {
			case 0:
				TARGET_MENU = 9;
				MEMORY_CARD_ID = -1;
				MEMORY_CARD_SLOT = -1;
				initializeDefaultSavegame();
				/* No options record (original cards too): explicit upgrade defaults. */
				modApplyOptions(NULL, 0);
				loadSavegame(&SAVE_FILE.saves[0]);
				break;
			case 1:
				VS_PLAYER_INDEX = 0;
				TARGET_MENU = 0x32;
				MAIN_MENU_ACTION = 5;
				break;
			case 2:
				TARGET_MENU = 0x48;
				MAIN_MENU_ACTION = 5;
				break;
			case 3:
				TARGET_MENU = 0x49;
				MEMORY_CARD_ID = 0;
				MAIN_MENU_ACTION = 7;
				break;
			}
			break;
		case 2:
			break;
		}
		break;

	case 0x48:
		if (MAIN_MENU_TICKS >= 3) {
			modMenuVsBattle(1); /* MOD (menu overlay): VS_REL is read over this code */
			TARGET_MENU = 0x47;
		}
		break;

	case 0x49:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[21];
		input = tickMenuInput(cursor, 1);
		switch (input) {
		case 1:
			switch (cursor->pos) {
			case 0:
				if (CONNECTED_CARDS & 1) {
					TARGET_MENU = 0x4A;
					MEMORY_CARD_OPERATION = 0;
				}
				break;
			case 1:
				TARGET_MENU = 0x47;
				break;
			}
			break;
		case 2:
			TARGET_MENU = 0x47;
			break;
		}
		break;

	case 0x4A:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			if (MemCardAccept(MEMORY_CARD_ID) == 0) {
				setMemoryCardReadError(0, 0x47);
				break;
			}
			MemCardSync(0, &command, &result);
			result = isMemcardUnformatted(MEMORY_CARD_ID, result);
			if (result == 0) {
				TARGET_MENU = 0x4B;
			} else {
				setMemoryCardReadError(result, 0x47);
			}
		}
		break;

	case 0x4B:
		TARGET_MENU = 0x4C;
		SAVE_FILE_NAME[0xF] = 0x3F;
		loadSaveSlotData(MEMORY_CARD_ID, SAVE_FILE_NAME,
				 SAVEGAME_SLOT_INFO, 0x47);
		break;

	case 0x4C:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[7];
		input = tickSelectSlotInput(cursor, 1);
		slot = cursor->pos + cursor->scroll;
		if (input != 2) {
			if (input == 1) {
				if (SAVEGAME_SLOT_INFO[slot].valid == 1) {
					TARGET_MENU = 0x4D;
					MEMORY_CARD_SLOT = slot;
				}
			}
		} else {
			TARGET_MENU = 0x49;
		}
		TARGET_MENU = getMenuOnCardChange(0x47);
		break;

	case 0x4D:
		cursor = (MenuCursor *)&MENU_HIGHLIGHTS[8];
		input = tickMenuInput(cursor, 1);
		if (input != 2) {
			if (input == 1) {
				if (cursor->pos == 0) {
					TARGET_MENU = 0x4E;
					MEMORY_CARD_OPERATION = 2;
				} else {
					goto cancel_add_item;
				}
			}
		} else {
	cancel_add_item:
			TARGET_MENU = 0x4C;
		}
		TARGET_MENU = getMenuOnCardChange(0x47);
		break;

	case 0x4E:
		if (MAIN_MENU_TICKS >= 3) {
			MemCardSync(0, &command, &result);
			SAVE_FILE_NAME[0xF] = HEX_DIGITS[MEMORY_CARD_SLOT];
			status = MemCardReadFile(MEMORY_CARD_ID, SAVE_FILE_NAME,
					    (void *)&SAVE_FILE, 0, 0x2000);
			if (status != 1) {
				setMemoryCardReadError(0, 0x47);
			}
			MemCardSync(0, &command, &result);
			if (result == 0) {
				if (SAVE_FILE.saves[0].checksum ==
				    createSavegameChecksum(0)) {
					/* No options record (original cards too): explicit upgrade defaults. */
					modApplyOptions(NULL, 0);
					loadSavegame(&SAVE_FILE.saves[0]);
					TARGET_MENU = 0x4F;
					MEMORY_CARD_OPERATION = 1;
					break;
				}
				if (SAVE_FILE.saves[1].checksum ==
				    createSavegameChecksum(1)) {
					SAVE_FILE.saves[0] =
						SAVE_FILE.saves[1];
					/* No options record (original cards too): explicit upgrade defaults. */
					modApplyOptions(NULL, 0);
					loadSavegame(&SAVE_FILE.saves[0]);
					TARGET_MENU = 0x4F;
					MEMORY_CARD_OPERATION = 1;
					break;
				}
				result = 2;
			}
			setMemoryCardReadError(result, 0x47);
		}
		break;

	case 0x4F:
		if (MAIN_MENU_TICKS >= 3) {
			inventory = &SAVE_FILE.saves[0].inventory;
			for (i = 0; i < inventory->size; i++) {
				if (inventory->types.array[i] == 0xff) {
					inventory->amounts.array[i] = 1;
					inventory->types.array[i] = SECRET_CODE + SECRET_CODE_ITEM;
					i = 0x63;
				}
				if ((SECRET_CODE + SECRET_CODE_ITEM) == inventory->types.array[i]) {
					i = 0x64;
				}
			}
			if (i < 0x64) {
				setMemoryCardReadError(9, 0x47);
				break;
			}
			if (i == 0x65) {
				setMemoryCardReadError(0xa, 0x47);
				break;
			}
			MemCardSync(0, &command, &result);
			SAVE_FILE.saves[0].checksum = createSavegameChecksum(0);
			SAVE_FILE.saves[1] = SAVE_FILE.saves[0];
			status = MemCardWriteFile(MEMORY_CARD_ID, SAVE_FILE_NAME,
						  (void *)&SAVE_FILE, 0, 0x2000);
			if (status != 1) {
				setMemoryCardReadError(0, 0x47);
			}
			MemCardSync(0, &command, &result);
			if (result == 0) {
				TARGET_MENU = 0x47;
				break;
			}
			setMemoryCardReadError(status, 0x47);
		}
		break;
#endif
	}
}

int32_t tickMenuInput(MenuCursor *cursor, int32_t which)
{
	int32_t input;
	int32_t lo;
	int32_t hi;

	lo = CHANGED_INPUT & 0xFFFF;
	hi = (uint32_t)(CHANGED_INPUT & 0xFFFF0000) >> 0x10;
	input = 0;
	if (which & 1) {
		input |= lo;
	}
	if (which & 2) {
		input |= hi;
	}
	if (input == 0x1000) {
		if (0 < cursor->pos) {
			playSound(0, 2);
			cursor->pos--;
		} else if (0 < cursor->scroll) {
			playSound(0, 2);
			cursor->scroll--;
		}
	}
	if (input == 0x4000) {
		if (cursor->pos < cursor->count - 1) {
			playSound(0, 2);
			cursor->pos++;
		} else if (cursor->scroll < cursor->max) {
			playSound(0, 2);
			cursor->scroll++;
		}
	}
	if (input == CANCEL_BUTTON) {
		playSound(0, 4);
		return 2;
	}
	if (input == CONFIRM_BUTTON) {
		playSound(0, 3);
		return 1;
	}
	return 0;
}

int32_t getMenuOnCardChange(int32_t menu)
{
	int32_t mask;
	int32_t mask2;

	if (MEMORY_CARD_ID == 0) {
		mask = 1;
	} else {
		mask = 0x10;
	}
	if (!(CONNECTED_CARDS & mask)) {
		return menu;
	}
	if (MEMORY_CARD_ID == 0) {
		mask2 = 1;
	} else {
		mask2 = 0x10;
	}
	if (NEW_CARDS & mask2) {
		return menu;
	}
	return TARGET_MENU;
}

void setMemoryCardReadError(int32_t id, int32_t returnMenu)
{
	TARGET_MENU = 3;
	MEMORY_CARD_ERROR = id;
	MEMORY_CARD_RETURN_MENU = returnMenu;
}

int32_t isMemcardUnformatted(int32_t chan, MemCardSyncWord mode)
{
	MemCardSyncWord cmd;
	MemCardSyncWord result;

	if ((mode == 0) || (mode == 3)) {
		return 0;
	}

	MemCardExist(chan);
	MemCardSync(0, &cmd, &result);
	if ((result == 0) && (mode == 4)) {
		result = 4;
	}

	return result;
}

int32_t getCardUsedBlockCount(int32_t channel, int32_t returnMenu)
{
	MemCardSyncWord cmd;
	MemCardSyncWord result;
	long fileCount;
	int32_t i;
	int32_t blocks;
	int32_t status;

	MemCardSync(0, &cmd, &result);
	status = MemCardGetDirentry(channel, MAIN_D_801346C0[1], MEMCARD_DIRENTRIES,
			&fileCount, 0, 15);
	switch (status) {
	case 0:
	case 3:
		break;
	case -1:
		setMemoryCardReadError(0, returnMenu);
		return -1;
	default:
		setMemoryCardReadError(status, returnMenu);
		return -1;
	}

	blocks = 0;
	for (i = 0; i < fileCount; i++) {
		blocks += (MEMCARD_DIRENTRIES[i].size + 0x1FFF) / 0x2000;
	}
	return blocks;
}

int32_t loadSaveSlotData(int32_t channel, char *filename, SaveSlotPreview *slots, int32_t unused)
{
	MemCardSyncWord cmd;
	MemCardSyncWord result;
	long fileCount;
	int32_t status;
	/* Retail reserves 0x200 bytes even though each header read requests 0x80. */
	uint8_t data[0x200];
	int32_t i;
	int32_t slot;
#if VERSION_IS(US)
	int32_t j; /* MOD (language) */
#endif

	/* The fourth caller argument is unused in retail. */
	MemCardSync(0, &cmd, &result);
	status = MemCardGetDirentry(channel, filename, MEMCARD_DIRENTRIES, &fileCount, 0, 15);
	switch (status) {
	case 0:
	case 3:
		break;
	case -1:
		setMemoryCardReadError(0, 0);
		return 0;
	default:
		setMemoryCardReadError(status, 0);
		return 0;
	}

	for (i = 0; i < 15; i++) {
		slots[i].valid = 0;
	}
	modSaveSlotsLayout(0); /* MOD (long names) */
	for (i = 0; i < fileCount; i++) {
		if ((MEMCARD_DIRENTRIES[i].name[15] >= '0') &&
				(MEMCARD_DIRENTRIES[i].name[15] < ':')) {
			slot = MEMCARD_DIRENTRIES[i].name[15] - '0';
		} else {
			slot = MEMCARD_DIRENTRIES[i].name[15] - '7';
		}
		status = MemCardReadFile(channel, MEMCARD_DIRENTRIES[i].name, (void *)data, 0, 0x80);
		if (status != 1) {
			setMemoryCardReadError(0, 0);
			return 0;
		}
		MemCardSync(0, &cmd, &result);
		if (result == 0) {
			slots[slot].valid = 1;
			if (modHeaderSaveNumber(data) > MOD_SAVE_NUMBER) { /* MOD (options) */
				MOD_SAVE_NUMBER = modHeaderSaveNumber(data);
			}
#if !VERSION_IS(US)
			strncpy(slots[slot].playerName, (char *)&data[0x12], 12);
			slots[slot].playerName[12] = '\0';
			strncpy(slots[slot].digimonName, (char *)&data[0x20], 16);
			slots[slot].digimonName[16] = '\0';
			strncpy(slots[slot].location, (char *)&data[0x32], 18);
			slots[slot].location[18] = '\0';
#else
			strncpy(slots[slot].playerName, (char *)&data[0x12], 13);
			slots[slot].playerName[13] = '\0';
			strncpy(slots[slot].digimonName, (char *)&data[0x20], 17);
			slots[slot].digimonName[17] = '\0';
			/* MOD (long names): the rest of a longer name (modTitleNameTail) */
			if (data[0x44] == 'L' && data[0x45] == 'N' && strlen(slots[slot].digimonName) == 17) {
				strncpy(slots[slot].digimonName + 17, (char *)&data[0x46], 2);
				slots[slot].digimonName[19] = '\0';
			}
			/* MOD (language): the accented letters of the player's name (modTitlePlayerName) */
			if (data[0x49] == 'A') {
				for (j = 0; j < 6; j++) {
					if (data[0x4a + j] != 0 && slots[slot].playerName[j * 2] != '\0') {
						slots[slot].playerName[j * 2] = 0xf0;
						slots[slot].playerName[j * 2 + 1] = data[0x4a + j];
					}
				}
			}
			/* the long layout of the slot screens when a species passes the original column */
			if (measureStringMode(slots[slot].digimonName, FIT_VANILLA) > SLOT_NAME_W) {
				modSaveSlotsLayout(1);
			}
			strncpy(slots[slot].location, (char *)&data[0x32], 24);
			slots[slot].location[23] = '\0';
#endif
		} else {
			setMemoryCardReadError(result, 0);
			return 0;
		}
	}
	return fileCount;
}

int32_t tickSelectSlotInput(MenuCursor *cursor, int32_t which)
{
	int32_t input;
	int32_t lo;
	int32_t hi;

	lo = CHANGED_INPUT & 0xFFFF;
	hi = (uint32_t)(CHANGED_INPUT & 0xFFFF0000) >> 0x10;
	input = 0;
	if (which & 1) {
		input |= lo;
	}
	if (which & 2) {
		input |= hi;
	}
	if (input == 0x1000) {
		if (0 < cursor->pos) {
			playSound(0, 2);
			cursor->pos--;
		} else if (0 < cursor->scroll) {
			playSound(0, 2);
			cursor->scroll--;
			drawSaveSlotText(cursor->scroll - 1, (cursor->scroll - 1) % 7);
		}
	}
	if (input == 0x4000) {
		if (cursor->pos < cursor->count - 1) {
			playSound(0, 2);
			cursor->pos++;
		} else if (cursor->scroll < 0xA) {
			playSound(0, 2);
			cursor->scroll++;
			drawSaveSlotText(cursor->scroll + 5, (cursor->scroll + 5) % 7);
		}
	}
	if (input == CANCEL_BUTTON) {
		playSound(0, 4);
		return 2;
	}
	if (input == CONFIRM_BUTTON) {
		playSound(0, 3);
		return 1;
	}
	return 0;
}

/* MOD (new game as data): written by tools/balance.py from content/tables/new-game.json; -1 = the rule does not exist */
int32_t MOD_NEWGAME[MOD_NEWGAME_SIZE] = { [0 ... MOD_NEWGAME_SIZE - 1] = -1 };

/* MOD (starter quiz): p254 is a list index, never a truncated species id. */
int32_t modNewGameStarter(void)
{
	uint32_t index = readPStat(PSTAT_254);
	return MOD_QUIZ_STARTERS[index < MOD_QUIZ_COUNT ? index : 0];
}

/* MOD (starter quiz): all seven answers contribute. A tie goes to the latest
 * supporting answer, then the earlier axis if both were supported together. */
uint8_t modQuizResult(const uint8_t *answers)
{
	uint8_t score[11] = { 0 };
	uint8_t last[11] = { 0 };
	int32_t q, j, axis, weight, lo, hi, best, cell = 0;

	for (q = 0; q < 7; q++) {
		for (j = 0; j < 3; j++) {
			weight = MOD_QUIZ_WEIGHTS[q * 9 + answers[q] * 3 + j];
			axis = weight >> 4;
			weight &= 15;
			if (weight != 0) {
				score[axis] += weight;
				last[axis] = q + 1;
			}
		}
	}
	for (lo = 0; lo < 11; lo = hi) {
		hi = lo == 0 ? 5 : lo + 3;
		best = lo;
		for (j = lo + 1; j < hi; j++) {
			if (score[j] > score[best] || (score[j] == score[best] && last[j] > last[best])) {
				best = j;
			}
		}
		cell = cell * (hi - lo) + best - lo;
	}
	return (MOD_OPT_NEW_DIGIMON ? MOD_QUIZ_CELLS : MOD_QUIZ_VANILLA_CELLS)[cell];
}

int32_t modNewGame(int32_t slot)
{
	return MOD_NEWGAME[slot] > 0 ? MOD_NEWGAME[slot] : 0;
}

void initializeDefaultSavegame(void)
{
	resetSpeciesFlags(); /* MOD (scale) */
	InventoryTable defaultInventoryCounts;
	InventoryTable defaultInventoryItems;
	SavegamePayload *savegame;
	uint8_t *cursor;
	int32_t i;

	savegame = &SAVE_FILE.saves[0];
	cursor = (uint8_t *)savegame;
	for (i = 0; i < 0xF00; i++) {
		*cursor++ = 0;
	}
	savegame->mainState = 0;
	savegame->currentScreen = SCREEN_LEGACY_NONE;
	savegame->currentExit = modNewGame(MOD_NG_START_EXIT);
	savegame->partnerType = modNewGame(MOD_NG_FIRST_SPECIES);
	strcpy(savegame->playerName, MAIN_D_801346C4);
	strcpy(savegame->digimonName, MAIN_D_8013468C);

	for (i = 0; i < 100; i++) {
		savegame->worldPoop[i].map = 0xFF;
		savegame->worldPoop[i].x = 0xFF;
		savegame->worldPoop[i].y = 0xFF;
		savegame->worldPoop[i].size = 0;
	}
	savegame->money = modNewGame(MOD_NG_MONEY); /* MOD (new game as data) */
	savegame->tamerLevel = 0;
	savegame->raisedCount = 1;
	savegame->year = 0;
	savegame->day = 0;
	savegame->hour = modNewGame(MOD_NG_START_HOUR);
	savegame->minute = modNewGame(MOD_NG_START_MINUTE);
	savegame->lastHandledFrame = savegame->currentFrame = modNewGame(MOD_NG_START_HOUR) * 1200 + modNewGame(MOD_NG_START_MINUTE) * 20;
	savegame->partnerLives = modNewGame(MOD_NG_LIVES);
	savegame->partnerPara.remainingLifetime = modNewGame(MOD_NG_FIRST_LIFETIME);
	savegame->partnerPara.age = modNewGame(MOD_NG_FIRST_AGE);
	for (i = 0; i < 6; i++) {
		savegame->scriptState[i] = modNewGame(MOD_NG_SHOP_CARD0 + i);
	}

	defaultInventoryCounts = DEFAULT_INVENTORY_AMOUNTS;
	defaultInventoryItems = DEFAULT_INVENTORY_TYPES;
	for (i = 0; i < 30; i++) {
		savegame->inventory.types.array[i] = defaultInventoryItems.array[i];
		savegame->inventory.amounts.array[i] = defaultInventoryCounts.array[i];
		savegame->inventory.names.array[i] = i;
	}
	savegame->inventory.size = 30;
}

char *_strncpy(char *dst, char *src, int32_t n)
{
	int32_t i;

	i = 0;
	while (i < n) {
		if (*src == 0) {
			break;
		}
		*dst++ = *src++;
		i++;
	}

	return dst;
}

int32_t createByteSum(uint8_t *data, int32_t len)
{
	uint32_t sum;
	int32_t i;

	sum = 0;
	for (i = 0; i < len; i++) {
		sum += *data++;
	}
	return sum;
}

#if !VERSION_IS(US)
void appendWideDigit(int32_t digit, char **dst, int32_t *started)
{
	digit %= 10;
	if ((*started == 1) || (digit != 0)) {
		strncpy(*dst, SAVEGAME_ID_LABEL[digit] + 2, 2);
		*started = 1;
	} else {
		strncpy(*dst, SAVEGAME_ID_LABEL[0], 2);
	}
	*dst += 2;
}
#endif

int32_t createSavegameChecksum(int32_t slot)
{
	uint32_t first;
	uint32_t second;
	uint32_t sum;

	first = createByteSum((uint8_t *)&SAVE_FILE.saves[slot], 0x4E4);
	second = createByteSum((uint8_t *)SAVE_FILE.saves[slot].battleRegistrationData, 0xa00);
	sum = first + second;
	return sum;
}

/* MOD: the inventory in RAM holds ItemId; the v1 save keeps u8 ids */
static void loadSavedInventory(SavedInventory *saved)
{
	int32_t i;

	for (i = 0; i < 30; i++) {
		INVENTORY.types.array[i] = itemFromLegacy(saved->types.array[i]);
	}
	INVENTORY.amounts = saved->amounts;
	INVENTORY.names = saved->names;
	INVENTORY.size = saved->size;
}

/* MOD: low byte of each id (exact while ids are < 255); ITEM_NONE -> 0xFF */
static void writeSavedInventory(SavedInventory *saved)
{
	int32_t i;

	for (i = 0; i < 30; i++) {
		saved->types.array[i] = (uint8_t)INVENTORY.types.array[i];
	}
	saved->amounts = INVENTORY.amounts;
	saved->names = INVENTORY.names;
	saved->size = INVENTORY.size;
}

void loadSavegame(SavegamePayload *savegame)
{
	int32_t i;

	modForgetGroundPicked(); /* A missing new record, in any old layout, means no pickups today. */

	SAVED_STATE.playerPos = savegame->tamerPosition;
	SAVED_STATE.money = savegame->money;
	SAVED_STATE.partnerPos = savegame->partnerPosition;
	SAVED_STATE.partnerType = savegame->partnerType;
	memcpy(&PARTNER_ENTITY.learnedMoves, savegame->partnerData, 0x10);
	CURRENT_FRAME = savegame->currentFrame;
	LAST_HANDLED_FRAME = savegame->lastHandledFrame;
	YEAR = savegame->year;
	DAY = savegame->day;
	HOUR = savegame->hour;
	MINUTE = savegame->minute;
	PLAYTIME_FRAMES = savegame->playtimeFrames;
	PLAYTIME_HOURS = savegame->playtimeHours;
	PLAYTIME_MINUTES = savegame->playtimeMinutes;
	PARTNER_ENTITY.digimonEntity.stats = savegame->stats;
	SAVED_STATE.partnerStats = savegame->stats;
	loadSavedInventory(&savegame->inventory);
	memcpy(PARTNER_ENTITY.name, savegame->digimonName, 0x14);
    /* The Latin naming keyboard cannot emit Chinese glyph bytes. Restore an automatic
     * species name in the selected language, including a Chinese save loaded on 2 MB. */
    /* Chinese species names can start with a Latin prefix such as V. Stop at
     * the first two-byte character; a Japanese trail byte is never a lead. */
    for (i = 0; i < 19 && (uint8_t)(PARTNER_ENTITY.name[i] - 0x20) < 0x5f; i++) {}
    if ((uint8_t)(PARTNER_ENTITY.name[i] - 0xA0) < 0x40)
        modPartnerNameSetSpecies(SAVED_STATE.partnerType);

	TAMER_ENTITY.tamerLevel = savegame->tamerLevel;
	TAMER_ENTITY.raisedCount = savegame->raisedCount;
	MONEY = savegame->money;
	for (i = 0; i < 8; i++) {
		SAVED_STATE.npcMaps[i] = savegame->npcMaps[i];
	}
	memcpy(DIGIMON_NAME(0), savegame->playerName, 0x14);
	for (i = 0; i < 100; i++) { /* MOD (screens beyond 255): SAVE_TAG_POOP_MAPS gives the wide screens (applyRecords) */
		WORLD_POOP[i].map = savegame->worldPoop[i].map == 0xFF ? -1 : savegame->worldPoop[i].map;
		WORLD_POOP[i].x = savegame->worldPoop[i].x;
		WORLD_POOP[i].y = savegame->worldPoop[i].y;
		WORLD_POOP[i].size = savegame->worldPoop[i].size;
	}
	SAVED_STATE.partnerPara = savegame->partnerPara;
	SAVED_STATE.lives = savegame->partnerLives;
	SAVED_STATE.currentScreen = savegame->currentScreen;
	SAVED_STATE.previousScreen = savegame->previousScreen;
	SAVED_STATE.currentExit = savegame->currentExit;
	SAVED_STATE.previousExit = savegame->previousExit;
	MERIT = savegame->merit;
	BATTLES_STARTED = savegame->battlesStarted;
	BATTLES_FLED = savegame->battlesFled;
	TOURNAMENTS_WON = savegame->tournamentsWon;
	TOURNAMENT_WINS = savegame->tournamentWins;
	TOURNAMENTS_LOST = savegame->tournamentsLost;
	memcpy(SCRIPT_STATE_PTR, savegame->scriptState, 0x17C);
	MAIN_STATE = savegame->mainState;
	memcpy(SAVED_STATE.tamerWaypointX, savegame->tamerWaypointX, 0x1E);
	memcpy(SAVED_STATE.tamerWaypointY, savegame->tamerWaypointY, 0x1E);
	SAVED_STATE.tamerPreviousTileX = savegame->tamerPreviousTileX;
	SAVED_STATE.tamerPreviousTileY = savegame->tamerPreviousTileY;
	SAVED_STATE.tamerWaypointCurrent = savegame->tamerWaypointCurrent;
	SAVED_STATE.tamerWaypointCount = savegame->tamerWaypointCount;
	SAVED_STATE.tamerStartTileX = savegame->tamerStartTileX;
	SAVED_STATE.tamerStartTileY = savegame->tamerStartTileY;
	SAVED_STATE.tamerWaypointActive = savegame->tamerWaypointActive;
}

void writeSavegame(SavegamePayload *savegame)
{
	int32_t restorePStat;
	int32_t i;

#if VERSION_EQUAL_OR_NEWER(US)
	recalculatePPandArena();
#endif
	restorePStat = 0;
	if (readPStat(0) == 3) {
		writePStat(0, 0);
		restorePStat = 1;
	}
	savegame->tamerPosition = TAMER_ENTITY.entity.posData->location;
	savegame->partnerPosition = PARTNER_ENTITY.digimonEntity.entity.posData->location;
	savegame->partnerType = PARTNER_ENTITY.digimonEntity.entity.type;
	savegame->money = MONEY;
	memcpy(savegame->partnerData, &PARTNER_ENTITY.learnedMoves, 0x10);
	savegame->hour = HOUR;
	savegame->minute = MINUTE;
	savegame->currentFrame = CURRENT_FRAME;
	savegame->lastHandledFrame = LAST_HANDLED_FRAME;
	savegame->day = DAY;
	savegame->year = YEAR;
	savegame->playtimeFrames = PLAYTIME_FRAMES;
	savegame->playtimeHours = PLAYTIME_HOURS;
	savegame->playtimeMinutes = PLAYTIME_MINUTES;
	writeSavedInventory(&savegame->inventory);
	savegame->stats = PARTNER_ENTITY.digimonEntity.stats;
	savegame->partnerLives = PARTNER_ENTITY.lives;
	memcpy(savegame->digimonName, PARTNER_ENTITY.name, 0x14);
	savegame->tamerLevel = TAMER_ENTITY.tamerLevel;
	savegame->raisedCount = TAMER_ENTITY.raisedCount;
	for (i = 0; i < 8; i++) {
		savegame->npcMaps[i] = NPC_ENTITIES[i].digimonEntity.entity.isOnMap;
	}
	memcpy(savegame->playerName, DIGIMON_NAME(0), 0x14);
	for (i = 0; i < 100; i++) { /* MOD (screens beyond 255): a screen the v1 byte cannot name is 0xFF (none) there */
		savegame->worldPoop[i].map = (uint32_t)WORLD_POOP[i].map < NUM_ORIGINAL_SCREENS ? WORLD_POOP[i].map : 0xFF;
		savegame->worldPoop[i].x = WORLD_POOP[i].x;
		savegame->worldPoop[i].y = WORLD_POOP[i].y;
		savegame->worldPoop[i].size = WORLD_POOP[i].size;
	}
	savegame->partnerPara = PARTNER_PARA;
	/* MOD: the v1 save keeps the low byte (exact while ids are < 255) */
	savegame->currentScreen = (uint8_t)CURRENT_SCREEN;
	savegame->previousScreen = (uint8_t)PREVIOUS_SCREEN;
	savegame->currentExit = CURRENT_EXIT;
	savegame->previousExit = PREVIOUS_EXIT;
	savegame->merit = MERIT;
	savegame->battlesStarted = BATTLES_STARTED;
	savegame->battlesFled = BATTLES_FLED;
	savegame->tournamentsWon = TOURNAMENTS_WON;
	savegame->tournamentWins = TOURNAMENT_WINS;
	savegame->tournamentsLost = TOURNAMENTS_LOST;
	memcpy(savegame->scriptState, SCRIPT_STATE_PTR, 0x17C);
	memcpy(savegame->tamerWaypointX, TAMER_WAYPOINT_X, 0x1E);
	memcpy(savegame->tamerWaypointY, TAMER_WAYPOINT_Y, 0x1E);
	savegame->tamerPreviousTileX = TAMER_PREVIOUS_TILE_X;
	savegame->tamerPreviousTileY = TAMER_PREVIOUS_TILE_Y;
	savegame->tamerWaypointCurrent = TAMER_WAYPOINT_CURRENT;
	savegame->tamerWaypointCount = TAMER_WAYPOINT_COUNT;
	savegame->tamerStartTileX = TAMER_START_TILE_X;
	savegame->tamerStartTileY = TAMER_START_TILE_Y;
	savegame->tamerWaypointActive = TAMER_WAYPOINT_ACTIVE;
	savegame->mainState = MAIN_STATE;
	if (restorePStat == 1) {
		writePStat(0, 3);
	}
}

/* MOD (save v2) ------------------------------------------------------------ */

/* the version words this build reads: 1 (no word), the v2 fixed extension, its own tagged one; any other is a layout it does not know */
static int32_t isTaggedVersion(int32_t version)
{
	return version == SAVE_FORMAT_VERSION;
}

static int32_t isSaveVersionReadable(int32_t version)
{
	return version == 1 || version == SAVE_LEGACY_VERSION || isTaggedVersion(version);
}

/* the version word and extension version this build writes for the file in SAVE_FILE */
static int32_t saveLayoutVersion(void)
{
	return SAVE_FORMAT_VERSION;
}

static int32_t getSaveVersion(SavegamePayload *savegame)
{
	uint8_t *w;

	w = &savegame->unknown4E1[3];
	if (w[0] == 'D' && w[1] == 'W' && w[2] == '2' && w[3] == '\0') {
		return w[4];
	}
	return 1;
}

static int32_t createExtensionChecksum(SaveExtension *ext)
{
	return createByteSum((uint8_t *)ext, __builtin_offsetof(SaveExtension, checksum));
}

/* the bytes of one copy's extension in the file being handled (SAVE_FILE_BYTES); a one-block file has none, but one is still built */
static int32_t saveExtSize(void)
{
	int32_t n = (SAVE_FILE_BYTES - SAVE_V1_BYTES) / 2;

	return n > 0 ? n : SAVE_EXT_BYTES;
}

static uint8_t *saveExtOf(int32_t copy)
{
	return (uint8_t *)SAVE_EXT_ADDR + copy * saveExtSize();
}

/* MOD (save v2): content the original executable cannot represent */
static int32_t needsNewExecutable(int32_t partnerType, int32_t currentScreen, int32_t previousScreen, int16_t *inventory)
{
	int32_t i;

	if ((uint32_t)partnerType >= NUM_ORIGINAL_DIGIMON || (uint32_t)currentScreen >= NUM_ORIGINAL_SCREENS ||
	    (uint32_t)previousScreen >= NUM_ORIGINAL_SCREENS) {
		return 1;
	}
	for (i = 0; i < 30; i++) {
		if (inventory[i] >= NUM_ITEMS) {
			return 1;
		}
	}
	/* History and recruits are required content even with a native partner. */
	for (i = 0; i < NUM_DIGIMON / 8; i++) {
		if (SPECIES_RAISED_BITS[i] || SPECIES_RECRUITED_BITS[i]) return 1;
	}
	return 0;
}

/* a v2 file's extension: the fixed struct */
static int32_t isLegacyExtensionValid(int32_t copy)
{
	SaveExtension *ext;

	ext = (SaveExtension *)saveExtOf(copy);
	return SAVE_FILE_BYTES >= SAVE_V2_BYTES && ext->magic[0] == 'D' && ext->magic[1] == 'W' &&
	       ext->magic[2] == '2' && ext->magic[3] == 'E' && ext->version == SAVE_LEGACY_VERSION &&
	       ext->size == sizeof(SaveExtension) && ext->checksum == createExtensionChecksum(ext) &&
	       ext->v1Checksum == SAVE_FILE.saves[copy].checksum;
}

/* the records of a v4 extension all lie inside it and end with the end record */
static int32_t areRecordsWellFormed(uint8_t *e, int32_t size)
{
	int32_t at = sizeof(SaveExtHead);
	int32_t tag, len;

	for (;;) {
		if (at + 4 > size - 4) {
			return 0;
		}
		tag = e[at] | (e[at + 1] << 8);
		len = e[at + 2] | (e[at + 3] << 8);
		if (tag == SAVE_TAG_END) {
			return len == 0;
		}
		at += 4 + len;
	}
}

static int32_t isTaggedExtensionValid(int32_t copy)
{
	SaveExtHead *h;
	uint8_t *e;
	int32_t size;

	size = saveExtSize();
	e = saveExtOf(copy);
	h = (SaveExtHead *)e;
	return SAVE_FILE_BYTES >= SAVE_V2_BYTES && h->magic[0] == 'D' && h->magic[1] == 'W' && h->magic[2] == '4' &&
	       h->magic[3] == 'E' && h->version == saveLayoutVersion() && h->size == size &&
	       *(int32_t *)(e + size - 4) == createByteSum(e, size - 4) && h->v1Checksum == SAVE_FILE.saves[copy].checksum &&
	       areRecordsWellFormed(e, size);
}

static int32_t isExtensionValid(int32_t copy)
{
	int32_t version = getSaveVersion(&SAVE_FILE.saves[copy]);

	if (isTaggedVersion(version)) {
		return isTaggedExtensionValid(copy);
	}
	return version == SAVE_LEGACY_VERSION && isLegacyExtensionValid(copy);
}

/* appends a record at p (the room is checked at the build: SAVE_RECORDS_MAX); returns where the next one goes */
static uint8_t *putRecord(uint8_t *p, int32_t tag, void *data, int32_t len)
{
	int32_t i;

	p[0] = tag;
	p[1] = tag >> 8;
	p[2] = len;
	p[3] = len >> 8;
	for (i = 0; i < len; i++) {
		p[4 + i] = ((uint8_t *)data)[i];
	}
	return p + 4 + len;
}

/*
 * The extension of copy 0 from the game state in RAM (fromRam) or, for the
 * default save of a new game, from its v1 payload; the version word, both
 * checksums and the mirror copy.
 */
/* MOD (options): the options as the save keeps them, SAVE_TAG_OPTIONS and the header's copy (modTitleOptions) */
static void modOptionBytes(uint8_t *d)
{
	d[0] = MOD_OPT_BUTTONS;
	d[1] = 1; /* Reserved digital-clock byte. */
	d[2] = 20; /* Reserved frame-rate byte; safe for older readers. */
	d[3] = 1; /* Reserved care-HUD byte: always enabled, including for older readers. */
#if FONT_SWITCH
	d[4] = MOD_FONT == MOD_FONT_NEW ? 1 : MOD_FONT == MOD_FONT_CLASSIC ? 2 : 0; /* FONT */
#else
	d[4] = 0;
#endif
	d[5] = MOD_OPT_EVO_GUIDE;
	d[6] = MOD_OPT_NEW_DIGIMON;
	d[7] = MOD_LANG; /* MOD (language) */
	d[8] = MOD_OPT_SIGNS;
	d[9] = MOD_OPT_STORY; /* MOD (story) */
	d[10] = MOD_OPT_DIFFICULTY; /* MOD (difficulty) */
	d[11] = MOD_OPT_QUICK_BATTLE;
	d[12] = (MOD_OPT_CHEATS ? MOD_OPT_CHEATS_BIT : 0) | (MOD_OPT_SHORTCUTS ? MOD_OPT_SHORTCUTS_BIT : 0);
	d[13] = 0; /* Reserved inverted enemy-bar flags: always ON/shown. */
}

/* MOD (options): a save's options (modOptionBytes, `len` of them), unless OPTIONS changed them since the start */
static void modApplyOptions(const uint8_t *d, int32_t len)
{
	/* not a row of OPTIONS: a save always decides it. Old saves and the card header: OFF */
	MOD_OPT_SHORTCUTS = len >= 13 && (d[12] & MOD_OPT_SHORTCUTS_BIT) != 0;
	if (MOD_OPT_TOUCHED) {
		return;
	}
	if (len >= 1) {
		modSetButtons(d[0]);
	}
	/* Bytes 1, 2, 3 and 13 are reserved; legacy display choices are ignored. */
#if FONT_SWITCH
	if (len >= 5) {
		modSetFont(d[4] == 0 ? MOD_FONT_ORIGINAL : d[4] == 2 ? MOD_FONT_CLASSIC : MOD_FONT_NEW);
	}
#endif
	if (len >= 6) {
		MOD_OPT_EVO_GUIDE = d[5] != 0;
	}
	if (len >= 7) {
		MOD_OPT_NEW_DIGIMON = d[6] != 0;
		modApplySpeciesLevels(); /* MOD (canon levels) */
	}
	if (len >= 8) { /* MOD (language) */
		modSetLanguage(d[7]);
	}
	if (len >= 9) {
		modSetSigns(d[8]);
	}
	if (len >= 10) { /* MOD (story) */
		modSetStory(d[9]);
	}
	/* Missing or invalid legacy difficulty defaults to NORMAL. */
	MOD_OPT_DIFFICULTY = len >= 11 && d[10] <= MOD_DIFF_HARD ? d[10] : MOD_DIFF_NORMAL;
	if (len >= 12) { /* MOD (quick battle start) */
		MOD_OPT_QUICK_BATTLE = d[11] != 0;
	}
	MOD_OPT_CHEATS = len >= 13 && (d[12] & MOD_OPT_CHEATS_BIT) != 0; /* old saves: OFF */
}

/*
 * MOD (options): the header's PocketStation bytes (0x50, unused by this game) keep a copy of the options after "O",
 * their count and a save number one past the highest seen (MOD_SAVE_NUMBER), so the title menu's OPTIONS shows the
 * newest save's before one is loaded (modCardOptions). Run after modTitleNameTail.
 */
static void modTitleOptions(void)
{
	uint8_t *p;
	uint8_t options[MOD_OPT_BYTES];

	p = SAVE_FILE.pocketStation;
	memset(p, 0, sizeof(SAVE_FILE.pocketStation));
	MOD_SAVE_NUMBER = (MOD_SAVE_NUMBER + 1) & 0xffff;
	p[0] = 'O';
	p[1] = MOD_OPT_HEADER_BYTES | (MOD_OPT_CHEATS ? MOD_OPT_HEADER_CHEATS : 0);
	p[2] = MOD_SAVE_NUMBER;
	p[3] = MOD_SAVE_NUMBER >> 8;
	modOptionBytes(options);
	memcpy(&p[4], options, MOD_OPT_HEADER_BYTES);
}

/* MOD (options): the save number in a save's header (modTitleOptions), -1 without one */
static int32_t modHeaderSaveNumber(const uint8_t *header)
{
	if (header[0x50] != 'O' || (header[0x51] & MOD_OPT_HEADER_COUNT) == 0 ||
	    (header[0x51] & MOD_OPT_HEADER_COUNT) > MOD_OPT_HEADER_BYTES) {
		return -1;
	}
	return header[0x52] | header[0x53] << 8;
}

/*
 * MOD (options): OPTIONS opened at the title before any change shows the options of the newest save on the cards
 * (the highest header save number), the ones a Continue of it applies. A card missing, unformatted or unreadable is
 * skipped without the card error screens.
 */
static void modCardOptions(void)
{
	MemCardSyncWord cmd;
	MemCardSyncWord result;
	long count;
	uint8_t data[0x80];
	uint8_t best[MOD_OPT_BYTES];
	int32_t bestNumber;
	int32_t bestLen;
	int32_t number;
	int32_t c;
	int32_t i;

	if (MOD_OPT_TOUCHED) {
		return;
	}
	bestNumber = -1;
	bestLen = 0;
	for (c = 0; c < 2; c++) {
		if (!(CONNECTED_CARDS & (c ? 0x10 : 1))) {
			continue;
		}
		MemCardSync(0, &cmd, &result);
		if (MemCardAccept(c ? 0x10 : 0) == 0) {
			continue;
		}
		MemCardSync(0, &cmd, &result);
		if (isMemcardUnformatted(c ? 0x10 : 0, result) != 0) {
			continue;
		}
		SAVE_FILE_NAME[0xF] = 0x3F;
		i = MemCardGetDirentry(c ? 0x10 : 0, SAVE_FILE_NAME, MEMCARD_DIRENTRIES, &count, 0, 15);
		if (i != 0 && i != 3) {
			continue;
		}
		for (i = 0; i < count; i++) {
			if (MemCardReadFile(c ? 0x10 : 0, MEMCARD_DIRENTRIES[i].name, (void *)data, 0, 0x80) != 1) {
				break;
			}
			MemCardSync(0, &cmd, &result);
			if (result != 0) {
				break;
			}
			number = modHeaderSaveNumber(data);
			if (number > bestNumber) {
				bestNumber = number;
				bestLen = data[0x51] & MOD_OPT_HEADER_COUNT;
				memcpy(best, &data[0x54], bestLen);
				if (bestLen == MOD_OPT_HEADER_BYTES) {
					best[12] = (data[0x51] & MOD_OPT_HEADER_CHEATS) != 0;
					best[13] = 0;
					bestLen = MOD_OPT_BYTES;
				}
			}
		}
	}
	if (bestNumber > MOD_SAVE_NUMBER) {
		MOD_SAVE_NUMBER = bestNumber;
	}
	if (bestNumber >= 0) {
		MOD_BOOT_LANGUAGE = 0;
		modApplyOptions(best, bestLen);
	}
}

/*
 * MOD (options): at boot, before the title (main.c), the newest save's options (modCardOptions), so the title screen
 * and its menu are in the save's language and font, not only once OPTIONS opens: which card slots hold a card, asked
 * at once (MemCardExist, waited for; the menu's own poll, updateMemoryCardState, finds one slot per answer)
 */
void modBootOptions(void)
{
	MemCardSyncWord cmd;
	MemCardSyncWord result;
	int32_t c;
	int32_t bit;

	for (c = 0; c < 2; c++) {
		bit = c ? 0x10 : 1;
		MemCardSync(0, &cmd, &result); /* the poll's request in flight, if any */
		MemCardExist(c ? 0x10 : 0);
		MemCardSync(0, &cmd, &result);
		if (result == 0 || result == 3) {
			CONNECTED_CARDS |= bit;
			if (result == 3) {
				NEW_CARDS |= bit;
			}
		} else {
			CONNECTED_CARDS &= ~bit;
		}
	}
	modButtonsBoot(); /* MOD (options): the Japanese disc starts with JAPANESE CONTROLS on, the saves' options win */
	MOD_BOOT_LANGUAGE = modLanguageCount() > 1;
	MOD_BOOT_LANGUAGE_CURSOR = 0;
	modCardOptions();
	modNoticeProbe();
}

/*
 * MOD (emulator notice): the title screen lists the settings the game plays best with when it could not find one of them, each on its own
 * row from its own check; with both there, nothing shows. RAM: modProbeRam. CPU: the reads of the hblank
 * counter that fit in MOD_CPU_LINES lines. The counter follows the screen whatever the emulated CPU does, so the count grows with
 * an overclock: DuckStation 3801 at 100 %, 7389 at 200 %, 9303 at 250 %, 10977 at 300 %. MOD_CPU_SPINS_FAST sits between 200 and 250.
 */
#define MOD_NOTICE_NO_RAM	1
#define MOD_NOTICE_SLOW_CPU	2
#define MOD_NOTICE_LEFT		-148 /* the panel, over the top of the logo */
#define MOD_NOTICE_TOP		12
#define MOD_NOTICE_WIDTH	296
#define MOD_NOTICE_HEIGHT	72
#define MOD_NOTICE_STRS		6
#define MOD_NOTICE_NAME_W	0x96 /* the two columns of a row, with any font */
#define MOD_NOTICE_STATE_W	0x78
#define MOD_NOTICE_V(i)		(((i) + ((i) == 5)) * 0xC) /* row 5 of the text area is the version's */
void modPlateRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, int32_t g, int32_t b, int32_t z);
void renderString(int32_t color, int16_t x, int16_t y, int16_t width, int16_t height, int16_t u, int16_t v, int32_t depth, int32_t shadow);

int32_t MOD_CPU_SPINS;
static int32_t MOD_NOTICE;
static uint8_t MOD_NOTICE_W[MOD_NOTICE_STRS];
static char *MOD_NOTICE_TEXT[MOD_NOTICE_STRS] = {
	MOD_NOTICE_STR_HEAD1, MOD_NOTICE_STR_HEAD2, MOD_NOTICE_STR_RAM, MOD_NOTICE_STR_CPU, MOD_NOTICE_STR_MISSING, MOD_NOTICE_STR_READY,
};

/* Keep this loop identical at boot and during idle presentation waits. */
int32_t modCpuProbe(void)
{
	volatile uint16_t *lines = (volatile uint16_t *)0x1F801110;
	uint16_t start = *lines;
	int32_t spins = 0;

	do {
		spins++;
	} while ((uint16_t)(*lines - start) < MOD_CPU_LINES);
	MOD_CPU_SPINS = spins;
	/* Keep CPU advice current without repeating the RAM probe during play. */
	MOD_NOTICE = (MOD_NOTICE & MOD_NOTICE_NO_RAM) | (spins < MOD_CPU_SPINS_FAST ? MOD_NOTICE_SLOW_CPU : 0);
	return spins;
}

void modNoticeProbe(void)
{
	modCpuProbe();
#if FPS != 20
	MOD_FPS_MODE_RUN = MOD_CPU_SPINS >= MOD_CPU_SPINS_FAST ? 2 : 0;
#endif
	MOD_NOTICE = (modProbeRam() ? 0 : MOD_NOTICE_NO_RAM) | (MOD_CPU_SPINS < MOD_CPU_SPINS_FAST ? MOD_NOTICE_SLOW_CPU : 0);
}

void modNoticeDraw(void)
{
	RECT rect;
	int32_t i;
	int32_t mode;
	char *str;

	if (MOD_NOTICE == 0) {
		return;
	}
	setRECT(&rect, 0, 0, 0x100, MOD_NOTICE_V(MOD_NOTICE_STRS));
	clearTextSubArea(&rect);
	for (i = 0; i < MOD_NOTICE_STRS; i++) {
		str = MOD_NOTICE_TEXT[i];
		mode = fitStringMode(str, i < 2 ? 0xFE : i < 4 ? MOD_NOTICE_NAME_W : MOD_NOTICE_STATE_W);
		drawStringMode(str, 0, MOD_NOTICE_V(i), mode);
		DrawSync(0);
		mode = measureStringMode(str, mode) + 1; /* and the shadow */
		MOD_NOTICE_W[i] = mode < 0xFF ? mode : 0xFF; /* a u of 8 bits */
	}
}

static void modNoticeString(int32_t color, int32_t x, int32_t y, int32_t i)
{
	renderString(color, x, y - 0x78, MOD_NOTICE_W[i], 0xC, 0, MOD_NOTICE_V(i), 0, 1);
}

void modNoticeRender(void)
{
	int32_t i;
	int32_t missing;

	if (MOD_NOTICE == 0) {
		return;
	}
	modPlateRect(MOD_NOTICE_LEFT, MOD_NOTICE_TOP - 0x78, MOD_NOTICE_WIDTH, MOD_NOTICE_HEIGHT, 0x46, 0xBE, 0xEB, 2);
	modPlateRect(MOD_NOTICE_LEFT + 1, MOD_NOTICE_TOP - 0x77, MOD_NOTICE_WIDTH - 2, MOD_NOTICE_HEIGHT - 2, 8, 10, 0x38, 1);
	modNoticeString(4, -MOD_NOTICE_W[0] / 2, MOD_NOTICE_TOP + 8, 0);
	modNoticeString(4, -MOD_NOTICE_W[1] / 2, MOD_NOTICE_TOP + 20, 1);
	for (i = 0; i < 2; i++) {
		missing = (MOD_NOTICE >> i) & 1;
		modNoticeString(0, MOD_NOTICE_LEFT + 12, MOD_NOTICE_TOP + 38 + i * 14, 2 + i);
		modNoticeString(missing ? 6 : 7, MOD_NOTICE_LEFT + MOD_NOTICE_WIDTH - 12 - MOD_NOTICE_W[5 - missing], MOD_NOTICE_TOP + 38 + i * 14,
				5 - missing);
	}
}

static void prepareSaveFile(int32_t fromRam)
{
	SavegamePayload *savegame;
	SaveExtHead *head;
	uint8_t *e;
	uint8_t *p;
	int16_t words[100];
	int16_t screens[2];
	int16_t inventory[30];
	int32_t partnerType;
	uint8_t options[MOD_OPT_BYTES]; /* MOD (options) */
	int16_t swap[2];
	int32_t size;
	int32_t i;

	savegame = &SAVE_FILE.saves[0];
	size = saveExtSize();
	e = saveExtOf(0);
	for (p = e, i = 0; i < size; i++) {
		*p++ = 0;
	}
	head = (SaveExtHead *)e;
	head->magic[0] = 'D';
	head->magic[1] = 'W';
	head->magic[2] = '4';
	head->magic[3] = 'E';
	head->version = saveLayoutVersion();
	head->size = size;
	p = e + sizeof(SaveExtHead);
	if (fromRam) {
		screens[0] = CURRENT_SCREEN;
		screens[1] = PREVIOUS_SCREEN;
		for (i = 0; i < 30; i++) {
			inventory[i] = INVENTORY.types.array[i];
		}
	} else {
		screens[0] = savegame->currentScreen;
		screens[1] = savegame->previousScreen;
		for (i = 0; i < 30; i++) {
			inventory[i] = itemFromLegacy(savegame->inventory.types.array[i]);
		}
	}
	partnerType = savegame->partnerType;
	p = putRecord(p, SAVE_TAG_SCREENS, screens, sizeof(screens));
	p = putRecord(p, SAVE_TAG_INVENTORY, inventory, sizeof(inventory));
	p = putRecord(p, SAVE_TAG_PARTNER_TYPE, &partnerType, sizeof(partnerType));
	if (fromRam) {
		for (i = 0; i < SAVE_EXT_SAVED_WREGS; i++) {
			words[i] = SCRIPT_STATE_PTR->wregs[i];
		}
		p = putRecord(p, SAVE_TAG_WREGS, words, 2 * SAVE_EXT_SAVED_WREGS);
	}
	for (i = 0; i < 40; i++) {
		words[i] = registeredSpecies(&savegame->battleRegistrationData[i]);
	}
	p = putRecord(p, SAVE_TAG_REGISTERED, words, 2 * 40);
	for (i = 0; i < 100; i++) { /* MOD (screens beyond 255): from RAM the wide screen */
		words[i] = fromRam ? WORLD_POOP[i].map : savegame->worldPoop[i].map == 0xFF ? -1 : savegame->worldPoop[i].map;
	}
	p = putRecord(p, SAVE_TAG_POOP_MAPS, words, 2 * 100);
	if (fromRam) {
		p = putRecord(p, SAVE_TAG_SPECIES_RAISED, SPECIES_RAISED_BITS, NUM_DIGIMON / 8);
		p = putRecord(p, SAVE_TAG_SPECIES_RECRUITED, SPECIES_RECRUITED_BITS, NUM_DIGIMON / 8);
	}
	modOptionBytes(options); /* MOD (options) */
	p = putRecord(p, SAVE_TAG_OPTIONS, options, sizeof(options));
	if (MOD_SWAP_TO > 0 && MOD_SWAP_TO == partnerType) { /* MOD (options): the partner NEW DIGIMON off turned */
		swap[0] = MOD_SWAP_FROM;
		swap[1] = MOD_SWAP_TO;
		p = putRecord(p, SAVE_TAG_VANILLA_SWAP, swap, sizeof(swap));
	}
	if (fromRam) { /* MOD (ground items a day) */
		/* Retain the old tag's meaning (an empty location list) for older readers. */
		p = putRecord(p, SAVE_TAG_GROUND_PICKED, 0, 0);
		p = putRecord(p, SAVE_TAG_GROUND_PICKUP_BITS, MOD_GROUND_PICKED, sizeof(MOD_GROUND_PICKED));
	}
	/* MOD (tier 2): the story state's records go here */
	putRecord(p, SAVE_TAG_END, 0, 0);
	p = &savegame->unknown4E1[3];
	p[0] = 'D';
	p[1] = 'W';
	p[2] = '2';
	p[3] = '\0';
	p[4] = saveLayoutVersion();
	savegame->checksum = createSavegameChecksum(0);
	if (needsNewExecutable(partnerType, screens[0], screens[1], inventory)) {
		savegame->checksum = ~savegame->checksum; /* the original refuses it */
	}
	head->v1Checksum = savegame->checksum;
	*(int32_t *)(e + size - 4) = createByteSum(e, size - 4);
	SAVE_FILE.saves[1] = SAVE_FILE.saves[0];
	for (i = 0; i < size; i++) {
		saveExtOf(1)[i] = e[i];
	}
	SAVE_FILE.blockNumber = SAVE_FILE_BYTES / SAVE_V1_BYTES;
}

/*
 * Copy 0 or 1 is intact: its v1 checksum matches, or it is a v2 save that
 * refuses the original executable (inverted checksum) with a valid extension.
 */
static int32_t isSaveCopyValid(int32_t copy)
{
	int32_t sum;

	sum = createSavegameChecksum(copy);
	if (SAVE_FILE.saves[copy].checksum == sum) {
		return 1;
	}
	return SAVE_FILE.saves[copy].checksum == ~sum && getSaveVersion(&SAVE_FILE.saves[copy]) >= 2 &&
	       isExtensionValid(copy);
}

/*
 * MOD (data from files): a species, item or technique whose file the build
 * did not have keeps its id with an empty row (no name; an all-zero move row).
 * A save that holds one must not load it: the partner becomes the first
 * reincarnation baby that exists (stats and name stay), the item leaves the
 * inventory, the technique is forgotten. Checked on the loaded globals.
 */
extern int16_t REINCARNATE_BABY_TYPE[4];

static int32_t isMoveAbsent(int32_t move)
{
	int32_t *row;
	int32_t i;

	if ((uint32_t)move >= NUM_MOVES) {
		return move != 0xFF;
	}
	row = (int32_t *)&MOVE_DATA[move];
	for (i = 0; i < (int32_t)(sizeof(Move) / 4); i++) {
		if (row[i] != 0) {
			return 0;
		}
	}
	return 1;
}

/* the registered Digimon (VS mode) of a species or with a technique the build does not have: the entry is emptied / unequipped */
static void dropAbsentRegistered(RegisteredDigimon *p, int32_t n)
{
	int32_t i, k;

	for (i = 0; i < n; i++, p++) {
		if (registeredSpecies(p) != p->digimonId && DIGIMON_NAME(registeredSpecies(p))[0] == '\0') {
			p->wideTag = 0; /* MOD (wide registered species): a species this build lacks fights as its stand-in */
		}
		if (p->digimonId != 0 && DIGIMON_NAME(p->digimonId)[0] == '\0') {
			p->digimonId = 0;
			continue;
		}
		for (k = 0; k < 3; k++) {
			if (isMoveAbsent(p->moves[k])) {
				p->moves[k] = 0xff;
			}
		}
	}
}

void setDigimonRaised(int32_t type);

/* MOD (options): the loaded partner becomes species `to`, its stats, care and age kept: the equipped techniques are
 * the new species' slots of the same techniques (none: its first learned one), the finisher its own */
static void swapPartnerSpecies(int32_t to)
{
	uint32_t *learned = &PARTNER_ENTITY.learnedMoves[0];
	uint8_t old[16];
	uint8_t *equipped;
	int32_t from;
	int32_t i;
	int32_t k;
	int32_t anim;
	int32_t tech;

	from = SAVED_STATE.partnerType;
	for (k = 0; k < 16; k++) {
		old[k] = DIGI_PARA(from).moves[k];
	}
	equipped = PARTNER_ENTITY.digimonEntity.stats.base.moves;
	for (i = 0; i < 3; i++) {
		anim = equipped[i];
		if (anim >= 0x2e && anim < 0x2e + 16) {
			tech = old[anim - 0x2e];
			for (k = 0; k < 16 && DIGI_PARA(to).moves[k] != tech; k++) {
			}
			equipped[i] = k < 16 ? 0x2e + k : 0xff;
		}
	}
	if (equipped[0] == 0xff && equipped[1] == 0xff && equipped[2] == 0xff) {
		for (k = 0; k < 16; k++) {
			tech = DIGI_PARA(to).moves[k];
			if (tech != 0xff && tech < 0x80 && (learned[tech / 32] >> (tech % 32)) & 1) {
				equipped[0] = 0x2e + k;
				break;
			}
		}
	}
	if (equipped[0] == 0xff && equipped[1] == 0xff && equipped[2] == 0xff) { /* none learned: its first one, learned */
		for (k = 0; k < 16; k++) {
			tech = DIGI_PARA(to).moves[k];
			if (tech != 0xff && tech < 0x3a) {
				learned[tech / 32] |= 1u << (tech % 32);
				equipped[0] = 0x2e + k;
				break;
			}
		}
	}
	equipped[3] = 0xff; /* the finisher: the last special technique of its list (as EVL_applyEvolution) */
	for (k = 0; k < 16; k++) {
		tech = DIGI_PARA(to).moves[k];
		if (tech >= 0x3a && tech <= 0x70) {
			equipped[3] = 0x2e + k;
		}
	}
	for (i = 0; i < 4; i++) {
		SAVED_STATE.partnerStats.base.moves[i] = equipped[i];
	}
	modPartnerNameFollow(from, to); /* MOD (optional name) */
	SAVED_STATE.partnerType = to;
	PARTNER_ENTITY.digimonEntity.entity.type = to;
	setDigimonRaised(to);
}

/*
 * MOD (options): NEW DIGIMON off and a partner of a new species: it becomes its vanilla species (vanilla_as), and the
 * save remembers what it was (SAVE_TAG_VANILLA_SWAP). Back on, a partner that is still that vanilla species is turned
 * back. Raised and recruited flags are never touched: the chart only hides the new species while the option is off.
 */
static void modVanillaSwap(void)
{
	int32_t t;
	int32_t to;

	t = SAVED_STATE.partnerType;
	if (MOD_OPT_NEW_DIGIMON) {
		if (MOD_SWAP_TO > 0 && t == MOD_SWAP_TO && MOD_SWAP_FROM > 0 && DIGIMON_NAME(MOD_SWAP_FROM)[0] != '\0') {
			swapPartnerSpecies(MOD_SWAP_FROM);
		}
		MOD_SWAP_FROM = MOD_SWAP_TO = 0;
		return;
	}
	if (t <= 0 || isVanillaSpecies(t)) {
		return;
	}
	to = getSpeciesVanillaAs(t);
	if (to <= 0 || !isVanillaSpecies(to) || DIGIMON_NAME(to)[0] == '\0') {
		return;
	}
	MOD_SWAP_FROM = t;
	MOD_SWAP_TO = to;
	swapPartnerSpecies(to);
}

/*
 * MOD (story): the save was played with the other story (EXTENDED STORY changed in OPTIONS since): its screen may not
 * exist in this one, nor the way out of it, so the game goes on in Jijimon's house, as the original's door from File
 * City picks it (205 once Angemon or Monzaemon live in the city or prosperity is 50, else 218), at its exit 0
 * (initializeLoadedMap places the tamer and the partner there). The flags stay: each story reads its own.
 */
#define MOD_STORY_HOUSE		218	/* Jijimon's house */
#define MOD_STORY_HOUSE_BIG	205	/* the same, grown */
#define MOD_STORY_CITY		168	/* File City, outside its door */
static int32_t MOD_SAVE_STORY; /* the story the loaded save was played with (SAVE_TAG_OPTIONS byte 9; none: extended) */

static void modStoryRelocate(void)
{
	if (MOD_SAVE_STORY == MOD_OPT_STORY || !modStoryChoice()) {
		return;
	}
	SAVED_STATE.currentScreen =
	    isDigimonRecruited(20) || isDigimonRecruited(DIGIMON_MONZAEMON) || readPStat(1) >= 50 ? MOD_STORY_HOUSE_BIG
												  : MOD_STORY_HOUSE;
	SAVED_STATE.previousScreen = MOD_STORY_CITY;
	SAVED_STATE.currentExit = 0;
	SAVED_STATE.previousExit = 0;
	SAVED_STATE.tamerWaypointActive = 0;
	SAVED_STATE.tamerWaypointCount = 0;
	MOD_STORY_ARRIVE = 1;
}

static void dropAbsentSaveContent(void)
{
	uint32_t *learned = &PARTNER_ENTITY.learnedMoves[0]; /* 128 bits: the struct names 64 (unk1, unk2 follow) */
	uint8_t *equipped;
	int32_t i, t, anim;

	t = SAVED_STATE.partnerType;
	if (DIGIMON_NAME(t)[0] == '\0') {
		for (i = 0; i < 4 && DIGIMON_NAME(REINCARNATE_BABY_TYPE[i])[0] == '\0'; i++) {
		}
		if (i < 4) {
			SAVED_STATE.partnerType = REINCARNATE_BABY_TYPE[i];
			PARTNER_ENTITY.digimonEntity.entity.type = REINCARNATE_BABY_TYPE[i];
			for (i = 0; i < 4; i++) { /* the animations were the old species' */
				PARTNER_ENTITY.digimonEntity.stats.base.moves[i] = 0xff;
				SAVED_STATE.partnerStats.base.moves[i] = 0xff;
			}
			learned[0] = learned[1] = learned[2] = learned[3] = 0;
		}
	}
	for (i = 0; i < 30; i++) {
		t = INVENTORY.types.array[i];
		if (t != ITEM_NONE && ((uint32_t)t >= NUM_ITEMS || ITEM_PARA[t].name[0] == '\0')) {
			INVENTORY.types.array[i] = ITEM_NONE;
			INVENTORY.amounts.array[i] = 0;
			INVENTORY.names.array[i] = 0xff;
		}
	}
	for (i = 0; i < NUM_MOVES; i++) {
		if (isMoveAbsent(i)) {
			learned[i / 32] &= ~(1u << (i % 32));
		}
	}
	memcpy(SAVE_FILE.saves[0].partnerData, learned, 0x10);
	dropAbsentRegistered(SAVE_FILE.saves[0].battleRegistrationData, 40);
	for (i = 0; i < 0x80; i++) { /* the item keeper's stock: a count per item id */
		if (SCRIPT_STATE_PTR->smth[0x54 + i] != 0 && ITEM_PARA[i].name[0] == '\0') {
			SCRIPT_STATE_PTR->smth[0x54 + i] = 0;
		}
	}
	equipped = PARTNER_ENTITY.digimonEntity.stats.base.moves;
	for (i = 0; i < 4; i++) { /* equipped: an animation id, the technique is the species' list at that slot */
		anim = equipped[i];
		if (anim >= 0x2e && anim < 0x2e + 16 && isMoveAbsent(DIGI_PARA(SAVED_STATE.partnerType).moves[anim - 0x2e])) {
			equipped[i] = 0xff;
			SAVED_STATE.partnerStats.base.moves[i] = 0xff;
		}
	}
}

static int32_t readRecord16(uint8_t *p)
{
	return (int16_t)(p[0] | (p[1] << 8));
}

/* a record this build knows; an unknown one is skipped, an unknown required one (bit 15) makes the file unreadable */
static int32_t isRecordKnown(int32_t tag)
{
	switch (tag) {
	case SAVE_TAG_SCREENS:
	case SAVE_TAG_INVENTORY:
	case SAVE_TAG_PARTNER_TYPE:
	case SAVE_TAG_WREGS:
	case SAVE_TAG_REGISTERED:
	case SAVE_TAG_POOP_MAPS:
	case SAVE_TAG_SPECIES_RAISED:
	case SAVE_TAG_SPECIES_RECRUITED:
	case SAVE_TAG_OPTIONS:
	case SAVE_TAG_VANILLA_SWAP:
	case SAVE_TAG_GROUND_PICKED:
	case SAVE_TAG_GROUND_PICKUP_BITS:
		return 1;
	}
	return 0;
}

static int32_t hasUnreadableRecord(uint8_t *e)
{
	int32_t at = sizeof(SaveExtHead);
	int32_t tag;

	for (;;) {
		tag = e[at] | (e[at + 1] << 8);
		if (tag == SAVE_TAG_END) {
			return 0;
		}
		if ((tag & SAVE_TAG_REQUIRED) && !isRecordKnown(tag)) {
			return 1;
		}
		at += 4 + (e[at + 2] | (e[at + 3] << 8));
	}
}

/*
 * The records of a valid extension into the game state: each known record is copied as far as it and this build both
 * have room; what it does not cover keeps what the v1 part (or resetSpeciesFlags and the zeroed registers) gave it.
 */
static void applyRecords(uint8_t *e)
{
	int32_t at = sizeof(SaveExtHead);
	int32_t tag, len, i, n;
	uint8_t *d;

	for (;;) {
		tag = e[at] | (e[at + 1] << 8);
		len = e[at + 2] | (e[at + 3] << 8);
		d = e + at + 4;
		at += 4 + len;
		switch (tag) {
		case SAVE_TAG_END:
			return;
		case SAVE_TAG_SCREENS:
			if (len >= 2) {
				SAVED_STATE.currentScreen = readRecord16(d);
			}
			if (len >= 4) {
				SAVED_STATE.previousScreen = readRecord16(d + 2);
			}
			break;
		case SAVE_TAG_INVENTORY:
			for (i = 0; i < len / 2 && i < 30; i++) {
				INVENTORY.types.array[i] = readRecord16(d + 2 * i);
			}
			break;
		case SAVE_TAG_WREGS:
			for (i = 0; i < len / 2 && i < SAVE_EXT_SAVED_WREGS; i++) {
				SCRIPT_STATE_PTR->wregs[i] = readRecord16(d + 2 * i);
			}
			break;
		case SAVE_TAG_POOP_MAPS: /* MOD (screens beyond 255): authoritative (was kept for the tools) */
			for (i = 0; i < len / 2 && i < 100; i++) {
				WORLD_POOP[i].map = readRecord16(d + 2 * i);
			}
			break;
		case SAVE_TAG_SPECIES_RAISED:
			n = len < NUM_DIGIMON / 8 ? len : NUM_DIGIMON / 8;
			for (i = 0; i < n; i++) {
				SPECIES_RAISED_BITS[i] = d[i];
			}
			break;
		case SAVE_TAG_SPECIES_RECRUITED:
			n = len < NUM_DIGIMON / 8 ? len : NUM_DIGIMON / 8;
			for (i = 0; i < n; i++) {
				SPECIES_RECRUITED_BITS[i] = d[i];
			}
			break;
		case SAVE_TAG_VANILLA_SWAP:
			if (len >= 4) {
				MOD_SWAP_FROM = readRecord16(d);
				MOD_SWAP_TO = readRecord16(d + 2);
			}
			break;
		case SAVE_TAG_GROUND_PICKUP_BITS:
			n = len < sizeof(MOD_GROUND_PICKED) ? len : sizeof(MOD_GROUND_PICKED);
			for (i = 0; i < n; i++) {
				MOD_GROUND_PICKED[i] = d[i];
			}
			break;
		case SAVE_TAG_OPTIONS: /* MOD (options): what the save had, unless OPTIONS changed it since the start */
			MOD_SAVE_STORY = len >= 10 ? d[9] != 0 : 1;
			modApplyOptions(d, len);
			break;
		}
	}
}

/*
 * Loads copy 0 or 1 (isSaveCopyValid): the v1 payload, then
 * the extension when it is valid. 0 = a newer format this build cannot read.
 */
static int32_t loadSaveCopy(int32_t copy)
{
	SaveExtension *ext;
	int32_t i;

	if (!isSaveVersionReadable(getSaveVersion(&SAVE_FILE.saves[copy]))) {
		return 0;
	}
	if (copy != 0) {
		SAVE_FILE.saves[0] = SAVE_FILE.saves[1];
		for (i = 0; i < saveExtSize(); i++) {
			saveExtOf(0)[i] = saveExtOf(1)[i];
		}
	}
	if (isTaggedVersion(getSaveVersion(&SAVE_FILE.saves[0])) && isExtensionValid(0) &&
	    hasUnreadableRecord(saveExtOf(0))) {
		return 0; /* it holds something this build would silently drop */
	}
	/* No options record (original cards too): explicit upgrade defaults. */
	modApplyOptions(NULL, 0);
	loadSavegame(&SAVE_FILE.saves[0]);
	for (i = 0; i < SCRIPT_NUM_WREGS; i++) {
		SCRIPT_STATE_PTR->wregs[i] = 0;
	}
	resetSpeciesFlags();
	MOD_SWAP_FROM = MOD_SWAP_TO = 0;
	MOD_SAVE_STORY = 1; /* MOD (story): a save from before the option was played with the story the disc had */
	MOD_STORY_ARRIVE = 0;
	if (isExtensionValid(0)) {
		if (isTaggedVersion(getSaveVersion(&SAVE_FILE.saves[0]))) {
			applyRecords(saveExtOf(0));
		} else {
			ext = (SaveExtension *)saveExtOf(0);
			for (i = 0; i < NUM_DIGIMON / 8; i++) {
				SPECIES_RAISED_BITS[i] = ext->speciesRaised[i];
				SPECIES_RECRUITED_BITS[i] = ext->speciesRecruited[i];
			}
			SAVED_STATE.currentScreen = ext->currentScreen;
			SAVED_STATE.previousScreen = ext->previousScreen;
			for (i = 0; i < 30; i++) {
				INVENTORY.types.array[i] = ext->inventoryTypes[i];
			}
			for (i = 0; i < SAVE_EXT_SAVED_WREGS; i++) {
				SCRIPT_STATE_PTR->wregs[i] = ext->wregs[i];
			}
		}
	}
	dropAbsentSaveContent();
	modVanillaSwap();
	modStoryRelocate();
	return 1;
}

/* the size of the slot's file on the card (SAVE_FILE_NAME), one block if unknown */
static int32_t getSaveFileBytes(void)
{
	long count;

	if (MemCardGetDirentry(MEMORY_CARD_ID, SAVE_FILE_NAME, MEMCARD_DIRENTRIES, &count, 0, 1) == 0 &&
	    count == 1 && MEMCARD_DIRENTRIES[0].size >= SAVE_V2_BYTES) {
		return SAVE_V2_BYTES;
	}
	return SAVE_V1_BYTES;
}

/* the error of the read or write saveFileStart ran (a MEMORY_CARD_ERROR_MESSAGES index), else 0 */
static long SAVE_IO_FAILED;

/*
 * Starts writing SAVE_FILE (prepared) to the slot's file, as MemCardWriteFile.
 * A one-block file becomes two blocks first when the card has a free block.
 */
static int32_t writeSaveFile(void)
{
	long count;
	int32_t blocks;
	int32_t i;

	if (SAVE_FILE_BYTES < SAVE_V2_BYTES &&
	    MemCardGetDirentry(MEMORY_CARD_ID, MAIN_D_801346C0[1], MEMCARD_DIRENTRIES, &count, 0, 15) == 0) {
		blocks = 0;
		for (i = 0; i < count; i++) {
			blocks += (MEMCARD_DIRENTRIES[i].size + 0x1FFF) / 0x2000;
		}
		if (blocks < 15 && MemCardDeleteFile(MEMORY_CARD_ID, SAVE_FILE_NAME) == 0) {
			if (MemCardCreateFile(MEMORY_CARD_ID, SAVE_FILE_NAME, SAVE_V2_BYTES / SAVE_V1_BYTES) == 0) {
				SAVE_FILE_BYTES = SAVE_V2_BYTES;
			} else {
				MemCardCreateFile(MEMORY_CARD_ID, SAVE_FILE_NAME, 1);
			}
		}
	}
	if (SAVE_FILE_BYTES < SAVE_V2_BYTES) {
		/* Never report success after dropping history, recruits or wide IDs. */
		SAVE_IO_FAILED = 7;
		return 1;
	}
	SAVE_FILE.blockNumber = SAVE_FILE_BYTES / SAVE_V1_BYTES;
	return saveFileStart(1);
}

/*
 * MOD (save extension area, save robustness): reads or writes the slot's file (SAVE_FILE_BYTES), waiting
 * for each part. A v1 file is one part. A v2 or v3 file is four, in copy order: copy 1's payload
 * (file 0x1100-0x2000, from SAVE_FILE), its extension (0x2000 + E, from SAVE_EXT_ADDR + E), then the header
 * and copy 0's payload (0-0x1100), its extension (0x2000). A copy needs its payload and, when the payload's
 * checksum is inverted, its extension: a write that stops anywhere (Reset, power, a card pulled) leaves one
 * copy wholly old or wholly new, which pickSaveCopy finds. Copy 1 goes first: in a file that has just grown
 * the old file's blocks still hold copy 0 and its extension where the old layout had them (a v2 file's
 * 0x2000) until the last part. Offsets and sizes are multiples of 128, as libmcrd needs. Always 1
 * (started, as MemCardWriteFile); the error, if any, is what saveFileSync reports.
 */
static long saveFileStart(int32_t write)
{
	MemCardSyncWord command;
	MemCardSyncWord result;
	int32_t ext;
	int32_t part;
	int32_t copy;
	int32_t offset;
	int32_t size;
	uint8_t *ram;

	SAVE_IO_FAILED = 0;
	ext = (SAVE_FILE_BYTES - SAVE_V1_BYTES) / 2; /* one extension; 0 for a v1 file */
	for (part = 0; part < 4; part++) {
		copy = part < 2;
		if (part & 1) {
			offset = SAVE_V1_BYTES + copy * ext;
			ram = (uint8_t *)SAVE_EXT_ADDR + copy * ext;
			size = ext;
		} else {
			offset = copy * __builtin_offsetof(SaveFile, saves[1]);
			ram = (uint8_t *)&SAVE_FILE + offset;
			size = ext == 0 ? SAVE_V1_BYTES : copy ? sizeof(SavegamePayload) : __builtin_offsetof(SaveFile, saves[1]);
		}
		if (ext == 0 && part != 2) {
			continue;
		}
		result = SAVE_IO_ERROR;
		if ((write ? MemCardWriteFile(MEMORY_CARD_ID, SAVE_FILE_NAME, (void *)ram, offset, size)
			   : MemCardReadFile(MEMORY_CARD_ID, SAVE_FILE_NAME, (void *)ram, offset, size)) == 1) {
			MemCardSync(0, (long *)&command, (long *)&result);
		}
		if (result != 0) {
			/* a part that did not start, or a libmcrd code past the messages: "Not Inserted" */
			SAVE_IO_FAILED = (unsigned long)result < SAVE_IO_MESSAGES ? (long)result : SAVE_IO_ERROR;
			break;
		}
	}
	return 1;
}

/* what saveFileStart did, as MemCardSync(0, ...) after a MemCardReadFile/MemCardWriteFile: result 0 = done */
static void saveFileSync(MemCardSyncWord *command, MemCardSyncWord *result)
{
	*result = SAVE_IO_FAILED;
}

/*
 * MOD (save robustness): the copy to load, 0 or 1, or -1: one whose extension links to it first, then one
 * that is valid without it (a save last written by the original executable, which leaves a stale
 * extension). After a write that stopped partway the copy whose extension still links is the whole one.
 */
static int32_t pickSaveCopy(void)
{
	int32_t i;

	for (i = 0; i < 4; i++) {
		if (isSaveCopyValid(i & 1) && (i >= 2 || isExtensionValid(i & 1))) {
			return i & 1;
		}
	}
	return -1;
}

int32_t countRegisteredDigimon(RegisteredDigimon *p)
{
	int32_t i;
	int32_t count;

	count = 0;
	i = 0;
	while (i < 0x28) {
		if (p->digimonId != 0) {
			count++;
		}
		p++;
		i++;
	}
	return count;
}

void renderMainMenu(void)
{
	int32_t menu;
    if (MOD_LANGUAGE_ERROR) modRenderLanguageError();
	if (MOD_BOOT_LANGUAGE && CURRENT_MENU == 0) {
		modRenderLanguagePicker();
		return;
	}

	if (CURRENT_MENU >= 0) {
		menu = MENU_VIEWS[CURRENT_MENU];
		if (menu != -1) {
			renderMenuSelector(&MENU_HIGHLIGHTS[menu]);
		}
		switch (menu) {
		case 0:
			renderInitialMenu();
			break;
		case 1:
			renderSelectNewGameCard();
			break;
		case 2:
		case 4:
			renderSelectCardSlot();
			break;
		case 5:
			renderCheckingCard();
			break;
		case 6:
			renderConfirmNoCard();
			break;
		case 7:
			renderSelectSlot();
			break;
		case 8:
			renderConfirmSlotSelection();
			break;
		case 9:
			renderFormatMemoryCard();
			break;
		case 10:
			renderMemoryCardReadError();
			break;
		case 11:
			renderSleepMenu();
			break;
		case 12:
			renderConfirmOverwrite();
			break;
		case 13:
			renderConfirmVSSlot();
			break;
		case 14:
			renderRegisterDigimon();
			break;
		case 15:
			renderSelectRegisterSlot();
			break;
		case 16:
			renderConfirmRegister();
			break;
		case 17:
			renderCantRegister();
			break;
		case 18:
			renderDoYouWantToSave();
			break;
		case 19:
			renderCantRegisterBaby();
			break;
		case 22: /* MOD (options) */
			renderOptionsMenu();
			break;
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
		case 20:
			renderBattleModeMenu();
			break;
		case 21:
			renderInsertGameCardPrompt();
			break;
#endif
		}
	}
}

/* MOD (wide registered species): the record's byte keeps a species below 256 and, past it, the original it is based
 * on (what older builds and the original game read); wideId keeps the species itself */
static void setRegisteredSpecies(RegisteredDigimon *rec, int32_t type)
{
	rec->digimonId = type < 256 ? type : getOriginalType(type);
	rec->wideId = type;
	rec->wideCheck = rec->digimonId;
	rec->wideTag = REGISTERED_WIDE_TAG;
    memcpy(rec->name, PARTNER_ENTITY.name, 0xe);
    if (MOD_LANG == MOD_LANG_ZH) zhRegisterName(rec->name, type);
    rec->name[0xd] = '\0';
    /* The fixed 13-byte nickname field must end on a whole character. */
    {
        int32_t end = 0;
        while (end < 13 && rec->name[end]) end += isAsciiEncoded(rec->name + end) ? 1 : 2;
        if (end > 13) rec->name[12] = '\0';
    }
}

void registerBattleData(void)
{
	RegisteredDigimon *rec;

	rec = &SAVE_FILE.saves[0].battleRegistrationData[BATTLE_REGISTRATION_SLOT];
	rec->hp = PARTNER_ENTITY.digimonEntity.stats.base.hp;
	rec->mp = PARTNER_ENTITY.digimonEntity.stats.base.mp;
	rec->offense = PARTNER_ENTITY.digimonEntity.stats.base.off;
	rec->defense = PARTNER_ENTITY.digimonEntity.stats.base.def;
	rec->speed = PARTNER_ENTITY.digimonEntity.stats.base.speed;
	rec->brains = PARTNER_ENTITY.digimonEntity.stats.base.brain;
	rec->discipline = PARTNER_PARA.discipline;
	setRegisteredSpecies(rec, PARTNER_ENTITY.digimonEntity.entity.type); /* MOD (scale, wide registered species) */
	memcpy(rec->moves, PARTNER_ENTITY.digimonEntity.stats.base.moves, 3);
}

void renderMenuSelector(MenuHighlight *b)
{
	LINE_F3 *prim;
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;

	x = b->x - 1;
	y = (b->y + (b->rowHeight * b->pos)) - 2;
	w = b->w + 1;
	h = b->h + 2;
	x -= 0xA0;
	y -= 0x78;
	prim = (LINE_F3 *)GsGetWorkBase();
	SetLineF3(prim);
	setRGB0(prim, 0xA0, 0xA0, 0);
	setXY3(prim, x, y, x + w, y, x + w, y + h);
	AddPrim(ACTIVE_ORDERING_TABLE->org, prim++);
	SetLineF3(prim);
	setRGB0(prim, 0xA0, 0xA0, 0);
	setXY3(prim, x + w, y + h, x, y + h, x, y);
	AddPrim(ACTIVE_ORDERING_TABLE->org, prim++);
	GsSetWorkBase((PACKET *)prim);
}

/*
 * MOD (menu overlay, docs/mods/memory.md "The menu overlay"): most of this file's code is linked into MENU_REL.BIN at
 * OVL_BTL_BASE (config/us/main.ld keeps the rest in the executable) and read where a card menu opens: the title
 * (runMainMenu), the sleep save (partner.c), the save machine and the game-clear save (below). Nothing of a battle,
 * the tournament or VS is loaded then; KAR (0x80053800) is, on the curling screen, so a menu closing in the world
 * reads it again.
 */
void loadMenuOverlay(void)
{
	loadDynamicLibrary(MENU_REL, NULL, 0, NULL, NULL);
}

/* The title shares EVL's spare tail with its picker. Load EVL first: MENU's
 * last sector may replace its unused first sector. World saves load MENU only. */
void loadTitleMenuOverlays(void)
{
	loadDynamicLibrary(EVL_REL, NULL, 0, NULL, NULL);
	loadMenuOverlay();
}

void closeMenuOverlay(void)
{
	checkCurlingMap(CURRENT_SCREEN);
}

/* a VS battle from the title's menus: VS_REL is read where this overlay is, so the battle runs from here, in the
 * executable, and the overlay is read again before the return into tickMainMenu (the same code at the same place) */
void modMenuVsBattle(int32_t trial)
{
	uint8_t complete;

	removeObject(0x1388, 0);
	removeObject(0xFA3, 0);
	loadDynamicLibrary(VS_REL, &complete, 0, 0, 0);
#if VERSION_IS(JP_TRIAL) || VERSION_IS(JP_BOMBOM)
	if (trial) {
		VS_initializeTrialBattle(VS__REGISTERED_DIGIMON_BUFFER, &VS__REGISTERED_DIGIMON_BUFFER[0x28]);
	} else
#else
	(void)trial;
#endif
	{
		VS__initialize(VS__REGISTERED_DIGIMON_BUFFER, &VS__REGISTERED_DIGIMON_BUFFER[0x28]);
	}
	loadTitleMenuOverlays();
	addObject(0xFA3, 0, NULL, (RenderFunction)renderMainMenuBackground);
	addObject(0x1388, 0, (TickFunction)tickMainMenu, (RenderFunction)renderMainMenu);
}

void openSaveMachine(void)
{
	loadMenuOverlay();
	CHECKED_MEMORY_CARD = 0x10;
	CURRENT_MENU = -2;
	MAIN_STATE = 1;
	TARGET_MENU = 0x3C;
	if (DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type).level < 3) {
		TARGET_MENU = 0x46;
	}

	if ((MEMORY_CARD_ID == -1) || (MEMORY_CARD_SLOT == -1)) {
		TARGET_MENU = 0x44;
	}
}

/* MOD (fps): the save machine menu is drawn from a script tick, into the first
 * render pass only; modReplayMainMenu draws it again on the extra passes, from
 * the same state (renderSelectSlot scrolls the slot list as it draws). */
extern int32_t MOD_TICK_PRIM_MAIN_MENU;
static int32_t modMainMenuScrollBefore;

void modReplayMainMenu(void)
{
	int32_t scroll;

	scroll = SAVE_SLOT_SCROLL_TENTHS;
	SAVE_SLOT_SCROLL_TENTHS = modMainMenuScrollBefore;
	renderMainMenu();
	SAVE_SLOT_SCROLL_TENTHS = scroll;
}

int32_t tickSaveMachine(void)
{
	tickMainMenu();
	modMainMenuScrollBefore = SAVE_SLOT_SCROLL_TENTHS;
	renderMainMenu();
	MOD_TICK_PRIM_MAIN_MENU = 1; /* MOD (fps): drawn again on extra passes */
	if (CURRENT_MENU == -1) {
		MOD_TICK_PRIM_MAIN_MENU = 0; /* MOD (menu overlay): no extra pass draws it once it may be gone */
		closeMenuOverlay();
		return 1;
	}

	return 0;
}

int16_t MOD_VS_UNLOCK[5] = { -1, -1, -1, -1, -1 };

void awardMachinedramonData(void)
{
	RegisteredDigimon *rec;

	/* MOD (rules as data): the species, techniques and slot come from MOD_VS_UNLOCK (-1: nothing is unlocked) */
	if (MOD_VS_UNLOCK[0] < 0 || MOD_VS_UNLOCK[4] < 0) {
		return;
	}
	rec = &SAVE_FILE.saves[0].battleRegistrationData[MOD_VS_UNLOCK[4]];
	rec->hp = PARTNER_ENTITY.digimonEntity.stats.base.hp;
	rec->mp = PARTNER_ENTITY.digimonEntity.stats.base.mp;
	rec->offense = PARTNER_ENTITY.digimonEntity.stats.base.off;
	rec->defense = PARTNER_ENTITY.digimonEntity.stats.base.def;
	rec->speed = PARTNER_ENTITY.digimonEntity.stats.base.speed;
	rec->brains = PARTNER_ENTITY.digimonEntity.stats.base.brain;
	rec->discipline = PARTNER_PARA.discipline;
	setRegisteredSpecies(rec, MOD_VS_UNLOCK[0]); /* MOD (wide registered species) */
	rec->moves[0] = MOD_VS_UNLOCK[1];
	rec->moves[1] = MOD_VS_UNLOCK[2];
	rec->moves[2] = MOD_VS_UNLOCK[3];
}

void gameClearSave(void)
{
	/*
	 * MOD (save robustness): the ending's sound bank (load_ending, script routine 0x34) may still be loading
	 * into GENERAL_BUFFER: wait for it, as start_ending (0x35) does, so no CD read or SPU upload runs next to
	 * the card write and its extensions at SAVE_EXT_ADDR.
	 */
	isSoundLoaded(0, 8);
	loadMenuOverlay();
	CHECKED_MEMORY_CARD = 0x10;
	CURRENT_MENU = -2;
	MAIN_STATE = 2;
	TARGET_MENU = 0x45;
	CURRENT_SCREEN = getFileCityTopMap();
	awardMachinedramonData();
	if ((MEMORY_CARD_ID == -1) || (MEMORY_CARD_SLOT == -1)) {
		TARGET_MENU = -1;
	}
}

int32_t scriptTickMainMenu(void)
{
	tickMainMenu();
	modMainMenuScrollBefore = SAVE_SLOT_SCROLL_TENTHS;
	renderMainMenu();
	MOD_TICK_PRIM_MAIN_MENU = 1; /* MOD (fps): drawn again on extra passes */
	if (CURRENT_MENU == -1) {
		MOD_TICK_PRIM_MAIN_MENU = 0; /* MOD (menu overlay): no extra pass draws it once it may be gone */
		closeMenuOverlay();
		return 1;
	}

	return 0;
}

void renderInitialMenu(void)
{
	POLY_FT4 *cur;

	cur = (POLY_FT4 *)GsGetWorkBase();
#if !VERSION_IS(US)
	renderText(cur++, 0x5e, 0x37, 0, 0, 0x84, 0xc, 0);
	if (CONNECTED_CARDS != 0) {
		renderText(cur++, 0x5e, 0x43, 0, 0xc, 0x84, 0x18, 0);
	} else {
		renderText(cur++, 0x5e, 0x43, 0, 0xc, 0x84, 0x18, 1);
	}
	renderText(cur++, 0x5e, 0x5b, 0, 0x24, 0x84, 0xc, 0);
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x54, 0x32, 0x98, 0x3a);
#else
	renderText(cur++, 0x52, 0x37, 0, 0, 0xB0, 0xC, 0);
	if (CONNECTED_CARDS != 0) {
		renderText(cur++, 0x52, 0x43, 0, 0xC, 0xB0, 0x18, 0);
	} else {
		renderText(cur++, 0x52, 0x43, 0, 0xC, 0xB0, 0x18, 1);
	}
	renderText(cur++, 0x52, 0x5B, 0, 0x24, 0xB0, 0xC, 0);
	renderText(cur++, 0x52, 0x67, 0, 0x30, 0xB0, 0xC, 0); /* MOD (options): OPTIONS */
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(0x48, 0x32, 0xB0, 0x46); /* MOD (options): was 0x3A high */
	modVersionRender(); /* MOD (version) */
#endif
}

/*
 * MOD (options): the box's rows, each a label and its values side by side (two, three for FONT); the chosen value is
 * drawn yellow and the others grey, so the row reads as a switch. Left and right step through the values, confirm takes
 * the next one (round). With the original font the values do not fit the screen beside "JAPANESE CONTROLS" (200 pixels,
 * and the values): a row then shows its chosen value only, as the original game's own settings do. When even that
 * does not fit with the labels and the values in two columns (the widest label beside the widest value, "TRANSLATED":
 * 342 pixels), each row's value ends at the box's right edge instead, so only a row's own label and values count.
 */
#define MOD_OPT_VALUES	MOD_LANGS	/* the most values a row has */
#define MOD_OPT_VALUE_Y	(MOD_OPT_ROWS * 0xC)	/* the values' rows in the text area: under the labels, two values a row */
#define MOD_OPT_VALUE_U	0x80	/* the second value of a row (the area's 256 rows hold 12 labels and 12 values no other way) */
#define MOD_OPT_GAP	8	/* pixels between the labels' column and the values', and between two values */
#define MOD_OPT_MARGIN	8	/* the box's frame to its text */

static char *MOD_OPT_TEXT[MOD_OPT_ROWS][3] = {
	{ MOD_OPT_STR_JP, MOD_OPT_STR_OFF, MOD_OPT_STR_ON },
	{ MOD_OPT_STR_EVO, MOD_OPT_STR_OFF, MOD_OPT_STR_ON },
	{ MOD_OPT_STR_NEWDIGI, MOD_OPT_STR_OFF, MOD_OPT_STR_ON },
	{ MOD_OPT_STR_STORY, MOD_OPT_STR_OFF, MOD_OPT_STR_ON }, /* MOD (story) */
	{ MOD_OPT_STR_DIFF, MOD_OPT_STR_EASY, MOD_OPT_STR_NORMAL }, /* MOD (difficulty) */
	{ MOD_OPT_STR_QUICK, MOD_OPT_STR_OFF, MOD_OPT_STR_ON }, /* MOD (quick battle start) */
#if FONT_SWITCH
	{ MOD_OPT_STR_FONT, MOD_OPT_STR_NEW, MOD_OPT_STR_CLASSIC },
#endif
	{ MOD_OPT_STR_LANG }, /* MOD (language) */
	{ MOD_OPT_STR_SIGNS, MOD_OPT_STR_ORIGINAL, MOD_OPT_STR_TRANSLATED },
	{ MOD_OPT_STR_CHEATS, MOD_OPT_STR_OFF, MOD_OPT_STR_ON },
};
/* the text area place of each value, its row + 0x100 in the right half: a value two rows share (OFF, ON, ORIGINAL) is
 * drawn once (drawOptionsText) */
/* FONT's third value uses language slot 0; HARD uses language slot 1.
 * Language values have separate shared storage, so neither adds resident arrays. */
typedef char mod_options_hard_slot[(MOD_OPT_ROW_LANG > MOD_OPT_ROW_DIFF) ? 1 : -1];
#if FONT_SWITCH
typedef char mod_options_font_slot[(MOD_OPT_ROW_LANG == MOD_OPT_ROW_FONT + 1) ? 1 : -1];
#endif
static int16_t MOD_OPT_V[MOD_OPT_ROWS * 2];
static int16_t MOD_OPT_LANG_V;
/* the pixels of each label and value as the font draws them (drawOptionsText) */
static uint8_t MOD_OPT_W[MOD_OPT_ROWS * 3];
static uint8_t MOD_OPT_LANG_W[1 + MOD_LANGS];
extern int32_t MOD_COLD_CACHE_TYPE;
extern uint32_t MOD_LANGUAGE_AVAILABLE;
static char *MOD_LANGUAGE_NAMES[MOD_LANGS] = { MOD_OPT_STR_ENGLISH, MOD_OPT_STR_SPANISH, MOD_OPT_STR_PORTUGUESE, MOD_OPT_STR_FRENCH, MOD_OPT_STR_GERMAN, MOD_OPT_STR_ITALIAN, MOD_OPT_STR_JAPANESE, MOD_OPT_STR_CATALAN, "" };

/* Language values share one texture position; the other option rows have at most three values. */
static char *modOptionText(int32_t row, int32_t value)
{
    if (row == MOD_OPT_ROW_DIFF && value == 3) return MOD_OPT_STR_HARD;
    if (row == MOD_OPT_ROW_LANG && value > 0) return MOD_LANGUAGE_NAMES[modLanguageAt(value - 1)];
#if FONT_SWITCH
    if (row == MOD_OPT_ROW_FONT && value == 3) return MOD_OPT_STR_ORIGINAL;
#endif
    return MOD_OPT_TEXT[row][value];
}

static int16_t *modOptionV(int32_t row, int32_t value)
{
    if (row == MOD_OPT_ROW_DIFF && value == 2) return &MOD_OPT_V[MOD_OPT_ROW_LANG * 2 + 1];
    return row == MOD_OPT_ROW_LANG ? &MOD_OPT_LANG_V : &MOD_OPT_V[row * 2 + value];
}

static uint8_t *modOptionW(int32_t row, int32_t value)
{
    if (row == MOD_OPT_ROW_DIFF && value == 3) return &MOD_OPT_W[MOD_OPT_ROW_LANG * 3 + 1];
    return row == MOD_OPT_ROW_LANG ? &MOD_OPT_LANG_W[value] : &MOD_OPT_W[row * 3 + value];
}

/* the box's layout in the font that draws it (drawOptionsText measures it): the labels' column on the screen, its
 * width, a value's, how many values a row shows (all, or 1 with the original font, 0 for 1 at the right edge: above)
 * and the text's width */
static int16_t MOD_OPT_TEXT_X;
static int16_t MOD_OPT_TEXT_W;
static int16_t MOD_OPT_LABEL_W;
static int16_t MOD_OPT_VALUE_W;
static int16_t MOD_OPT_SHOWN;

static int32_t modOptionCount(int32_t row)
{
    if (row == MOD_OPT_ROW_DIFF) return 3;
	if (row == MOD_OPT_ROW_LANG) {
        return modLanguageCount();
    }
#if FONT_SWITCH
    if (row == MOD_OPT_ROW_FONT) return MOD_LANG == MOD_LANG_ZH ? 1 : 3;
#endif
    return 2;
}

/* The index of the row's chosen value. */
static int32_t modOptionValue(int32_t row)
{
	switch (row) {
	case 0:
		return MOD_OPT_BUTTONS == MOD_BUTTONS_JP;
	case 1:
		return MOD_OPT_EVO_GUIDE != 0;
	case 2:
		return MOD_OPT_NEW_DIGIMON != 0;
	case MOD_OPT_ROW_STORY: /* MOD (story) */
		return MOD_OPT_STORY != 0;
	case MOD_OPT_ROW_DIFF: /* MOD (difficulty): EASY, NORMAL */
		return MOD_OPT_DIFFICULTY == MOD_DIFF_EASY ? 0 : MOD_OPT_DIFFICULTY == MOD_DIFF_HARD ? 2 : 1;
	case MOD_OPT_ROW_QUICK: /* MOD (quick battle start) */
		return MOD_OPT_QUICK_BATTLE != 0;
#if FONT_SWITCH
	case MOD_OPT_ROW_FONT: /* NEW, CLASSIC, ORIGINAL */
		return MOD_FONT == MOD_FONT_NEW ? 0 : MOD_FONT == MOD_FONT_CLASSIC ? 1 : 2;
#endif
	case MOD_OPT_ROW_LANG: /* MOD (language) */
		{
            int32_t index;
            for (index = 0; index < modLanguageCount(); index++) {
                if (modLanguageAt(index) == MOD_LANG) return index;
            }
            return 0;
        }
	case MOD_OPT_ROW_CHEATS:
		return MOD_OPT_CHEATS != 0;
	case MOD_OPT_ROW_SIGNS:
		return MOD_OPT_SIGNS;
	}
	return 0;
}

static void modSetOption(int32_t row, int32_t value)
{
    if (row == MOD_OPT_ROW_LANG && modLanguageAt(value) == MOD_LANG_ZH && MOD_COLD_CACHE_TYPE < 0) {
        modShowLanguageError();
        return;
    }
	MOD_OPT_TOUCHED = 1;
	switch (row) {
	case 0:
		modSetButtons(value ? MOD_BUTTONS_JP : MOD_BUTTONS_US);
		break;
	case 1:
		MOD_OPT_EVO_GUIDE = value;
		break;
	case 2:
		MOD_OPT_NEW_DIGIMON = value;
		modApplySpeciesLevels(); /* MOD (canon levels) */
		break;
	case MOD_OPT_ROW_STORY: /* MOD (story) */
		modSetStory(value);
		break;
	case MOD_OPT_ROW_DIFF: /* MOD (difficulty) */
		MOD_OPT_DIFFICULTY = value == 0 ? MOD_DIFF_EASY : value == 2 ? MOD_DIFF_HARD : MOD_DIFF_NORMAL;
		break;
	case MOD_OPT_ROW_QUICK: /* MOD (quick battle start) */
		MOD_OPT_QUICK_BATTLE = value;
		break;
#if FONT_SWITCH
	case MOD_OPT_ROW_FONT:
		modSetFont(value == 0 ? MOD_FONT_NEW : value == 1 ? MOD_FONT_CLASSIC : MOD_FONT_ORIGINAL);
		break;
#endif
	case MOD_OPT_ROW_LANG: /* MOD (language) */
		modSetLanguage(modLanguageAt(value));
		break;
	case MOD_OPT_ROW_CHEATS:
		MOD_OPT_CHEATS = value != 0;
		break;
	case MOD_OPT_ROW_SIGNS:
		modSetSigns(value);
		break;
	}
}

/*
 * The labels in the text area's first rows, the values in bands under them (the original font's "JAPANESE CONTROLS"
 * takes 200 of its 256 pixels), and the box laid out on the widest of each, centred on the screen.
 */
static void drawOptionsText(void)
{
	MenuHighlight *bar;
	int32_t r;
	int32_t k;
	int32_t w;

	int32_t y;
	int32_t r2;
	int32_t k2;

    MOD_OPT_LABEL_W = 0;
	MOD_OPT_VALUE_W = 0;
	y = MOD_OPT_VALUE_Y << 1; /* the value's place times two, its half in the low bit */
	for (r = 0; r < MOD_OPT_ROWS; r++) {
		for (k = 0; k <= modOptionCount(r); k++) {
			if (r == MOD_OPT_ROW_LANG && k > 0) {
				(*modOptionV(r, k - 1)) = (y >> 1) + ((y & 1) << 8);
				if (k - 1 == modOptionValue(r)) modDrawLanguageName(modLanguageAt(k - 1), (y & 1) * MOD_OPT_VALUE_U, y >> 1);
				if (k == modOptionCount(r)) y += (y & 1) ? 0xC * 2 - 1 : 1;
				goto drawn;
			}
			if (k > 0) { /* MOD (language): a value an earlier row has is drawn once */
				for (r2 = 0; r2 <= r; r2++) {
					for (k2 = 1; k2 <= modOptionCount(r2) && (r2 < r || k2 < k); k2++) {
						if (!strcmp(modOptionText(r2, k2), modOptionText(r, k))) {
							goto shared;
						}
					}
				}
				(*modOptionV(r, k - 1)) = (y >> 1) + ((y & 1) << 8);
				drawString(modOptionText(r, k), (y & 1) * MOD_OPT_VALUE_U, y >> 1);
				y += (y & 1) ? 0xC * 2 - 1 : 1;
				goto drawn;
			shared:
				(*modOptionV(r, k - 1)) = (*modOptionV(r2, k2 - 1));
				(*modOptionW(r, k)) = (*modOptionW(r2, k2));
				continue;
			}
			drawString(modOptionText(r, k), 0, r * 0xC);
		drawn:
			DrawSync(0);
			w = r == MOD_OPT_ROW_LANG && k > 0 && modLanguageAt(k - 1) == MOD_LANG_ZH ? 25 : measureStringMode(modOptionText(r, k), FIT_VANILLA) + 1; /* and the shadow */
			(*modOptionW(r, k)) = w < 0xFF ? w : 0xFF;
			if (k == 0) {
				MOD_OPT_LABEL_W = w > MOD_OPT_LABEL_W ? w : MOD_OPT_LABEL_W;
			} else {
				MOD_OPT_VALUE_W = w > MOD_OPT_VALUE_W ? w : MOD_OPT_VALUE_W;
			}
		}
	}
	MOD_OPT_SHOWN = MOD_OPT_VALUES;
	w = MOD_OPT_LABEL_W + MOD_OPT_SHOWN * (MOD_OPT_GAP + MOD_OPT_VALUE_W);
	if (w + 2 * MOD_OPT_MARGIN > 0x140 - 0x10) {
		MOD_OPT_SHOWN = 1;
		w = MOD_OPT_LABEL_W + MOD_OPT_GAP + MOD_OPT_VALUE_W;
	}
	if (w + 2 * MOD_OPT_MARGIN > 0x140 - 0x10) { /* each row as wide as its own label and longest value */
		MOD_OPT_SHOWN = 0;
		w = 0;
		for (r = 0; r < MOD_OPT_ROWS; r++) {
			k2 = 0;
			for (k = 1; k <= modOptionCount(r); k++) {
				k2 = (*modOptionW(r, k)) > k2 ? (*modOptionW(r, k)) : k2;
			}
			k2 += (*modOptionW(r, 0)) + MOD_OPT_GAP;
			w = k2 > w ? k2 : w;
		}
	}
	MOD_OPT_TEXT_W = w;
	MOD_OPT_TEXT_X = (0x140 - w) / 2;
	bar = &MENU_HIGHLIGHTS[22];
	bar->x = MOD_OPT_TEXT_X - MOD_OPT_MARGIN + 5;
	bar->w = w + 2 * MOD_OPT_MARGIN - 10;
}

static void renderOptionsMenu(void)
{
	POLY_FT4 *cur;
	int32_t r;
	int32_t k;
	int32_t x;
	int32_t y;
	int32_t value;

	cur = (POLY_FT4 *)GsGetWorkBase();
	for (r = 0; r < MOD_OPT_ROWS; r++) {
		y = 0x37 + r * 0xC;
		value = modOptionValue(r);
		renderText(cur++, MOD_OPT_TEXT_X, y, 0, r * 0xC, MOD_OPT_SHOWN ? MOD_OPT_LABEL_W : (*modOptionW(r, 0)), 0xC, 0);
		x = MOD_OPT_TEXT_X + MOD_OPT_LABEL_W + MOD_OPT_GAP;
		if (MOD_OPT_SHOWN == 0) { /* the chosen value at the right edge */
			value = value > 0 ? value : 0;
			k = (*modOptionV(r, value));
			renderText(cur++, MOD_OPT_TEXT_X + MOD_OPT_TEXT_W - (*modOptionW(r, 1 + value)), y, (k >> 8) * MOD_OPT_VALUE_U,
				   k & 0xFF, (*modOptionW(r, 1 + value)), 0xC, modOptionValue(r) >= 0 ? 2 : 1);
			continue;
		}
		if (MOD_OPT_SHOWN == 1) { /* the chosen value (the first one, grey, when none is) */
			k = (*modOptionV(r, value > 0 ? value : 0));
			renderText(cur++, x, y, (k >> 8) * MOD_OPT_VALUE_U, k & 0xFF, MOD_OPT_VALUE_W, 0xC, value >= 0 ? 2 : 1);
			continue;
		}
		for (k = 0; k < modOptionCount(r); k++) {
			renderText(cur++, x, y, ((*modOptionV(r, k)) >> 8) * MOD_OPT_VALUE_U, (*modOptionV(r, k)) & 0xFF, MOD_OPT_VALUE_W,
				   0xC, value == k ? 2 : 1);
			x += MOD_OPT_VALUE_W + MOD_OPT_GAP;
		}
	}
	GsSetWorkBase((PACKET *)cur);
	renderMenuBox(MOD_OPT_TEXT_X - MOD_OPT_MARGIN, 0x32,
		      MOD_OPT_TEXT_W + 2 * MOD_OPT_MARGIN,
		      0x16 + (MOD_OPT_ROWS - 1) * 0xC);
}

/* First boot: native language names and small procedural flags. Preferences are
 * remembered by the existing save-header options, with no extra card writes. */
static __attribute__((noinline)) void modDrawLanguagePicker(void)
{
    int32_t i, lang;
    clearTextArea();
    for (i = 0; i < modLanguageCount(); i++) {
        lang = modLanguageAt(i);
        modDrawLanguageName(lang, (i & 1) * 128, 16 + (i / 2) * 20);
        DrawSync(0);
        modDrawLanguageName(lang + MOD_LANGS, (lang & 1) * 128, 120 + (lang / 2) * 12);
        DrawSync(0);
    }
    drawString("START", 0, 188);
    DrawSync(0);
}

static __attribute__((noinline)) void modTickLanguagePicker(void)
{
    int32_t count = modLanguageCount();
    int32_t cursor = MOD_BOOT_LANGUAGE_CURSOR;
    int32_t column = cursor & 1;
    int32_t chosen;
    if (CHANGED_INPUT & 0x1000) {
        cursor -= 2;
        if (cursor < 0) cursor = ((count - 1 - column) / 2) * 2 + column;
    } else if (CHANGED_INPUT & 0x4000) {
        cursor += 2;
        if (cursor >= count) cursor = column;
    } else if (CHANGED_INPUT & 0xA000) {
        if ((cursor ^ 1) < count) cursor ^= 1;
    } else if (CHANGED_INPUT & (CONFIRM_BUTTON | 0x800)) {
        chosen = modLanguageAt(cursor);
        if (chosen == MOD_LANG_ZH && MOD_COLD_CACHE_TYPE < 0) {
            modShowLanguageError();
            return;
        }
        modSetLanguage(chosen);
        if (MOD_LANG != chosen) return;
        MOD_OPT_TOUCHED = 1;
        MOD_BOOT_LANGUAGE = 0;
        playSound(0, 3);
        drawMainMenuStrings(0);
        return;
    }
    if (cursor != MOD_BOOT_LANGUAGE_CURSOR) playSound(0, 2);
    MOD_BOOT_LANGUAGE_CURSOR = cursor;
}

static __attribute__((noinline)) void modFlagRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t rgb)
{
	TILE *tile = (TILE *)GsGetWorkBase();
	setTile(tile);
	setRGB0(tile, rgb >> 16, (rgb >> 8) & 255, rgb & 255);
	setXY0(tile, x - 160, y - 120);
	setWH(tile, w, h);
	AddPrim(ACTIVE_ORDERING_TABLE->org, tile++);
	GsSetWorkBase((PACKET *)tile);
}

#include "language_flags.h"

/* Names and headings share the licensed resident 12px pairs. The heading
 * selector is the language plus MOD_LANGS; no language bank is loaded on hover. */
static void __attribute__((noinline)) modDrawLanguageName(int32_t lang, int32_t x, int32_t y)
{
    uint32_t pixels[36];
    RECT rect;
    int32_t row, col;
    const uint16_t *glyphs = MOD_CHINESE_NAME;
    if (lang >= MOD_LANGS) {
        lang -= MOD_LANGS;
        glyphs = MOD_LANGUAGE_TITLE_GLYPHS + (lang == MOD_LANG_ZH ? 24 : 0);
        if (lang != MOD_LANG_JA && lang != MOD_LANG_ZH) {
            drawString(MOD_LANGUAGE_TITLES[lang], x, y);
            return;
        }
    } else if (lang != MOD_LANG_ZH) {
        drawString(MOD_LANGUAGE_NAMES[lang], x, y);
        return;
    }
    memset(pixels, 0, sizeof(pixels));
    for (row = 0; row < 12; row++) {
        for (col = 0; col < 24; col++) {
            if (!(glyphs[(col / 12) * 12 + row] & (0x8000 >> (col % 12))))
                pixels[row * 3 + col / 8] |= 1u << ((col % 8) * 4);
        }
    }
    setRECT(&rect, TEXT_VRAM_X + x / 4, TEXT_VRAM_Y + y, 6, 12);
    DrawSync(0);
    LoadImage(&rect, (u_long *)pixels);
    DrawSync(0);
}

/* Title-only scratch, never retained across a language/disc read. Keep the text
 * page intact so the modal sits above both existing menus without extra VRAM. */
static void __attribute__((noinline)) modShowLanguageError(void)
{
    RECT rect;
    char *lines[4] = { MOD_OPT_STR_CHINESE_RAM_0, MOD_OPT_STR_CHINESE_RAM_1,
                       MOD_OPT_STR_CHINESE_RAM_2, MOD_OPT_STR_CHINESE_RAM_3 };
    int32_t row;
    setRECT(&rect, TEXT_VRAM_X, TEXT_VRAM_Y, 64, 48);
    DrawSync(0);
    StoreImage(&rect, (u_long *)GENERAL_BUFFER);
    DrawSync(0);
    setRECT(&rect, 0, 0, 256, 48);
    clearTextSubArea(&rect);
    setTextColor(1);
    for (row = 0; row < 4; row++) {
        drawString(lines[row], 0, row * 12);
        DrawSync(0);
    }
    setRECT(&rect, TEXT_VRAM_X, TEXT_VRAM_Y, 64, 48);
    StoreImage(&rect, (u_long *)(GENERAL_BUFFER + 6144));
    DrawSync(0);
    LoadImage(&rect, (u_long *)GENERAL_BUFFER);
    DrawSync(0);
    MOD_LANGUAGE_ERROR = 1;
    playSound(0, 1);
}

static void __attribute__((noinline)) modRenderLanguageError(void)
{
    uint8_t *pixels = GENERAL_BUFFER + 6144;
    int32_t x, y, end;
    for (y = 0; y < 48; y++) {
        for (x = 0; x < 256; x = end) {
            end = x + 1;
            if (((pixels[y * 128 + x / 2] >> ((x & 1) * 4)) & 15) != 1) continue;
            while (end < 256 && ((pixels[y * 128 + end / 2] >> ((end & 1) * 4)) & 15) == 1) end++;
            modFlagRect(32 + x, 80 + y + (y / 12) * 2, end - x, 1, 0xE0E0E0);
        }
    }
    renderMenuBox(20, 68, 280, 80);
}

static __attribute__((noinline)) void modRenderLanguagePicker(void)
{
    POLY_FT4 *text;
    MenuHighlight selected = {0};
    int32_t i, x, y, end, lang, sx, sy;
    const uint8_t *run;
    uint32_t colour;
    text = (POLY_FT4 *)GsGetWorkBase();
    lang = modLanguageAt(MOD_BOOT_LANGUAGE_CURSOR);
    renderText(text++, (320 - (lang == MOD_LANG_JA || lang == MOD_LANG_ZH ? 24 :
               measureStringMode(MOD_LANGUAGE_TITLES[lang], FIT_VANILLA))) / 2,
               30, (lang & 1) * 128, 120 + (lang / 2) * 12, 112, 12, 0);
    for (i = 0; i < modLanguageCount(); i++) {
        renderText(text++, 48 + (i & 1) * 152, 68 + (i / 2) * 26,
                   (i & 1) * 128, 16 + (i / 2) * 20, 112, 12, i == MOD_BOOT_LANGUAGE_CURSOR ? 2 : 0);
    }
    renderText(text++, (320 - measureStringMode("START", FIT_VANILLA)) / 2, 204, 0, 188, 80, 12, 0);
    GsSetWorkBase((PACKET *)text);
    for (i = 0; i < modLanguageCount(); i++) {
        lang = modLanguageAt(i);
        sx = 16 + (i & 1) * 152;
        sy = 66 + (i / 2) * 26;
        run = MOD_FLAG_RUNS + MOD_FLAG_OFFSET[lang];
        for (y = 0; y < 16; y++) {
            for (x = 0; x < 24; x += end) {
                end = *run++;
                colour = MOD_FLAG_PALETTE[lang][end >> 5];
                end &= 31;
                modFlagRect(sx+x, sy+y, end, 1, colour);
            }
        }
        modFlagRect(sx-1, sy-1, 26, 18, 0x080808);
    }
    selected.x = 10 + (MOD_BOOT_LANGUAGE_CURSOR & 1) * 152;
    selected.y = 64 + (MOD_BOOT_LANGUAGE_CURSOR / 2) * 26;
    selected.w = 148; selected.h = 20;
    renderMenuSelector(&selected);
    renderMenuBox(8, 18, 304, 210);
}
