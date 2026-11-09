#include <string.h>

#include <libgs.h>

#include <dw/mod_options.h>
#include <dw/font.h>
#include <dw/garbage.h>
#include <dw/input.h>
#include <dw/item.h>
#include <dw/mod_font.h>
#include <dw/params.h>
#include <dw/script.h>
#include <dw/sjis.h>
#include <dw/sound.h>
#include <dw/tamer.h>
#include <dw/ui.h>
#include <dw/version.h>

extern uint8_t TEXTBOX_OPEN_TIMER;
extern GsOT *ACTIVE_ORDERING_TABLE;
extern char *MOVE_NAMES[];
extern int32_t ACTIVE_SCRIPT;
extern char *BGM_TRACK_NAMES[];
extern char *TOURNAMENT_NAMES[];

void renderSelectionCursor(int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4);
void setPosDataPolyFT4(POLY_FT4 *prim, int32_t posX, int32_t posY, int32_t width, int32_t height);
void setUVDataPolyFT4(POLY_FT4 *prim, int32_t xPos, int32_t yPos, int32_t width, int32_t height);
void renderString(int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8);
int32_t flipIdleTextboxPage(int32_t boxId);
void closeTextbox(int32_t boxId, RECT *target);
void renderCustomSizedTextbox(void);
int32_t isKeyDown(uint32_t key);
void setFreshDialogue(void);
int32_t isXPressedAfterDialogue(void);
void clearTextboxLineCount(int32_t boxId);
int32_t tickConfirmDialogue(void);
void renderDialogueSelectionCursor(int32_t x, int32_t y);
void tickCustomSizedTextbox(void);
void getVRAMModeCoords(int32_t mode, int32_t *outX, int32_t *outClut);
int32_t advanceTextbox(int32_t boxId);
void setupDialogueBox(uint8_t owner);
uint16_t showTextboxReady(uint8_t boxId, uint8_t speakerId);
#if !VERSION_IS(US)
int32_t drawTextboxStrings(int32_t boxId);
#else
int32_t drawTextboxStrings(int32_t boxId, int32_t flag);
#endif
int32_t tickSelectionDialogue(void);
int32_t tickBackgroundDialogue(void);
int32_t getSpeakerName(int32_t speakerId, uint8_t *buf);
uint8_t *intToStringSJIS(uint8_t *buf, int32_t value, uint8_t digits, int32_t flag);
void renderUIBox(int32_t boxId);
void createTextbox(int32_t boxId, int32_t flags, RECT *rect, RECT *origin, void *tick, void *render);

int32_t MAIN_D_80134F94;
#if VERSION_IS(US)
int8_t DRAW_STRING2_IS_FIXED_WIDTH;
#endif

static void *script_textbox_functions[] = {
	showTextboxReady,
	scriptSetTextboxSize,
	showTextbox,
	scriptShowSelection,
	renderUIBox,
	renderScriptDialogueBox,
	tickScriptDialogueBox,
	setupBoxOrigin,
	setupDialogueBox,
	advanceTextbox,
	flipIdleTextboxPage,
	drawString2,
	registerTextbox,
	triggerBoxCloseFlag,
	createTextbox,
	closeBox,
	closeAllTextboxes,
	closeTextbox,
	getVRAMModeCoords,
	tickTextboxHandling,
	initializeTextbox,
	renderCustomSizedTextbox,
	tickCustomSizedTextbox,
	renderDialogueSelectionCursor,
	tickBackgroundDialogue,
	tickConfirmDialogue,
	tickSelectionDialogue,
	clearTextboxLineCount,
	drawTextboxStrings,
};

void renderScriptDialogueBox(void)
{
	int16_t rowPx;
	int32_t i;
	int16_t x;
	int16_t y;
	int16_t ySave;

	rowPx = TEXTBOX_DATA.box[0].vramRow * 12;
	rowPx += (TEXTBOX_DATA.box[0].backPage * 48);
	x = UI_BOX_DATA[0].finalPos.x + 5;
	y = UI_BOX_DATA[0].finalPos.y + 4;
	ySave = y;

	i = 0;
	while (i < 4) {
		renderString(0, x, y, 0xfc, 0xc, 0, rowPx, 6, 1);
		i++;
		rowPx += 0xc;
		y += 0xd;
	}

	if (ACTIVE_INSTRUCTION == SCRIPT_OP_SET_SELECTION) {
		renderDialogueSelectionCursor(x, ySave);
	} else if (ACTIVE_INSTRUCTION == SCRIPT_OP_CALL_ROUTINE && SCRIPT_TEXTBOX_MODE == 2) {
		renderDialogueSelectionCursor(x, ySave);
	}

	if (MAIN_D_80134F94 != 0) {
		renderUIBox(0);
	}
}

#if FONT_SWITCH
/*
 * MOD (font): the one text box row that a screen copies in pieces (TEXTBOX_ROW_PIECES, dw/mod_font.h: the card shop's
 * Yes / No), laid out by drawString2Pieces; createTextbox of that box forgets it
 */
static const uint16_t *TEXTBOX_PIECES;
static uint8_t TEXTBOX_PIECES_BOX; /* the box id + 1; 0: none */
static uint8_t TEXTBOX_PIECES_ROW;
static const uint16_t *DRAW2_PIECES; /* set while drawString2 draws that row */
#endif

// clang-format off
#if !VERSION_IS(US)
int32_t drawTextboxStrings(boxId)
#else
int32_t drawTextboxStrings(boxId, flag)
#endif
	uint8_t boxId;
#if VERSION_IS(US)
	int32_t flag;
#endif
// clang-format on
{
	TextBoxData *box;
	int32_t done;
	uint32_t x;
	int32_t clut;
	int16_t px;
	int16_t row;
	uint8_t *buf;
#if FONT_SWITCH
	int32_t cellsRow; /* MOD (language) */
	int8_t fixed;
#endif

	box = &TEXTBOX_DATA.box[boxId];
	if (box->writeCount == box->renderCount) {
		return 0;
	}

	getVRAMModeCoords(box->vramMode, (int32_t *)&x, &clut);
	px = x;
	row = box->vramRow + box->writeRow;
	buf = TEXTBOX_LINES_PTR + (row << 6);
	if (x != 0) {
		buf += 0x20;
	}
	row = row * 12;
	if (box->vramMode == 0) {
		x = 1;
	} else {
		x = 2;
	}

#if VERSION_IS(US)
	FONT_CELLS = (TEXTBOX_CELLS >> boxId) & 1;
#endif
	while (x != 0) {
#if !VERSION_IS(US)
		done = drawString2(buf, px, row);
#else
#if FONT_SWITCH
		/* MOD (font): the row a screen copies in pieces (TEXTBOX_ROW_PIECES) */
		/* MOD (language): the row on either page (a box that flips writes its back page's rows after its front's) */
		cellsRow = boxId + 1 == TEXTBOX_PIECES_BOX &&
		           (box->vramRows != 0 ? box->writeRow % box->vramRows : box->writeRow) == TEXTBOX_PIECES_ROW;
		DRAW2_PIECES = FONT_PROPORTIONAL && cellsRow ? TEXTBOX_PIECES : NULL;
		/* MOD (language): with the original font that row is drawn in the original's cells, never in the name
		 * keyboard's 12-pixel ones (its labels, NAMING_LABEL_PIECES) */
		cellsRow = cellsRow && !FONT_PROPORTIONAL;
		if (cellsRow) {
			fixed = DRAW_STRING2_IS_FIXED_WIDTH;
			DRAW_STRING2_IS_FIXED_WIDTH = 0;
			FONT_CELLS = 1;
		}
		done = drawString2(buf, px, row, flag);
		if (cellsRow) {
			DRAW_STRING2_IS_FIXED_WIDTH = fixed;
			FONT_CELLS = (TEXTBOX_CELLS >> boxId) & 1;
		}
#else
		done = drawString2(buf, px, row, flag);
#endif
#endif
		box->writeRow++;
		if (done != 0) {
			box->writeCount = 1;
			box->renderCount = 1;
			box->flipCount = 0;
			box->registered = 1;
			break;
		}
		row += 12;
		buf += 0x40;
		x--;
	}
#if VERSION_IS(US)
	FONT_CELLS = 0;
#endif
#if FONT_SWITCH
	DRAW2_PIECES = NULL;
#endif

	return 1;
}

// clang-format off
void clearTextboxLineCount(boxId)
	uint8_t boxId;
// clang-format on
{
	TextBoxData *entry;
	int32_t rows;

	entry = &TEXTBOX_DATA.box[boxId];

	rows = entry->vramRows;
	if (entry->doubleBuffered == 1) {
		rows <<= 1;
	}

	entry->vramRows = 0;
	entry->registered = 1;
	TEXTBOX_DATA.usedRows -= rows;
}

int32_t tickSelectionDialogue(void)
{
	if (flipIdleTextboxPage(0) != 0) {
		return 0xffff;
	}

	if (isXPressedAfterDialogue() == 0) {
		return 0xffff;
	}

	if (isKeyDown(CONFIRM_BUTTON)) {
		advanceTextbox(0);
		playSound(0, 3);

		return DIALOGUE_SELECTION.current;
	}

	if (isKeyDown(CANCEL_BUTTON)) {
		if (isTriggerSet(0x31) == 0 &&
		    (ACTIVE_INSTRUCTION == SCRIPT_OP_CALL_ROUTINE || DIALOGUE_SELECTION.endPointer != NULL)) { /* MOD (cancel) */
			if (ACTIVE_INSTRUCTION != SCRIPT_OP_CALL_ROUTINE) {
				advanceTextbox(0);
				SCRIPT_POINTER = DIALOGUE_SELECTION.endPointer;
				playSound(0, 4);

				return 0xffff;
			}

			advanceTextbox(0);
			DIALOGUE_SELECTION.current = DIALOGUE_SELECTION.count - 1;
			playSound(0, 4);

			return DIALOGUE_SELECTION.current;
		}

		playSound(0, 0xb);
	} else if (isKeyDown(0x1000)) {
		if (DIALOGUE_SELECTION.current == 0) {
			DIALOGUE_SELECTION.current = DIALOGUE_SELECTION.count - 1;
		} else {
			DIALOGUE_SELECTION.current -= 1;
		}

		playSound(0, 2);
	} else if (isKeyDown(0x4000)) {
		DIALOGUE_SELECTION.current += 1;
		if (DIALOGUE_SELECTION.current == DIALOGUE_SELECTION.count) {
			DIALOGUE_SELECTION.current = 0;
		}

		playSound(0, 2);
	}
	return 0xffff;
}

int32_t tickConfirmDialogue(void)
{
	if (flipIdleTextboxPage(0) != 0) {
		return 0;
	}

	if (TEXTBOX_CLOSE_MODE == 0) {
		MAIN_D_80134F94 = 1;
	}

	if (isXPressedAfterDialogue() == 0) {
		return 0;
	}

	switch (TEXTBOX_CLOSE_MODE) {
	case 0:
		if (isKeyDown(CONFIRM_BUTTON) == 0) {
			goto ret0;
		}

		if (UI_BOX_DATA[0].state != 1) {
			goto ret0;
		}

		if (TEXTBOX_DATA.box[0].doubleBuffered == 1) {
			advanceTextbox(0);
			playSound(0, 3);
			return 1;
		} else {
			playSound(0, 3);
			ACTIVE_INSTRUCTION = 0;
			return 1;
		}
	case 2:
		if (AUTOCLOSE_FRAMES != 0) {
			goto ret0;
		}
		/* fall through */
	case 1:
		if (TEXTBOX_DATA.box[0].doubleBuffered == 1) {
			advanceTextbox(0);
			return 1;
		} else {
			ACTIVE_INSTRUCTION = 0;
			return 1;
		}
	}
ret0:
	return 0;
}

// clang-format off
void renderDialogueSelectionCursor(x, y)
	int16_t x;
	int16_t y;
// clang-format on
{
	if (TEXTBOX_DATA.box[0].idle == 1) {
		return;
	}

#if !VERSION_IS(US)
	renderSelectionCursor(x - 1, (long)y + DIALOGUE_SELECTION.cursorOffsetY + DIALOGUE_SELECTION.current * 13,
	                      DIALOGUE_SELECTION.cursorWidth, 0xd, 6);
#else
	renderSelectionCursor(x - 1,
	                      y + DIALOGUE_SELECTION.cursorOffsetY + DIALOGUE_SELECTION.current * 13 - 2,
	                      DIALOGUE_SELECTION.cursorWidth, 0xd, 6);
#endif
}

void tickCustomSizedTextbox(void)
{
	if (flipIdleTextboxPage(0) != 0) {
		return;
	}

	if (!isXPressedAfterDialogue()) {
		return;
	}

	if (isKeyDown(CONFIRM_BUTTON)) {
		if (UI_BOX_DATA[0].state != 1) {
			return;
		}

		if (advanceTextbox(0) != 0) {
			triggerBoxCloseFlag(0);
		}

		playSound(0, 3);

		return;
	}

	if (!isKeyDown(CANCEL_BUTTON)) {
		return;
	}

	if (UI_BOX_DATA[0].state == 1) {
		while (TEXTBOX_DATA.box[0].pageReady == 0) {
			showTextbox(0, 0xff);
		}

		ACTIVE_INSTRUCTION = 0;
		TEXTBOX_DATA.box[0].idle = 1;
		triggerBoxCloseFlag(0);
		playSound(0, 4);
	}
}

void renderCustomSizedTextbox(void)
{
	TextBoxData *box;
	int16_t rowPx;
	int16_t x;
	int16_t pagePx;
	int16_t y;
	int32_t i;

	box = TEXTBOX_DATA.box;
	pagePx = box->vramRows * 12;
	rowPx = box->vramRow * 12;
	rowPx += (int16_t)(box->backPage * pagePx);
	x = UI_BOX_DATA[0].finalPos.x + 5;
	y = UI_BOX_DATA[0].finalPos.y + 4;

	for (i = 0; i < box->vramRows; i++, rowPx += 0xc, y += 0xd) {
		renderString(0, x, y, 0xfc, 0xc, 0, rowPx, 6, 1);
	}
}

void initializeTextbox(void)
{
	TextBoxData *box;
	int32_t i;

	for (i = 0, box = TEXTBOX_DATA.box; i < 6; i++, box++) {
		box->flags = 0;
		box->vramRows = 0;
		box->registered = 1;
	}

	TEXTBOX_DATA.usedRows = 0;
	DELAY_FRAMES = 0;
	AUTOCLOSE_FRAMES = 0;
	TEXTBOX_OPEN_TIMER = 0;
}

#if !VERSION_IS(US)
void tickTextboxHandling(void)
#else
void tickTextboxHandling(int32_t flag)
#endif
{
	RECT area;
	int32_t drew;
	int32_t x;
	int32_t clut;
	TextBoxData *box;
	int32_t i;
	uint8_t flags;
	uint8_t features;
	uint8_t color;

	drew = 0;
	if (ACTIVE_INSTRUCTION != 0xff) {
		for (i = 0, box = TEXTBOX_DATA.box; i < 6; i++, box++) {
			if (box->vramRows != 0) {
				if (box->registered == 1 && box->writeCount != box->renderCount) {
					if (box->doubleBuffered == 1) {
						box->writeRow = (box->backPage ^ 1) * box->vramRows;
						getVRAMModeCoords(box->vramMode, &x, &clut);
						area.x = x;
						area.y = (box->vramRow + box->writeRow) * 12;
						area.w = clut;
						area.h = box->vramRows * 12;
						clearTextSubArea(&area);
						box->registered = 0;
					} else {
						box->writeRow = 0;
						box->registered = 0;
					}
				}
				if (drew == 0 && box->registered == 0) {
#if !VERSION_IS(US)
					drew = drawTextboxStrings(i & 0xff);
#else
					drew = drawTextboxStrings(i & 0xff, flag);
#endif
				}
				flags = box->flags;
				if ((int16_t)(flags & 0xf) != UI_BOX_DATA[i].state) {
					if ((flags & 0xf) == 1) {
						if (box->registered == 1 && UI_BOX_DATA[i].state == 0) {
							features = (flags >> 4) & 3;
							if (features == 0) {
								color = 1;
							} else {
								color = 0;
							}
							if ((flags & 0x80) == 0) {
								createStaticUIBox((int16_t)i, color, features, &box->rect, box->tick, box->render);
							} else {
								createAnimatedUIBox((int16_t)i, color, features, &box->rect, &box->origin, box->tick, box->render);
							}
						}
					} else if (UI_BOX_DATA[i].state == 1) {
						closeTextbox(i & 0xff, 0);
					}
				}
			}
		}
		if (DELAY_FRAMES != 0) {
			DELAY_FRAMES--;
		}
		if (AUTOCLOSE_FRAMES != 0) {
			AUTOCLOSE_FRAMES--;
		}
		TEXTBOX_OPEN_TIMER++;
		MAIN_D_80134F94 = 0;
		return;
	}
	for (i = 0; i < 6; i++) {
		if (UI_BOX_DATA[i].state != 0) {
			break;
		}
		setTextColor(1);
		IS_SCRIPT_PAUSED = 1;
	}
}

void getVRAMModeCoords(int32_t mode, int32_t *outX, int32_t *outClut)
{
	if (mode == 0) {
		*outX = 0;
		*outClut = 0xff;
	} else if (mode == 1) {
		*outX = 0;
		*outClut = 0x7f;
	} else {
		*outX = 0x80;
		*outClut = 0x7f;
	}
}

// clang-format off
void closeTextbox(boxId, target)
	int16_t boxId;
	RECT *target;
// clang-format on
{
	if (UI_BOX_DATA[boxId].state != 0 && UI_BOX_DATA[boxId].state != 3) {
		if ((TEXTBOX_DATA.box[boxId].flags & 0x40) == 0) {
			removeStaticUIBox(boxId);
		} else {
			removeAnimatedUIBox(boxId, target);
		}

		clearTextboxLineCount(boxId);
	}
}

void closeAllTextboxes(void)
{
	int32_t i;
	uint8_t flags;

	for (i = 0; i < 6; i++) {
		flags = TEXTBOX_DATA.box[i].flags;
		if ((flags & 0xf) != 0) {
			if (flags & 0x40) {
				ACTIVE_INSTRUCTION = 0xff;
				IS_SCRIPT_PAUSED = 0;
			}

			closeBox(i & 0xff);
		}
	}
}

#if VERSION_IS(US)
/*
 * MOD (font): the boxes (bit = box id) whose rows a screen cuts into pieces at offsets made for the original
 * advances (the tournament calendar, the shop's bits box; with the original font also the card shop's confirm box):
 * their rows are drawn in the original's cells (font.c FONT_CELLS; with the original font a space advances as the
 * original's, drawString2). createTextbox clears the bit; the creator of such a box sets it.
 */
uint8_t TEXTBOX_CELLS;
#endif
#if FONT_SWITCH
void textboxRowPieces(int32_t boxId, int32_t row, const uint16_t *pieces)
{
	TEXTBOX_PIECES = pieces;
	TEXTBOX_PIECES_BOX = boxId + 1;
	TEXTBOX_PIECES_ROW = row;
}
#endif

// clang-format off
void createTextbox(boxId, flags, rect, origin, tick, render)
	uint8_t boxId;
	uint8_t flags;
	RECT *rect;
	RECT *origin;
	void *tick;
	void *render;
// clang-format on
{
	TextBoxData *entry;

	entry = &TEXTBOX_DATA.box[boxId];

	if ((entry->flags & 0xf) == 1) {
		closeTextbox(boxId, 0);
	}

	entry->flags = flags;
	entry->rect = *rect;
	entry->origin = *origin;
	entry->tick = tick;
	entry->render = render;
#if VERSION_IS(US)
	TEXTBOX_CELLS &= ~(1 << boxId); /* MOD (font): its rows are laid out to the pixel unless its creator says */
#endif
#if FONT_SWITCH
	if (TEXTBOX_PIECES_BOX == boxId + 1) {
		TEXTBOX_PIECES_BOX = 0;
	}
#endif

	setFreshDialogue();
}

// clang-format off
void triggerBoxCloseFlag(boxId)
	uint8_t boxId;
// clang-format on
{
	TextBoxData *e = &TEXTBOX_DATA.box[boxId];
	uint8_t val = e->flags;

	if ((val & 0xf) != 0) {
		val = val & 0xf0;
		e->flags = val;
	}
}

// clang-format off
void registerTextbox(boxId, row, rows, doubleBuffer, mode)
	uint8_t boxId;
	int32_t row;
	int32_t rows;
	uint8_t doubleBuffer;
	int32_t mode;
// clang-format on
{
	TextBoxData *entry;
	int32_t usedRows;
	RECT rect;
	int32_t vramX;
	int32_t vramW;

	entry = &TEXTBOX_DATA.box[boxId];
	entry->vramRow = row;
	entry->vramRows = rows;
	entry->writeRow = 0;
	entry->vramMode = mode;
	entry->backPage = 0;
	entry->doubleBuffered = doubleBuffer;
	entry->writeCount = 0;
	entry->renderCount = 0;
	entry->flipCount = 0;
	entry->registered = 1;
	entry->pageReady = 0;
	entry->idle = 1;

	usedRows = entry->vramRows;
	if (entry->doubleBuffered == 1) {
		usedRows = usedRows << 1;
	}

	TEXTBOX_DATA.usedRows += usedRows;
	setTextColor(1);
	getVRAMModeCoords(entry->vramMode, &vramX, &vramW);
	setRECT(&rect, vramX, entry->vramRow * 12, vramW, usedRows * 12);
	clearTextSubArea(&rect);
}

#define DRAW_STRING2_SPACE_WIDTH 8 /* MOD: advance of a space (81 40) */

#if VERSION_IS(US)
/*
 * MOD (item names): TEXT_FIT_TIGHT / TEXT_FIT_CONDENSED (font.h): the name after the code and its colour, up to the next
 * code, drawn with drawStringMode; returns the pen after it, on the next cell of the text area's 4-pixel grid (the
 * clears of the codes that follow work in whole cells and would erase the name's last columns).
 */
static int16_t drawString2Fit(uint8_t **str, int16_t x, int16_t y, int16_t pos, int32_t code)
{
	char run[24];
	uint8_t *s;
	int32_t n;

	s = *str;
	if (*s != 0xff) {
		setTextColor(*s);
	}
	s++;
	for (n = 0; *s >= 0x20 && n < 23; n++) {
		run[n] = *s++;
	}
	run[n] = '\0';
	*str = s;
	n = x + pos + drawStringMode(run, x + pos, y, code == TEXT_FIT_TIGHT ? FIT_TIGHT : FIT_CONDENSED);
	return ((n + 3) & ~3) - x;
}
#endif

/*
 * MOD (font): with a font placed to the pixel (FONT_PROPORTIONAL) a clear that starts at the pen (a tab, a fixed
 * position) begins at the next grid cell of the text area: the 4 pixels the pen is in hold the previous glyph's last
 * columns, which clearTextSubArea (x / 4) would erase. It ends on the cell after the stop: the next glyph is drawn after.
 */
#if FONT_SWITCH
static void clearFromPen(int32_t px, int32_t py, int32_t w)
{
	RECT rect;
	int32_t x0;

	x0 = (px + 3) & ~3;
	w = px + w - x0;
	if (w > 0) {
		setRECT(&rect, x0, py, w + 3, 0xc);
		clearTextSubArea(&rect);
	}
}
#define CLEAR_FROM_PEN(px, py, w)                    \
	do {                                         \
		if (FONT_PROPORTIONAL) {             \
			clearFromPen(px, py, w);     \
		} else {                             \
			setRECT(&rect, px, py, w, 0xc); \
			clearTextSubArea(&rect);     \
		}                                    \
	} while (0)
#else
#define CLEAR_FROM_PEN(px, py, w)                    \
	do {                                         \
		setRECT(&rect, px, py, w, 0xc);      \
		clearTextSubArea(&rect);             \
	} while (0)
#endif


#if FONT_SWITCH
#define DRAW2_PIECES_MAX 32 /* a box row: 31 units */
/*
 * MOD (font): a script row that a screen copies in pieces (TEXTBOX_ROW_PIECES): its glyphs laid out by font.c
 * drawGlyphPieces, each one's original cell what drawString2 advanced in the original (its width; a space: the width
 * of the glyph before it, 12 at the row's start). Such a row holds text and colour codes; returns as drawString2 does.
 */
static int32_t drawString2Pieces(uint8_t *str, int16_t x, int16_t y)
{
	uint16_t glyphs[DRAW2_PIECES_MAX];
	uint8_t cells[DRAW2_PIECES_MAX];
	uint8_t colors[DRAW2_PIECES_MAX]; /* MOD (language): a row's colour codes (the name keyboard's labels) */
	int32_t color;
	int32_t n;
	int32_t space;
	int32_t ret;
	uint16_t glyph;
	uint8_t ch;

	n = 0;
	space = 0xc;
	color = -1;
	for (;;) {
		ch = *str++;
		if (ch == 0 || ch == 0xd) {
			ret = ch == 0;
			break;
		}
		if (ch == 1) {
			color = *str++;
			setTextColor(color);
			continue;
		}
		if (ch < 0x20) {
			continue; /* no other code in such a row */
		}
		if (isAsciiEncoded((char *)&ch) != 0) {
			glyph = swapShortBytes(ch == '\'' ? 0x8175 : ch == '"' ? 0x8176 : convertAsciiToJis(ch));
		} else {
			glyph = ch + (*str++ << 8);
		}
		if (n == DRAW2_PIECES_MAX) {
			continue;
		}
		glyphs[n] = glyph;
		colors[n] = color;
		if (glyph == 0x4081) {
			cells[n] = space;
		} else {
			cells[n] = space = glyphCell(glyph);
		}
		n++;
	}
	PIECE_COLORS = color >= 0 ? colors : NULL;
	drawGlyphPieces(glyphs, cells, n, x, y, DRAW2_PIECES);
	PIECE_COLORS = NULL;
	return ret;
}
#endif

#if !VERSION_IS(US)
int32_t drawString2(uint8_t *str, int16_t x, int16_t y)
#else
int32_t drawString2(uint8_t *str, int16_t x, int16_t y, int32_t flag)
#endif
{
#if !VERSION_IS(US)
#ifdef __MWERKS__
	extern void drawGlyph();
#endif
#else
	int32_t y2;
#endif
	RECT rect;
	uint8_t ch;
	int16_t pos;
	int16_t rem;
#if VERSION_IS(US)
	uint16_t adv;
#endif
#if VERSION_IS(US)
	uint16_t cellsSpace;
#endif
	uint16_t glyph;

#if FONT_SWITCH
	if (DRAW2_PIECES != NULL) {
		return drawString2Pieces(str, x, y);
	}
#endif
	pos = 0;
#if VERSION_IS(US)
	cellsSpace = 0xc;
#endif
	for (;;) {
		ch = *str++;
		switch (ch) {
		case 0:
			return 1;
		case 3:
			str += 3;
			break;
		case 2:
			str++;
			break;
		case 1:
			ch = *str++;
			setTextColor(ch);
			break;
		case 0xc:
			str++;
			rem = pos / 12 % 8;
			if (rem != 0) {
				rem = (8 - rem) * 12;
				CLEAR_FROM_PEN(x + pos, y, rem);
				pos = pos + rem;
			}
			break;
#if VERSION_IS(US)
		case 0x16:
			str++;
			CLEAR_FROM_PEN(x + pos - 6, y, 0x69); /* from 6 px back: after a 0F half space, the pen of the text */
			pos = 0x69;
			break;
		case 0x17:
			str++;
			CLEAR_FROM_PEN(x + pos, y, 0x69);
			pos = 0x69;
			break;
		case 0x1b:
			str++;
			CLEAR_FROM_PEN(x + pos, y, 0xa0);
			pos = 0xa0;
			break;
		case 0x1c:
			str++;
			CLEAR_FROM_PEN(x + pos, y, 0x64);
			pos = 0x64;
			break;
		case 0x18:
			str++;
			CLEAR_FROM_PEN(x + pos, y, 0x60);
			pos = 0x60;
			break;
		case 0x19:
			str++;
			CLEAR_FROM_PEN(x + pos, y, 0x9c);
			pos = 0x9c;
			break;
		case 0x1a:
			str++;
			CLEAR_FROM_PEN(x + pos, y, 0xb4);
			pos = 0xb4;
			break;
		case TEXT_FIT_TIGHT: /* MOD (item names) */
		case TEXT_FIT_CONDENSED:
			pos = drawString2Fit(&str, x, y, pos, ch);
			break;
		case TEXT_COLUMN: /* MOD (item names): from the next cell to the column */
			rem = *str++;
			pos = ((x + pos + 3) & ~3) - x;
			if (rem > pos) {
				setRECT(&rect, x + pos, y, rem - pos, 0xc);
				clearTextSubArea(&rect);
			}
			pos = rem;
			break;
#endif
		case 0xe:
			str++;
			rem = pos / 12 % 0xb;
			if (rem != 0) {
				rem = (0xb - rem) * 12;
				CLEAR_FROM_PEN(x + pos, y, rem);
				pos = pos + rem;
			}
			break;
		case 0xf:
			str++;
			CLEAR_FROM_PEN(x + pos, y, 6);
			pos += 6;
			break;
		case 0xd:
			return 0;
		default:
#if !VERSION_IS(US)
			glyph = ch + (*str++ << 8);
#else
			if (isAsciiEncoded((char *)&ch) != 0) {
#if FONT_SWITCH
				/* MOD (font): ' and " as the font draws them (packed rows, showTextbox; drawString does the same) */
				glyph = swapShortBytes(ch == '\'' ? 0x8175 : ch == '"' ? 0x8176 : convertAsciiToJis(ch));
#else
				glyph = swapShortBytes(convertAsciiToJis(ch));
#endif
			} else {
				glyph = ch + (*str++ << 8);
			}
#endif

#if FONT_SWITCH
			/*
			 * MOD (font): a space is the font's blank glyph, drawn like any other: its cell keeps the previous
			 * letter's last columns and clears the rest, and it advances its own width. The name keyboard
			 * (DRAW_STRING2_IS_FIXED_WIDTH) keeps 12-pixel cells, each letter centred in its own.
			 */
			if (!FONT_PROPORTIONAL) {
				/* the original font: below */
			} else if (DRAW_STRING2_IS_FIXED_WIDTH != 0) {
				drawGlyph(glyph, x + pos + (0xc - glyphWidth(glyph)) / 2, y);
				adv = 0xc;
			} else {
				adv = drawGlyph(glyph, x + pos, y);
				if (FONT_CELLS != 0 && glyph == 0x4081) {
					/* as the original advanced a space in these rows: by the glyph before it (the calendar's
					 * grades: a blank cell after a grade or at the row's start is 12, like the cell) */
					adv = cellsSpace;
				} else {
					cellsSpace = adv;
				}
			}
			if (!FONT_PROPORTIONAL)
#endif
			if (glyph == 0x4081) {
				setRECT(&rect, x + pos, y, 0xc, 0xc);
				clearTextSubArea(&rect);
#if VERSION_IS(US)
				/*
				 * MOD: the original left adv at the previous glyph's
				 * width (12 after a capital, 5 after ", " and ". ",
				 * which looked glued). A fixed 8 px is what it gave
				 * after most lowercase letters; no original line then
				 * starts a glyph past TEXT_DRAW_LIMIT (docs/mods/font.md).
				 */
				adv = DRAW_STRING2_IS_FIXED_WIDTH != 0 ? 0xc : DRAW_STRING2_SPACE_WIDTH;
				/*
				 * MOD (font): but in the rows a screen cuts into pieces (TEXTBOX_IN_CELLS: the tournament
				 * calendar's columns, the shop's bits box, the card shop's answers) as the original advanced
				 * it, by the glyph before it, 12 at the row's start (a blank grade cell); the fixed 8 moved
				 * the calendar's columns ("9 1 01 11 21")
				 */
				if (FONT_CELLS != 0) {
					adv = cellsSpace;
				}
#endif
			} else {
#if !VERSION_IS(US)
				drawGlyph(glyph, x + pos, y);
#else
				y2 = y;
				adv = drawGlyph(glyph, x + pos, y2);
				if (DRAW_STRING2_IS_FIXED_WIDTH != 0) {
					adv = 0xc;
				}
				cellsSpace = adv;
#endif
			}

#if !VERSION_IS(US)
			pos += 0xc;
#else
			pos += adv;
#endif
			break;
		}
	}
}

// clang-format off
int32_t flipIdleTextboxPage(boxId)
	uint8_t boxId;
// clang-format on
{
	TextBoxData *entry;

	entry = &TEXTBOX_DATA.box[boxId];

	if (entry->doubleBuffered == 0) {
		return 0;
	}

	if (entry->registered == 0) {
		return 1;
	}

	if (entry->writeCount != entry->renderCount) {
		return 1;
	}

	if (entry->idle == 0) {
		return 0;
	}

	if (entry->renderCount == 0) {
		return 1;
	}

	entry->idle = 0;
	entry->backPage ^= 1;
	entry->flipCount++;

	if (entry->pageReady == 0) {
		ACTIVE_INSTRUCTION = 0;
	}

	TEXTBOX_OPEN_TIMER = 0;

	return 1;
}

void closeBox(int32_t boxId)
{
	triggerBoxCloseFlag(boxId);
	closeTextbox(boxId, 0);
}

int32_t tickBackgroundDialogue(void)
{
	TextBoxData *entry;

	entry = TEXTBOX_DATA.box;

	if (flipIdleTextboxPage(0) != 0) {
		return 0;
	}

	if (entry->renderCount == entry->flipCount) {
		entry->idle = 1;
		entry->pageReady = 0;
		return 1;
	}

	return 0;
}

// clang-format off
int32_t advanceTextbox(boxId)
	uint8_t boxId;
// clang-format on
{
	TextBoxData *entry;

	entry = &TEXTBOX_DATA.box[boxId];
	if (entry->registered == 0) {
		return 0;
	}

	if (entry->pageReady == 1) {
		if (entry->renderCount != entry->flipCount) {
			entry->backPage ^= 1;
		} else {
			entry->idle = 1;
			entry->pageReady = 0;
			ACTIVE_INSTRUCTION = 0;
			return 1;
		}
	} else {
		entry->backPage ^= 1;
		ACTIVE_INSTRUCTION = 0;
	}

	entry->flipCount++;
	TEXTBOX_OPEN_TIMER = 0;

	return 0;
}

void setupDialogueBox(uint8_t owner)
{
	RECT rect;
	RECT origin;
	uint8_t flags;

	flags = 0x21;
	if (setupBoxOrigin(owner, &origin) != 0) {
		flags |= 0x80;
	}

	setRECT(&rect, -0x82, 0x2a, 0x106, 0x3b);
	createTextbox(0, flags, &rect, &origin, tickScriptDialogueBox,
	              renderScriptDialogueBox);
	registerTextbox(0, 0, 4, 1, 0);
}

int32_t setupBoxOrigin(int32_t ownerId, RECT *origin)
{
	DVECTOR pos;

	if (ownerId == 0xff) {
		return 0;
	}

	if (TEXTBOX_ORIGIN_X != -0x270f) {
		worldPosToScreenPos2(&TEXTBOX_ORIGIN_X, &TEXTBOX_ORIGIN_Y,
		                     &TEXTBOX_ORIGIN_Z);
		pos.vx = TEXTBOX_ORIGIN_X - 5;
		pos.vy = TEXTBOX_ORIGIN_Y - 5;
		TEXTBOX_ORIGIN_X = -0x270f;
	} else {
		int32_t entityId = scriptIdToEntityId(ownerId) & 0xff;
		if (entityId == 0xff) {
			return 0;
		}

		getEntityScreenPos(ENTITY_TABLE[entityId], 1, &pos);
	}

	setRECT(origin, pos.vx, pos.vy, 0xa, 0xa);

	return 1;
}

void tickScriptDialogueBox(void)
{
	if (ACTIVE_INSTRUCTION == SCRIPT_OP_SET_SELECTION) {
		if (tickSelectionDialogue() != 0xffff) {
			SCRIPT_POINTER = DIALOGUE_SELECTION.pointer + DIALOGUE_SELECTION.current * 2;
			SCRIPT_POINTER =
				(uint8_t *)((uint32_t)ACTIVE_SCRIPT +
			                    *(uint16_t *)SCRIPT_POINTER);
		}
	} else if (ACTIVE_INSTRUCTION == SCRIPT_OP_SHOW_TEXTBOX) {
		tickConfirmDialogue();
	} else if (ACTIVE_INSTRUCTION == SCRIPT_OP_CALL_ROUTINE) {
		switch (SCRIPT_TEXTBOX_MODE) {
		case 0:
			break;
		case 1:
			if (tickConfirmDialogue() == 1) {
				SCRIPT_STATE_2 = SCRIPT_NEXT_STATE_2;
				ACTIVE_INSTRUCTION = SCRIPT_OP_CALL_ROUTINE;
				SCRIPT_TEXTBOX_MODE = 0;
			}
			break;
		case 2:
			if (tickSelectionDialogue() != 0xffff) {
				SCRIPT_STATE_2 = (long)SCRIPT_NEXT_STATE_2 + DIALOGUE_SELECTION.current;
				ACTIVE_INSTRUCTION = SCRIPT_OP_CALL_ROUTINE;
				SCRIPT_TEXTBOX_MODE = 0;
			}
			break;
		case 3:
			if (tickBackgroundDialogue() == 1) {
				ACTIVE_INSTRUCTION = SCRIPT_OP_CALL_ROUTINE;
				SCRIPT_TEXTBOX_MODE = 0;
			}
			break;
		}
	}
}

void renderUIBox(int32_t boxId)
{
	POLY_FT4 poly;
	int16_t x;
	int16_t y;
	int16_t u;

	x = UI_BOX_DATA[boxId].finalPos.x + 0xef;
	y = UI_BOX_DATA[boxId].finalPos.y + 0x2b;
	u = TEXTBOX_OPEN_TIMER / 4 % 3 * 16 + 0x4c;
	SetPolyFT4(&poly);
	SetSemiTrans(&poly, 1);
	poly.tpage = GetTPage(0, 1, 320, 128);
	poly.clut = modButtonClut(); /* MOD (options): GetClut(96, 500), the ○ in red */
	setRGB0(&poly, 0x80, 0x80, 0x80);
	setUVDataPolyFT4(&poly, u, 0xb7, 0x10, 0xc);
	setPosDataPolyFT4(&poly, x, y, 0x10, 0xc);

	GsSortPoly(&poly, ACTIVE_ORDERING_TABLE, (6 - boxId));
}

void scriptShowSelection(void)
{
	uint8_t optionCount;
#if VERSION_IS(US)
	uint16_t height;
#endif

	pollNextScriptUByte(&optionCount);

	DIALOGUE_SELECTION.count = optionCount;
	DIALOGUE_SELECTION.current = 0;
	DIALOGUE_SELECTION.pointer = SCRIPT_POINTER;
	SCRIPT_POINTER = SCRIPT_POINTER + (optionCount + 1) * 2;
	DIALOGUE_SELECTION.cursorWidth = showTextbox(0, CURRENT_DIALOGUE_OWNER);
	DIALOGUE_SELECTION.cursorWidth = DIALOGUE_SELECTION.cursorWidth * 12 + 2;
#if VERSION_IS(US)
	height = DIALOGUE_SELECTION.cursorWidth;

	if (height > 0xf0) {
		DIALOGUE_SELECTION.cursorWidth = 0xf0;
	}
#endif

	/* MOD (cancel): the original skipped 2 bytes as the END_SELECTION (12 00) every selection was thought to end with; 23
	 * of the disc's have none, so △ ran their next instruction from its third byte (Piximon's Buy, Meteormon,
	 * WaruMonzaemon...). Without the marker there is no cancel path: △ is refused as under trigger 49. */
	if (SCRIPT_POINTER[0] == 0x12 && SCRIPT_POINTER[1] == 0) {
		SCRIPT_POINTER += 2;
		DIALOGUE_SELECTION.endPointer = SCRIPT_POINTER;
	} else {
		DIALOGUE_SELECTION.endPointer = NULL;
	}

	if (CURRENT_DIALOGUE_OWNER != 0xff) {
		DIALOGUE_SELECTION.cursorOffsetY = 0xd;
	} else {
		DIALOGUE_SELECTION.cursorOffsetY = 0;
	}

	ACTIVE_INSTRUCTION = SCRIPT_OP_SET_SELECTION;
}

GARBAGE(showTextbox, 1);

#if VERSION_IS(US)
/* MOD (font): the one-byte form of a two-byte glyph code in a script row, 0 if it keeps two bytes (drawString2 maps it
 * back: letters and digits by convertAsciiToJis, ' and " to the codes the font draws them at) */
static const uint16_t PACKED_CODES[] = { 0x8140, 0x8142, 0x8143, 0x8146, 0x8147, 0x8148, 0x8149, 0x8175, 0x8176,
					  0x817b, 0x817c, 0x8181 };
static const char PACKED_ASCII[] = " .,:;?!'\"+-=";

#if FONT_SWITCH
static int32_t packedAscii(uint32_t code)
{
	int32_t i;

	if (code >= 0x8260 && code <= 0x8279) {
		return code - 0x8260 + 'A';
	}
	if (code >= 0x8281 && code <= 0x829a) {
		return code - 0x8281 + 'a';
	}
	if (code >= 0x824f && code <= 0x8258) {
		return code - 0x824f + '0';
	}
	for (i = 0; i < (int32_t)(sizeof(PACKED_CODES) / sizeof(PACKED_CODES[0])); i++) {
		if (code == PACKED_CODES[i]) {
			return PACKED_ASCII[i];
		}
	}
	return 0;
}
#endif

/* MOD (script text): a script may store a letter, digit, space or one of ".,:;?!'\"+-=" as one byte, its ASCII form
 * (tools/dws/bytecode/__main__.py --pack-ascii; the original's text has no unit starting in 0x20..0x7e, so scripts without it read the
 * same). This is the two-byte glyph code that byte stands for, 0 if the byte is none of those */
static uint32_t scriptAsciiCode(uint32_t c)
{
	int32_t i;

	if (c >= 'A' && c <= 'Z') {
		return c - 'A' + 0x8260;
	}
	if (c >= 'a' && c <= 'z') {
		return c - 'a' + 0x8281;
	}
	if (c >= '0' && c <= '9') {
		return c - '0' + 0x824f;
	}
	for (i = 0; i < (int32_t)(sizeof(PACKED_CODES) / sizeof(PACKED_CODES[0])); i++) {
		if (c == (uint32_t)PACKED_ASCII[i]) {
			return PACKED_CODES[i];
		}
	}
	return 0;
}
#endif

uint16_t showTextbox(uint8_t boxId, uint8_t speakerId)
{
	TextBoxData *entry;
	uint8_t *base;
	uint8_t *out;
	uint16_t col;
	uint16_t maxCol;
	uint32_t row;
	int32_t lines;
	uint8_t ctrl;

	entry = &TEXTBOX_DATA.box[boxId];
	base = TEXTBOX_LINES_PTR + (entry->vramRow << 6);
	if (entry->vramMode == 2) {
		base += 0x20;
	}
	if (entry->doubleBuffered == 1) {
		base = (uint8_t *)(base + (((entry->backPage ^ 1) * entry->vramRows) << 6));
	}
	out = base;
	col = maxCol = 0;
	row = 0;
	if (speakerId != 0xff) {
		*out++ = 1;
		if (speakerId == 0xfd) {
			*out++ = 6;
		} else if (speakerId == 0xfc) {
			*out++ = 0xa;
		} else if (speakerId >= 0xc8) {
			*out++ = 5;
		} else {
			*out++ = 7;
		}
		lines = getSpeakerName(speakerId, out);
		out += lines;
		*out++ = 1;
		*out++ = 1;
		*out++ = 0xd;
		*out = 0;
		row++;
		out = base + (row << 6);
	}
top: {
	ctrl = *SCRIPT_POINTER++;
	{
		switch (ctrl) {
		case 3:
			*out++ = ctrl;
			*out++ = *SCRIPT_POINTER++;
			*out++ = *SCRIPT_POINTER++;
			*out++ = *SCRIPT_POINTER++;
			goto top;
		case 4:
			ctrl = *SCRIPT_POINTER++;
			ctrl = readPStat(ctrl);
			out = intToStringSJIS(out, ctrl, 3, 1);
			goto top;
		case 5:
			SCRIPT_POINTER++;
			lines = getSpeakerName(0xfd, out);
			out += lines;
			col += (uint16_t)(lines >> 1);
			goto top;
		case 6:
			SCRIPT_POINTER++;
			lines = getSpeakerName(0xfc, out);
			out += lines;
			col += (uint16_t)(lines >> 1);
			goto top;
		case 7:
			ctrl = *SCRIPT_POINTER++;
			{
				ctrl = readPStat(ctrl);
				strcpy(out,
				       (const char *)&DIGI_PARA(ctrl));
				lines = strlen(
					(const char *)&DIGI_PARA(ctrl));
				out += lines;
				col += (uint16_t)(lines >> 1);
			}
			goto top;
		case 8:
			ctrl = *SCRIPT_POINTER++;
			{
				ctrl = readPStat(ctrl);
				strcpy(out, MOVE_NAMES[ctrl]);
				lines = strlen(MOVE_NAMES[ctrl]);
				out += lines;
				col += (uint16_t)(lines >> 1);
			}
			goto top;
		case 9:
			ctrl = *SCRIPT_POINTER++;
			{
				ctrl = readPStat(ctrl);
				strcpy(out,
				       (const char *)&ITEM_PARA[ctrl]);
				lines = strlen(
					(const char *)&ITEM_PARA[ctrl]);
				out += lines;
				col += (uint16_t)(lines >> 1);
			}
			goto top;
		case 10:
			SCRIPT_POINTER++;
			out = intToStringSJIS(out, MONEY, 6, 0);
			goto top;
		case 11:
			SCRIPT_POINTER++;
			out = intToStringSJIS(out, MERIT, 4, 0);
			goto top;
		case 16:
			SCRIPT_POINTER++;
			out = intToStringSJIS(out, SHOP_VARIABLE, 5, 1);
			goto top;
		case 19:
			SCRIPT_POINTER++;
			out = intToStringSJIS(out, TOURNAMENTS_WON, 3, 1);
			goto top;
		case 20:
			SCRIPT_POINTER++;
			out = intToStringSJIS(out, TOURNAMENT_WINS, 3, 1);
			goto top;
		case 21:
			SCRIPT_POINTER++;
			out = intToStringSJIS(out, TOURNAMENTS_LOST, 3, 1);
			goto top;
		case 17:
			ctrl = *SCRIPT_POINTER++;
			{
				ctrl = readPStat(ctrl);
				strcpy(out, BGM_TRACK_NAMES[ctrl]);
				lines = strlen(BGM_TRACK_NAMES[ctrl]);
				out += lines;
				col += (uint16_t)(lines >> 1);
			}
			goto top;
		case 18:
			ctrl = *SCRIPT_POINTER++;
			{
				ctrl = readPStat(ctrl);
				strcpy(out, TOURNAMENT_NAMES[ctrl]);
				lines = strlen(TOURNAMENT_NAMES[ctrl]);
				out += lines;
				col += (uint16_t)(lines >> 1);
			}
			goto top;
		case 12:
			*out++ = ctrl;
			*out++ = *SCRIPT_POINTER++;
			goto top;
		case 14:
			*out++ = ctrl;
			*out++ = *SCRIPT_POINTER++;
			goto top;
		case 13:
			*out++ = ctrl;
			*out++ = *SCRIPT_POINTER++;
			if (col > maxCol) {
				maxCol = col;
			}
			col = 0;
			ctrl = *SCRIPT_POINTER;
			if (ctrl == 0) {
				SCRIPT_POINTER += 2;
				out -= 2;
				*out = ctrl;
				ctrl = *SCRIPT_POINTER;
				if (ctrl == SCRIPT_OP_SHOW_TEXTBOX) {
					goto done;
				}
				entry->pageReady = 1;
				goto done;
			}
			row++;
			out = base + (row << 6);
			goto top;
		case 1:
		case 2:
			*out++ = ctrl;
			*out++ = *SCRIPT_POINTER++;
			goto top;
		case 15:
		default:
#if VERSION_IS(US)
			/* MOD (script text): a one-byte unit is expanded to the form the box draws: its own byte in a packed row,
			 * else the two-byte glyph code */
			if (ctrl >= 0x20 && ctrl < 0x7f && (lines = scriptAsciiCode(ctrl)) != 0) {
#if FONT_SWITCH
				if (boxId == 0) {
					*out++ = ctrl;
				} else
#endif
				{
					*out++ = lines >> 8;
					*out++ = lines & 0xff;
				}
				col++;
				goto top;
			}
#endif
#if FONT_SWITCH
			/* MOD (font): a letter, digit, space or common punctuation of a dialogue or selection row (box 0) takes one
			 * byte, its ASCII form, which drawString2 draws as the same glyph: a 64-byte row holds 63 of them */
			if (boxId == 0 && (lines = packedAscii((ctrl << 8) | *SCRIPT_POINTER)) != 0) {
				*out++ = lines;
				SCRIPT_POINTER++;
				col++;
				goto top;
			}
#endif
			*out++ = ctrl;
			*out++ = *SCRIPT_POINTER++;
			col++;
			goto top;
		}
	}
}
done:
	entry->writeCount++;
	ACTIVE_INSTRUCTION = SCRIPT_OP_SHOW_TEXTBOX;

	return maxCol;
}

void scriptSetTextboxSize(void)
{
	uint8_t originId;
	uint8_t cols;
	uint8_t rows;
	uint8_t boxFlags;
	RECT rect1;
	RECT origin;
	uint16_t width;
	uint16_t height;

	pollNextScriptUByte(&originId);
	pollNextTwoScriptBytes(&cols, &rows);

	boxFlags = 0x21;
	if (setupBoxOrigin(originId, &origin) != 0) {
		boxFlags |= 0x80;
	}

	width = cols * 12 + 10;
	height = rows * 13 + 7;
	setRECT(&rect1, (0x140 - width) / 2 - 0xa0, (0xf0 - height) / 2 - 0x78, width, height);
	createTextbox(0, boxFlags, &rect1, &origin, tickCustomSizedTextbox,
	              renderCustomSizedTextbox);
	registerTextbox(0, 0, rows, 1, 0);
}

uint16_t showTextboxReady(uint8_t boxId, uint8_t speakerId)
{
	TEXTBOX_DATA.box[boxId].pageReady = 1;

	return showTextbox(boxId, speakerId);
}
