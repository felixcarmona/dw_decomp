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
 *   - "Status bubbles  All"  X (or left/right) switches Classic / All
 *                      (MOD status-bubbles)
 *   - "Close"
 * (The frame rate is not an option: it is chosen when the game is built,
 * Makefile FPS=20|30|60.)
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
#include <dw/screen.h>
#include <dw/mod_options.h>
#include <dw/tamer.h>
#include <dw/clock.h>
#include <dw/sound.h>
#include <text/main/mod_lab.h>   /* MOD (text sources): the lab menu's text, keyed (main.LAB_STR_*) */
#include <dw/species.h>
#include <dw/types.h>
#include <dw/ui.h>
#include <dw/anim.h>
#include <dw/model.h>
#include <dw/mod_font.h>
#include <dw/partner.h>
#include <dw/world_object.h>

#define MOD_LAB_TRIGGER	799	/* free and saved trigger (docs/mods/scripts.md, 717-799) */
#define MOD_LAB_BOX	5	/* same box the pause uses */
#define MOD_LAB_TEXT_Y	0x90	/* text area rows we use: 0x90..0xEF (0xF0 is the pause label) */
/* MOD (fps): no frame rate option; the frame rate is a build parameter
 * (Makefile FPS=20|30|60, docs/mods/60fps.md) */
#define MOD_LAB_OPTIONS	6
#define MOD_LAB_OPT_BUBBLES	4	/* MOD (status-bubbles): Classic / All */
#define MOD_LAB_OPT_EVOLVE	3	/* MOD (master-tyrannomon, mega) */
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
	LAB_STR_ADD_BITS,
	LAB_STR_HEAL,
	LAB_STR_GIFT,
	NULL, /* the evolve entry: "To <name>" of its target, drawn from the species data */
	LAB_STR_BUBBLES_ALL,
	LAB_STR_CLOSE,
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

/* the entries: fixed originals, the species of DIGI.DAT, the Mega */
static int32_t modLabEvoCount(void)
{
	return MOD_LAB_EVO_FIXED + getSpeciesPartnerCount() + 1;
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
	drawString(LAB_STR_TITLE, 0, TEXT_ROW_Y(ROW_TITLE));

	if (modLabMessage != NULL) {
		drawString(modLabMessage, 0, TEXT_ROW_Y(ROW_STATS));
	} else {
		p = appendStr(line, LAB_STR_BITS);
		p = appendStr(p, formatInteger(MONEY, num, 6));
		p = appendStr(p, LAB_STR_HP);
		p = appendStr(p, formatInteger(stats->current.currentHP, num, 4));
		p = appendStr(p, LAB_STR_OF);
		appendStr(p, formatInteger(stats->base.hp, num, 4));
		drawString(line, 0, TEXT_ROW_Y(ROW_STATS));
	}

	for (i = 0; i < MOD_LAB_OPTIONS; i++) {
		if (i == MOD_LAB_OPT_BUBBLES) {
			drawString(MOD_STATUS_BUBBLES ? LAB_STR_BUBBLES_ALL : LAB_STR_BUBBLES_CLASSIC,
				   0, TEXT_ROW_Y(ROW_OPTIONS + i));
		} else if (i == MOD_LAB_OPT_EVOLVE) {
			if (MOD_LAB_EVO_CHOICE >= MOD_LAB_EVO_FIXED + getSpeciesPartnerCount()) {
				/* MOD (mega): "To Mega <name>" */
				p = appendStr(modLabMegaText, LAB_STR_TO_MEGA);
				mega = modLabMegaTarget();
				appendStr(p, mega < 0 ? LAB_STR_NONE : DIGIMON_NAME(mega));
				drawString(modLabMegaText, 0, TEXT_ROW_Y(ROW_OPTIONS + i));
			} else {
				/* MOD (scale): "To <name>" of a species of DIGI.DAT; MOD (renames): of the fixed entries too,
				 * so the menu shows the species' name as the data has it */
				p = appendStr(modLabSpeciesText, LAB_STR_TO);
				mega = modLabEvoTarget();
				appendStr(p, mega < 0 ? LAB_STR_NONE : DIGIMON_NAME(mega));
				drawString(modLabSpeciesText, 0, TEXT_ROW_Y(ROW_OPTIONS + i));
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

static void modLabApply(void)
{
	Stats *stats = &PARTNER_ENTITY.digimonEntity.stats;
	int32_t mega;

	switch (MOD_LAB_CURSOR) {
	case 0:
		MONEY += 1000;
		modLabShowMessage(LAB_STR_MSG_BITS);
		break;
	case 1:
		stats->current.currentHP = stats->base.hp;
		stats->current.currentMP = stats->base.mp;
		modLabShowMessage(LAB_STR_MSG_HEALED);
		break;
	case 2:
		if (isTriggerSet(MOD_LAB_TRIGGER)) {
			modLabShowMessage(LAB_STR_MSG_GIFT_TAKEN);
		} else {
			giveItem(ITEM_MEAT, 1);
			setTrigger(MOD_LAB_TRIGGER);
			modLabShowMessage(LAB_STR_MSG_GOT_MEAT);
		}
		break;
	case MOD_LAB_OPT_BUBBLES:
		MOD_STATUS_BUBBLES = !MOD_STATUS_BUBBLES;
		break;
	case MOD_LAB_OPT_EVOLVE:
		/* MOD (master-tyrannomon, mega): as handleEvoItems does, with natural gains */
		mega = modLabEvoTarget();
		if (mega < 0) {
			modLabShowMessage(MOD_LAB_EVO_CHOICE >= MOD_LAB_EVO_FIXED + getSpeciesPartnerCount()
						  ? LAB_STR_MSG_NO_MEGA : LAB_STR_MSG_NOT_ON_DISC);
		} else if (!isPartnerModelFit(mega)) {
			modLabShowMessage(LAB_STR_MSG_TOO_BIG);
		} else if (PARTNER_ENTITY.digimonEntity.entity.type == mega) {
			modLabShowMessage(LAB_STR_MSG_ALREADY);
		} else if (partnerGetState() != 1 || tamerGetState() != 0 || EVOLUTION_TARGET != -1) {
			modLabShowMessage(LAB_STR_MSG_NOT_NOW);
		} else {
			EVOLUTION_TARGET = mega;
			HAS_USED_EVOITEM = 0;
			tamerSetState(6);
			partnerSetState(13);
			modLabShowMessage(LAB_STR_MSG_UNPAUSE);
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

	if (!MOD_OPT_CHEATS || IS_SCRIPT_PAUSED != 1) {
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

/*
 * MOD (roster switch, temporary test tool): on the map, SELECT + L1 turns the partner into the next species of
 * the roster and SELECT + R1 into the previous one (L2 and R2 are the screen jump, below), at once (no evolution scene), and a caption at the top of
 * the screen shows its name and level for a few seconds. The roster is every partner species in id order: the
 * original partners (1..65), the NPC partners of the lab menu and the partner species of DIGI.DAT; a model too big
 * for the partner buffer is skipped. The partner is reloaded as Continue loads it (loadMMD, its sounds), keeps its
 * stats and care, gets the species' first three techniques and its finisher as battle moves, and the species is
 * marked raised (setDigimonRaised), as if it had been reached by evolution.
 * Its sounds are staged at GENERAL_BUFFER, over the map background's palettes (memreclaim), so the .TFS header is
 * read again after them.
 * Hooked from tamerTickWalkingState() (tamer.c).
 */
#define PAD_L2		0x1
#define PAD_R2		0x2
#define ROSTER_CAPTION_ID	0x1f9a	/* world object id: above the species ids (world_object.h) */
#define ROSTER_CAPTION_TICKS	60	/* logic ticks the caption stays (20 a second) */
#define ROSTER_TEXT_Y		MOD_LAB_TEXT_Y	/* the lab menu's first text row: free while the game runs */

void learnMove(int32_t move);
void setDigimonRaised(int32_t digimonId);
int32_t isFileReadQueueBusy(void);
void readMapTFS(int32_t mapId);

extern Entity *ENTITY_TABLE[];

static char modRosterText[48];
static int32_t modRosterTimer;

static int32_t modRosterIsPartner(int32_t type)
{
	int32_t i;
	int32_t listed;

	listed = type >= 1 && type < NUM_PARTNER_DIGIMON;
	for (i = 0; !listed && i < MOD_LAB_EVO_FIXED; i++) {
		listed = MOD_LAB_EVO_TARGETS[i] == type;
	}
	for (i = 0; !listed && i < getSpeciesPartnerCount(); i++) {
		listed = getSpeciesPartnerId(i) == type;
	}
	return listed && isValidDigimonType(type) && isPartnerModelFit(type);
}

/* the species `step` (1 or -1) away from the partner's in the roster; the partner's own when there is no other */
static int32_t modRosterStep(int32_t step)
{
	int32_t type;
	int32_t i;

	type = PARTNER_ENTITY.digimonEntity.entity.type;
	for (i = 1; i < NUM_DIGIMON; i++) {
		type = ((type + step) % NUM_DIGIMON + NUM_DIGIMON) % NUM_DIGIMON;
		if (modRosterIsPartner(type)) {
			return type;
		}
	}
	return PARTNER_ENTITY.digimonEntity.entity.type;
}

/* battle moves: the first three techniques of the species and its finisher, learned (as tools/playtest/species_proof.py) */
static void modRosterSetMoves(int32_t type)
{
	uint8_t *moves;
	int32_t slot;
	int32_t i;

	moves = DIGI_PARA(type).moves;
	for (i = 0; i < 4; i++) {
		PARTNER_ENTITY.digimonEntity.stats.base.moves[i] = 0xff;
	}
	for (slot = 0, i = 0; i < 16; i++) {
		if (moves[i] == 0xff) {
			continue;
		}
		if (moves[i] >= 58 && moves[i] <= 112) {
			if (PARTNER_ENTITY.digimonEntity.stats.base.moves[3] == 0xff) {
				PARTNER_ENTITY.digimonEntity.stats.base.moves[3] = i + 0x2e;
				learnMove(moves[i]);
			}
		} else if (slot < 3) {
			PARTNER_ENTITY.digimonEntity.stats.base.moves[slot++] = i + 0x2e;
			learnMove(moves[i]);
		}
	}
}

static void modRosterRenderCaption(int32_t instanceId)
{
	int32_t width;

#if FONT_SWITCH
	if (FONT_PROPORTIONAL) {
		width = measureStringMode(modRosterText, FIT_VANILLA) + 2;
	} else
#endif
	{
		for (width = 4; modRosterText[(width - 4) / 8] != '\0'; width += 8) {
		}
	}
	renderString(0, -width / 2, -100, width, 12, 0, ROSTER_TEXT_Y, 0, 0);
}

static void modRosterTickCaption(int32_t instanceId)
{
	RECT area;

	if (--modRosterTimer <= 0) {
		removeObject(ROSTER_CAPTION_ID, 0);
		setRECT(&area, 0, ROSTER_TEXT_Y, 0x100, 0xC);
		clearTextSubArea(&area);
	}
}

static void modRosterShowCaption(int32_t type)
{
	static char *LEVEL_TEXT[] = {
		"", LAB_STR_LEVEL_FRESH, LAB_STR_LEVEL_IN_TRAINING, LAB_STR_LEVEL_ROOKIE,
		LAB_STR_LEVEL_CHAMPION, LAB_STR_LEVEL_ULTIMATE, LAB_STR_LEVEL_MEGA,
	};
	RECT area;
	char *p;
	int32_t level;

	level = DIGI_PARA(type).level;
	p = appendStr(modRosterText, DIGIMON_NAME(type));
	appendStr(p, level >= LEVEL_FRESH && level <= LEVEL_MEGA ? LEVEL_TEXT[level] : "");
	setRECT(&area, 0, ROSTER_TEXT_Y, 0x100, 0xC);
	clearTextSubArea(&area);
	drawString(modRosterText, 0, ROSTER_TEXT_Y);
	removeObject(ROSTER_CAPTION_ID, 0); /* a caption still up, or lost with the world objects (a battle, a map change) */
	addObject(ROSTER_CAPTION_ID, 0, modRosterTickCaption, modRosterRenderCaption);
	modRosterTimer = ROSTER_CAPTION_TICKS;
}

/* the partner becomes `type` where it stands (initializePartner's loading, the evolution's handover) */
static void modRosterSwitch(int32_t type)
{
	Entity *entity;
	int32_t x, y, z;
	int32_t rx, ry, rz;

	entity = &PARTNER_ENTITY.digimonEntity.entity;
	x = entity->posData->location.vx;
	y = entity->posData->location.vy;
	z = entity->posData->location.vz;
	rx = entity->posData->rotation.vx;
	ry = entity->posData->rotation.vy;
	rz = entity->posData->rotation.vz;
	modPartnerNameFollow(entity->type, type); /* MOD (optional name) */
	removeEntity(entity->type, 1);
	ENTITY_TABLE[1] = NULL;
	thunkLoadMMD(type, 3);
	ENTITY_TABLE[1] = (Entity *)&PARTNER_ENTITY;
	initializeDigimonObject(type, 1, partnerTick);
	setEntityPosition(1, x, y, z);
	setEntityRotation(1, rx, ry, rz);
	setupEntityMatrix(1);
	startAnimation(ENTITY_TABLE[1], 0);
	PARTNER_ENTITY.digimonEntity.stats.current.vabId = 4;
	loadPartnerSounds(type);
	readMapTFS(CURRENT_SCREEN); /* the sounds were staged over the background's palettes (GENERAL_BUFFER): read
	                              * them again, as the evolution's exit does, or the next time-of-day change loads noise */
	PARTNER_ENTITY.digimonEntity.entity.isOnMap = 1;
	PARTNER_ENTITY.digimonEntity.entity.isOnScreen = 1;
	modRosterSetMoves(type);
	setDigimonRaised(type); /* raised, as an evolution marks it (the chart, the raised count) */
}

/*
 * MOD (screen jump, temporary test tool): on the map, SELECT + R2 walks the tamer into the next new screen of the
 * disc (SCRN.DAT, ids 255..) and SELECT + L2 into the previous one, wrapping; from an original screen R2 goes to
 * the first and L2 to the last. The tamer arrives at the screen's exit 0, by the fade and caption of a warp tile.
 */
void unsetCameraFollowPlayer(void);

extern ScreenId TARGET_MAP;
extern uint8_t CURRENT_EXIT;

static int32_t modScreenJump(int32_t step)
{
	int32_t id = findNewScreen(CURRENT_SCREEN, step);

	if (id < 0 || id == CURRENT_SCREEN || !MOD_OPT_STORY) { /* MOD (story): the new screens are the extended story's */
		playSound(0, 4);
		return 0;
	}
	playSound(0, 2);
	TARGET_MAP = id;
	CURRENT_EXIT = 0;
	tamerSetState(5);
	unsetCameraFollowPlayer();
	stopGameTime();
	return 1;
}

/* returns 1 when it switched the partner or started a screen jump (the tamer's tick ends there) */
int32_t modRosterTick(void)
{
	uint32_t pressed;
	int32_t step;
	int32_t type;

	pressed = POLLED_INPUT & ~POLLED_INPUT_PREVIOUS;
	if (!MOD_OPT_CHEATS || !(POLLED_INPUT & PAD_SELECT) || !(pressed & (PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2))) {
		return 0;
	}
	if (IS_SCRIPT_PAUSED != 1 || partnerGetState() != 1 || EVOLUTION_TARGET != -1 || isFileReadQueueBusy()) {
		playSound(0, 4); /* not now: the press was heard */
		return 0;
	}
	if (pressed & (PAD_L2 | PAD_R2)) {
		return modScreenJump((pressed & PAD_R2) ? 1 : -1);
	}
	step = (pressed & PAD_L1) ? 1 : -1;
	type = modRosterStep(step);
	if (type != PARTNER_ENTITY.digimonEntity.entity.type) {
		modRosterSwitch(type);
	}
	playSound(0, 2);
	modRosterShowCaption(type);
	return 1;
}
