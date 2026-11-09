#ifndef DW_WORLD_OBJECT_H
#define DW_WORLD_OBJECT_H

#include <dw/types.h>

#define NUM_WORLD_OBJECTS	128

/* MOD (object ids): a Digimon's world object is (objectId = species id,
 * instanceId = entity slot) and removeObject() takes the first match. Species
 * ids 400..432 (0x190..0x1b0) are also the ids of the battle and VS objects,
 * so an object with the same instance number removed the Digimon instead of
 * itself: with species 402 the partner's damage text (0x192, slot 1) took the
 * partner's own object, the partner stopped ticking and the victory animation
 * never ended (the game froze at the end of every battle). The two objects of
 * the overworld and the normal battle that have an instance number get ids
 * above the species range (species files go up to 1023). The VS-only objects
 * (0x198, 0x19c, 0x1a3, 0x1a9..0x1b0) still sit in 400..432: new species
 * cannot enter VS. */
#define OBJECT_ID_ENTITY_TEXT	0x1f92
#define OBJECT_ID_DROPPED_ITEM	0x1f95

typedef void (*TickFunction)(int32_t);
typedef void (*RenderFunction)(int32_t);

typedef struct {
	int16_t objectId;
	int16_t instanceId;
	TickFunction tick;
	RenderFunction render;
} WorldObject;

extern WorldObject WORLD_OBJECTS[];

void initializeWorldObjects(void);
int32_t addObject(int32_t objectId, int32_t instanceId, TickFunction tick,
		  RenderFunction render);
int32_t removeObject(/* int32_t objectId, int32_t instanceId */);
void tickObjects(void);
void renderObjects(void);

#endif
