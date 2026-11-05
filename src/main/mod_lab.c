/* Laboratory UI and scenario editing; the field shortcuts below share its roster. */
#include <libetc.h>
#include <stddef.h>
#include <string.h>
#include <dw/btl.h>
#include <dw/entity.h>
#include <dw/ids.h>
#include <dw/level.h>
#include <dw/mod_rules.h>
#include <dw/caps.h>
#include <dw/params.h>
#include <dw/species.h>
#include <dw/font.h>
#include <dw/script.h>
#include <dw/screen.h>
#include <dw/mod_options.h>
#include <dw/input.h>
#include <dw/tamer.h>
#include <dw/clock.h>
#include <dw/sound.h>
#include <dw/ui.h>
#include <dw/anim.h>
#include <dw/model.h>
#include <dw/mod_font.h>
#include <dw/partner.h>
#include <dw/world_object.h>
#include <dw/bubble.h>
#include <text/main/mod_lab.h>

#include "mod_lab_menu.inc"

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

/* The synchronous UI shares MENU's battle slot. The world is paused; sound
 * staging stays below the slot. Card menus reload the same overlay at the same
 * addresses, while all live state remains resident or on the stack. */
void loadMenuOverlay(void);
void closeMenuOverlay(void);
void tickFileReadQueue(int32_t unused);
void modLabMenu(uint32_t *input, uint32_t *previous)
{
    if (!MOD_OPT_CHEATS || IS_SCRIPT_PAUSED != 1) return;
    while (isFileReadQueueBusy()) tickFileReadQueue(0);
    DrawSync(0);
    loadMenuOverlay();
    modLabMenuLoaded(input, previous);
    closeMenuOverlay();
}
