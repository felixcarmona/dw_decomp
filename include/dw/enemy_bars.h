#ifndef DW_ENEMY_BARS_H
#define DW_ENEMY_BARS_H

/* Compiled only into BTL/STD/VS: no resident code or digit table. */
#include <dw/btl.h>
#include <dw/params.h>
#include <dw/mod_options.h>

extern uint8_t BATTLE_TOGGLE_LIFEBAR; /* one = always visible; saved with the options */
int32_t worldPosToScreenPos(SVECTOR *pos, DVECTOR *out);

static const uint8_t ENEMY_HP_DIGITS[10][5] = {
	{ 7, 5, 5, 5, 7 }, { 2, 6, 2, 2, 7 }, { 7, 1, 7, 4, 7 }, { 7, 1, 3, 1, 7 }, { 5, 5, 7, 1, 1 },
	{ 7, 4, 7, 1, 7 }, { 7, 4, 7, 5, 7 }, { 7, 1, 1, 1, 1 }, { 7, 5, 7, 5, 7 }, { 7, 5, 7, 1, 7 },
};

static void enemyHPRect(int x, int y, int w, int h, int r, int g, int b, int z)
{
	TILE *p = (TILE *)GsGetWorkBase();
	setTile(p);
	setRGB0(p, r, g, b);
	setXY0(p, x, y);
	setWH(p, w, h);
	AddPrim(ACTIVE_ORDERING_TABLE->org + z, p++);
	GsSetWorkBase((PACKET *)p);
}

static void renderEnemyHeadHP(DigimonEntity *d, FighterData *fighter, int battleState)
{
	SVECTOR world;
	DVECTOR head, feet;
	int x, y, hp, max, divisor, width, fill, ratio, r, g, row, col, bits, start;

	hp = d->stats.current.currentHP;
	max = d->stats.base.hp;
	if (GAME_STATE != battleState || NO_AI_FLAG ||
	    hp <= 0 || max <= 0 || (fighter->flags & FIGHTER_FLAG_DEAD)) return;

	/* Feet decide visibility; a tall head can project above the top edge. */
	world.vx = d->entity.posData->location.vx;
	world.vy = d->entity.posData->location.vy;
	world.vz = d->entity.posData->location.vz;
	if (worldPosToScreenPos(&world, &feet) <= 0) return;
	/* Projection includes the scrolling map offset; HUD packets use centred coordinates,
	 * exactly as renderFleeBubble/getEntityScreenPos convert their projection. */
	feet.vx -= 160 - DRAWING_OFFSET_X;
	feet.vy -= 120 - DRAWING_OFFSET_Y;
	if (feet.vx < -160 || feet.vx >= 160 || feet.vy < -120 || feet.vy >= 120) return;
	world.vy = -DIGI_PARA(d->entity.type).height;
	if (worldPosToScreenPos(&world, &head) <= 0) return;
	head.vx -= 160 - DRAWING_OFFSET_X;
	head.vy -= 120 - DRAWING_OFFSET_Y;

	/* 32x3 track, black frame (34x5); number includes its outline. */
	for (divisor = 1; hp / divisor >= 10; divisor *= 10) {}
	/* Reserve the widest HP, so losing a digit never moves the group at an edge.
	 * Only the current value is drawn: the space stays to its right. */
	for (width = 41, fill = max > hp ? max : hp; fill >= 10; fill /= 10) width += 4;
	x = head.vx - 17;
	y = head.vy - 8; /* three clear pixels between frame and head */
	if (x < -159) x = -159;
	if (x + width > 159) x = 159 - width;
	if (y < -118) y = -118; /* digit outline extends one pixel above frame */
	if (y > 114) y = 114;
	/* Wild: portrait/Auto label at left, partner HP/MP at right.
	 * Arena/VS: wider HP/MP panels at both corners. Include the digit outline. */
	if (battleState == 1) {
		if (y < -50 && x < -96) y = -50;
		if (y < -74 && x + width > 43) y = -74;
	} else if (y < -74 && (x < -32 || x + width > 35)) y = -74;

	fill = hp * 32 / max;
	if (fill < 1) fill = 1;
	if (fill > 32) fill = 32;
	ratio = hp * 50 / max;
	/* Original green -> yellow -> red rule, converted from sprite modulation. */
	if (ratio >= 15) {
		r = (50 - (ratio > 50 ? 50 : ratio)) * 254 / 35;
		g = 254;
	} else {
		r = 254;
		g = ratio >= 5 ? 254 - (15 - ratio) * 254 / 10 : 0;
	}
	enemyHPRect(x, y, 34, 5, 0, 0, 0, 10);
	enemyHPRect(x + 1, y + 1, 32, 3, 52, 52, 60, 9);
	enemyHPRect(x + 1, y + 1, fill, 3, r, g, 0, 8);
	enemyHPRect(x + 1, y + 1, fill, 1, r > 205 ? 255 : r + 50, g > 205 ? 255 : g + 50, 50, 7);

	/* Runs use fewer packets than individual pixels. Draw dilated black runs
	 * at a deeper OT level so adjacent digits never erase white strokes. */
	x += 37; /* two blank pixels after frame, then the digit's left outline */
	for (; divisor; divisor /= 10, x += 4) {
		int digit = hp / divisor;
		hp %= divisor;
		for (row = 0; row < 5; row++) {
			bits = ENEMY_HP_DIGITS[digit][row];
			for (col = 0; col < 3; col++) {
				if (!(bits & (4 >> col))) continue;
				start = col;
				while (col + 1 < 3 && (bits & (4 >> (col + 1)))) col++;
				enemyHPRect(x + start - 1, y + row - 1, col - start + 3, 3, 0, 0, 0, 8);
				enemyHPRect(x + start, y + row, col - start + 1, 1, 255, 255, 255, 7);
			}
		}
	}
}
#endif
