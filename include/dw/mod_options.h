#ifndef DW_MOD_OPTIONS_H
#define DW_MOD_OPTIONS_H

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#include <dw/types.h>
#include <dw/lang.h>

/*
 * MOD (options): the title menu's OPTIONS (main_menu.c) and what they change (mod_options.c, docs/mods/options.md).
 */

#define MOD_BUTTONS_US	0	/* × confirms, △ cancels, ○ is the third button (the US disc) */
#define MOD_BUTTONS_JP	1	/* JAPANESE CONTROLS: ○ confirms, × cancels, △ is the third button (input.h) */

extern int32_t MOD_OPT_BUTTONS;	/* JAPANESE CONTROLS: MOD_BUTTONS_JP (input.h reads it) */
extern int32_t MOD_OPT_CLOCK;	/* DIGITAL CLOCK: the hour in digits under the clock (clock.c), 1 = shown */
extern int32_t MOD_OPT_BARS;	/* NEW STATUS BARS: happiness and discipline as one red/green fill (patches 0162-0164), 0 = the original's */
extern int32_t MOD_OPT_EVO_GUIDE; /* EVOLUTION GUIDE: the evolution chart's requirement card and lit lines (patch 0176), 0 = the original chart */
extern int32_t MOD_OPT_NEW_DIGIMON; /* NEW DIGIMON: the species the mod made raisable; 0 = the original's only */
extern int32_t MOD_OPT_CHEATS; /* CHEATS: lab menu and Select shortcuts, off by default */
extern int32_t MOD_OPT_TOUCHED;	/* changed in OPTIONS since the console started: a loaded save keeps it */

/* the code of the button glyph that does what the US layout's glyph `code` (Shift-JIS, big endian) does */
uint16_t modButtonGlyph(uint16_t code);
/* the colour (setTextColor's) to draw the US layout's button glyph `code` in, given the text's colour `color` */
int32_t modButtonColor(uint16_t code, int32_t color);
/* the CLUT of the dialogue box's button frames (ours: the ○ red, the × blue) */
uint16_t modButtonClut(void);
/* the button layout changes: the dialogue box's button in VRAM follows it */
void modSetButtons(int32_t layout);
/* 1 when the partner may become `type` (NEW DIGIMON on, or a species the original raises: isVanillaSpecies) */
int32_t modSpeciesAllowed(int32_t type);
/* NEW DIGIMON off turned the loaded partner from MOD_SWAP_FROM into MOD_SWAP_TO (0: no); back on, a partner still of
 * MOD_SWAP_TO is turned back (main_menu.c, SAVE_TAG_VANILLA_SWAP) */
extern int16_t MOD_SWAP_FROM;
extern int16_t MOD_SWAP_TO;
/* FRAME RATE: the rate the game is shown at, 30 or 60 (20 in an FPS=20 build, which has no choice) */
int32_t modFrameRate(void);
void modSetFrameRate(int32_t fps);
/* a TIM of ETCTIM.BIN was uploaded (loadStackedTIMFile): the SYSTEM_W page's button frames */
void modButtonsTimLoaded(GsIMAGE *img);
/* at boot: a disc whose dialogue box button is the Japanese ○ starts with JAPANESE CONTROLS on (saved options win) */
void modButtonsBoot(void);

/*
 * MOD (language): LANGUAGE and MAP SIGNS (docs/mods/language.md). The executable's strings and language tables are
 * switched by writing the other language's bytes over them (LANGEN.BIN / LANGES.BIN, tools/langpatch.py); the disc's
 * files are read from their language's copy when the disc has one, and the map backgrounds from their copy with the
 * signs translated unless MAP SIGNS is ORIGINAL (modLangFile: the table tools/build-full.sh writes, LANG_FILES). Only
 * at the title (OPTIONS, a loaded save's options), as the font.
 */
extern int32_t MOD_LANG;	/* the language the game shows */
extern int32_t MOD_OPT_SIGNS;	/* MAP SIGNS: 1 = translated (the language's), 0 = as the disc has them */
void modSetLanguage(int32_t lang);
int32_t modLanguageAt(int32_t index);
int32_t modLanguageCount(void);
void modSetSigns(int32_t translated);
/* the file name to read for `name` (no folder): its language's or translated signs' copy when the disc has it */
char *modLangFile(char *name);

/*
 * MOD (story): EXTENDED STORY (docs/mods/options.md). On, the game reads the disc's files: the script text and the maps
 * with the story mods. Off, the files they change are read from their copies as the original story has them
 * (modStoryFile: the same name with an underscore for its first letter, tools/dws/variants.py), the language's copy's
 * own copy for a file read in the language. A save keeps the story it was played with; a Continue with the other one moves the tamer to
 * Jijimon's house (main_menu.c modStoryRelocate, main.c initializeLoadedMap).
 */
extern int32_t MOD_OPT_STORY;	/* EXTENDED STORY: 1 = with the story mods, 0 = the original story */
extern int32_t MOD_STORY_ARRIVE; /* a Continue changed the story: the tamer arrives at the loaded screen's exit */
/* the file name to read for `name` (no folder): its original story's copy when the option is off and the disc has it */
char *modStoryFile(char *name);
/* 1 when the disc has the original story's copies (the option is a choice) */
int32_t modStoryChoice(void);
void modSetStory(int32_t extended);

/*
 * MOD (difficulty): DIFFICULTY (docs/mods/options.md). EASY: in the wild battles and the tournaments (the BTL and STD
 * overlays) every hit, finisher and block chip the partner deals is doubled and every one it takes, poison included, is
 * halved (rounded up, so a hit still does at least 1); the stats and the saved HP are not touched, so NORMAL is the original
 * game again at once. The VS mode is two players and stays as it is.
 */
#define MOD_DIFF_NORMAL	0
#define MOD_DIFF_EASY	1
extern int32_t MOD_OPT_DIFFICULTY; /* DIFFICULTY: MOD_DIFF_EASY or MOD_DIFF_NORMAL */
extern int32_t MOD_OPT_QUICK_BATTLE; /* QUICK BATTLE START: a field battle starts at once, its intro over the fight (battle_setup.c, btl.c); 0 = the original start */
/* the damage `dmg` that `attacker` deals `defender` with the difficulty (`partner`: the partner's entity) */
#define MOD_DIFF_DAMAGE(attacker, defender, partner, dmg)                                                              \
	(MOD_OPT_DIFFICULTY != MOD_DIFF_EASY ? (dmg)                                                                       \
	 : (Entity *)(attacker) == (partner) ? (dmg) * 2                                                                  \
	 : (Entity *)(defender) == (partner) ? ((dmg) + 1) / 2                                                               \
					     : (dmg))

#endif
