#ifndef DW_SPECIES_H
#define DW_SPECIES_H

/*
 * MOD (species): Digimon species beyond the disc's 180, described by the
 * disc file DIGI.DAT (docs/mods/ids.md, "Species beyond 180").
 *
 * DIGI.DAT is written by tools/species.py:
 *
 *   sector 0           SpeciesHeader
 *   sector 1..         one 512-byte SpeciesRecord per id, 4 per sector, ids
 *                      0..count-1 (0..179 are generated from the executable's
 *                      tables; the game only reads ids >= NUM_ORIGINAL_DIGIMON)
 *   after the records  each new species' MMD and its 0x4800-byte texture
 *                      (an ALLTIM.TIM entry), sector aligned, found through
 *                      the record (no fixed stride)
 *
 * The 180 original species keep their resident tables (DIGIMON_DATA,
 * DIGIMON_SKELETONS, ...): they are read every frame and never miss. A new
 * species' record is read into a small cache (SPECIES_CACHE_SLOTS records)
 * when its model is loaded, and its slot is not given away while a model of
 * it is in use (partner, NPC slots, evolution target). The per-partner
 * tables (RAISE_DATA, EVOLUTION_*, ...) are resident and sized NUM_DIGIMON;
 * the rows of new partner species are filled from DIGI.DAT at boot.
 */

#include <dw/evl.h>
#include <dw/ids.h>
#include <dw/model.h>
#include <dw/params.h>
#include <dw/partner.h>
#include <dw/types.h>

#define SPECIES_FILE_NAME	"DIGI.DAT"
#define SPECIES_MAGIC		"DIGI"
#define SPECIES_VERSION		2
#define SPECIES_RECORD_SIZE	0x200
#define SPECIES_RECORDS_PER_SECTOR (0x800 / SPECIES_RECORD_SIZE)
#define SPECIES_MAX_BONES	64
#define SPECIES_CACHE_SLOTS	16

/* SpeciesRecord.flags */
#define SPECIES_FLAG_DEFINED	0x01	/* the id exists */
#define SPECIES_FLAG_PARTNER	0x02	/* the partner block is valid: it can be a partner */
#define SPECIES_FLAG_COND_FX	0x04	/* conditionFx replaces CONDITION_FX_OFFSETS[originalType] */
#define SPECIES_FLAG_EFFECT_DEFAULT 0x08 /* effectDefault: a technique without an origin row still has one (efe.c) */
#define SPECIES_FLAG_SHOTS_FROM_DEFAULT 0x10 /* with EFFECT_DEFAULT: an entity-point technique starts there too (efe.c) */

#define SPECIES_EFFECT_ORIGINS	8
#define SPECIES_ANY_OFFSET	(-0x8000)	/* EffectOrigin.from[1]: any offset of that bone */

/* SpeciesRecord.texFile */
#define SPECIES_TEX_ALLTIM	0	/* CHDAT\ALLTIM.TIM */
#define SPECIES_TEX_ARCHIVE	1	/* DIGI.DAT */

typedef struct {
	char magic[4];		/* "DIGI" */
	uint16_t version;	/* SPECIES_VERSION */
	uint16_t recordSize;	/* SPECIES_RECORD_SIZE */
	uint16_t count;		/* ids 0..count-1 */
	uint16_t firstRecord;	/* id of the first record (0) */
	uint32_t recordsSector;	/* sector of the first record */
	uint32_t totalSectors;
	uint32_t indexSector;	/* MOD (scale): SpeciesPartnerList, then SpeciesLinks in the next sector */
	uint32_t fallbackPortrait; /* MOD (portraits): SPECIES_PORTRAIT_REF of the icon of a species without one (0: none) */
	uint32_t chartSector;	/* MOD (chart): the first sector of the chart block (0: none) */
} SpeciesHeader;

/*
 * MOD (portraits): the header sector holds, after SpeciesHeader, at SPECIES_HEADER_LEVELS one byte per
 * entry of SpeciesPartnerList: the level of that partner species (3 Rookie .. 6 Mega), so the tournament
 * can pick opponents of a level without reading a record per species.
 */
#define SPECIES_HEADER_LEVELS	0x40

/*
 * MOD (record originals): an original species (id < NUM_ORIGINAL_DIGIMON) can be carried by its DIGI.DAT record,
 * like a new species: model, texture, skeleton, effect origins, condition bubble, partner block and evolution
 * paths come from the record (the DW1 enemies made partners, the playable WereGarurumon). The header sector holds
 * a bitmap of those ids at SPECIES_HEADER_RECORD_ORIGINALS (bit t & 7 of byte t >> 3). At boot their record's
 * rows are copied over the resident tables (DIGIMON_DATA, RAISE_DATA, EVOLUTION_REQS / _STATS_GAINS,
 * ITEM_TAKE_DISTANCE, the voices, ORIGINAL_DIGIMON_TYPES, SPECIES_BATTLE_MUSIC), so the per-frame readers of
 * those tables need no test; the readers that already go through the record for new species (skeleton,
 * model, texture, training rows, effect origins, condition bubble, evolution paths, cup lists) take the
 * record. Without DIGI.DAT, or with a file that does not carry them, they are the disc's species.
 */
#define SPECIES_HEADER_RECORD_ORIGINALS	0x20
#define SPECIES_RECORD_ORIGINALS_BYTES	((NUM_ORIGINAL_DIGIMON + 7) / 8)
/*
 * MOD (options): a bitmap of the species the original game raises (the species files' "vanilla") at SPECIES_HEADER_VANILLA
 * of the header sector, one bit per id; OPTIONS' NEW DIGIMON off keeps the partner to them. A file without one (all 0)
 * has no new species to keep out: every species counts as vanilla then.
 */
#define SPECIES_HEADER_VANILLA	0x780
/*
 * MOD (canon levels): the originals whose species file gives them another level than the original game's (Phoenixmon,
 * HerculesKabuterimon, MetalEtemon ... are Mega), at SPECIES_HEADER_LEVEL_OFF of the header sector: a count, then
 * {id, level with NEW DIGIMON on, level with it off} per species. modApplySpeciesLevels writes the one of the current
 * option into DIGIMON_DATA (at boot and whenever NEW DIGIMON changes), so with the option off the game sees the original
 * levels and has no Mega.
 */
#define SPECIES_HEADER_LEVEL_OFF	0x500
#define SPECIES_LEVEL_SWAP_MAX	32
/* Header: u16 count, then {u16 id, u16 on[6], u16 off[6]}. */
#define SPECIES_HEADER_STATS_OFF 0x580
#define SPECIES_STATS_SWAP_MAX 19
/* Header u32 sector pointer; sector: u16 count, then 102-byte rows. */
#define SPECIES_HEADER_EVOLUTION_OFF 0x3c
#define SPECIES_EVOLUTION_SWAP_MAX 20
void modApplySpeciesLevels(void);
/* the level the original game gives a species (its NEW DIGIMON off level): the original evolution items keep their rule with it */
int32_t modOriginalLevel(int32_t type);
#define SPECIES_VANILLA_BYTES	(NUM_DIGIMON / 8)
int32_t isVanillaSpecies(int32_t type);
/* the vanilla species a new partner species becomes with NEW DIGIMON off (its record's vanillaAs), 0 none */
int32_t getSpeciesVanillaAs(int32_t type);
/* 1 if the original type is carried by its DIGI.DAT record (always 0 for ids >= NUM_ORIGINAL_DIGIMON) */
int32_t isRecordOriginal(int32_t type);

/*
 * MOD (scale): the two index sectors after the header. The first lists the
 * species that can be partners (the lab menu walks it), the second the
 * evolution links of new species with the original ones: the game cannot
 * know them without reading every record, and the 62 resident
 * EVOLUTION_PATHS rows are the only place the original species list their
 * targets. A link is {original type, new type | SPECIES_LINK_FROM?}: the new
 * type goes into a free "to" slot of the original (SPECIES_LINK_FROM: into a
 * free "from" slot). New-to-new links are inside the records.
 */
#define SPECIES_LINK_FROM	0x8000
#define SPECIES_MAX_PARTNERS	1022
#define SPECIES_MAX_LINKS	510
typedef struct {
	uint16_t count;
	uint16_t firstMega;		/* first partner species of level Mega, 0 if none */
	uint16_t ids[SPECIES_MAX_PARTNERS];
} SpeciesPartnerList;
typedef struct {
	uint16_t count;
	uint16_t reserved;
	uint16_t link[SPECIES_MAX_LINKS][2];
} SpeciesLinks;

/*
 * MOD (dcb-import): where this species' technique effects start. The EFE data
 * of each technique lists, per original type, a bone and an offset in that
 * bone's space (EfeTechBoneOffset). A new species borrows the entries of its
 * originalType, whose bone numbers belong to another skeleton: an entry here
 * whose `from` equals the borrowed {bone, x, y, z} (or {bone, ANY_OFFSET, ..})
 * replaces it with `to`, one of this species' bones and a point in it. Bone 0
 * (the entity) offsets are skeleton-independent and are normally left alone.
 */
typedef struct {
	int16_t from[4];		/* bone, x, y, z of the originalType's entry */
	int16_t to[4];			/* bone, x, y, z for this species (an EfeBoneOffset) */
} SpeciesEffectOrigin;

typedef struct {
	DigimonPara para;		/* 0x000 name, bones, radius, height, level, specials, drop, techniques */
	char modelName[8];		/* 0x034 "AGUM" = CHDAT\MMDn\AGUM.MMD; empty: the model is in DIGI.DAT */
	uint32_t modelSector;		/* 0x03C in DIGI.DAT (modelName empty) */
	uint32_t modelSize;		/* 0x040 bytes */
	uint32_t texSector;		/* 0x044 sector of the texture in texFile */
	uint16_t texFile;		/* 0x048 SPECIES_TEX_* */
	uint16_t texSectors;		/* 0x04A 9 (0x4800 bytes) */
	int16_t originalType;		/* 0x04C getOriginalType: technique effect offsets, aura, condition icons */
	uint8_t vballId;		/* 0x04E VBALL voice */
	uint8_t vlallId;		/* 0x04F VLALL voice (partners) */
	uint16_t flags;			/* 0x050 SPECIES_FLAG_* */
	uint8_t battleMusic;		/* 0x052 battle track (font 0x21) as a wild enemy: SPECIES_BATTLE_MUSIC */
	uint8_t reserved53;
	SkeletonBone bones[SPECIES_MAX_BONES]; /* 0x054 para.boneCount used */
	/* partner block (SPECIES_FLAG_PARTNER) */
	RaiseData raise;		/* 0x0D4 */
	EvoRequirements evoReqs;	/* 0x0F0 requirements to evolve into this species */
	EvoStatsGains evoGains;		/* 0x10C */
	EvolutionPath evoPaths;		/* 0x11A from[5] / to[6]; each "from" also gets this id in its "to" */
	int8_t itemTakeDistance;	/* 0x130 */
	int8_t trainingAnims[2];	/* 0x131 TRN_TRAINING_ANIM_IDS row */
	int8_t training2Anims[2];	/* 0x133 TRN2_TRAINING_ANIM_IDS row */
	uint8_t reserved135;
	/* MOD (dcb-import): effect origins of a species with its own skeleton */
	int16_t conditionFx[4];		/* 0x136 x, y, z, bone (SPECIES_FLAG_COND_FX) */
	uint8_t numEffectOrigins;	/* 0x13E */
	uint8_t reserved13F;
	SpeciesEffectOrigin effectOrigins[SPECIES_EFFECT_ORIGINS]; /* 0x140 */
	uint32_t portrait;		/* 0x1C0 MOD (portraits): SPECIES_PORTRAIT_REF of its icon (0: none, the fallback icon) */
	/* MOD (effect origins): where a technique whose EFE table has no row for originalType starts when its table's
	 * rows are on bones (shots, breaths): bone, x, y, z of this species (SPECIES_FLAG_EFFECT_DEFAULT; efe.c) */
	int16_t effectDefault[4];	/* 0x1C4 */
	int16_t vanillaAs;		/* 0x1CC MOD (options): NEW DIGIMON off, the vanilla species a loaded partner becomes */
	uint8_t reserved1CE[SPECIES_RECORD_SIZE - 0x1CE];
} SpeciesRecord;

#define SPECIES_LAYOUT_CHECK(name, cond) typedef char species_layout_##name[(cond) ? 1 : -1]
SPECIES_LAYOUT_CHECK(size, sizeof(SpeciesRecord) == SPECIES_RECORD_SIZE);
SPECIES_LAYOUT_CHECK(para, sizeof(DigimonPara) == 0x34);
SPECIES_LAYOUT_CHECK(music, __builtin_offsetof(SpeciesRecord, battleMusic) == 0x52);
SPECIES_LAYOUT_CHECK(bones, __builtin_offsetof(SpeciesRecord, bones) == 0x54);
SPECIES_LAYOUT_CHECK(raise, __builtin_offsetof(SpeciesRecord, raise) == 0xD4);
SPECIES_LAYOUT_CHECK(evoReqs, __builtin_offsetof(SpeciesRecord, evoReqs) == 0xF0);
SPECIES_LAYOUT_CHECK(evoGains, __builtin_offsetof(SpeciesRecord, evoGains) == 0x10C);
SPECIES_LAYOUT_CHECK(evoPaths, __builtin_offsetof(SpeciesRecord, evoPaths) == 0x11A);
SPECIES_LAYOUT_CHECK(item, __builtin_offsetof(SpeciesRecord, itemTakeDistance) == 0x130);
SPECIES_LAYOUT_CHECK(condfx, __builtin_offsetof(SpeciesRecord, conditionFx) == 0x136);
SPECIES_LAYOUT_CHECK(origins, __builtin_offsetof(SpeciesRecord, effectOrigins) == 0x140);
SPECIES_LAYOUT_CHECK(header, sizeof(SpeciesHeader) == 0x20);
SPECIES_LAYOUT_CHECK(portrait, __builtin_offsetof(SpeciesRecord, portrait) == 0x1C0);
SPECIES_LAYOUT_CHECK(effectDefault, __builtin_offsetof(SpeciesRecord, effectDefault) == 0x1C4);
SPECIES_LAYOUT_CHECK(partnerList, sizeof(SpeciesPartnerList) == 0x800);
SPECIES_LAYOUT_CHECK(links, sizeof(SpeciesLinks) <= 0x800);

/*
 * MOD (scale): the raised and recruited flags of species that have no trigger
 * of their own (ids >= NUM_ORIGINAL_DIGIMON and the originals whose trigger is
 * taken, ids.h HAS_RAISED_FLAG): one bit per type, saved in the v2 extension.
 */
extern uint8_t SPECIES_RAISED_BITS[NUM_DIGIMON / 8];
extern uint8_t SPECIES_RECRUITED_BITS[NUM_DIGIMON / 8];
void resetSpeciesFlags(void);
int32_t isDigimonRecruited(int32_t type);
void setDigimonRecruited(int32_t type);

/*
 * MOD (portraits): the icon of a species (tools/portraits.py, docs/mods/portraits.md): a 0x220-byte
 * block, a 16-colour CLUT (0x20 bytes, index 0 transparent) and a 32x32 4 bpp image (0x200 bytes) of four
 * 16x16 frames: idle, idle 2 (the chart's highlighted box), attack, hurt. DIGI.DAT packs three to a
 * sector; a species reaches its icon through SpeciesRecord.portrait, SPECIES_PORTRAIT_REF(sector, slot).
 */
#define SPECIES_PORTRAIT_SIZE	0x220
#define SPECIES_PORTRAITS_PER_SECTOR 3
#define SPECIES_PORTRAIT_REF(sector, slot) ((uint32_t)(sector) | (uint32_t)(slot) << 24)
/* 1: loads the icon of `type` (any species of DIGI.DAT; the fallback icon when it has none, 0 returned) into
 * `dest` (SPECIES_PORTRAIT_SIZE bytes, word aligned); -1 when the disc has no DIGI.DAT */
int32_t getSpeciesPortrait(int32_t type, void *dest);
/* uploads the icon of `type` to VRAM: its top `rows` (16 or 32) pixel rows at (x, y) in halfwords/rows
 * (a 32-pixel strip is 8 halfwords wide) and its CLUT at (clutX, clutY) */
void loadSpeciesPortrait(int32_t type, int32_t x, int32_t y, int32_t rows, int32_t clutX, int32_t clutY);
/*
 * MOD (chart): the evolution chart's block, written by tools/species.py (chart_tabs, chart_block). Its first sector
 * holds the number of species of each tab (CHART_TABS: Fresh and In-Training, Rookie, Champion, Ultimate, Mega) and
 * their ids, tab after tab, in the chart's order; the sectors after it the icons in the same order,
 * SPECIES_CHART_ICONS_PER_SECTOR to a sector: a 16-colour CLUT and the two 16x16 4 bpp idle frames (blank for the species
 * of the chart sheet, which keep their hand-drawn icon).
 * MOD (chart lists): after the ids, the first sector holds a count and that many {child, parent} pairs: the parents of
 * the chart's species without an evolution path row (an original 63..179 without a record), which no row lists.
 */
#define SPECIES_CHART_TABS		5
#define SPECIES_CHART_ICON_SIZE		0x120
#define SPECIES_CHART_ICONS_PER_SECTOR	7
/* the sector `k` of the chart block (0: the counts and ids, 1.. the icons) read into the species sector buffer, which
 * the next read of DIGI.DAT reuses; NULL when the file has no chart block */
/* City sector pointer at header byte 0x38; zero in archives without City. */
uint8_t *readSpeciesChart(int32_t k, int32_t city);

/* the partner species of DIGI.DAT whose level is lo..hi: writes their ids to `out` (at most `max`), returns how many */
int32_t getSpeciesPartnersByLevel(int32_t lo, int32_t hi, uint16_t *out, int32_t max);

/* partner species of DIGI.DAT: how many, the n-th id, the first Mega (0: none) */
int32_t getSpeciesPartnerCount(void);
int32_t getSpeciesPartnerId(int32_t n);
int32_t getFirstMegaSpecies(void);

/* ids 0..SPECIES_COUNT-1 exist (NUM_ORIGINAL_DIGIMON without DIGI.DAT) */
extern int32_t SPECIES_COUNT;
/* records read from the disc (cache misses), for tests */
extern int32_t SPECIES_RECORD_READS;

void initializeSpecies(void);
/* the record of a new species (id >= NUM_ORIGINAL_DIGIMON), NULL if it does not exist */
SpeciesRecord *getSpeciesRecord(int32_t type);
/* 1 if a model can be loaded for the type */
int32_t isValidDigimonType(int32_t type);
/* 1 if the type's partner model fits PARTNER_MODEL_BUFFER (memmap.h PARTNER_MMD_MAX) */
int32_t isPartnerModelFit(int32_t type);

SkeletonBone *getDigimonSkeleton(int32_t type);
/* the 4-letter model name (CHDAT\MMDn\NAME.MMD); empty for a model inside DIGI.DAT */
char *getDigimonModelName(int32_t type);
int32_t getDigimonVballId(int32_t type);
/* the battle track (font 0x21) of a wild enemy of the type (SPECIES_BATTLE_MUSIC, the record for ids >= 180) */
int32_t getSpeciesBattleMusic(int32_t type);
/* where the model and the texture of a type are: file name (or NULL for the
 * CHDAT\MMDn\NAME.MMD path), first sector and size */
int32_t getDigimonModelRef(int32_t type, char **file, int32_t *sector, int32_t *size);

/*
 * MOD (cold animations, tools/coldsplit.py): a partner's rare animations (eating, reactions, pooping) are not in its
 * model buffer. Their animation table entries are 0x80000000 | length in words << 16 | offset in words, into a cold part
 * that follows the model's (resident) part on the disc, from the next sector. startAnimation reads the one it starts into
 * COLD_ANIM_BUFFER (whole sectors: the longest cold animation plus 2047 bytes) and plays it from there; only the partner
 * has cold animations, and one plays at a time.
 */
#define COLD_ANIM_FLAG		0x80000000
#define COLD_ANIM_MAX		0x2000
uint8_t *loadColdAnimation(void *entity, uint32_t entry, int32_t animId); /* an Entity * */
extern uint32_t MOD_COLD_PENDING;
int32_t modProbeRam(void);
void refreshColdCache(void);	/* with 8 MB of RAM the partner's cold part stays in memory (mod_species.c) */
int32_t getDigimonTextureRef(int32_t type, char **file, int32_t *sector, int32_t *sectors);
/* MOD (flat sprites): a texture slot's flattened frames, (u, v) of idle, idle 2, attack, hurt and their CLUT row;
 * read from its texture when it is uploaded (mod_species.c) */
uint8_t *getFlatSprite(ModelComponent *model);
void loadFlatSprite(void *textureData, ModelComponent *model);
/* the evolution paths of a type (EVOLUTION_PATHS covers types 1..62, new
 * species have their own rows; any other type has none) */
EvolutionPath *getEvolutionPath(int32_t type);
/*
 * MOD (evolution): a Baby II's Rookie targets past its 6 "to" slots. A link whose original has no free "to" slot and
 * is an In-Training species goes here (up to SPECIES_EXTRA_TARGETS per Baby II); getInTrainingEvolutionTarget walks
 * them after the 6. Returns the count and points *targets at them (0: none).
 */
#define SPECIES_EXTRA_PARENTS	4
#define SPECIES_EXTRA_TARGETS	12
int32_t getEvolutionExtraTargets(int32_t type, DigimonId **targets);
/* MOD (chart lists): the Baby II of extra list k (< SPECIES_EXTRA_PARENTS; 0: unused) and its SPECIES_EXTRA_TARGETS
 * targets (0: empty), for the chart's "from" boxes */
int32_t getEvolutionExtraParent(int32_t k, DigimonId **targets);
/* MOD (dcb-import): the bone offset a technique effect of `type` starts
 * from: `borrowed` (the originalType's EFE entry) or this species' own one */
int16_t *getSpeciesEffectOrigin(int32_t type, int16_t *borrowed);
/* the original species whose technique effect offsets, aura and condition icons a type borrows */
int32_t getOriginalType(int32_t type);
/* the condition icon offset (x, y, z, bone) of a type */
SVECTOR *getConditionFxOffset(int32_t type);
/* a training animation row (TRN/TRN2 tables live in the overlays) */
int8_t *getTrainingAnimRow(int8_t (*table)[2], int32_t type, int32_t which);

/* Field NPC twins; reverse maps an entity back to its script identity. */
int32_t npcSpeciesType(int32_t type, int32_t reverse);

#endif
