#ifndef DW_MAIN_H
#define DW_MAIN_H

#include <dw/entity.h>
#include <dw/memmap.h>
#include <dw/params.h>
#include <dw/types.h>

#define GENERAL_BUFFER		((uint8_t *)GENERAL_BUFFER_ADDR)
#define TEXTURE_BUFFER		((uint8_t *)TEXTURE_BUFFER_ADDR)
#define BOSS_EFE_TMD_BUFFER	((uint8_t *)BOSS_EFE_TMD_BUFFER_ADDR)

typedef struct {
	VECTOR playerPos;
	VECTOR partnerPos;
	uint8_t unk20[0x80];
	int32_t partnerType;
	int32_t money;
	PartnerPara partnerPara;
	Stats partnerStats;
	int8_t lives;
	int8_t npcMaps[8];
	ScreenId currentScreen; /* MOD: u8 in the save */
	ScreenId previousScreen;
	uint8_t currentExit;
	uint8_t previousExit;
	int8_t tamerWaypointX[30];
	int8_t tamerWaypointY[30];
	int8_t tamerPreviousTileX;
	int8_t tamerPreviousTileY;
	int8_t tamerWaypointCurrent;
	int8_t tamerWaypointCount;
	int8_t tamerStartTileX;
	int8_t tamerStartTileY;
	int8_t tamerWaypointActive;
} SavedState;

typedef struct {
	GsVIEW2 view;
	GsCOORDINATE2 coord;
	SVECTOR rotation;
	VECTOR translation;
} StdVsCamera;

extern uint8_t *GENERAL_BUFFER_PTR;
extern SavedState SAVED_STATE;
extern StdVsCamera STDVS_CAMERA;

#endif
