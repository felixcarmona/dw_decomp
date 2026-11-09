#include <string.h>

#include <libgpu.h>
#include <libgs.h>

#include <dw/mod_options.h>

#include <dw/anim.h>
#include <dw/bubble.h>
#include <dw/butterfly.h>
#include <dw/clock.h>
#include <dw/dooa.h>
#include <dw/entity.h>
#include <dw/evl.h>
#include <dw/evolution.h>
#include <dw/file.h>
#include <dw/font.h>
#include <dw/garbage.h>
#include <dw/ids.h>
#include <dw/item.h>
#include <dw/level.h>
#include <dw/species.h>
#include <dw/mod_rules.h>
#include <dw/map.h>
#include <dw/map_object.h>
#include <dw/math.h>
#include <dw/model.h>
#include <dw/move.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/sound.h>
#include <dw/tamer.h>
#include <dw/types.h>
#include <dw/version.h>
#include <dw/world_object.h>
#include <text/main/partner_impl.h>

extern int8_t PARTNER_ANIMATION;
extern GsOT *ACTIVE_ORDERING_TABLE;

extern int32_t HAS_IMMORTAL_HOUR;
extern int8_t IMMORTAL_HOUR;
extern int16_t EVOLUTION_TARGET;
extern uint8_t HAS_USED_EVOITEM;

IS_SICK_TEXT

char MAIN_D_801225AC[16] = "\\ETCNA\\UNTI.TMD";
RaiseData RAISE_DATA[NUM_ORIGINAL_DIGIMON] = { /* MOD: was NUM_PARTNER_DIGIMON */
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 670, 200, -2270 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 6, 3, 18, 1200, 6, 0, 6, 8, 0, 5, 710, 140, -2210 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 10, 3, 18, 1200, 7, 0, 7, 8, 0, 10, 460, 210, -1770 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  40, 20, 5, 24, 1200, 8, 38, 1, 0, 0, 15, 550, 230, -1870 },
	{ { 0, 5, 9, 14, 19, -1, -1, -1 },
	  40, 22, 5, 24, 1200, 8, 63, 2, 4, 3, 15, 370, 230, -1570 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 45, 9, 36, 1200, 12, 40, 1, 0, 0, 30, 30, 390, -880 },
	{ { 4, 9, 14, 18, 23, -1, -1, -1 },
	  80, 30, 6, 36, 1200, 10, 46, 4, 6, 0, 20, 130, 350, -1120 },
	{ { 5, 11, 17, 23, -1, -1, -1, -1 },
	  80, 35, 6, 36, 1200, 10, 40, 1, 2, 2, 30, 260, 380, -1450 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  80, 55, 14, 36, 1200, 13, 51, 3, 0, 0, 30, 50, 340, -940 },
	{ { 4, 9, 14, 18, 23, -1, -1, -1 },
	  80, 35, 8, 36, 1200, 10, 41, 4, 0, 1, 20, 130, 360, -1090 },
	{ { 0, 4, 8, 12, 16, 20, -1, -1 },
	  80, 42, 11, 36, 1200, 11, 65, 2, 4, 3, 30, 230, 280, -1400 },
	{ { 0, 3, 6, 9, 12, 15, 18, 21 },
	  80, 34, 11, 36, 1200, 13, 68, 2, 6, 4, 20, 270, 280, -1340 },
	{ { 0, 4, 8, 12, 16, 20, -1, -1 },
	  80, 45, 11, 36, 1200, 15, 53, 2, 3, 0, 50, -30, 390, -780 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 24, 5, 36, 1200, 8, 52, 1, 3, 0, 5, 570, 170, -1900 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 40, 8, 36, 1200, 10, 47, 1, 2, 3, 40, 340, 300, -1460 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 6, 3, 18, 1200, 6, 0, 6, 8, 3, 5, 660, 160, -2090 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 10, 3, 18, 1200, 7, 0, 7, 8, 3, 10, 460, 220, -1720 },
	{ { 2, 5, 8, 11, 14, 17, 20, 23 },
	  40, 25, 8, 24, 1200, 8, 38, 2, 5, 3, 15, 420, 230, -1610 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  40, 23, 6, 24, 1200, 8, 62, 1, 5, 1, 15, 250, 240, -1410 },
	{ { 2, 7, 12, 17, 21, -1, -1, -1 },
	  80, 45, 9, 36, 1200, 12, 57, 5, 7, 1, 30, 210, 360, -1290 },
	{ { 7, 12, 17, -1, -1, -1, -1, -1 },
	  80, 24, 4, 36, 1200, 9, 59, 1, 2, 2, 20, 50, 350, -1030 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 35, 8, 36, 1200, 10, 49, 1, 0, 1, 20, 250, 400, -1320 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 35, 8, 36, 1200, 10, 39, 1, 1, 0, 30, 190, 290, -1200 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  80, 42, 11, 36, 1200, 11, 45, 3, 1, 2, 30, 80, 340, -1020 },
	{ { 2, 6, 10, 14, 18, 21, -1, -1 },
	  80, 50, 13, 36, 1200, 15, 48, 1, 4, 3, 40, 200, 360, -1360 },
	{ { 3, 7, 11, 14, 18, 23, -1, -1 },
	  80, 40, 10, 36, 1200, 11, 42, 0, 5, 2, 10, 280, 270, -1450 },
	{ { 4, 12, 20, -1, -1, -1, -1, -1 },
	  80, 30, 4, 36, 1200, 14, 67, 4, 6, 1, 30, 30, 390, -940 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 24, 5, 36, 1200, 8, 52, 1, 3, 3, 5, 550, 170, -1900 },
	{ { 4, 9, 14, 18, 23, -1, -1, -1 },
	  80, 35, 7, 36, 1200, 10, 53, 4, 3, 2, 15, 440, 350, -1730 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 6, 3, 18, 1200, 6, 0, 6, 8, 1, 5, 680, 270, -2140 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 10, 3, 18, 1200, 7, 0, 7, 8, 1, 10, 420, 220, -1630 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  40, 20, 5, 24, 1200, 8, 63, 1, 5, 0, 15, 470, 180, -1720 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  40, 20, 5, 24, 1200, 8, 69, 4, 7, 1, 15, 290, 280, -1380 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 35, 7, 36, 1200, 11, 39, 1, 2, 0, 30, 240, 370, -1360 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  80, 48, 12, 36, 1200, 13, 50, 3, 5, 1, 30, 280, 340, -1390 },
	{ { 0, 4, 8, 12, 16, 20, -1, -1 },
	  80, 56, 14, 36, 1200, 15, 54, 2, 4, 3, 40, 0, 320, -880 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 40, 8, 36, 1200, 11, 58, 1, 0, 2, 30, 310, 370, -1370 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 33, 8, 36, 1200, 9, 69, 4, 6, 2, 20, 490, 340, -1810 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 60, 12, 36, 1200, 12, 39, 4, 5, 1, 40, 210, 340, -1220 },
	{ { 0, 3, 6, 9, 12, 15, 18, 21 },
	  80, 35, 11, 36, 1200, 13, 68, 1, 6, 4, 30, 300, 260, -1420 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 32, 6, 36, 1200, 10, 46, 1, 3, 2, 40, 260, 360, -1400 },
	{ { 4, 9, 13, 18, 23, -1, -1, -1 },
	  80, 30, 6, 36, 1200, 8, 50, 3, 3, 1, 5, 370, 270, -1610 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 40, 10, 36, 1200, 10, 55, 1, 5, 1, 20, 500, 230, -1870 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 6, 3, 18, 1200, 6, 0, 6, 8, 2, 5, 670, 230, -2100 },
	{ { -1, -1, -1, -1, -1, -1, -1, -1 },
	  25, 10, 3, 18, 1200, 7, 0, 7, 8, 2, 10, 330, 230, -1480 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  40, 20, 5, 24, 1200, 8, 44, 1, 5, 2, 15, 540, 210, -1880 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  40, 23, 6, 24, 1200, 8, 44, 0, 5, 2, 15, 480, 260, -1750 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 53, 12, 36, 1200, 13, 66, 1, 0, 3, 40, 0, 350, -870 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 33, 7, 36, 1200, 10, 40, 1, 5, 0, 20, 200, 330, -1320 },
	{ { 4, 8, 13, 18, 23, -1, -1, -1 },
	  80, 35, 7, 36, 1200, 10, 64, 3, 4, 1, 30, 270, 380, -1400 },
	{ { 3, 8, 13, 18, 23, -1, -1, -1 },
	  80, 40, 8, 36, 1200, 11, 65, 0, 5, 3, 30, 310, 330, -1520 },
	{ { 4, 9, 14, 18, 23, -1, -1, -1 },
	  80, 43, 9, 36, 1200, 12, 56, 4, 7, 3, 30, 60, 333, -1030 },
	{ { 3, 8, 13, 18, 23, -1, -1, -1 },
	  80, 42, 8, 36, 1200, 11, 63, 0, 1, 0, 20, 360, 270, -1620 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  80, 35, 9, 36, 1200, 10, 40, 3, 6, 4, 20, 530, 230, -1890 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 45, 11, 36, 1200, 14, 48, 4, 3, 0, 50, 0, 460, -940 },
	{ { 3, 8, 13, 18, 23, -1, -1, -1 },
	  80, 24, 5, 36, 1200, 8, 52, 0, 2, 2, 5, 470, 330, -1800 },
	{ { 0, 3, 6, 9, 12, 15, 18, 21 },
	  80, 50, 14, 36, 1200, 9, 70, 1, 6, 0, 10, 550, 200, -1960 },
	{ { 1, 5, 9, 13, 17, 21, -1, -1 },
	  40, 22, 6, 24, 1200, 8, 62, 1, 1, 0, 15, 450, 240, -1760 },
	{ { 3, 8, 13, 18, 23, -1, -1, -1 },
	  80, 24, 6, 36, 1200, 8, 52, 0, 5, 1, 10, 550, 180, -1920 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 35, 7, 36, 1200, 12, 61, 1, 0, 2, 30, 120, 430, -1230 },
	{ { 2, 7, 12, 17, 21, -1, -1, -1 },
	  80, 45, 9, 36, 1200, 13, 60, 5, 7, 3, 40, 0, 360, -950 },
	{ { 0, 5, 9, 14, 19, -1, -1, -1 },
	  80, 42, 8, 36, 1200, 12, 67, 2, 4, 1, 30, 110, 320, -1180 },
	{ { 0, 0, 0, 0, 0, -1, -1, -1 },
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 33, 7, 36, 1200, 10, 40, 1, 2, 0, 20, 200, 330, -1320 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 45, 11, 36, 1200, 14, 48, 4, 3, 0, 50, 0, 460, -940 },
	{ { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 40, 10, 36, 1200, 10, 55, 1, 3, 1, 20, 500, 230, -1870 },
	/*
	 * MOD (master-tyrannomon): Tyrannomon's row (8) for MasterTyrannomon,
	 * with the default weight and the view offsets 1.5 times larger (its
	 * model is Tyrannomon's at 1.5 times). Types without a row are zero:
	 * they never become partners.
	 */
	[DIGIMON_MASTER_TYRANNOMON] = { { 1, 5, 9, 13, 17, 21, -1, -1 },
	  80, 55, 14, 36, 1200, 13, 51, 3, 0, 0, 45, 75, 510, -1410 },
	/*
	 * MOD (cherrymon): Vegiemon's row (25) with fewer, bigger meals and
	 * bigger poop, Red Berry (49) as the favourite food and the weight of
	 * a tree. The status screen draws the model at (viewX, viewY, viewZ)
	 * seen from DIGIVICE_ENTITY_VIEW (1300, 0, -3280): this puts the whole
	 * 650-high tree inside the Condition panel, feet at its bottom.
	 */
	[DIGIMON_CHERRYMON] = { { 3, 8, 13, 18, 23, -1, -1, -1 },
	  80, 35, 8, 36, 1200, 14, 49, 0, 5, 2, 30, -511, 630, -232 },
	/*
	 * MOD (partners-npc-a): Sukamon's row (39) for King of Sukamon, with
	 * bigger poop and weight 40. The status screen draws the model at
	 * (viewX, viewY, viewZ) seen from DIGIVICE_ENTITY_VIEW (1300, 0,
	 * -3280): Sukamon's point pushed 1.71 times as far from the eye, so
	 * the 1.71 times larger King fills the Condition panel the same way.
	 */
	[DIGIMON_KING_SUKAMON] = { { 0, 3, 6, 9, 12, 15, 18, 21 },
	  80, 35, 11, 36, 1200, 16, 68, 1, 6, 4, 40, -410, 445, -99 },
	/*
	 * MOD (partners-npc-a): Frigimon's row (23) for Icemon, weight 40 (a
	 * rock); the status-screen view is Frigimon's point brought 0.82
	 * times as far from the eye (Icemon is 310 high, Frigimon 380).
	 */
	[DIGIMON_ICEMON] = { { 1, 5, 9, 13, 17, 21, -1, -1 },
	  80, 42, 11, 36, 1200, 11, 45, 3, 1, 2, 40, 305, 277, -1436 },
	/*
	 * MOD (partners-npc-a): Monochromon's row (47) for Meteormon, weight
	 * 50; the status-screen view is Monochromon's point brought 1.21
	 * times as far from the eye (Meteormon is 400 high, Monochromon 330).
	 */
	[DIGIMON_METEORMON] = { { 2, 6, 10, 14, 18, 22, -1, -1 },
	  80, 53, 12, 36, 1200, 13, 66, 1, 0, 3, 50, -273, 424, -364 },
	/*
	 * MOD (partners-npc-b): Monzaemon's row (14). The status screen view
	 * keeps Monzaemon's framing for a model 1.25 times as tall: the same
	 * point seen from DIGIVICE_ENTITY_VIEW (1300, 0, -3280), 1.25 times as
	 * far away.
	 */
	/*
	 * MOD (partners-npc-b): Devimon's row (6), weight 30; the status view
	 * keeps Devimon's framing for a model 430/400 times as tall.
	 */
	/*
	 * MOD (partners-npc-b): Whamon's row (24), weight 60; the status view
	 * frames the 540-high frog like Cherrymon's tree (Cherrymon's point
	 * moved 540/650 of the way from DIGIVICE_ENTITY_VIEW).
	 */
	[DIGIMON_SHOGUNGEKOMON] = { { 2, 6, 10, 14, 18, 21, -1, -1 },
	  80, 50, 13, 36, 1200, 15, 48, 1, 4, 3, 60, -203, 523, -748 },
	[DIGIMON_MYOTISMON] = { { 4, 9, 14, 18, 23, -1, -1, -1 },
	  80, 30, 6, 36, 1200, 10, 46, 4, 6, 0, 30, 42, 376, -958 },
	[DIGIMON_WARUMONZAEMON] = { { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 40, 8, 36, 1200, 10, 47, 1, 2, 3, 40, 100, 375, -1005 },
	/*
	 * MOD (machinedramon): MetalGreymon's row (12), a bit hungrier, with a
	 * bigger poop and the weight of a war machine. The status-screen view
	 * (viewX/Y/Z, see Cherrymon's row) puts the whole 600-high body, cannons
	 * and tail included, in the Condition panel (tried in the game).
	 */
	[DIGIMON_MACHINEDRAMON] = { { 0, 4, 8, 12, 16, 20, -1, -1 },
	  80, 50, 13, 36, 1200, 16, 53, 2, 3, 0, 70, -560, 640, -150 },
	/*
	 * MOD (jijimon): Ogremon's row (34) for a small old man: smaller
	 * meals and poop, an early sleeper, light (weight 20); the
	 * status-screen view fits him in the Condition panel.
	 */
	[DIGIMON_JIJIMON] = { { 1, 5, 9, 13, 17, 21, -1, -1 },
	  80, 40, 9, 36, 1200, 8, 50, 1, 5, 1, 20, 280, 340, -1390 },
	/*
	 * MOD (analogman): Andromon's row (40): a late sleeper with regular
	 * meals, weight 30; the status-screen view fits him in the Condition
	 * panel.
	 */
	[DIGIMON_ANALOGMAN] = { { 2, 6, 11, 16, 21, -1, -1, -1 },
	  80, 32, 6, 36, 1200, 10, 46, 1, 3, 2, 30, 260, 360, -1400 },
};

typedef struct {
	uint8_t sleepyHour;
	uint8_t sleepyMinute;
	uint8_t wakeupHour;
	uint8_t wakeupMinute;
	uint8_t awakeHours;
	uint8_t sleepyHours;
} SleepPattern;

SleepPattern SLEEP_PATTERN[8] = {
	{16, 0, 1, 0, 15, 9},
	{19, 0, 4, 0, 15, 9},
	{22, 0, 7, 0, 15, 9},
	{2, 0, 11, 0, 15, 9},
	{7, 0, 16, 0, 15, 9},
	{10, 0, 19, 0, 15, 9},
	{0, 0, 0, 0, 4, 1},
	{0, 0, 0, 0, 8, 3},
};

/*
 * MOD (species JSON): the stats a Fresh species hatches with from the reincarnation egg, and the technique the
 * new-game partner starts with; were literals in initializeReincarnatedPartner and initializePartner. Written from
 * content/species/*.json ("reincarnation_stats", "new_game_technique") after linking (tools/balance.py build);
 * unused rows have type -1.
 */
/* (ReincarnationStats: partner.h, which the DOOA overlay reads too) */

typedef struct {
	int16_t type;
	uint8_t technique;	/* learnMove id */
	uint8_t animation;	/* 0x2E + its index in the species' techniques: moves[0] */
} NewGameTechnique;

ReincarnationStats REINCARNATION_STATS[4] = {
	{ 1, 90, 110, 10, 11, 9, 10 },
	{ 15, 120, 100, 16, 8, 8, 6 },
	{ 29, 100, 140, 12, 6, 6, 8 },
	{ 43, 100, 120, 8, 8, 8, 8 },
};

NewGameTechnique NEW_GAME_TECHNIQUES[2] = {
	{ 3, 2, 0x2e },
	{ 0x11, 0x2b, 0x30 },
};

int8_t ITEM_TAKE_DISTANCE[NUM_ORIGINAL_DIGIMON] = { /* MOD: was NUM_PARTNER_DIGIMON */
	10, 10, 10, 10, 15, 10, 10, 15, 10, 15, 10, 15, 10, 10, 10, 10, 10,
	10, 15, 10, 10, 15, 10, 15, 10, 25, 10, 10, 10, 10, 10, 10, 10, 10,
	15, 15, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 15,
	10, 10, 10, 10, 10, 10, 10, 10, 15, 15, 10, 10, 10, 10, 10,
	/* MOD (master-tyrannomon): by type - 1; Tyrannomon's 15 at 1.5 times */
	[DIGIMON_MASTER_TYRANNOMON - 1] = 22,
	/* MOD (cherrymon): long branch arms */
	[DIGIMON_CHERRYMON - 1] = 20,
	/* MOD (partners-npc-a): long claws */
	[DIGIMON_KING_SUKAMON - 1] = 16,
	[DIGIMON_ICEMON - 1] = 10, /* MOD (partners-npc-a) */
	[DIGIMON_METEORMON - 1] = 15, /* MOD (partners-npc-a) */
	/* MOD (partners-npc-b): Monzaemon's 10, longer claw arm */
	[DIGIMON_WARUMONZAEMON - 1] = 12,
	[DIGIMON_MYOTISMON - 1] = 11, /* MOD (partners-npc-b): Devimon's 10 */
	[DIGIMON_SHOGUNGEKOMON - 1] = 20, /* MOD (partners-npc-b): a wide frog */
	/* MOD (machinedramon): long arms, MetalGreymon's 15 at its size */
	[DIGIMON_MACHINEDRAMON - 1] = 20,
	/* MOD (jijimon): short arms */
	[DIGIMON_JIJIMON - 1] = 10,
	/* MOD (analogman): a human's reach */
	[DIGIMON_ANALOGMAN - 1] = 10,
};

extern uint8_t ITEM_CLUT_DATA[];
extern uint8_t EVOLUTION_ITEM_TARGET[];
extern GsOT *ACTIVE_ORDERING_TABLE;

extern int8_t GAME_STATE;
extern int32_t FADE_PROTECTION;

extern uint8_t MAP_LAYER_ENABLED;
extern int32_t ACTIVE_FRAMEBUFFER;
extern GsOT GS_ORDERING_TABLE[2];
SVECTOR POOP_ROTATION = { 0, 0, 0, 0 };

int32_t IS_NATURAL_DEATH;
int16_t STATUS_UI_OFFSET_X;
int32_t HAS_BUTTERFLY;
uint8_t CURRENT_POOP_ID;
int8_t ITEM_SCOLD_FLAG;
int8_t STOP_DISTANCE_TIMER;
int8_t PARTNER_STATE;
int8_t PARTNER_SUB_STATE;
int32_t BUTTERFLY_ID;
int32_t NANIMON_TRIGGER;
uint8_t POOP_TO_EAT;
int32_t CONDITION_BUBBLE_TYPE;
int32_t CONDITION_BUBBLE_ID;
int32_t CONDITION_BUBBLE_TIMER;

static void *partner_impl_sbss_order[] = {
#if VERSION_IS(JP)
	&POOP_TO_EAT,
	&NANIMON_TRIGGER,
	&BUTTERFLY_ID,
	&PARTNER_SUB_STATE,
	&PARTNER_STATE,
	&STOP_DISTANCE_TIMER,
	&ITEM_SCOLD_FLAG,
	&CURRENT_POOP_ID,
	&HAS_BUTTERFLY,
	&CONDITION_BUBBLE_TIMER,
	&CONDITION_BUBBLE_ID,
	&CONDITION_BUBBLE_TYPE,
#else
	&CONDITION_BUBBLE_TIMER,
	&CONDITION_BUBBLE_ID,
	&CONDITION_BUBBLE_TYPE,
	&POOP_TO_EAT,
	&NANIMON_TRIGGER,
	&BUTTERFLY_ID,
	&PARTNER_SUB_STATE,
	&PARTNER_STATE,
	&STOP_DISTANCE_TIMER,
	&ITEM_SCOLD_FLAG,
	&CURRENT_POOP_ID,
	&HAS_BUTTERFLY,
#endif
	&STATUS_UI_OFFSET_X,
	&IS_NATURAL_DEATH,
};

uint8_t POOP_MODEL_BUFFER[2048];
GsDOBJ2 POOP_OBJECT;
GsCOORDINATE2 POOP_POSITION;
PartnerPara PARTNER_PARA;
Stats DEATH_STATS;
PoopPile WORLD_POOP[100];

static void *partner_impl_bss_order[] = {
	WORLD_POOP,
	&DEATH_STATS,
	&PARTNER_PARA,
	&POOP_POSITION,
	&POOP_OBJECT,
	POOP_MODEL_BUFFER,
};

extern uint16_t CURRENT_FRAME;
extern uint16_t LAST_HANDLED_FRAME;
extern int32_t IS_SCRIPT_PAUSED;

void writePStat(int32_t id, uint8_t value);
void storePartnerSpecies(uint8_t index);
void callScriptSection(int32_t a, int32_t b, int32_t c);
ModelComponent *thunkLoadMMD(int32_t digiType, int32_t modelType);
void learnMove(int32_t move);
void addTamerLevel(int32_t chance, int32_t amount);
void setSleepDisabled(int32_t disabled);
void getModelTile(VECTOR *pos, int16_t *outTileX, int16_t *outTileY);
void setTrigger(uint16_t flag);
void createCloudFX(SVECTOR *pos);
void updateTimeOfDay(void);
int32_t isInDaytimeTransition(void);
void renderRectPolyFT4(int32_t posX, int32_t posY, int32_t width,
		       int32_t height, int32_t texX, int32_t texY,
		       int32_t texturePage, int32_t clut, int32_t zIndex,
		       int32_t flag);

void initializeStatusObjects(void);
void initializePoop(void);
void initializePartner(int32_t type, int32_t posX, int32_t posY,
		       int32_t posZ, int32_t rotX, int32_t rotY,
		       int32_t rotZ);
void setSleepTimes(PartnerPara *para, int32_t type);
void setFoodTimer(int16_t type);
void renderPoop(int32_t instanceId);
void initializeEvolvedPartner(int32_t type, int32_t posX, int32_t posY,
			      int32_t posZ, int32_t rotX, int32_t rotY,
			      int32_t rotZ);
void resetPartnerPara(PartnerPara *para, int32_t type);
void initializeReincarnatedPartner(int32_t type, int32_t posX, int32_t posY,
				    int32_t posZ, int32_t rotX, int32_t rotY,
				    int32_t rotZ);
void setReincarnateStats(int16_t hp, int16_t mp, int16_t offense,
			 int16_t defense, int16_t speed, int16_t brains);
void setupPartnerOnWarp(int32_t posX, int32_t posY, int32_t posZ,
			int32_t rotation);
void tickConditionBoundaries(void);
void tickSleepMechanics(void);
void tickUnhappinessMechanics(void);
void handlePraiseScold(int32_t state);
void handleSleeping(void);
int32_t createPoopPile(int16_t tileX, int16_t tileY);
void sleepRegen(void);
void addWithLimit(int16_t *value, int16_t amount, int16_t limit);
void tickTirednessMechanics(void);
int32_t partnerWillRefuseItem(void);
void tickHungerMechanics(void);
void handleConditionBubble(void);
void partnerHandleFoodFeed(int32_t itemType);
void tickPoopingMechanic(void);
void tickPoopDetection(void);
void handleToilet(void);
void handlePoopWeightLoss(int32_t type);
void handleWildPoop(void);
void handleEatingPoop(void);
void tickSicknessMechanics(void);
void tickDeathCondition(void);
#if !VERSION_IS(US)
void skipHours(int16_t hours);
#else
void skipHours(int32_t hours);
#endif
void handlePostBattleTiredness(void);
void tickConditions(void);
void renderStatusBars(int32_t state);
void setImmortalHour(void);

static void *partner_impl_functions[] = {
	setImmortalHour,
	renderStatusBars,
	tickConditions,
	handlePostBattleTiredness,
	skipHours,
	tickDeathCondition,
	tickSicknessMechanics,
	handleEatingPoop,
	handleWildPoop,
	handlePoopWeightLoss,
	handleToilet,
	tickPoopDetection,
	tickPoopingMechanic,
	partnerHandleFoodFeed,
	handleConditionBubble,
	tickHungerMechanics,
	partnerWillRefuseItem,
	tickTirednessMechanics,
	sleepRegen,
	createPoopPile,
	handleSleeping,
	handlePraiseScold,
	tickUnhappinessMechanics,
	tickSleepMechanics,
	tickConditionBoundaries,
	setupPartnerOnWarp,
	setReincarnateStats,
	initializeReincarnatedPartner,
	resetPartnerPara,
	initializeEvolvedPartner,
	renderPoop,
	setFoodTimer,
	setSleepTimes,
	initializePartner,
	initializePoop,
	initializeStatusObjects,
};

void initializeStatusObjects(void)
{
	initializeConditionBubbles();

	initializeButterfly();
	HAS_BUTTERFLY = -1;

	initializePoop();

	CURRENT_POOP_ID = 0;
	EVOLUTION_TARGET = -1;
	HAS_USED_EVOITEM = 0;
	IS_NATURAL_DEATH = 0;
	ITEM_SCOLD_FLAG = 0;
	STATUS_UI_OFFSET_X = 75;
}

void initializePoop(void)
{
	char *buf;

	buf = (char *)POOP_MODEL_BUFFER;
	readFile(MAIN_D_801225AC, buf);

	GsMapModelingData((unsigned long *)&buf[4]);
	GsLinkObject4((unsigned long)&buf[0xc], &POOP_OBJECT, 0);

	GsInitCoordinate2(NULL, &POOP_POSITION);
	POOP_OBJECT.attribute = 0;
	POOP_OBJECT.coord2 = &POOP_POSITION;
}

GARBAGE(initializePartner, 14);

void initializePartner(int32_t type, int32_t posX, int32_t posY,
		       int32_t posZ, int32_t rotX, int32_t rotY,
		       int32_t rotZ)
{
	int32_t i;

	thunkLoadMMD(type, 3);
	ENTITY_TABLE[1] = (Entity *)&PARTNER_ENTITY;
	initializeDigimonObject(type, 1, partnerTick);
	setEntityPosition(1, posX, posY, posZ);
	setEntityRotation(1, rotX, rotY, rotZ);
	setupEntityMatrix(1);
	startAnimation(ENTITY_TABLE[1], 0);

	STOP_DISTANCE_TIMER = 0;
	PARTNER_ENTITY.digimonEntity.stats.current.vabId = 4;
	loadPartnerSounds(type);
	PARTNER_ENTITY.digimonEntity.entity.isOnMap = 1;
	PARTNER_ENTITY.digimonEntity.entity.isOnScreen = 1;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	PARTNER_PARA.condition = 0;
#endif
	setSleepTimes(&PARTNER_PARA, type);

	PARTNER_PARA.missedSleepHours = 0;
	PARTNER_PARA.poopLevel = RAISE_ROW(type).poopTimer;
	PARTNER_PARA.poopingTimer = -1;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	PARTNER_PARA.virusBar = 0;
	PARTNER_PARA.tiredness = 0;
#endif
	PARTNER_PARA.discipline = modNewGame(MOD_NG_BIRTH_DISCIPLINE);
	PARTNER_PARA.happiness = modNewGame(MOD_NG_BIRTH_HAPPINESS);
	PARTNER_PARA.unused3 = modNewGame(MOD_NG_BIRTH_HAPPINESS);
	setFoodTimer(type);

	PARTNER_PARA.energyLevel = RAISE_ROW(type).energyThreshold;
	PARTNER_PARA.remainingLifetime = modNewGame(MOD_NG_BIRTH_LIFETIME);
	PARTNER_PARA.weight = RAISE_ROW(type).defaultWeight;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	PARTNER_PARA.trainBoostFlag = PARTNER_PARA.trainBoostValue =
		PARTNER_PARA.trainBoostTimer = 0;
	PARTNER_PARA.careMistakes = PARTNER_PARA.battles =
		PARTNER_PARA.evoTimer = 0;
	PARTNER_PARA.timesBeingSick = PARTNER_PARA.areaEffectTimer =
		PARTNER_PARA.sicknessTimer = PARTNER_PARA.injuryTimer = 0;
#endif

	PARTNER_ENTITY.lives = modNewGame(MOD_NG_LIVES);
	PARTNER_ENTITY.digimonEntity.stats.base.moves[1] = 0xff;
	PARTNER_ENTITY.digimonEntity.stats.base.moves[2] = 0xff;
	for (i = 15; i >= 0; i--) {
		if (DIGI_PARA(type).moves[i] == 0xff)
			continue;
		if (DIGI_PARA(type).moves[i] < 0x3a)
			continue;
		if (DIGI_PARA(type).moves[i] >= 0x71)
			continue;

		PARTNER_ENTITY.digimonEntity.stats.base.moves[3] = i + 0x2e;
		break;
	}
	if (i < 0)
		PARTNER_ENTITY.digimonEntity.stats.base.moves[3] = 0xff;

	addObject(0xfa8, 0, NULL, renderPoop);
	HAS_IMMORTAL_HOUR = 0;
	IMMORTAL_HOUR = -1;
}

/* Starter techniques belong to new games. Continue has already restored the
 * learned bits; initializeLoadedMap restores the equipped slots afterward. */
void modInitializeStarterTechniques(int32_t type)
{
	int32_t i;
	int32_t slot;
	int32_t base;
	int32_t move;
	int32_t best;
	int32_t special;
	int32_t window;
	DigimonPara *para;

	/* The rebirth path equips the first list animation (0x2e); keep the two
	 * authored opening exceptions. */
	PARTNER_ENTITY.digimonEntity.stats.base.moves[0] = 0x2e;
	for (i = 0; i < 2; i++) {	/* MOD (species JSON): was Agumon and Gabumon by literal */
		if (NEW_GAME_TECHNIQUES[i].type == type) {
			PARTNER_ENTITY.digimonEntity.stats.base.moves[0] = NEW_GAME_TECHNIQUES[i].animation;
			learnMove(NEW_GAME_TECHNIQUES[i].technique);
			return;
		}
	}

	/* MOD (starter techniques): every other starter opens with what evolving
	 * into it teaches (EVL_applyEvolution): the weakest technique of its main
	 * element, by the same formula slots. Its first list entry can be of any
	 * strength (Gotsumon's is Megaton Punch). */
	para = &DIGI_PARA(type);
	special = para->special[0];
	base = special <= 6 ? MOD_EVO_FORMULA[MOD_EVF_LEARN_BASE0 + special] : -1;
	window = evoAmt(MOD_EVF_LEARN_WINDOW);
	best = 0xff;
	slot = 0;
	for (i = 0; base >= 0 && i < 16; i++) {
		move = para->moves[i];
		if ((uint32_t)(move - base) <= (uint32_t)window &&
		    (best == 0xff || (MOVE_DATA[best].power > MOVE_DATA[move].power && MOVE_DATA[move].power != 0))) {
			best = move;
			slot = i;
		}
	}

	/* No technique of its element: the first list entry. Fresh Bubble attacks
	 * are intrinsic, outside the learned-move bitset. */
	if (best == 0xff) {
		best = para->moves[0];
		if (best >= 0x3a) {
			return;
		}
	}
	PARTNER_ENTITY.digimonEntity.stats.base.moves[0] = slot + 0x2e;
	learnMove(best);
}

void renderPoop(int32_t instanceId)
{
	VECTOR pos;
	VECTOR scale;
	SVECTOR rotation = POOP_ROTATION;
	GsOT_TAG *ot;
	int32_t i;
	int16_t x;
	int16_t scaleValue;
	int16_t height;
	int16_t z;

	if (!MAP_LAYER_ENABLED)
		return;

	SetRotMatrix(&GsWSMATRIX);
	SetTransMatrix(&GsWSMATRIX);
	ot = ACTIVE_ORDERING_TABLE->org;
	for (i = 0; i < 100; i++) {
		if ((WORLD_POOP[i].size != 0) && (CURRENT_SCREEN == WORLD_POOP[i].map)) {
			x = (WORLD_POOP[i].x - 50) * 100 + 50;
			height = PARTNER_ENTITY.digimonEntity.entity.posData->location.vy;
			z = (50 - WORLD_POOP[i].y) * 100 - 50;
			scaleValue = WORLD_POOP[i].size * 4096 / 10;
			pos.vx = x;
			pos.vy = height;
			pos.vz = z;
			scale.vx = scaleValue;
			scale.vy = scaleValue;
			scale.vz = scaleValue;
			projectPosition(&POOP_POSITION, &pos, &rotation, &scale);
			drawObject(&POOP_OBJECT, GS_ORDERING_TABLE + ACTIVE_FRAMEBUFFER, 2);
		}
	}
}

void initializeEvolvedPartner(int32_t type, int32_t posX, int32_t posY,
			      int32_t posZ, int32_t rotX, int32_t rotY,
			      int32_t rotZ)
{
	removeObject((int16_t)type, 1);
	applyMMD(type, 3, &EVO_SEQUENCE_DATA.modelData);

	ENTITY_TABLE[1] = (Entity *)&PARTNER_ENTITY;

	initializeDigimonObject(type, 1, partnerTick);

	setEntityPosition(1, posX, posY, posZ);
	setEntityRotation(1, rotX, rotY, rotZ);
	setupEntityMatrix(1);

	startAnimation(ENTITY_TABLE[1], 0);

	STOP_DISTANCE_TIMER = 0;
	PARTNER_ENTITY.digimonEntity.stats.current.vabId = 4;
	PARTNER_PARA.condition = 0;

	setSleepTimes(&PARTNER_PARA, type);
	resetPartnerPara(&PARTNER_PARA, type);

	HAS_IMMORTAL_HOUR = 0;
	IMMORTAL_HOUR = -1;
}

void resetPartnerPara(PartnerPara *para, int32_t type)
{
	PARTNER_PARA.poopLevel = RAISE_ROW(type).poopTimer;
	PARTNER_PARA.unused2 = 0;
	PARTNER_PARA.unused1 = 0;
	PARTNER_PARA.poopingTimer = -1;
	PARTNER_PARA.tiredness = 0;
	PARTNER_PARA.subTiredness = 0;
	PARTNER_PARA.tirednessHungerTimer = 0;
	PARTNER_PARA.timesBeingSick = 0;
	PARTNER_PARA.areaEffectTimer = 0;
	PARTNER_PARA.sicknessTimer = 0;
	PARTNER_PARA.injuryTimer = 0;
	PARTNER_PARA.sicknessTries = 0;
	PARTNER_PARA.unused4 = 0;

	setFoodTimer(type);

	PARTNER_PARA.starvationTimer = -1;
	PARTNER_PARA.emptyStomachTimer = 0;
	PARTNER_PARA.weight = RAISE_ROW(type).defaultWeight;
	PARTNER_PARA.careMistakes = 0;
	PARTNER_PARA.battles = 0;
}

void setSleepTimes(PartnerPara *para, int32_t type)
{
	int16_t level;
	int16_t roll;
	int16_t awakeTimeBase;

	level = DIGI_PARA(type).level;

	if (level < 3) {
		if (level == 1) {
			awakeTimeBase = careAmt(MOD_CARE_SLEEP_AWAKE_FRESH);
		} else {
			awakeTimeBase = careAmt(MOD_CARE_SLEEP_AWAKE_TRAIN);
		}

		roll = randomLimit(careDie(MOD_CARE_SLEEP_AWAKE_JITTER));
		para->sleepyHour = (HOUR + (awakeTimeBase + roll)) % 24;
		para->sleepyMinute = 0;
		para->wakeupHour =
			(para->sleepyHour +
			 SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyHours);
		para->wakeupMinute = para->sleepyMinute;
		para->wakeupHour %= 24;
		para->hoursAwakeDefault = awakeTimeBase + roll;
		para->hoursAsleepDefault =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyHours;
	} else {
		para->sleepyHour =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyHour;
		para->sleepyMinute =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyMinute;
		para->wakeupHour =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].wakeupHour;
		para->wakeupMinute =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].wakeupMinute;
		para->hoursAwakeDefault =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].awakeHours;
		para->hoursAsleepDefault =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyHours;
	}

	PARTNER_PARA.timeAwakeToday = PARTNER_PARA.hoursAwakeDefault * 6;
	PARTNER_PARA.sicknessCounter = 0;
	PARTNER_PARA.tirednessSleepTimer = 0;
}

void setFoodTimer(int16_t type)
{
	int32_t i;
	uint8_t level;
	int32_t hungerEvery;

	level = DIGI_PARA(type).level;

	if (level == 1) {
		/* MOD (care rules as data): every hungerEvery hours aligned to the clock (a rule that is left out: once a day) */
		hungerEvery = careAmt(MOD_CARE_HUNGER_EVERY_FRESH) > 0 ? careAmt(MOD_CARE_HUNGER_EVERY_FRESH) : 24;
		PARTNER_PARA.nextHungerHour = HOUR / hungerEvery * hungerEvery + hungerEvery;
		if (PARTNER_PARA.nextHungerHour >= 24) {
			PARTNER_PARA.nextHungerHour -= 24;
		}
	}
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	else
#endif
	if (level == 2) {
		hungerEvery = careAmt(MOD_CARE_HUNGER_EVERY_TRAIN) > 0 ? careAmt(MOD_CARE_HUNGER_EVERY_TRAIN) : 24;
		PARTNER_PARA.nextHungerHour = HOUR / hungerEvery * hungerEvery + hungerEvery;
		if (PARTNER_PARA.nextHungerHour >= 24) {
			PARTNER_PARA.nextHungerHour -= 24;
		}
	} else {
#if VERSION_IS(JP)
		for (i = 0; i < 8; i++) {
			if (HOUR < RAISE_ROW(type).hungerTimes[i]) {
				PARTNER_PARA.nextHungerHour =
					RAISE_ROW(type).hungerTimes[i];
				break;
			}
			if ((RAISE_ROW(type).hungerTimes[i + 1] == -1) ||
			    (i == 7))
				PARTNER_PARA.nextHungerHour =
					RAISE_ROW(type).hungerTimes[0];
		}
#else
		for (i = 0; i < 8; i++) {
			if (HOUR <= PARTNER_PARA.nextHungerHour) {
				if (PARTNER_PARA.nextHungerHour <
				    RAISE_ROW(type).hungerTimes[i]) {
					PARTNER_PARA.nextHungerHour =
						RAISE_ROW(type).hungerTimes[i];
					goto calculateTimer;
				}
			} else if (HOUR < RAISE_ROW(type).hungerTimes[i]) {
				PARTNER_PARA.nextHungerHour =
					RAISE_ROW(type).hungerTimes[i];
				goto calculateTimer;
			}

			if (RAISE_ROW(type).hungerTimes[i + 1] == -1 ||
			    i == 7) {
				int32_t j;

				for (j = 0; j < 8; j++) {
					if (HOUR < RAISE_ROW(type).hungerTimes[j]) {
						PARTNER_PARA.nextHungerHour =
							RAISE_ROW(type)
								.hungerTimes[j];
						goto calculateTimer;
					}

					if (RAISE_ROW(type).hungerTimes[j + 1] == -1 ||
					    j == 7) {
						PARTNER_PARA.nextHungerHour =
							RAISE_ROW(type)
								.hungerTimes[0];
						goto calculateTimer;
					}
				}

				goto calculateTimer;
			}
		}
#endif
	}

#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
calculateTimer:
#endif
	if (HOUR <= PARTNER_PARA.nextHungerHour) {
		PARTNER_PARA.foodLevel =
			(PARTNER_PARA.nextHungerHour - HOUR) * 60;
		if (MINUTE != 0) {
			PARTNER_PARA.foodLevel -= MINUTE;
		}
	} else {
		PARTNER_PARA.foodLevel = (24 - HOUR) * 60;
		PARTNER_PARA.foodLevel += PARTNER_PARA.nextHungerHour * 60;
		if (MINUTE != 0) {
			PARTNER_PARA.foodLevel -= MINUTE;
		}
	}
}

void initializeReincarnatedPartner(int32_t type, int32_t posX, int32_t posY,
				    int32_t posZ, int32_t rotX, int32_t rotY,
				    int32_t rotZ)
{
	int32_t i;

	removeObject((int16_t)type, 1);
	applyMMD(type, 3,
		 (EvoModelData *)&DOOA_REINCARNATION_SEQ.modelData);

	ENTITY_TABLE[1] = (Entity *)&PARTNER_ENTITY;
	initializeDigimonObject(type, 1, partnerTick);
	setEntityPosition(1, posX, posY, posZ);
	setEntityRotation(1, rotX, rotY, rotZ);
	setupEntityMatrix(1);
	startAnimation(ENTITY_TABLE[1], 0);

	STOP_DISTANCE_TIMER = 0;
	PARTNER_ENTITY.digimonEntity.stats.current.vabId = 4;

	for (i = 0; i < 4; i++) {	/* MOD (species JSON): was four literal rows */
		if (REINCARNATION_STATS[i].type == type) {
			setReincarnateStats(REINCARNATION_STATS[i].hp, REINCARNATION_STATS[i].mp,
					    REINCARNATION_STATS[i].offense, REINCARNATION_STATS[i].defense,
					    REINCARNATION_STATS[i].speed, REINCARNATION_STATS[i].brains);
		}
	}

	PARTNER_PARA.condition = 0;
	setSleepTimes(&PARTNER_PARA, type);
	PARTNER_PARA.missedSleepHours = 0;
	PARTNER_PARA.tirednessSleepTimer = 0;
	resetPartnerPara(&PARTNER_PARA, type);

	PARTNER_PARA.discipline = modNewGame(MOD_NG_BIRTH_DISCIPLINE);
	PARTNER_PARA.happiness = modNewGame(MOD_NG_BIRTH_HAPPINESS);
	PARTNER_PARA.unused3 = modNewGame(MOD_NG_BIRTH_HAPPINESS);
	PARTNER_PARA.timesBeingSick = 0;
	PARTNER_PARA.remainingLifetime = modNewGame(MOD_NG_BIRTH_LIFETIME);
	PARTNER_PARA.age = 0;
	PARTNER_PARA.evoTimer = 0;
	EVOLUTION_TARGET = -1;
	PARTNER_PARA.careMistakes = 0;
	PARTNER_PARA.battles = 0;
	PARTNER_PARA.weight = RAISE_ROW(type).defaultWeight;

	TAMER_ENTITY.raisedCount++;
	if (TAMER_ENTITY.raisedCount >= 100)
		TAMER_ENTITY.raisedCount = 99;

	HAS_IMMORTAL_HOUR = 0;
	IMMORTAL_HOUR = -1;
}

void setReincarnateStats(int16_t hp, int16_t mp, int16_t offense,
			 int16_t defense, int16_t speed, int16_t brains)
{
	int16_t percentage;
	Stats *stats;
	int16_t inheritedHp;
	int16_t inheritedMp;
	int16_t inheritedOffense;
	int16_t inheritedDefense;
	int16_t inheritedSpeed;
	int16_t inheritedBrains;

	/* MOD (tamer level as data): the percent per tamer level is MOD_TAMER_LEVEL's (-1: nothing is inherited) */
	if (IS_NATURAL_DEATH == 1 && MOD_TAMER_LEVEL[MOD_TL_REBIRTH_PERCENT] >= 0) {
		percentage = TAMER_ENTITY.tamerLevel * MOD_TAMER_LEVEL[MOD_TL_REBIRTH_PERCENT];
	} else {
		percentage = 0;
	}

	inheritedHp = DEATH_STATS.base.hp * percentage / 100;
	inheritedMp = DEATH_STATS.base.mp * percentage / 100;
	inheritedOffense = DEATH_STATS.base.off * percentage / 100;
	inheritedDefense = DEATH_STATS.base.def * percentage / 100;
	inheritedSpeed = DEATH_STATS.base.speed * percentage / 100;
	inheritedBrains = DEATH_STATS.base.brain * percentage / 100;
	stats = &PARTNER_ENTITY.digimonEntity.stats;

	stats->current.currentHP = stats->base.hp = hp + inheritedHp;
	stats->current.currentMP = stats->base.mp = mp + inheritedMp;
	stats->base.off = offense + inheritedOffense;
	stats->base.def = defense + inheritedDefense;
	stats->base.speed = speed + inheritedSpeed;
	stats->base.brain = brains + inheritedBrains;

	IS_NATURAL_DEATH = 0;
}

// clang-format off
void setupPartnerOnWarp(posX, posY, posZ, rotation)
	int16_t posX;
	int16_t posY;
	int16_t posZ;
	int16_t rotation;
// clang-format on
{
	setEntityPosition(1, posX, posY, posZ);
	setEntityRotation(1, PARTNER_ENTITY.digimonEntity.entity.posData->rotation.vx,
			  rotation,
			  PARTNER_ENTITY.digimonEntity.entity.posData->rotation.vz);
	setupEntityMatrix(1);

	startAnimation(ENTITY_TABLE[1], 0);

	PARTNER_STATE = 1;
	PARTNER_SUB_STATE = 0;
	PARTNER_PARA.areaEffectTimer = 0;
}

/*
 * MOD (care rules as data): MOD_CARE's slots (-1 = the rule does not exist), written by tools/balance.py build from
 * content/tables/care-rules.json. careEvery: a period that has come round on a new frame (a period left out never does);
 * careEveryRaw: the same without the new-frame test, for the places the original had none; careHas: the threshold exists;
 * careAmt: an amount (left out = 0).
 */
int16_t MOD_CARE[MOD_CARE_SIZE] = { [0 ... MOD_CARE_SIZE - 1] = -1 };
/* MOD (care rules as data): per-species overrides of MOD_CARE, written by tools/balance.py from the species files' "care" (-1 rows = none) */
int16_t MOD_CARE_SPECIES[MOD_CARE_OVERRIDE_MAX][3] = { { -1, -1, -1 } };

/*
 * careGet: a rule's value for the partner's species: its own override (MOD_CARE_SPECIES, {species, slot, value} rows ended by
 * species -1; a value of -1 switches the rule off for it) or the global MOD_CARE.
 */
int32_t careGet(int32_t slot)
{
	int32_t type = PARTNER_ENTITY.digimonEntity.entity.type;
	int32_t i;

	for (i = 0; i < MOD_CARE_OVERRIDE_MAX && MOD_CARE_SPECIES[i][0] >= 0; i++) {
		if (MOD_CARE_SPECIES[i][0] == type && MOD_CARE_SPECIES[i][1] == slot)
			return MOD_CARE_SPECIES[i][2];
	}
	return MOD_CARE[slot];
}

/* a period (frames of game time: 20 = a game minute) came round this tick; clockCrossed does not need the tick to land on it */
static int32_t careEveryRaw(int32_t slot)
{
	return clockCrossed(careGet(slot));
}

static int32_t careEvery(int32_t slot)
{
	return careEveryRaw(slot) && CURRENT_FRAME != LAST_HANDLED_FRAME;
}

int32_t careHas(int32_t slot)
{
	return careGet(slot) >= 0;
}

int32_t careAmt(int32_t slot)
{
	int32_t v = careGet(slot);

	return v > 0 ? v : 0;
}

/* a percent factor: a rule that is left out changes nothing (100) */
int32_t carePercent(int32_t slot)
{
	int32_t v = careGet(slot);

	return v >= 0 ? v : 100;
}

/* a die: rolling 0 or less is a roll of 1 (always 0) */
int32_t careDie(int32_t slot)
{
	int32_t v = careGet(slot);

	return v > 0 ? v : 1;
}

/* the clamps of the partner's states (limit.* of care-rules.json): a bound that is left out is not applied */
void careClamp(int16_t *value, int32_t minSlot, int32_t maxSlot)
{
	if (careGet(maxSlot) != -1 && *value > careGet(maxSlot)) {
		*value = careGet(maxSlot);
	}
	if (careGet(minSlot) != -1 && *value < careGet(minSlot)) {
		*value = careGet(minSlot);
	}
}

void tickConditionBoundaries(void)
{
	careClamp(&PARTNER_PARA.happiness, MOD_CARE_LIM_HAPPY_MIN, MOD_CARE_LIM_HAPPY_MAX); /* MOD (care rules as data) */
	careClamp(&PARTNER_PARA.discipline, MOD_CARE_LIM_DISC_MIN, MOD_CARE_LIM_DISC_MAX);
	careClamp(&PARTNER_PARA.tiredness, MOD_CARE_LIM_TIRED_MIN, MOD_CARE_LIM_TIRED_MAX);

	if (PARTNER_PARA.energyLevel >
	    RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyCap) {
		PARTNER_PARA.energyLevel =
			RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyCap;
	}

	if (PARTNER_PARA.energyLevel < 0) {
		PARTNER_PARA.energyLevel = 0;
	}

	careClamp(&PARTNER_PARA.weight, MOD_CARE_LIM_WEIGHT_MIN, MOD_CARE_LIM_WEIGHT_MAX);
}

void tickSleepMechanics(void)
{
	int32_t wasSleepy;
	int32_t type;
	int16_t level;
	int8_t hoursLate;
	int8_t minutePart;

	wasSleepy = PARTNER_PARA.condition & 1;

	if (careEvery(MOD_CARE_SLEEP_TIRED_PERIOD)) {
		PARTNER_PARA.tirednessSleepTimer +=
			(int16_t)(PARTNER_PARA.tiredness * careAmt(MOD_CARE_SLEEP_TIRED_PERCENT) / 100);
	}

	if ((wasSleepy == 0) && careHas(MOD_CARE_SLEEP_SLEEPY_AT) &&
	    (PARTNER_PARA.tirednessSleepTimer >= careGet(MOD_CARE_SLEEP_SLEEPY_AT)) &&
	    careEvery(MOD_CARE_SLEEP_SLEEPY_PERIOD)) {
		PARTNER_PARA.timeAwakeToday--;
		PARTNER_PARA.tirednessSleepTimer = 0;

		hoursLate = (PARTNER_PARA.hoursAwakeDefault * 6 -
			     PARTNER_PARA.timeAwakeToday) / 6;
		if (((PARTNER_PARA.hoursAwakeDefault * 6 -
		      PARTNER_PARA.timeAwakeToday) % 6) != 0)
			hoursLate++;

		minutePart = PARTNER_PARA.timeAwakeToday % 6;
		PARTNER_PARA.sleepyHour =
			PARTNER_PARA.wakeupHour -
			PARTNER_PARA.hoursAsleepDefault - hoursLate;
		if (PARTNER_PARA.sleepyHour < 0)
			PARTNER_PARA.sleepyHour += 24;
		PARTNER_PARA.sleepyMinute = minutePart * 10;
	}

	type = PARTNER_ENTITY.digimonEntity.entity.type;
	level = DIGI_PARA(type).level;
	if (PARTNER_PARA.condition & 1) {
		if (level == 1) {
			if (careEvery(MOD_CARE_SLEEP_FRESH_PERIOD)) {
				PARTNER_PARA.happiness -= careAmt(MOD_CARE_SLEEP_FRESH_HAPPY);
				PARTNER_PARA.discipline -= careAmt(MOD_CARE_SLEEP_FRESH_DISC);
			}
		} else if (level == 2) {
			if (careEvery(MOD_CARE_SLEEP_TRAIN_PERIOD)) {
				PARTNER_PARA.happiness -= careAmt(MOD_CARE_SLEEP_TRAIN_HAPPY);
				PARTNER_PARA.discipline -= careAmt(MOD_CARE_SLEEP_TRAIN_DISC);
			}
		} else if (careEvery(MOD_CARE_SLEEP_LATER_PERIOD)) {
			PARTNER_PARA.happiness -= careAmt(MOD_CARE_SLEEP_LATER_HAPPY);
			PARTNER_PARA.discipline -= careAmt(MOD_CARE_SLEEP_LATER_DISC);
		}

		if (careEvery(MOD_CARE_SLEEP_MISSED_PERIOD) &&
		    (HOUR != PARTNER_PARA.sleepyHour)) {
			PARTNER_PARA.sicknessCounter++;
			PARTNER_PARA.missedSleepHours++;
		}
	}

	if ((PARTNER_PARA.condition & 1) &&
	    (((PARTNER_PARA.sleepyHour < PARTNER_PARA.wakeupHour) &&
	      (PARTNER_PARA.sleepyHour < HOUR) &&
	      (PARTNER_PARA.wakeupHour <= HOUR)) ||
	     ((PARTNER_PARA.wakeupHour < PARTNER_PARA.sleepyHour) &&
	      (HOUR < PARTNER_PARA.sleepyHour) &&
	      (PARTNER_PARA.wakeupHour <= HOUR)))) {
		if (level < 3) {
			PARTNER_PARA.sleepyHour =
				PARTNER_PARA.wakeupHour +
				PARTNER_PARA.hoursAwakeDefault;
			if (PARTNER_PARA.sleepyHour >= 24)
				PARTNER_PARA.sleepyHour -= 24;
			PARTNER_PARA.sleepyMinute = 0;

			PARTNER_PARA.wakeupHour =
				PARTNER_PARA.sleepyHour +
				PARTNER_PARA.hoursAsleepDefault;
			if (PARTNER_PARA.wakeupHour >= 24)
				PARTNER_PARA.wakeupHour -= 24;
			PARTNER_PARA.wakeupMinute = 0;
		} else {
			PARTNER_PARA.sleepyHour =
				SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyHour;
			PARTNER_PARA.sleepyMinute =
				SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyMinute;
			PARTNER_PARA.wakeupHour =
				SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].wakeupHour;
			PARTNER_PARA.wakeupMinute =
				SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].wakeupMinute;
		}

		PARTNER_PARA.timeAwakeToday =
			PARTNER_PARA.hoursAwakeDefault * 6;
		PARTNER_PARA.tirednessSleepTimer = 0;
		PARTNER_PARA.condition &= ~1;
		PARTNER_PARA.careMistakes += careAmt(MOD_CARE_SLEEP_LATE_MISTAKES);
		updateConditionAnimation();
		modTamerLevelEvent(MOD_TL_SLEEPS_LATE_CHANCE, -1); /* MOD (tamer level as data) */
	}

	if (wasSleepy == 0) {
		if (PARTNER_PARA.wakeupHour < PARTNER_PARA.sleepyHour) {
			if ((HOUR >= PARTNER_PARA.sleepyHour) ||
			    (HOUR < PARTNER_PARA.wakeupHour)) {
				PARTNER_PARA.condition |= 1;
			}
		} else if (PARTNER_PARA.wakeupHour >
			   PARTNER_PARA.sleepyHour) {
			if ((HOUR >= PARTNER_PARA.sleepyHour) &&
			    (HOUR < PARTNER_PARA.wakeupHour)) {
				PARTNER_PARA.condition |= 1;
			}
		}
	}

	if (PARTNER_PARA.condition & 1)
		setSleepDisabled(0);
	else
		setSleepDisabled(1);
}

void tickUnhappinessMechanics(void)
{
	int16_t happiness;
	int16_t randomValue;
	Stats *stats;

	if ((PARTNER_PARA.condition == 0) &&
	    (PARTNER_PARA.happiness < 0) &&
	    careEvery(MOD_CARE_SULK_PERIOD)) {
		happiness = PARTNER_PARA.happiness;
		if (PARTNER_PARA.happiness < 0)
			happiness = -PARTNER_PARA.happiness;

		randomValue = randomLimit(100);
		if (randomValue < happiness - PARTNER_PARA.discipline)
			PARTNER_PARA.condition |= 0x10;
	}

	if (PARTNER_PARA.condition & 0x10) {
		if (careEvery(MOD_CARE_SULK_RECOVER_PERIOD)) {
			PARTNER_PARA.happiness += careAmt(MOD_CARE_SULK_RECOVER_GAIN);
		}

		if (careEvery(MOD_CARE_SULK_STAT_PERIOD)) {
			stats = &PARTNER_ENTITY.digimonEntity.stats;
			stats->base.hp -=
				(int16_t)(stats->base.hp * careAmt(MOD_CARE_SULK_STAT_PERCENT) / 100);
			stats->base.mp -=
				(int16_t)(stats->base.mp * careAmt(MOD_CARE_SULK_STAT_PERCENT) / 100);
			stats->base.off -=
				(int16_t)(stats->base.off * careAmt(MOD_CARE_SULK_STAT_PERCENT) / 100);
			stats->base.def -=
				(int16_t)(stats->base.def * careAmt(MOD_CARE_SULK_STAT_PERCENT) / 100);
			stats->base.speed -=
				(int16_t)(stats->base.speed * careAmt(MOD_CARE_SULK_STAT_PERCENT) / 100);
			stats->base.brain -=
				(int16_t)(stats->base.brain * careAmt(MOD_CARE_SULK_STAT_PERCENT) / 100);

			if (stats->base.hp <= 0)
				stats->base.hp = careAmt(MOD_CARE_SULK_STAT_FLOOR);
			if (stats->base.mp <= 0)
				stats->base.mp = careAmt(MOD_CARE_SULK_STAT_FLOOR);
			if (stats->base.off <= 0)
				stats->base.off = careAmt(MOD_CARE_SULK_STAT_FLOOR);
			if (stats->base.def <= 0)
				stats->base.def = careAmt(MOD_CARE_SULK_STAT_FLOOR);
			if (stats->base.speed <= 0)
				stats->base.speed = careAmt(MOD_CARE_SULK_STAT_FLOOR);
			if (stats->base.brain <= 0)
				stats->base.brain = careAmt(MOD_CARE_SULK_STAT_FLOOR);

			if (stats->current.currentHP >= stats->base.hp)
				stats->current.currentHP = stats->base.hp;
			if (stats->current.currentMP >= stats->base.mp)
				stats->current.currentMP = stats->base.mp;
		}
	}

	if ((PARTNER_PARA.condition & 0x10) &&
	    (PARTNER_PARA.happiness >= 0)) {
		PARTNER_PARA.condition &= ~0x10;
		PARTNER_ENTITY.digimonEntity.entity.anim.loopCount = 1;
		unsetButterfly(BUTTERFLY_ID);
		HAS_BUTTERFLY = -1;
	}
}

// clang-format off
void handlePraiseScold(state)
	int16_t state;
// clang-format on
{
	int16_t oldDiscipline;
	int16_t oldHappiness;
	int16_t disciplineDelta;
	int16_t happinessDelta;

	oldDiscipline = PARTNER_PARA.discipline;
	oldHappiness = PARTNER_PARA.happiness;

	if (state == 4) {
		if (ITEM_SCOLD_FLAG == 1) {
			disciplineDelta = careAmt(MOD_CARE_SCOLD_VALID_DISC);
			ITEM_SCOLD_FLAG = 2;
		} else {
			disciplineDelta = careAmt(MOD_CARE_SCOLD_UNJUST_DISC);
			happinessDelta = -careAmt(MOD_CARE_SCOLD_UNJUST_HAPPY);
		}

		PARTNER_PARA.refusedFavFood = 0;
		PARTNER_PARA.condition &= ~0x10;

		if (HAS_BUTTERFLY == 0) {
			unsetButterfly(BUTTERFLY_ID);
			HAS_BUTTERFLY = -1;
		}
	} else {
		disciplineDelta = -careAmt(MOD_CARE_PRAISE_DISC);
		happinessDelta = (careGet(MOD_CARE_PRAISE_HAPPY_DIV) > 0 ? PARTNER_PARA.discipline / careGet(MOD_CARE_PRAISE_HAPPY_DIV) : 0) +
				 careAmt(MOD_CARE_PRAISE_HAPPY_BASE);
	}

	PARTNER_PARA.discipline += disciplineDelta;
	PARTNER_PARA.happiness += happinessDelta;

	if (state == 4) {
		NANIMON_TRIGGER = 0;
		/* MOD (evolution rules as data): the level and the discipline and happiness before the scolding that arm the flag */
		if (MOD_EVO_SPECIAL[MOD_SPECIAL_SCOLD_LEVEL] >= 0 &&
		    (DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type).level == MOD_EVO_SPECIAL[MOD_SPECIAL_SCOLD_LEVEL]) &&
		    (oldDiscipline == MOD_EVO_SPECIAL[MOD_SPECIAL_SCOLD_DISCIPLINE]) &&
		    (oldHappiness == MOD_EVO_SPECIAL[MOD_SPECIAL_SCOLD_HAPPINESS])) {
			NANIMON_TRIGGER = 1;
		}
	}
}

void handleSleeping(void)
{
	int16_t tileX;
	int16_t tileY;
	int16_t hoursSlept;
	int32_t type;

	if (HOUR < PARTNER_PARA.wakeupHour)
		hoursSlept = PARTNER_PARA.wakeupHour - HOUR;
	else
		hoursSlept = PARTNER_PARA.wakeupHour + (24 - HOUR);

	type = PARTNER_ENTITY.digimonEntity.entity.type;
	advanceToTime(PARTNER_PARA.wakeupHour, PARTNER_PARA.wakeupMinute);
	PARTNER_PARA.evoTimer += PARTNER_PARA.hoursAsleepDefault;
	PARTNER_PARA.remainingLifetime -= hoursSlept;
	if (PARTNER_PARA.remainingLifetime < 0)
		PARTNER_PARA.remainingLifetime = 0;

	if (DIGI_PARA(type).level < 3) {
		PARTNER_PARA.sleepyHour =
			PARTNER_PARA.wakeupHour +
			PARTNER_PARA.hoursAwakeDefault;
		if (PARTNER_PARA.sleepyHour >= 24)
			PARTNER_PARA.sleepyHour -= 24;
		PARTNER_PARA.sleepyMinute = 0;

		PARTNER_PARA.wakeupHour =
			PARTNER_PARA.sleepyHour +
			PARTNER_PARA.hoursAsleepDefault;
		if (PARTNER_PARA.wakeupHour >= 24)
			PARTNER_PARA.wakeupHour -= 24;
		PARTNER_PARA.wakeupMinute = 0;
	} else {
		PARTNER_PARA.sleepyHour =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyHour;
		PARTNER_PARA.sleepyMinute =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].sleepyMinute;
		PARTNER_PARA.wakeupHour =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].wakeupHour;
		PARTNER_PARA.wakeupMinute =
			SLEEP_PATTERN[RAISE_ROW(type).sleepCycle].wakeupMinute;
	}

	setFoodTimer((int16_t)type);
	PARTNER_PARA.starvationTimer = 0;
	PARTNER_PARA.condition &= ~4;
	PARTNER_PARA.timeAwakeToday = PARTNER_PARA.hoursAwakeDefault * 6;
	PARTNER_PARA.tirednessSleepTimer = 0;
	PARTNER_PARA.sicknessCounter = 0;
	PARTNER_PARA.condition &= ~1;
	setSleepDisabled(1);
	HAS_IMMORTAL_HOUR = 1;
	IMMORTAL_HOUR = (int8_t)HOUR;

	if (PARTNER_PARA.condition & 8) {
		getModelTile(&PARTNER_ENTITY.digimonEntity.entity.posData->location,
			     &tileX, &tileY);
		createPoopPile(tileX, tileY);
		PARTNER_PARA.condition &= ~8;
		PARTNER_PARA.poopLevel =
			RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type)
				.poopTimer;
		PARTNER_PARA.poopingTimer = -1;
	}

	if (PARTNER_PARA.condition & 4)
		PARTNER_PARA.weight -= careAmt(MOD_CARE_SLEEP_HUNGRY_WEIGHT);

	if (PARTNER_PARA.condition & 0x40) {
		PARTNER_PARA.sicknessTimer += hoursSlept;
		PARTNER_PARA.happiness -= careAmt(MOD_CARE_SLEEP_SICK_HAPPY);
		PARTNER_PARA.discipline -= careAmt(MOD_CARE_SLEEP_SICK_DISC);
		PARTNER_PARA.tiredness += careAmt(MOD_CARE_SLEEP_SICK_TIRED);
	}

	if (PARTNER_PARA.condition & 0x20) {
		PARTNER_PARA.injuryTimer += hoursSlept;
		PARTNER_PARA.happiness -= careAmt(MOD_CARE_SLEEP_INJ_HAPPY);
		PARTNER_PARA.discipline -= careAmt(MOD_CARE_SLEEP_INJ_DISC);
		PARTNER_PARA.tiredness += careAmt(MOD_CARE_SLEEP_INJ_TIRED);
	}
}

int32_t createPoopPile(int16_t tileX, int16_t tileY)
{
	int16_t rotation;
	int16_t i;
	int16_t count;
	SVECTOR pos;
	int32_t direction;

	rotation = PARTNER_ENTITY.digimonEntity.entity.posData->rotation.vy;
	if (((0 <= rotation) && (rotation <= 0x300)) || (rotation >= 0xD00))
		tileY--;
	if ((rotation >= 0x100) && (rotation < 0x700))
		tileX++;
	if ((rotation >= 0x500) && (rotation <= 0xB00))
		tileY++;
	if ((rotation >= 0x900) && (rotation <= 0xF00))
		tileX--;

	count = 0;
	for (i = 0; i < 100; i++) {
		if ((tileX == WORLD_POOP[i].x) &&
		    (tileY == WORLD_POOP[i].y) &&
		    (CURRENT_SCREEN == WORLD_POOP[i].map)) {
			direction = randomLimit(4);
			if (direction == 0)
				tileX--;
			else if (direction == 1)
				tileX++;
			else if (direction == 2)
				tileY--;
			else
				tileY++;
			i = -1;
			count = 0;
		} else {
			if (count >= 16)
				break;
			if (WORLD_POOP[i].size != 0) {
				if (CURRENT_SCREEN == WORLD_POOP[i].map)
					count++;
			} else {
				WORLD_POOP[i].map = CURRENT_SCREEN;
				WORLD_POOP[i].x = tileX;
				WORLD_POOP[i].y = tileY;
				WORLD_POOP[i].size =
					RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopSize;
				break;
			}
		}
	}

	if (i >= 100) {
		WORLD_POOP[CURRENT_POOP_ID].map = CURRENT_SCREEN;
		WORLD_POOP[CURRENT_POOP_ID].x = tileX;
		WORLD_POOP[CURRENT_POOP_ID].y = tileY;
		WORLD_POOP[CURRENT_POOP_ID].size =
			RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopSize;
		CURRENT_POOP_ID++;
	}
	if (count >= 16) {
		for (i = CURRENT_POOP_ID; i < 100; i++) {
			if (CURRENT_SCREEN == WORLD_POOP[i].map) {
				WORLD_POOP[i].map = CURRENT_SCREEN;
				WORLD_POOP[i].x = tileX;
				WORLD_POOP[i].y = tileY;
				WORLD_POOP[i].size =
					RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopSize;
				CURRENT_POOP_ID++;
				break;
			}
			if (i == 99) {
				i = -1;
				CURRENT_POOP_ID = 0;
			}
		}
	}

	/* MOD (partners-npc-a): the King of Sukamon catches no virus either */
	if (PARTNER_ENTITY.digimonEntity.entity.type != 0x27 &&
	    PARTNER_ENTITY.digimonEntity.entity.type != DIGIMON_KING_SUKAMON)
		PARTNER_PARA.virusBar++;
	if (CURRENT_POOP_ID >= 100)
		CURRENT_POOP_ID = 0;

	pos.vx = (tileX - 50) * 100 + 50;
	pos.vy = PARTNER_ENTITY.digimonEntity.entity.posData->location.vy;
	pos.vz = (50 - tileY) * 100 - 50;
	createCloudFX(&pos);
	return i;
}

void sleepRegen(void)
{
	int32_t healed;
	int16_t hoursSlept;
	int16_t sleepFactor;
	int16_t healRoll;
	int16_t maxHP;
	int16_t maxMP;
	int16_t hoursAsleepDefault;

	hoursAsleepDefault = PARTNER_PARA.hoursAsleepDefault;
	if (HOUR < PARTNER_PARA.wakeupHour)
		hoursSlept = PARTNER_PARA.wakeupHour - HOUR;
	else
		hoursSlept = PARTNER_PARA.wakeupHour + (24 - HOUR);

	if ((MINUTE > 0) && (HOUR != PARTNER_PARA.sleepyHour))
		hoursSlept++;

	sleepFactor =
		(int16_t)(hoursSlept * 100 / hoursAsleepDefault);
	if (getItemCount(0x22) != 0)
		sleepFactor = (int16_t)(sleepFactor * carePercent(MOD_CARE_REGEN_BED) / 100);

	if (MAP_TILE_DATA.partnerAreaResponse == 1)
		sleepFactor = (int16_t)(sleepFactor * carePercent(MOD_CARE_REGEN_GOOD) / 100);
	else if (MAP_TILE_DATA.partnerAreaResponse == 2)
		sleepFactor = (int16_t)(sleepFactor * carePercent(MOD_CARE_REGEN_BAD) / 100);

	maxHP = PARTNER_ENTITY.digimonEntity.stats.base.hp;
	maxMP = PARTNER_ENTITY.digimonEntity.stats.base.mp;
	healRoll = randomLimit(careDie(MOD_CARE_REGEN_HPMP_ROLL));
	/* MOD (caps): a bar of up to 30000 wrapped the 16-bit sum and left the partner with negative HP */
	healed = sleepFactor * (maxHP * (healRoll + careAmt(MOD_CARE_REGEN_HPMP_BASE)) / 100) / 100;
	addWithLimit(&PARTNER_ENTITY.digimonEntity.stats.current.currentHP, healed > maxHP ? maxHP : healed, maxHP);
	healed = sleepFactor * (maxMP * (healRoll + careAmt(MOD_CARE_REGEN_HPMP_BASE)) / 100) / 100;
	addWithLimit(&PARTNER_ENTITY.digimonEntity.stats.current.currentMP, healed > maxMP ? maxMP : healed, maxMP);

	healRoll = randomLimit(careDie(MOD_CARE_REGEN_TIRED_ROLL));
	PARTNER_PARA.tiredness -=
		(int16_t)(sleepFactor *
			  (PARTNER_PARA.tiredness * (healRoll + careAmt(MOD_CARE_REGEN_TIRED_BASE)) / 100) /
			  100);

	if (PARTNER_PARA.tiredness < 0)
		PARTNER_PARA.tiredness = 0;

	PARTNER_PARA.weight -=
		(int16_t)(careGet(MOD_CARE_REGEN_WEIGHT_DIV) > 0 ? RAISE_ROW(ENTITY_TABLE[1]->type).defaultWeight / careGet(MOD_CARE_REGEN_WEIGHT_DIV) : 0);
	if (PARTNER_PARA.weight <= 0)
		PARTNER_PARA.weight = 1;
}

void tickTirednessMechanics(void)
{
	uint8_t animId;

	animId = PARTNER_ENTITY.digimonEntity.entity.anim.animId;
	if (careGet(MOD_CARE_AREA_PERIOD) > 0 && ((PARTNER_PARA.areaEffectTimer % careGet(MOD_CARE_AREA_PERIOD)) < clockAdvance()) &&
	    (PARTNER_PARA.areaEffectTimer > 0)) {
		if (MAP_TILE_DATA.partnerAreaResponse == 1) {
			PARTNER_PARA.happiness += careAmt(MOD_CARE_AREA_GOOD_HAPPY);
			PARTNER_PARA.tiredness -= careAmt(MOD_CARE_AREA_GOOD_TIRED);
		}
		if (MAP_TILE_DATA.partnerAreaResponse == 2) {
			PARTNER_PARA.happiness -= careAmt(MOD_CARE_AREA_BAD_HAPPY);
			PARTNER_PARA.tiredness += careAmt(MOD_CARE_AREA_BAD_TIRED);
		}
	}
	if (careHas(MOD_CARE_TIRED_SUB) && PARTNER_PARA.subTiredness >= careGet(MOD_CARE_TIRED_SUB)) {
		PARTNER_PARA.tiredness += careAmt(MOD_CARE_TIRED_SUB_GAIN);
		if (careHas(MOD_CARE_LIM_TIRED_MAX) && PARTNER_PARA.tiredness >= careGet(MOD_CARE_LIM_TIRED_MAX)) {
			PARTNER_PARA.tiredness = careGet(MOD_CARE_LIM_TIRED_MAX);
		}
		PARTNER_PARA.subTiredness = 0;
	}
	if (careHas(MOD_CARE_TIRED_HUNGER_AT) && PARTNER_PARA.tiredness >= careGet(MOD_CARE_TIRED_HUNGER_AT)) {
		PARTNER_PARA.tirednessHungerTimer += clockAdvance();
	} else {
		PARTNER_PARA.tirednessHungerTimer = 0;
	}
	if (careHas(MOD_CARE_TIRED_EXHAUSTED_AT) && PARTNER_PARA.tiredness >= careGet(MOD_CARE_TIRED_EXHAUSTED_AT)) {
		PARTNER_PARA.condition |= 2;
		if (careEvery(MOD_CARE_TIRED_EXHAUSTED_PERIOD)) {
			PARTNER_PARA.happiness -= careAmt(MOD_CARE_TIRED_EXHAUSTED_HAPPY);
		}
	} else {
		PARTNER_PARA.condition &= ~2;
	}
}

/* Read the owned target table on every build, including imported targets. */
#define ITEM_EVOLUTION_TARGET(item) EVOLUTION_ITEM_TARGET[(item) - 0x47]

int32_t partnerWillRefuseItem(void)
{
	int32_t type;
	uint32_t condition;
	int16_t chance;
	int32_t item;
	int16_t disciplineChance;
	uint32_t isHungry;

	PARTNER_PARA.refusedFavFood = 0;
	type = PARTNER_ENTITY.digimonEntity.entity.type;
	condition = PARTNER_PARA.condition;
	chance = randomLimit(careDie(MOD_CARE_REFUSE_FAV_DIE));
	item = TAMER_ITEM.worldItem.type;

	if ((item >= 0x73) && (item != 0x79) && (item != 0x7A) &&
	    (item < 0x7C))
		return 1;
	if ((item >= 0x21) && (item < 0x26))
		return 1;

	if (((item >= 0x47) && (item < 0x73)) ||
	    ((item >= 0x7C) && (item < 0x80))) {
		if (item >= 0x7C) {
			/* MOD (evolution rules as data): the level the partner must have is MOD_EVO_ITEMS'; an item without a row does nothing */
			ModEvoItem *modItem = getModEvoItem(item);

			/* MOD (canon levels): an item whose target is a Mega asks for an Ultimate (Metalbanana -> MetalEtemon) */
			if (modItem == NULL ||
			    DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type).level !=
			    (DIGI_PARA(modItem->target).level == LEVEL_MEGA ? LEVEL_ULTIMATE : modItem->level))
				return 1;
		} else if (!modSpeciesAllowed(ITEM_EVOLUTION_TARGET(item)) ||
			   DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type).level + 1 !=
			   modNativeItemTargetLevel(ITEM_EVOLUTION_TARGET(item))) {
			/* Match handleEvoItems: only the predecessor stage may consume an item. */
			return 1;
		}
	}

	if (ITEM_SCOLD_FLAG == 2) {
		ITEM_SCOLD_FLAG = 0;
		return 0;
	}
	if ((0 <= item) && (item < 0x26)) {
		disciplineChance = randomLimit(100);
		if (careHas(MOD_CARE_REFUSE_BASE) &&
		    disciplineChance < careGet(MOD_CARE_REFUSE_BASE) -
		    (PARTNER_PARA.discipline + (randomLimit(careDie(MOD_CARE_REFUSE_JITTER)) + careAmt(MOD_CARE_REFUSE_OFFSET)))) {
			ITEM_SCOLD_FLAG = 1;
			return 1;
		}
	}
	if (((isHungry = condition & 4) == 0) &&
	    (item == RAISE_ROW(type).favoriteFood) && careHas(MOD_CARE_REFUSE_FAV_DIE) && careHas(MOD_CARE_REFUSE_FAV_BELOW) &&
	    (chance < careGet(MOD_CARE_REFUSE_FAV_BELOW)))
		PARTNER_PARA.refusedFavFood = 2;

	if (PARTNER_PARA.refusedFavFood != 0) {
		if ((PARTNER_PARA.condition & 4) != 0)
			ITEM_SCOLD_FLAG = 1;
		return 1;
	}
	return 0;
}

void tickHungerMechanics(void)
{
	int32_t type = PARTNER_ENTITY.digimonEntity.entity.type;
	uint32_t isHungry;

	if ((PARTNER_PARA.condition & 4) != 0) {
		if (careEvery(MOD_CARE_HUNGER_STARVE_PERIOD))
			PARTNER_PARA.starvationTimer--;
	} else if (careEvery(MOD_CARE_HUNGER_FOOD_PERIOD)) {
		PARTNER_PARA.foodLevel--;
	}

	if (careEvery(MOD_CARE_HUNGER_ENERGY_PERIOD)) {
		PARTNER_PARA.energyLevel -= RAISE_ROW(type).energyUsage;
		if (PARTNER_PARA.energyLevel < 0)
			PARTNER_PARA.energyLevel = 0;
	}

	isHungry = PARTNER_PARA.condition & 4;
	if (!isHungry && (PARTNER_PARA.foodLevel <= 0)) {
		PARTNER_PARA.starvationTimer = careAmt(MOD_CARE_HUNGER_STARVE_TIMER);
		PARTNER_PARA.condition |= 4;
		handleConditionBubble();
	}
	if (((PARTNER_PARA.condition & 4) != 0) &&
	    (PARTNER_PARA.starvationTimer <= 0)) {
		setFoodTimer(type);
		PARTNER_PARA.starvationTimer = 0;
		PARTNER_PARA.condition &= ~4;
		if (PARTNER_PARA.energyLevel <
		    RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyThreshold)
			PARTNER_PARA.careMistakes += careAmt(MOD_CARE_HUNGER_STARVE_MISTAKES);
	}

	if (PARTNER_PARA.energyLevel <= 0) {
		if (careEveryRaw(MOD_CARE_HUNGER_WEIGHT_PERIOD)) {
			PARTNER_PARA.weight -= careAmt(MOD_CARE_HUNGER_EMPTY_WEIGHT);
			if (PARTNER_PARA.weight <= 0)
				PARTNER_PARA.weight = 1;
		}
		if (careEvery(MOD_CARE_HUNGER_EMPTY_PERIOD))
			PARTNER_PARA.emptyStomachTimer++;
	}
	if (careHas(MOD_CARE_HUNGER_TIRED_AFTER) &&
	    (PARTNER_PARA.tirednessHungerTimer >= careGet(MOD_CARE_HUNGER_TIRED_AFTER)) &&
	    careEvery(MOD_CARE_HUNGER_TIRED_PERIOD))
		PARTNER_PARA.foodLevel -= careAmt(MOD_CARE_HUNGER_TIRED_LOSS);

	if (RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyThreshold <=
	    PARTNER_PARA.energyLevel)
		setTrigger(0x280);
}

void handleConditionBubble(void)
{
	int32_t newBubble;
	uint32_t cond;

	newBubble = -1;

	if ((PARTNER_STATE != 1) && (PARTNER_STATE != 0xa)) {
		return;
	}

	cond = PARTNER_PARA.condition;

	if ((cond & 0x40) && (CONDITION_BUBBLE_TYPE != 2)) {
		newBubble = 2;
	}

	if ((cond & 0x20) && (CONDITION_BUBBLE_TYPE != 6)) {
		newBubble = 6;
	}

	if ((cond & 0x08) && (CONDITION_BUBBLE_TYPE != 1)) {
		newBubble = 1;
	}

	if ((cond & 0x04) && (CONDITION_BUBBLE_TYPE != 0)) {
		newBubble = 0;
	}

	if ((cond & 0x02) && (CONDITION_BUBBLE_TYPE != 4)) {
		newBubble = 4;
	}

	if ((cond & 0x01) && (CONDITION_BUBBLE_TYPE != 3)) {
		newBubble = 3;
	}

	if ((cond & 0x10) && (newBubble == -1) &&
	    (HAS_BUTTERFLY != 0) && (cond == 0x10)) {
		removeConditionBubble(CONDITION_BUBBLE_ID);
		BUTTERFLY_ID = setButterfly(ENTITY_TABLE[1]);
		HAS_BUTTERFLY = 0;
		return;
	} else if ((newBubble != CONDITION_BUBBLE_TYPE) && (newBubble != -1) &&
		   (CONDITION_BUBBLE_TIMER >= 50)) {
		if (HAS_BUTTERFLY == 0) {
			unsetButterfly(BUTTERFLY_ID);
			HAS_BUTTERFLY = -1;
			PARTNER_PARA.condition &= ~0x10;
			PARTNER_ENTITY.digimonEntity.entity.anim.loopCount = 1;
		}

		removeConditionBubble(CONDITION_BUBBLE_ID);
		CONDITION_BUBBLE_ID = addConditionBubble(newBubble, ENTITY_TABLE[1]);
		CONDITION_BUBBLE_TIMER = 0;
		CONDITION_BUBBLE_TYPE = newBubble;
	}

	CONDITION_BUBBLE_TIMER += 1;
	if (CONDITION_BUBBLE_TIMER >= 60) {
		CONDITION_BUBBLE_TYPE = -1;
	}
}

void partnerHandleFoodFeed(int32_t itemType)
{
	if (PARTNER_PARA.condition & 4) {
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
		if (((itemType < 0x26) || (itemType >= 0x47)) &&
		    (itemType != 0x79) && (itemType != 0x7a)) {
			return;
		}
#endif

		if (!(RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyThreshold >
		      PARTNER_PARA.energyLevel)) {
			PARTNER_PARA.happiness += careAmt(MOD_CARE_FEED_HUNGRY_HAPPY);
			PARTNER_PARA.discipline += careAmt(MOD_CARE_FEED_HUNGRY_DISC);
			PARTNER_PARA.condition &= ~4;
			setFoodTimer(PARTNER_ENTITY.digimonEntity.entity.type);
			PARTNER_PARA.starvationTimer = 0;
		}

		if ((TAMER_ITEM.worldItem.type ==
		     RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).favoriteFood) ||
		    !(PARTNER_PARA.energyLevel <
		      RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyThreshold)) {
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			PARTNER_ANIMATION = 0xb;
#endif
			startAnimation(&PARTNER_ENTITY.digimonEntity.entity, 0xb);
		}

		return;
	}

	if (((itemType < 0x26) || (itemType >= 0x47)) &&
	    (itemType != 0x79) && (itemType != 0x7a)) {
		return;
	}

	PARTNER_PARA.happiness -= careAmt(MOD_CARE_FEED_FULL_HAPPY);
	PARTNER_PARA.discipline -= careAmt(MOD_CARE_FEED_FULL_DISC);
}

void tickPoopingMechanic(void)
{
	uint32_t isPoopy;

	if (tamerGetState() != 0) {
		return;
	}

	isPoopy = PARTNER_PARA.condition & 8;

	if (!isPoopy && careEvery(MOD_CARE_POOP_PERIOD)) {
		PARTNER_PARA.poopLevel -= 1;
	}

	if (!isPoopy && (PARTNER_PARA.poopLevel <= 0)) {
		PARTNER_PARA.condition |= 8;
		PARTNER_PARA.poopingTimer =
			((PARTNER_PARA.discipline + careAmt(MOD_CARE_POOP_HOLD_OFFSET)) * careAmt(MOD_CARE_POOP_HOLD_SCALE)) / 100;
	}

	if (isPoopy && (CURRENT_FRAME != LAST_HANDLED_FRAME)) {
		PARTNER_PARA.poopingTimer -= clockAdvance();
	}

	if (isPoopy && (PARTNER_PARA.poopingTimer <= 0)) {
		partnerSetState(7);
		PARTNER_PARA.poopingTimer = -1;
		ITEM_SCOLD_FLAG = 1;
	}
}

void tickPoopDetection(void)
{
	int16_t radius;
	int16_t tileX;
	int16_t tileZ;
	int16_t posX;
	int16_t posZ;
	int32_t i;

	if (!careHas(MOD_CARE_EAT_SPECIES0) && !careHas(MOD_CARE_EAT_SPECIES1)) {
		return;
	}
	if ((PARTNER_ENTITY.digimonEntity.entity.type != careGet(MOD_CARE_EAT_SPECIES0)) &&
	    (PARTNER_ENTITY.digimonEntity.entity.type != careGet(MOD_CARE_EAT_SPECIES1))) {
		return;
	}

	for (i = 0; i < 100; ++i) {
		if ((WORLD_POOP[i].x == -1) ||
		    (WORLD_POOP[i].map != CURRENT_SCREEN)) {
			continue;
		}

		if (WORLD_POOP[i].size < careAmt(MOD_CARE_EAT_FAR_FROM)) {
			radius = careAmt(MOD_CARE_EAT_REACH_NEAR);
		} else {
			radius = careAmt(MOD_CARE_EAT_REACH_FAR);
		}

		tileX = ((WORLD_POOP[i].x - 50) * 100) + 50;
		tileZ = ((50 - WORLD_POOP[i].y) * 100) - 50;

		posX = TAMER_ENTITY.entity.posData->location.vx;
		posZ = TAMER_ENTITY.entity.posData->location.vz;

		if (((tileX - radius) < posX) && (posX < (tileX + radius)) &&
		    ((tileZ - radius) < posZ) && (posZ < (tileZ + radius))) {
			POOP_TO_EAT = i;
			PARTNER_STATE = 9;
			PARTNER_SUB_STATE = 0;
			return;
		}
	}
}

void handleToilet(void)
{
	PARTNER_PARA.happiness += careAmt(MOD_CARE_TOILET_HAPPY);
	PARTNER_PARA.discipline += careAmt(MOD_CARE_TOILET_DISC);
#if !VERSION_IS(US)
	PARTNER_PARA.poopLevel = RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopTimer;
#else
	PARTNER_PARA.poopLevel =
		(RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopTimer +
		 RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopTimer);
#endif
	PARTNER_PARA.condition &= ~8;
	handlePoopWeightLoss(ENTITY_TABLE[1]->type);
}

void handlePoopWeightLoss(int32_t type)
{
	int32_t r;

	r = randomLimit(careDie(MOD_CARE_POOPW_DIE));

	PARTNER_PARA.weight -= careGet(MOD_CARE_POOPW_DIV) > 0 ? (r + RAISE_ROW(type).poopSize) / careGet(MOD_CARE_POOPW_DIV) : 0;
	if (PARTNER_PARA.weight < 1) {
		PARTNER_PARA.weight = 1;
	}
}

void handleWildPoop(void)
{
	PARTNER_PARA.condition &= ~8;
	PARTNER_PARA.careMistakes += careAmt(MOD_CARE_WILD_MISTAKES);
	PARTNER_PARA.happiness -= careAmt(MOD_CARE_WILD_HAPPY);
	PARTNER_PARA.discipline -= careAmt(MOD_CARE_WILD_DISC);
#if !VERSION_IS(US)
	PARTNER_PARA.poopLevel = RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopTimer;
#else
	PARTNER_PARA.poopLevel =
		(RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopTimer +
		 RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopTimer);
#endif

	handlePoopWeightLoss(ENTITY_TABLE[1]->type);
}

void handleEatingPoop(void)
{
	Stats *stats;
	int16_t healingChance;
	int16_t r;

	stats = &PARTNER_ENTITY.digimonEntity.stats;
	healingChance = 0;

	/* MOD (care rules as data): three sizes of poop; the part of the maximum HP/MP it gives (a divisor; left out: none), its weight and the chance it cures */
	if (WORLD_POOP[POOP_TO_EAT].size < careAmt(MOD_CARE_EAT_MED_FROM)) {
		addWithLimit(&stats->current.currentHP, careGet(MOD_CARE_EAT_S_HP) > 0 ? stats->base.hp / careGet(MOD_CARE_EAT_S_HP) : 0, stats->base.hp);
		addWithLimit(&stats->current.currentMP, careGet(MOD_CARE_EAT_S_MP) > 0 ? stats->base.mp / careGet(MOD_CARE_EAT_S_MP) : 0, stats->base.mp);
		PARTNER_PARA.weight += careAmt(MOD_CARE_EAT_S_W);
		healingChance = careAmt(MOD_CARE_EAT_S_CURE);
	} else if (WORLD_POOP[POOP_TO_EAT].size < careAmt(MOD_CARE_EAT_LARGE_FROM)) {
		addWithLimit(&stats->current.currentHP, careGet(MOD_CARE_EAT_M_HP) > 0 ? stats->base.hp / careGet(MOD_CARE_EAT_M_HP) : 0, stats->base.hp);
		addWithLimit(&stats->current.currentMP, careGet(MOD_CARE_EAT_M_MP) > 0 ? stats->base.mp / careGet(MOD_CARE_EAT_M_MP) : 0, stats->base.mp);
		PARTNER_PARA.weight += careAmt(MOD_CARE_EAT_M_W);
		healingChance = careAmt(MOD_CARE_EAT_M_CURE);
	} else {
		addWithLimit(&stats->current.currentHP, careGet(MOD_CARE_EAT_L_HP) > 0 ? stats->base.hp / careGet(MOD_CARE_EAT_L_HP) : 0, stats->base.hp);
		addWithLimit(&stats->current.currentMP, careGet(MOD_CARE_EAT_L_MP) > 0 ? stats->base.mp / careGet(MOD_CARE_EAT_L_MP) : 0, stats->base.mp);
		PARTNER_PARA.weight += careAmt(MOD_CARE_EAT_L_W);
		healingChance = careAmt(MOD_CARE_EAT_L_CURE);
	}

	careClamp(&PARTNER_PARA.weight, MOD_CARE_LIM_WEIGHT_MIN, MOD_CARE_LIM_WEIGHT_MAX);

	r = randomLimit(100);
	if (r < healingChance) {
		if ((PARTNER_PARA.condition & 0x20) != 0) {
			PARTNER_PARA.condition &= ~0x20;
			PARTNER_PARA.injuryTimer = 0;
		}

		if ((PARTNER_PARA.condition & 0x40) != 0) {
			PARTNER_PARA.condition &= ~0x40;
			PARTNER_PARA.sicknessTimer = 0;
			PARTNER_PARA.areaEffectTimer = 0;
		}
	}

	WORLD_POOP[POOP_TO_EAT].map = -1; /* MOD (screens beyond 255): was 0xff */
	WORLD_POOP[POOP_TO_EAT].x = -1;
	WORLD_POOP[POOP_TO_EAT].y = -1;
	WORLD_POOP[POOP_TO_EAT].size = 0;
}

void tickSicknessMechanics(void)
{
	int32_t wasSick;
	int16_t roll;
	uint8_t chance;
	int32_t nameLength;

	wasSick = PARTNER_PARA.condition & 0x40;
	if (careHas(MOD_CARE_SICK_MIN_COUNTER) && (PARTNER_PARA.sicknessCounter >= careGet(MOD_CARE_SICK_MIN_COUNTER)) &&
	    careEvery(MOD_CARE_SICK_PERIOD) && !wasSick) {
		PARTNER_PARA.sicknessTries++;
		chance = (PARTNER_PARA.sicknessTries - careAmt(MOD_CARE_SICK_FREE_TRIES)) * careAmt(MOD_CARE_SICK_PER_TRY) +
			 (PARTNER_PARA.tiredness - careAmt(MOD_CARE_SICK_REST_TIRED));
		roll = randomLimit(100);
		if (roll < chance) {
			PARTNER_PARA.condition |= 0x40;
			PARTNER_PARA.timesBeingSick++;
			PARTNER_PARA.sicknessTimer = 1;
			PARTNER_PARA.happiness -= careAmt(MOD_CARE_SICK_ONSET_HAPPY);
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
			PARTNER_PARA.sicknessCounter = 0;
#endif
		}
	}

	if (!wasSick && careHas(MOD_CARE_SICK_AREA_AFTER) && (PARTNER_PARA.areaEffectTimer > careGet(MOD_CARE_SICK_AREA_AFTER)) &&
	    careEvery(MOD_CARE_SICK_PERIOD) &&
	    (MAP_TILE_DATA.partnerAreaResponse == 2)) {
		PARTNER_PARA.condition |= 0x40;
		PARTNER_PARA.timesBeingSick++;
		PARTNER_PARA.sicknessTimer = 1;
		PARTNER_PARA.areaEffectTimer = 0;
	}
	if (careEvery(MOD_CARE_SICK_PERIOD) &&
	    ((PARTNER_PARA.condition & 0x40) != 0)) {
		PARTNER_PARA.happiness -= careAmt(MOD_CARE_SICK_TICK_HAPPY);
		PARTNER_PARA.discipline -= careAmt(MOD_CARE_SICK_TICK_DISC);
		PARTNER_PARA.tiredness += careAmt(MOD_CARE_SICK_TICK_TIRED);
	}
	if (!wasSick && (PARTNER_PARA.sicknessTimer > 0))
		PARTNER_PARA.sicknessTimer = 0;

	if (careEvery(MOD_CARE_SICK_PERIOD)) {
		if ((PARTNER_PARA.condition & 0x40) != 0)
			PARTNER_PARA.sicknessTimer++;
		if ((PARTNER_PARA.condition & 0x20) != 0)
			PARTNER_PARA.injuryTimer++;
	}
	if ((HAS_IMMORTAL_HOUR == 1) && (IMMORTAL_HOUR == HOUR))
		return;

	if (careHas(MOD_CARE_SICK_INJURY_AFTER) && (PARTNER_PARA.injuryTimer >= careGet(MOD_CARE_SICK_INJURY_AFTER)) && !wasSick &&
	    (tamerGetState() == 0)) {
		PARTNER_PARA.condition |= 0x40;
		PARTNER_PARA.timesBeingSick++;
		PARTNER_PARA.sicknessTimer = 1;
		PARTNER_PARA.condition &= ~0x20;
		PARTNER_PARA.injuryTimer = 0;
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	}
	if (!wasSick && ((PARTNER_PARA.condition & 0x40) != 0)) {
#endif
		tamerSetState(0x14);
		clearTextArea();
		setTextColor(10);
		drawString(PARTNER_ENTITY.name, 0, 120);
		nameLength = strlen(PARTNER_ENTITY.name) / 2;
		setTextColor(1);
		drawString(MAIN_D_801225A0, nameLength * 12, 120);
	}

	if (careHas(MOD_CARE_SICK_LIFE_AFTER) && (PARTNER_PARA.sicknessTimer >= careGet(MOD_CARE_SICK_LIFE_AFTER)) && (PARTNER_STATE != 8) &&
	    (tamerGetState() == 0) && (PARTNER_PARA.remainingLifetime != 0) &&
	    (IS_SCRIPT_PAUSED == 1)) {
		writePStat(0xFF, 0);
		PARTNER_ENTITY.lives--;
		callScriptSection(0, 0x4DE, 0);
		PARTNER_PARA.sicknessTimer = 0;
	}
}

void tickDeathCondition(void)
{
	if ((HAS_IMMORTAL_HOUR == 1) && (IMMORTAL_HOUR == HOUR)) {
		return;
	}

	if (PARTNER_PARA.remainingLifetime > 0) {
		return;
	}

	if (PARTNER_STATE == 8) {
		return;
	}

	if (tamerGetState() != 0) {
		return;
	}

	if (IS_SCRIPT_PAUSED != 1) {
		return;
	}

	IS_NATURAL_DEATH = 1;

	writePStat(0xff, 0);
	PARTNER_PARA.remainingLifetime = 0;
	callScriptSection(0, 0x4de, 0);
}

#if !VERSION_IS(US)
void skipHours(int16_t hours)
{
	PARTNER_PARA.evoTimer += hours;
	PARTNER_PARA.remainingLifetime -= hours;
	if (HOUR + hours >= 24) {
		PARTNER_PARA.age += (HOUR + hours) / 24;
	}
	updateTimeOfDay();

	if ((HOUR + (hours % 4) == 0) && careHas(MOD_CARE_LIFE_BELOW) && careGet(MOD_CARE_LIFE_PER_POINTS) > 0 &&
	    (PARTNER_PARA.happiness < careGet(MOD_CARE_LIFE_BELOW)))
		PARTNER_PARA.remainingLifetime = PARTNER_PARA.remainingLifetime -
			((PARTNER_PARA.happiness - careGet(MOD_CARE_LIFE_BELOW)) / -careGet(MOD_CARE_LIFE_PER_POINTS) + 1);
	if (PARTNER_PARA.remainingLifetime < 0)
		PARTNER_PARA.remainingLifetime = 0;

	if ((PARTNER_PARA.condition & 1) != 0) {
		PARTNER_PARA.sicknessCounter += hours;
		PARTNER_PARA.missedSleepHours += hours;
	}
	if ((PARTNER_PARA.condition & 4) != 0) {
		PARTNER_PARA.starvationTimer -= (int16_t)(hours * careAmt(MOD_CARE_SKIP_STARVE));
		if ((PARTNER_PARA.starvationTimer <= 0) &&
		    (PARTNER_PARA.energyLevel <
		     RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyThreshold))
			PARTNER_PARA.careMistakes++;
	} else {
		PARTNER_PARA.foodLevel -= (int16_t)(hours * careAmt(MOD_CARE_SKIP_FOOD));
	}
	PARTNER_PARA.energyLevel -=
		RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyUsage;
	if ((PARTNER_PARA.condition & 8) != 0)
		PARTNER_PARA.poopingTimer -= 1200;
	else
		PARTNER_PARA.poopLevel -= (int16_t)(hours * careAmt(MOD_CARE_SKIP_POOP));

	if ((PARTNER_PARA.condition & 0x40) != 0)
		PARTNER_PARA.sicknessTries += hours;
	if ((PARTNER_PARA.condition & 0x20) != 0)
		PARTNER_PARA.injuryTimer += hours;
	if ((PARTNER_PARA.condition & 0x40) != 0)
		PARTNER_PARA.sicknessTimer += hours;
}
#else
void skipHours(int32_t hours)
{
	int16_t shortHours;
	int32_t rawHours;

	PARTNER_PARA.evoTimer += (int16_t)hours;
	shortHours = hours;
	PARTNER_PARA.remainingLifetime -= shortHours;
	rawHours = hours;
	if (HOUR + hours >= 24) {
		hours = HOUR + hours;
		PARTNER_PARA.age += (int16_t)(hours / 24);
	}
	updateTimeOfDay();

	if ((HOUR + (rawHours % 4) == 0) && careHas(MOD_CARE_LIFE_BELOW) && careGet(MOD_CARE_LIFE_PER_POINTS) > 0 &&
	    (PARTNER_PARA.happiness < careGet(MOD_CARE_LIFE_BELOW)))
		PARTNER_PARA.remainingLifetime = PARTNER_PARA.remainingLifetime -
			((PARTNER_PARA.happiness - careGet(MOD_CARE_LIFE_BELOW)) / -careGet(MOD_CARE_LIFE_PER_POINTS) + 1);
	if (PARTNER_PARA.remainingLifetime < 0)
		PARTNER_PARA.remainingLifetime = 0;

	if ((PARTNER_PARA.condition & 1) != 0) {
		PARTNER_PARA.sicknessCounter += shortHours;
		PARTNER_PARA.missedSleepHours += shortHours;
	}
	if ((PARTNER_PARA.condition & 4) != 0) {
		PARTNER_PARA.starvationTimer -= (int16_t)(rawHours * careAmt(MOD_CARE_SKIP_STARVE));
		if ((PARTNER_PARA.starvationTimer <= 0) &&
		    (PARTNER_PARA.energyLevel <
		     RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyThreshold))
			PARTNER_PARA.careMistakes++;
	} else {
		PARTNER_PARA.foodLevel -= (int16_t)(rawHours * careAmt(MOD_CARE_SKIP_FOOD));
	}
	PARTNER_PARA.energyLevel -=
		RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyUsage;
	if ((PARTNER_PARA.condition & 8) != 0)
		PARTNER_PARA.poopingTimer -= 1200;
	else
		PARTNER_PARA.poopLevel -= (int16_t)(rawHours * careAmt(MOD_CARE_SKIP_POOP));

	if ((PARTNER_PARA.condition & 0x40) != 0)
		PARTNER_PARA.sicknessTries += shortHours;
	if ((PARTNER_PARA.condition & 0x20) != 0)
		PARTNER_PARA.injuryTimer += shortHours;
	if ((PARTNER_PARA.condition & 0x40) != 0)
		PARTNER_PARA.sicknessTimer += shortHours;
}
#endif

void handlePostBattleTiredness(void)
{
	int16_t diff;

	PARTNER_PARA.tiredness += careAmt(MOD_CARE_BATTLE_TIRED);

	diff = (PARTNER_ENTITY.digimonEntity.stats.base.mp * careAmt(MOD_CARE_BATTLE_MP_LOW)) / 100;
	if (PARTNER_ENTITY.digimonEntity.stats.current.currentMP < (PARTNER_ENTITY.digimonEntity.stats.base.mp - diff)) {
		diff = PARTNER_ENTITY.digimonEntity.stats.base.mp - PARTNER_ENTITY.digimonEntity.stats.current.currentMP;
		/*
		 * NOTE: Vanilla MP factor calculation is broken.
		 * The factor should be ((diff * 10) / base.mp).
		 */
		PARTNER_PARA.tiredness += (diff / PARTNER_ENTITY.digimonEntity.stats.base.mp) * careAmt(MOD_CARE_BATTLE_MP_MULT);
	}

	PARTNER_PARA.foodLevel -= careAmt(MOD_CARE_BATTLE_FOOD);
}

void tickConditions(void)
{
#ifdef __MWERKS__
	extern int32_t getFreshEvolutionTarget(int32_t currentDigimon);
	extern int32_t getInTrainingEvolutionTarget(int32_t currentDigimon);
	extern int32_t getRookieEvolutionTarget(int32_t currentDigimon);
	extern int32_t getChampionEvolutionTarget(int32_t currentDigimon);
	extern int16_t tamerGetState(void);
#endif
	int16_t evoTimer;
	uint8_t level;
	int16_t tamerState;
	int32_t due;

	if (GAME_STATE != 0 || PARTNER_STATE != 1 || !IS_GAMETIME_RUNNING ||
	    CURRENT_FRAME == LAST_HANDLED_FRAME || FADE_PROTECTION == 1)
		return;
	tickDeathCondition();
	tickSleepMechanics();
	tickSicknessMechanics();
	tickTirednessMechanics();
	tickHungerMechanics();
	tickUnhappinessMechanics();
	tickConditionBoundaries();
	PARTNER_PARA.areaEffectTimer += clockAdvance();
#if VERSION_EQUAL_OR_NEWER(JP_TRIAL)
	if (PARTNER_PARA.areaEffectTimer > 28800)
		PARTNER_PARA.areaEffectTimer = 0;
#endif
	PARTNER_PARA.trainBoostTimer -= clockAdvance();
	if (PARTNER_PARA.trainBoostTimer <= 0) {
		PARTNER_PARA.trainBoostFlag = 0;
		PARTNER_PARA.trainBoostTimer = 0;
	}
	if (careEveryRaw(MOD_CARE_LIFE_PERIOD) && careHas(MOD_CARE_LIFE_BELOW) && careGet(MOD_CARE_LIFE_PER_POINTS) > 0 &&
	    PARTNER_PARA.happiness < careGet(MOD_CARE_LIFE_BELOW)) {
		PARTNER_PARA.remainingLifetime = PARTNER_PARA.remainingLifetime -
			((PARTNER_PARA.happiness - careGet(MOD_CARE_LIFE_BELOW)) / -careGet(MOD_CARE_LIFE_PER_POINTS) + 1);
		if (PARTNER_PARA.remainingLifetime < 0)
			PARTNER_PARA.remainingLifetime = 0;
	}
	if (!isInDaytimeTransition())
		return;
	/* Loading and waking grace protects against death, not a due evolution.
	 * The death and sickness checks enforce their own grace period. */
	if (HAS_IMMORTAL_HOUR == 1 && IMMORTAL_HOUR != HOUR) {
		HAS_IMMORTAL_HOUR = 0;
		IMMORTAL_HOUR = -1;
	}
	evoTimer = PARTNER_PARA.evoTimer;
	level = DIGI_PARA(PARTNER_ENTITY.digimonEntity.entity.type).level;
	tamerState = tamerGetState();
	if (CURRENT_FRAME != LAST_HANDLED_FRAME && PARTNER_STATE == 1 &&
	    tamerState == 0) {
		/* MOD (evolution rules as data): the hours of each level come from MOD_EVO_TIMING (-1 = never) */
		due = level >= 1 && level <= LEVEL_ULTIMATE && MOD_EVO_TIMING[level] >= 0 &&
		      evoTimer >= MOD_EVO_TIMING[level];
		if (due) {
			EVOLUTION_TARGET = getNaturalEvolutionTarget(PARTNER_ENTITY.digimonEntity.entity.type);
			/* The Fresh picker is also used to preview training's stop; only the tick resets its timer. */
			if (level == 1)
				PARTNER_PARA.evoTimer = 0;
		}
		/* MOD (evolution rules as data): the Champion hour that runs the praise-time rules */
		if (MOD_EVO_SPECIAL[MOD_SPECIAL_CHECK_LEVEL] >= 0 && level == MOD_EVO_SPECIAL[MOD_SPECIAL_CHECK_LEVEL] &&
		    evoTimer == MOD_EVO_SPECIAL[MOD_SPECIAL_CHECK_HOURS])
			EVOLUTION_TARGET = handleSpecialEvolutions(
				3, ENTITY_TABLE[1]);
	}
	/* MOD (evolution rules as data): the virus bar's threshold, the species it becomes and the two it does not apply to */
	if (MOD_EVO_SPECIAL[MOD_SPECIAL_VIRUS_THRESHOLD] >= 0 && MOD_EVO_SPECIAL[MOD_SPECIAL_VIRUS_TARGET] >= 0 &&
	    PARTNER_PARA.virusBar >= MOD_EVO_SPECIAL[MOD_SPECIAL_VIRUS_THRESHOLD]) {
		if (PARTNER_ENTITY.digimonEntity.entity.type != MOD_EVO_SPECIAL[MOD_SPECIAL_VIRUS_IMMUNE_A] &&
		    PARTNER_ENTITY.digimonEntity.entity.type != MOD_EVO_SPECIAL[MOD_SPECIAL_VIRUS_IMMUNE_B]) {
			EVOLUTION_TARGET = MOD_EVO_SPECIAL[MOD_SPECIAL_VIRUS_TARGET];
			storePartnerSpecies(5);
			PARTNER_PARA.sukaBackupHP =
				PARTNER_ENTITY.digimonEntity.stats.base.hp;
			PARTNER_PARA.sukaBackupMP =
				PARTNER_ENTITY.digimonEntity.stats.base.mp;
			PARTNER_PARA.sukaBackupOff =
				PARTNER_ENTITY.digimonEntity.stats.base.off;
			PARTNER_PARA.sukaBackupDef =
				PARTNER_ENTITY.digimonEntity.stats.base.def;
			PARTNER_PARA.sukaBackupSpeed =
				PARTNER_ENTITY.digimonEntity.stats.base.speed;
			PARTNER_PARA.sukaBackupBrain =
				PARTNER_ENTITY.digimonEntity.stats.base.brain;
			PARTNER_PARA.virusBar = 0;
		}
	}
	if (EVOLUTION_TARGET != -1 && PARTNER_STATE != 13) {
		tamerSetState(6);
		partnerSetState(13);
	}
}

/* MOD (fps): called by lateTickGameClock once per tick */
void tickStatusBarsSlide(int32_t state)
{
	if (state == 0)
		STATUS_UI_OFFSET_X += 50;
	else
		STATUS_UI_OFFSET_X -= 50;
	if (STATUS_UI_OFFSET_X > 240)
		STATUS_UI_OFFSET_X = 240;
	if (STATUS_UI_OFFSET_X < 75)
		STATUS_UI_OFFSET_X = 75;
}

/* B5a: tile art and fills live in unused ETCTIM cells, never in main RAM.
   Up to 13 FT4 packets use 520 bytes of the existing frame buffer. */
static void renderCareHUD(void)
{
	/* Texture u/v and screen x/y offsets for the seven icon variants. */
	static const uint8_t uv[4][7] = {
		{144,178,212,144,178,212,110},
		{224,224,224,237,237,237,242},
		{1,1,1,1,39,39,39},
		{72,72,86,86,65,79,93}
	};
	int32_t i, value, max, red, icon, x, y, pixels, active, warn, period, dim, clut;
	for (i = 0; i < 5; i++) {
		max = 100;
		icon = i + 2;
		active = warn = 0;
		switch (i) {
		case 0:
			value = PARTNER_PARA.happiness + 100;
			max = 200;
			red = PARTNER_PARA.happiness < 0;
			icon = red;
			break;
		case 1:
			value = PARTNER_PARA.discipline;
			red = value < 50;
			icon = 2 + red;
			break;
		case 2:
			value = PARTNER_PARA.foodLevel;
			period = careGet(MOD_CARE_HUNGER_FOOD_PERIOD);
			active = (PARTNER_PARA.condition & 4) != 0;
			if (period > 0) {
				active |= value <= 0;
				warn = value * period <= 1200;
			}
			red = active;
			max = 180;
			break;
		case 3:
			period = careGet(MOD_CARE_POOP_PERIOD);
			active = (PARTNER_PARA.condition & 8) != 0;
			max = RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).poopTimer * 2;
			value = 0;
			if (max <= 0)
				max = 1;
			else {
				value = PARTNER_PARA.poopLevel;
				if (period > 0) {
					active |= PARTNER_PARA.poopLevel <= 0;
					warn = PARTNER_PARA.poopLevel * period <= 1200;
				}
			}
			red = active;
			break;
		default:
			value = PARTNER_PARA.tiredness;
			red = value >= 50;
			period = careGet(MOD_CARE_TIRED_EXHAUSTED_AT);
			active = (PARTNER_PARA.condition & 2) != 0;
			if (period >= 0) {
				active |= value >= period;
				/* ceil(100 / 18): the last native gauge step before exhaustion. */
				warn = value + 6 >= period;
			}
			/* Energy remaining; condition thresholds still use raw fatigue. */
			value = max - value;
		}
		/* Needs share light-blue fills; active changes only the icon palette. */
		x = STATUS_UI_OFFSET_X + uv[2][icon];
		y = uv[3][icon];
		if (value < 0)
			value = 0;
		if (value > max)
			value = max;
		pixels = ((uint32_t)value * 18 + (uint32_t)max / 2) / (uint32_t)max;
		if (value && !pixels)
			pixels = 1;
		uint32_t phase = PLAYTIME_FRAMES / 5;
		/* One shared quotient preserves the 5-tick face and 20-tick warning phases. */
		dim = ((!active) & warn & (phase >> 2)) << 3;
		clut = getClut(272, 500 + icon + active * 4);
		/* Native disliked-habitat phase: five worse-face ticks, five normal. */
		if (i == 0 && MAP_TILE_DATA.partnerAreaResponse == 2 && !(phase & 1))
			clut = getClut(272, 511) - icon;
		/* Warning dimming affects the outlined bar, never the icon. */
		renderRectPolyFT4(x, y, dim ? 13 : 34, 13, uv[0][icon], uv[1][icon], 31, clut, 14, 0);
		if (dim)
			renderRectPolyFT4(x + 14, y + 5, 20, 4, uv[0][icon] + 14, uv[1][icon] + 5, 31, clut, 14, dim);
		if (pixels)
			renderRectPolyFT4(x + 15, y + 6, pixels, 2, i < 2 ? 144 : 164, i < 2 ? 251 + red * 2 : 251, 31, getClut(272, 507), 13, dim);
	}
}

// clang-format off
void renderStatusBars(state)
	int8_t state;
// clang-format on
{
	renderCareHUD();
}

void setImmortalHour(void)
{
	HAS_IMMORTAL_HOUR = 1;
	IMMORTAL_HOUR = HOUR;
}

/*
 * MOD (optional name): a partner the player did not name (OK on the empty keyboard) carries its species' name, and
 * that name follows it into every species it becomes. A typed name is two bytes a letter, a species name ASCII, so
 * the two are never equal: a name equal to the species' is always one the player left out.
 */
int32_t modPartnerNameIsSpecies(int32_t type)
{
	return strcmp(PARTNER_ENTITY.name, DIGIMON_NAME(type)) == 0;
}

void modPartnerNameSetSpecies(int32_t type)
{
	strncpy(PARTNER_ENTITY.name, DIGIMON_NAME(type), sizeof(PARTNER_ENTITY.name) - 1);
	PARTNER_ENTITY.name[sizeof(PARTNER_ENTITY.name) - 1] = '\0';
}

void modPartnerNameFollow(int32_t from, int32_t to)
{
	if (from != to && modPartnerNameIsSpecies(from)) {
		modPartnerNameSetSpecies(to);
	}
}
