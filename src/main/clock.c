#include <libgpu.h>
#include <dw/mod_options.h>
#include <libgs.h>

#include <dw/clock.h>
#include <dw/mod_rules.h>
#include <dw/level.h>
#include <dw/params.h>
#include <dw/pstat.h>
#include <dw/script.h>
#include <dw/mod_fps.h>
#include <dw/world_object.h>

void renderRectPolyFT4(int32_t posX, int32_t posY,
		       int32_t width, int32_t height,
		       int32_t texX, int32_t texY,
		       int32_t texturePage, int32_t clut,
		       int32_t zIndex, int32_t flag);

void updateTimeOfDay();
int32_t isFishing();
void addTamerLevel(int32_t chance, int32_t amount);
void updateBGM();
void renderStatusBars(int32_t isGameTimeRunning);
void tickStatusBarsSlide(int32_t isGameTimeRunning);

extern uint16_t LAST_HANDLED_FRAME;
extern int8_t GAME_STATE;
extern uint8_t MAP_LAYER_ENABLED;
extern GsOT *ACTIVE_ORDERING_TABLE;

uint8_t CLOCK_TEXCOORD_U[2][4] = {
	{ 128, 160, 128, 160 },
	{ 160, 128, 160, 128 }
};

uint8_t CLOCK_TEXCOORD_V[2][4] = {
	{ 192, 192, 208, 192 },
	{ 208, 224, 208, 224 }
};

uint8_t HOUR_POINT_X[24] = {
	20, 23, 26, 31, 34, 36, 36, 36,
	34, 31, 26, 23, 20, 17, 14,  9,
	 6,  4,  4,  4,  6,  9, 14, 17,
};

uint8_t HOUR_POINT_Y[24] = {
	 5,  6 , 6 , 8, 12, 15, 19, 23,
	26, 30, 32, 32, 33, 32, 32, 30,
	26, 23, 19, 15, 12,  8,  6,  6,
};

uint16_t SUBFRAME_COUNT;
int16_t MOD_CLOCK[MOD_CLOCK_SIZE] = { -1 };       /* MOD (clock as data): written by tools/balance.py from content/tables/clock.json */
static int32_t CLOCK_ACC;                       /* the fraction of a frame the clock owes (in 1/(2 x day ticks)) */
int16_t CLOCK_OFFSET_X;
int8_t IS_GAMETIME_RUNNING;

static void *clock_sbss_order[] = {
	&IS_GAMETIME_RUNNING,
	&CLOCK_OFFSET_X,
	&SUBFRAME_COUNT,
};

GsSPRITE CLOCK_SPRITE;

static void *clock_text_order[] = {
	startGameTime,
	stopGameTime,
	updateMinuteHand,
	advanceToTime,
	tickPlaytime,
	renderGameClock,
	tickGameClock,
	addClock,
	initializeClockData
};

static void *clock_data_order[] = {
	HOUR_POINT_Y,
	HOUR_POINT_X,
};

void addClock(void)
{
	addObjectLate(0xfa2, 0, tickGameClock, lateTickGameClock, renderGameClock);
	addObject(0xfb9, 0, tickPlaytime, 0);
}

void tickGameClock(int32_t instanceId)
{
	uint8_t timeSpeed;
	int32_t steps;

	LAST_HANDLED_FRAME = CURRENT_FRAME;

	if ((GAME_STATE != 0) ||
	    (isFishing() == 1) ||
	    (MAP_LAYER_ENABLED == 0) ||
	    (IS_GAMETIME_RUNNING == 0)) {
		return;
	}

	if ((IS_GAMETIME_RUNNING == 1) &&
	    (readPStat(PSTAT_TIME_SPEED) == 3)) {
		writePStat(PSTAT_TIME_SPEED, 0);
	}

	timeSpeed = readPStat(PSTAT_TIME_SPEED);
	if (timeSpeed == 3) {
		return;
	}

	/*
	 * MOD (clock as data): a day lasts MOD_CLOCK[MOD_CLK_DAY_SECONDS] real seconds (28800 frames = 24 h x 60 min x 20): the frames
	 * this tick adds accumulate in halves (speed 0 = 1 frame at a 1440 s day, speed 1 = twice that, speed 2 = half) and every
	 * frame is run one by one, so a minute never slips by. No value: game time does not run.
	 */
	steps = 0;
	if (MOD_CLOCK[MOD_CLK_DAY_SECONDS] > 0) {
		CLOCK_ACC += 28800 * (timeSpeed == 0 ? 2 : (timeSpeed == 1 ? 4 : 1));
		steps = CLOCK_ACC / (MOD_CLOCK[MOD_CLK_DAY_SECONDS] * 40);
		CLOCK_ACC %= MOD_CLOCK[MOD_CLK_DAY_SECONDS] * 40;
	}

	for (; steps > 0; steps--) {
		CURRENT_FRAME++;
		if ((CURRENT_FRAME % 20) != 0) {
			continue;
		}
		MINUTE++;
		if (MINUTE == 60) {
			HOUR++;
			MINUTE = 0;
			PARTNER_PARA.evoTimer++;
			PARTNER_PARA.remainingLifetime--;
			SUBFRAME_COUNT = 0;
			if (HOUR == 24) {
				PARTNER_PARA.age++;
				DAY++;
				HOUR = 0;
				CURRENT_FRAME = 0;
				dailyPStatTrigger();
				if (PARTNER_PARA.remainingLifetime < 0) {
					PARTNER_PARA.remainingLifetime = 0;
				}
				if (DAY >= 30) {
					DAY = 0;
					YEAR++;
					/* MOD (tamer level as data): MOD_TAMER_LEVEL's chances and happiness marks */
					if (MOD_TAMER_LEVEL[MOD_TL_YEAR_HAPPY_AT] >= 0 &&
					    PARTNER_PARA.happiness == MOD_TAMER_LEVEL[MOD_TL_YEAR_HAPPY_AT]) {
						modTamerLevelEvent(MOD_TL_YEAR_HAPPY_CHANCE, 1);
					} else if (MOD_TAMER_LEVEL[MOD_TL_YEAR_UNHAPPY_BELOW] >= 0 &&
						   PARTNER_PARA.happiness < MOD_TAMER_LEVEL[MOD_TL_YEAR_UNHAPPY_BELOW]) {
						modTamerLevelEvent(MOD_TL_YEAR_UNHAPPY_CHANCE, -1);
					}
				}
			}
		}
	}

	if (YEAR >= 100) {
		YEAR = 0;
	}

	CLOCK_SPRITE.rotate = MINUTE * 0x6000;

	updateBGM();
}

void tickPlaytime(int32_t instanceId)
{
	++PLAYTIME_FRAMES;
	if ((PLAYTIME_FRAMES % 1200) == 0) {
		++PLAYTIME_MINUTES;
		if (PLAYTIME_MINUTES > 59) {
			++PLAYTIME_HOURS;
			PLAYTIME_MINUTES = 0;
			PLAYTIME_FRAMES = 0;
			if (PLAYTIME_HOURS > 999) {
				PLAYTIME_HOURS = 999;
			}
		}
	}

	if (PLAYTIME_HOURS == 999) {
		PLAYTIME_MINUTES = 59;
	}
}

void advanceToTime(int32_t hour, int16_t minute)
{
	if ((hour < HOUR) || ((HOUR == hour) && (minute < MINUTE))) {
		DAY += 1;
		PARTNER_PARA.age += 1;
		dailyPStatTrigger();
		if (DAY >= 30) {
			DAY = 0;
			YEAR += 1;
		}
	}
	HOUR = hour % 24;
	MINUTE = minute;
	CURRENT_FRAME = HOUR * 1200 + MINUTE * 20;
	CLOCK_SPRITE.rotate = MINUTE * 0x6000;
	updateTimeOfDay();
}

void initializeClockData(void)
{
	YEAR = 0;
	DAY = 0;
	HOUR = modNewGame(MOD_NG_START_HOUR);
	MINUTE = modNewGame(MOD_NG_START_MINUTE);
	CURRENT_FRAME = HOUR * 1200 + MINUTE * 20;
	PLAYTIME_FRAMES = 0;
	PLAYTIME_HOURS = 0;
	PLAYTIME_MINUTES = 0;
	SUBFRAME_COUNT = 0;
	CLOCK_OFFSET_X = -135;
	CLOCK_SPRITE.attribute = 0;
	CLOCK_SPRITE.x = CLOCK_OFFSET_X + 23;
	CLOCK_SPRITE.y = -66;
	setWH(&CLOCK_SPRITE, 8, 16);
	CLOCK_SPRITE.tpage = GetTPage(0, 0, 896, 448);
	CLOCK_SPRITE.cx = 256;
	CLOCK_SPRITE.cy = 499;
	CLOCK_SPRITE.u = 120;
	CLOCK_SPRITE.v = 192;
	CLOCK_SPRITE.r = CLOCK_SPRITE.g = CLOCK_SPRITE.b = 0x80;
	CLOCK_SPRITE.mx = 3;
	CLOCK_SPRITE.my = 13;
	CLOCK_SPRITE.scalex = CLOCK_SPRITE.scaley = 0x1000;
	CLOCK_SPRITE.rotate = MINUTE * 0x6000;
	IS_GAMETIME_RUNNING = 1;
}

/* MOD (fps): the clock and the status bars slide in or out (when game time
 * stops) once per tick, after all the object ticks, where the render used to
 * move them. */
void lateTickGameClock(int32_t instanceId)
{
	if (IS_GAMETIME_RUNNING == 0) {
		CLOCK_OFFSET_X -= 50;
	} else {
		CLOCK_OFFSET_X += 50;
	}

	if (CLOCK_OFFSET_X > -135) {
		CLOCK_OFFSET_X = -135;
	}

	if (CLOCK_OFFSET_X < -220) {
		CLOCK_OFFSET_X = -220;
	}

	tickStatusBarsSlide(IS_GAMETIME_RUNNING);
}

int16_t MOD_DAYTIME[MOD_DAYTIME_SIZE] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };

/* MOD (rules as data): 0 dusk, 1 night, 2 day; without the three hours it is always day */
int32_t modTimeOfDay(int32_t hour)
{
	if (MOD_DAYTIME[MOD_DT_DUSK] < 0 || MOD_DAYTIME[MOD_DT_NIGHT] < 0 || MOD_DAYTIME[MOD_DT_DAWN] < 0) {
		return 2;
	}
	if (hour >= MOD_DAYTIME[MOD_DT_DUSK] && hour < MOD_DAYTIME[MOD_DT_NIGHT]) {
		return 0;
	}
	if (hour < MOD_DAYTIME[MOD_DT_DAWN] || hour >= MOD_DAYTIME[MOD_DT_NIGHT]) {
		return 1;
	}
	return 2;
}

/* MOD (digital clock): the hour as HH:MM in 3x5 digits on a small plate under the clock; each row of a digit is
   3 bits (bit 2 the left column), drawn as flat runs */
static const uint8_t CLOCK_DIGITS[10][5] = {
	{ 7, 5, 5, 5, 7 }, { 2, 6, 2, 2, 7 }, { 7, 1, 7, 4, 7 }, { 7, 1, 3, 1, 7 }, { 5, 5, 7, 1, 1 },
	{ 7, 4, 7, 1, 7 }, { 7, 4, 7, 5, 7 }, { 7, 1, 1, 1, 1 }, { 7, 5, 7, 5, 7 }, { 7, 5, 7, 1, 7 },
};

static void addClockRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, int32_t g, int32_t b, int32_t z)
{
	POLY_F4 *p;

	p = (POLY_F4 *)GsGetWorkBase();
	SetPolyF4(p);
	setRGB0(p, r, g, b);
	setXYWH(p, x, y, w, h);
	AddPrim(ACTIVE_ORDERING_TABLE->org + z, p++);
	GsSetWorkBase((PACKET *)p);
}

static void addClockDigit(int32_t x, int32_t y, int32_t digit)
{
	int32_t row;
	int32_t bits;

	for (row = 0; row < 5; row++) {
		bits = CLOCK_DIGITS[digit][row];
		if (bits == 7)
			addClockRect(x, y + row, 3, 1, 0xF0, 0xF0, 0xF0, 8);
		else if (bits == 6)
			addClockRect(x, y + row, 2, 1, 0xF0, 0xF0, 0xF0, 8);
		else if (bits == 3)
			addClockRect(x + 1, y + row, 2, 1, 0xF0, 0xF0, 0xF0, 8);
		else {
			if (bits & 4)
				addClockRect(x, y + row, 1, 1, 0xF0, 0xF0, 0xF0, 8);
			if (bits & 2)
				addClockRect(x + 1, y + row, 1, 1, 0xF0, 0xF0, 0xF0, 8);
			if (bits & 1)
				addClockRect(x + 2, y + row, 1, 1, 0xF0, 0xF0, 0xF0, 8);
		}
	}
}

/* the plate is 23 x 9 pixels, centred under the clock's face (x + 24), its top 2 pixels below it; a later prim at the
   same depth is drawn under an earlier one, so the edge, the dark inside and the digits go 11, 10, 9, 8 */
static void renderDigitalClock(int32_t x, int32_t y)
{
	int32_t hour = HOUR;
	int32_t minute = MINUTE;

	if (hour < 0 || hour > 23)
		hour = 0;
	if (minute < 0 || minute > 59)
		minute = 0;
	addClockRect(x, y, 23, 9, 0x62, 0xB4, 0xF6, 11);
	addClockRect(x + 1, y + 1, 21, 7, 0x18, 0x18, 0x18, 10);
	addClockRect(x + 2, y + 2, 19, 5, 0x20, 0x20, 0x20, 9);
	addClockDigit(x + 3, y + 2, hour / 10);
	addClockDigit(x + 7, y + 2, hour % 10);
	addClockRect(x + 11, y + 3, 1, 1, 0xF0, 0xF0, 0xF0, 8);
	addClockRect(x + 11, y + 5, 1, 1, 0xF0, 0xF0, 0xF0, 8);
	addClockDigit(x + 13, y + 2, minute / 10);
	addClockDigit(x + 17, y + 2, minute % 10);
}

void renderGameClock(int32_t instanceId)
{
	uint8_t isNight;
	uint8_t frame;

	if (MOD_DAYTIME[MOD_DT_CLOCK_TO] < 0 || (HOUR >= MOD_DAYTIME[MOD_DT_CLOCK_FROM] && HOUR < MOD_DAYTIME[MOD_DT_CLOCK_TO])) {
		isNight = 0;
	} else {
		isNight = 1;
	}

	frame = (CURRENT_FRAME % 16) / 4;

	renderRectPolyFT4(CLOCK_OFFSET_X - 2, -90, 16, 16, 192, 224,
			  GetTPage(0, 0, 896, 448), GetClut(256, 497), 9, 0);

	renderRectPolyFT4(CLOCK_OFFSET_X + 34, -90, 16, 16, 192, 240,
			  GetTPage(0, 0, 896, 448), GetClut(256, 497), 9, 0);

	renderRectPolyFT4(CLOCK_OFFSET_X + 8, -100, 32, 16,
			  CLOCK_TEXCOORD_U[isNight][frame],
			  CLOCK_TEXCOORD_V[isNight][frame],
			  GetTPage(0, 0, 896, 448), GetClut(256, 498), 9, 0);

	renderRectPolyFT4(CLOCK_OFFSET_X, -88, 48, 40, 208, 215,
			  GetTPage(0, 0, 896, 448), GetClut(256, 496), 10, 0);

	CLOCK_SPRITE.x = CLOCK_OFFSET_X + 23;
	GsSortSprite(&CLOCK_SPRITE, ACTIVE_ORDERING_TABLE, 9);

	renderRectPolyFT4(CLOCK_OFFSET_X + HOUR_POINT_X[HOUR],
			  HOUR_POINT_Y[HOUR] - 88,
			  6, 6, 203, 216,
			  GetTPage(0, 0, 896, 448), GetClut(256, 497), 9, 0);

	if (MOD_OPT_CLOCK) /* MOD (options): DIGITAL CLOCK */
		renderDigitalClock(CLOCK_OFFSET_X + 13, -46);

	renderStatusBars(IS_GAMETIME_RUNNING);
}

void updateMinuteHand(hour, minute)
int32_t hour;
int16_t minute;
{
	CLOCK_SPRITE.rotate = minute * 0x6000;
}

void stopGameTime(void)
{
	IS_GAMETIME_RUNNING = 0;
}

void startGameTime(void)
{
	IS_GAMETIME_RUNNING = 1;
}

/* MOD (clock as data): frames the clock advanced since the last tick (CURRENT_FRAME - LAST_HANDLED_FRAME, across the midnight wrap);
 * a script's jump in time (advanceToTime) counts as one */
int32_t clockAdvance(void)
{
	int32_t d = (int32_t)CURRENT_FRAME - (int32_t)LAST_HANDLED_FRAME;

	if (d < 0) {
		d += 28800;
	}
	return (d >= 1 && d <= 64) ? d : 1;
}

/* a multiple of period (frames) was reached or passed this tick: the clock ticks one frame at the default day, more on a short one */
int32_t clockCrossed(int32_t period)
{
	int32_t d = (int32_t)CURRENT_FRAME - (int32_t)LAST_HANDLED_FRAME;

	if (period <= 0) {
		return 0;
	}
	if (d < 0) {
		d += 28800;
	}
	if (d < 1 || d > 64) {
		return 0;
	}
	return ((int32_t)CURRENT_FRAME / period) != ((int32_t)LAST_HANDLED_FRAME / period);
}
