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
} ModRollbackSet;

/* the tick / present helper (main.c): modTickStart before the loop's logic,
 * modPresent after tickObjects instead of render + VSync + swap */
#define MOD_PRESENT_BATTLE 1 /* origin from DRAWING_OFFSET at each swap */
void modTickStart(int32_t flags);
void modPresent(int32_t flags, ModRollbackSet *extra);

extern int32_t MOD_RENDER_PASS;
extern int32_t MOD_RENDER_MULTIPASS;
/* rand() for render code: the real one on the first pass of a tick */
int32_t modRenderRand(void);
int32_t modRenderCustomRandom(long min, long max);
void removeObjectAfterRender(int32_t objectId, int32_t instanceId);
extern int32_t MOD_FPS_EFE_NO_ROOM;

#endif
