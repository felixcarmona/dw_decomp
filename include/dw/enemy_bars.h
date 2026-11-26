#ifndef DW_ENEMY_BARS_H
#define DW_ENEMY_BARS_H

/* Compiled only into BTL: no resident code. Arena and VS show the opponent's
 * HP in their own top-right panel, so only wild battles draw the head bar. */
#include <dw/btl.h>
#include <dw/params.h>
#include <dw/mod_options.h>

int32_t worldPosToScreenPos(SVECTOR *pos, DVECTOR *out);
void drawEntityText(int32_t color, int32_t n, int32_t x, int32_t y, int32_t value, int32_t layer);

#define ENEMY_BAR_W 50 /* the original is 55 */

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

static void renderEnemyHeadHP(DigimonEntity *d, FighterData *fighter)
{
	SVECTOR world;
	DVECTOR head, feet;
	GsSPRITE fill;
	int x, y, hp, max, digits, w, r, g;

	hp = d->stats.current.currentHP;
	max = d->stats.base.hp;
	if (GAME_STATE != 1 || NO_AI_FLAG ||
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

	/* The original bar's look, shortened: a green outline, the HUD strip inside
	 * and the big battle digits above its right end. The number is
	 * right-aligned in a fixed field, so its last digit stays put as the HP
	 * loses digits. The group spans x..x+ENEMY_BAR_W+13 and y-10..y+6. */
	x = head.vx - ENEMY_BAR_W / 2;
	y = head.vy - 10; /* three clear pixels between frame and head */
	if (x < -159) x = -159;
	if (x > 145 - ENEMY_BAR_W) x = 145 - ENEMY_BAR_W;
	if (y < -108) y = -108;
	if (y > 112) y = 112;
	/* Clear of the portrait/Auto label at left and the partner HP/MP at right. */
	if (y < -41 && x < -96) y = -41;
	if (y < -65 && x + ENEMY_BAR_W + 14 > 43) y = -65;

	enemyHPRect(x, y, ENEMY_BAR_W, 1, 10, 180, 10, 7);
	enemyHPRect(x, y + 6, ENEMY_BAR_W, 1, 10, 180, 10, 7);
	enemyHPRect(x, y + 1, 1, 5, 10, 180, 10, 7);
	enemyHPRect(x + ENEMY_BAR_W - 1, y + 1, 1, 5, 10, 180, 10, 7);

	/* Green -> yellow -> orange modulation as BTL_renderEnemyHPBars, on its
	 * 50-step scale; a living enemy keeps at least one pixel of fill. */
	w = hp * 50 / max;
	if (w < 1) w = 1;
	if (w > 50) w = 50;
	if (w >= 15) {
		r = (50 - w) * 127 / 35 + 128;
		g = 255;
	} else if (w >= 5) {
		r = 255;
		g = 255 - (15 - w) * 127 / 10;
	} else {
		r = 255;
		g = 128;
	}
	fill.attribute = 0;
	fill.x = x + 2;
	fill.y = y + 2;
	fill.w = hp * (ENEMY_BAR_W - 5) / max;
	if (fill.w < 1) fill.w = 1;
	if (fill.w > ENEMY_BAR_W - 5) fill.w = ENEMY_BAR_W - 5;
	fill.h = 3;
	fill.tpage = 0x1f;
	fill.u = 0x58;
	fill.v = 0xec;
	fill.cx = 0x110;
	fill.cy = 0x1f3;
	fill.r = r;
	fill.g = g;
	fill.b = 128;
	fill.mx = fill.my = 0;
	fill.scalex = fill.scaley = ONE;
	fill.rotate = 0;
	GsSortFastSprite(&fill, ACTIVE_ORDERING_TABLE, 7);

	/* Four digits as the original (9999); five only past it, extending left. */
	digits = (max > 9999 || hp > 9999) ? 5 : 4;
	drawEntityText(0, digits, x + ENEMY_BAR_W - 22 - (digits - 4) * 9, y - 9, hp, 7);
}
#endif
