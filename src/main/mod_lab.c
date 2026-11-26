/*
 * MOD: lab menu.
 *
 * With the game paused (START) and no script running, SELECT opens a box with
 * the partner's state and four options (up/down to choose, X to accept,
 * triangle or SELECT to close):
 *   - "+1000 bits"     adds 1000 to the money
 *   - "Heal partner"   current HP and MP to the maximum
 *   - "Gift, once"     mini quest: gives 1 Meat and sets MOD_LAB_TRIGGER;
 *                      afterwards it says the gift was already taken
 *   - "Close"
 *
 * Hooked from handlePause() (utils2.c). Only uses game functions:
 * drawString/renderString (text in the VRAM text area), UI boxes
 * (createStaticUIBox), giveItem, setTrigger/isTriggerSet, playSound.
 * The game font has no '>', '/' or parentheses: only A-Z a-z 0-9 space
 * . ? ! " ' , \ : ; + - = are safe.
 */
#include <libetc.h>

#include <dw/btl.h>
#include <dw/entity.h>
#include <dw/font.h>
#include <dw/script.h>
#include <dw/sound.h>
#include <dw/types.h>
#include <dw/ui.h>

#define MOD_LAB_TRIGGER	799	/* free and saved trigger (docs/mods/scripts.md, 717-799) */
#define MOD_LAB_BOX	5	/* same box the pause uses */
#define MOD_LAB_TEXT_Y	0xA0	/* text area rows we use: 0xA0..0xEF */
#define MOD_LAB_LINES	6
#define MOD_LAB_OPTIONS	4
#define ITEM_MEAT	38

#define PAD_SELECT	0x100
#define PAD_UP		0x1000
#define PAD_DOWN	0x4000
#define PAD_TRIANGLE	0x10
#define PAD_CROSS	0x40

extern UIBoxData UI_BOX_DATA[];
extern int32_t IS_SCRIPT_PAUSED;

void drawString(char *str, int32_t x, int32_t y);
void renderString(int32_t color, int32_t x, int32_t y, int32_t w, int32_t h,
		  int32_t u, int32_t v, int32_t layer, int32_t shadow);
void clearTextSubArea(RECT *rect);
char *formatInteger(int32_t value, char *buf, int32_t digits);
int32_t giveItem(uint8_t item, uint8_t amount);
void pauseFrame(void);

static int32_t modLabCursor;
static char *modLabMessage;

static char *MOD_LAB_OPTION_TEXT[MOD_LAB_OPTIONS] = {
	"+1000 bits",
	"Heal partner",
	"Gift, once",
	"Close",
};

static char *appendStr(char *dst, char *src)
{
	while (*src != '\0') {
		*dst++ = *src++;
	}
	*dst = '\0';
	return dst;
}

static void modLabDrawText(void)
{
	RECT area;
	char line[40];
	char num[12];
	char *p;
	int32_t i;
	Stats *stats = &PARTNER_ENTITY.digimonEntity.stats;

	setRECT(&area, 0, MOD_LAB_TEXT_Y, 0x100, MOD_LAB_LINES * 0xC);
	clearTextSubArea(&area);

	drawString("LAB MENU", 0, MOD_LAB_TEXT_Y);

	p = appendStr(line, "Bits ");
	p = appendStr(p, formatInteger(MONEY, num, 6));
	p = appendStr(p, "  HP ");
	p = appendStr(p, formatInteger(stats->current.currentHP, num, 4));
	p = appendStr(p, " of ");
	appendStr(p, formatInteger(stats->base.hp, num, 4));
	drawString(line, 0, MOD_LAB_TEXT_Y + 0xC);

	for (i = 0; i < MOD_LAB_OPTIONS; i++) {
		p = appendStr(line, i == modLabCursor ? "= " : "  ");
		appendStr(p, MOD_LAB_OPTION_TEXT[i]);
		drawString(line, 0, MOD_LAB_TEXT_Y + 0x18 + i * 0xC);
	}
	DrawSync(0);
}

static void modLabRender(int32_t instanceId)
{
	GsBOXF box;
	RECT *pos;

	box.attribute = 0x40000000;
	box.r = box.g = box.b = 0;
	setRECT(&box, -0xA0, -0x78, 0x140, 0xF0);
	GsSortBoxFill(&box, ACTIVE_ORDERING_TABLE, 7 - instanceId);
	pos = &UI_BOX_DATA[MOD_LAB_BOX].finalPos;
	renderString(0, pos->x + 8, pos->y + 6, 0xB8, MOD_LAB_LINES * 0xC, 0,
		     MOD_LAB_TEXT_Y, 0, 1);
	if (modLabMessage != NULL) {
		/* the message goes in the row right after the options (0xE8) */
		renderString(3, pos->x + 8, pos->y + 6 + MOD_LAB_LINES * 0xC, 0xB8, 0xC,
			     0, MOD_LAB_TEXT_Y + MOD_LAB_LINES * 0xC, 0, 1);
	}
}

static void modLabShowMessage(char *msg)
{
	RECT area;

	modLabMessage = msg;
	setRECT(&area, 0, MOD_LAB_TEXT_Y + MOD_LAB_LINES * 0xC, 0x100, 0xC);
	clearTextSubArea(&area);
	drawString(msg, 0, MOD_LAB_TEXT_Y + MOD_LAB_LINES * 0xC);
	DrawSync(0);
}

static void modLabFrames(int32_t n)
{
	while (n-- > 0) {
		pauseFrame();
	}
}

static void modLabApply(void)
{
	Stats *stats = &PARTNER_ENTITY.digimonEntity.stats;

	switch (modLabCursor) {
	case 0:
		MONEY += 1000;
		modLabShowMessage("Bits +1000");
		break;
	case 1:
		stats->current.currentHP = stats->base.hp;
		stats->current.currentMP = stats->base.mp;
		modLabShowMessage("Partner healed");
		break;
	case 2:
		if (isTriggerSet(MOD_LAB_TRIGGER)) {
			modLabShowMessage("Gift already taken");
		} else {
			giveItem(ITEM_MEAT, 1);
			setTrigger(MOD_LAB_TRIGGER);
			modLabShowMessage("You got 1 Meat");
		}
		break;
	}
}

/*
 * Called from the pause loop with the pad input already read. Returns when
 * the menu closes; the caller recreates the pause box.
 */
void modLabMenu(uint32_t *input, uint32_t *previous)
{
	RECT pos;
	uint32_t pressed;

	if (IS_SCRIPT_PAUSED != 1) {
		return; /* with a dialogue running the text area is in use */
	}

	removeStaticUIBox(MOD_LAB_BOX);
	modLabCursor = 0;
	modLabMessage = NULL;
	modLabDrawText();
	setRECT(&pos, -0x60, -0x48, 0xC8, (MOD_LAB_LINES + 2) * 0xC + 0xC);
	createStaticUIBox(MOD_LAB_BOX, 1, 0, &pos, NULL, modLabRender);
	playSound(0, 3);
	modLabFrames(12);

	for (;;) {
		*input = PadRead(1);
		pressed = *input & ~*previous;
		*previous = *input;
		if (pressed & (PAD_TRIANGLE | PAD_SELECT)) {
			break;
		}
		if (pressed & PAD_UP) {
			modLabCursor = (modLabCursor + MOD_LAB_OPTIONS - 1) % MOD_LAB_OPTIONS;
			playSound(0, 2);
		} else if (pressed & PAD_DOWN) {
			modLabCursor = (modLabCursor + 1) % MOD_LAB_OPTIONS;
			playSound(0, 2);
		} else if (pressed & PAD_CROSS) {
			if (modLabCursor == MOD_LAB_OPTIONS - 1) {
				break;
			}
			playSound(0, 3);
			modLabApply();
		} else {
			VSync(0);
			continue;
		}
		modLabDrawText();
		modLabFrames(2);
	}

	playSound(0, 4);
	removeStaticUIBox(MOD_LAB_BOX);
	setRECT(&pos, 0, MOD_LAB_TEXT_Y, 0x100, (MOD_LAB_LINES + 1) * 0xC);
	clearTextSubArea(&pos);
	modLabFrames(8);
}
