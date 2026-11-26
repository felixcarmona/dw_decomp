#include "dw/partner.h"
#include <dw/evolution.h>

#include <dw/entity.h>
#include <dw/evl.h>
#include <dw/species.h>
#include <dw/garbage.h>
#include <dw/ids.h>
#include <dw/level.h>
#include <dw/math.h>
#include <dw/model.h>
#include <dw/params.h>
#include <dw/tamer.h>
#include <dw/types.h>

void initializeReincarnatedPartner(int32_t type, int32_t posX, int32_t posY,
                                   int32_t posZ, int32_t rotX, int32_t rotY,
                                   int32_t rotZ);
int32_t hasDigimonRaised(int32_t digimonId);
void setDigimonRaised(int32_t type);

extern int32_t NANIMON_TRIGGER;

extern Stats DEATH_STATS;

int8_t calculateRequirementScore(int32_t current, DigimonId target,
                                 int8_t isMaxCM, int8_t isMaxBattles,
                                 DigimonId currentBest);
int8_t getNumMasteredMoves(void);

// clang-format off
EvoRequirements EVOLUTION_REQS[NUM_ORIGINAL_DIGIMON] = { /* MOD: was NUM_EVOLVING_DIGIMON (by target type) */
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0x0002, 0x0001, 0x0001, 0x0001, 0xffff, 0xffff, 0xffff,
		0x0000, 0x000f, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0x002c, 0x0001, 0x0001, 0xffff, 0x0001, 0xffff, 0xffff,
		0x0000, 0x000f, 0xffff, 0xffff, 0xfffe, 0x0000, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x0064, 0x0064, 0x0064, 0x0064,
		0x0001, 0x001e, 0x005a, 0xffff, 0xffff, 0x0023, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0x0064, 0xffff, 0xffff, 0x0064, 0x0064,
		0x0001, 0x001e, 0x005a, 0xffff, 0xffff, 0x0023, 0x10,
		0x00,
	},
	{
		0xffff, 0x0064, 0xffff, 0xffff, 0x0064, 0xffff, 0xffff,
		0x0005, 0x001e, 0xffff, 0xffff, 0x0005, 0x001c, 0x11,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x0064, 0xffff, 0xffff, 0xffff,
		0x0005, 0x0014, 0xffff, 0xffff, 0x000a, 0x001c, 0x00,
		0x00,
	},
	{
		0xffff, 0x0064, 0x0064, 0xffff, 0xffff, 0xffff, 0xffff,
		0x0003, 0x001e, 0xffff, 0xffff, 0x0005, 0x001c, 0x01,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0x0190, 0x012c, 0x01f4, 0x01f4, 0x012c, 0x012c,
		0x000a, 0x0041, 0x005f, 0xffff, 0x001e, 0x001e, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x0190, 0x012c, 0x012c, 0x0190,
		0x000f, 0x0005, 0xffff, 0x005a, 0xffff, 0x0019, 0x00,
		0x00,
	},
	{
		0xffff, 0x012c, 0x012c, 0x012c, 0x012c, 0x012c, 0x012c,
		0x0000, 0x0028, 0xffff, 0xffff, 0x0032, 0x0031, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0x0002, 0xffff, 0xffff, 0xffff, 0x0001, 0x0001, 0x0001,
		0x0000, 0x000f, 0xffff, 0xffff, 0xfffe, 0x0000, 0x00,
		0x00,
	},
	{
		0x0010, 0x0001, 0xffff, 0x0001, 0xffff, 0x0001, 0xffff,
		0x0000, 0x000f, 0xffff, 0xffff, 0xfffe, 0x0000, 0x00,
		0x00,
	},
	{
		0x0020, 0x0064, 0x0064, 0x0064, 0xffff, 0x0064, 0xffff,
		0x0001, 0x001e, 0xffff, 0xffff, 0xffff, 0x0023, 0x10,
		0x00,
	},
	{
		0x001f, 0xffff, 0x0064, 0xffff, 0xffff, 0xffff, 0x0064,
		0x0000, 0x0014, 0xffff, 0xffff, 0xffff, 0x0023, 0x10,
		0x00,
	},
	{
		0x002d, 0xffff, 0xffff, 0xffff, 0xffff, 0x0064, 0xffff,
		0x0003, 0x0014, 0xffff, 0xffff, 0xffff, 0x0023, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0x0064, 0xffff, 0xffff, 0x0064, 0xffff,
		0x0001, 0x001e, 0x005a, 0xffff, 0xffff, 0x001c, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0x0064, 0xffff, 0xffff, 0xffff, 0x0064,
		0x0005, 0x001e, 0xffff, 0x0032, 0xffff, 0x001c, 0x10,
		0x00,
	},
	{
		0xffff, 0x0064, 0xffff, 0xffff, 0xffff, 0xffff, 0x0064,
		0x0005, 0x0028, 0x003c, 0xffff, 0xffff, 0x001c, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0x0064, 0xffff, 0xffff, 0xffff, 0xffff,
		0x0005, 0x000a, 0xffff, 0x0032, 0xffff, 0x0015, 0x00,
		0x00,
	},
	{
		0xffff, 0x0190, 0x0258, 0x0190, 0x0190, 0x00c8, 0x01f4,
		0x000a, 0x001e, 0xffff, 0xffff, 0x0028, 0x002d, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x01f4, 0x0190, 0x0190, 0x0190,
		0x000f, 0x000a, 0xffff, 0x005f, 0xffff, 0x001e, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0x001e, 0x0001, 0xffff, 0x0001, 0xffff, 0xffff, 0x0001,
		0x0000, 0x000f, 0xffff, 0xffff, 0xfffe, 0x0000, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0x0064, 0xffff, 0xffff, 0xffff, 0x0064, 0xffff,
		0x0003, 0x001e, 0xffff, 0xffff, 0x000a, 0x0023, 0x10,
		0x00,
	},
	{
		0xffff, 0x0064, 0xffff, 0x0064, 0xffff, 0xffff, 0xffff,
		0x0005, 0x001e, 0xffff, 0xffff, 0x000f, 0x0023, 0x00,
		0x00,
	},
	{
		0x0004, 0x0064, 0xffff, 0xffff, 0x0064, 0xffff, 0xffff,
		0x0005, 0x0028, 0xffff, 0xffff, 0xffff, 0x0023, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0064,
		0x0003, 0x001e, 0x003c, 0xffff, 0xffff, 0x001c, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0x0064, 0xffff, 0xffff, 0xffff, 0xffff,
		0x0003, 0x0014, 0xffff, 0x0032, 0xffff, 0x001c, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x0064, 0xffff, 0xffff, 0xffff,
		0x0003, 0x0028, 0xffff, 0x0032, 0xffff, 0x001c, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0x00c8, 0x0190, 0x00c8, 0x0190, 0x00c8, 0x0190,
		0x0005, 0x0028, 0x005f, 0xffff, 0x001e, 0x001e, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x0190, 0xffff, 0x012c, 0x0190,
		0x000f, 0x0005, 0xffff, 0x005f, 0x0064, 0x0023, 0x00,
		0x00,
	},
	{
		0xffff, 0x00c8, 0x012c, 0x0190, 0x00c8, 0x0190, 0x012c,
		0x0000, 0x000f, 0xffff, 0xffff, 0x0032, 0x0031, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0x001e, 0xffff, 0x0001, 0xffff, 0x0001, 0x0001, 0xffff,
		0x0000, 0x000f, 0xffff, 0xffff, 0xfffe, 0x0000, 0x00,
		0x00,
	},
	{
		0x002c, 0xffff, 0x0001, 0xffff, 0xffff, 0x0001, 0x0001,
		0x0000, 0x000f, 0xffff, 0xffff, 0xfffe, 0x0000, 0x00,
		0x00,
	},
	{
		0xffff, 0x0064, 0xffff, 0xffff, 0x0064, 0xffff, 0x0064,
		0x0003, 0x0028, 0xffff, 0xffff, 0x0005, 0x0023, 0x11,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x0064, 0xffff, 0x0064, 0x0064,
		0x0001, 0x0014, 0xffff, 0xffff, 0x000a, 0x0023, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0x0064, 0xffff, 0xffff,
		0x0003, 0x001e, 0xffff, 0xffff, 0x0005, 0x0023, 0x01,
		0x00,
	},
	{
		0x002d, 0x0064, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0x0003, 0x001e, 0xffff, 0xffff, 0xffff, 0x001c, 0x00,
		0x00,
	},
	{
		0x0020, 0x0064, 0x0064, 0x0064, 0xffff, 0x0064, 0xffff,
		0x0005, 0x001e, 0xffff, 0xffff, 0xffff, 0x001c, 0x00,
		0x00,
	},
	{
		0xffff, 0x0064, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0x0005, 0x0014, 0xffff, 0xffff, 0x0005, 0x001c, 0x01,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x00,
		0x00,
	},
	{
		0xffff, 0x012c, 0x01f4, 0x01f4, 0x012c, 0x0190, 0x0190,
		0x000a, 0x0037, 0xffff, 0xffff, 0x001e, 0x001e, 0x10,
		0x00,
	},
	{
		0xffff, 0xffff, 0xffff, 0x012c, 0x012c, 0x0190, 0x0190,
		0x000f, 0x0005, 0xffff, 0x005f, 0xffff, 0x0019, 0x00,
		0x00,
	},
	{
		0xffff, 0x012c, 0x012c, 0x0190, 0x0190, 0x0190, 0x012c,
		0x0000, 0x000a, 0xffff, 0xffff, 0x0064, 0x0031, 0x10,
		0x00,
	},
	{
		0x0010, 0xffff, 0x0001, 0xffff, 0x0001, 0xffff, 0x0001,
		0x0000, 0x000f, 0xffff, 0xffff, 0xfffe, 0x0000, 0x00,
		0x00,
	},
	{
		0xffff, 0xffff, 0x0064, 0x0064, 0xffff, 0x0064, 0xffff,
		0x0001, 0x000a, 0xffff, 0xffff, 0x000f, 0x0023, 0x10,
		0x00,
	},
	{
		0xffff, 0x0190, 0x0190, 0xffff, 0xffff, 0x0190, 0x0258,
		0x0003, 0x001e, 0x0064, 0xffff, 0x0000, 0x0028, 0x11,
		0x00,
	},
	{
		0xffff, 0x02bc, 0xffff, 0x0190, 0x0258, 0x0190, 0xffff,
		0x0005, 0x0037, 0xffff, 0xffff, 0x0000, 0x0028, 0x11,
		0x00,
	},
	{
		0xffff, 0xffff, 0x0190, 0x01f4, 0x0190, 0xffff, 0x0190,
		0x0005, 0x001e, 0xffff, 0xffff, 0x0000, 0x0028, 0x11,
		0x00,
	},
	{
		0xffff, 0x0190, 0xffff, 0xffff, 0x0190, 0x0190, 0x0190,
		0x0003, 0x001e, 0xffff, 0xffff, 0x0000, 0x0031, 0x11,
		0x00,
	},
	/*
	 * MOD (master-tyrannomon): from Tyrannomon (its only path). Heavier
	 * than MetalGreymon (65) and Megadramon (55), the other two targets of
	 * Tyrannomon, and with no Speed/MP requirement: HP 5000, Off 450,
	 * Def 450, Brains 300, at most 5 care mistakes (flag 0x10), weight
	 * 75..85; bonus: discipline 90 or 50 battles.
	 */
	[DIGIMON_MASTER_TYRANNOMON] = {
		0xffff, 0x01f4, 0xffff, 0x01c2, 0x01c2, 0xffff, 0x012c,
		0x0005, 0x0050, 0x005a, 0xffff, 0x0032, 0xffff, 0x10,
		0x00,
	},
	/*
	 * MOD (cherrymon): from Vegiemon (with Piximon, which needs 15 care
	 * mistakes or more). A well kept, heavy tree: HP 4000, MP 4000,
	 * Def 400, Brains 400, at most 5 care mistakes (flag 0x10), weight
	 * 25..35; bonus: discipline 90 or 30 techniques learned.
	 */
	[DIGIMON_CHERRYMON] = {
		0xffff, 0x0190, 0x0190, 0xffff, 0x0190, 0xffff, 0x0190,
		0x0005, 0x001e, 0x005a, 0xffff, 0xffff, 0x001e, 0x10,
		0x00,
	},
	/*
	 * MOD (partners-npc-a): from Sukamon (with Etemon, which wants a
	 * spotless record: 0 care mistakes, weight 15). The King is the
	 * neglected, well fed road: HP 3000, MP 2000, Off 300, Def 300,
	 * Brains 200, at least 10 care mistakes (no flag 0x10), weight
	 * 35..45; bonus: 30 battles.
	 */
	[DIGIMON_KING_SUKAMON] = {
		0xffff, 0x012c, 0x00c8, 0x012c, 0x012c, 0xffff, 0x00c8,
		0x000a, 0x0028, 0xffff, 0xffff, 0x001e, 0xffff, 0x00,
		0x00,
	},
	/*
	 * MOD (partners-npc-a): from Frigimon (with MetalMamemon and
	 * Mamemon, which want a light Digimon: weight 10 and 5). Icemon is
	 * the heavy, well kept road: HP 4000, Off 300, Def 500, Brains 300,
	 * at most 5 care mistakes (flag 0x10), weight 35..45; bonus: 40
	 * battles.
	 */
	[DIGIMON_ICEMON] = {
		0xffff, 0x0190, 0xffff, 0x012c, 0x01f4, 0xffff, 0x012c,
		0x0005, 0x0028, 0xffff, 0xffff, 0x0028, 0xffff, 0x10,
		0x00,
	},
	/*
	 * MOD (partners-npc-a): from Monochromon (with MetalGreymon, weight
	 * 65, and MetalMamemon, weight 10). Meteormon is the middle weight:
	 * HP 4000, MP 3000, Off 450, Def 450, at most 5 care mistakes (flag
	 * 0x10), weight 45..55; bonus: 50 battles.
	 */
	[DIGIMON_METEORMON] = {
		0xffff, 0x0190, 0x012c, 0x01c2, 0x01c2, 0xffff, 0xffff,
		0x0005, 0x0032, 0xffff, 0xffff, 0x0032, 0xffff, 0x10,
		0x00,
	},
	/*
	 * MOD (partners-npc-a): natural roads to the three partners that
	 * only evolution items reached (their rows were past the original
	 * table, all zero).
	 * Panjyamon, from Elecmon (with Leomon: Off/Speed/Brains 100, at
	 * most 1 care mistake, weight 20): HP 1000, Off 100, Def 100,
	 * Speed 100, at most 3 care mistakes, weight 25..35; bonus
	 * discipline 70 or 15 battles.
	 */
	[DIGIMON_PANJYAMON] = {
		0xffff, 0x0064, 0xffff, 0x0064, 0x0064, 0x0064, 0xffff,
		0x0003, 0x001e, 0x0046, 0xffff, 0x000f, 0xffff, 0x10,
		0x00,
	},
	/*
	 * Gigadramon, from Seadramon (with Megadramon, weight 50..60, and
	 * MegaSeadramon, weight 25..35 and no battles): MP 5000, Off 500,
	 * Def 400, Brains 500, at most 5 care mistakes, weight 65..75 (the
	 * heaviest dragon); bonus 35 techniques. Without HP and Speed in
	 * the list it also wins the stat-average tie-break against
	 * Megadramon when both qualify.
	 */
	[DIGIMON_GIGADRAMON] = {
		0xffff, 0xffff, 0x01f4, 0x01f4, 0x0190, 0xffff, 0x01f4,
		0x0005, 0x0046, 0xffff, 0xffff, 0xffff, 0x0023, 0x10,
		0x00,
	},
	/*
	 * MetalEtemon, from Etemon after 96 hours as an Ultimate: HP 5000,
	 * MP 6000, Off 600, Def 500, Speed 600, Brains 500, no care
	 * mistakes, weight 10..20; bonus 100 battles or 45 techniques.
	 */
	[DIGIMON_METALETEMON] = {
		0xffff, 0x01f4, 0x0258, 0x0258, 0x01f4, 0x0258, 0x01f4,
		0x0000, 0x000f, 0xffff, 0xffff, 0x0064, 0x002d, 0x10,
		0x00,
	},
	/*
	 * MOD (partners-npc-b): from Numemon, the neglected road: Monzaemon
	 * needs no care mistake at all, WaruMonzaemon at least 10 (no flag
	 * 0x10: a minimum). HP 3000, Off 400, Def 300, weight 35..45; bonus:
	 * 50 battles or 40 techniques learned.
	 */
	/*
	 * MOD (partners-npc-b): from Devimon (with SkullGreymon, 10 care
	 * mistakes or more, and Megadramon, weight 50..60): a slender, well
	 * kept night lord. HP 4000, MP 6000, Speed 400, Brains 500, at most 5
	 * care mistakes (flag 0x10), weight 25..35; bonus: 40 battles or 35
	 * techniques.
	 */
	/*
	 * MOD (partners-npc-b): from Whamon (with MegaSeadramon, weight
	 * 25..35, and Mamemon, 15 care mistakes or more): a heavy, well kept
	 * lord of the swamp. HP 5000, Def 400, Brains 300, at most 5 care
	 * mistakes (flag 0x10), weight 55..65; bonus: discipline 90 or 30
	 * techniques.
	 */
	[DIGIMON_SHOGUNGEKOMON] = {
		0xffff, 0x01f4, 0xffff, 0xffff, 0x0190, 0xffff, 0x012c,
		0x0005, 0x003c, 0x005a, 0xffff, 0xffff, 0x001e, 0x10,
		0x00,
	},
	[DIGIMON_MYOTISMON] = {
		0xffff, 0x0190, 0x0258, 0xffff, 0xffff, 0x0190, 0x01f4,
		0x0005, 0x001e, 0xffff, 0xffff, 0x0028, 0x0023, 0x10,
		0x00,
	},
	[DIGIMON_WARUMONZAEMON] = {
		0xffff, 0x012c, 0xffff, 0x0190, 0x012c, 0xffff, 0xffff,
		0x000a, 0x0028, 0xffff, 0xffff, 0x0032, 0x0028, 0x00,
		0x00,
	},
	/*
	 * MOD (machinedramon): from Megadramon or Gigadramon, the only
	 * Ultimates that evolve again (getModUltimateEvolutionTarget). The
	 * final boss: HP 7000, MP 6000, Off 600, Def 600, Speed 400, Brains
	 * 400, at most 3 care mistakes (flag 0x10), weight 65..75; bonus:
	 * discipline 95 or 100 battles.
	 */
	[DIGIMON_MACHINEDRAMON] = {
		0xffff, 0x02bc, 0x0258, 0x0258, 0x0258, 0x0190, 0x0190,
		0x0003, 0x0046, 0x005f, 0xffff, 0x0064, 0xffff, 0x10,
		0x00,
	},
};

EvoStatsGains EVOLUTION_STATS_GAINS[NUM_ORIGINAL_DIGIMON] = { /* MOD: was NUM_PARTNER_DIGIMON */
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x0000 },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x0001 },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x0002 },
	{ 0x03e8, 0x01f4, 0x0064, 0x0032, 0x0032, 0x0032, 0x0003 },
	{ 0x03e8, 0x01f4, 0x0032, 0x0064, 0x0032, 0x0032, 0x0004 },
	{ 0x07d0, 0x05dc, 0x00c8, 0x00c8, 0x00c8, 0x00c8, 0x0005 },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x0006 },
	{ 0x05dc, 0x07d0, 0x0096, 0x0096, 0x00c8, 0x00c8, 0x0007 },
	{ 0x07d0, 0x05dc, 0x0096, 0x00c8, 0x0096, 0x0096, 0x0008 },
	{ 0x03e8, 0x05dc, 0x00fa, 0x0096, 0x0096, 0x0096, 0x0009 },
	{ 0x07d0, 0x07d0, 0x0096, 0x0096, 0x0064, 0x0096, 0x000a },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x000b },
	{ 0x1388, 0x1388, 0x01f4, 0x01f4, 0x01f4, 0x01f4, 0x000c },
	{ 0x0bb8, 0x0bb8, 0x0258, 0x01f4, 0x01f4, 0x0258, 0x000d },
	{ 0x1388, 0x1388, 0x01f4, 0x01f4, 0x01f4, 0x01f4, 0x000e },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x000f },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x0010 },
	{ 0x01f4, 0x01f4, 0x0032, 0x0064, 0x0064, 0x0032, 0x0011 },
	{ 0x01f4, 0x01f4, 0x0064, 0x0032, 0x0064, 0x0032, 0x0012 },
	{ 0x07d0, 0x05dc, 0x00c8, 0x00c8, 0x00c8, 0x0064, 0x0013 },
	{ 0x05dc, 0x07d0, 0x0096, 0x0096, 0x0096, 0x00fa, 0x0014 },
	{ 0x05dc, 0x05dc, 0x0096, 0x0064, 0x00fa, 0x0096, 0x0015 },
	{ 0x05dc, 0x05dc, 0x0096, 0x00c8, 0x0096, 0x0096, 0x0016 },
	{ 0x05dc, 0x07d0, 0x0064, 0x0096, 0x0096, 0x00c8, 0x0017 },
	{ 0x09c4, 0x05dc, 0x0064, 0x0096, 0x0064, 0x00c8, 0x0018 },
	{ 0x05dc, 0x07d0, 0x0064, 0x0096, 0x0096, 0x0064, 0x0019 },
	{ 0x1388, 0x1388, 0x0258, 0x0258, 0x0190, 0x0190, 0x001a },
	{ 0x0bb8, 0x0bb8, 0x0258, 0x0258, 0x0258, 0x01f4, 0x001b },
	{ 0x1388, 0x1388, 0x01f4, 0x01f4, 0x01f4, 0x01f4, 0x001c },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x001d },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x001e },
	{ 0x01f4, 0x01f4, 0x0064, 0x0032, 0x0032, 0x0064, 0x001f },
	{ 0x03e8, 0x03e8, 0x0032, 0x0032, 0x0032, 0x0032, 0x0020 },
	{ 0x07d0, 0x05dc, 0x0096, 0x0096, 0x00c8, 0x0096, 0x0021 },
	{ 0x09c4, 0x03e8, 0x00fa, 0x0096, 0x0096, 0x0064, 0x0022 },
	{ 0x07d0, 0x05dc, 0x0096, 0x00fa, 0x0064, 0x0064, 0x0023 },
	{ 0x05dc, 0x05dc, 0x0096, 0x0096, 0x0096, 0x00fa, 0x0024 },
	{ 0x05dc, 0x09c4, 0x0096, 0x0064, 0x0096, 0x0064, 0x0025 },
	{ 0x05dc, 0x05dc, 0x00fa, 0x0096, 0x0096, 0x0096, 0x0026 },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0005, 0x0027 },
	{ 0x0fa0, 0x1770, 0x0190, 0x0258, 0x0190, 0x0258, 0x0028 },
	{ 0x0bb8, 0x0bb8, 0x0258, 0x0258, 0x01f4, 0x0258, 0x0029 },
	{ 0x0fa0, 0x1388, 0x0258, 0x0190, 0x0258, 0x01f4, 0x002a },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x002b },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x002c },
	{ 0x01f4, 0x03e8, 0x0032, 0x0032, 0x0064, 0x0032, 0x002d },
	{ 0x01f4, 0x03e8, 0x0032, 0x0032, 0x0032, 0x0064, 0x002e },
	{ 0x07d0, 0x05dc, 0x0096, 0x00fa, 0x0096, 0x00c8, 0x002f },
	{ 0x05dc, 0x05dc, 0x00fa, 0x0096, 0x00c8, 0x00c8, 0x0030 },
	{ 0x05dc, 0x05dc, 0x0096, 0x00c8, 0x0096, 0x0096, 0x0031 },
	{ 0x09c4, 0x05dc, 0x0064, 0x0096, 0x0096, 0x0096, 0x0032 },
	{ 0x07d0, 0x07d0, 0x00c8, 0x0096, 0x00c8, 0x0064, 0x0033 },
	{ 0x07d0, 0x05dc, 0x0096, 0x0096, 0x0096, 0x0096, 0x0034 },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000a, 0x0035 },
	{ 0x1770, 0x1770, 0x0258, 0x01f4, 0x0258, 0x01f4, 0x0036 },
	{ 0x0bb8, 0x0bb8, 0x01f4, 0x01f4, 0x0258, 0x0258, 0x0037 },
	{ 0x1388, 0x1388, 0x0258, 0x0258, 0x0258, 0x01f4, 0x0038 },
	{ 0x01f4, 0x01f4, 0x0032, 0x0064, 0x0032, 0x0064, 0x0039 },
	{ 0x05dc, 0x07d0, 0x00c8, 0x0096, 0x00c8, 0x0096, 0x003a },
	{ 0x1770, 0x1770, 0x0190, 0x0190, 0x0258, 0x0258, 0x003b },
	{ 0x1388, 0x1388, 0x01f4, 0x01f4, 0x01f4, 0x01f4, 0x003c },
	{ 0x0bb8, 0x1770, 0x0258, 0x0258, 0x012c, 0x0258, 0x003d },
	{ 0x1770, 0x0bb8, 0x012c, 0x0258, 0x0258, 0x0258, 0x003e },
	{ 0x08ca, 0x08ca, 0x0177, 0x00e1, 0x012c, 0x012c, 0x003f },
	{ 0x2328, 0x2328, 0x0384, 0x02ee, 0x0384, 0x02ee, 0x0040 },
	{ 0x1770, 0x1d4c, 0x0384, 0x02bc, 0x0384, 0x02ee, 0x0041 },
	/* MOD (master-tyrannomon): an Ultimate between MetalGreymon and Gigadramon */
	[DIGIMON_MASTER_TYRANNOMON] = { 0x1770, 0x0fa0, 0x028a, 0x028a, 0x0190, 0x01f4,
					DIGIMON_MASTER_TYRANNOMON },
	/* MOD (cherrymon): a slow, sturdy Ultimate: HP 6000, MP 5000, Off 550, Def 650,
	 * Speed 350, Brains 550 */
	[DIGIMON_CHERRYMON] = { 0x1770, 0x1388, 0x0226, 0x028a, 0x015e, 0x0226, DIGIMON_CHERRYMON },
	/* MOD (partners-npc-a): a tough, slow Ultimate: HP 5000, MP 4000, Off 500, Def 550,
	 * Speed 300, Brains 400 (from Sukamon the backup formula applies instead) */
	[DIGIMON_KING_SUKAMON] = { 0x1388, 0x0fa0, 0x01f4, 0x0226, 0x012c, 0x0190,
				   DIGIMON_KING_SUKAMON },
	/* MOD (partners-npc-a): a hard, slow Ultimate: HP 5000, MP 3000, Off 550, Def 650,
	 * Speed 300, Brains 450 */
	[DIGIMON_ICEMON] = { 0x1388, 0x0bb8, 0x0226, 0x028a, 0x012c, 0x01c2, DIGIMON_ICEMON },
	/* MOD (partners-npc-a): a hard hitter: HP 5500, MP 4500, Off 650, Def 550, Speed 350,
	 * Brains 400 */
	[DIGIMON_METEORMON] = { 0x157c, 0x1194, 0x028a, 0x0226, 0x015e, 0x0190, DIGIMON_METEORMON },
	/* MOD (partners-npc-b): a brawler, harder hitting than Monzaemon: HP 5000, MP 4000,
	 * Off 600, Def 450, Speed 450, Brains 400 */
	/* MOD (partners-npc-b): a fast caster: HP 6000, MP 6000, Off 550, Def 450, Speed 600,
	 * Brains 600 */
	/* MOD (partners-npc-b): a slow, heavy Ultimate: HP 7000, MP 3500, Off 500, Def 600,
	 * Speed 300, Brains 500 */
	[DIGIMON_SHOGUNGEKOMON] = { 0x1b58, 0x0dac, 0x01f4, 0x0258, 0x012c, 0x01f4, DIGIMON_SHOGUNGEKOMON },
	[DIGIMON_MYOTISMON] = { 0x1770, 0x1770, 0x0226, 0x01c2, 0x0258, 0x0258, DIGIMON_MYOTISMON },
	[DIGIMON_WARUMONZAEMON] = { 0x1388, 0x0fa0, 0x0258, 0x01c2, 0x01c2, 0x0190, DIGIMON_WARUMONZAEMON },
	/* MOD (machinedramon): stronger than any Ultimate: HP 8000, MP 7000, Off 800,
	 * Def 750, Speed 600, Brains 650 */
	[DIGIMON_MACHINEDRAMON] = { 0x1f40, 0x1b58, 0x0320, 0x02ee, 0x0258, 0x028a,
				    DIGIMON_MACHINEDRAMON },
	/* MOD (jijimon): a wise old Ultimate, reached only from the lab menu: HP 4000,
	 * MP 6000, Off 450, Def 500, Speed 300, Brains 800 */
	[DIGIMON_JIJIMON] = { 0x0fa0, 0x1770, 0x01c2, 0x01f4, 0x012c, 0x0320, DIGIMON_JIJIMON },
	/* MOD (analogman): a brilliant, frail Ultimate, reached only from the lab menu:
	 * HP 3500, MP 7000, Off 400, Def 350, Speed 450, Brains 900 */
	[DIGIMON_ANALOGMAN] = { 0x0dac, 0x1b58, 0x0190, 0x015e, 0x01c2, 0x0384, DIGIMON_ANALOGMAN },
};

EvolutionPath EVOLUTION_PATHS[NUM_EVOLVING_DIGIMON - 1] = {
	{
		{   -1,   -1,   -1,   -1,   -1 },
		{   -1,   -1, 0x02,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x01,   -1,   -1 },
		{   -1,   -1, 0x03, 0x11,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x02,   -1,   -1 },
		{ 0x05, 0x09, 0x15, 0x24, 0x2f, 0x08 },
	},
	{
		{   -1,   -1, 0x2c,   -1,   -1 },
		{   -1, 0x0a, 0x18, 0x23, 0x31,   -1 },
	},
	{
		{   -1,   -1, 0x03,   -1,   -1 },
		{   -1,   -1, 0x0c, 0x1a,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		/* MOD (partners-npc-b): + Myotismon, centred like Ninjamon's three */
		{   -1, 0x1a, 0x36, DIGIMON_MYOTISMON,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x2d,   -1,   -1 },
		{   -1,   -1, 0x36, 0x3b,   -1,   -1 },
	},
	{
		{   -1, 0x03, 0x11, 0x1f,   -1 },
		/* MOD (master-tyrannomon): + MasterTyrannomon, centred like Ninjamon's three */
		{   -1, 0x0c, 0x36, DIGIMON_MASTER_TYRANNOMON,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x03,   -1,   -1 },
		{   -1,   -1, 0x0c, 0x28,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x04,   -1,   -1 },
		/* MOD (partners-npc-a): + Gigadramon, Megadramon's twin */
		{   -1, 0x36, 0x3d, DIGIMON_GIGADRAMON,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		/* MOD (partners-npc-b): + WaruMonzaemon, centred like the other pairs */
		{   -1,   -1, 0x0e, DIGIMON_WARUMONZAEMON,   -1,   -1 },
	},
	{
		{ 0x05, 0x09, 0x2f, 0x26, 0x08 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{ 0x30, 0x3a, 0x18, 0x17, 0x34 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x0b,   -1,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		{   -1,   -1, 0x10,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x0f,   -1,   -1 },
		{   -1,   -1, 0x12, 0x39,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x02,   -1,   -1 },
		{ 0x24, 0x2f, 0x26, 0x08, 0x22, 0x16 },
	},
	{
		{   -1,   -1, 0x10,   -1,   -1 },
		/* MOD (partners-npc-a): + Panjyamon, Leomon's ice twin */
		{   -1, 0x30, 0x14, 0x25, 0x32, DIGIMON_PANJYAMON },
	},
	{
		{   -1, 0x2d, 0x20,   -1,   -1 },
		{   -1,   -1, 0x3c, 0x1b,   -1,   -1 },
	},
	{
		{   -1, 0x1f, 0x12,   -1,   -1 },
		{   -1,   -1, 0x28, 0x3b,   -1,   -1 },
	},
	{
		{   -1, 0x03, 0x2d,   -1,   -1 },
		{   -1,   -1, 0x3b,   -1,   -1,   -1 },
	},
	{
		{   -1, 0x11, 0x39,   -1,   -1 },
		{   -1,   -1, 0x1a, 0x3d,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x39,   -1,   -1 },
		/* MOD (partners-npc-a): Frigimon -> MetalMamemon, Mamemon or Icemon */
		{   -1, 0x1b, 0x0d, DIGIMON_ICEMON,   -1,   -1 },
	},
	{
		{   -1, 0x2e, 0x04, 0x39,   -1 },
		/* MOD (partners-npc-b): + ShogunGekomon, centred like Ninjamon's three */
		{   -1, 0x3d, 0x0d, DIGIMON_SHOGUNGEKOMON,   -1,   -1 },
	},
	{
		{   -1, 0x20, 0x2e,   -1,   -1 },
		/* MOD (cherrymon): + Cherrymon, centred like the other pairs */
		{   -1,   -1, 0x37, DIGIMON_CHERRYMON,   -1,   -1 },
	},
	{
		{ 0x05, 0x06, 0x25, 0x16, 0x34 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{ 0x2f, 0x13, 0x3a, 0x17,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		{   -1,   -1, 0x1e,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x1d,   -1,   -1 },
		{   -1,   -1, 0x1f, 0x2d,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x1e,   -1,   -1 },
		{ 0x26, 0x08, 0x22, 0x30, 0x14, 0x21 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		{   -1, 0x25, 0x13, 0x33, 0x19,   -1 },
	},
	{
		{   -1, 0x1f, 0x2d,   -1,   -1 },
		{   -1,   -1, 0x29, 0x3b,   -1,   -1 },
	},
	{
		{   -1, 0x11, 0x1f,   -1,   -1 },
		{   -1,   -1, 0x28, 0x29,   -1,   -1 },
	},
	{
		{   -1, 0x04, 0x39,   -1,   -1 },
		{   -1,   -1, 0x3c, 0x3d,   -1,   -1 },
	},
	{
		{   -1, 0x03, 0x11,   -1,   -1 },
		{   -1,   -1, 0x28, 0x29,   -1,   -1 },
	},
	{
		{   -1, 0x12, 0x20,   -1,   -1 },
		{   -1,   -1, 0x1a, 0x29,   -1,   -1 },
	},
	{
		{   -1, 0x11, 0x1f,   -1,   -1 },
		{   -1,   -1, 0x0c,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		/* MOD (partners-npc-a): Sukamon -> Etemon or King of Sukamon */
		{   -1,   -1, 0x2a, DIGIMON_KING_SUKAMON,   -1,   -1 },
	},
	{
		{ 0x09, 0x24, 0x22, 0x30, 0x14 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{ 0x24, 0x22, 0x25, 0x21,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x27,   -1,   -1 },
		/* MOD (partners-npc-a): Etemon -> MetalEtemon (an Ultimate evolving: partner_impl.c) */
		{   -1,   -1, DIGIMON_METALETEMON,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		{   -1,   -1, 0x2c,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x2b,   -1,   -1 },
		{   -1,   -1, 0x2e, 0x04,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x1e,   -1,   -1 },
		{ 0x15, 0x07, 0x32, 0x21, 0x13,   -1 },
	},
	{
		{   -1,   -1, 0x2c,   -1,   -1 },
		{ 0x33, 0x19, 0x3a, 0x18, 0x31,   -1 },
	},
	{
		{   -1, 0x03, 0x11,   -1,   -1 },
		/* MOD (partners-npc-a): Monochromon -> MetalGreymon, MetalMamemon or Meteormon */
		{   -1, 0x0c, 0x1b, DIGIMON_METEORMON,   -1,   -1 },
	},
	{
		{   -1, 0x1f, 0x12,   -1,   -1 },
		{   -1,   -1, 0x28, 0x0d,   -1,   -1 },
	},
	{
		{   -1, 0x2e, 0x04,   -1,   -1 },
		{   -1,   -1, 0x3d,   -1,   -1,   -1 },
	},
	{
		{   -1, 0x12, 0x2d,   -1,   -1 },
		{   -1,   -1, 0x3b, 0x37,   -1,   -1 },
	},
	{
		{   -1, 0x20, 0x2e,   -1,   -1 },
		{   -1,   -1, 0x3c, 0x37,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x39,   -1,   -1 },
		{   -1,   -1, 0x1a, 0x0d,   -1,   -1 },
	},
	{
		{   -1,   -1,   -1,   -1,   -1 },
		{   -1,   -1, 0x38,   -1,   -1,   -1 },
	},
	{
		{ 0x08, 0x06, 0x07, 0x0a,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{ 0x32, 0x33, 0x19, 0x3a,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x35,   -1,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{   -1,   -1, 0x10,   -1,   -1 },
		{ 0x18, 0x23, 0x16, 0x17, 0x34,   -1 },
	},
	{
		{   -1,   -1, 0x2e,   -1,   -1 },
		{   -1, 0x37, 0x1b, 0x0d,   -1,   -1 },
	},
	{
		{ 0x15, 0x14, 0x07, 0x32, 0x21 },
		{   -2,   -1,   -1,   -1,   -1, 0x00 }, /* MOD: 0xfe as s8 */
	},
	{
		{   -1, 0x13, 0x33, 0x23,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{ 0x0a, 0x18, 0x23, 0x31, 0x16 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
	{
		{ 0x30, 0x16, 0x17, 0x34,   -1 },
		{   -1,   -1,   -1,   -1,   -1,   -1 },
	},
};
// clang-format on

int16_t EVOLUTION_TARGET;
uint8_t HAS_USED_EVOITEM;

GARBAGE(getFreshEvolutionTarget, 10);

/*
 * MOD (evolution rules as data): the rules are tables that start empty (-1 = does not exist) and tools/balance.py build
 * writes from content/tables/evolution-rules.json (include/dw/level.h)
 */
int16_t MOD_EVO_TIMING[MOD_EVO_TIMING_SIZE] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int16_t MOD_EVO_FRESH[MOD_EVO_PAIR_MAX][2] = { { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 },
					       { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 } };
int16_t MOD_EVO_ULTIMATE[MOD_EVO_PAIR_MAX][2] = { { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 },
						  { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 } };
int16_t MOD_EVO_SPECIAL[MOD_EVO_SPECIAL_SIZE] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
#define MOD_EVO_NO_RULE                                                                                        \
	{                                                                                                      \
		-1, { -1, -1, -1, -1 }, 0, 0, -1, 0, { { -1, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 } } \
	}
ModEvoItem MOD_EVO_ITEMS[MOD_EVO_ITEM_MAX] = { { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 } };

ModEvoItem *getModEvoItem(int32_t item)
{
	int32_t i;

	for (i = 0; i < MOD_EVO_ITEM_MAX; i++) {
		if (MOD_EVO_ITEMS[i].item == item && MOD_EVO_ITEMS[i].target >= 0)
			return &MOD_EVO_ITEMS[i];
	}
	return NULL;
}

ModEvoRule MOD_EVO_RULES[MOD_EVO_RULES_MAX] = { MOD_EVO_NO_RULE, MOD_EVO_NO_RULE, MOD_EVO_NO_RULE, MOD_EVO_NO_RULE,
						MOD_EVO_NO_RULE, MOD_EVO_NO_RULE, MOD_EVO_NO_RULE, MOD_EVO_NO_RULE,
						MOD_EVO_NO_RULE, MOD_EVO_NO_RULE };

int16_t getFreshEvolutionTarget(int32_t currentDigimon) {
  int32_t result;
  int32_t i;

  result = -1;
  for (i = 0; i < MOD_EVO_PAIR_MAX; i++) {
    if (MOD_EVO_FRESH[i][0] == currentDigimon)
      result = MOD_EVO_FRESH[i][1];
  }

  PARTNER_PARA.evoTimer = 0;
  return result; // possible undefined behavior: 'result' may be uninitialized
}

/* MOD (evolution): one Rookie target of a Baby II, scored as the original loop did; the best so far in *best/*bestScore */
static void scoreInTrainingTarget(int32_t currentDigimon, DigimonId target, int32_t *best, int8_t *bestScore)
{
  int8_t requirementScore;
  int8_t isMaxCM;
  int8_t isMaxBattles;

  if (target == -1 || target == 0 || !isPartnerModelFit(target)) /* MOD (scale): a model that does not fit is no target */
    return;
  isMaxCM = (EVO_REQ_ROW(target).flags & 0x10) >> 4;
  isMaxBattles = EVO_REQ_ROW(target).flags & 1;
  requirementScore = calculateRequirementScore(currentDigimon, target, isMaxCM, isMaxBattles, *best);
  if (*bestScore < requirementScore && requirementScore >= 3) {
    *best = target;
    *bestScore = requirementScore;
  }
}

int16_t getInTrainingEvolutionTarget(int32_t currentDigimon) {
  DigimonId *targetPtr; /* MOD: was int8_t */
  DigimonId *extra;
  int32_t currentBest;
  int32_t i;
  int32_t n;
  int8_t bestScore;

  currentBest = -1;
  bestScore = 0;
  targetPtr = getEvolutionPath(currentDigimon)->to; /* MOD (species) */
  for (i = 0; i < 6; i++) {
    scoreInTrainingTarget(currentDigimon, targetPtr[i], &currentBest, &bestScore);
  }
  /* MOD (evolution): the Rookies past the 6 slots (getEvolutionExtraTargets) */
  n = getEvolutionExtraTargets(currentDigimon, &extra);
  for (i = 0; i < n; i++) {
    scoreInTrainingTarget(currentDigimon, extra[i], &currentBest, &bestScore);
  }

  return currentBest;
}

int16_t getRookieEvolutionTarget(int32_t currentDigimon) {
  DigimonId *targetPtr; /* MOD: was int8_t */
  int32_t currentBestDigimon;
  int32_t i;
  int16_t bestScore;
  int16_t statCount;
  int16_t statTotal;
  int8_t isMaxCM;
  int8_t isMaxBattles;

  targetPtr = getEvolutionPath(currentDigimon)->to; /* MOD (species) */
  currentBestDigimon = -1;
  bestScore = statTotal = statCount = 0U;

  for (i = 0; i < 6; i++) {
    if (*targetPtr == -1 || !isPartnerModelFit(*targetPtr)) { /* MOD (scale): a model that does not fit is no target */
      targetPtr++;
      continue;
    }

    isMaxCM = (EVO_REQ_ROW(*targetPtr).flags & 0x10) >> 4;
    isMaxBattles = EVO_REQ_ROW(*targetPtr).flags & 1;
    if (calculateRequirementScore(currentDigimon, *targetPtr, isMaxCM,
                                  isMaxBattles, currentBestDigimon) >= 3) {
      if (EVO_REQ_ROW((int32_t)*targetPtr).hp != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.hp / 10;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).mp != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.mp / 10;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).offense != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.off;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).defense != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.def;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).speed != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.speed;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).brain != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.brain;
        statCount++;
      }

      statTotal /= statCount;
      if (bestScore < statTotal) {
        currentBestDigimon = *targetPtr;
        bestScore = statTotal;
        statTotal = statCount = 0;
      }
    }
    targetPtr++;
  }

  /* MOD (evolution rules as data): the default target (Numemon) and its wait, from content/tables/evolution-rules.json */
  if (currentBestDigimon == -1 && MOD_EVO_TIMING[MOD_EVO_FALLBACK_HOURS] >= 0 &&
      MOD_EVO_TIMING[MOD_EVO_FALLBACK_TARGET] >= 0 &&
      PARTNER_PARA.evoTimer >= MOD_EVO_TIMING[MOD_EVO_FALLBACK_HOURS])
    currentBestDigimon = MOD_EVO_TIMING[MOD_EVO_FALLBACK_TARGET];

  return currentBestDigimon;
}


int16_t getChampionEvolutionTarget(int32_t currentDigimon) {
  DigimonId *targetPtr; /* MOD: was int8_t */
  int32_t currentBestDigimon;
  int32_t i;
  int16_t bestScore;
  int16_t statCount;
  int16_t statTotal;
  int8_t isMaxCM;
  int8_t isMaxBattles;

  targetPtr = getEvolutionPath(currentDigimon)->to; /* MOD (species) */
  currentBestDigimon = -1;
  bestScore = statTotal = statCount = 0U;

  for (i = 0; i < 6; i++) {
    if (*targetPtr == -1 || !isPartnerModelFit(*targetPtr)) { /* MOD (scale): a model that does not fit is no target */
      targetPtr++;
      continue;
    }

    isMaxCM = (EVO_REQ_ROW(*targetPtr).flags & 0x10) >> 4;
    isMaxBattles = EVO_REQ_ROW(*targetPtr).flags & 1;
    if (calculateRequirementScore(currentDigimon, *targetPtr, isMaxCM,
                                  isMaxBattles, currentBestDigimon) >= 3) {
      if (EVO_REQ_ROW((int32_t)*targetPtr).hp != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.hp / 10;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).mp != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.mp / 10;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).offense != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.off;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).defense != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.def;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).speed != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.speed;
        statCount++;
      }
      if (EVO_REQ_ROW((int32_t)*targetPtr).brain != -1) {
        statTotal += PARTNER_ENTITY.digimonEntity.stats.base.brain;
        statCount++;
      }

      statTotal /= statCount;
      if (bestScore < statTotal) {
        currentBestDigimon = *targetPtr;
        bestScore = statTotal;
        statTotal = statCount = 0;
      }
    }
    targetPtr++;
  }

  return currentBestDigimon;
}

// clang-format off
int32_t handleSpecialEvolutions(mode, entity)
  int8_t mode;
  Entity *entity;
// clang-format on
{
  int32_t type;
  int16_t level;
  int16_t def;
  int16_t discipline;
  int16_t happiness;
  int16_t evotimer;
  int16_t battles;
  int16_t tiredness;
  int16_t rand;
  int16_t evoTarget;
  ModEvoRule *rule;
  int32_t i;
  int32_t k;
  int32_t stat;

  if (tamerGetState() != 0)
    return -1;

  type = entity->type;
  level = DIGI_PARA(type).level;
  def = PARTNER_ENTITY.digimonEntity.stats.base.def;
  discipline = PARTNER_PARA.discipline;
  happiness = PARTNER_PARA.happiness;
  evotimer = PARTNER_PARA.evoTimer;
  battles = PARTNER_PARA.battles;
  tiredness = PARTNER_PARA.tiredness;
  rand = randomLimit(100);
  evoTarget = -1;

  /* MOD (evolution rules as data): the rows of MOD_EVO_RULES, in order; the last that matches wins. One roll for all of them */
  for (i = 0; i < MOD_EVO_RULES_MAX; i++) {
    rule = &MOD_EVO_RULES[i];
    if (rule->target < 0 || rule->mode != mode || rand >= rule->chance)
      continue;
    if (rule->level >= 0 && level != rule->level)
      continue;
    if (rule->from[0] >= 0) {
      for (k = 0; k < 4 && rule->from[k] >= 0 && rule->from[k] != type; k++)
        ;
      if (k == 4 || rule->from[k] != type)
        continue;
    }
    for (k = 0; k < MOD_EVO_COND_MAX && rule->cond[k].stat >= 0; k++) {
      switch (rule->cond[k].stat) {
      case MOD_STAT_DISCIPLINE: stat = discipline; break;
      case MOD_STAT_HAPPINESS: stat = happiness; break;
      case MOD_STAT_TIREDNESS: stat = tiredness; break;
      case MOD_STAT_BATTLES: stat = battles; break;
      case MOD_STAT_DEFENSE: stat = def; break;
      case MOD_STAT_EVOTIMER: stat = evotimer; break;
      case MOD_STAT_SCREEN: stat = CURRENT_SCREEN; break;
      case MOD_STAT_SCOLD_FLAG: stat = NANIMON_TRIGGER; break;
      default: stat = 0; break;
      }
      if (rule->cond[k].op == 1 ? stat < rule->cond[k].value : stat != rule->cond[k].value)
        break;
    }
    if (k < MOD_EVO_COND_MAX && rule->cond[k].stat >= 0)
      continue;
    if (rule->flags & 1)
      NANIMON_TRIGGER = 0;
    evoTarget = rule->target;
  }

  if (evoTarget != -1) {
    EVOLUTION_TARGET = evoTarget;
    tamerSetState(6);
    partnerSetState(0xd);
  }

  return evoTarget;
}

int8_t calculateRequirementScore(int32_t current, DigimonId target,
                                 int8_t isMaxCM, int8_t isMaxBattles,
                                 DigimonId currentBest) {
  EvoRequirements *reqs;
  BaseStats *partnerEntity;
  PartnerPara *partner;
  int32_t isTargetRaised;
  int32_t isCurrentBestRaised;
  int16_t statsArray[6];
  int16_t statsArray2[6];
  int16_t reqArray[6];
  int32_t i;
  int32_t j;
  int32_t isHighestStat;
  int32_t highestStat;
  int8_t reqPoints;
  int8_t numMoves;
  int8_t isBonusFulfilled;

  reqs = &EVO_REQ_ROW(target);
  partnerEntity = &PARTNER_ENTITY.digimonEntity.stats.base;
  partner = &PARTNER_PARA;
  reqPoints = 0;

  if (isMaxCM == 0) {
    if (partner->careMistakes >= reqs->care)
      reqPoints += 1;
  } else if (partner->careMistakes <= reqs->care) {
    reqPoints += 1;
  }

  if (reqs->weight - 5 <= partner->weight &&
      partner->weight <= reqs->weight + 5)
    reqPoints += 1;

  if (DIGI_PARA(target).level == 3U) {
    statsArray[0] = statsArray2[0] = partnerEntity->hp / 10;
    statsArray[1] = statsArray2[1] = partnerEntity->mp / 10;
    statsArray[2] = statsArray2[2] = partnerEntity->off;
    statsArray[3] = statsArray2[3] = partnerEntity->def;
    statsArray[4] = statsArray2[4] = partnerEntity->speed;
    statsArray[5] = statsArray2[5] = partnerEntity->brain;
    reqArray[0] = reqs->hp;
    reqArray[1] = reqs->mp;
    reqArray[2] = reqs->offense;
    reqArray[3] = reqs->defense;
    reqArray[4] = reqs->speed;
    reqArray[5] = reqs->brain;

    for (i = 0; i < 6; i++) {
      isHighestStat = 1;
      for (j = 0; j < 6; j++) {
        if (statsArray[i] < statsArray[j])
          isHighestStat = 0;
      }
      if (isHighestStat == 1)
        highestStat = i;
    }
    if (reqArray[highestStat] == 1) {
      reqPoints += 1;
    }
  } else {
    if (partnerEntity->hp / 10 >= reqs->hp &&
        partnerEntity->mp / 10 >= reqs->mp &&
        partnerEntity->off >= reqs->offense &&
        partnerEntity->def >= reqs->defense &&
        partnerEntity->speed >= reqs->speed &&
        partnerEntity->brain >= reqs->brain) {
      reqPoints += 1;
    }
  }

  isBonusFulfilled = 0;
  if (reqs->digimon != -1 && current == reqs->digimon)
    isBonusFulfilled = 1;
  if (reqs->discipline != -1 && partner->discipline >= reqs->discipline)
    isBonusFulfilled = 1;
  if (reqs->happiness != -1 && partner->happiness >= reqs->happiness)
    isBonusFulfilled = 1;
  if (reqs->battles != -1) {
    if (isMaxBattles == 0) {
      if (partner->battles >= reqs->battles)
        isBonusFulfilled = 1;
    } else if (partner->battles <= reqs->battles) {
      isBonusFulfilled = 1;
    }
  }
  if (reqs->techs != -1) {
    numMoves = getNumMasteredMoves();
    if (numMoves >= reqs->techs)
      isBonusFulfilled = 1;
  }

  reqPoints += isBonusFulfilled;

  if (reqPoints >= 3 && currentBest != -1) {
    isTargetRaised = hasDigimonRaised(EVO_GAINS_ROW(target).targetDigimon);
    isCurrentBestRaised =
        hasDigimonRaised(EVO_GAINS_ROW(currentBest).targetDigimon);

    if (isTargetRaised == 1 && isCurrentBestRaised == 0)
      reqPoints = 0;

    if (isTargetRaised == 0 && isCurrentBestRaised == 1)
      reqPoints++;
  }

  return reqPoints;
}

int8_t getNumMasteredMoves(void) {
  int8_t moveCount = 0;
  int32_t i;
  int32_t j;

  for (i = 0; i < 2; i++) {
    for (j = 0; j < 32; j++) {
      if (PARTNER_ENTITY.learnedMoves[i] & 1 << j)
        moveCount++;
    }
  }

  return moveCount;
}

// clang-format off
void reincarnatePartner(unused, stats, partner, digimonId)
  int32_t unused;
  Stats *stats;
  PartnerPara *partner;
  int16_t digimonId;
// clang-format on
{
  EvoStatsGains *ptr;
  int32_t id;
  int32_t previousId;

  DEATH_STATS = *stats;
  ptr = &EVO_GAINS_ROW(digimonId);
  id = ptr->targetDigimon;

  PARTNER_ENTITY.digimonEntity.stats.base.moves[0] = 0x2e;
  PARTNER_ENTITY.digimonEntity.stats.base.moves[1] = 0xff;
  PARTNER_ENTITY.digimonEntity.stats.base.moves[2] = 0xff;
  PARTNER_ENTITY.digimonEntity.stats.base.moves[3] = 0xff;
  previousId = PARTNER_ENTITY.digimonEntity.entity.type;
  removeEntity(PARTNER_ENTITY.digimonEntity.entity.type, 1);
  ENTITY_TABLE[1] = NULL;
  thunkUnloadModel(previousId, 3);
  initializeReincarnatedPartner(id, 0, 0, 0, 0, 0, 0);
  PARTNER_ENTITY.lives = 3;
  setDigimonRaised(id);
}

/*
 * MOD (machinedramon): the Ultimate evolution check. In the original an
 * Ultimate never evolves (tickConditions only checks levels 1-4). Megadramon
 * and Gigadramon, from 48 hours after their own evolution, evolve into
 * Machinedramon when its requirements (EVOLUTION_REQS) score 3 points, like
 * a Champion's targets. Returns -1 otherwise (and for every other type).
 */
int16_t getModUltimateEvolutionTarget(int32_t currentDigimon)
{
	EvoRequirements *reqs;
	int32_t i;
	int32_t target;

	/* MOD (evolution rules as data): the {from, to} pairs of MOD_EVO_ULTIMATE (Megadramon/Gigadramon -> Machinedramon) */
	for (i = 0; i < MOD_EVO_PAIR_MAX; i++) {
		target = MOD_EVO_ULTIMATE[i][1];
		if (MOD_EVO_ULTIMATE[i][0] != currentDigimon || target < 0) {
			continue;
		}
		reqs = &EVO_REQ_ROW(target);
		if (calculateRequirementScore(currentDigimon, target, (reqs->flags & 0x10) >> 4, reqs->flags & 1, -1) >= 3) {
			return target;
		}
	}
	return -1;
}
