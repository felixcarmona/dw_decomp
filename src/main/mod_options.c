/*
 * MOD (options): what the title menu's OPTIONS change (main_menu.c draws and runs the menu; docs/mods/options.md).
 *
 * JAPANESE CONTROLS. The Japanese disc's code differs from the US one in three masks only (input.h): ○ confirms, ×
 * cancels, △ is the third button; whatever the code tests by its bit (△ opening the field menu) is the same on both.
 * With the option on, input.h's CONFIRM_BUTTON, CANCEL_BUTTON and ALT_BUTTON read MOD_OPT_BUTTONS and are the
 * Japanese disc's, so every screen answers as on that disc.
 * The text follows: the font draws each button glyph as the button that now does its job (modButtonGlyph, font.c), so
 * "×Select △Cancel" reads "○Select ×Cancel" without a text of its own; a △ painted green ({color 5}, the fishing help's
 * "submenu, under △") is the field menu's button and stays. A button glyph the text paints in its button's colour
 * (× cyan 6, △ green 5, ○ orange 10) takes the colour of the button it now is (modButtonColor). The dialogue box's
 * button (SYSTEM_W.TIM, three 16x12 frames pressing it, drawn by renderUIBox and the care menu's badge) is made a ○
 * from the disc's own frames: the cross is painted over with the button's face and a ring drawn where it was, in the
 * red of the PlayStation's ○ (its own CLUT, modButtonClut).
 *
 * DIGITAL CLOCK. The hour as HH:MM on a plate under the clock (patch 0166, renderDigitalClock in clock.c), on or off.
 *
 * NEW STATUS BARS. The field's happiness and discipline bars as patches 0162-0164 draw them (one fill, red below the
 * midpoint, green from it, on a grey track), or off, as the original drew them (renderStatusBars, partner_impl.c).
 *
 * FRAME RATE. 20 (the original: one render per tick) or 60 fps (MOD_FPS_MODE, dw/mod_fps.h); the game logic runs
 * at 20 Hz in both (30 is still a mode for an FPS=30 build). A build made with FPS=20
 * keeps 20 and the option shows it without a choice.
 *
 * LANGUAGE. English or Spanish (docs/mods/language.md). The executable is built in both languages with every string
 * padded to the longer of its two texts, so the two builds differ only in those bytes: the disc carries the other
 * language's as runs (address, bytes; tools/langpatch.py), written over the strings when the language changes. The
 * files that have a copy in the language (the script text, textures with words in them, overlays) are read from it
 * (modLangFile); the script text is read again and FI_INFO.TIM uploaded again at once, ETCTIM.BIN is loaded
 * after the title anyway.
 *
 * MAP SIGNS. The signs painted in the map backgrounds in the language (the disc's copies of the .TFS files) or as the
 * disc has them (some still in Japanese).
 *
 * EXTENDED STORY. The story with the mods built onto the disc (new places, the city's new residents) or the original
 * one: the files the mods change are read from their copies as the original story has them (modStoryFile); the script
 * text is read again at once, the maps when they are next entered. A disc built without a story mod has no copies and
 * no choice.
 *
 * DIFFICULTY. EASY doubles the damage the partner deals and halves the damage it takes in the battles and tournaments
 * (MOD_DIFF_DAMAGE, applied by BTL_calculateDamage and STD_calculateDamage, and to the poison); NORMAL is the original.
 */
#include <dw/mod_options.h>
#include <dw/species.h>
#include <dw/mod_fps.h>
#include <dw/mod_font.h>
#include <lang_base.h>
#include <dw/main.h>
#include <string.h>
#include <dw/file_table.h>

#include "common.h"

extern uint8_t COLORCODE_LOWBITS; /* font.c: the text's colour */

int32_t MOD_OPT_BUTTONS = MOD_BUTTONS_US;
int32_t MOD_OPT_CLOCK = 1;
int32_t MOD_OPT_BARS = 1;
int32_t MOD_OPT_EVO_GUIDE = 1;
int32_t MOD_OPT_NEW_DIGIMON = 1;
int16_t MOD_SWAP_FROM;
int16_t MOD_SWAP_TO;

int32_t modSpeciesAllowed(int32_t type)
{
	return MOD_OPT_NEW_DIGIMON || isVanillaSpecies(type);
}
int32_t MOD_OPT_TOUCHED = 0;
int32_t MOD_LANG = MOD_LANG_BASE;
int32_t MOD_OPT_SIGNS = 1;
int32_t MOD_OPT_STORY = 1;
int32_t MOD_OPT_DIFFICULTY = MOD_DIFF_NORMAL;
int32_t MOD_STORY_ARRIVE = 0;

/* the colour the text draws a button glyph in (the US disc's: × cyan, △ green, ○ orange); -1 = not a button */
static int32_t buttonColor(uint16_t code)
{
	switch (code) {
	case 0x81A2:
		return 6;
	case 0x817E:
		return 5;
	case 0x819B:
		return 10;
	}
	return -1;
}

int32_t modButtonColor(uint16_t code, int32_t color)
{
	if (MOD_OPT_BUTTONS == MOD_BUTTONS_JP && color == buttonColor(code)) {
		return buttonColor(modButtonGlyph(code));
	}
	return color;
}

uint16_t modButtonGlyph(uint16_t code)
{
	if (MOD_OPT_BUTTONS != MOD_BUTTONS_JP) {
		return code;
	}
	if (code == 0x817E && COLORCODE_LOWBITS == 5) {
		return code; /* a green △: the field menu's button, △ on both discs */
	}
	switch (code) {
	case 0x81A2: /* × confirms: ○ */
		return 0x819B;
	case 0x817E: /* △ cancels: × */
		return 0x81A2;
	case 0x819B: /* ○, the third button: △ */
		return 0x817E;
	}
	return code;
}

/*
 * The button frames: u 0x4C, v 0xB7 of the UI texture page (320, 0), 4 bpp, CLUT (96, 500); in SYSTEM_W.TIM (at
 * (320, 128)) VRAM words 19..30, rows 55..66. Index 0xC is the cross's light blue, 0xD its dark blue, 3 the face.
 */
#define FRAMES_X	19	/* VRAM words from the TIM's left edge */
#define FRAMES_Y	55	/* rows from its top */
#define FRAMES_WORDS	12	/* 3 frames of 16 texels */
#define FRAMES_ROWS	12
#define INK_LIGHT	0xC
#define INK_DARK	0xD
#define FACE		0x3
#define FRAMES_CLUT_Y	12	/* CLUT (96, 500): row 12 of the TIM's 16 */
/*
 * The ring's CLUT: the frames' one with the cross's two blues made the ○'s red. It lives in the page's filler (u 0x80..0xBF
 * of v 0xE6, all index 15: nothing draws it) at VRAM (352, 230); with the US layout the filler is put back.
 */
#define RING_CLUT_X	352
#define RING_CLUT_Y	230
#define RING_LIGHT	0x359F	/* (248, 96, 104) */
#define RING_DARK	0x1CD4	/* (160, 48, 56) */

static uint16_t FRAMES_CROSS[FRAMES_ROWS * FRAMES_WORDS];
static uint16_t FRAMES_RING[FRAMES_ROWS * FRAMES_WORDS];
static int32_t FRAMES_READY = 0;
static uint16_t RING_CLUT[16];
static uint16_t RING_CLUT_FILLER[16];

/* the ring that takes the cross's 6x5 place: L light, D dark, . the face */
static const char RING[5][7] = {
	".DLLD.",
	"DL..LD",
	"L....L",
	"DL..LD",
	".DLLD.",
};

static int32_t texel(uint16_t *f, int32_t x, int32_t y)
{
	return (f[y * FRAMES_WORDS + x / 4] >> (4 * (x & 3))) & 0xF;
}

static void setTexel(uint16_t *f, int32_t x, int32_t y, int32_t c)
{
	uint16_t *w = &f[y * FRAMES_WORDS + x / 4];

	*w = (*w & ~(0xF << (4 * (x & 3)))) | (c << (4 * (x & 3)));
}

/* FRAMES_RING from FRAMES_CROSS; 0 when a frame's cross is not the 6x5 one this expects */
static int32_t makeRing(void)
{
	int32_t frame, x, y, c, x0, y0, x1, y1;

	for (x = 0; x < FRAMES_ROWS * FRAMES_WORDS; x++) {
		FRAMES_RING[x] = FRAMES_CROSS[x];
	}
	for (frame = 0; frame < 3; frame++) {
		x0 = y0 = 99;
		x1 = y1 = -1;
		for (y = 0; y < FRAMES_ROWS; y++) {
			for (x = frame * 16; x < frame * 16 + 16; x++) {
				c = texel(FRAMES_RING, x, y);
				if (c == INK_LIGHT || c == INK_DARK) {
					setTexel(FRAMES_RING, x, y, FACE);
					x0 = x < x0 ? x : x0;
					y0 = y < y0 ? y : y0;
					x1 = x > x1 ? x : x1;
					y1 = y > y1 ? y : y1;
				}
			}
		}
		if (x1 - x0 != 5 || y1 - y0 != 4) {
			return 0;
		}
		for (y = 0; y < 5; y++) {
			for (x = 0; x < 6; x++) {
				if (RING[y][x] != '.') {
					setTexel(FRAMES_RING, x0 + x, y0 + y, RING[y][x] == 'L' ? INK_LIGHT : INK_DARK);
				}
			}
		}
	}
	return 1;
}

static void uploadFrames(void)
{
	RECT rect;

	if (!FRAMES_READY) {
		return;
	}
	setRECT(&rect, 320 + FRAMES_X, 128 + FRAMES_Y, FRAMES_WORDS, FRAMES_ROWS);
	LoadImage(&rect, (u_long *)(MOD_OPT_BUTTONS == MOD_BUTTONS_JP ? FRAMES_RING : FRAMES_CROSS));
	setRECT(&rect, RING_CLUT_X, RING_CLUT_Y, 16, 1);
	LoadImage(&rect, (u_long *)(MOD_OPT_BUTTONS == MOD_BUTTONS_JP ? RING_CLUT : RING_CLUT_FILLER));
}

uint16_t modButtonClut(void)
{
	if (FRAMES_READY && MOD_OPT_BUTTONS == MOD_BUTTONS_JP) {
		return GetClut(RING_CLUT_X, RING_CLUT_Y);
	}
	return GetClut(96, 500);
}

void modButtonsTimLoaded(GsIMAGE *img)
{
	uint16_t *src;
	int32_t x, y;

	if (img->px != 320 || img->py != 128 || (img->pmode & 7) != 0 || img->pw < FRAMES_X + FRAMES_WORDS ||
	    img->ph < FRAMES_Y + FRAMES_ROWS) {
		return;
	}
	src = (uint16_t *)img->pixel;
	for (y = 0; y < FRAMES_ROWS; y++) {
		for (x = 0; x < FRAMES_WORDS; x++) {
			FRAMES_CROSS[y * FRAMES_WORDS + x] = src[(FRAMES_Y + y) * img->pw + FRAMES_X + x];
		}
	}
	FRAMES_READY = 0;
	if (img->cw < 16 || img->ch <= FRAMES_CLUT_Y || ((img->pmode >> 3) & 1) == 0) {
		return;
	}
	for (x = 0; x < 16; x++) {
		RING_CLUT[x] = ((uint16_t *)img->clut)[FRAMES_CLUT_Y * img->cw + x];
		RING_CLUT_FILLER[x] = src[(RING_CLUT_Y - 128) * img->pw + (RING_CLUT_X - 320) + x];
	}
	RING_CLUT[INK_LIGHT] = RING_LIGHT;
	RING_CLUT[INK_DARK] = RING_DARK;
	FRAMES_READY = makeRing();
	if (MOD_OPT_BUTTONS == MOD_BUTTONS_JP) {
		uploadFrames();
	}
}

void modSetButtons(int32_t layout)
{
	MOD_OPT_BUTTONS = layout == MOD_BUTTONS_JP ? MOD_BUTTONS_JP : MOD_BUTTONS_US;
	uploadFrames();
}

int32_t modFrameRate(void)
{
	return MOD_FPS_MODE == 2 ? 60 : MOD_FPS_MODE == 1 ? 30 : 20;
}

void modSetFrameRate(int32_t fps)
{
#if FPS != 20
	MOD_FPS_MODE_RUN = fps == 60 ? 2 : fps == 30 ? 1 : 0;
#endif
}

/* MOD (language): the disc has the file (its FILE_OFFSET_TABLE entry got a sector: build-disc.py wrote it) */
int32_t modFileOnDisc(char *name)
{
	FileEntry *entry;

	for (entry = FILE_OFFSET_TABLE; entry->size != 0; ++entry) {
		if (fileEntryIs(entry, name)) {
			return entry->sector != 0;
		}
	}
	return 0;
}

/*
 * MOD (language): a file's copy in the language, and a map background's (.TFS, and the .MAP whose sprites cover a sign)
 * with its signs translated (the Spanish copy also has them in Spanish; MAP SIGNS ORIGINAL reads the disc's .TFS), from
 * the table tools/build-full.sh writes (LANG_FILES: the name, its Spanish copy, its copy with English signs); the file
 * itself when the disc has no such copy. A row is the name's FNV-1a hash and the copies' places among the new files
 * (tools/gen_lang_files.py, which refuses a hash two names of the disc share): the names are FILE_OFFSET_TABLE's.
 */
typedef struct {
	uint32_t hash;
	uint16_t copy[2]; /* Spanish, English signs: a place after FILE_TABLE_NEW_FIRST, 0xFFFF none */
} LangFileRow;

static LangFileRow LANG_FILE_TABLE[] = {
#include <lang_files.h>
	{ 0, { 0xFFFF, 0xFFFF } },
};

static char MOD_LANG_NAME[FILE_NAME_MAX + 1]; /* the copy's name, ended (an entry's has no NUL at 12 letters) */

static uint32_t modNameHash(char *name)
{
	uint32_t h = 0x811C9DC5;

	while (*name != '\0') {
		h = (h ^ (uint8_t)*name++) * 0x01000193;
	}
	return h;
}

char *modLangFile(char *name)
{
	uint32_t i;
	uint32_t hash;
	int32_t tfs;
	int32_t copy;

	hash = modNameHash(name);
	for (i = 0; i < sizeof(LANG_FILE_TABLE) / sizeof(LangFileRow) - 1; i++) {
		if (LANG_FILE_TABLE[i].hash != hash) {
			continue;
		}
		tfs = strlen(name) > 4 && (!strcmp(name + strlen(name) - 4, ".TFS") || !strcmp(name + strlen(name) - 4, ".MAP")); /* a background or its sprites */
		if (tfs && !MOD_OPT_SIGNS) {
			return name;
		}
		copy = LANG_FILE_TABLE[i].copy[MOD_LANG == MOD_LANG_ES ? 0 : 1];
		if (copy == 0xFFFF || (!tfs && MOD_LANG != MOD_LANG_ES)) {
			return name;
		}
		memcpy(MOD_LANG_NAME, FILE_OFFSET_TABLE[FILE_TABLE_NEW_FIRST + copy].filename, FILE_NAME_MAX);
		return FILE_OFFSET_TABLE[FILE_TABLE_NEW_FIRST + copy].sector != 0 ? MOD_LANG_NAME : name;
	}
	return name;
}

/*
 * MOD (story): a file's copy as the original story has it, with EXTENDED STORY off: the same name with its first letter
 * an underscore (_G.SCN for DG.SCN, _AYO03.MAP for MAYO03.MAP; tools/dws/variants.py names them), when the disc has it.
 * A rule, not a table: a hundred copies would cost the executable's heap their rows and names. lookupFileTable looks
 * the language's copy up first, and the copy of a language's copy follows the same rule (LE012.TFS, _E012.TFS): the
 * original story in Spanish needs no language rows of its own.
 */
static char MOD_STORY_NAME[16];

char *modStoryFile(char *name)
{
	if (MOD_OPT_STORY || name[0] == '\0' || strlen(name) >= sizeof(MOD_STORY_NAME)) {
		return name;
	}
	strcpy(MOD_STORY_NAME, name);
	MOD_STORY_NAME[0] = '_';
	return modFileOnDisc(MOD_STORY_NAME) ? MOD_STORY_NAME : name;
}

int32_t modStoryChoice(void)
{
	return modFileOnDisc("_G.SCN");
}

/* MOD (language): the patch that turns the executable's strings into each language */
static char *LANG_PATCH_FILE[MOD_LANGS] = { "LANGEN.BIN", "LANGES.BIN" };
extern char MAIN_D_8012CE64[]; /* main.c: "\\ETCDAT\\FI_INFO.TIM" */
void tickFileReadQueue(int32_t unused);
int32_t isFileReadQueueBusy(void);
int32_t readFile(char *path, void *buffer);
int32_t loadTIMFile(char *path, void *buffer);
void modFontScriptsChanged(void);

void modSetLanguage(int32_t lang)
{
	uint32_t *run;
	uint32_t count;
	uint32_t len;

	lang = lang == MOD_LANG_ES ? MOD_LANG_ES : MOD_LANG_EN;
	if (lang == MOD_LANG || !modFileOnDisc(LANG_PATCH_FILE[lang])) { /* a disc in one language: no choice */
		return;
	}
	while (isFileReadQueueBusy()) {
		tickFileReadQueue(0);
	}
	DrawSync(0);
	readFile(LANG_PATCH_FILE[lang], GENERAL_BUFFER); /* free at the title: the next loads put their own files there */
	run = (uint32_t *)GENERAL_BUFFER;
	if (run[0] != 0x504C5744) { /* "DWLP" */
		return;
	}
	count = run[1];
	run += 2;
	while (count-- > 0) {
		len = run[1];
		memcpy((void *)run[0], &run[2], len);
		run += 2 + (len + 3) / 4;
	}
	MOD_LANG = lang;
	modFontScriptsChanged(); /* the script text of the language */
	loadTIMFile(MAIN_D_8012CE64, GENERAL_BUFFER); /* the triangle menu's labels */
}

void modSetSigns(int32_t translated)
{
	MOD_OPT_SIGNS = translated != 0; /* the next map reads its background */
}

/* MOD (story): EXTENDED STORY on or off; the script text of that story is read again (the maps when entered) */
void modSetStory(int32_t extended)
{
	extended = extended != 0;
	if (extended == MOD_OPT_STORY || !modStoryChoice()) { /* a disc without the copies: no choice */
		return;
	}
	MOD_OPT_STORY = extended;
	modFontScriptsChanged();
}
