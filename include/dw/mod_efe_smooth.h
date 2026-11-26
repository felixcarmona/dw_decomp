#ifndef DW_MOD_EFE_SMOOTH_H
#define DW_MOD_EFE_SMOOTH_H

/*
 * MOD (smooth effects, docs/reference/frame-rate.md): the battle effects at the frame rate the game renders at.
 *
 * The EFE engine moves its effects once per 20 Hz tick; at 30 or 60 fps every pass of a tick drew the same pose. Each
 * draw sub-opcode of a render script now goes through modSmoothCall: on a pass that shows a fraction of the tick
 * (MOD_RENDER_ALPHA < 4096) it draws its operands (positions, scales, colours, angles, screen coordinates, and the
 * instance transform for the model draws) between the values the same draw had one tick earlier and its own. Nothing
 * persistent is written: the operands are rewritten on the script stack (pointers swapped for local copies) and the
 * instance transform is put back after the draw, so ticks, hits, damage and sounds keep their timing.
 *
 * A draw is the same draw one tick later when it has the same slot, sub-effect and order inside that sub-effect's render
 * script, the same opcode, script, instance, pointer operands and identifiers (sprite cells, models, flags), and its
 * instance is exactly one frame further. Everything else draws its own pose: spawns, flipbook switches, a restarted
 * sub-effect, and any value that jumps further than its kind allows in one tick (a pose cut). Values that already
 * differ between the passes of a tick (read from the interpolated bones or camera) are taken as they are.
 *
 * Included once by each battle engine (BTL, STD, VS): all of it is overlay code and data.
 */

#include <dw/efe.h>
#include <dw/mod_fps.h>

extern int32_t MOD_RENDER_PASS;
extern int32_t MOD_RENDER_ALPHA;
extern int32_t MOD_RENDER_CADENCE;
extern int8_t MOD_TICK_FAST;
extern int32_t MOD_FPS_TICKS;
extern int32_t MOD_INTERP_STATS[7];

/* operand kinds: low 3 bits, bit 3 = a pointer to three int32 of that kind */
#define MSM_CUR 0 /* the current value */
#define MSM_ID 1  /* must match */
#define MSM_POS 2 /* world position */
#define MSM_PIX 3 /* screen coordinate */
#define MSM_COL 4 /* colour component */
#define MSM_ANG 5 /* angle, 4096 a turn */
#define MSM_DEG 6 /* angle in degrees */
#define MSM_SCL 7 /* scale, 4096 = 1 */
#define MSM_P 8

#define MSM_T 0x80 /* the draw reads the instance's position and rotation too */

#define MSM_WORDS 768 /* per tick log: Pepper Breath peaks at 284 */
#define MSM_VALS 24 /* values of one draw, at most */
#define MSM_PTRS 5  /* pointer operands of one draw, at most */

#define MSM_K(a, b) ((a) | ((b) << 4))

/* opcode, operand count (| MSM_T), the operands' kinds two per byte, bottom of the stack first */
static const uint8_t MOD_SMOOTH_DRAWS[] = {
	/* drawTMD, drawTMDYXZ, drawTMDScreenSpace: model id, scale */
	3, 2 | MSM_T, MSM_K(MSM_ID, MSM_P | MSM_SCL),
	77, 2 | MSM_T, MSM_K(MSM_ID, MSM_P | MSM_SCL),
	79, 2 | MSM_T, MSM_K(MSM_ID, MSM_P | MSM_SCL),
	/* renderCenteredSprite: position, colour, scale x y, cell u v w h */
	4, 8, MSM_K(MSM_P | MSM_POS, MSM_P | MSM_COL), MSM_K(MSM_SCL, MSM_SCL), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID),
	/* renderProjectedSprite: position, colour, scale x y, u v w h mx my, rotate, attribute, clut row */
	24, 13, MSM_K(MSM_P | MSM_POS, MSM_P | MSM_COL), MSM_K(MSM_SCL, MSM_SCL), MSM_K(MSM_ID, MSM_ID),
	MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_DEG, MSM_ID), MSM_K(MSM_ID, 0),
	/* renderScreenSprite: x y, u v w h mx my, rotate, depth, scale x y, flip */
	20, 13, MSM_K(MSM_PIX, MSM_PIX), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID),
	MSM_K(MSM_DEG, MSM_CUR), MSM_K(MSM_SCL, MSM_SCL), MSM_K(MSM_ID, 0),
	/* renderParticleFlashSprite: x y, u v, size x y, scale, depth, clut */
	22, 9, MSM_K(MSM_PIX, MSM_PIX), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_SCL, MSM_CUR),
	MSM_K(MSM_ID, 0),
	/* renderEFELine: four screen coordinates, colour, depth, flags */
	60, 7, MSM_K(MSM_PIX, MSM_PIX), MSM_K(MSM_PIX, MSM_PIX), MSM_K(MSM_P | MSM_COL, MSM_CUR), MSM_K(MSM_ID, 0),
	/* render2DTexturedQuad: x y w h, depth, colour, u v du dv, clut row, semi (corner pointers when < 0: drawn as is) */
	65, 12, MSM_K(MSM_PIX, MSM_PIX), MSM_K(MSM_PIX, MSM_PIX), MSM_K(MSM_CUR, MSM_P | MSM_COL), MSM_K(MSM_ID, MSM_ID),
	MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID),
	/* render3DTexturedQuad: four corners, colour, u v du dv, clut row, semi */
	70, 11, MSM_K(MSM_P | MSM_POS, MSM_P | MSM_POS), MSM_K(MSM_P | MSM_POS, MSM_P | MSM_POS),
	MSM_K(MSM_P | MSM_COL, MSM_ID), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, 0),
	/* renderWireframeGrid: base, rotation, colour, x1 x2, n1, z1 z2, n2, y */
	66, 10, MSM_K(MSM_P | MSM_POS, MSM_P | MSM_ANG), MSM_K(MSM_P | MSM_COL, MSM_POS), MSM_K(MSM_POS, MSM_ID),
	MSM_K(MSM_POS, MSM_POS), MSM_K(MSM_ID, MSM_POS),
	/* renderWireframeBox: base, extent, colour */
	68, 3 | MSM_T, MSM_K(MSM_P | MSM_POS, MSM_P | MSM_POS), MSM_K(MSM_P | MSM_COL, 0),
	/* renderRadialWaves: scale */
	82, 1 | MSM_T, MSM_K(MSM_P | MSM_SCL, 0),
	/* renderRibbonStrip: points, scale, width */
	85, 3 | MSM_T, MSM_K(MSM_ID, MSM_P | MSM_SCL), MSM_K(MSM_POS, 0),
	/* renderRingTube: centre, then arrays and counts */
	86, 6, MSM_K(MSM_P | MSM_POS, MSM_ID), MSM_K(MSM_ID, MSM_ID), MSM_K(MSM_ID, MSM_ID),
	/* renderScreenOverlay: layer, colour, mode */
	87, 3, MSM_K(MSM_ID, MSM_P | MSM_COL), MSM_K(MSM_ID, 0),
	/* renderScreenFade: level */
	92, 1, MSM_K(MSM_COL, 0),
	/* renderParallaxSprites: sprites, offset x y */
	31, 3, MSM_K(MSM_ID, MSM_PIX), MSM_K(MSM_PIX, 0),
	0,
};

/* the largest change one tick may make and still be motion, per kind */
static const int16_t MOD_SMOOTH_CUT[8] = {0, 0, 2048, 160, 64, 1024, 90, 4096};

typedef struct {
	int32_t *cur;	/* this tick's draws: key, hash, frame | count << 6 | live << 11, raw values */
	int32_t *prev;	/* the previous tick's */
	int32_t curN;
	int32_t prevN;
	int32_t tick;	/* MOD_FPS_TICKS of the last logged first pass */
	int32_t snaps;	/* MOD_INTERP_STATS[3] then */
	int8_t on;	/* this pass logs and looks up */
	int8_t valid;	/* the previous log is the tick before, no cut between */
	int8_t emitTicked;
	int8_t emitOk;
	int8_t off;	/* set by the tests: draw every pass as the tick left it */
	int32_t key;
	int32_t pc;	/* cursors into prev and cur (the extra passes) */
	int32_t cc;
	int32_t peak;	/* the most words one tick logged (read by the tests) */
} ModSmooth;

static ModSmooth modSm;
static int32_t modSmLog[2][MSM_WORDS];

/* every render of the engine, before its slots */
static void modSmoothBegin(void)
{
	ModSmooth *s = &modSm;
	int32_t *t;

	s->on = 0;
	s->pc = 0;
	s->cc = 0;
	s->key = 0;
	if (s->cur == NULL) {
		s->cur = modSmLog[0];
		s->prev = modSmLog[1];
	}
	if (MOD_FPS_MODE == 0 || MOD_FPS_INTERP == 0 || (MOD_RENDER_CADENCE & 16) || MOD_TICK_FAST || s->off) {
		/* 20 fps (the option or the ladder's level), no interpolation, or a battle x2 tick (one pass, no in-between):
		 * nothing changes, nothing is kept */
		s->curN = 0;
		s->tick = MOD_FPS_TICKS - 2;
		s->emitTicked = 0;
		s->emitOk = 0;
		return;
	}
	if (MOD_RENDER_PASS == 0) {
		if (s->tick == MOD_FPS_TICKS) {
			return; /* a second render in one tick: not expected */
		}
		t = s->prev;
		s->prev = s->cur;
		s->cur = t;
		s->prevN = s->curN;
		s->curN = 0;
		if (s->prevN > s->peak) {
			s->peak = s->prevN;
		}
		s->valid = s->tick == MOD_FPS_TICKS - 1 && s->snaps == MOD_INTERP_STATS[3];
		s->tick = MOD_FPS_TICKS;
		s->snaps = MOD_INTERP_STATS[3];
		s->emitOk = s->valid && s->emitTicked;
		s->emitTicked = 0;
		s->on = 1;
	} else {
		/* an extra pass at 3/3 shows the tick's own pose: nothing to look up (steady 30's late pass stays cheap) */
		s->on = s->tick == MOD_FPS_TICKS && MOD_RENDER_ALPHA < 4096;
	}
}

/* after the engine's render: scripts run later are not drawn by it */
static inline void modSmoothEnd(void)
{
	modSm.on = 0;
}

/* before each sub-effect's render script */
static inline void modSmoothSub(int32_t slot, int32_t sub)
{
	modSm.key = (slot << 20) | (sub << 12);
}

static int32_t modSmoothWrap(int32_t d, int32_t kind)
{
	if (kind == MSM_ANG) {
		d = ((d + 2048) & 4095) - 2048;
	} else if (kind == MSM_DEG) {
		d %= 360;
		if (d >= 180) {
			d -= 360;
		} else if (d < -180) {
			d += 360;
		}
	}
	return d;
}

/* the record for key in a log from cursor *c on (keys only grow during a render), or NULL */
static int32_t *modSmoothFind(int32_t *log, int32_t n, int32_t *c, int32_t key, uint32_t hash, int32_t m)
{
	int32_t *r;

	while (*c < n) {
		r = &log[*c];
		if (r[0] > key) {
			break;
		}
		*c += 3 + ((r[2] >> 6) & 31);
		if (r[0] == key) {
			return ((uint32_t)r[1] == hash && ((r[2] >> 6) & 31) == m) ? r : NULL;
		}
	}
	return NULL;
}

static void modSmoothCall(void (*fn)(void), int32_t op)
{
	ModSmooth *s = &modSm;
	const uint8_t *d;
	int32_t n;
	int32_t i;
	int32_t j;
	int32_t k;
	int32_t m;
	int32_t c;
	uint32_t h;
	int32_t key;
	int32_t live;
	int32_t frame;
	int32_t *sp;
	int32_t *ptr;
	int32_t *rec;
	int32_t *pr;
	int32_t *inst;
	int32_t raw[MSM_VALS];
	uint8_t kind[MSM_VALS];
	int32_t vec[MSM_PTRS][4];
	int32_t save[6];

	if (!s->on) {
		fn();
		return;
	}
	for (d = MOD_SMOOTH_DRAWS; d[0] != 0 && d[0] != op; d += 2 + (((d[1] & 0x1f) + 1) >> 1)) {
	}
	if (d[0] == 0) {
		fn();
		return;
	}
	n = d[1] & 0x1f;
	sp = EFE_DATA_STACK - n;
	if (op == 65 && sp[11] < 0) {
		fn();
		return;
	}
	key = s->key++;
	inst = (int32_t *)EFE_CURRENT_DATA_SEGMENT;
	frame = inst[0];
	h = ((uint32_t)op * 0x9e3779b1u) ^ (uint32_t)EFE_SCRIPT_HEAD ^ ((uint32_t)inst << 3);
	m = 0;
	for (i = 0; i < n; i++) {
		k = (d[2 + (i >> 1)] >> ((i & 1) << 2)) & 15;
		if (k & MSM_P) {
			ptr = (int32_t *)sp[i];
			h = (h ^ (uint32_t)ptr) * 0x01000193u;
			for (c = 0; c < 3; c++) {
				raw[m] = ptr[c];
				kind[m++] = k & 7;
			}
		} else if (k == MSM_ID) {
			h = (h ^ (uint32_t)sp[i]) * 0x01000193u;
		} else if (k != MSM_CUR) {
			raw[m] = sp[i];
			kind[m++] = k;
		}
	}
	if (d[1] & MSM_T) {
		for (c = 0; c < 6; c++) {
			raw[m] = inst[1 + c];
			kind[m++] = c < 3 ? MSM_POS : MSM_ANG;
		}
	}

	pr = s->valid ? modSmoothFind(s->prev, s->prevN, &s->pc, key, h, m) : NULL;
	if (pr != NULL && (((pr[2] & 63) + 1 - frame) & 63) != 0) {
		pr = NULL; /* the instance did not move on by one frame: restarted */
	}
	if (MOD_RENDER_PASS == 0) {
		live = pr != NULL ? (uint32_t)pr[2] >> 11 : 0;
		if (s->curN + 3 + m <= MSM_WORDS) {
			rec = &s->cur[s->curN];
			rec[0] = key;
			rec[1] = h;
			rec[2] = (frame & 63) | (m << 6) | (live << 11);
			for (j = 0; j < m; j++) {
				rec[3 + j] = raw[j];
			}
			s->curN += 3 + m;
		} else {
			pr = NULL; /* not logged: drawn as it is */
		}
	} else {
		rec = modSmoothFind(s->cur, s->curN, &s->cc, key, h, m);
		if (rec == NULL) {
			fn();
			return;
		}
		live = (uint32_t)rec[2] >> 11;
		for (j = 0; j < m; j++) {
			if (raw[j] != rec[3 + j]) {
				live |= 1 << j; /* moves between the passes: already interpolated */
			}
		}
		rec[2] = (rec[2] & 0x7ff) | (live << 11);
	}
	if (pr == NULL || MOD_RENDER_ALPHA >= 4096) {
		fn();
		return;
	}
	pr += 3;
	for (j = 0; j < m; j++) {
		if (!(live & (1 << j))) {
			c = modSmoothWrap(raw[j] - pr[j], kind[j]);
			if (c > MOD_SMOOTH_CUT[kind[j]] || c < -MOD_SMOOTH_CUT[kind[j]]) {
				fn(); /* a pose cut */
				return;
			}
			raw[j] -= (c * (4096 - MOD_RENDER_ALPHA)) >> 12;
		}
	}

	m = 0;
	j = 0;
	for (i = 0; i < n; i++) {
		k = (d[2 + (i >> 1)] >> ((i & 1) << 2)) & 15;
		if (k & MSM_P) {
			for (c = 0; c < 3; c++) {
				vec[j][c] = raw[m++];
			}
			vec[j][3] = ((int32_t *)sp[i])[3];
			sp[i] = (int32_t)vec[j++];
		} else if (k != MSM_ID && k != MSM_CUR) {
			sp[i] = raw[m++];
		}
	}
	if (d[1] & MSM_T) {
		for (c = 0; c < 6; c++) {
			save[c] = inst[1 + c];
			inst[1 + c] = raw[m++];
		}
		fn();
		for (c = 0; c < 6; c++) {
			inst[1 + c] = save[c];
		}
		return;
	}
	fn();
}

/* the particle emitters' tick */
static inline void modSmoothEmitTicked(void)
{
	modSm.emitTicked = 1;
}

/* a particle's distance for this pass: back along its last step (not on the tick it was born) */
static int32_t modSmoothParticle(int32_t distance, int32_t velocity, int32_t acceleration, int32_t start)
{
	int32_t step = velocity + acceleration;

	if (!modSm.emitOk || MOD_RENDER_ALPHA >= 4096 || distance + step == start) {
		return distance;
	}
	return distance + ((step * (4096 - MOD_RENDER_ALPHA)) >> 12);
}

#endif
