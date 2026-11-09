#ifndef DW_MOD_PRESENT_H
#define DW_MOD_PRESENT_H

#include <dw/types.h>

/* MOD (fps): the part of dw/mod_fps.h the overlays use (it does not declare
 * addObject, which some overlay files declare their own way). */

typedef struct {
	void *ptr;
	int32_t size;
} ModRollbackVar;

/* render-side state of a loop's caller (an overlay) rolled back like the
 * built-in list: vars[count], and two buffers of at least the sum of their
 * sizes, each rounded up to 4 */
typedef struct {
	const ModRollbackVar *vars;
	int32_t count;
	uint32_t *pre;
	uint32_t *post;
	/* adds again, at the start of every extra pass, the prims the caller's
	 * ticks put straight into the first pass's ordering table (NULL: none) */
	void (*replay)(void);
} ModRollbackSet;

/* the tick / present helper (main.c): modTickStart before the loop's logic,
 * modPresent after tickObjects instead of render + VSync + swap */
#define MOD_PRESENT_BATTLE 1 /* origin from DRAWING_OFFSET at each swap */
/* pauseFrame: the extra rollback set is the one of the loop that paused (the
 * last modPresent call without this flag) */
#define MOD_PRESENT_PAUSE 2
void modTickStart(int32_t flags);
void modPresent(int32_t flags, ModRollbackSet *extra);

/* A scene may request evenly spaced 30 fps when 60 cannot fit. Cleared by
 * modTickStart; the scene arms it again from its tick. The player's 20 fps
 * option still takes precedence. */
/* 0: inactive; 1/2: normal/early half of a steady 30 fps pair;
 * 4: curling at the requested rate, with deferred tile uploads;
 * 8 | 1/2 and 16: the same halves and one frame at 3/3, armed by the ladder in main.c (no curling tile flush). */
extern int32_t MOD_RENDER_CADENCE;

/* MOD (battle speed, main.c): the R2 switch; set before modTickStart when the coming tick may run fast (it clears
 * it); the tick that is running is fast; 0 on a fast pair's second tick */
extern int8_t MOD_BATTLE_FAST;
extern int8_t MOD_BATTLE_FAST_TICK;
extern int8_t MOD_TICK_FAST;
extern int8_t MOD_REAL_TICK;

extern int32_t MOD_RENDER_PASS;
extern int32_t MOD_RENDER_MULTIPASS;
/* rand() for render code: the real one on the first pass of a tick */
int32_t modRenderRand(void);
int32_t modRenderCustomRandom(long min, long max);
void removeObjectAfterRender(int32_t objectId, int32_t instanceId);
extern int32_t MOD_FPS_EFE_NO_ROOM;
/* the effect render scripts' script/data area (before the file's TMD),
 * copied aside before an extra pass runs them and put back after: the copy
 * goes right below top (the start of whatever the battle loaded above the
 * EFE heap), at least 0x5000 bytes above the heap pointer; 0 if it does not
 * fit (counted in MOD_FPS_EFE_NO_ROOM) */
int32_t modEfeAreaSave(int32_t data, uint32_t top, uint32_t **buf);
void modEfeAreaRestore(int32_t data, uint32_t *buf, int32_t size);

#endif
