/*
 * MOD (memreclaim): stream the map background (.TFS) from the CD.
 *
 * The original reads the whole .TFS (up to 482 KB, 30 tiles of 128x128 8 bpp)
 * into GENERAL_BUFFER and keeps it there for as long as the screen lasts;
 * only the 4x3 tiles of the VRAM window are visible at a time. Here only the
 * header (the 1-5 palettes, which MAP_CLUTS points into) stays resident at
 * TFS_HEADER_ADDR, and a ring of TFS_RING_SLOTS tile slots follows it
 * (include/dw/memmap.h). The window logic (handleTileUpdate,
 * updateTileColumn/Row, uploadMapTileImages) and what reaches VRAM are
 * unchanged; only the LoadImage source changes (tfsTilePixels).
 *
 * - Prefetch: once per tick (from the file read queue object), the tile the
 *   window may need soonest that is not in the ring is read asynchronously
 *   (one tile per CdRead, 9 sectors), only while the game's own read queue
 *   is empty. "Soonest" is how far the camera is from each of
 *   handleTileUpdate's four thresholds (wantTiles).
 * - The window's hysteresis is 1-10 px, so right after a shift the strip
 *   that just left is the nearest again; it is copied back from VRAM into
 *   the ring as it leaves (StoreImage in tfsLoadTile), not read again.
 * - A tile that is not in the ring when the window needs it is read
 *   synchronously (a "miss", counted in MOD_TFS_STATS). Whole-window
 *   uploads (map load, curling, fishing) read each row in one go
 *   (tfsStageRow).
 * - CD arbitration: the read queue does not start a request while a tile
 *   read is in flight, and the synchronous readers (readFile,
 *   readFileSectors, readFileSection) wait for it first.
 * - Anything that loads into [GENERAL_BUFFER, TFS_RING_END) (sound staging,
 *   overlays, the evolution model) and every battle invalidate the ring; the
 *   game's readMapTFS calls (map load, battle exit, evolution, death,
 *   jukebox...) reopen it. A tile needed while the ring is invalid reopens
 *   it first, in the world only (ringMayReopen): in a battle the load area
 *   is the fight's (the EFE engine's slot table sits at GENERAL_BUFFER, its
 *   heap and BTL_REL above), and a reopen there wrote the .TFS header and a
 *   tile over them: the fight's second render jumped through the garbage
 *   (2026-10-22, the camera lift of a tall partner asking for a tile row in
 *   the second fight of a screen). Outside the world such a tile is simply
 *   not uploaded (the window keeps what VRAM holds; the next readMapTFS
 *   restores it) and MOD_TFS_STATS.refused counts it.
 * - The palettes are not read from the header once the screen is set up: a
 *   load over it in the world is only noticed by the ring, and the header
 *   is read again lazily (the next tile the window needs), while the hour
 *   can change first (updateTimeOfDay, tickDaytimeTransition). Each header
 *   read copies them to MOD_TFS_CLUTS, which MAP_CLUTS points to.
 */
#include <libcd.h>
#include <string.h>
#include <libetc.h>
#include <libgpu.h>
#include <sys/types.h>

#include <dw/btl.h>
#include <dw/file.h>
#include <dw/map.h>
#include <dw/memmap.h>
#include <dw/tfs_stream.h>
#include <dw/types.h>

#define TFS_TILE_STRIDE	0x4004	/* u32 source position + 128 * 128 bytes */
#define TFS_TILE_BYTES	0x4000

#define SLOT_FREE	0
#define SLOT_LOADING	1
#define SLOT_READY	2

typedef struct {
	int8_t tile;		/* MAP_TILE_DATA.tiles index */
	int8_t state;
	int16_t pad;
	uint32_t lastUse;
} TfsSlot;

/* k-th non-empty tile of the file for each MAP_TILE_DATA.tiles entry (setupMap) */
int8_t MOD_TFS_TILE_SEQ[35];
TfsStreamStats MOD_TFS_STATS;
u_long MOD_TFS_CLUTS[3][128];

static TfsSlot TFS_SLOTS[TFS_RING_SLOTS];
static char TFS_PATH[32];
static int32_t TFS_ACTIVE;
static int32_t TFS_FIRST_SECTOR;
static int32_t TFS_DATA_OFFSET;	/* 8 + 512 * nCLUT */
static int32_t TFS_NUM_SLOTS;
static int32_t TFS_INFLIGHT = -1;
static int32_t TFS_INFLIGHT_START;
static uint32_t TFS_CLOCK;
static int32_t TFS_LAST_END;	/* vsync the last tile read ended */

int32_t isFileReadQueueIdle(void);
int32_t isFileReadQueueReading(void);
int CD_sync(int mode, u_char *result);	/* libcd: CdSync without the wrapper (not linked) */
void tfsForgetCells(void);
static int32_t wantTiles(int8_t *tiles, int16_t *dist);

/*
 * Never poll CdReadSync for a read that ended a while ago: libcd's
 * CdReadSync takes "no progress for 60 vsyncs" as a stalled read and
 * restarts the LAST read into its buffer (cd_read_retry, "CdRead:
 * retry..."), even when that read had finished. So it is only called for a
 * read of ours that is in flight, or for the queue's while the queue says it
 * is reading (it polls every tick).
 */
static void waitQueueRead(void)
{
	if (isFileReadQueueReading()) {
		while (CdReadSync(1, NULL) > 0);
	}
}

static uint8_t *slotAddr(int32_t slot)
{
	return (uint8_t *)TFS_RING_ADDR + slot * TFS_SLOT_SIZE;
}

static int32_t tileOffset(int32_t seq)
{
	return TFS_DATA_OFFSET + seq * TFS_TILE_STRIDE + 4;
}

static void finishInflight(int32_t result)
{
	int32_t t;

	TFS_SLOTS[TFS_INFLIGHT].state = (result == 0) ? SLOT_READY : SLOT_FREE;
	if (result != 0) {
		MOD_TFS_STATS.readErrors++;
	}
	t = VSync(-1) - TFS_INFLIGHT_START;
	if (t > MOD_TFS_STATS.prefetchVsyncsMax) {
		MOD_TFS_STATS.prefetchVsyncsMax = t;
	}
	TFS_INFLIGHT = -1;
	TFS_LAST_END = VSync(-1);
}

int32_t tfsStreamBusy(void)
{
	int32_t r;

	if (TFS_INFLIGHT < 0) {
		return 0;
	}
	r = CdReadSync(1, NULL);
	if (r > 0) {
		return 1;
	}
	finishInflight(r);
	return 0;
}

void tfsStreamWaitIdle(void)
{
	int32_t r;

	if (TFS_INFLIGHT < 0) {
		return;
	}
	while ((r = CdReadSync(1, NULL)) > 0);
	finishInflight(r);
}

/* file seq range of a whole-window row staged at its original place (-1: none) */
static int32_t TFS_SPAN_FIRST = -1;
static int32_t TFS_SPAN_LAST = -1;
static uint32_t TFS_SPAN_BASE;	/* + file offset = where the staged row is */

void tfsStreamInvalidate(void)
{
	int32_t i;

	TFS_SPAN_FIRST = TFS_SPAN_LAST = -1;
	tfsStreamWaitIdle();
	if (TFS_ACTIVE) {
		MOD_TFS_STATS.invalidations++;
	}
	TFS_ACTIVE = 0;
	for (i = 0; i < TFS_RING_SLOTS; i++) {
		TFS_SLOTS[i].state = SLOT_FREE;
		TFS_SLOTS[i].tile = -1;
	}
}

void tfsStreamCheckTarget(void *buffer, int32_t bytes)
{
	uint32_t a = (uint32_t)buffer;

	/*
	 * A synchronous read must not cut short the queue's read in flight: a
	 * second CdRead aborts it, the queue's next CdReadSync sees the newer read
	 * complete and the request ends with only its first sectors (load_ending's
	 * ENDI_REL at 0x80060000 kept BTL_REL's code above 0x80060800, and the
	 * ending jumped through it: farewell at 237, 2026-10-24).
	 */
	waitQueueRead();
	tfsStreamWaitIdle();
	if ((a < TFS_RING_END) && (a + bytes > TFS_HEADER_ADDR)) {
		tfsStreamInvalidate();
	}
}

static void startRead(int32_t slot, int32_t tile)
{
	CdlLOC loc;
	int32_t off;
	int32_t sectors;

	uint16_t l0 = *(volatile uint16_t *)0x1F801110;

	off = tileOffset(MOD_TFS_TILE_SEQ[tile]);
	sectors = ((off & 0x7FF) + TFS_TILE_BYTES + 0x7FF) >> 11;
	CdIntToPos(TFS_FIRST_SECTOR + (off >> 11), &loc);
	do {
		while (CdControl(CdlSetloc, (u_char *)&loc, NULL) == 0);
	} while (CdRead(sectors, (u_long *)slotAddr(slot), CdlModeSpeed) == 0);

	TFS_SLOTS[slot].tile = tile;
	TFS_SLOTS[slot].state = SLOT_LOADING;
	TFS_SLOTS[slot].lastUse = ++TFS_CLOCK;
	TFS_INFLIGHT = slot;
	TFS_INFLIGHT_START = VSync(-1);
	l0 = (*(volatile uint16_t *)0x1F801110 - l0) & 0xFFFF;
	if (l0 > MOD_TFS_STATS.startLinesMax) {
		MOD_TFS_STATS.startLinesMax = l0;
	}
}

/* Reads the header (palettes) of the .TFS at path and empties the ring. */
void tfsStreamOpen(char *path)
{
	FileLookup lookup;
	int32_t size;
	int32_t i;

	tfsStreamInvalidate();
	tfsForgetCells();	/* VRAM may hold another map's (or file's) tiles */
	if (path != TFS_PATH) {
		for (i = 0; (TFS_PATH[i] = path[i]) != 0 && i < 31; i++);
		TFS_PATH[31] = 0;
	}
	if (lookupFileTable(&lookup, TFS_PATH) == 0) {
		return;
	}
	TFS_FIRST_SECTOR = CdPosToInt(&lookup.pos);
	readFileSectors(TFS_PATH, (void *)TFS_HEADER_ADDR, 0, TFS_HEADER_SIZE >> 11);
	TFS_DATA_OFFSET = 8 + 512 * ((uint32_t *)TFS_HEADER_ADDR)[1];
	memcpy(MOD_TFS_CLUTS, (uint8_t *)TFS_HEADER_ADDR + 8, sizeof(MOD_TFS_CLUTS));

	/*
	 * Never use more of the load area than this screen's whole TFS took in
	 * the original (whatever the game writes above a small TFS stays safe).
	 */
	size = ((lookup.size + 0x7FF) & ~0x7FF) - TFS_HEADER_SIZE;
	TFS_NUM_SLOTS = size / TFS_SLOT_SIZE;
	if (TFS_NUM_SLOTS > TFS_RING_SLOTS) {
		TFS_NUM_SLOTS = TFS_RING_SLOTS;
	} else if (TFS_NUM_SLOTS < 1) {
		TFS_NUM_SLOTS = 1;
	}
	TFS_ACTIVE = 1;
	MOD_TFS_STATS.opens++;
}

static int32_t findSlot(int32_t tile)
{
	int32_t i;

	for (i = 0; i < TFS_NUM_SLOTS; i++) {
		if ((TFS_SLOTS[i].state != SLOT_FREE) && (TFS_SLOTS[i].tile == tile)) {
			return i;
		}
	}
	return -1;
}

int8_t TFS_CELL[12];	/* tile in each VRAM window cell, -1 = none (tfsLoadTile) */

static int32_t inVram(int32_t tile)
{
	int32_t i;

	for (i = 0; i < 12; i++) {
		if (TFS_CELL[i] == tile) {
			return 1;
		}
	}
	return 0;
}

static int32_t inWindow(int32_t tile)
{
	int32_t x = tile % MAP_TILE_DATA.width;
	int32_t y = tile / MAP_TILE_DATA.width;

	return (x >= MAP_TILE_X) && (x < MAP_TILE_X + 4) && (y >= MAP_TILE_Y) && (y < MAP_TILE_Y + 3);
}

/*
 * A slot to load into: a free one; else the least recently used ready one
 * whose tile is already in the VRAM window (nothing needs it from RAM until
 * it leaves, and then it is captured back); else one outside the window
 * that is not in protect[0..count). Never a tile inside the window that is
 * not in VRAM yet (a strip being uploaded) nor `avoid`.
 */
static int32_t victimSlot(int8_t *protect, int32_t count, int32_t avoid)
{
	int32_t i;
	int32_t j;
	int32_t cls;
	int32_t best = -1;
	int32_t bestCls = 3;
	int32_t tile;

	for (i = 0; i < TFS_NUM_SLOTS; i++) {
		if (TFS_SLOTS[i].state == SLOT_FREE) {
			return i;
		}
	}
	for (i = 0; i < TFS_NUM_SLOTS; i++) {
		tile = TFS_SLOTS[i].tile;
		if ((TFS_SLOTS[i].state != SLOT_READY) || (tile == avoid)) {
			continue;
		}
		if (inVram(tile)) {
			cls = 0;
		} else if (inWindow(tile)) {
			continue;
		} else {
			for (j = 0; j < count; j++) {
				if (protect[j] == tile) {
					break;
				}
			}
			if (j != count) {
				continue;
			}
			cls = 1;
		}
		if ((cls < bestCls) || ((cls == bestCls) && (TFS_SLOTS[i].lastUse < TFS_SLOTS[best].lastUse))) {
			best = i;
			bestCls = cls;
		}
	}
	return best;
}

/*
 * Whether an invalid ring may be reopened on demand: only while the world
 * runs. GAME_STATE 1-3 is a battle (loading, fighting, its menus), whose
 * EFE engine owns [GENERAL_BUFFER, ...): see the header comment.
 */
static int32_t ringMayReopen(void)
{
	/*
	 * Not while the game's queue holds a request either: it may target the
	 * load area (a sound bank at GENERAL_BUFFER, an overlay), and its read
	 * would land over the header and tiles read now (the cell keeps what VRAM
	 * holds, as in a battle).
	 */
	if ((GAME_STATE == 0) && isFileReadQueueIdle()) {
		return 1;
	}
	MOD_TFS_STATS.refused++;
	return 0;
}

/*
 * Whole-window upload (uploadMapTileImages: map load, curling, fishing):
 * each window row is contiguous in the file, so it is read in one CdRead
 * over the ring instead of tile by tile. tfsTilePixels returns the row's
 * tiles from there; the ring is emptied because the row lands on top of it.
 */

void tfsStageRow(int32_t firstTile, int32_t count)
{
	int32_t i;
	int32_t first = -1;
	int32_t last = -1;
	int32_t off0;
	int32_t off1;
	int32_t s0;
	CdlLOC loc;

	for (i = 0; i < count; i++) {
		if (MOD_TFS_TILE_SEQ[firstTile + i] >= 0) {
			if (first < 0) {
				first = MOD_TFS_TILE_SEQ[firstTile + i];
			}
			last = MOD_TFS_TILE_SEQ[firstTile + i];
		}
	}
	if (first < 0) {
		return;
	}
	if (!TFS_ACTIVE) {
		if (!ringMayReopen()) {
			return;	/* no span: tfsTilePixels gives NULL for the row's tiles and nothing is uploaded */
		}
		MOD_TFS_STATS.reopens++;
		tfsStreamOpen(TFS_PATH);
	}
	tfsStreamInvalidate();	/* the row overwrites the ring */
	TFS_ACTIVE = 1;
	waitQueueRead();
	off0 = tileOffset(first);
	off1 = tileOffset(last) + TFS_TILE_BYTES;
	s0 = off0 >> 11;
	/*
	 * Into the ring when the row fits (8 slots take a row of 4 tiles), so
	 * WORLD_SCRATCH is not touched; else where the original had it (only on
	 * maps whose whole TFS ends below the ring's end).
	 */
	if ((((off1 + 0x7FF) >> 11) - s0) << 11 <= TFS_NUM_SLOTS * TFS_SLOT_SIZE) {
		TFS_SPAN_BASE = TFS_RING_ADDR - (s0 << 11);
	} else {
		TFS_SPAN_BASE = TFS_HEADER_ADDR;
	}
	CdIntToPos(TFS_FIRST_SECTOR + s0, &loc);
	i = VSync(-1);
	do {
		do {
			while (CdControl(CdlSetloc, (u_char *)&loc, NULL) == 0);
		} while (CdRead(((off1 + 0x7FF) >> 11) - s0,
				(u_long *)(TFS_SPAN_BASE + (s0 << 11)), CdlModeSpeed) == 0);
	} while (CdReadSync(0, NULL) != 0);
	i = VSync(-1) - i;
	MOD_TFS_STATS.rowReads++;
	MOD_TFS_STATS.rowVsyncs += i;
	TFS_SPAN_FIRST = first;
	TFS_SPAN_LAST = last;
}

void tfsUnstage(void)
{
	TFS_SPAN_FIRST = TFS_SPAN_LAST = -1;
}

/*
 * The pixels of tile (MAP_TILE_DATA.tiles index) for LoadImage; NULL when
 * the ring is invalid and may not be reopened (not in the world).
 */
u_long *tfsTilePixels(int32_t tile)
{
	int32_t slot;
	int32_t t;

	t = MOD_TFS_TILE_SEQ[tile];
	if (t < 0) {
		/* off the map (the original reads a stale pointer here too) */
		return (u_long *)TFS_HEADER_ADDR;
	}
	if ((t >= TFS_SPAN_FIRST) && (t <= TFS_SPAN_LAST)) {
		return (u_long *)(TFS_SPAN_BASE + tileOffset(t));
	}
	if (!TFS_ACTIVE) {
		if (!ringMayReopen()) {
			return NULL;
		}
		MOD_TFS_STATS.reopens++;
		tfsStreamOpen(TFS_PATH);
	}
	slot = findSlot(tile);
	if ((slot >= 0) && (TFS_SLOTS[slot].state == SLOT_LOADING)) {
		t = VSync(-1);
		tfsStreamWaitIdle();
		MOD_TFS_STATS.waits++;
		t = VSync(-1) - t;
		MOD_TFS_STATS.syncVsyncs += t;
		if (t > MOD_TFS_STATS.syncVsyncsMax) {
			MOD_TFS_STATS.syncVsyncsMax = t;
		}
		if (TFS_SLOTS[slot].state != SLOT_READY) {
			slot = -1;
		}
	}
	if (slot >= 0) {
		MOD_TFS_STATS.hits++;
	} else {
		/* miss: read it now, after whatever the CD is doing */
		MOD_TFS_STATS.misses++;
		t = VSync(-1);
		tfsStreamWaitIdle();
		waitQueueRead();
		slot = victimSlot(NULL, 0, -1);
		if (slot < 0) {
			slot = 0;	/* cannot happen with 1 slot or more free of the window */
		}
		startRead(slot, tile);
		tfsStreamWaitIdle();
		t = VSync(-1) - t;
		MOD_TFS_STATS.syncVsyncs += t;
		if (t > MOD_TFS_STATS.syncVsyncsMax) {
			MOD_TFS_STATS.syncVsyncsMax = t;
		}
	}
	TFS_SLOTS[slot].lastUse = ++TFS_CLOCK;
	return (u_long *)(slotAddr(slot) + (tileOffset(MOD_TFS_TILE_SEQ[tile]) & 0x7FF));
}

/*
 * The tiles the window may need next, nearest first. For each side, how far
 * the camera has to move before handleTileUpdate shifts the window there
 * (its thresholds: right at cameraX >= 128 * (wx + 1) + 10, left below
 * 128 * wx + 1, down at cameraY >= 128 * (wy + 1), up below 128 * wy + 1).
 * Right after a shift the strip that just left is 1-10 px away (the
 * original's hysteresis is that small), which is why it is also captured
 * from VRAM when it leaves (tfsLoadTile).
 */
#define TFS_MAX_WANTED	24

/*
 * Recent camera motion per axis (sign, held for TFS_MOVE_HOLD ticks after
 * the camera stops): a side the camera is not moving toward is
 * TFS_AWAY_PENALTY px farther, so the strip ahead is read before a nearer
 * one beside or behind (the one behind is captured from VRAM anyway).
 */
#define TFS_AWAY_PENALTY	96
#define TFS_MOVE_HOLD		40
static int8_t TFS_MOVE_X;
static int8_t TFS_MOVE_Y;
static int16_t TFS_MOVE_TIMER_X;
static int16_t TFS_MOVE_TIMER_Y;
static int16_t TFS_PREV_CAM_X;
static int16_t TFS_PREV_CAM_Y;

static void trackMotion(void)
{
	int32_t dx = MAP_TILE_DATA.cameraX - TFS_PREV_CAM_X;
	int32_t dy = MAP_TILE_DATA.cameraY - TFS_PREV_CAM_Y;

	TFS_PREV_CAM_X = MAP_TILE_DATA.cameraX;
	TFS_PREV_CAM_Y = MAP_TILE_DATA.cameraY;
	if ((dx != 0) && (dx > -64) && (dx < 64)) {
		TFS_MOVE_X = (dx > 0) ? 1 : -1;
		TFS_MOVE_TIMER_X = TFS_MOVE_HOLD;
	} else if ((TFS_MOVE_TIMER_X > 0) && (--TFS_MOVE_TIMER_X == 0)) {
		TFS_MOVE_X = 0;
	}
	if ((dy != 0) && (dy > -64) && (dy < 64)) {
		TFS_MOVE_Y = (dy > 0) ? 1 : -1;
		TFS_MOVE_TIMER_Y = TFS_MOVE_HOLD;
	} else if ((TFS_MOVE_TIMER_Y > 0) && (--TFS_MOVE_TIMER_Y == 0)) {
		TFS_MOVE_Y = 0;
	}
}

static int32_t wantTiles(int8_t *tiles, int16_t *dist)
{
	int32_t W = MAP_TILE_DATA.width;
	int32_t H = MAP_TILE_DATA.height;
	int32_t cx = MAP_TILE_DATA.cameraX;
	int32_t cy = MAP_TILE_DATA.cameraY;
	int32_t wx = MAP_TILE_X;
	int32_t wy = MAP_TILE_Y;
	int32_t sx[2], sd[2], ns = 0;	/* candidate columns and their distance */
	int32_t ty[2], td[2], nt = 0;	/* candidate rows */
	int32_t n = 0;
	int32_t i, j, k, x, y, d, tile;

	if (wx + 4 < W) {
		sx[ns] = wx + 4;
		sd[ns++] = 128 * wx + 138 - cx + ((TFS_MOVE_X > 0) ? 0 : TFS_AWAY_PENALTY);
	}
	if (wx > 0) {
		sx[ns] = wx - 1;
		sd[ns++] = cx - 128 * wx + ((TFS_MOVE_X < 0) ? 0 : TFS_AWAY_PENALTY);
	}
	if (wy + 3 < H) {
		ty[nt] = wy + 3;
		td[nt++] = 128 * (wy + 1) - cy + ((TFS_MOVE_Y > 0) ? 0 : TFS_AWAY_PENALTY);
	}
	if (wy > 0) {
		ty[nt] = wy - 1;
		td[nt++] = cy - 128 * wy + ((TFS_MOVE_Y < 0) ? 0 : TFS_AWAY_PENALTY);
	}
	/* columns (3 tiles), rows (4 tiles) and the corners between them */
	for (i = 0; i < ns; i++) {
		for (k = 0; k < 3; k++) {
			tiles[n] = sx[i] + (wy + k) * W;
			dist[n++] = sd[i];
		}
	}
	for (j = 0; j < nt; j++) {
		for (k = 0; k < 4; k++) {
			tiles[n] = (wx + k) + ty[j] * W;
			dist[n++] = td[j];
		}
	}
	for (i = 0; i < ns; i++) {
		for (j = 0; j < nt; j++) {
			tiles[n] = sx[i] + ty[j] * W;
			dist[n++] = (sd[i] > td[j]) ? sd[i] : td[j];
		}
	}
	/* drop off-map and empty tiles, then sort by distance (insertion) */
	for (i = j = 0; i < n; i++) {
		tile = tiles[i];
		x = tile % W;
		y = tile / W;
		if ((x >= W) || (y >= H) || (MOD_TFS_TILE_SEQ[tile] < 0)) {
			continue;
		}
		d = (dist[i] < 0) ? 0 : dist[i];
		for (k = j; (k > 0) && (dist[k - 1] > d); k--) {
			tiles[k] = tiles[k - 1];
			dist[k] = dist[k - 1];
		}
		tiles[k] = tile;
		dist[k] = d;
		j++;
	}
	return j;
}

/*
 * Once per tick: finish the read in flight, then start reading the nearest
 * tile the window may need that is not in the ring. A ring tile is only
 * given up for one at least TFS_EVICT_MARGIN px nearer (so a camera standing
 * between two strips does not make them evict each other).
 */
#define TFS_EVICT_MARGIN	32

void tfsStreamTick(void)
{
	int8_t wanted[TFS_MAX_WANTED];
	int16_t dist[TFS_MAX_WANTED];
	int32_t n;
	int32_t i;
	int32_t p;
	int32_t slot;

	if (tfsStreamBusy()) {
		return;
	}
	if (GAME_STATE != 0) {
		/* battles write the load area outside the CD (EFE engine) */
		if (TFS_ACTIVE) {
			tfsStreamInvalidate();
		}
		return;
	}
	if (!TFS_ACTIVE || (MAP_TILE_DATA.width == 0)) {
		return;
	}
	trackMotion();
	if (!isFileReadQueueIdle()) {
		return;
	}
	/*
	 * Starting a read waits for the drive to acknowledge each command; right
	 * after a read the drive is still pausing and that wait took up to 565
	 * lines of the tick. Start only when the last command has completed and
	 * not in the vsyncs right after a read ended.
	 */
	if ((CD_sync(1, NULL) == CdlNoIntr) || ((uint32_t)(VSync(-1) - TFS_LAST_END) < 3)) {
		return;
	}

	n = wantTiles(wanted, dist);
	for (i = 0; i < n; i++) {
		if (findSlot(wanted[i]) >= 0) {
			continue;
		}
		/* protect every wanted tile not clearly farther than this one */
		for (p = i + 1; (p < n) && (dist[p] < dist[i] + TFS_EVICT_MARGIN); p++);
		slot = victimSlot(wanted, p, -1);
		if (slot < 0) {
			return;
		}
		MOD_TFS_STATS.prefetches++;
		startRead(slot, wanted[i]);
		return;
	}
}

/*
 * The window's VRAM uploads (map.c) go through these two: they remember
 * which tile each of the 12 VRAM cells holds, and before a cell is
 * overwritten the tile leaving the window is copied back from VRAM into the
 * ring (StoreImage) if the ring does not have it, so walking back across
 * the threshold never waits for the CD.
 */
static void captureOutgoing(RECT *rect, int32_t incoming)
{
	int32_t c = ((rect->x - 768) >> 6) + (rect->y >> 7) * 4;
	int32_t old;
	int32_t slot;
	int32_t n;
	int8_t wanted[TFS_MAX_WANTED];
	int16_t dist[TFS_MAX_WANTED];
	uint16_t l0, l1, l2;

	if ((c < 0) || (c >= 12)) {
		return;
	}
	old = TFS_CELL[c];
	TFS_CELL[c] = incoming;
	if (!TFS_ACTIVE || (old < 0) || (old == incoming) || (MOD_TFS_TILE_SEQ[old] < 0) ||
	    (findSlot(old) >= 0)) {
		return;
	}
	n = wantTiles(wanted, dist);
	slot = victimSlot(wanted, n, incoming);
	if (slot < 0) {
		return;
	}
	l0 = *(volatile uint16_t *)0x1F801110;	/* root counter 1: lines */
	DrawSync(0);
	l1 = *(volatile uint16_t *)0x1F801110;
	StoreImage(rect, (u_long *)(slotAddr(slot) + (tileOffset(MOD_TFS_TILE_SEQ[old]) & 0x7FF)));
	DrawSync(0);
	l2 = *(volatile uint16_t *)0x1F801110;
	if (((l1 - l0) & 0xFFFF) > MOD_TFS_STATS.captureWaitMax) {
		MOD_TFS_STATS.captureWaitMax = (l1 - l0) & 0xFFFF;
	}
	if (((l2 - l1) & 0xFFFF) > MOD_TFS_STATS.captureLinesMax) {
		MOD_TFS_STATS.captureLinesMax = (l2 - l1) & 0xFFFF;
	}
	TFS_SLOTS[slot].tile = old;
	TFS_SLOTS[slot].state = SLOT_READY;
	TFS_SLOTS[slot].lastUse = ++TFS_CLOCK;
	MOD_TFS_STATS.captures++;
}

void tfsLoadTile(RECT *rect, int32_t tile)
{
	u_long *pixels;

	if (!TFS_ACTIVE && !ringMayReopen()) {
		return;	/* the cell keeps what VRAM holds; captureOutgoing would write the ring too */
	}
	captureOutgoing(rect, tile);
	pixels = tfsTilePixels(tile);
	if (pixels != NULL) {
		LoadImage(rect, pixels);
	}
}

void tfsClearTile(RECT *rect)
{
	captureOutgoing(rect, -1);
	ClearImage(rect, 0, 0, 0);
}

void tfsForgetCells(void)
{
	int32_t i;

	for (i = 0; i < 12; i++) {
		TFS_CELL[i] = -1;
	}
}
