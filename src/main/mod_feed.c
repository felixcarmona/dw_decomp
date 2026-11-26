/*
 * MOD (care menu): X in front of the partner opens a list of what answers its needs.
 *
 * On the map, with the tamer facing the partner (within 45 degrees) and close enough (its item take distance plus
 * FEED_SLACK, as checkEatDistance measures), X opens a box laid out as the inventory's (two columns of icon, name
 * and amount, the description box under it) with what the partner's condition asks for, every need at once:
 * the triangle menu's commands a need asks for first, Scold when a scolding is fair (ITEM_SCOLD_FLAG 1: it refused
 * an item out of indiscipline, it pooped on the ground) and Sleep when it is sleepy, then in inventory order the
 * items that answer a need
 * it has: food when hungry (ITEM_FUNCTIONS[type] == handleFood), the food that lowers tiredness when tired, the
 * Port. potty when it needs to go, what cures an injury or a sickness when it has one (handleStatusItems with a
 * cure in its effect row: Bandage, Medicine). X gives the chosen item as Items > Use does (startFeedingItem: the
 * partner comes, takes it or refuses it, one unit is used), a command does what the triangle menu's does (Scold:
 * discipline up and the next item taken; Sleep), triangle closes the box. When the partner refuses the item this
 * menu gave and a scolding is fair, the menu opens again on its own with Scold under the cursor. With nothing to
 * offer X does nothing here. While the box is open the clock stops, as with the triangle menu.
 * Hooked from tamerTickWalkingState() (tamer.c), after checkMapInteraction(): talking to an NPC, a chest or a
 * press tile take the X first.
 */
#include <libgs.h>

#include <dw/clock.h>
#include <dw/entity.h>
#include <dw/fade.h>
#include <dw/font.h>
#include <dw/input.h>
#include <dw/item.h>
#include <dw/level.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/script.h>
#include <dw/types.h>
#include <dw/ui.h>
#include <dw/version.h>
#include <dw/world_object.h>

#define FEED_OBJECT_ID	0x1f9b	/* world object id: above the species ids (world_object.h) */
#define FEED_LIST_BOX	0	/* box 0: the game reads it as "a menu is open" (the triangle menu, the inventory) */
#define FEED_DESC_BOX	1
#define FEED_ROWS_SHOWN	5	/* rows of two items, as the 10-slot inventory */
#define FEED_SLACK	12	/* added to the item take distance (x 20 units in checkEatDistance) */
#define FEED_FACING	0x200	/* 45 degrees either side */
#define PARTNER_NORMAL	1	/* PARTNER_STATE */
#define PARTNER_SLEEP	3	/* PARTNER_STATE: partnerTickSleep, as the triangle menu's Sleep */
#define PORT_POTTY	0x20
#define PARTNER_SCOLD	4	/* PARTNER_STATE: partnerTickPraiseScold, as the triangle menu's Scold */
#define PARTNER_FEEDING	5	/* PARTNER_STATE: partnerTickFeedItem */
#define FEED_SCOLD	0xFE	/* feedSlots entries from here up: a command, not an inventory slot */
#define FEED_SLEEP	0xFF
#define IS_COMMAND(slot)	((slot) >= FEED_SCOLD)
#define SCOLD_PENDING	1	/* ITEM_SCOLD_FLAG: a scolding now is a fair one */

enum { FEED_OPENING, FEED_OPEN, FEED_CLOSING, FEED_GIVING, FEED_WATCHING };

typedef struct {
	char *name;
	char *desc;
	uint8_t texX; /* the triangle menu's icon (MENU_OPTIONS): u, v - 0xC0 */
	uint8_t texY;
	int8_t partnerState;
} FeedCommand;

extern int32_t POLLED_INPUT;
extern int32_t POLLED_INPUT_PREVIOUS;
extern uint32_t CHANGED_INPUT;
extern int32_t IS_IN_MENU;
extern int8_t TAMER_STATE;
extern int8_t PARTNER_STATE;
extern int8_t ITEM_SCOLD_FLAG;
extern uint8_t INVENTORY_POINTER;
extern TamerEntity TAMER_ENTITY;
extern char *ITEM_DESCS[];

int32_t _atan(int32_t dy, int32_t dx);
int32_t checkEatDistance(int32_t distance);
void tamerSetState(int32_t state);
void startAnimation(Entity *entity, int32_t animId);
void setPartnerIdling(void);
void setCameraFollowPlayer(void);
void unsetCameraFollowPlayer(void);
void getEntityScreenPos(Entity *e, int32_t mode, DVECTOR *out);
void startFeedingItem(ItemId type);
void playSound(int32_t vabId, uint32_t note);
void drawString(char *str, int32_t x, int32_t y);
void renderString(int32_t color, int32_t x, int32_t y, int32_t w, int32_t h, int32_t u, int32_t v, int32_t layer,
                  int32_t shadow);
void renderSelectionCursor(int32_t x, int32_t y, int16_t w, int16_t h, int32_t layer);
void renderItemSprite(ItemId type, int16_t x, int16_t y, int32_t layer);
void renderItemAmount(int32_t color, int32_t n, int32_t x, int32_t y, int32_t value, int32_t layer);
void clearTextSubArea(RECT *rect);
void renderRectPolyFT4(int16_t posX, int16_t posY, uint8_t width, uint8_t height, uint8_t texX, uint8_t texY,
                       int16_t texturePage, int16_t clut, int32_t zIndex, int8_t flag);
u_short GetClut(int32_t x, int32_t y);

static char FEED_SCOLD_NAME[] = "Scold";
static char FEED_SCOLD_DESC[] = "It misbehaved: teach it.";
static char FEED_SLEEP_NAME[] = "Sleep";
static char FEED_SLEEP_DESC[] = "Put it to bed until morning.";

/* by slot - FEED_SCOLD */
static FeedCommand FEED_COMMANDS[2] = {
	{ FEED_SCOLD_NAME, FEED_SCOLD_DESC, 0x00, 0x28, PARTNER_SCOLD }, /* MENU_OPTIONS[5] */
	{ FEED_SLEEP_NAME, FEED_SLEEP_DESC, 0x28, 0x28, PARTNER_SLEEP }, /* MENU_OPTIONS[6] */
};

static uint8_t feedSlots[32]; /* the commands, then the inventory slots shown, in inventory order */
static int32_t feedCount;
static int32_t feedCursor;
static int32_t feedRowOffset;
static int32_t feedState;

/* the item answers one of the needs in condition */
static int32_t answersNeed(ItemId type, uint32_t condition)
{
	const int16_t *fx;

	if (type == ITEM_NONE) {
		return 0;
	}
	fx = itemFx(type);
	if (ITEM_FUNCTIONS[type] == (ItemFunction)handleFood) {
		return (condition & CONDITION_HUNGRY) || ((condition & CONDITION_TIRED) && MOD_FX(fx, MOD_FX_TIREDNESS) > 0);
	}
	if (ITEM_FUNCTIONS[type] == (ItemFunction)handleStatusItems) {
		return ((condition & CONDITION_INJURED) && MOD_FX(fx, MOD_FX_CURE_INJURY) > 0) ||
		       ((condition & CONDITION_SICK) && MOD_FX(fx, MOD_FX_CURE_SICK) > 0);
	}
	return type == PORT_POTTY && (condition & CONDITION_POOPY);
}

static int32_t feedCollect(void)
{
	int32_t i;

	feedCount = 0;
	if (ITEM_SCOLD_FLAG == SCOLD_PENDING) {
		feedSlots[feedCount++] = FEED_SCOLD;
	}
	if (PARTNER_PARA.condition & CONDITION_SLEEPY) {
		feedSlots[feedCount++] = FEED_SLEEP;
	}
	for (i = 0; i < INVENTORY.size; i++) {
		if (INVENTORY.amounts.array[i] != 0 && answersNeed(INVENTORY.types.array[i], PARTNER_PARA.condition)) {
			feedSlots[feedCount++] = i;
		}
	}
	return feedCount;
}

static char *feedName(int32_t i)
{
	return IS_COMMAND(feedSlots[i]) ? FEED_COMMANDS[feedSlots[i] - FEED_SCOLD].name
	                                : ITEM_NAME(INVENTORY.types.array[feedSlots[i]]);
}

/* the partner is within FEED_FACING of where the tamer looks, and close enough to take an item */
static int32_t feedPartnerAhead(void)
{
	VECTOR *t = &TAMER_ENTITY.entity.posData->location;
	VECTOR *p = &PARTNER_ENTITY.digimonEntity.entity.posData->location;
	int32_t diff;
	int32_t reach;

	reach = getItemTakeDistance(PARTNER_ENTITY.digimonEntity.entity.type) + FEED_SLACK;
	if (reach > 127) {
		reach = 127;
	}
	if (checkEatDistance(reach) == 0) {
		return 0;
	}
	diff = (_atan(p->vz - t->vz, p->vx - t->vx) - TAMER_ENTITY.entity.posData->rotation.vy) & 0xFFF;
	return diff <= FEED_FACING || diff >= 0x1000 - FEED_FACING;
}

static int32_t feedRows(void)
{
	return (feedCount + 1) / 2;
}

/* the names in the text area at the inventory's places by list index (column * 0x60, row * 0xC), the description at
 * 0xB4 (updateItemDescription's row) */
static void feedDrawNames(void)
{
	int32_t i;
	char *name;

	clearTextArea();
	for (i = 0; i < feedCount; i++) {
		name = feedName(i);
		drawStringMode(name, (i & 1) * 0x60, (i / 2) * 0xC, fitStringMode(name, ITEM_FIELD_W));
	}
	DrawSync(0);
}

static void feedDrawDescription(void)
{
	RECT area;

	setRECT(&area, 0, 0xB4, 0xFC, 0xC);
	clearTextSubArea(&area);
	drawString(IS_COMMAND(feedSlots[feedCursor]) ? FEED_COMMANDS[feedSlots[feedCursor] - FEED_SCOLD].desc
	                                             : ITEM_DESCS[INVENTORY.types.array[feedSlots[feedCursor]]],
	           0, 0xB4);
	DrawSync(0);
}

static void feedRenderList(int32_t boxId)
{
	UIBoxData *box = &UI_BOX_DATA[boxId];
	int32_t i;
	int32_t row;
	int32_t x;
	int32_t y;
	ItemId type;
	FeedCommand *command;

	for (i = feedRowOffset * 2; i < feedCount && i < (feedRowOffset + FEED_ROWS_SHOWN) * 2; i++) {
		row = i / 2 - feedRowOffset;
		x = box->finalPos.x + ((i & 1) ? 0x9A : 0xA);
		y = box->finalPos.y + 8 + row * 0x12;
		if (IS_COMMAND(feedSlots[i])) {
			/* the triangle menu's icon, 20 x 20 centred on the 16 x 16 item icon */
			command = &FEED_COMMANDS[feedSlots[i] - FEED_SCOLD];
			renderRectPolyFT4(x - 2, y - 2, 0x14, 0x14, command->texX, command->texY + 0xC0, 0x1E,
			                  GetClut(0x100, 0xD + 0x1F0), 6 - boxId, 0);
			renderString(8, x + 0x13, y + 2, 0x60, 0xC, (i & 1) * 0x60, (i / 2) * 0xC, 6 - boxId, 1);
			continue;
		}
		type = INVENTORY.types.array[feedSlots[i]];
		renderItemSprite(type, x, y, 6 - boxId);
		renderString(ITEM_PARA[type].itemColor == 0xFF ? 8 : ITEM_PARA[type].itemColor + 5, x + 0x13, y + 2, 0x60,
		             0xC, (i & 1) * 0x60, (i / 2) * 0xC, 6 - boxId, 1);
		renderItemAmount(0, 2, x + 0x79, y + 4, INVENTORY.amounts.array[feedSlots[i]], 6 - boxId);
	}
	row = feedCursor / 2 - feedRowOffset;
	renderSelectionCursor(box->finalPos.x + ((feedCursor & 1) ? 0x97 : 7), box->finalPos.y + 7 + row * 0x12, 0x8A,
	                      0x12, 6 - boxId);
	box->rowOffset = feedRowOffset;
}

static void feedRenderDescription(int32_t boxId)
{
	renderString(9, UI_BOX_DATA[boxId].finalPos.x + 0x1A, UI_BOX_DATA[boxId].finalPos.y + 8, 0xFC, 0xC, 0, 0xB4,
	             6 - boxId, 1);
}

static void feedOpenBoxes(void)
{
	RECT finalPos;
	RECT startPos;
	DVECTOR xy;
	UIBoxData *box = &UI_BOX_DATA[FEED_LIST_BOX];
	int32_t shown;
	uint8_t features;

	shown = feedRows() < FEED_ROWS_SHOWN ? feedRows() : FEED_ROWS_SHOWN;
	features = 2;
	box->rowOffset = 0;
	box->visibleRows = FEED_ROWS_SHOWN;
	box->totalRows = feedRows();
	if (feedRows() > FEED_ROWS_SHOWN) {
		features |= 4; /* the scroll bar */
	}
	setRECT(&finalPos, -0x98, -0x68, 0x130, 0x14 + shown * 0x12);
	getEntityScreenPos(&PARTNER_ENTITY.digimonEntity.entity, 1, &xy);
	setRECT(&startPos, xy.vx - 5, xy.vy - 5, 10, 10);
	createAnimatedUIBox(FEED_LIST_BOX, 0, features, &finalPos, &startPos, NULL, (RenderFunction)feedRenderList);
	setRECT(&startPos, finalPos.x + 8, finalPos.y + 0xE, 0x10, 0x10);
	setRECT(&finalPos, -0x98, finalPos.y + finalPos.h + 2, 0x130, 0x1C);
	createAnimatedUIBox(FEED_DESC_BOX, 0, 2, &finalPos, &startPos, NULL, (RenderFunction)feedRenderDescription);
}

static void feedCloseBoxes(void)
{
	RECT rect;
	DVECTOR xy;

	getEntityScreenPos(&PARTNER_ENTITY.digimonEntity.entity, 1, &xy);
	setRECT(&rect, xy.vx - 5, xy.vy - 5, 10, 10);
	removeAnimatedUIBox(FEED_DESC_BOX, &rect);
	removeAnimatedUIBox(FEED_LIST_BOX, &rect);
}

/* the tamer stops, the clock stops, the partner waits: as the triangle menu opens */
static void feedEnterMenu(void)
{
	tamerSetState(1);
	startAnimation(&TAMER_ENTITY.entity, 0);
	unsetCameraFollowPlayer();
	IS_IN_MENU = 1;
	stopGameTime();
	setPartnerIdling();
	playSound(0, 3);
}

static void feedMoveCursor(int32_t cursor)
{
	if (cursor < 0 || cursor >= feedCount || cursor == feedCursor) {
		return;
	}
	playSound(0, 2);
	feedCursor = cursor;
	if (feedCursor / 2 < feedRowOffset) {
		feedRowOffset = feedCursor / 2;
	}
	if (feedCursor / 2 >= feedRowOffset + FEED_ROWS_SHOWN) {
		feedRowOffset = feedCursor / 2 - FEED_ROWS_SHOWN + 1;
	}
	if (!IS_COMMAND(feedSlots[feedCursor])) {
		INVENTORY_POINTER = feedSlots[feedCursor]; /* removeOneSelectedItem takes the used unit from this slot */
	}
	feedDrawDescription();
}

static int32_t feedOpen(void);

static void feedTick(int32_t instanceId)
{
	int32_t pressed = POLLED_INPUT & ~POLLED_INPUT_PREVIOUS;

	(void)instanceId;
	switch (feedState) {
	case FEED_OPENING:
		if (UI_BOX_DATA[FEED_LIST_BOX].state == 1 && UI_BOX_DATA[FEED_DESC_BOX].state == 1) {
			feedState = FEED_OPEN;
		}
		break;
	case FEED_OPEN:
		if (pressed & 0x1000) {
			feedMoveCursor(feedCursor - 2);
		} else if (pressed & 0x4000) {
			/* down from the right column onto a last row with one item: to that item */
			feedMoveCursor(feedCursor + 2 < feedCount ? feedCursor + 2 : (feedCursor & 1) ? feedCursor + 1 : -1);
		} else if ((pressed & 0x8000) && (feedCursor & 1)) {
			feedMoveCursor(feedCursor - 1);
		} else if ((pressed & 0x2000) && !(feedCursor & 1)) {
			feedMoveCursor(feedCursor + 1);
		} else if (pressed & CONFIRM_BUTTON) {
			playSound(0, 3);
			feedCloseBoxes();
			feedState = FEED_GIVING;
		} else if (pressed & CANCEL_BUTTON) {
			playSound(0, 4);
			feedCloseBoxes();
			feedState = FEED_CLOSING;
		}
		break;
	case FEED_WATCHING:
		/* the item handed over: once the partner took it or refused it, a refusal a scolding answers opens the menu
		 * again, Scold first */
		if (PARTNER_STATE == PARTNER_FEEDING) {
			break;
		}
		if (ITEM_SCOLD_FLAG != SCOLD_PENDING || PARTNER_STATE != PARTNER_NORMAL || TAMER_STATE != 0 || !feedOpen()) {
			removeObject(FEED_OBJECT_ID, 0);
		}
		break;
	case FEED_CLOSING:
		if (UI_BOX_DATA[FEED_LIST_BOX].state == 0 && UI_BOX_DATA[FEED_DESC_BOX].state == 0) {
			removeObject(FEED_OBJECT_ID, 0);
			tamerSetState(0);
			setCameraFollowPlayer();
			IS_IN_MENU = 0;
			startGameTime();
		}
		break;
	case FEED_GIVING:
		if (UI_BOX_DATA[FEED_LIST_BOX].state == 0 && UI_BOX_DATA[FEED_DESC_BOX].state == 0) {
			if (IS_COMMAND(feedSlots[feedCursor])) {
				removeObject(FEED_OBJECT_ID, 0);
				/* the tamer, the camera and the clock as the triangle menu's command has them */
				partnerSetState(FEED_COMMANDS[feedSlots[feedCursor] - FEED_SCOLD].partnerState);
				IS_IN_MENU = 0;
				startGameTime();
				break;
			}
			INVENTORY_POINTER = feedSlots[feedCursor];
			startFeedingItem(INVENTORY.types.array[feedSlots[feedCursor]]); /* clock back on, tamer and partner */
			feedState = FEED_WATCHING;
		}
		break;
	}
}

/* the box opens with what the partner's condition asks for: 0 when there is nothing to offer */
static int32_t feedOpen(void)
{
	if (feedCollect() == 0) {
		return 0;
	}
	feedCursor = 0;
	feedRowOffset = 0;
	feedState = FEED_OPENING;
	if (!IS_COMMAND(feedSlots[0])) {
		INVENTORY_POINTER = feedSlots[0];
	}
	feedDrawNames();
	feedDrawDescription();
	feedEnterMenu();
	feedOpenBoxes();
	return 1;
}

/* from tamerTickWalkingState: 1 when X opened the menu */
int32_t modFeedTick(void)
{
	if ((CHANGED_INPUT & CONFIRM_BUTTON) == 0 || TAMER_STATE != 0 || IS_SCRIPT_PAUSED != 1 || FADE_PROTECTION != 0 ||
	    UI_BOX_DATA[FEED_LIST_BOX].state != 0 || UI_BOX_DATA[FEED_LIST_BOX].frame != 0 ||
	    UI_BOX_DATA[FEED_DESC_BOX].state != 0 || UI_BOX_DATA[FEED_DESC_BOX].frame != 0 ||
	    PARTNER_STATE != PARTNER_NORMAL || TAMER_ITEM.worldItem.type != ITEM_NONE || !feedPartnerAhead() ||
	    !feedOpen()) {
		return 0;
	}
	addObject(FEED_OBJECT_ID, 0, feedTick, NULL);
	return 1;
}
