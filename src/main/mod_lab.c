/*
 * MOD: lab menu.
 *
 * With the game paused (START) and no script running, SELECT opens a box with
 * the partner's state and six options (up/down to choose, X to accept,
 * triangle or SELECT to close):
 *   - "+1000 bits"     adds 1000 to the money
 *   - "Heal partner"   current HP and MP to the maximum
 *   - "One-time gift"  mini quest: gives 1 Meat and sets MOD_LAB_TRIGGER;
 *                      afterwards it says the gift was already taken
 *   - "Frame rate  60 fps"  X (or right) cycles 20 -> 30 -> 60 fps, left
 *                      goes back (MOD_FPS_MODE, main.c; experimental branch)
 *   - "To MasterTyrannomon"  starts the real evolution sequence into the shown
 *                      species when the game is unpaused, the way an
 *                      evolution item does, but with the natural stat gains
 *                      (HAS_USED_EVOITEM = 0). Left/right choose the target:
 *                      "To MasterTyrannomon" (124), "To Cherrymon" (121; MOD
 *                      cherrymon), "To King of Sukamon" (120; MOD
 *                      partners-npc-a), "To Icemon" (94), "To Meteormon"
 *                      (113), the partners of MOD partners-npc-b, "To
 *                      Machinedramon" (115; MOD machinedramon), "To Jijimon"
 *                      (117), "To Analogman" (116), then every partner species
 *                      of DIGI.DAT in id order (MOD scale: the imported ones,
 *                      Veemon 180, Puppetmon, Kimeramon..., any id up to
 *                      NUM_DIGIMON; L1/R1 jump 10), and "To Mega <name>"
 *                      (MOD mega: the Mega the partner, an Ultimate, evolves
 *                      to, otherwise the first Mega species in DIGI.DAT,
 *                      docs/mods/mega.md); a species the disc's DIGI.DAT
 *                      does not define is skipped
 *   - "Close"
 *
 * The result of an option replaces the stats line until the next key, so the
 * box keeps 8 text rows (0x90..0xEF; the pause label is at 0xF0).
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
#include <dw/ids.h>
#include <dw/level.h>
#include <dw/params.h>
#include <dw/species.h>
#include <dw/font.h>
#include <dw/script.h>
#include <dw/sound.h>
#include <dw/species.h>
#include <dw/types.h>
#include <dw/ui.h>

#define MOD_LAB_TRIGGER	799	/* free and saved trigger (docs/mods/scripts.md, 717-799) */
#define MOD_LAB_BOX	5	/* same box the pause uses */
#define MOD_LAB_TEXT_Y	0x90	/* text area rows we use: 0x90..0xEF (0xF0 is the pause label) */
#define MOD_LAB_OPTIONS	7
#define MOD_LAB_OPT_BUBBLES	5	/* MOD (status-bubbles): Classic / All */
#define MOD_LAB_OPT_FPS	3	/* the frame rate option */
#define MOD_LAB_OPT_EVOLVE	4	/* MOD (master-tyrannomon, mega) */
#define ITEM_MEAT	38

#define PAD_SELECT	0x100
#define PAD_UP		0x1000
#define PAD_DOWN	0x4000
#define PAD_LEFT	0x8000
#define PAD_RIGHT	0x2000
#define PAD_L1		0x4
#define PAD_R1		0x8
#define PAD_TRIANGLE	0x10
#define PAD_CROSS	0x40

extern UIBoxData UI_BOX_DATA[];
extern int32_t IS_SCRIPT_PAUSED;
extern int32_t MOD_FPS_MODE; /* 0 = 20 fps, 1 = 30, 2 = 60 (main.c) */
extern int16_t EVOLUTION_TARGET;
extern uint8_t HAS_USED_EVOITEM;

void drawString(char *str, int32_t x, int32_t y);
void renderString(int32_t color, int32_t x, int32_t y, int32_t w, int32_t h,
		  int32_t u, int32_t v, int32_t layer, int32_t shadow);
void clearTextSubArea(RECT *rect);
char *formatInteger(int32_t value, char *buf, int32_t digits);
int32_t giveItem(ItemId item, uint8_t amount);
void pauseFrame(void);
void renderSelectionCursor(int32_t x, int32_t y, int16_t w, int16_t h, int32_t layer);
int32_t partnerGetState(void);
void partnerSetState(int32_t state);
int32_t tamerGetState(void);
void tamerSetState(int32_t state);

extern int32_t MOD_STATUS_BUBBLES;
int32_t MOD_LAB_CURSOR; /* the lab menu's cursor (global for tests) */
int32_t MOD_LAB_EVO_CHOICE; /* MOD (cherrymon): the evolve entry (global so tests can read and set it) */
static char *modLabMessage;

static char *MOD_LAB_OPTION_TEXT[MOD_LAB_OPTIONS] = {
	"+1000 bits",
	"Heal partner",
	"One-time gift",
	NULL, /* MOD_LAB_FPS_TEXT */
	NULL, /* MOD_LAB_EVO_TEXT */
	"Status bubbles  All",
	"Close",
};

/*
 * MOD (cherrymon): the evolve option cycles through these targets.
 * MOD (partners-npc-c): the count follows the arrays, so new targets are
 * only appended to both.
 */
/* MOD (mega): the entry whose target is modLabMegaTarget() */
#define MOD_LAB_EVO_MEGA	-2

static int16_t MOD_LAB_EVO_TARGETS[] = {
	DIGIMON_MASTER_TYRANNOMON,
	DIGIMON_CHERRYMON,
	DIGIMON_KING_SUKAMON, /* MOD (partners-npc-a) */
	DIGIMON_ICEMON,       /* MOD (partners-npc-a) */
	DIGIMON_METEORMON,    /* MOD (partners-npc-a) */
	DIGIMON_WARUMONZAEMON, /* MOD (partners-npc-b) */
	DIGIMON_MYOTISMON,
	DIGIMON_SHOGUNGEKOMON,
	DIGIMON_MACHINEDRAMON, /* MOD (machinedramon) */
	DIGIMON_JIJIMON,       /* MOD (jijimon) */
	DIGIMON_ANALOGMAN,     /* MOD (analogman) */
};
/* MOD (scale): after these come the partner species of DIGI.DAT (getSpeciesPartnerId), then the Mega entry */
#define MOD_LAB_EVO_FIXED ((int32_t)(sizeof(MOD_LAB_EVO_TARGETS) / sizeof(MOD_LAB_EVO_TARGETS[0])))

static char *MOD_LAB_EVO_TEXT[] = {
	"To MasterTyrannomon",
	"To Cherrymon",
	"To King of Sukamon", /* MOD (partners-npc-a) */
	"To Icemon",          /* MOD (partners-npc-a) */
	"To Meteormon",       /* MOD (partners-npc-a) */
	"To WaruMonzaemon", /* MOD (partners-npc-b) */
	"To Myotismon",
	"To ShogunGekomon",
	"To Machinedramon", /* MOD (machinedramon) */
	"To Jijimon",       /* MOD (jijimon) */
	"To Analogman",     /* MOD (analogman) */
};
/* the entries: fixed originals, the species of DIGI.DAT, the Mega */
static int32_t modLabEvoCount(void)
{
	return MOD_LAB_EVO_FIXED + getSpeciesPartnerCount() + 1;
}

static char *MOD_LAB_FPS_TEXT[3] = {
	"Frame rate  20 fps",
	"Frame rate  30 fps",
	"Frame rate  60 fps",
};

static int32_t modLabFpsMode(void)
{
	if (MOD_FPS_MODE < 0) {
		return 0;
	}
	return MOD_FPS_MODE > 2 ? 2 : MOD_FPS_MODE;
}

static char *appendStr(char *dst, char *src)
{
	while (*src != '\0') {
		*dst++ = *src++;
	}
	*dst = '\0';
	return dst;
}

/* MOD (mega): the Mega the partner evolves to if it is an Ultimate with one in
 * its paths, otherwise the first Mega species (-1: none on the disc) */
static int32_t modLabMegaTarget(void)
{
	EvolutionPath *path;
	int32_t type;
	int32_t i;

	type = PARTNER_ENTITY.digimonEntity.entity.type;
	if (DIGI_PARA(type).level == LEVEL_ULTIMATE) {
		path = getEvolutionPath(type);
		for (i = 0; i < 6; i++) {
			if (path->to[i] > 0 && DIGI_PARA(path->to[i]).level == LEVEL_MEGA) {
				return path->to[i];
			}
		}
	}
	/* MOD (scale): the first Mega partner species, from the DIGI.DAT index */
	type = getFirstMegaSpecies();
	return type > 0 ? type : -1;
}

static char modLabMegaText[32];
static char modLabSpeciesText[32]; /* MOD (scale): "To <name>" of a DIGI.DAT species */

/* MOD (integration): the species of the current evolve entry, -1 when the disc has none */
static int32_t modLabEvoTarget(void)
{
	int32_t t;

	if (MOD_LAB_EVO_CHOICE >= MOD_LAB_EVO_FIXED + getSpeciesPartnerCount()) {
		return modLabMegaTarget();
	}
	if (MOD_LAB_EVO_CHOICE >= MOD_LAB_EVO_FIXED) {
		return getSpeciesPartnerId(MOD_LAB_EVO_CHOICE - MOD_LAB_EVO_FIXED); /* defined partner species */
	}
	t = MOD_LAB_EVO_TARGETS[MOD_LAB_EVO_CHOICE];
	return isValidDigimonType(t) ? t : -1;
}

/* MOD (dcb-import): next target (step 1 or COUNT-1) whose species exists */
static void modLabStepEvoChoice(int32_t step)
{
	int32_t i;

	int32_t count;

	count = modLabEvoCount();
	for (i = 0; i < count; i++) {
		MOD_LAB_EVO_CHOICE = ((MOD_LAB_EVO_CHOICE + step) % count + count) % count;
		if (modLabEvoTarget() >= 0) {
			break;
		}
	}
}

/* Layout copied from the game's own menus (inventory option boxes): text 12 px
 * inside the box, one row every 0x12 px, the native selection bar behind the
 * selected option. Each row has its own 0xC-high line in the VRAM text area. */
#define ROW_PITCH	0x12
#define BOX_W		0xD0
#define BOX_SHIFT_X	0x16	/* 22 px right of centre: the HUD clock (top left) reaches x = 77 */
#define ROW_TITLE	0
#define ROW_STATS	1
#define ROW_OPTIONS	2
#define ROW_COUNT	(ROW_OPTIONS + MOD_LAB_OPTIONS)
#define TEXT_ROW_Y(r)	(MOD_LAB_TEXT_Y + (r) * 0xC)
#define BOX_H		(8 + ROW_COUNT * ROW_PITCH + 2) /* centred vertically */

static void modLabClearRow(int32_t row)
{
	RECT area;

	setRECT(&area, 0, TEXT_ROW_Y(row), 0x100, 0xC);
	clearTextSubArea(&area);
}

static void modLabDrawText(void)
{
	char line[40];
	char num[12];
	char *p;
	int32_t i;
	int32_t mega;
	Stats *stats = &PARTNER_ENTITY.digimonEntity.stats;

	for (i = 0; i < ROW_COUNT; i++) {
		modLabClearRow(i);
	}
	drawString("LAB MENU", 0, TEXT_ROW_Y(ROW_TITLE));

	if (modLabMessage != NULL) {
		drawString(modLabMessage, 0, TEXT_ROW_Y(ROW_STATS));
	} else {
		p = appendStr(line, "Bits ");
		p = appendStr(p, formatInteger(MONEY, num, 6));
		p = appendStr(p, "   HP ");
		p = appendStr(p, formatInteger(stats->current.currentHP, num, 4));
		p = appendStr(p, " of ");
		appendStr(p, formatInteger(stats->base.hp, num, 4));
		drawString(line, 0, TEXT_ROW_Y(ROW_STATS));
	}

	for (i = 0; i < MOD_LAB_OPTIONS; i++) {
		if (i == MOD_LAB_OPT_FPS) {
			drawString(MOD_LAB_FPS_TEXT[modLabFpsMode()], 0, TEXT_ROW_Y(ROW_OPTIONS + i));
		} else if (i == MOD_LAB_OPT_BUBBLES) {
			drawString(MOD_STATUS_BUBBLES ? "Status bubbles  All" : "Status bubbles  Classic",
				   0, TEXT_ROW_Y(ROW_OPTIONS + i));
		} else if (i == MOD_LAB_OPT_EVOLVE) {
			if (MOD_LAB_EVO_CHOICE >= MOD_LAB_EVO_FIXED + getSpeciesPartnerCount()) {
				/* MOD (mega): "To Mega <name>" */
				p = appendStr(modLabMegaText, "To Mega ");
				mega = modLabMegaTarget();
				appendStr(p, mega < 0 ? "none" : DIGIMON_NAME(mega));
				drawString(modLabMegaText, 0, TEXT_ROW_Y(ROW_OPTIONS + i));
			} else if (MOD_LAB_EVO_CHOICE >= MOD_LAB_EVO_FIXED) {
				/* MOD (scale): "To <name>" of a species of DIGI.DAT */
				p = appendStr(modLabSpeciesText, "To ");
				mega = modLabEvoTarget();
				appendStr(p, mega < 0 ? "none" : DIGIMON_NAME(mega));
				drawString(modLabSpeciesText, 0, TEXT_ROW_Y(ROW_OPTIONS + i));
			} else {
				drawString(MOD_LAB_EVO_TEXT[MOD_LAB_EVO_CHOICE], 0, TEXT_ROW_Y(ROW_OPTIONS + i));
			}
		} else {
			drawString(MOD_LAB_OPTION_TEXT[i], 0, TEXT_ROW_Y(ROW_OPTIONS + i));
		}
	}
	DrawSync(0);
}

static void modLabRender(int32_t instanceId)
{
	GsBOXF box;
	RECT *pos;
	int32_t row;
	int32_t color;

	box.attribute = 0x40000000;
	box.r = box.g = box.b = 0;
	setRECT(&box, -0xA0, -0x78, 0x140, 0xF0);
	GsSortBoxFill(&box, ACTIVE_ORDERING_TABLE, 7 - instanceId);
	pos = &UI_BOX_DATA[MOD_LAB_BOX].finalPos;

	for (row = 0; row < ROW_COUNT; row++) {
		color = row == ROW_TITLE ? 4 : (row == ROW_STATS && modLabMessage != NULL) ? 3 : 0;
		renderString(color, pos->x + 12, pos->y + 8 + row * ROW_PITCH, BOX_W - 24, 0xC,
			     0, TEXT_ROW_Y(row), 0, 1);
	}
	renderSelectionCursor(pos->x + 9, pos->y + 6 + (ROW_OPTIONS + MOD_LAB_CURSOR) * ROW_PITCH,
			      BOX_W - 18, 0x10, 0);
}

/* shown in the stats row by the next modLabDrawText, until the next key */
static void modLabShowMessage(char *msg)
{
	modLabMessage = msg;
}

static void modLabFrames(int32_t n)
{
	while (n-- > 0) {
		pauseFrame();
	}
}

static void modLabSetFps(int32_t step)
{
	MOD_FPS_MODE = (modLabFpsMode() + step + 3) % 3;
	/* the option shows the new rate; an older message would only confuse */
	modLabMessage = NULL;
}

static void modLabApply(void)
{
	Stats *stats = &PARTNER_ENTITY.digimonEntity.stats;
	int32_t mega;

	switch (MOD_LAB_CURSOR) {
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
	case MOD_LAB_OPT_FPS:
		modLabSetFps(1);
		break;
	case MOD_LAB_OPT_BUBBLES:
		MOD_STATUS_BUBBLES = !MOD_STATUS_BUBBLES;
		break;
	case MOD_LAB_OPT_EVOLVE:
		/* MOD (master-tyrannomon, mega): as handleEvoItems does, with natural gains */
		mega = modLabEvoTarget();
		if (mega < 0) {
			modLabShowMessage(MOD_LAB_EVO_CHOICE >= MOD_LAB_EVO_FIXED + getSpeciesPartnerCount()
						  ? "No Mega on this disc" : "Not on this disc");
		} else if (!isPartnerModelFit(mega)) {
			modLabShowMessage("Model too big for it");
		} else if (PARTNER_ENTITY.digimonEntity.entity.type == mega) {
			modLabShowMessage("Already evolved");
		} else if (partnerGetState() != 1 || tamerGetState() != 0 || EVOLUTION_TARGET != -1) {
			modLabShowMessage("Not now, try later");
		} else {
			EVOLUTION_TARGET = mega;
			HAS_USED_EVOITEM = 0;
			tamerSetState(6);
			partnerSetState(13);
			modLabShowMessage("Unpause to evolve");
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
	MOD_LAB_CURSOR = 0;
	modLabMessage = NULL;
	modLabDrawText();
	setRECT(&pos, -BOX_W / 2 + BOX_SHIFT_X, -BOX_H / 2, BOX_W, BOX_H);
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
		if (pressed & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_L1 | PAD_R1 | PAD_CROSS)) {
			modLabMessage = NULL;
		}
		if (pressed & PAD_UP) {
			MOD_LAB_CURSOR = (MOD_LAB_CURSOR + MOD_LAB_OPTIONS - 1) % MOD_LAB_OPTIONS;
			playSound(0, 2);
		} else if (pressed & PAD_DOWN) {
			MOD_LAB_CURSOR = (MOD_LAB_CURSOR + 1) % MOD_LAB_OPTIONS;
			playSound(0, 2);
		} else if ((pressed & (PAD_LEFT | PAD_RIGHT)) && MOD_LAB_CURSOR == MOD_LAB_OPT_FPS) {
			playSound(0, 2);
			modLabSetFps((pressed & PAD_RIGHT) ? 1 : -1);
		} else if ((pressed & (PAD_LEFT | PAD_RIGHT)) && MOD_LAB_CURSOR == MOD_LAB_OPT_BUBBLES) {
			playSound(0, 2);
			MOD_STATUS_BUBBLES = !MOD_STATUS_BUBBLES;
		} else if ((pressed & (PAD_LEFT | PAD_RIGHT)) && MOD_LAB_CURSOR == MOD_LAB_OPT_EVOLVE) {
			/* MOD (cherrymon): choose the evolution target */
			playSound(0, 2);
			modLabStepEvoChoice((pressed & PAD_RIGHT) ? 1 : -1);
		} else if ((pressed & (PAD_L1 | PAD_R1)) && MOD_LAB_CURSOR == MOD_LAB_OPT_EVOLVE) {
			playSound(0, 2);
			modLabStepEvoChoice((pressed & PAD_R1) ? 10 : -10);
		} else if (pressed & PAD_CROSS) {
			if (MOD_LAB_CURSOR == MOD_LAB_OPTIONS - 1) {
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
	setRECT(&pos, 0, MOD_LAB_TEXT_Y, 0x100, ROW_COUNT * 0xC);
	clearTextSubArea(&pos);
	modLabFrames(8);
}
