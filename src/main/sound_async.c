#include <libsnd.h>
#include <string.h>

#include <dw/file_queue.h>
#include <dw/main.h>
#include <dw/mod_options.h> /* MOD (post-battle stall) */
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

	soundId = getPartnerVlallSoundId(idx); /* MOD (scale): 0xf outside the partner ids */

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

/* MOD (quick battle start): playMusic without the wait. The music font is read through the queue like the other
 * banks (staged at GENERAL_BUFFER, opened into bank 2 by loadSoundCompleteCallback) and the sequence starts when it
 * is in, from the copy of the font still at GENERAL_BUFFER; until then nothing plays (the font being replaced is the
 * one the old sequence was playing). */
extern int16_t CURRENT_SEQ_FONT;
extern int16_t CURRENT_SEQ_TRACK;
extern uint8_t SEQ_BUFFER[];
void seqOpen(void);
void seqPlay(void);
void seqStop(void);
void seqClose(void);

static int16_t modMusicFont;
static int16_t modMusicTrack;
static uint8_t modMusicLoading;
static int8_t modFieldMusic;	/* MOD (post-battle stall): the font being read is the field's, after a battle */
static uint32_t modMusicEnd;	/* where the staging at GENERAL_BUFFER of the font being read ends */

static int32_t modMusicComplete(void *param)
{
	uint32_t *buf;

	if (loadSoundCompleteCallback(param) != 0) {
		return 1;
	}
	buf = (uint32_t *)GENERAL_BUFFER;
	if ((SOUND_BUFFERS[2].vabId < 0) || (buf[3] - buf[2] > SEQ_BUFFER_SIZE)) {
		return 0; /* as loadMusicFont: a font that does not load plays nothing */
	}
	memcpy(SEQ_BUFFER, (uint8_t *)buf + ((buf[2] >> 2) << 2), buf[3] - buf[2]);
	CURRENT_SEQ_FONT = modMusicFont;
	CURRENT_SEQ_TRACK = modMusicTrack;
	seqOpen();
	seqPlay();
	return 0;
}

static int32_t modMusicFinish(void *param)
{
	LOAD_SOUND_COMPLETE_STATE = 0;
	setFileReadCallback2(modMusicComplete, param);
	return 0;
}

/*
 * MOD (post-battle stall): the field's font after a battle is read through the queue while the world runs. Nothing may
 * play music, stage anything over it or read it from the disc in between: playMusic, the next modPlayMusicAsync and the
 * synchronous readers into its staging (tfsStreamCheckTarget) wait for it to be in and playing first, in the original's order.
 */
int32_t modFieldMusicPending(void)
{
	return (modMusicLoading && modFieldMusic) ? (int32_t)modMusicEnd : 0;
}

void modFieldMusicWait(void)
{
	while (modMusicLoading && modFieldMusic) {
		tickFileReadQueue(0);
	}
	modFieldMusic = 0;
}

void modPlayMusicAsync(int32_t font, int32_t track)
{
	int32_t sector;
	int32_t sectors;

	modFieldMusicWait();
	if ((font == CURRENT_SEQ_FONT) && (track == CURRENT_SEQ_TRACK)) {
		return;
	}
	seqStop();
	seqClose();
	if (font == CURRENT_SEQ_FONT) {
		CURRENT_SEQ_TRACK = track;
		seqOpen();
		seqPlay();
		return;
	}
	CURRENT_SEQ_FONT = -1;
	if (!getSoundChunk(SOUND_FILE_FAALL, font - 1, &sector, &sectors)) {
		return;
	}
	modMusicFont = font;
	modMusicTrack = track;
	modMusicEnd = (uint32_t)GENERAL_BUFFER + (sectors << 11);
	SOUND_BUFFERS[2].buffer = GENERAL_BUFFER;
	addFileReadRequestSection("FAALL.VHB", GENERAL_BUFFER, sector, sectors, &modMusicLoading,
				  modMusicFinish, (void *)2);
}

extern uint8_t ACTIVE_BGM_FONT;
extern uint8_t ACTIVE_BGM_TRACK;
void handleMusicOverride(uint8_t *outFont, uint8_t *outVariant);
void playBGM(int32_t bgmId);
void stopBGM(void);

/* MOD (post-battle stall): playBGM(ACTIVE_BGM_FONT) for the way back from a battle, without the wait for the font when
 * QUICK BATTLES is on (the original reads it before the world runs again, a seek and 27 sectors) */
void modResumeFieldMusic(void)
{
	uint8_t font;
	uint8_t variant;

	font = ACTIVE_BGM_FONT;
	if (font != 0xff) {
		handleMusicOverride(&font, &variant);
	}
	/* the same font needs no read: playBGM keeps (or restarts) it as the original did */
	if (!MOD_OPT_QUICK_BATTLE || ACTIVE_BGM_FONT == 0xff || font == CURRENT_SEQ_FONT) {
		playBGM(ACTIVE_BGM_FONT);
		return;
	}
	ACTIVE_BGM_FONT = font;
	ACTIVE_BGM_TRACK = variant;
	stopBGM();
	modPlayMusicAsync(font, variant);
	modFieldMusic = modMusicLoading;
}
