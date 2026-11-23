#ifndef DW_MOD_FPS_H
#define DW_MOD_FPS_H

#include <dw/types.h>
#include <dw/world_object.h>

/* MOD (fps): the scene can be rendered several times per 20 Hz logic tick
 * (see docs/mods/60fps.md). renderObjects() must then be idempotent:
 *
 * - State that render callbacks used to advance runs in a "late tick" phase
 *   (lateTickObjects), once per tick, right after every object's tick and
 *   before the first render pass, in the slot order of WORLD_OBJECTS (the
 *   order the renders used to run in). Objects get one with addObjectLate().
 * - MOD_RENDER_PASS is 0 during the logic tick, the late tick and the first
 *   render pass of a tick, and 1, 2... on the extra passes. Side effects that
 *   are not drawing (sounds, VRAM uploads, object list changes) only happen
 *   when it is 0.
 * - Small render-only counters (menu state machines, spinning medal...) are in
 *   a rollback list: extra passes start from the value they had before pass 0
 *   and the value pass 0 left is put back afterwards, so every pass draws the
 *   same picture and the tick sees what one render would have left. */
extern int32_t MOD_RENDER_PASS;

void lateTickObjects(void);
int32_t addObjectLate(int32_t objectId, int32_t instanceId, TickFunction tick,
		      TickFunction lateTick, RenderFunction render);
/* Free an object's slot after the render passes of this tick (for renders
 * that drew their last frame from a late tick that ended them). */
void removeObjectAfterRender(int32_t objectId, int32_t instanceId);
void modFlushRemovals(void);
extern int32_t MOD_RENDER_MULTIPASS;

void modRenderRollbackSave(void);
void modRenderRollbackBegin(void);
void modRenderRollbackEnd(void);

/* rand() replacement for render code: the real rand() on pass 0 (the game's
 * RNG stream advances exactly as with one render per tick), a private LCG on
 * the extra passes. */
int32_t modRenderRand(void);

/* Interpolation of the render passes (main.c, docs/mods/60fps.md 2.3-2.4):
 * MOD_FPS_INTERP switches it (with more than one render per tick);
 * MOD_INTERP_MAP_GEN is bumped when a map is set up or unloaded (a snap). */
extern int32_t MOD_FPS_INTERP;
extern int32_t MOD_INTERP_MAP_GEN;
extern int32_t MOD_MIST_SHIFT[3];
void modInterpInvalidate(void);

#endif
