#include <libsnd.h>
#include <string.h>

#include <dw/file_queue.h>
#include <dw/main.h>
#include <dw/sound.h>
#include <dw/sound_async.h>
#include <dw/species.h>
#include <dw/types.h>
#include <dw/version.h>

#include "common.h"

typedef struct {
	uint8_t *buffer;
	int16_t vabId;
	int8_t isLoading;
	int8_t pad;
} SoundBuffer;

SoundBuffer SOUND_BUFFERS[NUM_SOUND_BUFFERS] = {
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
	{ NULL, 0, 1, 0 },
};

char SOUND_SB_PATH[] = "SOUND\\SB";

int32_t LOAD_SOUND_COMPLETE_STATE;

void loadFullVHB(int32_t vabId, char *path, uint8_t *buffer);
void concatStrings2(char *dst, char *s1, char *s2);
int32_t loadSoundFinishCallback(void *param);
void loadVHBFile(int32_t vabId, char *path, uint8_t *buffer, int32_t offset,
		 int32_t sectors);
int32_t loadSoundCompleteCallback(void *param);
void uploadSoundBuffer(int32_t vabId);

static void *sound_async_functions[] = {
	waitForSoundBufferLoading,
	loadVLALL,
	isSoundLoaded,
	loadMapSounds2,
	readVBALLSection,
	loadSB,
	isSoundBufferLoading,
	uploadSoundBuffer,
	loadSoundCompleteCallback,
	loadVHBFile,
#if VERSION_IS(EU)
	concatStrings2,
	loadSoundFinishCallback,
#else
	loadSoundFinishCallback,
	concatStrings2,
#endif
	loadFullVHB,
};

void loadFullVHB(int32_t vabId, char *path, uint8_t *buffer)
{
	SoundBuffer *sb;
	char *filename;
	char pathBuf[64];

	sb = &SOUND_BUFFERS[vabId];

	if (NULL == (filename = strrchr(path, '\\'))) {
		filename = path;
	} else {
		++filename;
	}

	concatStrings2(pathBuf, filename, ".VHB");
	sb->buffer = buffer;
	addFileReadRequestLookup(pathBuf, buffer, (uint8_t *)&sb->isLoading,
				 loadSoundFinishCallback, (void *)vabId);
}

void concatStrings2(char *dst, char *s1, char *s2)
{
	while (*s1) {
		*dst++ = *s1++;
	}

	while (*s2) {
		*dst++ = *s2++;
	}

	*dst = 0;
}

int32_t loadSoundFinishCallback(void *param)
{
	LOAD_SOUND_COMPLETE_STATE = 0;
	setFileReadCallback2(loadSoundCompleteCallback, param);
}

void loadVHBFile(int32_t vabId, char *path, uint8_t *buffer, int32_t offset,
		 int32_t sectors)
{
	SoundBuffer *sb;
	char *filename;
	char pathBuf[64];

	sb = &SOUND_BUFFERS[vabId];

	if (NULL == (filename = strrchr(path, '\\'))) {
		filename = path;
	} else {
		++filename;
	}

	concatStrings2(pathBuf, filename, ".VHB");

	sb->buffer = buffer;
	addFileReadRequestSection(pathBuf, buffer, offset, sectors,
				  (uint8_t *)&sb->isLoading,
				  loadSoundFinishCallback, (void *)vabId);
}

int32_t loadSoundCompleteCallback(void *param)
{
	SoundBuffer *sb;
	uint8_t *buffer;
	unsigned long addr;

	sb = &SOUND_BUFFERS[(int32_t)param];
	buffer = sb->buffer;

	switch (LOAD_SOUND_COMPLETE_STATE) {
	case 0:
		sb->vabId = 0;
		/* MOD (sound limits, step 3): checked, allocated in SPU RAM */
		if ((sb->vabId = openSoundBank((int32_t)param, buffer)) < 0) {
			sb->vabId = -1;
			return 0;
		}

		LOAD_SOUND_COMPLETE_STATE = 4;
		return 1;
	case 4:
		if (SsVabTransCompleted(0)) {
			LOAD_SOUND_COMPLETE_STATE = 5;
		}
		return 1;
	case 5:
		addr = SsUtGetVBaddrInSB(sb->vabId);
		return 0;
	}
}

void uploadSoundBuffer(int32_t vabId)
{
	SoundBuffer *sb;
	uint8_t *buffer;
	unsigned long addr;

	sb = &SOUND_BUFFERS[vabId];
	buffer = sb->buffer;
	sb->vabId = 0;
	/* MOD (sound limits, step 3): checked, allocated in SPU RAM */
	if ((sb->vabId = openSoundBank(vabId, buffer)) < 0) {
		sb->vabId = -1;
		return;
	}

	SsVabTransCompleted(1);
	addr = SsUtGetVBaddrInSB(sb->vabId);
}

int32_t isSoundBufferLoading(int32_t vabId)
{
	SoundBuffer *sb;

	sb = &SOUND_BUFFERS[vabId];

	return sb->isLoading;
}

int32_t loadSB(void)
{
	ACTIVE_MAP_SOUND_ID = -1;

	loadFullVHB(8, SOUND_SB_PATH, GENERAL_BUFFER);

	return 8;
}

int32_t readVBALLSection(int32_t vabId, int32_t idx)
{
	SoundBuffer *sb;
	int32_t soundId;
	int32_t sector;
	int32_t sectors;

	sb = &SOUND_BUFFERS[vabId];
	if ((vabId < 4) || (7 < vabId)) {
		sb->vabId = -1;
	}

	/* MOD (sound limits): the index instead of soundId * 7 */
	soundId = getDigimonVballId(idx); /* MOD (species) */
	if (!getSoundChunk(SOUND_FILE_VBALL, soundId, &sector, &sectors)) {
		sb->vabId = -1;
		return vabId;
	}

	loadVHBFile(vabId, "VBALL", GENERAL_BUFFER, sector, sectors);

	return vabId;
}

int32_t loadMapSounds2(int32_t mapSoundId)
{
	int32_t sector;
	int32_t sectors;

	if (ACTIVE_MAP_SOUND_ID == mapSoundId) {
		return 1;
	}

	ACTIVE_MAP_SOUND_ID = mapSoundId;

	/* MOD (sound limits): the index instead of MAP_SOUND_PARA */
	if (!getSoundChunk(SOUND_FILE_ESALL, mapSoundId, &sector, &sectors)) {
		SOUND_BUFFERS[8].vabId = -1;
		return 8;
	}

	loadVHBFile(8, "ESALL", GENERAL_BUFFER, sector, sectors);

	return 8;
}

int32_t isSoundLoaded(int32_t mode, int32_t vabId)
{
	SoundBuffer *sb;

	sb = &SOUND_BUFFERS[vabId];

	if (mode == 0) {
		while (sb->isLoading) {
			tickFileReadQueue(0);
		}
	} else if (sb->isLoading) {
		return -1;
	}

	if (sb->vabId == -1) {
		return 0;
	}

	return 1;
}

int32_t loadVLALL(int32_t idx, uint8_t *buffer)
{
	uint32_t soundId;
	int32_t vabId;
	char *path;
	uint8_t *vhbBuffer;
	char pathBuf[64];
	SoundBuffer *sb;
	char *filename;
	int32_t sector;
	int32_t sectors;

	readVBALLSection(4, idx);

	if ((idx >= 1) && (idx < NUM_DIGIMON)) { /* MOD: was idx <= 0x41 */
		soundId = DIGIMON_VLALL_SOUND_ID[idx];
	} else {
		soundId = 0xf;
	}

	vabId = 3;
	path = "VLALL";
	vhbBuffer = buffer;
	sb = &SOUND_BUFFERS[vabId];

	if (NULL == (filename = strrchr(path, '\\'))) {
		filename = path;
	} else {
		++filename;
	}

	concatStrings2(pathBuf, filename, ".VHB");

	sb->buffer = vhbBuffer;
	sb->buffer = vhbBuffer;

	/* MOD (sound limits): the index instead of soundId * 15 */
	if (!getSoundChunk(SOUND_FILE_VLALL, soundId, &sector, &sectors)) {
		sb->vabId = -1;
		return 3;
	}

	addFileReadRequestSection(pathBuf, GENERAL_BUFFER, sector, sectors,
				  (uint8_t *)&sb->isLoading, NULL, 0);

	return 3;
}

void waitForSoundBufferLoading(int32_t vabId)
{
	SoundBuffer *sb;

	sb = &SOUND_BUFFERS[vabId];
	while (sb->isLoading) {
		tickFileReadQueue(0);
	}

	uploadSoundBuffer(vabId);
}
