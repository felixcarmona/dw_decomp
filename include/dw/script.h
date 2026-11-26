#ifndef DW_SCRIPT_H
#define DW_SCRIPT_H

#include <setjmp.h>

#include <libgpu.h>

#include <dw/entity.h>
#include <dw/font.h>
#include <dw/types.h>
#include <dw/version.h>

/*
 * Script opcodes, named as in the DW1 script instruction sheet. Arguments
 * follow the opcode byte; "_" is a padding byte, u8/u16/s16/s32 are
 * little-endian values, "id" is a script id (an NPC, or a SPEAKER_* value) and
 * "pstat" the index of a pstat. Offsets are from the start of the script.
 * ACTIVE_INSTRUCTION holds the opcode the script waits on, such as
 * SCRIPT_OP_SHOW_TEXTBOX while a textbox is shown.
 */
#define SCRIPT_OP_SET_SELECTION			0x10	/* u8 count, u16 targets[count], text */
#define SCRIPT_OP_UNUSED_11			0x11
#define SCRIPT_OP_UNUSED_12			0x12
#define SCRIPT_OP_JUMP_AND_LINK			0x13	/* _, u16 offset */
#define SCRIPT_OP_JUMP_TO_FILE_AND_LINK		0x14	/* _, u16 script, u16 section */
#define SCRIPT_OP_JUMP_RETURN			0x15	/* _ */
#define SCRIPT_OP_JUMP_TO			0x16	/* _, u16 offset */
#define SCRIPT_OP_JUMP_TO_FILE			0x17	/* _, u16 script, u16 section */
#define SCRIPT_OP_SWITCH			0x18	/* pstat, u16 count, u16 offsets[count] */
#define SCRIPT_OP_IF				0x19	/* _, entries, 0x19, _ */
#define SCRIPT_OP_SHOW_TEXTBOX			0x1A	/* _, text */
#define SCRIPT_OP_SET_DIALOG_OWNER		0x1B	/* id */
#define SCRIPT_OP_SET_TRIGGER			0x1C	/* _, u16 trigger */
#define SCRIPT_OP_UNSET_TRIGGER			0x1D	/* _, u16 trigger */
#define SCRIPT_OP_SET_PSTAT			0x1E	/* _, pstat, u8 value */
#define SCRIPT_OP_ADD_TO_PSTAT			0x1F	/* _, pstat, u8 value */
#define SCRIPT_OP_REDUCE_PSTAT			0x20	/* _, pstat, u8 value */
#define SCRIPT_OP_STORE_MAP_ID			0x21	/* pstat */
#define SCRIPT_OP_STORE_DIGIMON_TYPE		0x22	/* pstat */
#define SCRIPT_OP_SET_INVENTORY_SIZE		0x23	/* u8 size */
#define SCRIPT_OP_STORE_RANDOM			0x24	/* _, pstat, u8 max */
#define SCRIPT_OP_STORE_DATE			0x25	/* pstat: year, day, hour, minute */
#define SCRIPT_OP_SET_TEXTBOX_SIZE		0x26	/* id origin, u8 cols, u8 rows */
#define SCRIPT_OP_FADEOUT_HUD			0x27	/* u8 box */
#define SCRIPT_OP_GIVE_ITEM			0x28	/* _, u8 item, u8 amount */
#define SCRIPT_OP_REMOVE_ITEM			0x29	/* _, u8 item, u8 amount */
#define SCRIPT_OP_ADD_MONEY			0x2A	/* _, s32 amount */
#define SCRIPT_OP_REDUCE_MONEY			0x2B	/* _, s32 amount */
#define SCRIPT_OP_COMPARE_DATE			0x2C	/* pstat date, u16 trigger, u8 compare, u8 year, u8 day, u8 hour, u8 minute, _ */
#define SCRIPT_OP_LEARN_MOVE			0x2D	/* u8 move */
#define SCRIPT_OP_UNUSED_2E			0x2E	/* u8 */
#define SCRIPT_OP_GIVE_CARD			0x2F	/* u8 card */
#define SCRIPT_OP_TAKE_CARD			0x30	/* u8 card */
#define SCRIPT_OP_SET_MERIT			0x31	/* _, u16 value */
#define SCRIPT_OP_ADD_MERIT			0x32	/* _, u16 value */
#define SCRIPT_OP_REDUCE_MERIT			0x33	/* _, u16 value; sets MERIT to -value */
#define SCRIPT_OP_SET_STAT			0x34	/* u8 stat, u16 value; a SCRIPT_STAT_* */
#define SCRIPT_OP_ADD_STAT			0x35	/* u8 stat, u16 value */
#define SCRIPT_OP_REDUCE_STAT			0x36	/* u8 stat, u16 value */
#define SCRIPT_OP_ADVANCE_TO_DATE_AT		0x37	/* pstat: year, day, hour, minute */
#define SCRIPT_OP_ADD_MINUTES_TO_DATE_AT	0x38	/* pstat, s32 minutes */
#define SCRIPT_OP_ADD_MINUTES_TO_DATE_AT_2	0x39	/* pstat, s32 minutes */
#define SCRIPT_OP_UNUSED_3A			0x3A	/* _, u8, u8 */
#define SCRIPT_OP_UNUSED_3B			0x3B	/* _, u8, u8 */
#define SCRIPT_OP_UNUSED_3C			0x3C	/* _, u8, u8 */
#define SCRIPT_OP_UNUSED_3D			0x3D	/* u8 */
#define SCRIPT_OP_UNUSED_3E			0x3E	/* u8 */
#define SCRIPT_OP_STORE_DIGIMON_VALUE		0x3F	/* _, pstat digimon, pstat out */
#define SCRIPT_OP_LOAD_DIGIMON			0x46	/* u8 digimon */
#define SCRIPT_OP_SET_DIGIMON			0x47	/* u8 digimon, u8 slot, u8 autotalk */
#define SCRIPT_OP_UNLOAD_ENTITY			0x48	/* id */
#define SCRIPT_OP_CALL_DIGIMON_ROUTINE		0x49	/* u8 routine, see callDigimonRoutine() */
#define SCRIPT_OP_WAIT_FOR_ENTITY		0x4A	/* id, or 0xFF for all */
#define SCRIPT_OP_WARP_TO			0x4B	/* u8 screen, u8 exit, u8 section */
#define SCRIPT_OP_ENTITY_LOOK_AT_ENTITY		0x4C	/* _, id, id target */
#define SCRIPT_OP_ENTITY_SET_ROTATION		0x4D	/* id, s16 rotation */
#define SCRIPT_OP_ENTITY_WALK_TO		0x4E	/* id, s16 x, s16 z, u8 anim, u8 */
#define SCRIPT_OP_MOVE_CAMERA_TO		0x4F	/* u8 speed, s16 x, s16 z */
#define SCRIPT_OP_MOVE_CAMERA_TO_ENTITY		0x50	/* _, id, u8 speed */
#define SCRIPT_OP_ENTITY_WALK_TO_ENTITY		0x51	/* id, u8 anim, id target */
#define SCRIPT_OP_ENTITY_WALK_TO_WITH_CAMERA	0x52	/* as ENTITY_WALK_TO, the camera follows */
#define SCRIPT_OP_ENTITY_WALK_TO_ENTITY_WITH_CAMERA	0x53	/* as ENTITY_WALK_TO_ENTITY, the camera follows */
#define SCRIPT_OP_RESET_ENTITY_ORIGIN		0x54	/* id */
#define SCRIPT_OP_SET_TEXTBOX_ORIGIN		0x55	/* _, s16 x, s16 y, s16 z */
#define SCRIPT_OP_PLAY_ANIMATION		0x56	/* _, id, u8 anim */
#define SCRIPT_OP_SET_OBJ_VISIBILITY		0x57	/* _, u8 object, u8 flag */
#define SCRIPT_OP_TELEPORT			0x58	/* pstat: screen, exit */
#define SCRIPT_OP_PLAY_SOUND			0x5A	/* _, u8 bank, u8 sound */
#define SCRIPT_OP_UNUSED_5B			0x5B	/* u8 */
#define SCRIPT_OP_UNUSED_5C			0x5C	/* u8 */
#define SCRIPT_OP_SET_BGM			0x5D	/* u8 bgm */
#define SCRIPT_OP_STOP_BGM			0x5E	/* u8 */
#define SCRIPT_OP_UNUSED_5F			0x5F	/* u8 */
#define SCRIPT_OP_CALL_ROUTINE			0x64	/* u8 builtin */
#define SCRIPT_OP_REMOVE_CONDITION		0x65	/* u8 mask of CONDITION_* */
#define SCRIPT_OP_START_BATTLE			0x66	/* u8 */
#define SCRIPT_OP_DELAY				0x67	/* _, u16 frames */
#define SCRIPT_OP_SET_TEXTBOX_MODE		0x68	/* _, u8 mode, u8 frames */
#define SCRIPT_OP_DEAL_DAMAGE			0x69	/* pstat percent of max HP */
#define SCRIPT_OP_SET_AUTOTALK			0x6A	/* _, id, u8 autotalk */
#define SCRIPT_OP_DATA				0x6B	/* _[, u8 value...], 0xFF */
#define SCRIPT_OP_ENTITY_MOVE_TO		0x6C	/* id, s16 x, s16 z, u8 speed, u8 */
#define SCRIPT_OP_ENTITY_MOVE_TO_ENTITY		0x6D	/* id, id target, u8 speed */
#define SCRIPT_OP_ENTITY_MOVE_TO_WITH_CAMERA	0x6E	/* as ENTITY_MOVE_TO, but uses ENTITY_MOVE_TO_ENTITY_WITH_CAMERA's movement */
#define SCRIPT_OP_ENTITY_MOVE_TO_ENTITY_WITH_CAMERA	0x6F	/* as ENTITY_MOVE_TO_ENTITY, the camera follows */
#define SCRIPT_OP_ROTATE_3D_OBJECT		0x70	/* u8 door, u8, u8 target */
#define SCRIPT_OP_MOVE_OBJECT_TO		0x71	/* u8 slot, id, u8 speed, s8 angle, _, s16 x, s16 z */
#define SCRIPT_OP_ENTITY_MOVE_TO_AXIS		0x72	/* id, s16 target, u8 axis, u8 speed */
#define SCRIPT_OP_ENTITY_MOVE_TO_AXIS_WITH_CAMERA	0x73	/* as ENTITY_MOVE_TO_AXIS, the camera follows */
#define SCRIPT_OP_SPAWN_ITEM			0x74	/* u8 item, s16 x, s16 z */
#define SCRIPT_OP_SPAWN_CHEST			0x75	/* u8 item, s16 x, y, z, w, u16 trigger */
#define SCRIPT_OP_SPAWN_BOULDER			0x76	/* u8 */
#define SCRIPT_OP_MOVE_BOULDER			0x77	/* u8, s16 x, s16 z */
#define SCRIPT_OP_DESPAWN_BOULDER		0x78	/* u8 */
#define SCRIPT_OP_UNLOAD_DIGIMON		0x79	/* u8 digimon */
#define SCRIPT_OP_COPY_PSTAT			0x7A	/* _, pstat from, pstat to */
#define SCRIPT_OP_SECTION_ON_EXIT		0x7B	/* u8 section to run after a screen change */
#define SCRIPT_OP_SET_RECT_IMPASSABLE		0x7C	/* u8, s16 x, s16 z, u8 w, u8 h */
#define SCRIPT_OP_SPAWN_SPRITE_AT_LOCATION	0x7D	/* u8 sprite, s16 x, y, z, w */
#define SCRIPT_OP_SPAWN_SPRITE_AT_ENTITY	0x7E	/* id, u8 node, u8 sprite */
/*
 * MOD: wide opcodes (docs/mods/ids.md § 8.3). 16-bit variants of the
 * opcodes that carry a Digimon, screen, item, technique, card or music id,
 * and the wide registers w0..w63. Every u16 sits at an even offset and
 * every instruction has an even length. 0x94-0x9F are reserved: like
 * every other unknown opcode, they end the script.
 */
#define SCRIPT_OP_WIDE_FIRST			0x80
#define SCRIPT_OP_LOAD_DIGIMON16		0x80	/* _, u16 digimon */
#define SCRIPT_OP_SET_DIGIMON16			0x81	/* _, u16 digimon, u8 slot, u8 autotalk */
#define SCRIPT_OP_UNLOAD_DIGIMON16		0x82	/* _, u16 digimon */
#define SCRIPT_OP_WARP_TO16			0x83	/* _, u16 screen, u8 exit, u8 section */
#define SCRIPT_OP_GIVE_ITEM16			0x84	/* _, u16 item, u8 amount, _ */
#define SCRIPT_OP_REMOVE_ITEM16			0x85	/* _, u16 item, u8 amount, _ */
#define SCRIPT_OP_SPAWN_ITEM16			0x86	/* _, u16 item, s16 x, s16 z */
#define SCRIPT_OP_SPAWN_CHEST16			0x87	/* _, u16 item, s16 x, y, z, w, u16 trigger */
#define SCRIPT_OP_LEARN_MOVE16			0x88	/* _, u16 move */
#define SCRIPT_OP_GIVE_CARD16			0x89	/* _, u16 card */
#define SCRIPT_OP_TAKE_CARD16			0x8A	/* _, u16 card */
#define SCRIPT_OP_SET_BGM16			0x8B	/* _, u16 bgm (0xFFFF stops it, like 0xFF) */
#define SCRIPT_OP_SET_WREG			0x8C	/* wreg, u16 value */
#define SCRIPT_OP_STORE_DIGIMON_TYPE_W		0x8D	/* wreg */
#define SCRIPT_OP_STORE_MAP_ID_W		0x8E	/* wreg */
#define SCRIPT_OP_TELEPORT_W			0x8F	/* wreg: screen in wN, exit in wN+1 */
#define SCRIPT_OP_COPY_W			0x90	/* _, wreg from, wreg to */
#define SCRIPT_OP_ADD_W				0x91	/* wreg, u16 value; saturates at 0xFFFF */
#define SCRIPT_OP_SUB_W				0x92	/* wreg, u16 value; stops at 0 */
#define SCRIPT_OP_SWITCH_W			0x93	/* wreg, u16 count, u16 offsets[count] */
#define SCRIPT_OP_SPAWN_ITEM_DAILY		0x9F	/* _, u16 item, s16 x, s16 z, u16 stable pickup ID, u16 screen */
#define SCRIPT_OP_WIDE_END			0xA0	/* first opcode after the wide block */
#define SCRIPT_OP_SET_SCRIPT			0xFB	/* _, u16 script, u16 map */
#define SCRIPT_OP_UNUSED_FC			0xFC
#define SCRIPT_OP_UNUSED_FD			0xFD
#define SCRIPT_OP_END_SECTION			0xFE
#define SCRIPT_OP_END_SCRIPT			0xFF	/* as END_SECTION; marks the end of a script */

/* The stats that SCRIPT_OP_SET_STAT and the like change, see getStatsPointer(). */
#define SCRIPT_STAT_OFFENSE		0x00
#define SCRIPT_STAT_DEFENSE		0x01
#define SCRIPT_STAT_SPEED		0x02
#define SCRIPT_STAT_BRAINS		0x03
#define SCRIPT_STAT_MAX_HP		0x04
#define SCRIPT_STAT_MAX_MP		0x05
#define SCRIPT_STAT_CURRENT_HP		0x06
#define SCRIPT_STAT_CURRENT_MP		0x07
#define SCRIPT_STAT_TIREDNESS		0x08
#define SCRIPT_STAT_HAPPINESS		0x09
#define SCRIPT_STAT_DISCIPLINE		0x0A
#define SCRIPT_STAT_ENERGY		0x0B
#define SCRIPT_STAT_VIRUS		0x0C
#define SCRIPT_STAT_LIFETIME		0x0D
#define SCRIPT_STAT_MERIT		0x0E
#define SCRIPT_STAT_STARTED_BATTLES	0x0F
#define SCRIPT_STAT_FLED_BATTLES	0x10
#define SCRIPT_STAT_TOURNAMENTS_WON	0x11
#define SCRIPT_STAT_TOURNAMENT_WINS	0x12
#define SCRIPT_STAT_TOURNAMENTS_LOST	0x13
#define SCRIPT_STAT_WEIGHT		0x14
#define SCRIPT_STAT_TAMER_LEVEL		0x15
#define SCRIPT_STAT_LIVES		0x16

typedef struct {
	uint8_t *scriptPtr;
	uint16_t scriptId;
	uint8_t smth[2];
} StackEntry;

/*
 * MOD: wide registers w0..w63 (docs/mods/ids.md § 8.3): u16 script
 * variables for ids that do not fit in a pstat. They are appended to
 * ScriptState, so every original offset stays the same; they are zeroed
 * with the rest at boot and are not saved yet (save v2, ids.md § 8.4).
 * CALL_ROUTINE 0 (choose an item) also leaves the item in
 * SCRIPT_WREG_CHOSEN_ITEM (0xFFFF = none), next to p254.
 */
#define SCRIPT_NUM_WREGS		64
#define SCRIPT_WREG_CHOSEN_ITEM		62

typedef struct {
	uint8_t smth[212];
	uint8_t cards[33];
	uint8_t triggers[100];
	uint8_t pstats[256];
	StackEntry stack[8];
	uint16_t wregs[SCRIPT_NUM_WREGS]; /* MOD */
} ScriptState;

void scriptSetChosenItem(int32_t item); /* MOD: p254 and SCRIPT_WREG_CHOSEN_ITEM */

typedef struct {
	uint8_t mapId;
	uint8_t unk_0x1;
	uint16_t trigger;
	uint32_t cost;
} BattleEntry;

extern BattleEntry BIRDRA_TRANSPORT_TARGETS[];

typedef struct {
	uint8_t *pointer;
	uint8_t *endPointer;
	uint16_t count;
	uint16_t current;
	uint16_t cursorOffsetY;
	uint16_t cursorWidth;
} DialogueSelection;

extern DialogueSelection DIALOGUE_SELECTION;

typedef struct {
	uint8_t digimonId;
	uint8_t spriteId;
	int16_t meritValue;
} CardData;

extern CardData CARD_DATA[];

typedef struct {
	uint8_t *buf;
	int32_t isOpen;
	uint8_t itemCount;
	uint8_t topRow;
	uint8_t cursor;
	uint8_t visibleRows;
	uint8_t boxId;
	uint8_t prevTopRow;
	uint8_t prevCursor;
	RECT rect;
	uint8_t itemRow[8];
} ItemMenuBox;

typedef struct {
	uint8_t type;
	uint8_t entityId;
	uint8_t target;
	uint8_t speed;
	int16_t posX;
	int16_t posY;
	int16_t targetX;
	int16_t targetY;
} ScriptCameraMovement;

extern uint16_t DELAY_FRAMES;
extern uint8_t AUTOCLOSE_FRAMES;
extern ScriptState *SCRIPT_STATE_PTR;
extern uint8_t ACTIVE_BGM_FONT;
extern uint8_t *TEXTBOX_LINES_PTR;
extern uint8_t *SCRIPT_POINTER;
extern uint8_t *MAPHEAD_SCRIPT_PTR;
extern int32_t LOADED_DIGIMON_MODELS[];
extern ItemMenuBox *ITEM_MENU_LEFT;
extern ItemMenuBox *ITEM_MENU_RIGHT;
extern int8_t TRN_LOADING_COMPLETE;
extern uint8_t CURRENT_DIALOGUE_OWNER;
extern uint16_t SCRIPT_STACK_OFFSET;
extern int16_t ITEM_MENU_POS[];
extern uint16_t CURRENT_SCRIPT_ID;
extern uint16_t CURRENT_SCREEN_ID;
extern int32_t MAIN_D_80134FEC;
extern jmp_buf SCRIPT_JMP_BUF;
extern uint8_t TEXTBOX_CLOSE_MODE;
extern uint8_t MAIN_D_80134FE7;
extern int32_t SOME_SCRIPT_SYNC_BIT;
extern int32_t IS_SCRIPT_PAUSED;
extern int16_t MAIN_D_80134F9C;
extern uint16_t ACTIVE_MAP_SCRIPT;
extern int16_t MERIT;
extern int16_t BATTLES_STARTED;
extern int16_t BATTLES_FLED;
extern int16_t TOURNAMENTS_WON;
extern int16_t TOURNAMENT_WINS;
extern int16_t TOURNAMENTS_LOST;
extern int16_t TEXTBOX_ORIGIN_X;
extern int16_t TEXTBOX_ORIGIN_Y;
extern int16_t TEXTBOX_ORIGIN_Z;
extern uint16_t NAMING_SELECTOR;
extern uint8_t SHOP_ITEM_TYPE;
extern int16_t TMP_TAMER_LEVEL;
extern int16_t TMP_LIVES;
extern int32_t SHOP_ACTION_SELECTED;
extern int32_t MAIN_D_80134F74;
extern uint8_t RECYCLABLE_ITEMS[];
extern int32_t SHOP_VARIABLE;
extern uint16_t SCRIPT_STATE_2;
extern uint16_t SCRIPT_NEXT_STATE_2;
extern uint8_t SCRIPT_TEXTBOX_MODE;
extern int32_t SCRIPT_MAP_CHANGE_SHOW_NAME;
extern int32_t SCRIPT_SECTION_IS_EVENT;
extern int32_t MONEY;
extern char NAMING_BUFFER[20];
extern int32_t UPDATE_SHOP_BIT_BOX;
/* MOD (amounts): the amount boxes step by this rule (script_common.c); the card list shows a purchase of packs (script_menu.c) */
int32_t modAmountStep(void);
extern uint8_t MOD_PACK_RESULTS;
extern uint8_t ACTIVE_INSTRUCTION;
extern uint8_t ITEM_MENU_TYPE;
extern uint8_t MAIN_D_80135000;
extern uint8_t WAIT_FOR_ENTITY_ID;
extern uint16_t SCRIPT_PARAM_1;
extern uint8_t NAMING_BOX_FLAG;
extern int8_t DRAW_STRING2_IS_FIXED_WIDTH;
uint8_t scriptIdToEntityId(int32_t scriptId);
uint16_t showTextbox(uint8_t boxId, uint8_t speakerId);
#if !VERSION_IS(US)
int32_t drawString2(uint8_t *str, int16_t x, int16_t y);
#else
int32_t drawString2(uint8_t *str, int16_t x, int16_t y, int32_t flag);
#endif
void closeBox(int32_t boxId);
void scriptPauseGame(int32_t owner);
void updateItemMenuStrings(ItemMenuBox *box, int32_t startRow, int32_t style);
void startAnimationTamer(int32_t animId);
void tickScriptDialogueBox(void);
void renderScriptDialogueBox(void);
void returnFromScriptFile(void);
void getEntityScreenPos(Entity *entity, int32_t flag, DVECTOR *outPos);
void readFileSection(char *filename, void *dest, uint32_t offset,
		     uint32_t size);
void dailyPStatTrigger(void);
void showMapheadSelection(int32_t idx, uint8_t owner, uint16_t x, int32_t *outSel, uint16_t section);
int32_t setCardAmount(int32_t cardId, int32_t value);
int32_t popScriptStack(StackEntry *entry);
void scriptIfInstruction(void);
void scriptCompareDate(void);
void readMapTFS(int32_t mapId);
void handleItemLoss(void);
void setMovementEnabled(int32_t a0, int32_t a1);
void scriptNPCStartAnimation(uint32_t scriptId, int32_t animId);
uint8_t rollCard(void);
void showCardTextbox(void);
void mojyaTradeFillItemList(void);
void renderNameDisplayBox(void);
void inputInit(void);
void setInputRepeatMask(uint32_t mask);
int32_t isPartnerBaby(void);
void openShop(void);
void allocateItemMenuBox(ItemMenuBox **box, int32_t bufSize, uint8_t rows, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void showShopkeeperTextbox(int32_t idx, int32_t owner, int32_t boxId);
void destroyItemMenuBox(ItemMenuBox **box);
void createShopBitsBox(int32_t showBits);
void showShopkeepSelection(int32_t idx, int32_t owner, int32_t boxId,
			   int32_t *outSelection);
void createItemMenu(void);
void openDiscardItem(void);
ItemMenuBox *getItemMenuFromType(void);
void initItemMenuBox(ItemMenuBox *box, int32_t boxId, int32_t startRow);
void playShopSoundOnlyInSavannah(void);
void setDialogueOwner(int32_t owner);
void createMonochromonMoodBubble(void);
void loadShopLibrary(void);
void initializeTextbox(void);
void closeAllTextboxes(void);
void triggerBoxCloseFlag(int32_t boxId);
void registerTextbox(int32_t boxId, int32_t row, int32_t rows,
		     int32_t doubleBuffer, int32_t mode);
int32_t setupBoxOrigin(int32_t ownerId, RECT *origin);
void scriptShowSelection(void);
void scriptSetTextboxSize(void);
int32_t scriptCompareStat(void);
int16_t *getStatsPointer(int32_t stat);
int32_t scriptCompareCard(void);
int32_t scriptCompareMove(void);
int32_t scriptCompareCondition(void);
int32_t scriptCompareItemCount(void);
int32_t scriptCompareMoney(void);
int16_t enforceStatsLimits(uint8_t stat, int16_t value);
void tickScriptedMovements(void);
void scriptInstructionFBtoFF(int32_t op);
void scriptInstruction10to27(int32_t op);
void scriptInstruction28to3F(int32_t op);
void scriptInstruction46to58(int32_t op);
void scriptInstruction5Ato5F(int32_t op);
void scriptInstruction64to7E(int32_t op);
void scriptUpdateEnergyBoundaries(int32_t a0, int32_t a1);
void consumeMapChangeShowName(void);
void scriptSetEntityWalking(uint8_t actorId, int32_t animationId);
void scriptStartAnimation(uint8_t actorId, int32_t animationId);
void handleMusicOverride(uint8_t *outFont, uint8_t *outVariant);
void tickScriptedMovement(int32_t slot);
void callScriptSection(int32_t scriptId, int32_t section, int32_t param);
void callScriptSection(int32_t scriptId, int32_t section, int32_t param);
int32_t tickScript(void);
uint8_t *getScript(uint16_t mapId);
uint8_t readPStat(uint8_t index);
/* MOD (species past 255): a pstat is one byte and a species is not. storePartnerSpecies keeps a partner past 254 as
   PSTAT_WIDE_SPECIES, which no narrow test matches by accident, and pstatSpecies reads the whole species back. The
   Sukamon backup outlives the partner it names, so it also keeps the species in a saved wide register. */
#define PSTAT_WIDE_SPECIES	0xFF
#define WREG_SUKAMON_BACKUP	15
void storePartnerSpecies(uint8_t index);
int32_t pstatSpecies(uint8_t index);
int32_t sukamonBackupSpecies(void);
int32_t isTriggerSet(uint16_t trigger);
void writePStat(uint8_t index, uint8_t value);
void skipOneReadTwoShort(uint16_t *out1, uint16_t *out2);
void pushScriptStack(StackEntry *entry);
void resetBGM(void);
void skipOneReadOneUShort(uint16_t *out);
void pollOneUByteOneUShort(uint8_t *outByte, uint16_t *outShort);
void pollNextScriptUByte(uint8_t *out);
void setTrigger(uint16_t trigger);
void unsetTrigger(uint16_t trigger);
void skipOnePollTwoScriptBytes(uint8_t *out1, uint8_t *out2);
void pollNextScriptUShort(uint16_t *out);
void pollNextTwoScriptBytes(uint8_t *out1, uint8_t *out2);
int32_t scriptCompareValue(uint8_t op, uint32_t lhs, uint32_t rhs);
void skipOneReadInteger(int32_t *out);
void scriptLearnMove(int32_t moveId);
uint8_t getCardAmount(int32_t cardId);
uint32_t dateToSeconds(int32_t years, int32_t days, int32_t hours,
		       int32_t minutes);
void pollNextInt(int32_t *out);
void secondsToDate(uint32_t totalMinutes, uint8_t *outYear,
		   uint8_t *outDay, uint8_t *outHour,
		   uint8_t *outMinute);
void scriptLoadModel(int32_t modelId);
void pollNextScriptShort(int16_t *out);
void pollNextTwoScriptShorts(int16_t *out1, int16_t *out2);
void playBGM(int32_t bgmId);
void scriptUnloadModel(int16_t modelId);
void getTriggerOffset(int32_t trigger, uint8_t **outPtr, uint8_t *outMask);
int32_t _hasMove(int32_t moveId);
int32_t fillRecycleShopItemList(void);
int32_t hasAnyCardToBuy(void);
void createCardMenu(void);
int32_t shopFillSellCardList(void);
int32_t shopFillMeritItemList(void);
void itemKeeperFillItemList(void);
void createItemKeepWindow(void);
void jukeboxFillTitleList(void);
void createJukeboxMenu(void);
void birdraFillTargetList(void);
void createBirdraTransportMenu(void);
void createMojyaTradeMenu(void);
void setMojyaItemTradedTrigger(void);
void setupNewGameDialogueBox(void);
void showNewGameDialogue(int32_t textId, uint16_t nextState);
void showNewGameSelection(int32_t textId, uint16_t nextState);
void setupNameSelectorBox(void);
void setupNameDisplayBox(void);
void openRecycleShop(void);
void rollCardPack(void);
void openBuyCardMenu(void);
void openSellCardMenu(void);
void openMeritShop(void);
void tickItemKeeper(void);
void openJukebox(void);
void tickBirdraTransport(void);
void openMojyamonShop(void);
void initializeNamingBuffer(uint8_t flags);
int32_t newGameStateMachine(void);
int16_t *getStatsPointer(int32_t stat);
uint8_t *getScript(uint16_t mapId);
#if !VERSION_IS(US)
void tickTextboxHandling(void);
#else
void tickTextboxHandling(int32_t flag);
#endif
extern void lostAllLives(void);

#endif
