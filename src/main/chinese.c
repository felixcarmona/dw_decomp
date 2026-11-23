/* Loaded only for Chinese on an 8 MB console; code, font and work buffers stay above 2 MB. */
#include <libcd.h>
#include <libgpu.h>
#include <dw/chinese.h>
#include <dw/file_table.h>
#include <dw/mod_options.h>
#include <dw/mod_font.h>
#include <dw/sjis.h>
#include <dw/font.h>
#include <dw/entity.h>
#include <dw/partner.h>
#include <string.h>

#define ZH_HEADER ((uint32_t *)ZH_BASE)
#define ZH_GLYPHS ((uint16_t *)(ZH_FONT + 32))
#define ZH_SLOTS 32
#define ZH_STRIDE 96
static uint8_t ZH_UPLOAD[ZH_SLOTS * ZH_STRIDE] __attribute__((aligned(4)));
static int32_t ZH_SLOT __attribute__((section(".bss.zh_state")));
static int32_t ZH_LAST_X __attribute__((section(".bss.zh_state")));
static int32_t ZH_LAST_Y __attribute__((section(".bss.zh_state")));
static uint8_t *ZH_LAST_SLOT __attribute__((section(".bss.zh_state")));
static uint32_t ZH_SECTOR __attribute__((section(".bss.zh_state")));
static int32_t ZH_PREVIOUS_FONT __attribute__((section(".bss.zh_state")));
extern uint8_t COLORCODE_LOWBITS;
extern int16_t GLYPH_CARRY_Y;
extern int8_t FONT_CELLS;
uint16_t *glyphRows(uint16_t codepoint);

typedef struct {
    uint32_t hash, flags, sector, size;
} ZhFile;

/* Copies are indexed by type, so one cache eviction cannot change a held parameter pointer. */
static DigimonPara ZH_PARA[NUM_DIGIMON];
DigimonPara *zhSpeciesPara(DigimonPara *para, int32_t type)
{
    if ((uint32_t)type >= NUM_DIGIMON) return para;
    memcpy(&ZH_PARA[type], para, sizeof(DigimonPara));
    if (((char *)ZH_NAMES)[type * 20]) memcpy(ZH_PARA[type].name, (char *)ZH_NAMES + type * 20, 20);
    return &ZH_PARA[type];
}

int32_t zhStart(void)
{
    uint32_t *h = ZH_HEADER;
    uint32_t *font = (uint32_t *)ZH_FONT;
    uint32_t *patch = (uint32_t *)ZH_PATCH;
    uint32_t *run, count, i, hash = 0x811C9DC5;
    uint8_t *bytes = (uint8_t *)ZH_BASE;
    FileEntry *entry;
    int32_t type = PARTNER_ENTITY.digimonEntity.entity.type;
    int32_t followName = type > 0 && (uint32_t)type < NUM_DIGIMON && modPartnerNameIsSpecies(type);
    if (h[0] != ZH_MAGIC || h[1] != 1 || h[2] != ZH_PREFIX_SIZE || h[3] > 0xF800 ||
        h[4] > 0x20000 || h[5] > 4096 || h[6] < 8 || h[6] > 0x10000) return 0;
    for (i = 32; i < ZH_PREFIX_SIZE; i++) hash = (hash ^ bytes[i]) * 0x01000193;
    if (hash != h[7] || font[0] != 0x485A5744 || font[1] != 1 || font[2] != 12 || font[3] != 12 ||
        font[4] != 26 || font[5] != 95 || font[6] > 12032 || 32 + (font[5] + font[6]) * 26 != h[4] ||
        patch[0] != 0x504C5744) return 0;
    entry = findFileEntry("LANGZH.BIN");
    if (!entry || !entry->sector) return 0;
    /* Validate all writes before changing the active language's resident data. */
    run = patch + 2;
    for (count = patch[1]; count; count--) {
        if ((uint32_t)(run + 2) > ZH_PATCH + h[6] || run[0] < 0x80090000 || run[0] >= 0x80200000 ||
            run[1] > 0x80200000 - run[0] || run[1] > h[6] ||
            (uint32_t)(run + 2) + ((run[1] + 3) & ~3) > ZH_PATCH + h[6]) return 0;
        run += 2 + (run[1] + 3) / 4;
    }
    run = patch + 2;
    for (count = patch[1]; count; count--) {
        memcpy((void *)run[0], run + 2, run[1]);
        run += 2 + (run[1] + 3) / 4;
    }
    if (followName) {
        int32_t previous = MOD_LANG;
        MOD_LANG = MOD_LANG_ZH;
        modPartnerNameSetSpecies(type);
        MOD_LANG = previous;
    }
    ZH_PREVIOUS_FONT = MOD_FONT;
    ZH_SECTOR = entry->sector;
    ZH_SLOT = 0;
    ZH_LAST_Y = -1;
    return 1;
}

int32_t zhLookupFile(FileLookup *lookup, char *name)
{
    uint32_t hash = fileNameHash(name), i, story = !MOD_OPT_STORY;
    const ZhFile *files = (const ZhFile *)ZH_DIRECTORY;
    for (i = 0; i < ZH_HEADER[5]; i++) {
        if (files[i].hash == hash && (files[i].flags & 1) == story) {
            if ((files[i].flags & 2) && !MOD_OPT_SIGNS) return 0;
            lookup->size = files[i].size;
            CdIntToPos(ZH_SECTOR + files[i].sector, &lookup->pos);
            return 1;
        }
    }
    return 0;
}

static int32_t zhIndex(uint16_t codepoint)
{
    uint32_t code = (codepoint >> 8) | ((codepoint & 255) << 8);
    uint32_t lead = code >> 8, trail = code & 255;
    int32_t i;
    if (lead >= 0xA0 && lead <= 0xDF && trail >= 0x40 && trail <= 0xFC && trail != 0x7F) {
        i = (lead - 0xA0) * 188 + trail - 0x40 - (trail > 0x7F);
        return i < ((uint32_t *)ZH_FONT)[6] ? i + 95 : -1;
    }
    /* Existing callers pass ASCII through the game's SJIS conversion first. */
    if (code == 0x8175) code = 0x8166;
    if (code == 0x8176) code = 0x8168;
    for (i = 32; i < 127; i++) if ((uint32_t)convertAsciiToJis(i) == code) return i - 32;
    return -1; /* Use the existing button bitmaps and control-colour mapping. */
}

int32_t zhGlyphWidth(uint16_t codepoint)
{
    int32_t index = zhIndex(codepoint);
    return index >= 0 ? ZH_GLYPHS[index * 13] : glyphRows(codepoint)[11] & 255;
}

int32_t zhDrawGlyph(uint16_t codepoint, uint16_t x, uint16_t y)
{
    int32_t index = zhIndex(codepoint), cx, shift, shared = 0, row, col;
    uint16_t *glyph, *legacy, fallback[13];
    uint32_t ink;
    uint8_t *slot, *carry = NULL;
    int32_t color = modButtonColor((codepoint >> 8) | ((codepoint & 255) << 8), COLORCODE_LOWBITS), advance;
    RECT rect;
    if (index < 0) {
        legacy = glyphRows(codepoint);
        fallback[0] = legacy[11] & 255;
        for (row = 0; row < 11; row++) fallback[row+1] = legacy[row];
        fallback[12] = 0xFFFF;
        glyph = fallback;
    } else glyph = ZH_GLYPHS + index * 13;
    advance = glyph[0];
    if (FONT_CELLS && index < 95) {
        int32_t cell = glyphCell(codepoint);
        x += (cell - advance + 1) / 2;
        advance = cell;
    }
    if (x >= 244 || y >= 244) return 0;
    if (++ZH_SLOT == ZH_SLOTS) { ZH_SLOT = 0; DrawSync(0); }
    slot = ZH_UPLOAD + ZH_SLOT * ZH_STRIDE;
    cx = x & ~3; shift = x & 3;
    if (GLYPH_CARRY_Y == y && ZH_LAST_Y == y && ZH_LAST_X < cx && cx < ZH_LAST_X + 16) {
        shared = (ZH_LAST_X + 16 - cx) / 2;
        carry = ZH_LAST_SLOT + (cx - ZH_LAST_X) / 2;
    }
    for (row = 0; row < 12; row++) {
        ink = ((uint16_t)~glyph[row + 1] & 0xFFF0) >> shift;
        for (col = 0; col < 8; col++) {
            slot[row * 8 + col] = ((ink & (0x8000 >> (col * 2))) ? color : 0) |
                ((ink & (0x4000 >> (col * 2))) ? color << 4 : 0);
            if (col < shared) slot[row * 8 + col] |= carry[row * 8 + col];
        }
    }
    setRECT(&rect, cx / 4 + TEXT_VRAM_X, y + TEXT_VRAM_Y, 4, 12);
    LoadImage(&rect, (u_long *)slot);
    ZH_LAST_X = cx; ZH_LAST_Y = y; ZH_LAST_SLOT = slot; GLYPH_CARRY_Y = y;
    return advance;
}

int32_t zhPreviousFont(void)
{
    int32_t type = PARTNER_ENTITY.digimonEntity.entity.type;
    if (type > 0 && (uint32_t)type < NUM_DIGIMON &&
        !strcmp(PARTNER_ENTITY.name, (char *)ZH_NAMES + type * 20)) {
        MOD_LANG = MOD_LANG_EN;
        modPartnerNameSetSpecies(type);
        MOD_LANG = MOD_LANG_ZH;
    }
    return ZH_PREVIOUS_FONT;
}
