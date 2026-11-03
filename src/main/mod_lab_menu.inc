/* Private laboratory implementation; included by mod_lab.c. Ten texture rows fit below UV 256; createPauseBox restores its shared last row on return. */
#define MOD_LAB_BOX 5
#define MOD_LAB_TEXT_Y 0x84
#define MOD_LAB_OPTIONS 10
#define MOD_LAB_OPT_EVOLVE 3
#define PAD_SELECT 0x100
#define PAD_UP 0x1000
#define PAD_DOWN 0x4000
#define PAD_LEFT 0x8000
#define PAD_RIGHT 0x2000
#define PAD_L1 4
#define PAD_R1 8
#define PAD_TRIANGLE 0x10
#define PAD_CROSS 0x40
#define LAB_VISIBLE 8
#define LAB_ROWS 10
#define LAB_PITCH 18
#define LAB_WIDTH 288
#define LAB_HEIGHT 220
#define TEXT_ROW_Y(r) (MOD_LAB_TEXT_Y + (r) * 12)

extern UIBoxData UI_BOX_DATA[];
extern int32_t IS_SCRIPT_PAUSED;
extern int16_t EVOLUTION_TARGET;
extern uint8_t HAS_USED_EVOITEM, PAUSE_STATE;
extern int32_t HAS_BUTTERFLY, BUTTERFLY_ID, CONDITION_BUBBLE_ID, CONDITION_BUBBLE_TYPE;
extern ScreenId TARGET_MAP;
extern uint8_t CURRENT_EXIT;
void pauseFrame(void);
void renderString(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
void drawLine2P(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t);
char *formatInteger(int32_t, char *, int32_t);
int32_t isFileReadQueueBusy(void);
void setFoodTimer(int16_t);
void setSleepTimes(PartnerPara *, int32_t);
void setSleepDisabled(int32_t);
void unsetButterfly(int32_t);
void unsetCameraFollowPlayer(void);
static int32_t modRosterIsPartner(int32_t type);

/* Retained for the field shortcut's set of added native partners. */
static int16_t MOD_LAB_EVO_TARGETS[] = { DIGIMON_MASTER_TYRANNOMON, DIGIMON_CHERRYMON,
    DIGIMON_KING_SUKAMON, DIGIMON_ICEMON, DIGIMON_METEORMON, DIGIMON_WARUMONZAEMON,
    DIGIMON_MYOTISMON, DIGIMON_SHOGUNGEKOMON, DIGIMON_MACHINEDRAMON, DIGIMON_JIJIMON, DIGIMON_ANALOGMAN };
#define MOD_LAB_EVO_FIXED ((int32_t)(sizeof(MOD_LAB_EVO_TARGETS) / sizeof(MOD_LAB_EVO_TARGETS[0])))

enum { LAB_HOME, LAB_VALUES, LAB_NUMBER, LAB_EVOLVE, LAB_MAPS, LAB_CONFIRM };
int32_t MOD_LAB_VIEW, MOD_LAB_CURSOR, MOD_LAB_VALUE, MOD_LAB_TARGET;
int32_t MOD_CHEATS_CARD_MODE, MOD_CHEATS_CARD_DONE, MOD_CHEATS_TRANSITION;
static int32_t labAction;
int32_t MOD_LAB_ACTIVE; /* pauseFrame renders the frozen world while this static UI is open */
int32_t MOD_LAB_EVO_CHOICE; /* selected species id, exposed for playtests */
static int32_t labTop, labCount, labField, labPrevious, labPreviousCursor;
static int32_t labStep;
static uint16_t *labChoices; /* points into modLabMenu's stack, valid until the UI closes */
static char *modLabMessage;
static char labText[LAB_ROWS][96];
static char *LAB_HOME_TEXT[] = { LAB_STR_ADD_BITS, LAB_STR_SATIATE, LAB_STR_PARAMETERS,
    LAB_STR_EVOLVE, LAB_STR_TELEPORT, LAB_STR_HEAL, LAB_STR_SAVE_GAME, LAB_STR_LOAD_GAME, LAB_STR_MAIN_MENU, LAB_STR_CLOSE };
static char *LAB_STAGES[] = { "", "Baby I", "Baby II", "Rookie", "Champion", "Ultimate", "Mega" };

static char *appendStr(char *dst, char *src)
{
    while (*src) *dst++ = *src++;
    *dst = '\0';
    return dst;
}

static char *labInt(char *p, int32_t value)
{
    char digits[16];
    if (value < 0) { p = appendStr(p, "-"); value = -value; }
    return appendStr(p, formatInteger(value, digits, 1));
}

/* Every selectable field is an int16_t. Bounds use the active species' care rules. */
typedef struct {
    char *name;
    uint16_t offset;
    int16_t min, max, careMin, careMax;
    uint8_t group; /* 0 PartnerPara, 1 Stats */
} LabField;
#define LAB_P(name, field, lo, hi, cmin, cmax) { name, __builtin_offsetof(PartnerPara, field), lo, hi, cmin, cmax, 0 }
#define LAB_S(name, field, lo, hi) { name, __builtin_offsetof(Stats, field), lo, hi, -1, -1, 1 }
static LabField LAB_FIELDS[] = {
    LAB_P(LAB_STR_DISCIPLINE, discipline, 0, 100, MOD_CARE_LIM_DISC_MIN, MOD_CARE_LIM_DISC_MAX),
    LAB_P(LAB_STR_HAPPINESS, happiness, -100, 100, MOD_CARE_LIM_HAPPY_MIN, MOD_CARE_LIM_HAPPY_MAX),
    LAB_P(LAB_STR_WEIGHT, weight, 1, 99, MOD_CARE_LIM_WEIGHT_MIN, MOD_CARE_LIM_WEIGHT_MAX),
    LAB_P(LAB_STR_MISTAKES, careMistakes, 0, 999, -1, -1),
    LAB_P(LAB_STR_BATTLES, battles, 0, 9999, -1, -1),
    LAB_P(LAB_STR_EVO_HOURS, evoTimer, 0, 32767, -1, -1),
    LAB_P(LAB_STR_AGE, age, 0, 999, -1, -1),
    LAB_P(LAB_STR_LIFETIME, remainingLifetime, 0, 32767, -1, -1),
    LAB_P(LAB_STR_VIRUS, virusBar, 0, 100, -1, -1),
    LAB_P(LAB_STR_TIREDNESS, tiredness, 0, 100, MOD_CARE_LIM_TIRED_MIN, MOD_CARE_LIM_TIRED_MAX),
    LAB_P(LAB_STR_ENERGY, energyLevel, 0, 32767, -1, -1),
    LAB_P(LAB_STR_FOOD, foodLevel, 0, 1440, -1, -1),
    LAB_S(LAB_STR_HP_MAX, base.hp, 1, HPMP_CAP),
    LAB_S(LAB_STR_MP_MAX, base.mp, 1, HPMP_CAP),
    LAB_S(LAB_STR_OFFENSE, base.off, 1, STAT_CAP),
    LAB_S(LAB_STR_DEFENSE, base.def, 1, STAT_CAP),
    LAB_S(LAB_STR_SPEED, base.speed, 1, STAT_CAP),
    LAB_S(LAB_STR_BRAINS, base.brain, 1, STAT_CAP),
    LAB_S(LAB_STR_HP_CURRENT, current.currentHP, 0, HPMP_CAP),
    LAB_S(LAB_STR_MP_CURRENT, current.currentMP, 0, HPMP_CAP),
};
#define LAB_FIELD_COUNT ((int32_t)(sizeof(LAB_FIELDS) / sizeof(LAB_FIELDS[0])))

static int16_t *modLabFieldPtr(int32_t field)
{
    LabField *f = &LAB_FIELDS[field];
    uint8_t *base = f->group ? (uint8_t *)&PARTNER_ENTITY.digimonEntity.stats : (uint8_t *)&PARTNER_PARA;
    return (int16_t *)(base + f->offset);
}

static int32_t modLabBound(int32_t field, int32_t maximum)
{
    LabField *f = &LAB_FIELDS[field];
    int32_t slot = maximum ? f->careMax : f->careMin;
    int32_t value = slot >= 0 ? careGet(slot) : -1;
    if (value == -1) value = maximum ? f->max : f->min;
    if (maximum && field == 10) value = RAISE_ROW(PARTNER_ENTITY.digimonEntity.entity.type).energyCap;
    if (maximum && field == 18) value = PARTNER_ENTITY.digimonEntity.stats.base.hp;
    if (maximum && field == 19) value = PARTNER_ENTITY.digimonEntity.stats.base.mp;
    return value;
}

static int32_t modLabClamp(int32_t field, int32_t value)
{
    int32_t lo = modLabBound(field, 0), hi = modLabBound(field, 1);
    return value < lo ? lo : value > hi ? hi : value;
}

static void modLabSetValue(int32_t field, int32_t value)
{
    Stats *s = &PARTNER_ENTITY.digimonEntity.stats;
    *modLabFieldPtr(field) = modLabClamp(field, value);
    if (s->current.currentHP > s->base.hp) s->current.currentHP = s->base.hp;
    if (s->current.currentMP > s->base.mp) s->current.currentMP = s->base.mp;
}

static void modLabHeal(void)
{
    Stats *s = &PARTNER_ENTITY.digimonEntity.stats;
    s->current.currentHP = s->base.hp;
    s->current.currentMP = s->base.mp;
}

static void modLabSatiate(void)
{
    int32_t type = PARTNER_ENTITY.digimonEntity.entity.type;
    int32_t awake;
    modLabHeal();
    setFoodTimer(type);
    setSleepTimes(&PARTNER_PARA, type);
    awake = PARTNER_PARA.hoursAwakeDefault;
    if (awake < 1 || awake > 23) awake = 16;
    PARTNER_PARA.sleepyHour = (HOUR + awake) % 24;
    PARTNER_PARA.sleepyMinute = MINUTE;
    PARTNER_PARA.wakeupHour = (PARTNER_PARA.sleepyHour + PARTNER_PARA.hoursAsleepDefault) % 24;
    PARTNER_PARA.wakeupMinute = MINUTE;
    PARTNER_PARA.timeAwakeToday = awake * 6;
    PARTNER_PARA.condition &= ~0x7f;
    PARTNER_PARA.energyLevel = RAISE_ROW(type).energyCap;
    PARTNER_PARA.poopLevel = RAISE_ROW(type).poopTimer;
    PARTNER_PARA.poopingTimer = -1;
    PARTNER_PARA.tiredness = PARTNER_PARA.subTiredness = PARTNER_PARA.tirednessHungerTimer = 0;
    PARTNER_PARA.starvationTimer = PARTNER_PARA.emptyStomachTimer = PARTNER_PARA.refusedFavFood = 0;
    PARTNER_PARA.sicknessCounter = PARTNER_PARA.sicknessTries = PARTNER_PARA.sicknessTimer = 0;
    PARTNER_PARA.injuryTimer = PARTNER_PARA.missedSleepHours = PARTNER_PARA.tirednessSleepTimer = 0;
    PARTNER_PARA.happiness = modLabBound(1, 1);
    setSleepDisabled(1);
    if (HAS_BUTTERFLY == 0) { unsetButterfly(BUTTERFLY_ID); HAS_BUTTERFLY = -1; }
    if (CONDITION_BUBBLE_ID >= 0) removeConditionBubble(CONDITION_BUBBLE_ID);
    CONDITION_BUBBLE_TYPE = -1;
    if (partnerGetState() == 1) updateConditionAnimation();
}

static int32_t modLabNewSpecies(int32_t type)
{
    return type >= NUM_PARTNER_DIGIMON || !isVanillaSpecies(type);
}

static void modLabList(int32_t view)
{
    int32_t i, first, id;
    labCount = 0;
    MOD_LAB_VIEW = view;
    MOD_LAB_CURSOR = labTop = 0;
    if (view == LAB_EVOLVE) {
        for (i = 1; i < NUM_ORIGINAL_DIGIMON; i++) {
            if (modRosterIsPartner(i)) labChoices[labCount++] = i;
        }
        /* The partner index is ordered by id; no duplicate added native species. */
        for (i = 0; i < getSpeciesPartnerCount(); i++) {
            id = getSpeciesPartnerId(i);
            if (id >= NUM_ORIGINAL_DIGIMON && modRosterIsPartner(id)) labChoices[labCount++] = id;
        }
    } else {
        for (i = 0; i < NUM_ORIGINAL_SCREENS; i++) {
            MapEntry *e = getScreenEntry(i);
            char filename[16];
            if (!e->filename[0]) continue;
            appendStr(appendStr(filename, e->filename), ".MAP");
            /* Zero background images is valid (Mount Infinity, 3D rooms). */
            if (modFileOnDisc(filename)) labChoices[labCount++] = i;
        }
        first = findNewScreen(-1, 1);
        if (first >= 0) {
            id = first;
            do {
                labChoices[labCount++] = id;
                id = findNewScreen(id, 1);
            } while (id >= 0 && id != first && labCount < NUM_SCREENS);
        }
    }
    id = view == LAB_EVOLVE ? PARTNER_ENTITY.digimonEntity.entity.type : CURRENT_SCREEN;
    for (i = 0; i < labCount; i++) if (labChoices[i] == id) MOD_LAB_CURSOR = i;
}

static int32_t modLabRowCount(void)
{
    return MOD_LAB_VIEW == LAB_HOME ? MOD_LAB_OPTIONS : MOD_LAB_VIEW == LAB_VALUES ? LAB_FIELD_COUNT : labCount;
}

/* Copy only whole encoded glyphs, then fit to the texture width in every font. */
static void modLabFit(char *s, int32_t width)
{
    char *p, *last;
    while (measureStringMode(s, FIT_VANILLA) > width) {
        last = s;
        for (p = s; *p; ) {
            last = p;
            p += (uint8_t)*p >= 0x80 ? 2 : 1;
        }
        if (last == s) { *s = 0; break; }
        *last = 0;
    }
}

/* Keep the stage and new marker visible even when a long name needs fitting. */
static void modLabEntry(char *out, int32_t id, int32_t evolution)
{
    char name[96], suffix[48];
    char *p = suffix;
    int32_t level;
    suffix[0] = 0;
    appendStr(name, evolution ? DIGIMON_NAME(id) : getScreenCaption(id));
    if (evolution) {
        level = DIGI_PARA(id).level;
        /* The original unused WereGarurumon row has no stage; content/species/62 declares Ultimate. */
        if (id == 62 && level == 0) level = 5;
        p = appendStr(p, " - ");
        p = appendStr(p, level >= 1 && level <= 6 ? LAB_STAGES[level] : "?");
    }
    if (!evolution) {
        p = appendStr(p, " - ID "); p = labInt(p, id);
    }
    if (evolution ? modLabNewSpecies(id) : isNewScreen(id)) appendStr(p, LAB_STR_NEW_MARK);
    modLabFit(name, 248 - measureStringMode(suffix, FIT_VANILLA));
    appendStr(appendStr(out, name), suffix);
}

static void modLabDrawText(void)
{
    RECT area;
    char lines[LAB_ROWS][96];
    char *p;
    int32_t row, n, id, count = modLabRowCount();
    int32_t firstRow = 1, visible = 8;
    if (MOD_LAB_CURSOR < labTop) labTop = MOD_LAB_CURSOR;
    if (MOD_LAB_CURSOR >= labTop + visible) labTop = MOD_LAB_CURSOR - visible + 1;
    memset(lines, 0, sizeof(lines));
    appendStr(lines[0], MOD_LAB_VIEW == LAB_HOME ? LAB_STR_TITLE :
        MOD_LAB_VIEW == LAB_VALUES ? LAB_STR_PARAMETERS : MOD_LAB_VIEW == LAB_NUMBER ? LAB_FIELDS[labField].name :
        MOD_LAB_VIEW == LAB_EVOLVE ? LAB_STR_EVOLVE : MOD_LAB_VIEW == LAB_MAPS ? LAB_STR_TELEPORT :
        labPrevious == LAB_HOME ? LAB_HOME_TEXT[labPreviousCursor] : LAB_STR_CONFIRM);
    if (MOD_LAB_VIEW == LAB_NUMBER) {
        labInt(lines[3], MOD_LAB_VALUE);
        p = appendStr(lines[5], LAB_STR_RANGE); p = labInt(p, modLabBound(labField, 0));
        p = appendStr(p, LAB_STR_TO_RANGE); labInt(p, modLabBound(labField, 1));
        p = appendStr(lines[6], LAB_STR_SIMPLE_STEP); p = labInt(p, labStep); appendStr(p, LAB_STR_STEP_ARROWS);
        appendStr(lines[7], LAB_STR_SIMPLE_ADJUST);
    } else if (MOD_LAB_VIEW == LAB_CONFIRM) {
        if (labPrevious == LAB_HOME) {
            appendStr(lines[1], MOD_LAB_TARGET == 1 ? LAB_STR_ASK_SAVE : LAB_STR_ASK_TITLE);
            if (MOD_LAB_TARGET == 3) appendStr(lines[3], LAB_STR_UNSAVED);
        } else {
            appendStr(lines[1], labPrevious == LAB_EVOLVE ? LAB_STR_ASK_EVOLVE : LAB_STR_ASK_MAP);
            modLabEntry(lines[3], MOD_LAB_TARGET, labPrevious == LAB_EVOLVE);
        }
    } else for (row = 0; row < visible; row++) {
        n = labTop + row;
        if (n >= count) break;
        p = lines[row + firstRow];
        if (MOD_LAB_VIEW == LAB_HOME) appendStr(p, LAB_HOME_TEXT[n]);
        else if (MOD_LAB_VIEW == LAB_VALUES) {
            p = appendStr(p, LAB_FIELDS[n].name); p = appendStr(p, ": "); labInt(p, *modLabFieldPtr(n));
        } else {
            id = labChoices[n];
            modLabEntry(p, id, MOD_LAB_VIEW == LAB_EVOLVE);
        }
    }
    p = appendStr(lines[9], "\x81\xa2 ");
    p = appendStr(p, MOD_LAB_VIEW == LAB_NUMBER ? LAB_STR_SAVE : MOD_LAB_VIEW == LAB_CONFIRM ?
        (labPrevious == LAB_HOME ? LAB_STR_DO_IT : labPrevious == LAB_EVOLVE ? LAB_STR_SIMPLE_EVOLVE : LAB_STR_SIMPLE_TRAVEL) : LAB_STR_CHOOSE);
    p = appendStr(p, "    \x81\x7e "); appendStr(p, MOD_LAB_VIEW == LAB_NUMBER || MOD_LAB_VIEW == LAB_CONFIRM ? LAB_STR_SIMPLE_CANCEL : LAB_STR_BACK);
    if (modLabMessage) { lines[9][0] = 0; appendStr(lines[9], modLabMessage); }
    for (row = 0; row < LAB_ROWS; row++) {
        modLabFit(lines[row], 248);
        if (!strcmp(labText[row], lines[row])) continue;
        setRECT(&area, 0, TEXT_ROW_Y(row), 256, 12); clearTextSubArea(&area);
        drawString(lines[row], 0, TEXT_ROW_Y(row));
        strcpy(labText[row], lines[row]);
    }
    DrawSync(0);
}

/* The title, body and controls have their own space, regardless of how many rows a view uses. */
static int32_t modLabRowY(int32_t row)
{
    if (row == 0) return 12;
    if (row == 9) return 206;
    if (MOD_LAB_VIEW == LAB_NUMBER) {
        if (row == 3) return 76;
        if (row == 5) return 116;
        if (row == 6) return 148;
        if (row == 7) return 169;
    }
    return 44 + (row - 1) * LAB_PITCH;
}

static void modLabFill(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, int32_t g, int32_t b, int32_t order)
{
    GsBOXF box;
    box.attribute = 0; box.r = r; box.g = g; box.b = b;
    setRECT(&box, x, y, w, h); GsSortBoxFill(&box, ACTIVE_ORDERING_TABLE, order);
}

static void modLabRender(int32_t instanceId)
{
    RECT *pos = &UI_BOX_DATA[MOD_LAB_BOX].finalPos;
    int32_t row, x, y;
    int32_t form = MOD_LAB_VIEW == LAB_NUMBER || MOD_LAB_VIEW == LAB_CONFIRM;
    int32_t selected = 1 + MOD_LAB_CURSOR - labTop;
    modLabFill(-160, -120, 320, 240, 0, 0, 0, 7 - instanceId);
    /* Same panel as the game, with a distinct header and a separate control footer. */
    drawLine2P(0x778b97, pos->x + 12, pos->y + 34, pos->x + LAB_WIDTH - 12, pos->y + 34, 0, 0);
    drawLine2P(0x778b97, pos->x + 12, pos->y + 196, pos->x + LAB_WIDTH - 12, pos->y + 196, 0, 0);
    /* Texture UVs are bytes: a width of 256 wraps to zero. */
    for (row = 0; row < LAB_ROWS; row++) {
        if (!labText[row][0]) continue;
        x = pos->x + 16;
        y = pos->y + modLabRowY(row);
        if (form && row > 0 && row < 9) x = -measureStringMode(labText[row], FIT_VANILLA) / 2;
        renderString(0, x, y, 248, 12, 0, TEXT_ROW_Y(row), 0, 1);
    }
    if (!form) {
        y = pos->y + modLabRowY(selected);
        modLabFill(pos->x + 10, y - 3, LAB_WIDTH - 20, 18, 61, 81, 100, 0);
    } else if (MOD_LAB_VIEW == LAB_NUMBER) {
        x = pos->x + 64; y = pos->y + 67;
        drawLine2P(0x9daeb8, x, y, x + 160, y, 0, 0);
        drawLine2P(0x9daeb8, x, y + 30, x + 160, y + 30, 0, 0);
        drawLine2P(0x9daeb8, x, y, x, y + 30, 0, 0);
        drawLine2P(0x9daeb8, x + 160, y, x + 160, y + 30, 0, 0);
        drawLine2P(0xeeeeee, x + 18, y + 10, x + 13, y + 15, 0, 0);
        drawLine2P(0xeeeeee, x + 13, y + 15, x + 18, y + 20, 0, 0);
        drawLine2P(0xeeeeee, x + 142, y + 10, x + 147, y + 15, 0, 0);
        drawLine2P(0xeeeeee, x + 147, y + 15, x + 142, y + 20, 0, 0);
    }
}

static void modLabFrames(int32_t frames)
{
    while (frames-- > 0) pauseFrame();
}

/* Confirmations stage navigation only after the lab has closed and pause has ended. */
static int32_t modLabPerform(void)
{
    int32_t type = MOD_LAB_TARGET;
    if (partnerGetState() != 1 || tamerGetState() != 0 || EVOLUTION_TARGET != -1 || isFileReadQueueBusy()) {
        modLabMessage = LAB_STR_MSG_NOT_NOW; return 0;
    }
    if (labPrevious == LAB_HOME) {
        labAction = type;
    } else if (labPrevious == LAB_EVOLVE) {
        if (!isPartnerModelFit(type)) { modLabMessage = LAB_STR_MSG_TOO_BIG; return 0; }
        if (type == PARTNER_ENTITY.digimonEntity.entity.type) { modLabMessage = LAB_STR_MSG_ALREADY; return 0; }
        EVOLUTION_TARGET = type; HAS_USED_EVOITEM = 0;
        tamerSetState(6); partnerSetState(13);
    } else {
        TARGET_MAP = type; CURRENT_EXIT = 0;
        tamerSetState(5); unsetCameraFollowPlayer(); stopGameTime();
    }
    PAUSE_STATE = 0;
    return 1;
}

extern int32_t CURRENT_MENU, TARGET_MENU, MAIN_STATE, MAIN_MENU_ACTION, CHECKED_MEMORY_CARD;
extern int32_t MEMORY_CARD_ID, MEMORY_CARD_SLOT, MEMORY_CARD_OPERATION;
extern char *SLOT_ACTION_TITLES[], *SLOT_ACTION_QUESTIONS[];
void loadMenuOverlay(void);
void closeMenuOverlay(void);
void tickMainMenu(void);
void renderMainMenu(int32_t);
void pollInputMenu(void);

static void modLabCardMenu(int32_t action)
{
    int32_t oldState = MAIN_STATE, oldCard = MEMORY_CARD_ID, oldSlot = MEMORY_CARD_SLOT;
    char *oldTitle = SLOT_ACTION_TITLES[action - 1], *oldQuestion = SLOT_ACTION_QUESTIONS[action - 1];
    MOD_LAB_ACTIVE = 1;
    MOD_CHEATS_CARD_MODE = action; MOD_CHEATS_CARD_DONE = 0;
    if (action == 1) MAIN_STATE = 1;
    while (PadRead(1) & (CONFIRM_BUTTON | CANCEL_BUTTON)) VSync(0);
    pollInputMenu();
    loadMenuOverlay();
    SLOT_ACTION_TITLES[action - 1] = action == 1 ? LAB_STR_SAVE_GAME : LAB_STR_LOAD_GAME;
    SLOT_ACTION_QUESTIONS[action - 1] = action == 1 ? LAB_STR_ASK_SAVE : LAB_STR_ASK_LOAD;
    CHECKED_MEMORY_CARD = 0x10; CURRENT_MENU = -2;
    MAIN_MENU_ACTION = action == 1 ? 0 : 1;
    MEMORY_CARD_OPERATION = 0;
    TARGET_MENU = action == 2 ? 0x14 : oldCard < 0 || oldSlot < 0 ? 0xA : 0x29;
    addObject(5000, 0, NULL, (RenderFunction)renderMainMenu);
    while (CURRENT_MENU != -1) {
        /* The title/new-game branches are exits here, never a request to start over. */
        if (TARGET_MENU == 0 || (action == 1 && TARGET_MENU == 0x13)) TARGET_MENU = 9;
        pollInputMenu(); tickMainMenu(); pauseFrame();
    }
    removeObject(5000, 0);
    SLOT_ACTION_TITLES[action - 1] = oldTitle; SLOT_ACTION_QUESTIONS[action - 1] = oldQuestion;
    closeMenuOverlay();
    while (PadRead(1) & (CONFIRM_BUTTON | CANCEL_BUTTON)) VSync(0);
    pollInputMenu();
    MAIN_STATE = oldState;
    if (!MOD_CHEATS_CARD_DONE) { MEMORY_CARD_ID = oldCard; MEMORY_CARD_SLOT = oldSlot; }
    if (MOD_CHEATS_CARD_DONE == 2) MOD_CHEATS_TRANSITION = 1;
    MOD_CHEATS_CARD_MODE = 0; MOD_LAB_ACTIVE = 0;
}

void modLabMenu(uint32_t *input, uint32_t *previous)
{
    uint16_t choices[NUM_DIGIMON > NUM_SCREENS ? NUM_DIGIMON : NUM_SCREENS];
    uint32_t pressed;
    RECT pos;
    int32_t count, step, done = 0;
    if (!MOD_OPT_CHEATS || IS_SCRIPT_PAUSED != 1) return;
    MOD_LAB_ACTIVE = 1;
    labChoices = choices;
    MOD_LAB_VIEW = LAB_HOME; MOD_LAB_CURSOR = labTop = 0;
    modLabMessage = NULL; labAction = 0;
    removeStaticUIBox(MOD_LAB_BOX);
    setRECT(&pos, 0, MOD_LAB_TEXT_Y, 256, LAB_ROWS * 12); clearTextSubArea(&pos);
    memset(labText, 0, sizeof(labText));
    modLabDrawText();
    setRECT(&pos, -LAB_WIDTH / 2, -LAB_HEIGHT / 2, LAB_WIDTH, LAB_HEIGHT);
    createStaticUIBox(MOD_LAB_BOX, 1, 0, &pos, NULL, modLabRender);
    playSound(0, 3); modLabFrames(12);
    while (!done) {
        *input = PadRead(1); pressed = *input & ~*previous; *previous = *input;
        if (!pressed) { VSync(0); continue; }
        modLabMessage = NULL;
        if (pressed & PAD_SELECT) break;
        if (pressed & CANCEL_BUTTON) {
            if (MOD_LAB_VIEW == LAB_HOME) break;
            if (MOD_LAB_VIEW == LAB_NUMBER) { MOD_LAB_VIEW = LAB_VALUES; MOD_LAB_CURSOR = labField; }
            else if (MOD_LAB_VIEW == LAB_CONFIRM) { MOD_LAB_VIEW = labPrevious; MOD_LAB_CURSOR = labPreviousCursor; }
            else { MOD_LAB_CURSOR = MOD_LAB_VIEW == LAB_VALUES ? 2 : MOD_LAB_VIEW == LAB_EVOLVE ? 3 : 4; MOD_LAB_VIEW = LAB_HOME; }
            labTop = 0;
        } else if (MOD_LAB_VIEW == LAB_NUMBER) {
            if (pressed & CONFIRM_BUTTON) {
                modLabSetValue(labField, MOD_LAB_VALUE); MOD_LAB_VIEW = LAB_VALUES; MOD_LAB_CURSOR = labField; labTop = 0;
                modLabMessage = LAB_STR_APPLIED;
            } else {
                if (pressed & PAD_UP) { if (labStep < 10000) labStep *= 10; }
                else if (pressed & PAD_DOWN) { if (labStep > 1) labStep /= 10; }
                else if (pressed & PAD_RIGHT) MOD_LAB_VALUE = modLabClamp(labField, MOD_LAB_VALUE + labStep);
                else if (pressed & PAD_LEFT) MOD_LAB_VALUE = modLabClamp(labField, MOD_LAB_VALUE - labStep);
            }
        } else if (MOD_LAB_VIEW == LAB_CONFIRM) {
            if (pressed & CONFIRM_BUTTON) done = modLabPerform();
        } else if (pressed & CONFIRM_BUTTON) {
            if (MOD_LAB_VIEW == LAB_HOME) {
                switch (MOD_LAB_CURSOR) {
                case 0:
                    step = econCap(MOD_ECON_MONEY_CAP, 2147483647);
                    MONEY = MONEY > step - 10000 ? step : MONEY + 10000;
                    modLabMessage = LAB_STR_MSG_BITS; break;
                case 1:
                    if (partnerGetState() != 1 || tamerGetState() != 0) modLabMessage = LAB_STR_MSG_NOT_NOW;
                    else { modLabSatiate(); modLabMessage = LAB_STR_SATISFIED; }
                    break;
                case 2: MOD_LAB_VIEW = LAB_VALUES; MOD_LAB_CURSOR = labTop = 0; break;
                case 3: modLabList(LAB_EVOLVE); break;
                case 4: modLabList(LAB_MAPS); break;
                case 5: modLabHeal(); modLabMessage = LAB_STR_MSG_HEALED; break;
                case 6: case 8:
                    labPrevious = LAB_HOME; labPreviousCursor = MOD_LAB_CURSOR;
                    MOD_LAB_TARGET = MOD_LAB_CURSOR == 6 ? 1 : 3;
                    MOD_LAB_VIEW = LAB_CONFIRM; break;
                case 7:
                    labPrevious = LAB_HOME; labPreviousCursor = MOD_LAB_CURSOR;
                    MOD_LAB_TARGET = 2; done = modLabPerform(); break;
                case 9: done = 1; break;
                }
            } else if (MOD_LAB_VIEW == LAB_VALUES) {
                labField = MOD_LAB_CURSOR; MOD_LAB_VALUE = modLabClamp(labField, *modLabFieldPtr(labField)); MOD_LAB_VIEW = LAB_NUMBER; labStep = 1;
            } else if (labCount) {
                labPrevious = MOD_LAB_VIEW; labPreviousCursor = MOD_LAB_CURSOR;
                MOD_LAB_TARGET = labChoices[MOD_LAB_CURSOR];
                if (MOD_LAB_VIEW == LAB_EVOLVE) MOD_LAB_EVO_CHOICE = MOD_LAB_TARGET;
                MOD_LAB_VIEW = LAB_CONFIRM;
            }
        } else {
            count = modLabRowCount();
            step = (pressed & (PAD_L1 | PAD_R1)) ? 10 : 1;
            if (pressed & (PAD_UP | PAD_LEFT | PAD_L1)) MOD_LAB_CURSOR -= step;
            if (pressed & (PAD_DOWN | PAD_RIGHT | PAD_R1)) MOD_LAB_CURSOR += step;
            if (count > 0) { if (MOD_LAB_CURSOR < 0) MOD_LAB_CURSOR = 0; if (MOD_LAB_CURSOR >= count) MOD_LAB_CURSOR = count - 1; }
        }
        playSound(0, (pressed & CONFIRM_BUTTON) ? 3 : (pressed & CANCEL_BUTTON) ? 4 : 2);
        modLabDrawText(); modLabFrames(2);
    }
    removeStaticUIBox(MOD_LAB_BOX);
    setRECT(&pos, 0, MOD_LAB_TEXT_Y, 256, LAB_ROWS * 12); clearTextSubArea(&pos);
    labChoices = NULL;
    playSound(0, 4); modLabFrames(8);
    MOD_LAB_ACTIVE = 0;
    if (labAction == 1 || labAction == 2) modLabCardMenu(labAction);
    if (labAction == 3) {
        while (PadRead(1) & (CONFIRM_BUTTON | CANCEL_BUTTON)) VSync(0);
        pollInputMenu(); MOD_CHEATS_TRANSITION = 2;
    }
}

void unloadMapParts(void);
void clearDroppedItems(void);
void checkFishingMap(int32_t, int32_t);
void clearTextArea(void);
void modCheatsCleanup(void)
{
    int32_t i;
    unloadMapParts(); clearDroppedItems(); checkFishingMap(0, 0);
    if (CONDITION_BUBBLE_ID >= 0) removeConditionBubble(CONDITION_BUBBLE_ID);
    CONDITION_BUBBLE_ID = CONDITION_BUBBLE_TYPE = -1;
    if (HAS_BUTTERFLY == 0) unsetButterfly(BUTTERFLY_ID);
    HAS_BUTTERFLY = -1;
    for (i = 0; i < 6; i++) removeStaticUIBox(i);
    clearTextArea();
}
