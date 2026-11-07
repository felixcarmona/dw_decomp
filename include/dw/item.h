#ifndef DW_ITEM_H
#define DW_ITEM_H

#include <libgte.h>

#include <dw/entity.h>
#include <dw/ids.h>
#include <dw/types.h>
#include <dw/version.h>

typedef struct
{
#if !VERSION_IS(EU)
	char name[20];
#endif
	int32_t value;
	uint16_t meritValue;
	int16_t sortingValue;
	uint8_t itemColor;
	uint8_t droppable;
	uint16_t unk;
} Item;

typedef struct {
	SVECTOR spriteLocation;
	int32_t type;
} WorldItem;

typedef struct {
	WorldItem worldItem;
	long time;
} TamerItem;

typedef struct {
	WorldItem worldItem;
	int16_t tileX;
	int16_t tileY;
} DroppedItem;

typedef struct {
	uint8_t array[30];
} InventoryTable;

typedef struct {
	ItemId array[30];
} ItemIdTable;

typedef struct {
	ItemIdTable types; /* MOD: was u8 with 0xFF = empty; now ITEM_NONE */
	InventoryTable amounts;
	InventoryTable names;
	uint8_t size;
} Inventory;

/* MOD: the v1 save layout of the inventory (u8 ids, 0xFF = empty) */
typedef struct {
	InventoryTable types;
	InventoryTable amounts;
	InventoryTable names;
	uint8_t size;
} SavedInventory;

/* MOD: an item id from a u8 field (script byte, pstat, table): 0xFF = none */
static inline ItemId itemFromLegacy(uint8_t id)
{
	return (id == LEGACY_ID_NONE) ? ITEM_NONE : id;
}

typedef void (*ItemFunction)(int16_t);

extern Item ITEM_PARA[];
#if VERSION_IS(EU)
extern char *ITEM_NAMES[];
#define ITEM_NAME(type) ITEM_NAMES[type]
#else
#define ITEM_NAME(type) ITEM_PARA[type].name
#endif
extern TamerItem TAMER_ITEM;
extern Inventory INVENTORY;
extern DroppedItem DROPPED_ITEMS[11];
extern ItemFunction ITEM_FUNCTIONS[NUM_ITEMS];

void initializeInventory(void);
void setInventorySize(uint8_t size);
int32_t giveItem(ItemId type, uint8_t amount);
void removeItem(ItemId type, uint8_t amount);
uint8_t getItemCount(ItemId type);
void sortItemsById(ItemId *data, long count);
void swapItemId(ItemId *a, ItemId *b);

void initializeDroppedItems(void);
void clearDroppedItems(void);
void spawnItem(ItemId type, int16_t tileX, int16_t tileY);
void spawnDroppedItems(Entity *e, ItemId type);
int32_t pickupItem(int16_t itemId);

/* Stable IDs are authored in content/ground-items.json and carried by SPAWN_ITEM_DAILY.
 * Only this per-day bitset is resident. IDs are append-only save state; never renumber them.
 * The build checks every spawn and the bound against this target's header. */
#define MOD_GROUND_PICKUP_MAX 1024
extern uint8_t MOD_GROUND_PICKED[MOD_GROUND_PICKUP_MAX / 8];
void modForgetGroundPicked(void);
void spawnDailyItem(ItemId type, int16_t tileX, int16_t tileY, int32_t pickupId);
void removeTamerItem(void);
void renderOverworldItem(WorldItem *item);

void handleBuffDisks(int32_t type);
void handleChips(int16_t chipId);
void handleDoubleFloppy(int32_t itemId);
void handleEvoItems(int16_t arg);
void handleFood(int16_t arg);
void handleHPHealingItem(unsigned char idx);
void handleMPHealingItem(unsigned char idx);
void handleRestore(int16_t arg);
void handleStatusItems(int32_t itemId);

#endif
