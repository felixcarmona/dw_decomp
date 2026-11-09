#include <libetc.h>
#include <libsnd.h>
#include <libspu.h>
#include <string.h>

#include <dw/file.h>
#include <dw/main.h>
#include <dw/params.h>
#include <dw/sound.h>
#include <dw/species.h>
#include <dw/types.h>

#define VAB_MAX_PROGRAMS	128
#define VAB_TONES_PER_PROGRAM	16

typedef struct {
	VabHdr hdr;
	ProgAtr programs[VAB_MAX_PROGRAMS];
	VagAtr tones[][VAB_TONES_PER_PROGRAM];
} VhbFile;

uint8_t VHB_HEADER_SS[0x1000];
uint8_t VHB_HEADER_SL[0x1000];
uint8_t VHB_HEADER_FAALL[0x2000];
uint8_t VHB_HEADER_VLALL[0x1000];
uint8_t VHB_HEADER_VBALL_0[0x1000];
uint8_t VHB_HEADER_VBALL_1[0x1000];
uint8_t VHB_HEADER_VBALL_2[0x1000];
uint8_t VHB_HEADER_VBALL_3[0x1000];
uint8_t VHB_HEADER_ESALL_SB[0x1800];

void *VHB_HEADER_ADDR[NUM_SOUND_BUFFERS] = {
	VHB_HEADER_SS,
	VHB_HEADER_SL,
	VHB_HEADER_FAALL,
	VHB_HEADER_VLALL,
	VHB_HEADER_VBALL_0,
	VHB_HEADER_VBALL_1,
	VHB_HEADER_VBALL_2,
	VHB_HEADER_VBALL_3,
	VHB_HEADER_ESALL_SB,
	NULL,
};

unsigned long VHB_SOUNDBUFFER_START[NUM_SOUND_BUFFERS] = {
	0x1010,
	0xe800,
	0x1b800,
	0x2a000,
	0x31800,
	0x34000,
	0x36800,
	0x39000,
	0x3b800,
	0x0,
};

uint8_t DIGIMON_VLALL_SOUND_ID[NUM_DIGIMON] = { /* MOD: was NUM_PARTNER_DIGIMON */
	0x00,	/* main character */
	0x00,	/* Botamon */
	0x00,	/* Koromon */
	0x01,	/* Agumon */
	0x01,	/* Betamon */
	0x02,	/* Greymon */
	0x03,	/* Devimon */
	0x04,	/* Airdramon */
	0x02,	/* Tyrannomon */
	0x0b,	/* Meramon */
	0x05,	/* Seadramon */
	0x06,	/* Numemon */
	0x0a,	/* MetalGreymon */
	0x00,	/* Mamemon */
	0x0f,	/* Monzaemon */
	0x00,	/* Punimon */
	0x00,	/* Tsunomon */
	0x01,	/* Gabumon */
	0x01,	/* Elecmon */
	0x08,	/* Kabuterimon */
	0x0b,	/* Angemon */
	0x04,	/* Birdramon */
	0x07,	/* Garurumon */
	0x03,	/* Frigimon */
	0x05,	/* Whamon */
	0x0e,	/* Vegiemon */
	0x02,	/* SkullGreymon */
	0x00,	/* MetalMamemon */
	0x0a,	/* Vademon */
	0x00,	/* Poyomon */
	0x00,	/* Tokomon */
	0x01,	/* Patamon */
	0x01,	/* Kunemon */
	0x09,	/* Unimon */
	0x03,	/* Ogremon */
	0x06,	/* Shellmon */
	0x09,	/* Centarumon */
	0x00,	/* Bakemon */
	0x07,	/* Drimogemon */
	0x06,	/* Sukamon */
	0x0a,	/* Andromon */
	0x0a,	/* Giromon */
	0x0c,	/* Etemon */
	0x00,	/* Yuramon */
	0x00,	/* Tanemon */
	0x01,	/* Biyomon */
	0x01,	/* Palmon */
	0x07,	/* Monochromon */
	0x03,	/* Leomon */
	0x05,	/* Coelamon */
	0x04,	/* Kokatorimon */
	0x08,	/* Kuwagamon */
	0x03,	/* Mojyamon */
	0x03,	/* Nanimon */
	0x02,	/* Megadramon */
	0x00,	/* Piximon */
	0x01,	/* Digitamamon */
	0x01,	/* Penguinmon */
	0x00,	/* Ninjamon */
	0x04,	/* Phoenixmon */
	0x08,	/* H-Kabuterimon */
	0x05,	/* MegaSeadramon */
	0x07,	/* WereGarurumon */
	0x03,	/* Panjyamon */
	0x02,	/* Gigadramon */
	0x0c,	/* MetalEtemon */
	/*
	 * MOD (master-tyrannomon): the table covers every type. Types that
	 * were outside it keep the voice the guards gave them (15);
	 * MasterTyrannomon uses Tyrannomon's partner sounds.
	 */
	[NUM_PARTNER_DIGIMON ... NUM_DIGIMON - 1] = 15,
	[DIGIMON_MASTER_TYRANNOMON] = 0x02,
	[DIGIMON_CHERRYMON] = 0x0e, /* MOD (cherrymon): Vegiemon's partner sounds */
	[DIGIMON_KING_SUKAMON] = 0x06, /* MOD (partners-npc-a): Sukamon's partner sounds */
	[DIGIMON_ICEMON] = 0x03, /* MOD (partners-npc-a): Frigimon's partner sounds */
	[DIGIMON_METEORMON] = 0x07, /* MOD (partners-npc-a): Monochromon's partner sounds */
	[DIGIMON_WARUMONZAEMON] = 0x0f, /* MOD (partners-npc-b): Monzaemon's partner sounds */
	[DIGIMON_MYOTISMON] = 0x03,     /* MOD (partners-npc-b): Devimon's partner sounds */
	[DIGIMON_SHOGUNGEKOMON] = 0x05, /* MOD (partners-npc-b): Whamon's partner sounds */
	[DIGIMON_MACHINEDRAMON] = 0x0a, /* MOD (machinedramon): MetalGreymon's partner sounds */
	[DIGIMON_JIJIMON] = 0x03, /* MOD (jijimon): Ogremon's partner sounds */
	[DIGIMON_ANALOGMAN] = 0x0a, /* MOD (analogman): Andromon's partner sounds */
};

uint8_t DIGIMON_VBALL_SOUND_ID[NUM_ORIGINAL_DIGIMON] = {
	0x00,	/* main character */
	0x01,	/* Botamon */
	0x02,	/* Koromon */
	0x03,	/* Agumon */
	0x04,	/* Betamon */
	0x05,	/* Greymon */
	0x06,	/* Devimon */
	0x07,	/* Airdramon */
	0x08,	/* Tyrannomon */
	0x09,	/* Meramon */
	0x0a,	/* Seadramon */
	0x0b,	/* Numemon */
	0x0c,	/* MetalGreymon */
	0x0d,	/* Mamemon */
	0x0e,	/* Monzaemon */
	0x01,	/* Punimon */
	0x02,	/* Tsunomon */
	0x0f,	/* Gabumon */
	0x10,	/* Elecmon */
	0x11,	/* Kabuterimon */
	0x12,	/* Angemon */
	0x13,	/* Birdramon */
	0x14,	/* Garurumon */
	0x15,	/* Frigimon */
	0x16,	/* Whamon */
	0x17,	/* Vegiemon */
	0x18,	/* SkullGreymon */
	0x19,	/* MetalMamemon */
	0x1a,	/* Vademon */
	0x01,	/* Poyomon */
	0x02,	/* Tokomon */
	0x1b,	/* Patamon */
	0x1c,	/* Kunemon */
	0x1d,	/* Unimon */
	0x1e,	/* Ogremon */
	0x1f,	/* Shellmon */
	0x20,	/* Centarumon */
	0x21,	/* Bakemon */
	0x22,	/* Drimogemon */
	0x23,	/* Sukamon */
	0x24,	/* Andromon */
	0x25,	/* Giromon */
	0x26,	/* Etemon */
	0x01,	/* Yuramon */
	0x02,	/* Tanemon */
	0x27,	/* Biyomon */
	0x28,	/* Palmon */
	0x29,	/* Monochromon */
	0x2a,	/* Leomon */
	0x2b,	/* Coelamon */
	0x2c,	/* Kokatorimon */
	0x2d,	/* Kuwagamon */
	0x2e,	/* Mojyamon */
	0x2f,	/* Nanimon */
	0x30,	/* Megadramon */
	0x31,	/* Piximon */
	0x32,	/* Digitamamon */
	0x33,	/* Penguinmon */
	0x34,	/* Ninjamon */
	0x35,	/* Phoenixmon */
	0x36,	/* H-Kabuterimon */
	0x37,	/* MegaSeadramon */
	0x00,	/* WereGarurumon */
	0x2a,	/* Panjyamon */
	0x30,	/* Gigadramon */
	0x26,	/* MetalEtemon */
	0x38,	/* Myotismon */
	0x39,	/* Yanmamon */
	0x3a,	/* Gotsumon */
	0x3b,	/* Flarerizamon */
	0x3c,	/* WaruMonzaemon */
	0x03,	/* SnowAgumon */
	0x1e,	/* Hyogamon */
	0x23,	/* PlatinumSukamon */
	0x1c,	/* Dokunemon */
	0x1d,	/* ShimaUnimon */
	0x3d,	/* Tankmon */
	0x3e,	/* RedVegiemon */
	0x3f,	/* J-Mojyamon */
	0x22,	/* NiseDrimogemon */
	0x40,	/* Goburimon */
	0x41,	/* MudFrigimon */
	0x0f,	/* Psychemon */
	0x04,	/* ModokiBetamon */
	0x03,	/* ToyAgumon */
	0x12,	/* Piddomon */
	0x28,	/* Aruraumon */
	0x0b,	/* Geremon */
	0x29,	/* Vermilimon */
	0x1e,	/* Fugamon */
	0x25,	/* Tekkamon */
	0x1f,	/* MoriShellmon */
	0x24,	/* Guardromon */
	0x33,	/* Muchomon */
	0x3a,	/* Icemon */
	0x2c,	/* Akatorimon */
	0x1b,	/* Tsukaimon */
	0x40,	/* Sharmamon */
	0x03,	/* ClearAgumon */
	0x17,	/* Weedmon */
	0x06,	/* IceDevimon */
	0x3b,	/* Darkrizamon */
	0x39,	/* SandYanmamon */
	0x40,	/* SnowGoburimon */
	0x09,	/* BlueMeramon */
	0x14,	/* Gururumon */
	0x13,	/* Saberdramon */
	0x21,	/* Soulmon */
	0x15,	/* Rockmon */
	0x42,	/* Otamamon */
	0x43,	/* Gekomon */
	0x44,	/* Tentomon */
	0x45,	/* WaruSeadramon */
	0x3a,	/* Meteormon */
	0x46,	/* ? */
	0x30,	/* Machinedramon */
	0x24,	/* Analogman; MOD (analogman): Andromon's voice (was 0x00; his NPC animations use no voice) */
	0x1e,	/* Jijimon; MOD (jijimon): Ogremon's voice (was 0x00; his NPC animations use no voice) */
	0x00,	/* Market Manager */
	0x43,	/* ShogunGekomon; MOD (partners-npc-b): Gekomon's voice (was 0x00) */
	0x23,	/* King of Sukamon; MOD (partners-npc-a): Sukamon's voice (was 0x00) */
	0x17,	/* Cherrymon; MOD (cherrymon): Vegiemon's voice (was 0x00) */
	0x00,	/* Hagurumon */
	0x00,	/* Tinmon */
	0x00,	/* Master Tyrannomon */
	0x00,	/* Goburimon */
	0x00,	/* Brachiomon */
	0x00,	/* DemiMeramon */
	0x04,	/* Betamon */
	0x05,	/* Greymon */
	0x06,	/* Devimon */
	0x07,	/* Airdramon */
	0x08,	/* Tyrannomon */
	0x09,	/* Meramon */
	0x0a,	/* Seadramon */
	0x0b,	/* Numemon */
	0x0c,	/* MetalGreymon */
	0x0d,	/* Mamemon */
	0x0e,	/* Monzaemon */
	0x0f,	/* Gabumon */
	0x10,	/* Elecmon */
	0x11,	/* Kabuterimon */
	0x12,	/* Angemon */
	0x13,	/* Birdramon */
	0x14,	/* Garurumon */
	0x15,	/* Frigimon */
	0x16,	/* Whamon */
	0x17,	/* Vegiemon */
	0x18,	/* SkullGreymon */
	0x19,	/* MetalGreymon */
	0x1a,	/* Vademon */
	0x1b,	/* Patamon */
	0x1c,	/* Kunemon */
	0x1d,	/* Unimon */
	0x1e,	/* Ogremon */
	0x1f,	/* Shellmon */
	0x20,	/* Centarumon */
	0x21,	/* Bakemon */
	0x22,	/* Drimogemon */
	0x23,	/* Sukamon */
	0x24,	/* Andromon */
	0x25,	/* Giromon */
	0x26,	/* Etemon */
	0x27,	/* Biyomon */
	0x28,	/* Palmon */
	0x29,	/* Monochromon */
	0x2a,	/* Leomon */
	0x2b,	/* Coelamon */
	0x2c,	/* Kokatorimon */
	0x2d,	/* Kuwagamon */
	0x2e,	/* Mojyamon */
	0x2f,	/* Nanimon */
	0x30,	/* Megadramon */
	0x31,	/* Piximon */
	0x32,	/* Digitamamon */
	0x34,	/* Ninjamon */
	0x33,	/* Penguinmon */
	0x38,	/* Myotismon */
	0x05,	/* Greymon */
	0x0c,	/* MetalGreymon */
};

MapSoundPara MAP_SOUND_PARA[22] = {
	{ 0x0000, 0x00e8 },
	{ 0x00e8, 0x0064 },
	{ 0x014c, 0x00de },
	{ 0x022a, 0x004c },
	{ 0x0276, 0x00f8 },
	{ 0x036e, 0x00e6 },
	{ 0x0454, 0x00ee },
	{ 0x0542, 0x0046 },
	{ 0x0588, 0x00de },
	{ 0x0666, 0x0086 },
	{ 0x06ec, 0x0038 },
	{ 0x0724, 0x00f8 },
	{ 0x081c, 0x00ee },
	{ 0x090a, 0x00a4 },
	{ 0x09ae, 0x009e },
	{ 0x0a4c, 0x0062 },
	{ 0x0aae, 0x0062 },
	{ 0x0b10, 0x00b2 },
	{ 0x0bc2, 0x003e },
	{ 0x0c00, 0x00bc },
	{ 0x0cbc, 0x007e },
	{ 0x0d3a, 0x00e2 },
};

int32_t FREE_CHANNEL_INDEX;
int16_t SEQ_ACCESS_NUM;
int16_t CURRENT_SEQ_FONT;
int16_t CURRENT_SEQ_TRACK;

/* MOD (sound limits, step 3): SPU allocation, see dw/sound.h */
unsigned long SPU_BANK_SIZE[NUM_SOUND_BUFFERS];
int32_t SPU_ALLOC_FAILS;

/* the original fixed slots: where each bank goes when it fits there */
static unsigned long SPU_HOME[NUM_SOUND_BUFFERS] = {
	0x1010, 0xe800, 0x1b800, 0x2a000, 0x31800, 0x34000, 0x36800, 0x39000, 0, 0,
};

/* size of each VHB_HEADER_* buffer: a VAB header must fit (was unchecked) */
static uint16_t VHB_HEADER_SIZE[NUM_SOUND_BUFFERS] = {
	0x1000, 0x1000, 0x2000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1800, 0,
};

char SEQ_TABLE[SS_SEQ_TABSIZ];
uint8_t SEQ_BUFFER[SEQ_BUFFER_SIZE]; /* MOD (sound limits): was 0x5800, dw/memmap.h */

char VHB_EXT[] = ".VHB";

int32_t ACTIVE_MAP_SOUND_ID = -1;

/* MOD (sound limits): chunk index, see dw/sound.h */
SoundChunk SOUND_CHUNKS_FAALL[SOUND_MAX_FONTS];
SoundChunk SOUND_CHUNKS_VBALL[SOUND_MAX_VBALL];
SoundChunk SOUND_CHUNKS_VLALL[SOUND_MAX_VLALL];
SoundChunk SOUND_CHUNKS_ESALL[SOUND_MAX_ESALL];
int16_t SOUND_CHUNK_COUNT[NUM_SOUND_INDEXED_FILES];
int32_t SOUND_INDEX_LOADED;

typedef struct {
	char *name;
	SoundChunk *chunks;
	int16_t cap;
	int16_t stride;		/* original layout: sectors per chunk, 0 = MAP_SOUND_PARA */
} SoundIndexFile;

static SoundIndexFile SOUND_INDEX_FILES[NUM_SOUND_INDEXED_FILES] = {
	{ "FAALL.VHB", SOUND_CHUNKS_FAALL, SOUND_MAX_FONTS, 39 },
	{ "VBALL.VHB", SOUND_CHUNKS_VBALL, SOUND_MAX_VBALL, 7 },
	{ "VLALL.VHB", SOUND_CHUNKS_VLALL, SOUND_MAX_VLALL, 15 },
	{ "ESALL.VHB", SOUND_CHUNKS_ESALL, SOUND_MAX_ESALL, 0 },
};

/*
 * Reads the index sector of every multi-chunk file (once; GENERAL_BUFFER is
 * free at boot) or, for a file without one, rebuilds the original layout.
 */
void loadSoundIndex(void)
{
	SoundIndexFile *f;
	uint32_t *hdr;
	int32_t i;
	int32_t k;
	int32_t count;

	if (SOUND_INDEX_LOADED) {
		return;
	}
	SOUND_INDEX_LOADED = 1;

	hdr = (uint32_t *)GENERAL_BUFFER;
	for (k = 0; k < NUM_SOUND_INDEXED_FILES; ++k) {
		f = &SOUND_INDEX_FILES[k];
		hdr[0] = 0;
		readFileSectors(f->name, GENERAL_BUFFER, 0, 1);
		if (hdr[0] == SOUND_INDEX_MAGIC) {
			count = hdr[1] & 0xffff;
			if ((hdr[1] >> 16) > 1) {
				readFileSectors(f->name, GENERAL_BUFFER, 0, hdr[1] >> 16);
			}
			if (count > f->cap) {
				count = f->cap;
			}
			memcpy(f->chunks, &hdr[2], count * sizeof(SoundChunk));
		} else if (f->stride != 0) {
			count = lookupFileSize(f->name) / (f->stride * 0x800);
			if (count > f->cap) {
				count = f->cap;
			}
			for (i = 0; i < count; ++i) {
				f->chunks[i].sector = i * f->stride;
				f->chunks[i].sectors = f->stride;
			}
		} else {
			count = sizeof(MAP_SOUND_PARA) / sizeof(MAP_SOUND_PARA[0]);
			for (i = 0; i < count; ++i) {
				f->chunks[i].sector = MAP_SOUND_PARA[i].sectorId / 2;
				f->chunks[i].sectors = MAP_SOUND_PARA[i].sectorCount / 2;
			}
		}
		SOUND_CHUNK_COUNT[k] = count;
	}
}

/* Where chunk `id` of an indexed file is; 0 if there is no such chunk */
int32_t getSoundChunk(int32_t file, int32_t id, int32_t *sector, int32_t *sectors)
{
	SoundChunk *c;

	if ((id < 0) || (id >= SOUND_CHUNK_COUNT[file])) {
		return 0;
	}

	c = &SOUND_INDEX_FILES[file].chunks[id];
	if (c->sectors == 0) {
		return 0;
	}

	*sector = c->sector;
	*sectors = c->sectors;
	return 1;
}

int32_t readVHBFile(int32_t vabId, char *filename, uint8_t *buffer);
int32_t readVHBFileSectors(int32_t vabId, char *filename, uint8_t *buffer,
			   int32_t offset, int32_t sectors);
int32_t loadMusicFont(int32_t font);
uint32_t getNextFreeChannel(int32_t arg);
uint32_t startSound(int32_t vabId, char prog, char note);
void seqOpen(void);
void seqPlay(void);
void seqStop(void);
void seqClose(void);
void concatStrings3(char *dst, char *src1, char *src2);
void stopSoundMask(uint32_t mask);

static void *sound_functions[] = {
	concatStrings3,
	loadMusicFont,
	seqPlay,
	seqOpen,
	readVHBFileSectors,
	seqClose,
	seqStop,
	readVHBFile,
};

static void *sound_bss[] = {
	SEQ_BUFFER,
	SEQ_TABLE,
	&CURRENT_SEQ_TRACK,
	&CURRENT_SEQ_FONT,
	&SEQ_ACCESS_NUM,
	&FREE_CHANNEL_INDEX,
};

static int32_t spuFits(int32_t bank, unsigned long addr, unsigned long size)
{
	int32_t i;

	if ((size > SPU_REVERB_START - SPU_BANKS_START) || (addr < SPU_BANKS_START) ||
	    (addr > SPU_REVERB_START - size)) {
		return 0;
	}

	for (i = 0; i < NUM_SOUND_BUFFERS - 1; ++i) {
		if ((i != bank) && (SPU_BANK_SIZE[i] != 0) &&
		    (addr < VHB_SOUNDBUFFER_START[i] + SPU_BANK_SIZE[i]) &&
		    (VHB_SOUNDBUFFER_START[i] < addr + size)) {
			return 0;
		}
	}

	return 1;
}

/*
 * Where bank `bank` puts a body of `size` bytes (its previous body is given
 * up). Bank 8 goes as high as possible (ending at the reverb area); the
 * others go to their original slot if nothing is there, else to the lowest
 * free space. With the original files every bank fits its original slot, so
 * nothing ever moves except bank 8, which stays inside its original slot.
 * 0 = no room: the bank is left unloaded (the original overwrote the next
 * bank instead).
 */
static unsigned long spuFindPlace(int32_t bank, unsigned long size)
{
	unsigned long best;
	unsigned long cand;
	int32_t i;

	cand = (bank == 8) ? SPU_REVERB_START - size : SPU_HOME[bank];
	if (spuFits(bank, cand, size)) {
		best = cand;
	} else {
		best = 0;
		for (i = -1; i < NUM_SOUND_BUFFERS - 1; ++i) {
			if (bank == 8) {
				/* below the reverb area or below another bank */
				cand = (i < 0) ? SPU_REVERB_START : VHB_SOUNDBUFFER_START[i];
				if ((i >= 0) && (SPU_BANK_SIZE[i] == 0)) {
					continue;
				}
				cand = (cand - size) & ~15;
				if (spuFits(bank, cand, size) && (cand > best)) {
					best = cand;
				}
			} else {
				/* at the bottom or right after another bank */
				if ((i >= 0) && (SPU_BANK_SIZE[i] == 0)) {
					continue;
				}
				cand = (i < 0) ? SPU_BANKS_START
					       : VHB_SOUNDBUFFER_START[i] + SPU_BANK_SIZE[i];
				if (spuFits(bank, cand, size) && ((best == 0) || (cand < best))) {
					best = cand;
				}
			}
		}
	}

	return best;
}

unsigned long spuPlaceBank(int32_t bank, unsigned long size)
{
	unsigned long best;

	size = (size + 15) & ~15;
	if (size == 0) {
		size = 16;
	}
	SPU_BANK_SIZE[bank] = 0;

	best = spuFindPlace(bank, size);

	/*
	 * No room. Two banks can make way because the game reloads them by
	 * itself right after: the map effects region (bank 8 outside battles,
	 * reloaded by loadMap) when another bank grows on a screen change, and
	 * a music font that overflowed its original slot when the battle bank
	 * (SB) needs the space (the battle font replaces it next). With the
	 * original files neither ever happens.
	 */
	if ((best == 0) && (bank != 8) && (SPU_BANK_SIZE[8] != 0) && (ACTIVE_MAP_SOUND_ID >= 0)) {
		SsVabClose(8);
		SPU_BANK_SIZE[8] = 0;
		ACTIVE_MAP_SOUND_ID = -1;
		best = spuFindPlace(bank, size);
	}
	if ((best == 0) && (bank == 8) && (SPU_BANK_SIZE[2] != 0) &&
	    (VHB_SOUNDBUFFER_START[2] != SPU_HOME[2])) {
		seqStop();
		seqClose();
		SsVabClose(2);
		SPU_BANK_SIZE[2] = 0;
		CURRENT_SEQ_FONT = -1;
		best = spuFindPlace(bank, size);
	}

	if (best == 0) {
		++SPU_ALLOC_FAILS;
		return 0;
	}

	VHB_SOUNDBUFFER_START[bank] = best;
	SPU_BANK_SIZE[bank] = size;
	return best;
}

/*
 * The part common to every VHB load (sync and async): checks the chunk,
 * copies the VAB header to the bank's buffer, places the body in SPU RAM and
 * starts its transfer. Returns the libsnd vab id or -1; the caller waits for
 * the transfer as before.
 */
int32_t openSoundBank(int32_t vabId, uint8_t *buffer)
{
	uint32_t *w;
	uint32_t hdrSize;
	uint32_t bodySize;
	uint32_t fsize;
	unsigned long addr;
	short vabid;

	w = (uint32_t *)buffer;
	hdrSize = w[1] - w[0];
	bodySize = w[2] - w[1];

	SsVabClose(vabId);
	SPU_BANK_SIZE[vabId] = 0;

	/* a chunk header (0x0C or 0x10 bytes), then "pBAV" and a header that
	 * fits the bank's buffer (the original copied it unchecked) */
	if (((w[0] != 0xc) && (w[0] != 0x10)) || (w[1] <= w[0]) || (w[2] < w[1]) ||
	    (*(uint32_t *)&buffer[w[0]] != 0x56414270) ||
	    (hdrSize > VHB_HEADER_SIZE[vabId])) {
		++SPU_ALLOC_FAILS;
		return -1;
	}

	memcpy(VHB_HEADER_ADDR[vabId], &buffer[(w[0] >> 2) << 2], hdrSize);

	/* VabHdr.fsize = header + body: libsnd may transfer that much */
	fsize = *(uint32_t *)&buffer[w[0] + 0xc];
	if ((fsize > hdrSize) && (fsize - hdrSize > bodySize)) {
		bodySize = fsize - hdrSize;
	}

	if ((addr = spuPlaceBank(vabId, bodySize)) == 0) {
		return -1;
	}

	if ((vabid = SsVabOpenHeadSticky(VHB_HEADER_ADDR[vabId], vabId, addr)) < 0) {
		SPU_BANK_SIZE[vabId] = 0;
		return -1;
	}

	if (SsVabTransBody(&buffer[w[1]], vabid) != vabid) {
		SPU_BANK_SIZE[vabId] = 0;
		return -1;
	}

	return vabid;
}

int32_t readVHBFile(int32_t vabId, char *filename, uint8_t *buffer)
{
	short vabid;
	char *str;
	char pathBuf[64];
	unsigned long addr;

	if (NULL == (str = strrchr(filename, '\\'))) {
		str = filename;
	} else {
		++str;
	}

	concatStrings3(pathBuf, str, VHB_EXT);
	readFile(pathBuf, buffer);
	/* MOD (sound limits, step 3): checked, allocated in SPU RAM */
	if ((vabid = openSoundBank(vabId, buffer)) < 0) {
		return -1;
	}

	SsVabTransCompleted(SS_WAIT_COMPLETED);
	addr = SsUtGetVBaddrInSB(vabid);

	return vabid;
}

void seqStop(void)
{
	if (SEQ_ACCESS_NUM == -1) {
		return;
	}

	SsSeqStop(SEQ_ACCESS_NUM);
}

void seqClose(void)
{
	if (SEQ_ACCESS_NUM == -1) {
		return;
	}

	SsSeqClose(SEQ_ACCESS_NUM);
	SEQ_ACCESS_NUM = -1;
}

int32_t readVHBFileSectors(int32_t vabId, char *filename, uint8_t *buffer,
			   int32_t offset, int32_t sectors)
{
	short vabid;
	char *str;
	char pathBuf[64];
	unsigned long addr;

	if (NULL == (str = strrchr(filename, '\\'))) {
		str = filename;
	} else {
		++str;
	}

	concatStrings3(pathBuf, str, VHB_EXT);
	readFileSectors(pathBuf, buffer, offset, sectors);
	/* MOD (sound limits, step 3): checked, allocated in SPU RAM */
	if ((vabid = openSoundBank(vabId, buffer)) < 0) {
		return -1;
	}

	SsVabTransCompleted(SS_WAIT_COMPLETED);
	addr = SsUtGetVBaddrInSB(vabid);

	return vabid;
}

void seqOpen(void)
{
	uint32_t *buf;

	buf = (uint32_t *)SEQ_BUFFER;
	if ((buf[0] >> 2) <= CURRENT_SEQ_TRACK) {
		CURRENT_SEQ_TRACK = 0;
	}

	SEQ_ACCESS_NUM =
		SsSeqOpen((unsigned long *)((uint8_t *)buf + buf[CURRENT_SEQ_TRACK]), 2);
}

void seqPlay(void)
{
	if (SEQ_ACCESS_NUM == -1) {
		return;
	}

	SsSeqSetVol(SEQ_ACCESS_NUM, 0x50, 0x50);

	if (((CURRENT_SEQ_FONT == 0x1d) && (CURRENT_SEQ_TRACK != 1)) ||
	    ((CURRENT_SEQ_FONT == 0x1e) && (CURRENT_SEQ_TRACK != 2)) ||
	    ((CURRENT_SEQ_FONT == 0x1f) && (CURRENT_SEQ_TRACK != 2)) ||
	    ((CURRENT_SEQ_FONT == 0x20) && (CURRENT_SEQ_TRACK != 2))) {
		SsSeqPlay(SEQ_ACCESS_NUM, SSPLAY_PLAY, 1);
	} else {
		SsSeqPlay(SEQ_ACCESS_NUM, SSPLAY_PLAY, SSPLAY_INFINITY);
	}
}

void concatStrings3(char *dst, char *src1, char *src2)
{
	while (*src1 != 0) {
		*dst++ = *src1++;
	}
	while (*src2 != 0) {
		*dst++ = *src2++;
	}
	*dst = 0;
}

int32_t initializeMusic(void)
{
	SpuReverbAttr attr;

	ResetCallback();
	SsInit();
	SsSetTableSize(SEQ_TABLE, 1, 1);
	SsSetTickMode(SS_TICK240);

	/* MOD (sound limits, step 3): every bank starts unloaded */
	memset(SPU_BANK_SIZE, 0, sizeof(SPU_BANK_SIZE));

	if (readVHBFile(0, "SOUND\\SS", GENERAL_BUFFER) == -1) {
		return 0;
	}

	if (readVHBFile(1, "SOUND\\SL", GENERAL_BUFFER) == -1) {
		return 0;
	}

	loadSoundIndex(); /* MOD (sound limits) */

	SsStart();
	SsSetMVol(127, 127);

	SEQ_ACCESS_NUM = -1;
	CURRENT_SEQ_FONT = -1;

	attr.mask = SPU_REV_MODE | SPU_REV_DEPTHL | SPU_REV_DEPTHR;
	attr.mode = SPU_REV_MODE_CLEAR_WA | SPU_REV_MODE_STUDIO_B;
	attr.depth.left = attr.depth.right = 0x7000;
	SpuSetReverbModeParam(&attr);
	SpuClearReverbWorkArea(SPU_REV_MODE_STUDIO_B);
	SpuSetReverbDepth(&attr);
	SpuSetReverbVoice(SPU_ON, 0x7fffff);
	SpuSetReverb(SPU_ON);

	return 1;
}

void finalizeMusic(void)
{
	int32_t i;

	seqStop();
	seqClose();

	for (i = 0; i < NUM_SOUND_BUFFERS; ++i) {
		SsVabClose(i);
		SPU_BANK_SIZE[i] = 0; /* MOD (sound limits) */
	}

	SsEnd();
	SsQuit();
}

uint32_t getNextFreeChannel(int32_t arg)
{
	int32_t i;
	int32_t val;
	int32_t status;

	for (i = 0; i < 14; ++i) {
		val = (i + FREE_CHANNEL_INDEX) % 14 + 10;
		status = SpuGetKeyStatus(1 << val);
		if (status != SPU_ON) {
			break;
		}
	}

	if (i == 14) {
		val = FREE_CHANNEL_INDEX + 10;
	}

	FREE_CHANNEL_INDEX = (val - 9) % 14;

	return val;
}

static void startSound__garbage__(void)
{
	int16_t a;
	int16_t b;

	a = FREE_CHANNEL_INDEX;
	b = VHB_HEADER_SS[0];
	a = (a * 100) / b;
	a = (a * 100) / (b + 1);
	FREE_CHANNEL_INDEX = a;
}

uint32_t startSound(int32_t vabId, char prog, char note)
{
	VhbFile *vhb;
	VagAtr *tones;
	int32_t channel;
	uint32_t mask;
	int32_t i;

	mask = 0;
	/* MOD (sound limits, step 3): never key a bank that is not loaded (a
	 * stale entity vabId, or a load that did not fit) */
	if ((vabId < 0) || (vabId >= NUM_SOUND_BUFFERS) || (SPU_BANK_SIZE[vabId] == 0)) {
		return 0;
	}
	tones = ((VhbFile *)VHB_HEADER_ADDR[vabId])->tones[0];
	tones += prog * VAB_TONES_PER_PROGRAM;
	channel = 0x18;

	for (i = 0; i < VAB_TONES_PER_PROGRAM; i++) {
		if (note == tones[i].max) {
			if ((channel = getNextFreeChannel(channel)) == -1) {
				break;
			}

			SsUtKeyOnV(channel, vabId, prog, i, note, 0, 0x7f, 0x7f);
			mask |= 1 << channel;
		}
	}

	return mask;
}

void stopSoundMask(uint32_t mask)
{
	int32_t i;

	for (i = 0; i < 0x18; ++i) {
		if ((mask & (1 << i)) != 0) {
			SsUtAutoVol(i, 0x7f, 0, 6);
			SsUtKeyOffV(i);
		}
	}
}

/* MOD (fps): renders that play sounds do it on the first render pass of a
 * tick only (dw/mod_fps.h) */
extern int32_t MOD_RENDER_PASS;

uint32_t playSound(int32_t vabId, int32_t val)
{
	if (MOD_RENDER_PASS != 0) {
		return 0;
	}
	if ((vabId == 0) || (vabId == 1) || (vabId == 8)) {
		return startSound(vabId, val / 16, (val % 16) + 0x3c);
	} else if ((2 < vabId) && (vabId < 8)) {
		return startSound(vabId, 0, val);
	}
}

void stopSound(void)
{
	int32_t i;
	int32_t status;

	SsUtAllKeyOff(0);

	for (i = 10; i < 0x18; ++i) {
		status = SpuGetKeyStatus(1 << i);
		if (status == SPU_ON) {
			SsUtAutoVol(i, 0x7f, 0, 6);
			SsUtKeyOffV(i);
		}
	}
}

uint32_t playSound2(int32_t vabId, int32_t val)
{
	if (MOD_RENDER_PASS != 0) {
		return 0;
	}
	return startSound(vabId, val / 16, (val % 16) + 0x3c);
}

void thunkStopSoundMask(uint32_t mask)
{
	stopSoundMask(mask);
}

int32_t loadPartnerSounds(int32_t type)
{
	int32_t soundId;
	int32_t sector;
	int32_t sectors;

	if ((type > 0) && (type < NUM_DIGIMON)) { /* MOD: was NUM_PARTNER_DIGIMON */
		soundId = DIGIMON_VLALL_SOUND_ID[type];
	} else {
		soundId = 15;
	}

	/* MOD (sound limits): the index instead of soundId * 15 */
	if (!getSoundChunk(SOUND_FILE_VLALL, soundId, &sector, &sectors)) {
		return 0;
	}

	if (readVHBFileSectors(3, "VLALL", GENERAL_BUFFER, sector, sectors) == -1) {
		return 0;
	}

	return loadDigimonSounds(4, type);
}

int32_t loadDigimonSounds(int32_t vabId, int32_t type)
{
	int32_t soundId;
	int32_t sector;
	int32_t sectors;

	if ((vabId < 4) || (7 < vabId)) {
		return 0;
	}

	/* MOD (sound limits): the index instead of soundId * 7 */
	soundId = getDigimonVballId(type); /* MOD (species) */
	if (!getSoundChunk(SOUND_FILE_VBALL, soundId, &sector, &sectors)) {
		return 0;
	}

	if (readVHBFileSectors(vabId, "VBALL", GENERAL_BUFFER, sector, sectors) == -1) {
		return 0;
	}

	return 1;
}

int32_t VSLoadSounds(void)
{
	ACTIVE_MAP_SOUND_ID = -1;

	if (readVHBFile(8, "SOUND\\SB", GENERAL_BUFFER) == -1) {
		return 0;
	}

	return 1;
}

int32_t loadMapSounds(int32_t mapSoundId)
{
	int32_t sector;
	int32_t sectors;

	if (ACTIVE_MAP_SOUND_ID == mapSoundId) {
		return 1;
	}

	ACTIVE_MAP_SOUND_ID = mapSoundId;

	/* MOD (sound limits): the index instead of MAP_SOUND_PARA */
	if (!getSoundChunk(SOUND_FILE_ESALL, mapSoundId, &sector, &sectors)) {
		return 0;
	}

	if (readVHBFileSectors(8, "ESALL", GENERAL_BUFFER, sector, sectors) == -1) {
		return 0;
	}

	return 1;
}

int32_t loadMusicFont(int32_t font)
{
	uint32_t *buf;
	int32_t sector;
	int32_t sectors;

	/* MOD (sound limits): the index instead of (font - 1) * 39 */
	if (!getSoundChunk(SOUND_FILE_FAALL, font - 1, &sector, &sectors)) {
		return 0;
	}

	buf = (uint32_t *)GENERAL_BUFFER;
	if (readVHBFileSectors(2, "FAALL", GENERAL_BUFFER, sector, sectors) == -1) {
		return 0;
	}

	/* MOD (sound limits): the copy was unchecked; a block that does not fit
	 * is refused (playMusic then plays nothing) instead of overflowing */
	if (buf[3] - buf[2] > SEQ_BUFFER_SIZE) {
		return 0;
	}

	memcpy(SEQ_BUFFER, (uint8_t *)buf + ((buf[2] >> 2) << 2), buf[3] - buf[2]);

	return 1;
}

int32_t playMusic(int32_t font, int32_t track)
{
	/* MOD (sound limits): every font of the index, not 1..0x21 */
	if ((font <= 0) || (SOUND_CHUNK_COUNT[SOUND_FILE_FAALL] < font)) {
		return 0;
	}

	if ((font == CURRENT_SEQ_FONT) && (track == CURRENT_SEQ_TRACK)) {
		return 1;
	}

	seqStop();
	seqClose();

	if (CURRENT_SEQ_FONT != font) {
		/* MOD (sound limits): a font that failed to load plays nothing
		 * (the original opened the previous font's sequences) */
		if (!loadMusicFont(font)) {
			CURRENT_SEQ_FONT = -1;
			return 0;
		}
		CURRENT_SEQ_FONT = font;
	}

	CURRENT_SEQ_TRACK = track;

	seqOpen();
	seqPlay();

	return 1;
}

void stopBGM(void)
{
	seqStop();
	seqClose();
	CURRENT_SEQ_TRACK = -1;
}
