#ifndef DW_SOUND_H
#define DW_SOUND_H

#include <dw/ids.h>
#include <dw/types.h>

#define NUM_SOUND_BUFFERS	10

typedef struct {
	uint32_t vhOffset;
	uint32_t vbOffset;
} VhbFileHeader;

typedef struct {
	uint16_t sectorId;
	uint16_t sectorCount;
} MapSoundPara;

/*
 * MOD (sound limits, docs/mods/sound.md § 8): the chunks of the multi-chunk
 * sound files are found through an index instead of fixed sector strides.
 * A file written by tools/sound.py starts with an index sector:
 *   u32 magic "SIDX", u16 count, u16 index sectors,
 *   then count x SoundChunk (sector from the start of the file, length).
 * Files without it (the original disc) get the original layout: strides of
 * 39 (FAALL), 7 (VBALL) and 15 (VLALL) sectors and MAP_SOUND_PARA (ESALL).
 */
#define SOUND_INDEX_MAGIC	0x58444953	/* "SIDX" */

enum {
	SOUND_FILE_FAALL,	/* music fonts: chunk = font - 1 */
	SOUND_FILE_VBALL,	/* short voices: DIGIMON_VBALL_SOUND_ID */
	SOUND_FILE_VLALL,	/* long voices: DIGIMON_VLALL_SOUND_ID */
	SOUND_FILE_ESALL,	/* map effects: MAP_ENTRIES.flags & 0x1F */
	NUM_SOUND_INDEXED_FILES
};

/* Capacity of the index tables in RAM (4 bytes per entry) */
#define SOUND_MAX_FONTS		254	/* fonts 1..254; 0xFF stops the music */
#define SOUND_MAX_VBALL		256	/* u8 ids in DIGIMON_VBALL_SOUND_ID */
#define SOUND_MAX_VLALL		256	/* u8 ids in DIGIMON_VLALL_SOUND_ID */
#define SOUND_MAX_ESALL		32	/* 5-bit MAP_ENTRIES.flags field */

typedef struct {
	uint16_t sector;
	uint16_t sectors;
} SoundChunk;

extern int16_t SOUND_CHUNK_COUNT[];

/*
 * MOD (sound limits, step 3): SPU RAM is allocated per load instead of fixed
 * slots. VHB_SOUNDBUFFER_START[bank] is the bank's current address and
 * SPU_BANK_SIZE[bank] its bytes (0 = not loaded). Bank 8 (map effects /
 * battle) grows down from the reverb area; the others keep their original
 * slot when they fit there and otherwise take the lowest free space.
 */
extern unsigned long SPU_BANK_SIZE[];
extern int32_t SPU_ALLOC_FAILS;

int32_t openSoundBank(int32_t vabId, uint8_t *buffer);

void loadSoundIndex(void);
int32_t getSoundChunk(int32_t file, int32_t id, int32_t *sector, int32_t *sectors);

extern void *VHB_HEADER_ADDR[];
extern unsigned long VHB_SOUNDBUFFER_START[];
extern int32_t ACTIVE_MAP_SOUND_ID;
extern MapSoundPara MAP_SOUND_PARA[];
extern uint8_t DIGIMON_VBALL_SOUND_ID[];
extern uint8_t DIGIMON_VLALL_SOUND_ID[NUM_DIGIMON];

int32_t initializeMusic(void);
void finalizeMusic(void);

int32_t loadMapSounds(int32_t mapSoundId);
int32_t VSLoadSounds(void);
int32_t loadDigimonSounds(int32_t vabId, int32_t type);
int32_t loadPartnerSounds(int32_t type);

uint32_t playSound(int32_t vabId, int32_t val);
uint32_t playSound2(int32_t vabId, int32_t val);
void stopSound(void);
void thunkStopSoundMask(uint32_t mask);

int32_t playMusic(int32_t font, int32_t track);
void stopBGM(void);

#endif
