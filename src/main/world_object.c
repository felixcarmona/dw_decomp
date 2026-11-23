#include <dw/world_object.h>
#include <dw/mod_fps.h>


WorldObject WORLD_OBJECTS[NUM_WORLD_OBJECTS + 1];

/* MOD (fps): late tick per slot (see dw/mod_fps.h). */
static TickFunction LATE_TICKS[NUM_WORLD_OBJECTS + 1];
static int32_t modLastAddedSlot;
static int32_t modLateTickPending;
#define MOD_MAX_PENDING_REMOVALS 16
static int16_t modPendingRemovals[MOD_MAX_PENDING_REMOVALS][3];
static int32_t modNumPendingRemovals;
/* set by gameLoop around its render passes: removals wait until all are done */
int32_t MOD_RENDER_MULTIPASS;

static void modCancelPendingRemoval(int32_t slot)
{
	int32_t k;

	for (k = 0; k < modNumPendingRemovals; k++) {
		if (modPendingRemovals[k][0] == slot) {
			modPendingRemovals[k][0] = NUM_WORLD_OBJECTS;
		}
	}
}

void initializeWorldObjects(void)
{
	int32_t i;

	for (i = 0; i < NUM_WORLD_OBJECTS; ++i) {
		WORLD_OBJECTS[i].objectId = -1;
		WORLD_OBJECTS[i].instanceId = -1;
		WORLD_OBJECTS[i].tick = NULL;
		WORLD_OBJECTS[i].render = NULL;
		LATE_TICKS[i] = NULL;
	}
	modNumPendingRemovals = 0;
	modLateTickPending = 0;
}


int32_t addObject(objectId, instanceId, tick, render)
int16_t objectId;
int16_t instanceId;
TickFunction tick;
RenderFunction render;
{
	int32_t i;

	WORLD_OBJECTS[NUM_WORLD_OBJECTS].objectId = -1;

	i = 0;
	while (1) {
		if (WORLD_OBJECTS[i].objectId == -1) {
			break;
		}

		++i;
	}

	if (i == NUM_WORLD_OBJECTS) {
		return 0;
	}

	WORLD_OBJECTS[i].objectId = objectId;
	WORLD_OBJECTS[i].instanceId = instanceId;
	WORLD_OBJECTS[i].tick = tick;
	WORLD_OBJECTS[i].render = render;
	LATE_TICKS[i] = NULL;
	modLastAddedSlot = i;

	return 1;
}

int32_t addObjectLate(int32_t objectId, int32_t instanceId, TickFunction tick,
		      TickFunction lateTick, RenderFunction render)
{
	if (addObject(objectId, instanceId, tick, render) == 0) {
		return 0;
	}
	LATE_TICKS[modLastAddedSlot] = lateTick;
	return 1;
}


int32_t removeObject(objectId, instanceId)
int16_t objectId;
int16_t instanceId;
{
	int32_t i;

	WORLD_OBJECTS[NUM_WORLD_OBJECTS].objectId = objectId;
	WORLD_OBJECTS[NUM_WORLD_OBJECTS].instanceId = instanceId;

	i = 0;
	while (1) {
		if ((WORLD_OBJECTS[i].objectId == objectId) &&
		    (WORLD_OBJECTS[i].instanceId == instanceId)) {
			break;
		}

		++i;
	}

	if (i == NUM_WORLD_OBJECTS) {
		return 0;
	}

	WORLD_OBJECTS[i].objectId = -1;
	WORLD_OBJECTS[i].instanceId = -1;
	WORLD_OBJECTS[i].tick = NULL;
	WORLD_OBJECTS[i].render = NULL;
	LATE_TICKS[i] = NULL;
	modCancelPendingRemoval(i);

	return 1;
}

void removeObjectAfterRender(int32_t objectId, int32_t instanceId)
{
	int32_t i;

	for (i = 0; i < NUM_WORLD_OBJECTS; ++i) {
		if ((WORLD_OBJECTS[i].objectId == objectId) &&
		    (WORLD_OBJECTS[i].instanceId == instanceId)) {
			break;
		}
	}
	if (i == NUM_WORLD_OBJECTS) {
		return;
	}
	/* no more late ticks; the render still runs until the passes end */
	LATE_TICKS[i] = NULL;
	if (modNumPendingRemovals == MOD_MAX_PENDING_REMOVALS) {
		removeObject(objectId, instanceId);
		return;
	}
	modPendingRemovals[modNumPendingRemovals][0] = i;
	modPendingRemovals[modNumPendingRemovals][1] = objectId;
	modPendingRemovals[modNumPendingRemovals][2] = instanceId;
	modNumPendingRemovals++;
}

void modFlushRemovals(void)
{
	int32_t k;
	int32_t i;

	for (k = 0; k < modNumPendingRemovals; k++) {
		i = modPendingRemovals[k][0];
		if (i == NUM_WORLD_OBJECTS) {
			continue;
		}
		if ((WORLD_OBJECTS[i].objectId == modPendingRemovals[k][1]) &&
		    (WORLD_OBJECTS[i].instanceId == modPendingRemovals[k][2])) {
			WORLD_OBJECTS[i].objectId = -1;
			WORLD_OBJECTS[i].instanceId = -1;
			WORLD_OBJECTS[i].tick = NULL;
			WORLD_OBJECTS[i].render = NULL;
			LATE_TICKS[i] = NULL;
		}
	}
	modNumPendingRemovals = 0;
}

void lateTickObjects(void)
{
	int32_t i;

	modLateTickPending = 0;
	for (i = 0; i < NUM_WORLD_OBJECTS; ++i) {
		if ((WORLD_OBJECTS[i].objectId != -1) && (LATE_TICKS[i] != NULL)) {
			LATE_TICKS[i](WORLD_OBJECTS[i].instanceId);
		}
	}
	modRenderRollbackSave();
}

void tickObjects(void)
{
	int32_t i;

	modFlushRemovals();

	for (i = 0; i < NUM_WORLD_OBJECTS; ++i) {
		if ((WORLD_OBJECTS[i].objectId != -1) &&
		    (WORLD_OBJECTS[i].tick != NULL)) {
			WORLD_OBJECTS[i].tick(WORLD_OBJECTS[i].instanceId);
		}
	}
	modLateTickPending = 1;
}

void renderObjects(void)
{
	int32_t i;
	WorldObject *banner = NULL;

	/* loops that do not call lateTickObjects() themselves (battle, menus)
	 * get it here, once per tick, still before anything is drawn */
	if (modLateTickPending) {
		lateTickObjects();
	}

	for (i = 0; i < NUM_WORLD_OBJECTS; ++i) {
		if ((WORLD_OBJECTS[i].objectId != -1) &&
		    (WORLD_OBJECTS[i].render != NULL)) {
			/* addObject reuses holes, so the banner can otherwise run before
			 * models. It must budget against all of this frame's scene. */
			if (WORLD_OBJECTS[i].objectId == OBJECT_ID_BATTLE_START_TEXT) {
				banner = &WORLD_OBJECTS[i];
			} else {
				WORLD_OBJECTS[i].render(WORLD_OBJECTS[i].instanceId);
			}
		}
	}
	/* Another renderer can remove/reuse a slot; only call a surviving banner. */
	if (banner != NULL && banner->objectId == OBJECT_ID_BATTLE_START_TEXT && banner->render != NULL) {
		banner->render(banner->instanceId);
	}
	if (!MOD_RENDER_MULTIPASS) {
		modFlushRemovals();
	}
}
