#ifndef DW_MOD_FPS_H
#define DW_MOD_FPS_H

#include <dw/types.h>
#include <dw/world_object.h>
#include <dw/mod_present.h>
#include <fps.h> /* FPS: the build's frame rate (Makefile `FPS=20|30|60`, generated) */

/* FPS=20 compiles out extra passes. FPS=30|60 includes the automatic
 * 20/60 selector: boot and idle-time CPU measurements choose the mode.
 * Logic cadence is independent of this presentation setting. */
#define MOD_CPU_LINES 16
#define MOD_CPU_SPINS_FAST 8500
#define MOD_CPU_SPINS_SLOW 7500
int32_t modCpuProbe(void);
/* OPTIONS indices; save byte 2 uses 0xA0 + index, never legacy 20/30/60. */
#define MOD_FPS_AUTO 0
#define MOD_FPS_60 1
#define MOD_FPS_20 2
extern int32_t MOD_OPT_FPS;
void modSetFrameRate(int32_t mode);
#if FPS == 20
#define MOD_FPS_MODE 0
#elif FPS == 30 || FPS == 60
extern int32_t MOD_FPS_MODE_RUN;
#define MOD_FPS_MODE MOD_FPS_MODE_RUN
#else
#error "FPS must be 20, 30 or 60"
#endif
/* the same value in RAM (read-only), so tools can tell which build runs */
extern const int32_t MOD_FPS_BUILD;

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
 * MOD_INTERP_MAP_GEN is bumped when a map is set up or unloaded (a snap);
 * render code that moves an entity itself calls modInterpUnapplyEntity first
 * to get the tick's values back. */
extern int32_t MOD_FPS_INTERP;
extern int32_t MOD_INTERP_MAP_GEN;
extern int32_t MOD_MIST_SHIFT[3];
/* bones 1..n: 0 = not interpolated, 1 = tamer and partner, 2 = all */
extern int32_t MOD_FPS_INTERP_BONES;
/* startAnimation: the entity's pose jumps, its bones are not interpolated */
void modInterpAnimStarted(void *entity);
void modSmoothBeforeStart(void *entity, int32_t destination);
void modSmoothForget(int32_t instance);
void modSmoothMapReset(void);
void modInterpUnapplyEntity(int32_t entityId);
void modInterpInvalidate(void);

#endif
