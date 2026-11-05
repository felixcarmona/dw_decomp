/*
 * MOD (training block): a gym's training menu has a "Normal training x5" row (story/base: the script sets the Bonus
 * Try flag, pstat 246, to 2 for it; never with Bonus Try, whose reels are played). In a block the animation is
 * the first session's; then the gym's own award function runs once per session, each with its hour of game time
 * and its care costs, the gains of one added to the stats before the next (so a brains mark is crossed as it would
 * be), and the block stops after the session that left the partner hungry, needing the toilet, sleepy, tired, hurt
 * or sick, due to digivolve, at the end of its life, or that taught it a technique or new orders (the gym's box).
 * The stats box then shows the block's total, and a box under it how many sessions were done and why it stopped;
 * the field answers the need as after one session.
 *
 * Included by trn_reward.c (TRN) and trn2_hud.c (TRN2) with:
 *   TB(name)       the overlay's name of a function or variable (TRN_name, TRN2_name)
 *   TB_SLOT        its Bonus Try flag (pstat 0xf6, read by the setup)
 *   TB_RULES       its session rules (MOD_TRAIN_RULES, MOD_TRAIN2_RULES)
 *   TB_START_MIN   the minute of the day its session started at (TB(saveTrainingStartTime))
 */
/* the TRN_BLOCK_* texts: in the including file's text (text/trn/trn_reward.yaml, text/trn2/trn2_hud.yaml) */

#include <libgpu.h>
#include <libgs.h>
#include <dw/mod_qol.h>
#include <dw/ui.h>

extern int16_t INITIAL_COMBAT_STATS[][6];
extern int16_t STATS_GAINS[6];
void clearTextSubArea(RECT *rect);
void createMenuBox(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, void (*tick)(void), void (*render)(void));
void renderString(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h, int32_t i);

static TRN_BLOCK_DONE_TEXT
static TRN_BLOCK_HUNGRY_TEXT
static TRN_BLOCK_TOILET_TEXT
static TRN_BLOCK_SLEEPY_TEXT
static TRN_BLOCK_TIRED_TEXT
static TRN_BLOCK_UNWELL_TEXT
static TRN_BLOCK_EVOLVE_TEXT
static TRN_BLOCK_LIFE_TEXT

/* by MOD_TB_* (dw/mod_qol.h) */
static char *const TB_REASONS[] = {
	NULL, TRN_BLOCK_HUNGRY, TRN_BLOCK_TOILET, TRN_BLOCK_SLEEPY, TRN_BLOCK_TIRED,
	TRN_BLOCK_UNWELL, TRN_BLOCK_EVOLVE, TRN_BLOCK_LIFE,
};

int8_t TB(MOD_QUIET); /* the stats box waits for the block's last session */
static int8_t TB_x5; /* the menu's x5 row */
static int8_t TB_count;
static int8_t TB_done;
static int8_t TB_reason;
static int8_t TB_box1Seen;
static int8_t TB_box2Before; /* box 2 was open when the block began */
static int16_t TB_pre[6];
static int16_t TB_sum[6];
static char TB_doneText[64];
static int8_t TB_box2Dy; /* box 2 under the stats box: lower by the skip hint's row when the box has one */

/* MOD (skip hint, patch "Show how to skip the gym's and the battle's tallies"): the stats box grows by a row
   (MOD_COUNT_ROW_H) when a gain counts, so box 2 (the block's box, and in TRN the gym's technique and new orders
   boxes) opens that much lower, under it, with its text (TB_box2Dy). TB_box2Y: box 2's y for the gains in
   STATS_GAINS, by the gym's own check (TB(statGainsAreZero)) */
int32_t TB(statGainsAreZero)(void);
static int32_t TB_box2Y(void)
{
	TB_box2Dy = TB(statGainsAreZero)() ? 0 : MOD_COUNT_ROW_H;
	return 0x12 + TB_box2Dy;
}

/* from the session's setup (TB(saveTrainingStartTime)), after it read the flag: the menu's x5 row is a normal session
   in a block (not a block's next hour, which arms again) */
void TB(modBlockArm)(void)
{
	if (!TB(MOD_QUIET)) {
		TB_x5 = TB_SLOT == 2;
		if (TB_x5) {
			TB_SLOT = 0;
		}
	}
}

/* the box under the stats box closes with it */
static void TB_tickMessage(void)
{
	if (UI_BOX_DATA[1].state == 1) {
		TB_box1Seen = 1;
	} else if (TB_box1Seen) {
		TB_box1Seen = 0;
		removeAnimatedUIBox(2, NULL);
	}
}

static void TB_renderMessage(void)
{
	renderString(0, -78, 24 + TB_box2Dy, 156, 12, 0, 120, 4, 0);
	renderString(0, -78, 36 + TB_box2Dy, 156, 12, 0, 132, 4, 0);
}

/* before the gym's award function */
void TB(modBlockBegin)(void)
{
	int32_t i;

	TB_count = TB_x5 ? 5 : 1;
	TB_done = 0;
	TB_reason = MOD_TB_GO;
	TB_box2Before = UI_BOX_DATA[2].state != 0;
	TB(MOD_QUIET) = TB_count > 1;
	if (TB_count > 1) {
		TB(saveBaseStats)();
		for (i = 0; i < 6; i++) {
			TB_pre[i] = INITIAL_COMBAT_STATS[0][i];
			TB_sum[i] = 0;
		}
	}
}

/* after it: 1 to award the next session of the block */
int32_t TB(modBlockNext)(void)
{
	RECT area;
	int32_t i;
	int32_t v;
	char *s;
	char *d;

	if (TB_count <= 1) {
		return 0;
	}
	/* the session's gains go into the stats the next one starts from */
	for (i = 0; i < 6; i++) {
		v = INITIAL_COMBAT_STATS[0][i] + STATS_GAINS[i];
		if (v > CAP_OF_STAT_INDEX(i)) {
			v = CAP_OF_STAT_INDEX(i);
		}
		TB_sum[i] += v - INITIAL_COMBAT_STATS[0][i];
		INITIAL_COMBAT_STATS[0][i] = v;
		STATS_GAINS[i] = 0;
	}
	TB(applyBaseStats)();
	TB_done++;
	/* the gym's box (a technique, new orders) opened by this session: a box 2 open before the block (the Bonus
	   Try's reels, a block's own box closing) is not one */
	if (!TB_box2Before && UI_BOX_DATA[2].state != 0) {
		TB_reason = MOD_TB_LEARNED;
	} else {
		TB_reason = modTrainStopReason(TB_START_MIN, TB_RULES[MOD_TR_TIRED_AT]);
	}
	if (TB_reason == MOD_TB_GO && TB_done < TB_count) {
		TB(saveTrainingStartTime)(); /* the next session's hour */
		return 1;
	}

	/* the stats box adds the block's total to the stats from before it */
	for (i = 0; i < 6; i++) {
		INITIAL_COMBAT_STATS[0][i] = TB_pre[i];
		STATS_GAINS[i] = TB_sum[i];
	}
	TB(applyBaseStats)();
	TB(MOD_QUIET) = 0;
	if (TB_reason == MOD_TB_LEARNED) {
		/* the gym's box drew its text in rows 120..155: only the stats box's rows are cleared */
		setRECT(&area, 0, 0, 256, 120);
		clearTextSubArea(&area);
		setRECT(&area, 0, 240, 256, 12);
		clearTextSubArea(&area);
	} else {
		clearTextArea();
	}
	TB(modOpenStatsBox)();
	if (TB_reason != MOD_TB_GO && TB_reason != MOD_TB_LEARNED) {
		/* the first one-byte 0 (not the second byte of a two-byte glyph) is the number */
		s = TRN_BLOCK_DONE;
		d = TB_doneText;
		v = 0;
		while (*s != 0 && d < TB_doneText + sizeof(TB_doneText) - 1) {
			if (*s == '0' && v == 0 && (s == TRN_BLOCK_DONE || (uint8_t)s[-1] < 0x80)) {
				*d++ = (char)('0' + TB_done);
				v = 1;
			} else {
				*d++ = *s;
			}
			s++;
		}
		*d = 0;
		setTextColor(1);
		drawString(TB_doneText, 0, 120);
		drawString(TB_REASONS[TB_reason], 0, 132);
		TB_box1Seen = 0;
		createMenuBox(2, -0x58, TB_box2Y(), 0xb0, 0x23, 2, TB_tickMessage, TB_renderMessage);
	}
	return 0;
}
