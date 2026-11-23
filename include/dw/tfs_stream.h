#ifndef DW_TFS_STREAM_H
#define DW_TFS_STREAM_H

#include <sys/types.h>
#include <libgpu.h>

#include <dw/types.h>

/* MOD (memreclaim): map background streaming, src/main/tfs_stream.c */

typedef struct {
	int32_t opens;			/* readMapTFS calls */
	int32_t prefetches;		/* tiles read ahead (async) */
	int32_t hits;			/* window uploads served from the ring */
	int32_t waits;			/* ... that had to wait for their own prefetch */
	int32_t misses;			/* ... that read the tile synchronously */
	int32_t syncVsyncs;		/* vsyncs spent in waits + misses */
	int32_t syncVsyncsMax;		/* longest single wait or miss */
	int32_t prefetchVsyncsMax;	/* longest async tile read, start to done */
	int32_t invalidations;		/* ring dropped (a load or battle wrote over it) */
	int32_t reopens;		/* tile needed while the ring was invalid */
	int32_t readErrors;
	int32_t rowReads;		/* whole-window rows read in one go */
	int32_t rowVsyncs;
	int32_t captures;		/* tiles leaving the window copied back from VRAM */
	int32_t captureWaitMax;		/* lines waiting for the GPU before one */
	int32_t captureLinesMax;	/* lines of one StoreImage */
	int32_t startLinesMax;		/* lines to start one tile read (CdControl + CdRead) */
	int32_t refused;		/* tile wanted outside the world while the ring was invalid: not reopened (the load area is a battle's) */
} TfsStreamStats;

extern int8_t MOD_TFS_TILE_SEQ[35];
extern TfsStreamStats MOD_TFS_STATS;
/*
 * The first three palettes of the last .TFS header read (day, evening, night; MAP_CLUTS points here, not into
 * TFS_HEADER_ADDR): the header shares GENERAL_BUFFER with every sound bank, overlay and model staged in the world,
 * and a palette load reads it whenever the hour changes, long after such a load.
 */
extern u_long MOD_TFS_CLUTS[3][128];

void tfsStreamOpen(char *path);
void tfsStreamInvalidate(void);
void tfsStreamWaitIdle(void);
int32_t tfsStreamBusy(void);
void tfsStreamCheckTarget(void *buffer, int32_t bytes);
void tfsStreamTick(void);
u_long *tfsTilePixels(int32_t tile);
void tfsStageRow(int32_t firstTile, int32_t count);
void tfsUnstage(void);
void tfsLoadTile(RECT *rect, int32_t tile);
void tfsClearTile(RECT *rect);

#endif
