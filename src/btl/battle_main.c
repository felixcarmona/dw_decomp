#include <inline_n.h>
#include <libetc.h>
#include <libgpu.h>
#include <libgte.h>

#include <dw/mod_battle.h>
#include <dw/mod_options.h>
#include <dw/caps.h> /* MOD (caps) */
#include <dw/attack_object.h>
#include <dw/btl.h>
#include <dw/combat.h>
#include <dw/ids.h>
#include <dw/pstat.h> /* MOD (timed battle) */
#include <dw/script.h>
#include <dw/item.h>
#include <dw/math.h>
#include <dw/mod_present.h>
#include <dw/mod_qol.h>
#include <dw/move.h>
#include <dw/species.h>
#include <dw/params.h>
#include <dw/script.h>
#include <dw/sound.h>
#include <dw/sound_async.h>
#include <dw/swap.h>
#include <dw/version.h>
#include <dw/mod_rules.h>

#include "common.h"

typedef struct {
	int16_t flags[4];
	int16_t enemies[5];
	int16_t count;
} TargetChoice;

extern int8_t PARTNER_PREVIOUS_TILE_X;
extern int8_t PARTNER_PREVIOUS_TILE_Y;
extern uint8_t TYPE_FACTORS[][7];
extern int32_t COMBAT_AREA_X;
extern int32_t COMBAT_AREA_Y;
extern int32_t ACTIVE_FRAMEBUFFER;
extern GsOT GS_ORDERING_TABLE[];
extern PACKET GS_WORK_BASES[2][0x14000];
extern DR_OFFSET DR_OFFSETS[2];
extern DigimonEntity *BATTLE_TARGETED_DIGIMON;
extern DigimonEntity *BATTLE_ATTACKING_DIGIMON;
extern int8_t PARTNER_WAYPOINT_COUNT;
extern int8_t PARTNER_WAYPOINT_CURRENT;
extern int8_t PARTNER_WAYPOINT_X[];
extern int8_t PARTNER_WAYPOINT_Y[];
extern uint8_t BTL_COMMAND_ICON_UVS[8][2];
extern uint8_t BTL_SPECIAL_ICON_UVS[8][2];

void removeObject(int32_t objectId, int32_t instanceId);
void addObject(int32_t objectId, int32_t instanceId, void *tick, void *render);
void getEntityTile(Entity *entity, int8_t *outTileX, int8_t *outTileY);
void startBattleIdleAnimation(DigimonEntity *entity, Stats *stats, int32_t flags);
void tickObjects(void);
void renderObjects(void);
void handlePause(void);
void removePauseBox(void);
void BTL_tickPartnerAI(void);
void BTL_tickEnemyAI(void);
void BTL_tickFighterStates(void);
void BTL_tickPartnerWaypointTrail(void);
void BTL_increaseSpeedBuffer(FighterData *fighter, Stats *stats);
int32_t isOffScreen(DVECTOR *xy, int16_t w, int16_t h);
void BTL_tickStatusEffects(void);
int32_t BTL_startEFE(int32_t arg0);
int32_t BTL_applyBuffMove(DigimonEntity *digimon, int32_t slot, int16_t anim);
int32_t getMapSoundId(int32_t mapId);
void handleBattleEndBox(void);
void drawLine2P(int32_t color, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t order, uint32_t mode);
void drawLine3P(int32_t color, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t otz, int32_t flag);
void damageTick(FighterData *fighter, Stats *stats);
void drawEntityText(int32_t a, int32_t b, int32_t x, int32_t y, int32_t value, int32_t order);
int32_t BTL_calculateDamage(DigimonEntity *attacker, DigimonEntity *defender, int16_t move);
void BTL_deinitializeCombat(void);
void closeInventoryBoxes(void);
void resetStatsAfterCombat(void);
void removeEntityText(int32_t id);
void BTL_selectEnemyMove(DigimonEntity *digimon, FighterData *fighter, int16_t index);
void BTL_selectPartnerMove(DigimonEntity *digimon, FighterData *fighter, int16_t index);
void BTL_tickChargeState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter);
void BTL_tickAttackHits(void);
void BTL_tickConfusedState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int32_t arg3);
void BTL_initializeFinisherChargeup(int16_t tech);
int32_t BTL_addFinisherAura(Entity *entity, int32_t arg1);
void BTL_setupMoveExecution(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter);
int16_t BTL_calculateHitChance(DigimonEntity *attacker, DigimonEntity *defender, FighterData *fighter, int16_t move);
void BTL_applyMoveStatus(DigimonEntity *digimon, FighterData *fighter, int32_t move);
int32_t BTL_battleMain(void);
void BTL_getWeakestEnemy(Entity *self, FighterData *fighter, int16_t *outScore, int16_t *outIndex);
int16_t BTL_calculateTargetScore(DigimonEntity *self, DigimonEntity *other);
void BTL_addBuffRingsEffect(int16_t kind, Entity *entity);
void BTL_healStatusEffect(int32_t arg0);
int16_t BTL_getNearestEnemy(Entity *self, int16_t *flags);
void BTL_tickAttackState(Entity *entity, DigimonEntity *target, int32_t id);
void handleBattleIdle(DigimonEntity *digimon, Stats *stats, int32_t flags);
void createParticleFX(uint8_t kind, int32_t count, SVECTOR *pos, Entity *entity, int32_t lifetime);
void BTL_resetFlatten(int16_t index);
void BTL_handleVictorySequence(void);
int32_t BTL_addPoisonEffect(Entity *entity);
int32_t BTL_addConfusionEffect(Entity *entity);
int32_t BTL_addStunEffect(Entity *entity, int32_t val);
void BTL_removePoisonEffect(int32_t id, Entity *entity);
void BTL_removeConfusionEffect(int32_t id, Entity *entity);
void BTL_removeStunEffect(int32_t id, Entity *entity);
uint32_t BTL_getMoveWithHighestDistance(DigimonEntity *digimon);
void BTL_getRemainingEnemies(Entity *self, int16_t *out, int16_t *count);
void BTL_findUnblockedRotation(Entity *entity, int16_t *rot, int16_t hit, int16_t orig);
void collisionGrace(Entity *a, Entity *entity, int32_t c, int32_t d);
void BTL_moveTowardLocation(DigimonEntity *digimon, VECTOR *target, int16_t dx, int16_t dy);
void BTL_selectFighterTarget(DigimonEntity *digimon, FighterData *fighter, uint8_t target, int32_t arg3);
void BTL_clearConfusion(DigimonEntity *digimon, FighterData *fighter);
void BTL_faintDigimon(DigimonEntity *digimon, FighterData *fighter, int16_t arg2);
void BTL_addStunStatusVisual(DigimonEntity *digimon, FighterData *fighter);
int32_t BTL_handlePartnerMoveCommand(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter);
void BTL_tickRangedAttack(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int16_t move);
int32_t BTL_tickMeleeAttack(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int16_t arg3);
void BTL_startQueuedMove(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter);
void setPartnerWaypoint(int16_t index, int8_t x, int8_t y);
void initializePartnerWaypoint(void);
void BTL_removeFinisherAura(int32_t arg0);
int32_t isFiveTileWidePathBlocked(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void BTL_addAuraProjectile(Entity *entity);
void BTL_handleHitReaction(Entity *entity, FighterData *fighter, AttackObject *attack, int16_t index);
void BTL_retargetAfterHit(DigimonEntity *digimon, FighterData *fighter, AttackObject attack);
void BTL_applyFlattenScale(VECTOR *scale, int32_t t);
void BTL_tickQueuedMove(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int32_t arg3);
void BTL_tickFlatState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int32_t arg3);
void BTL_confusedRotate(Entity *entity);
void BTL_startHitAnimation(Entity *entity, AttackObject *attack, uint8_t animId);
void BTL_removeStatusEffects(DigimonEntity *digimon, FighterData *fighter);
void BTL_applyChargeRequirement(DigimonEntity *digimon, FighterData *fighter, int16_t tech);
int32_t BTL_addBlockedAttack(FighterData *fighter, FighterData *other);
void BTL_setupQueuedMove(DigimonEntity *digimon, FighterData *fighter, int16_t arg2, uint8_t moveIndex);
void BTL_tickFighterAction(int32_t arg0);
int16_t BTL_getStrongestMove(int32_t arg0, int16_t *flags);
int16_t BTL_getMostEffectiveMove(int32_t index, int16_t *flags);
int16_t BTL_getCheapestMove(int32_t arg0, int16_t *flags);
void BTL_tickHitState(Entity *entity, FighterData *fighter, int32_t arg2);
int32_t BTL_selectMoveByPower(int32_t arg0, int16_t *flags);
int32_t BTL_selectMoveByMpCost(int32_t arg0, int16_t *flags);
void addEntityText(Entity *entity, long slot, int32_t color, int32_t value, uint8_t flag);
void addWithLimit(/* int16_t *value, int16_t amount, int16_t limit */);
void BTL_buffStats(DigimonEntity *digimon, int32_t slot, int32_t value, int16_t *stat, int32_t color, int32_t flag);
int16_t BTL_getRandomUsableMove(int16_t *flags);
int32_t entityGetTechFromAnim(Entity *entity, int32_t anim);
int32_t BTL_getContactRangeSquared(int32_t *a, int32_t *b);
void BTL_maintainTargetDistance(DigimonEntity *attacker, DigimonEntity *target, FighterData *fighter);
void BTL_maintainDistanceRange(DigimonEntity *attacker, DigimonEntity *target, FighterData *fighter, uint32_t arg3, uint32_t arg4);
int16_t BTL_calculateElementBonus(int16_t arg0, int16_t arg1);
void BTL_applySquashScale(VECTOR *scale, int32_t angle);
void BTL_healFlatten(Entity *entity, FighterData *fighter, int32_t arg2);
void BTL_addConfusionStatusVisual(DigimonEntity *digimon, FighterData *fighter);
int32_t BTL_isMoveOnCooldown(Entity *entity, FighterData *fighter);
void BTL_calculateScoreRanks(int32_t *values, int32_t *groups, int32_t count);
void BTL_stopEFESubEffect(int32_t a, int32_t b);
void startAnimation(Entity *entity, uint8_t animId);
void BTL_removeStatusEffectVisual(DigimonEntity *digimon, FighterData *fighter, uint8_t kind);
uint8_t BTL_isFighterDefeated(uint8_t index);
void BTL_tickSenileState(Entity *entity, FighterData *fighter);
void BTL_removeMoveEffect(DigimonEntity *digimon, FighterData *fighter);
void BTL_applyStretchScale(VECTOR *scale, int32_t angle);
void BTL_clearPoison(DigimonEntity *digimon, FighterData *fighter);
int16_t BTL_getNpcEntityIndex(Entity *entity);
int32_t BTL_countLivingEnemies(void);
void BTL_tickStunState(Entity *entity);
void BTL_clearBlockedAttacks(FighterData *fighter);
int16_t BTL_getFighterSlot(int16_t entityId);
int32_t BTL_getDistanceSquared(Entity *a, Entity *b);
void BTL_addStatusEffectVisual(DigimonEntity *digimon, FighterData *fighter, uint8_t kind);
void BTL_addPoisonStatusVisual(DigimonEntity *digimon, FighterData *fighter);
void BTL_removeEnemyHPBars(void);
void BTL_setWalking(Entity *entity, Stats *stats, uint16_t flags);
void BTL_startWalkingAnimation(Entity *entity, Stats *stats, uint16_t flags);
void BTL_renderEnemyHPBars(void);
void BTL_resetFighterAction(FighterData *fighter);
void BTL_setFighterCooldown(DigimonEntity *digimon, FighterData *fighter);
int32_t BTL_getUsableMoves(int16_t *out, int16_t index);
int16_t BTL_getAttackTech(AttackObject *attack);
void BTL_addFinisherProgress(FighterData *fighter, int16_t amount);
void BTL_getStatusAfflictedEnemies(Entity *self, int16_t *out, int16_t *count);
void BTL_sortScoresDescending(int32_t *values, int32_t *keys, int32_t *groups, int32_t count);
void BTL_sortScoresAscending(int32_t *values, int32_t *keys, int32_t *groups, int32_t count);
void BTL_removeCombatObjects(void);
int32_t BTL_hasAffordableMoves(int16_t *out, int16_t index);
int32_t BTL_isMoveUsable(DigimonEntity *digimon, FighterData *fighter, int16_t slot);
void BTL_tickCooldownState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter);
void BTL_clearStun(DigimonEntity *digimon, FighterData *fighter);
void BTL_backAwayFromTarget(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter);
void removeBuffModelObject(void);
void BTL_getHighestScoredMove(int16_t *values, int16_t *marks, int16_t *out, int16_t count);
void BTL_getLowestScoredMove(int16_t *values, int16_t *marks, int16_t *out, int16_t count);
void BTL_selectConfusedMove(DigimonEntity *digimon, FighterData *fighter, long index);

static void *battle_main_functions[] = {
	BTL_setCommandIconUV,
	BTL_selectPartnerMove,
	BTL_selectEnemyMove,
	BTL_getNearestEnemy,
	BTL_isFighterDefeated,
	BTL_selectFighterTarget,
	BTL_selectMoveByMpCost,
	BTL_selectMoveByPower,
	BTL_getRemainingEnemies,
	BTL_selectConfusedMove,
	BTL_calculateScoreRanks,
	BTL_countLivingEnemies,
	BTL_calculateElementBonus,
	BTL_sortScoresAscending,
	BTL_sortScoresDescending,
	BTL_calculateTargetScore,
	BTL_getWeakestEnemy,
	BTL_getStatusAfflictedEnemies,
	BTL_getNpcEntityIndex,
	BTL_getLowestScoredMove,
	BTL_getHighestScoredMove,
	BTL_getCheapestMove,
	BTL_getMostEffectiveMove,
	BTL_getStrongestMove,
	BTL_getRandomUsableMove,
	BTL_setFighterCooldown,
	BTL_getUsableMoves,
	BTL_healStatusEffect,
	BTL_removeStatusEffects,
	BTL_addStunStatusVisual,
	BTL_addConfusionStatusVisual,
	BTL_addPoisonStatusVisual,
	BTL_resetFighterAction,
	BTL_tickStatusEffects,
	BTL_healFlatten,
	BTL_clearPoison,
	BTL_removeStatusEffectVisual,
	BTL_applySquashScale,
	BTL_applyStretchScale,
	BTL_addStatusEffectVisual,
	BTL_applyFlattenScale,
	BTL_clearStun,
	BTL_clearConfusion,
	BTL_addFinisherProgress,
	BTL_battleTickFrame,
	BTL_removeMoveEffect,
	BTL_setupMoveExecution,
	BTL_isMoveOnCooldown,
	BTL_startQueuedMove,
	BTL_applyChargeRequirement,
	BTL_setupQueuedMove,
	BTL_getDistanceSquared,
	BTL_isMoveUsable,
	BTL_renderEnemyHPBars,
	BTL_buffStats,
	BTL_tickAttackHits,
	BTL_startHitAnimation,
	BTL_addBlockedAttack,
	BTL_getFighterSlot,
	BTL_applyMoveStatus,
	BTL_handleHitReaction,
	BTL_calculateDamage,
	BTL_retargetAfterHit,
	BTL_calculateHitChance,
	BTL_applyBuffMove,
	BTL_getAttackTech,
	BTL_battleMain,
	BTL_findUnblockedRotation,
	BTL_clearBlockedAttacks,
	BTL_addEnemyHPBars,
	BTL_initializeEnemyHPBarSprites,
	BTL_initializePartnerTile,
	BTL_startWalkingAnimation,
	BTL_hasAffordableMoves,
	BTL_faintDigimon,
	BTL_increaseSpeedBuffer,
	BTL_getContactRangeSquared,
	BTL_maintainDistanceRange,
	BTL_maintainTargetDistance,
	BTL_confusedRotate,
	BTL_tickFighterAction,
	BTL_backAwayFromTarget,
	BTL_getMoveWithHighestDistance,
	BTL_moveTowardLocation,
	BTL_setWalking,
	BTL_tickRangedAttack,
	BTL_tickMeleeAttack,
	BTL_handlePartnerMoveCommand,
	BTL_tickQueuedMove,
	BTL_tickCooldownState,
	BTL_tickChargeState,
	BTL_tickSenileState,
	BTL_tickConfusedState,
	BTL_tickStunState,
	BTL_tickFlatState,
	BTL_tickHitState,
	BTL_tickAttackState,
	BTL_handleVictorySequence,
	BTL_resetFlatten,
	BTL_removeEnemyHPBars,
	BTL_removeCombatObjects,
	BTL_deinitializeCombat,
	BTL_tickPartnerWaypointTrail,
	BTL_tickFighterStates,
	BTL_tickEnemyAI,
	BTL_tickPartnerAI,
};

uint8_t TARGET_ATTACKER_CHANCE[4] = { 70, 40, 30, 0 };
/* MOD (battle as data): the three "Your call" priority rows (power, MP, wide) are MOD_BT_PRIO_POWER0.. / _MP0.. / _WIDE0.. */
int8_t BTL_COMMAND_MENU_CLOSED_FRAMES = -1;

void BTL_tickPartnerAI(void)
{
	int32_t i;
	int32_t result;
	int16_t enemies[4];
	int16_t moveFlags[4];
	Stats *stats;
	int16_t targetFlags[4];
	int16_t count;
	FighterData *fighter;
	uint16_t *flagsPtr;
	Entity *partner;

	partner = ENTITY_TABLE[1];
	stats = &((DigimonEntity *)partner)->stats;
	fighter = COMBAT_DATA_PTR->fighter;
	flagsPtr = &fighter->flags;
	if (COMBAT_DATA_PTR->player.commandDelay[0] == 0) {
		COMBAT_DATA_PTR->player.currentCommand[0] = COMBAT_DATA_PTR->player.bufferedCommand[0];
	} else if (!(*flagsPtr & (FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED | FIGHTER_FLAG_DEAD)) && (fighter->flatTimer == 0)) {
		COMBAT_DATA_PTR->player.commandDelay[0]--;
	}

	if (NO_AI_FLAG == 0) {
		if (stats->current.currentHP > fighter->hpDamageBuffer) {
			if (stats->base.brain < BM(SEN_BRAIN)) {
				if ((BATTLE_FRAME_COUNT % ((BMD(stats->base.brain, SEN_INT_DIV) + 1) * BMS(SEN_INT_MULT))) == 0) {
					if ((BM(SEN_PARTNER) - PARTNER_PARA.discipline) > randomLimit(100)) {
						*flagsPtr |= FIGHTER_FLAG_SENILE;
						fighter->senileTimer = BM(SEN_FRAMES);
					}
				}
			}
		}
		if (fighter->cooldown >= 2) {
			fighter->cooldown--;
		}
		if (!(*flagsPtr & FIGHTER_FLAG_SENILE)) {
			BTL_increaseSpeedBuffer(fighter, stats);
		}
	}

	if (*flagsPtr & (FIGHTER_FLAG_KNOCKED_BACK | FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_BLOCKING | FIGHTER_FLAG_DEAD)) {
		return;
	}

	if (stats->current.currentHP == 0) {
		BTL_faintDigimon((DigimonEntity *)partner, fighter, 0);
		return;
	}

	BTL_getRemainingEnemies(partner, enemies, &count);
	if (count == 0) {
		handleBattleIdle((DigimonEntity *)partner, stats, *flagsPtr);
		fighter->moveRange = -1;
		BTL_resetFlatten(0);
		BTL_removeStatusEffects((DigimonEntity *)partner, fighter);
		*flagsPtr = 0;
		*flagsPtr |= FIGHTER_FLAG_TRANSFORMING;
		return;
	}

	if (NO_AI_FLAG != 0) {
		return;
	}

	if (!(*flagsPtr & (FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED | FIGHTER_FLAG_DEAD)) && (fighter->flatTimer == 0)) {
		switch (COMBAT_DATA_PTR->player.currentCommand[0]) {
		case 7:
			if (fighter->targetId == 0xff) {
				break;
			}
			if (COMBAT_DATA_PTR->player.changeTarget != 1) {
				break;
			}
			switch (count) {
			case 2:
				if (fighter->targetId == enemies[0]) {
					fighter->targetId = enemies[1];
				} else {
					fighter->targetId = enemies[0];
				}
				break;
			case 3:
				if ((COMBAT_DATA_PTR->player.unk5[0] == 0xff) && (COMBAT_DATA_PTR->player.unk5[1] == 0xff)) {
					for (i = 1; i < 4; i++) {
						targetFlags[i] = 1;
					}
					fighter->targetId = BTL_getNearestEnemy(partner, targetFlags);
					COMBAT_DATA_PTR->player.unk5[COMBAT_DATA_PTR->player.unk7] = fighter->targetId;
					COMBAT_DATA_PTR->player.unk7 = (COMBAT_DATA_PTR->player.unk7 + 1) & 1;
				} else {
					for (i = 1; i < 4; i++) {
						if ((fighter->targetId != i) && (COMBAT_DATA_PTR->player.unk5[(COMBAT_DATA_PTR->player.unk7 + 1) & 1] != i)) {
							break;
						}
					}
					COMBAT_DATA_PTR->player.unk5[COMBAT_DATA_PTR->player.unk7] = fighter->targetId;
					COMBAT_DATA_PTR->player.unk7 = (COMBAT_DATA_PTR->player.unk7 + 1) & 1;
					fighter->targetId = i;
				}
				break;
			}
			COMBAT_DATA_PTR->player.changeTarget = 0;
			return;
		case 8:
		case 9:
		case 10:
			if (BTL_isMoveUsable((DigimonEntity *)partner, fighter, COMBAT_DATA_PTR->player.currentCommand[0] - 8) == 0) {
				break;
			}
			if ((fighter->targetId != 0xff) && (BTL_isFighterDefeated(fighter->targetId) != 0)) {
				BTL_selectFighterTarget((DigimonEntity *)partner, fighter, fighter->targetId, 0);
			}
			if ((((DigimonEntity *)partner)->stats.base.moves[COMBAT_DATA_PTR->player.currentCommand[0] - 8] != fighter->queuedAnim) || (fighter->moveRange <= 0)) {
				BTL_setupQueuedMove((DigimonEntity *)partner, fighter, 0, COMBAT_DATA_PTR->player.currentCommand[0] - 8);
			}
			BTL_applyChargeRequirement((DigimonEntity *)partner, fighter, DIGI_PARA(partner->type).moves[fighter->queuedAnim - 0x2e]);
			return;
		case 11:
			if ((fighter->targetId != 0) && (BTL_isFighterDefeated(fighter->targetId) != 0)) {
				BTL_selectFighterTarget((DigimonEntity *)partner, fighter, fighter->targetId, 0);
			}
			if ((((DigimonEntity *)partner)->stats.base.moves[3] != fighter->queuedAnim) || (fighter->moveRange <= 0)) {
				BTL_setupQueuedMove((DigimonEntity *)partner, fighter, 0, 3);
			}
			return;
		}
		if ((COMBAT_DATA_PTR->player.currentCommand[0] != 2) && (COMBAT_DATA_PTR->player.currentCommand[0] != 4)) {
			((DigimonEntity *)partner)->stats.current.chargeMode = BTL_SAVED_CHARGE_MODE;
		}
	}

	if ((*flagsPtr & FIGHTER_FLAG_FLATTENED) && BMEVERY(BATTLE_FRAME_COUNT, T_FLAT_RETARGET)) {
		fighter->targetId = enemies[randomLimit(count)];
	}

	if (*flagsPtr & FIGHTER_FLAG_TRANSFORMING) {
		return;
	}

	if (COMBAT_DATA_PTR->player.currentCommand[0] == 1) {
		fighter->moveRange = 1;
		*flagsPtr |= FIGHTER_FLAG_TRANSFORMING;
	}

	if (*flagsPtr & FIGHTER_FLAG_FLATTENED) {
		fighter->queuedAnim = 0;
		BTL_selectFighterTarget((DigimonEntity *)partner, fighter, fighter->targetId, 0);
		fighter->moveRange = 2;
		partner->flatSprite = 0;
		fighter->flags |= FIGHTER_FLAG_TRANSFORMING;
		return;
	}

	if (*flagsPtr & FIGHTER_FLAG_STUNNED) {
		return;
	}

	if (*flagsPtr & FIGHTER_FLAG_CONFUSED) {
		BTL_selectConfusedMove((DigimonEntity *)partner, fighter, 0);
		return;
	}

	if (*flagsPtr & FIGHTER_FLAG_ON_CHARGEUP) {
		return;
	}

	if (*flagsPtr & FIGHTER_FLAG_ON_COOLDOWN) {
		return;
	}

	if (*flagsPtr & FIGHTER_FLAG_SENILE) {
		return;
	}

	if (!(*flagsPtr & FIGHTER_FLAG_10)) {
		BTL_selectFighterTarget((DigimonEntity *)partner, fighter, fighter->targetId, 0);
	}

	result = -1;
	switch (COMBAT_DATA_PTR->player.currentCommand[0]) {
	case 2:
		if (BTL_hasAffordableMoves(moveFlags, 0) == 0) {
			fighter->cooldown = BM(CD_IDLE);
			fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
			return;
		}
		result = BTL_selectMoveByPower(0, moveFlags);
		((DigimonEntity *)partner)->stats.current.chargeMode = 0;
		break;
	case 4:
		if (BTL_hasAffordableMoves(moveFlags, 0) == 0) {
			fighter->cooldown = BM(CD_IDLE);
			fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
			return;
		}
		result = BTL_selectMoveByMpCost(0, moveFlags);
		((DigimonEntity *)partner)->stats.current.chargeMode = 2;
		break;
	}

	if (result == -1) {
		BTL_selectPartnerMove((DigimonEntity *)partner, fighter, 0);
	} else {
		BTL_setupQueuedMove((DigimonEntity *)partner, fighter, 0, result);
	}
}

void BTL_tickEnemyAI(void)
{
	long i;
	DVECTOR out;
	CombatData *combat;
	Entity *entity;
	Entity *partner;
	SVECTOR pos;
	MATRIX *m;
	int16_t anim;
	FighterData *fighter;
	uint16_t *flagsPtr;
	Stats *stats;

	combat = COMBAT_DATA_PTR;
	partner = ENTITY_TABLE[1];
	if (BMEVERY(BATTLE_FRAME_COUNT, T_PRIO_STEP)) {
		for (i = 0; i <= ENEMY_COUNT; i++) {
			if (combat->fighter[i].buffPrioTimer != 0) {
				combat->fighter[i].buffPrioTimer--;
			}
		}
	}

	for (i = 1; i <= ENEMY_COUNT; i++) {
		fighter = &combat->fighter[i];
		entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		stats = &((DigimonEntity *)entity)->stats;
		flagsPtr = &fighter->flags;
		if (NO_AI_FLAG == 0) {
			if (stats->current.currentHP > fighter->hpDamageBuffer) {
				if (stats->base.brain < BM(SEN_BRAIN)) {
					if ((BATTLE_FRAME_COUNT % ((BMD(stats->base.brain, SEN_INT_DIV) + 1) * BMS(SEN_INT_MULT))) == 0) {
						if (BMD(BM(SEN_ENEMY) - stats->base.brain, SEN_ENEMY_DIV) > randomLimit(100)) {
							*flagsPtr |= FIGHTER_FLAG_SENILE;
							fighter->senileTimer = BM(SEN_FRAMES);
						}
					}
				}
			}
			if (!(*flagsPtr & FIGHTER_FLAG_SENILE)) {
				BTL_increaseSpeedBuffer(fighter, stats);
			}
			if (fighter->cooldown >= 2) {
				fighter->cooldown--;
			}
		}
		if (*flagsPtr & (FIGHTER_FLAG_KNOCKED_BACK | FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_BLOCKING | FIGHTER_FLAG_DEAD)) {
			continue;
		}
		if (stats->current.currentHP == 0) {
			BTL_faintDigimon((DigimonEntity *)entity, fighter, i);
			continue;
		}
		if (stats->current.currentHP <= fighter->hpDamageBuffer) {
			handleBattleIdle((DigimonEntity *)entity, stats, *flagsPtr);
			fighter->moveRange = -1;
			BTL_resetFlatten(i);
			BTL_removeStatusEffects((DigimonEntity *)entity, fighter);
			fighter->flags &= ~(FIGHTER_FLAG_POISONED | FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED);
			continue;
		}
		if (((DigimonEntity *)partner)->stats.current.currentHP == 0) {
			handleBattleIdle((DigimonEntity *)entity, stats, *flagsPtr);
			BTL_resetFlatten(i);
			*flagsPtr &= ~(FIGHTER_FLAG_KNOCKED_BACK | FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_BLOCKING);
			*flagsPtr |= FIGHTER_FLAG_TRANSFORMING;
			fighter->moveRange = -1;
			continue;
		}
		if (NO_AI_FLAG != 0) {
			return;
		}
		if (*flagsPtr & FIGHTER_FLAG_TRANSFORMING) {
			continue;
		}
		if (*flagsPtr & FIGHTER_FLAG_FLATTENED) {
			fighter->queuedAnim = 0;
			fighter->targetId = 0;
			fighter->moveRange = 2;
			startAnimation(entity, 0x23);
			entity->flatSprite = 0;
			fighter->flags |= FIGHTER_FLAG_TRANSFORMING;
			continue;
		}
		if (*flagsPtr & FIGHTER_FLAG_STUNNED) {
			continue;
		}
		if (*flagsPtr & FIGHTER_FLAG_CONFUSED) {
			BTL_selectConfusedMove((DigimonEntity *)entity, fighter, i);
			continue;
		}
		if (*flagsPtr & FIGHTER_FLAG_SENILE) {
			continue;
		}
		if (*flagsPtr & FIGHTER_FLAG_ON_CHARGEUP) {
			continue;
		}
		if (*flagsPtr & FIGHTER_FLAG_ON_COOLDOWN) {
			continue;
		}
		if (!(*flagsPtr & (FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED | FIGHTER_FLAG_ON_CHARGEUP | FIGHTER_FLAG_ON_COOLDOWN | FIGHTER_FLAG_DEAD)) && (fighter->flatTimer == 0)) {
			anim = ((DigimonEntity *)entity)->stats.base.moves[3];
			if (anim != 0xff) {
				anim = entityGetTechFromAnim(entity, anim);
				if ((anim >= 0x3a) && (anim < 0x71) && (fighter->finisherProgress == fighter->finisherGoal)) {
					fighter->targetId = 0;
					BTL_setupQueuedMove((DigimonEntity *)entity, fighter, i, 3);
					fighter->finisherProgress = 0;
					continue;
				}
			}
		}
		if (!(*flagsPtr & FIGHTER_FLAG_10)) {
			fighter->targetId = 0;
		}
		GsSetLsMatrix(&GsWSMATRIX);
		m = &entity->posData[1].posMatrix.workm;
		pos.vx = m->t[0];
		pos.vy = m->t[1];
		pos.vz = m->t[2];
		gte_ldv0(&pos);
		gte_rtps();
		gte_stsxy(&out);
		if (isOffScreen(&out, 0xc8, 0xa0) == 1) {
			stats->current.chargeMode = 2;
		} else {
			stats->current.chargeMode = randomLimit(2);
		}
		BTL_selectEnemyMove((DigimonEntity *)entity, fighter, i);
	}
}

void BTL_tickFighterStates(void)
{
	long id;
	DigimonEntity *entity;
	DigimonEntity *target;
	CombatData *combat;
	FighterData *fighter;
	uint16_t *flags;
	int32_t i;

	if (FLEE_DISABLED[1] > 0) {
		FLEE_DISABLED[1]--;
	}

	combat = COMBAT_DATA_PTR;
	for (i = 0; ENEMY_COUNT >= i; i++) {
		fighter = &combat->fighter[i];
		flags = &fighter->flags;
		entity = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		BTL_addFinisherProgress(fighter, 1);
		id = fighter->targetId;

		if (id != 0xff) {
			target = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[id]];
		} else {
			target = NULL;
		}
		if (*flags & FIGHTER_FLAG_ATTACKING) {
			BTL_tickAttackState(&entity->entity, target, i);
		} else if ((*flags & FIGHTER_FLAG_KNOCKED_BACK) || (*flags & FIGHTER_FLAG_BLOCKING)) {
			BTL_tickHitState(&entity->entity, fighter, i);
		} else if (fighter->moveRange != -1) {
			if (*flags & FIGHTER_FLAG_FLATTENED) {
				BTL_tickFlatState(entity, target, fighter, i);
			} else if (*flags & FIGHTER_FLAG_STUNNED) {
				BTL_tickStunState(&entity->entity);
			} else if (*flags & FIGHTER_FLAG_CONFUSED) {
				BTL_tickConfusedState(entity, target, fighter, i);
			} else if (*flags & FIGHTER_FLAG_SENILE) {
				BTL_tickSenileState(&entity->entity, fighter);
			} else if (*flags & FIGHTER_FLAG_ON_CHARGEUP) {
				BTL_tickChargeState(entity, target, fighter);
			} else if (*flags & FIGHTER_FLAG_ON_COOLDOWN) {
				BTL_tickCooldownState(entity, target, fighter);
			} else {
				BTL_tickQueuedMove(entity, target, fighter, i);
			}
		}
	}

	BTL_tickAttackHits();
	BTL_tickStatusEffects();
}

void BTL_tickPartnerWaypointTrail(void)
{
	int8_t tamerTileX;
	int8_t tamerTileY;
	int8_t partnerTileX;
	int8_t partnerTileY;
	long n;
	int16_t src;
	int16_t dst;

	getEntityTile(ENTITY_TABLE[1], &partnerTileX, &partnerTileY);
	if ((partnerTileX != PARTNER_PREVIOUS_TILE_X) || (partnerTileY != PARTNER_PREVIOUS_TILE_Y)) {
		getEntityTile(ENTITY_TABLE[0], &tamerTileX, &tamerTileY);
		if (isFiveTileWidePathBlocked(tamerTileX, tamerTileY, partnerTileX, partnerTileY) == 1) {
			if (PARTNER_WAYPOINT_COUNT != 0) {
				if (isFiveTileWidePathBlocked(partnerTileX, partnerTileY, PARTNER_WAYPOINT_X[PARTNER_WAYPOINT_CURRENT], PARTNER_WAYPOINT_Y[PARTNER_WAYPOINT_CURRENT]) == 1) {
					for (n = PARTNER_WAYPOINT_COUNT; n > 0; n--) {
						src = (PARTNER_WAYPOINT_CURRENT + n - 1) % 30;
						dst = (PARTNER_WAYPOINT_CURRENT + n) % 30;
						PARTNER_WAYPOINT_X[dst] = PARTNER_WAYPOINT_X[src];
						PARTNER_WAYPOINT_Y[dst] = PARTNER_WAYPOINT_Y[src];
					}
					setPartnerWaypoint(PARTNER_WAYPOINT_CURRENT, PARTNER_PREVIOUS_TILE_X, PARTNER_PREVIOUS_TILE_Y);
				}
			} else {
				initializePartnerWaypoint();
				setPartnerWaypoint(PARTNER_WAYPOINT_CURRENT, PARTNER_PREVIOUS_TILE_X, PARTNER_PREVIOUS_TILE_Y);
			}
		} else {
			initializePartnerWaypoint();
		}
	}

	PARTNER_PREVIOUS_TILE_X = partnerTileX;
	PARTNER_PREVIOUS_TILE_Y = partnerTileY;
}

void BTL_deinitializeCombat(void)
{
	int16_t *src;
	Stats *stats;
	int32_t i;
	long j;
	int32_t found;

	PARTNER_ENTITY.digimonEntity.stats.current.chargeMode = BTL_SAVED_CHARGE_MODE;
	GAME_STATE = 2;
	BTL_removeCombatObjects();
	closeInventoryBoxes();
	for (i = 0; ENEMY_COUNT >= i; i++) {
		BTL_resetFlatten(i);
		BTL_removeStatusEffects((DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]], &COMBAT_DATA_PTR->fighter[i]);
		if (i != 0) {
			stats = &((DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]])->stats;
			src = (int16_t *)&INITIAL_COMBAT_STATS[i];
			stats->current.currentHP = *src++;
			stats->current.currentMP = *src++;
			stats->base.off = *src++;
			stats->base.def = *src++;
			stats->base.speed = *src++;
			stats->base.brain = *src;
		}
		COMBAT_DATA_PTR->fighter[i].flags = 0;
	}

	if ((PARTNER_ENTITY.digimonEntity.stats.current.currentHP != 0) && (COMBAT_DATA_PTR->player.currentCommand[0] != 1)) {
		BTL_handleVictorySequence();
	} else {
		resetStatsAfterCombat();
		BTL_removeDeathCountdown();
		for (i = 0; ENEMY_COUNT >= i; i++) {
			removeEntityText(i);
		}
	}

	BTL_COMMAND_MENU_ACTIVE = 0;
	while (BTL_isCommandMenuClosed() == 0) {
		BTL_battleTickFrame();
	}

	BTL_removePartnerStatusBars();
	if (FLEE_DISABLED[0] != 0) {
		for (i = 2; i < 10; i++) {
			if (ENTITY_TABLE[i] != NULL) {
				found = 0;
				for (j = 1; j <= ENEMY_COUNT; j++) {
					if (COMBAT_DATA_PTR->player.entityIds[j] == i) {
						found = 1;
					}
				}
				if (found == 0) {
					if (ENTITY_TABLE[i]->isOnScreen == 1) {
						ENTITY_TABLE[i]->isOnMap = 0;
					} else {
						ENTITY_TABLE[i]->isOnMap = 1;
					}
				}
			}
		}
	} else {
		for (i = 2; i < 10; i++) {
			if (ENTITY_TABLE[i] != NULL) {
				if (ENTITY_TABLE[i]->isOnScreen == 1) {
					ENTITY_TABLE[i]->anim.animFlag &= 0xfb;
					ENTITY_TABLE[i]->isOnMap = 0;
				} else {
					ENTITY_TABLE[i]->isOnMap = 1;
				}
			}
		}
	}

	stopBGM();
	stopSound();
}

void BTL_removeCombatObjects(void)
{
	int32_t i;
	DigimonEntity *digimon;

	BTL_removeAllFinisherAuras();
	BTL_removeAllPoisonEffects();
	BTL_removeAllStunEffects();
	BTL_removeAllBuffRingsEffects();
	BTL_removeAllBuffDiskEffects();
	BTL_removeAllItemParticles();
	removeBuffModelObject();
	BTL_removeEnemyHPBars();
	BTL_removeAllAuraProjectiles();

	for (i = 0; ENEMY_COUNT >= i; i++) {
		digimon = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		BTL_removeMoveEffect(digimon, &COMBAT_DATA_PTR->fighter[i]);
	}

	BTL_unloadAllEFESlots();
	BTL_removeEFEEngine();
}

void BTL_removeEnemyHPBars(void)
{
	removeObject(0x191, 0);
}

void BTL_resetFlatten(int16_t index)
{
	Entity *entity;
	FighterData *fighter;

	entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]];
	fighter = &COMBAT_DATA_PTR->fighter[index];
	entity->flatSprite = -1;
	fighter->flags &= ~FIGHTER_FLAG_FLATTENED;
	fighter->flatTimer = 0;
	entity->posData->scale.vx = entity->posData->scale.vy = entity->posData->scale.vz = modModelScale(entity->type); /* MOD (species rules as data) */
}

void BTL_handleVictorySequence(void)
{
	int32_t count;
	int32_t done;
	int32_t ready;
	int32_t n;
	int32_t t;
	VECTOR loc;
	SVECTOR tmp;
	int32_t sound;
	int32_t soundId;
	int32_t loaded;
	Entity *entity;
	int32_t i;

	n = 0;
	ready = 0;
	startAnimation(ENTITY_TABLE[0], 1);
	startAnimation(ENTITY_TABLE[1], 0x21);
	stopBGM();
	stopSound();
	playSound(0, 0x15);

	t = 0;
	soundId = getMapSoundId(CURRENT_SCREEN);
	sound = loadMapSounds2(soundId);
	for (;;) {
		loaded = isSoundLoaded(1, sound);
		if ((loaded == -1) || (t < 0x29)) {
			BTL_battleTickFrame();
			t++;
		} else {
			break;
		}
	}

	playSound(0, 0x18);
	startAnimation(ENTITY_TABLE[0], 0xb);
	startAnimation(ENTITY_TABLE[1], 0x2a);
	while ((ready == 0) || (n == 0)) {
		if (!(ENTITY_TABLE[1]->anim.animFlag & 1)) {
			n = 1;
			startBattleIdleAnimation(&PARTNER_ENTITY.digimonEntity, &PARTNER_ENTITY.digimonEntity.stats, COMBAT_DATA_PTR->fighter[0].flags);
		}
		if (!(ENTITY_TABLE[0]->anim.animFlag & 1)) {
			ready = 1;
			startAnimation(ENTITY_TABLE[0], 1);
		}
		BTL_battleTickFrame();
	}

	handleBattleEndBox();
	for (i = 1; ENEMY_COUNT >= i; i++) {
		entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		startAnimation(entity, 0x2c);
	}

	if (FLEE_DISABLED[0] != 0) {
		for (;;) {
			for (i = 1; ENEMY_COUNT >= i; i++) {
				if (!(ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]]->anim.animFlag & 1)) {
					break;
				}
			}
			if (i == (ENEMY_COUNT + 1)) {
				break;
			}
			BTL_battleTickFrame();
		}
		for (i = 0; ENEMY_COUNT >= i; i++) {
			startAnimation(ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]], 0x21);
		}
		return;
	}

	done = 0;
	while (done == 0) {
		count = 0;
		for (i = 1; ENEMY_COUNT >= i; i++) {
			entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
			if (!(entity->anim.animFlag & 1) && (entity->anim.animId == 0x2c)) {
				startAnimation(entity, 0x24);
				entity->anim.animFlag |= 4;
				tmp = ((NPCEntity *)entity)->flee;
				copyVector(&loc, &tmp);
				entityLookAtLocation(entity, &loc);
			}
			if (entityIsOffScreen(entity, 0x140, 0xf0) != 0) {
				count++;
			}
		}
		if (count == ENEMY_COUNT) {
			done = 1;
		}
		BTL_battleTickFrame();
	}
}

void BTL_tickAttackState(Entity *entity, DigimonEntity *target, int32_t id)
{
	int32_t i;
	Entity *other;

	if ((entity->anim.animFlag & 1) == 0) {
		if (entity->anim.frameCount != entity->anim.animFrame) {
			for (i = 0; i <= ENEMY_COUNT; ++i) {
				if (i != id) {
					other = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
					if (DIGI_PARA(other->type).moves[(uint32_t)(other->anim.animId - 0x2e)] == 0x2d) {
						if (id == COMBAT_DATA_PTR->fighter[i].targetId) {
							return;
						}
					}
				}
			}
			entity->anim.animFlag |= 1;
		}
	}

	BTL_tickFighterAction(id);
}

void BTL_tickHitState(Entity *entity, FighterData *fighter, int32_t arg2)
{
	BTL_tickFighterAction(arg2);
	if (!(entity->anim.animFlag & 1)) {
		fighter->invulnerableTimer--;
		if (fighter->invulnerableTimer == 0) {
			entity->anim.animFlag |= 1;
		}
	}
}

void BTL_tickFlatState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int32_t arg3)
{
	if (NO_AI_FLAG != 0) {
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		return;
	}

	if (BTL_handlePartnerMoveCommand(digimon, target, fighter) == 0) {
		BTL_tickRangedAttack(digimon, target, fighter, 0x79);
		if (fighter->flags & FIGHTER_FLAG_ATTACKING) {
			digimon->entity.flatSprite = 2;
		}
	}
}

void BTL_tickStunState(Entity *entity)
{
	if (entity->anim.animId != 0x22) {
		startAnimation(entity, 0x22);
	}

	entity->anim.animFlag &= 0xfe;
}

void BTL_tickConfusedState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int32_t arg3)
{
	if (NO_AI_FLAG != 0) {
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		return;
	}

	if ((fighter->flags & FIGHTER_FLAG_ON_COOLDOWN) || (fighter->flags & FIGHTER_FLAG_ON_CHARGEUP)) {
		BTL_confusedRotate(&digimon->entity);
		BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
		collisionGrace(NULL, &digimon->entity, 0x118, 0xc8);
		if (fighter->cooldown < 2) {
			fighter->flags &= ~FIGHTER_FLAG_ON_COOLDOWN;
			fighter->cooldown = 0;
		}
		return;
	}

	if (target == NULL) {
		BTL_confusedRotate(&digimon->entity);
		if (BTL_tickMeleeAttack(digimon, NULL, fighter, arg3) != 0) {
			collisionGrace(NULL, &digimon->entity, 0x118, 0xc8);
		}
		if (fighter->flags & FIGHTER_FLAG_ATTACKING) {
			return;
		}
		if (randomLimit(100) >= BM(IDLE_STOP_PCT)) {
			return;
		}
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		BTL_startQueuedMove(digimon, target, fighter);
		return;
	}

	switch (fighter->moveRange) {
	case 1:
		if (BTL_tickMeleeAttack(digimon, target, fighter, arg3) != 0) {
			collisionGrace(&target->entity, &digimon->entity, 0x118, 0xc8);
		}
		break;
	case 2:
	case 3:
		BTL_tickRangedAttack(digimon, target, fighter, entityGetTechFromAnim(&digimon->entity, fighter->queuedAnim));
		break;
	case 4:
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		BTL_startQueuedMove(digimon, target, fighter);
		break;
	}
}

void BTL_tickSenileState(Entity *entity, FighterData *fighter)
{
	fighter->senileTimer--;
	if (fighter->senileTimer == 0) {
		fighter->flags &= ~(FIGHTER_FLAG_TRANSFORMING | FIGHTER_FLAG_SENILE);
		return;
	}

	if (entity->anim.animId != 0x13) {
		startAnimation(entity, 0x13);
	}
}

void BTL_tickChargeState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter)
{
	int32_t r;
	int16_t tech;

	if (NO_AI_FLAG != 0) {
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		return;
	}

	r = BTL_handlePartnerMoveCommand(digimon, target, fighter);
	if (fighter->cooldown != 0) {
		if (r == 0) {
			switch (digimon->stats.current.chargeMode) {
			case 0:
				BTL_maintainTargetDistance(digimon, target, fighter);
				break;
			case 1:
				handleBattleIdle(digimon, &digimon->stats, fighter->flags);
				entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
				fighter->hasCollidedWhileDistanceCmd = 0;
				break;
			case 2:
				BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
				BTL_backAwayFromTarget(digimon, target, fighter);
				break;
			}
		}
		if (fighter->cooldown < 2) {
			fighter->flags &= ~(FIGHTER_FLAG_TRANSFORMING | FIGHTER_FLAG_ON_CHARGEUP);
			fighter->cooldown = 0;
		}
		return;
	}

	if (r != 0) {
		return;
	}

	switch (digimon->stats.current.chargeMode) {
	case 0:
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
		fighter->hasCollidedWhileDistanceCmd = 0;
		if (fighter->speedBuffer > 0) {
			fighter->flags &= ~FIGHTER_FLAG_ON_CHARGEUP;
		}
		break;
	case 1:
		BTL_maintainTargetDistance(digimon, target, fighter);
		tech = entityGetTechFromAnim(&digimon->entity, fighter->queuedAnim);
		if ((fighter->speedBuffer >= BM(SPEED_MAX)) || (fighter->speedBuffer >= modSpeciesMove(digimon->entity.type, tech)->power)) {
			fighter->flags &= ~FIGHTER_FLAG_ON_CHARGEUP;
		}
		break;
	case 2:
		BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
		BTL_backAwayFromTarget(digimon, target, fighter);
		if (fighter->speedBuffer >= BM(SPEED_MAX)) {
			fighter->flags &= ~FIGHTER_FLAG_ON_CHARGEUP;
		}
		break;
	}
}

void BTL_tickCooldownState(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter)
{
	if (NO_AI_FLAG != 0) {
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		return;
	}

	switch (digimon->stats.current.chargeMode) {
	case 0:
	case 1:
		BTL_maintainTargetDistance(digimon, target, fighter);
		break;
	case 2:
		BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
		BTL_backAwayFromTarget(digimon, target, fighter);
		break;
	}

	if (fighter->cooldown < 2) {
		fighter->flags &= ~(FIGHTER_FLAG_TRANSFORMING | FIGHTER_FLAG_ON_COOLDOWN);
		fighter->cooldown = 0;
	}
}

void BTL_tickQueuedMove(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int32_t arg3)
{
	if ((NO_AI_FLAG != 0) && (FINISHING_ENTITY != &digimon->entity)) {
		handleBattleIdle(digimon, &digimon->stats, fighter->flags);
		return;
	}

	if (BTL_handlePartnerMoveCommand(digimon, target, fighter) == 0) {
		switch (fighter->moveRange) {
		case 1:
			if (BTL_tickMeleeAttack(digimon, target, fighter, arg3) != 0) {
				collisionGrace(&target->entity, &digimon->entity, 0x118, 0xc8);
			}
			break;
		case 2:
		case 3:
			BTL_tickRangedAttack(digimon, target, fighter, DIGI_PARA(digimon->entity.type).moves[fighter->queuedAnim - 0x2e]);
			break;
		case 4:
			handleBattleIdle(digimon, &digimon->stats, fighter->flags);
			BTL_startQueuedMove(digimon, target, fighter);
			break;
		}
	}
}

int32_t BTL_handlePartnerMoveCommand(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter)
{
	uint32_t reach;

	if (NO_AI_FLAG != 0) {
		return 0;
	}

	if (digimon == (DigimonEntity *)ENTITY_TABLE[1]) {
		switch (COMBAT_DATA_PTR->player.currentCommand[0]) {
		case 1:
			BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
			BTL_moveTowardLocation(digimon, &ENTITY_TABLE[0]->posData->location, 0, 0);
			if (entityIsOffScreen(ENTITY_TABLE[1], 0x140, 0xf0) != 0) {
				startAnimation(ENTITY_TABLE[1], 0x21);
				ENTITY_TABLE[1]->anim.animFlag |= 2;
			}
			fighter->hasCollidedWhileDistanceCmd = 0;
			return 1;
		case 6:
			handleBattleIdle(digimon, &digimon->stats, fighter->flags);
			entityLookAtLocation(&digimon->entity, &ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[fighter->targetId]]->posData->location);
			fighter->hasCollidedWhileDistanceCmd = 0;
			return 1;
		case 5:
			reach = BTL_getMoveWithHighestDistance(target) + 0x9c400;
			if ((digimon->entity.anim.animId >= 0x23) && (digimon->entity.anim.animId < 0x25)) {
				reach += 0x27100;
			}
			if (BTL_getDistanceSquared(&digimon->entity, &target->entity) < reach) {
				BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
				BTL_backAwayFromTarget(digimon, target, fighter);
			} else {
				fighter->hasCollidedWhileDistanceCmd = 0;
				handleBattleIdle(digimon, &digimon->stats, fighter->flags);
				entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
			}
			return 1;
		}
	}

	return 0;
}

extern int8_t MOD_BATTLE_LOADING; /* MOD (quick battle start), btl.c */

int32_t BTL_tickMeleeAttack(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int16_t arg3)
{
	int16_t *rot;
	PositionData *pos;
	uint32_t dist;
	int32_t radius;
	int16_t orig;
	int16_t tech;

	pos = digimon->entity.posData;
	rot = &pos->rotation.vy;
	digimon->entity.anim.animFlag &= 0xfd;
	orig = *rot;
	if (target != NULL) {
		dist = BTL_getDistanceSquared(&digimon->entity, &target->entity);
		radius = DIGI_PARA(digimon->entity.type).radius + DIGI_PARA(target->entity.type).radius;
		radius = radius * radius;
		if (dist <= radius) {
			handleBattleIdle(digimon, &digimon->stats, fighter->flags);
			/* Melee starts its animation and effect here, bypassing BTL_startQueuedMove.
			 * Keep the queued move until the sounds and effects have finished loading. */
			if (MOD_BATTLE_LOADING) {
				return 0;
			}
			if (NO_AI_FLAG != 0) {
				if (FINISHING_ENTITY != &digimon->entity) {
					return 0;
				}
				if (BTL_FINISHER_TIMER > 0) {
					BTL_FINISHER_TIMER--;
					entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
					return 0;
				}
			} else {
				tech = entityGetTechFromAnim(&digimon->entity, fighter->queuedAnim);
				if ((tech >= 0x3a) && (tech < 0x71)) {
					NO_AI_FLAG = 1;
				}
				if (NO_AI_FLAG != 0) {
					FINISHING_ENTITY = &digimon->entity;
					if (digimon == (DigimonEntity *)ENTITY_TABLE[1]) {
						BTL_initializeFinisherChargeup(tech);
					}
					startAnimation(&digimon->entity, fighter->queuedAnim);
					digimon->entity.anim.animFlag &= 0xfe;
					BTL_FINISHER_AURA_ID = BTL_addFinisherAura(&digimon->entity, BM(FIN_HOLD));
					BTL_FINISHER_TIMER = BM(FIN_HOLD);
					return 0;
				}
			}
			if (BTL_isMoveOnCooldown(&digimon->entity, fighter) != 0) {
				return 0;
			}
			startAnimation(&digimon->entity, fighter->queuedAnim);
			fighter->flags |= FIGHTER_FLAG_ATTACKING;
			BTL_setupMoveExecution(digimon, target, fighter);
			return 0;
		}
		if (NO_AI_FLAG == 0) {
			BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
			entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
			goto check;
		}
		if (MOD_BATTLE_LOADING) {
			return 0;
		}
		if (FINISHING_ENTITY != &digimon->entity) {
			return 0;
		}
		if (BTL_FINISHER_TIMER > 0) {
			BTL_FINISHER_TIMER--;
			entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
			return 0;
		}
		startAnimation(&digimon->entity, fighter->queuedAnim);
		fighter->flags |= FIGHTER_FLAG_ATTACKING;
		BTL_setupMoveExecution(digimon, target, fighter);
		return 0;
	}

	BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
check:
	if (entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8) == -1) {
		return 0;
	} else {
		*rot = orig;
		return 1;
	}
}

void BTL_tickRangedAttack(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter, int16_t move)
{
	uint32_t dist;
	uint32_t total;

	if (fighter->unk15 > BM(T_APPROACH)) {
		BTL_startQueuedMove(digimon, target, fighter);
		fighter->unk15 = 0;
		return;
	}

	if ((NO_AI_FLAG != 0) && (&digimon->entity == FINISHING_ENTITY)) {
		BTL_startQueuedMove(digimon, target, fighter);
		return;
	}

	dist = BTL_getDistanceSquared(&digimon->entity, &target->entity);
	total = DIGI_PARA(digimon->entity.type).radius + DIGI_PARA(target->entity.type).radius;
	total = *(uint32_t *)&MOVE_DATA[move].distance + (total * total);
	if (dist > (total + (total * 3 / 10))) {
		BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
		BTL_moveTowardLocation(digimon, &target->entity.posData->location, 0x118, 0xc8);
		fighter->unk15++;
		return;
	}

	if (dist < (total - (total * 3 / 10))) {
		BTL_setWalking(&digimon->entity, &digimon->stats, fighter->flags);
		BTL_backAwayFromTarget(digimon, target, fighter);
		fighter->unk15++;
		return;
	}

	fighter->unk15 = 0;
	handleBattleIdle(digimon, &digimon->stats, fighter->flags);
	if (fighter->flags & FIGHTER_FLAG_FLATTENED) {
		if (BMEVERY(BATTLE_FRAME_COUNT, T_FLAT_REPEAT)) {
			BTL_startQueuedMove(digimon, target, fighter);
		} else {
			entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
		}
	} else {
		BTL_startQueuedMove(digimon, target, fighter);
	}
}

void BTL_setWalking(Entity *entity, Stats *stats, uint16_t flags)
{
	if ((entity->anim.animId != 0x24) && (entity->anim.animId != 0x23)) {
		BTL_startWalkingAnimation(entity, stats, flags);
	}
}

void BTL_moveTowardLocation(DigimonEntity *digimon, VECTOR *target, int16_t dx, int16_t dy)
{
	int16_t facing;

	facing = digimon->entity.posData->rotation.vy;
	entityLookAtLocation(&digimon->entity, target);
	if (entityCheckCollision(NULL, &digimon->entity, dx, dy) != -1) {
		digimon->entity.posData->rotation.vy = facing;
		collisionGrace(NULL, &digimon->entity, dx, dy);
	}
}

uint32_t BTL_getMoveWithHighestDistance(DigimonEntity *digimon)
{
	uint32_t best;
	int32_t i;
	uint8_t tech;
	uint8_t anim;

	best = 0;
	for (i = 0; i < 4; i++) {
		anim = digimon->stats.base.moves[i];
		if (anim != 0xff) {
			tech = entityGetTechFromAnim(&digimon->entity, anim);
			if (best < MOVE_DATA[tech].distance) {
				best = MOVE_DATA[tech].distance;
			}
		}
	}

	return best;
}

void BTL_backAwayFromTarget(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter)
{
	int16_t *targetRot;
	int16_t *rot;
	int16_t orig;
	int16_t away;
	int16_t ccw;
	int16_t cw;
	int16_t hit;
	int16_t diff;

	rot = &digimon->entity.posData->rotation.vy;
	orig = *rot;
	entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
	away = (*rot + 0x800) & 0xfff;

	if (fighter->hasCollidedWhileDistanceCmd == 0) {
		*rot = away;

		hit = entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8);
		if (hit != -1) {
			fighter->hasCollidedWhileDistanceCmd = 1;
			BTL_findUnblockedRotation(&digimon->entity, rot, hit, orig);
		}

		return;
	}

	*rot = orig;
	targetRot = &target->entity.posData->rotation.vy;
	if (orig == *targetRot) {
		*rot = (*rot + 0x400) & 0xfff;

		if (entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8) == -1) {
			return;
		}

		*rot = orig;
		*rot = (*rot + 0xc00) & 0xfff;

		if (entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8) == -1) {
			return;
		}

		*rot = orig;

		hit = entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8);
		if (hit != -1) {
			BTL_findUnblockedRotation(&digimon->entity, rot, hit, orig);
		}

		return;
	}

	ccw = orig - away;
	if (ccw < 0) {
		ccw = (ccw + 0x1000) & 0xfff;
	}

	cw = away - orig;
	if (cw < 0) {
		cw = (cw + 0x1000) & 0xfff;
	}

	diff = ccw - cw;
	if (diff < 0) {
		if (ccw > 20) {
			*rot = (*rot + 0xfec) & 0xfff;
		} else {
			*rot = (*rot + 0x1000 - ccw) & 0xfff;
		}
	} else {
		if (cw > 20) {
			*rot = (*rot + 20) & 0xfff;
		} else {
			*rot = (*rot + cw) & 0xfff;
		}
	}

	if (entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8) == -1) {
		return;
	}

	*rot = orig;

	hit = entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8);
	if (hit != -1) {
		BTL_findUnblockedRotation(&digimon->entity, rot, hit, orig);
	}
}

void BTL_tickFighterAction(int32_t index)
{
	DigimonEntity *digimon;
	FighterData *fighter;

	digimon = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]];
	fighter = &COMBAT_DATA_PTR->fighter[index];
	if (entityCheckCollision(NULL, &digimon->entity, 0x118, 0xc8) != -1) {
		digimon->entity.anim.animFlag |= 2;
	} else {
		digimon->entity.anim.animFlag &= 5;
	}

	if ((NO_AI_FLAG != 0) && (FINISHING_ENTITY == &digimon->entity) && (BTL_FINISHER_TIMER > 0)) {
		return;
	}

	if ((fighter->flags & (FIGHTER_FLAG_FLATTENED | FIGHTER_FLAG_ATTACKING)) == 0x28) {
		fighter->flatAttackTimer--;
		switch (fighter->flatAttackTimer) {
		case 0x1c:
			BTL_addAuraProjectile(&digimon->entity);
			break;
		case 0xa:
			digimon->entity.flatSprite = 0;
			break;
		case 0:
			digimon->entity.anim.animFlag &= 0xfe;
			break;
		}
	}

	if (digimon->entity.anim.animFlag & 1) {
		return;
	}

	if (FINISHING_ENTITY == &digimon->entity) {
		NO_AI_FLAG = 0;
		FINISHING_ENTITY = NULL;
	}

	fighter->hasCollidedWhileDistanceCmd = 0;
	if (fighter->flags & FIGHTER_FLAG_ATTACKING) {
		if (index == 0) {
			COMBAT_DATA_PTR->player.hitCount++;
		}
		fighter->flags &= ~FIGHTER_FLAG_10;
		fighter->flags |= FIGHTER_FLAG_ON_COOLDOWN;
		fighter->cooldown = BM(CD_ATTACK);
		BTL_addFinisherProgress(fighter, fighter->finisherGoal * BM(FIN_ATTACK_PCT) / 100);
	}

	if (fighter->invulnerableTimer <= 0) {
		if (fighter->flags & FIGHTER_FLAG_FLATTENED) {
			digimon->entity.flatSprite = 0;
		}
		if (!(fighter->flags & FIGHTER_FLAG_ATTACKING)) {
			digimon->stats.current.isHit = 0;
		}
		if (fighter->flags & FIGHTER_FLAG_BLOCKING) {
			fighter->flags &= ~FIGHTER_FLAG_BLOCKING;
			BTL_clearBlockedAttacks(fighter);
		} else {
			fighter->flags &= ~(FIGHTER_FLAG_KNOCKED_BACK | FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_TRANSFORMING | FIGHTER_FLAG_BLOCKING);
		}
	}

	if (!(fighter->flags & FIGHTER_FLAG_KNOCKED_BACK)) {
		if (fighter->flatTimer == -1) {
			fighter->flatTimer = 0x41;
		}
	} else {
		fighter->senileTimer = 0;
		fighter->flags &= ~FIGHTER_FLAG_SENILE;
	}
}

void BTL_confusedRotate(Entity *entity)
{
	int16_t *rot;

	if (randomLimit(0xa) >= 8) {
		rot = &entity->posData->rotation.vy;
		*rot += randomLimit(0x400) - 0x200;
		if (*rot < 0) {
			*rot += 0x1000;
		} else {
			*rot %= 4096;
		}
	}
}

void BTL_maintainTargetDistance(DigimonEntity *attacker, DigimonEntity *target, FighterData *fighter)
{
	if ((attacker->entity.anim.animId >= 0x23) && (attacker->entity.anim.animId < 0x25)) {
		BTL_maintainDistanceRange(attacker, target, fighter, 0x27100, 0x4e200);
	} else {
		BTL_maintainDistanceRange(attacker, target, fighter, 0, 0x75300);
	}
}

void BTL_maintainDistanceRange(DigimonEntity *attacker, DigimonEntity *target, FighterData *fighter, uint32_t nearLimit, uint32_t farLimit)
{
	uint32_t dist;
	uint32_t reach;

	dist = BTL_getDistanceSquared(&attacker->entity, &target->entity);
	reach = BTL_getContactRangeSquared(&attacker->entity.type, &target->entity.type);
	if (dist < (reach + nearLimit)) {
		BTL_setWalking(&attacker->entity, &attacker->stats, fighter->flags);
		BTL_backAwayFromTarget(attacker, target, fighter);
	} else if (dist > (reach + farLimit)) {
		BTL_setWalking(&attacker->entity, &attacker->stats, fighter->flags);
		BTL_moveTowardLocation(attacker, &target->entity.posData->location, 0x118, 0xc8);
	} else {
		fighter->hasCollidedWhileDistanceCmd = 0;
		handleBattleIdle(attacker, &attacker->stats, fighter->flags);
		entityLookAtLocation(&attacker->entity, &target->entity.posData->location);
	}
}

int32_t BTL_getContactRangeSquared(int32_t *a, int32_t *b)
{
	int32_t reach;

	reach = DIGI_PARA(*a).radius + DIGI_PARA(*b).radius + 0xc8;

	return reach * reach;
}

void BTL_increaseSpeedBuffer(FighterData *fighter, Stats *stats)
{
	if (fighter->speedBuffer < BM(SPEED_MAX)) { /* MOD (battle as data) */
		if (BMEVERY(BATTLE_FRAME_COUNT, T_SPEED_STEP)) {
			fighter->speedBuffer += BMD(stats->base.speed, SPEED_DIV) + BM(SPEED_PLUS);
		}
		if (fighter->speedBuffer > BM(SPEED_MAX)) {
			fighter->speedBuffer = BM(SPEED_MAX);
		}
	}
}

void BTL_faintDigimon(DigimonEntity *digimon, FighterData *fighter, int16_t arg2)
{
	digimon->stats.current.isHit = 1;
	fighter->flags |= FIGHTER_FLAG_DEAD;
	startAnimation(&digimon->entity, 0x2b);
	BTL_resetFlatten(arg2);
	BTL_removeStatusEffects(digimon, fighter);
	fighter->flags &= ~(FIGHTER_FLAG_POISONED | FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED | FIGHTER_FLAG_KNOCKED_BACK | FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_BLOCKING);
	fighter->flags |= FIGHTER_FLAG_TRANSFORMING;
	fighter->moveRange = -1;
	BTL_resetFighterAction(fighter);
}

int32_t BTL_hasAffordableMoves(int16_t *out, int16_t index)
{
	DigimonEntity *digimon;
	FighterData *fighter;
	int32_t found;
	int32_t i;

	digimon = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]];
	fighter = &COMBAT_DATA_PTR->fighter[index];
	found = 0;

	for (i = 0; i < 4; ++i) {
		if (BTL_isMoveUsable(digimon, fighter, i) != 0) {
			out[i] = 1;
			found = 1;
		} else {
			out[i] = 0;
		}
	}

	return found;
}

void BTL_startWalkingAnimation(Entity *entity, Stats *stats, uint16_t flags)
{
	int32_t walking;
	int32_t animId;

	walking = 1;

	if (!(flags & 1)) {
		if (stats->current.currentHP > BMD(stats->base.hp, LOWHP_DIV)) {
			walking = 0;
		}
	}

	if (walking != 0) {
		animId = 0x23;
	} else {
		animId = 0x24;
	}

	startAnimation(entity, (uint8_t)animId);
}

void BTL_initializePartnerTile(void)
{
	getEntityTile(ENTITY_TABLE[1], &PARTNER_PREVIOUS_TILE_X, &PARTNER_PREVIOUS_TILE_Y);
}

void BTL_initializeEnemyHPBarSprites(void)
{
	PlayerDataSub *sub;
	int32_t i;
	PlayerDataSub *slot;

	sub = COMBAT_DATA_PTR->player.unk1;
	for (i = 0; i < 4; i++) {
		slot = sub;
		slot->unk1 = 0;
		slot->unk2 = 0x32;
		slot->unk3 = 3;
		slot->unk5 = 0x58;
		slot->unk6 = 0xec;
		slot->unk7 = 0x110;
		slot->unk8 = 0x1f3;
		slot->unk4 = 0x1f;
		slot->unk9 = 0x80;
		slot->unk10 = 0xff;
		slot->unk11 = 0x80;
		sub++;
	}
}

void BTL_addEnemyHPBars(void)
{
	addObject(0x191, 0, NULL, BTL_renderEnemyHPBars);
}

void BTL_clearBlockedAttacks(FighterData *fighter)
{
	int32_t i;

	for (i = 0; i < 0x96; i++) {
		if (fighter->table1[i] == -1) {
			break;
		}
		fighter->table1[i] = -1;
		fighter->table2[i] = -1;
	}
}

void BTL_findUnblockedRotation(Entity *entity, int16_t *rot, int16_t hit, int16_t orig)
{
	int16_t cand[4];
	int32_t i;
	int16_t base;

	if ((GAME_STATE == 4) && (hit == 0xa)) {
		hit = 0xb;
	}

	if (hit != 0xb) {
		goto grace;
	}

	base = *rot / 1024;
	cand[0] = BTL_CARDINAL_ROTATIONS[base];
	cand[1] = (BTL_CARDINAL_ROTATIONS[base] + 0x400) & 0xfff;
	for (i = 0; i < 2; i++) {
		*rot = cand[i];
		if (entityCheckCollision(NULL, entity, 0x118, 0xc8) == -1) {
			break;
		}
	}

	switch (i) {
	case 0:
		for (i = 0; i < 3; i++) {
			*rot = cand[i] = (BTL_CARDINAL_ROTATIONS[base] + 0xc00 + (i * 0x200)) & 0xfff;
			if (entityCheckCollision(NULL, entity, 0x118, 0xc8) != -1) {
				goto common;
			}
		}
		break;
	case 1:
		for (i = 0; i < 3; i++) {
			*rot = cand[i] = (BTL_CARDINAL_ROTATIONS[base] + 0x400 + (i * 0x200)) & 0xfff;
			if (entityCheckCollision(NULL, entity, 0x118, 0xc8) != -1) {
				goto common;
			}
		}
		break;
	default:
		for (i = 0; i < 3; i++) {
			*rot = cand[i] = (BTL_CARDINAL_ROTATIONS[base] + 0x800 + (i * 0x200)) & 0xfff;
			if (entityCheckCollision(NULL, entity, 0x118, 0xc8) != -1) {
				break;
			}
		}
		if (i == 3) {
			*rot = (orig + 0x800) & 0xfff;
			if (entityCheckCollision(NULL, entity, 0x118, 0xc8) == -1) {
				return;
			}
		}
		break;
	}

common:
	if (i != 3) {
		*rot = orig;
		collisionGrace(NULL, entity, 0x118, 0xc8);
		return;
	}

	*rot = cand[0] + randomLimit(0x400);
	if (entityCheckCollision(NULL, entity, 0x118, 0xc8) != -1) {
		*rot = orig;
		collisionGrace(NULL, entity, 0x118, 0xc8);
	}

	return;
grace:
	*rot = orig;
	collisionGrace(NULL, entity, 0x118, 0xc8);
}

void modBattleLoadFinish(void); /* MOD (quick battle start), btl.c */
void BTL_tickQuickStartText(void); /* MOD (quick battle start), battle_setup.c */
void BTL_endQuickStartText(void);

int32_t BTL_battleMain(void)
{
	COMBAT_AREA_X = 0xa0 - DRAWING_OFFSET_X;
	COMBAT_AREA_Y = 0x78 - DRAWING_OFFSET_Y;
	int32_t timeLimit; /* MOD (timed battle) */
	int32_t timedOut;
	int32_t fighterIndex;
	int16_t enemies[4];
	int16_t remaining;

	timeLimit = readPStat(PSTAT_BATTLE_TIME_LIMIT) * 20;
	timedOut = 0;
	writePStat(PSTAT_BATTLE_TIME_LIMIT, 0);
	writePStat(PSTAT_BATTLE_TIMED_OUT, 0);
	BTL_initializeCombat();
	BTL_handleBattleIntro();
	for (;;) {
		if (BTL_isBattleFinished() != 0) {
			break;
		}
		if ((timeLimit > 0) && (BATTLE_FRAME_COUNT >= timeLimit) &&
		    (NO_AI_FLAG == 0) && (FINISHING_ENTITY == NULL) &&
		    (PARTNER_ENTITY.digimonEntity.stats.current.currentHP - COMBAT_DATA_PTR->fighter[0].hpDamageBuffer > 0)) {
			/* Let techniques and hit reactions close before removing their EFE engine.
			 * With no surviving enemy, let the normal victory/death animations finish. */
			BTL_getRemainingEnemies(ENTITY_TABLE[1], enemies, &remaining);
			if (remaining > 0) {
				for (fighterIndex = 0; fighterIndex <= ENEMY_COUNT; fighterIndex++) {
					FighterData *fighter = &COMBAT_DATA_PTR->fighter[fighterIndex];
					if ((fighter->flags & (FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_KNOCKED_BACK | FIGHTER_FLAG_BLOCKING)) ||
					    (fighter->hpDamageBuffer != 0)) {
						break;
					}
				}
				if (fighterIndex > ENEMY_COUNT) {
					timedOut = 1;
					writePStat(PSTAT_BATTLE_TIMED_OUT, 1);
					break;
				}
			}
		}
		BTL_tickPartnerAI();
		BTL_tickEnemyAI();
		BTL_tickFighterStates();
		BTL_tickPartnerWaypointTrail();
		BTL_battleTickFrame();
		BTL_tickQuickStartText(); /* MOD (quick battle start), battle_setup.c */
		handlePause();
		BATTLE_FRAME_COUNT++;
	}

	BTL_endQuickStartText();
	modBattleLoadFinish(); /* MOD (quick battle start), btl.c: what is still loading lands before the clean-up */
	removePauseBox();
	BTL_deinitializeCombat();
	if (timedOut) { /* the time ran out: a battle survived, neither lost nor fled */
		COMBAT_DATA_PTR->player.currentCommand[0] = 0;
		return 1;
	}
	if (PARTNER_ENTITY.digimonEntity.stats.current.currentHP == 0) {
		COMBAT_DATA_PTR->player.currentCommand[0] = 0;
		return -1;
	}

	if (COMBAT_DATA_PTR->player.currentCommand[0] == 1) {
		COMBAT_DATA_PTR->player.currentCommand[0] = 0;
		return 0;
	}

	return 1;
}

int16_t BTL_getAttackTech(AttackObject *attack)
{
	int16_t slot;
	int32_t i;
	int16_t tech;

	if (attack->effectId != 0x179) {
		slot = BTL_getFighterSlot(attack->casterId);
		for (i = 0; i < 4; i++) {
			if (attack->effectId == COMBAT_DATA_PTR->fighter[slot].effectSlot[i]) {
				break;
			}
		}
		tech = entityGetTechFromAnim(ENTITY_TABLE[attack->casterId], ((DigimonEntity *)ENTITY_TABLE[attack->casterId])->stats.base.moves[i]);
	} else {
		tech = 0x79;
	}

	return tech;
}

int32_t BTL_applyBuffMove(DigimonEntity *digimon, int32_t slot, int16_t anim)
{
	Stats *stats;
	int32_t row;

	stats = &digimon->stats;
	/* MOD (battle as data): the buff moves and what they add, in percent of the stat (the rows of MOD_BATTLE) */
	for (row = 0; row < 5; row++) {
		if (BMV(BUFF_MOVE0 + row) == anim) {
			break;
		}
	}
	if (row == 5) {
		return 0;
	}
	if (BM(BUFF_OFF0 + row) > 0) {
		BTL_buffStats(digimon, slot, (int16_t)(stats->base.off * BM(BUFF_OFF0 + row) / 100), &stats->base.off, 0xb, 3);
	}
	if (BM(BUFF_DEF0 + row) > 0) {
		BTL_buffStats(digimon, slot, (int16_t)(stats->base.def * BM(BUFF_DEF0 + row) / 100), &stats->base.def, 0xb, 4);
	}
	if (BM(BUFF_SPD0 + row) > 0) {
		BTL_buffStats(digimon, slot, (int16_t)(stats->base.speed * BM(BUFF_SPD0 + row) / 100), &stats->base.speed, 0xb, 5);
	}

	return 1;
}

int16_t BTL_calculateHitChance(DigimonEntity *attacker, DigimonEntity *defender, FighterData *fighter, int16_t move)
{
	int32_t x;
	int32_t speedDiff;
	int16_t result;

	if (DIGI_PARA(defender->entity.type).level < BM(HIT_AUTO_LEVEL)) { /* MOD (battle as data) */
		return 100;
	}

	if (fighter->flags & (FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED | FIGHTER_FLAG_SENILE)) {
		return 100;
	}

	if (PARTNER_PARA.condition & 0x40) {
		return 100;
	}

	if (fighter->speedBuffer <= 0) {
		return 100;
	}

	if (defender->entity.anim.animId >= 0x2eU) {
		return 100;
	}

	if ((move >= 0x3a) && (move < 0x71)) {
		if (fighter->flags & FIGHTER_FLAG_BLOCKING) {
			fighter->flags &= ~FIGHTER_FLAG_BLOCKING;
			fighter->invulnerableTimer = 0;
		}
		return 100;
	}

	if (fighter->flags & FIGHTER_FLAG_BLOCKING) {
		return 0;
	}

	/* MOD (caps): the speed difference is limited to +-999, the disc's stat range: identical for the originals' stats, and
	   a stat past 999 (caps up to 9999) cannot push x past half the accuracy, so the hit chance never reaches 0 or 100 on speed alone */
	speedDiff = defender->stats.base.speed - BMD(attacker->stats.base.speed, HIT_SPEED_DIV);
	if (speedDiff > BM(HIT_SPEED_CLAMP)) {
		speedDiff = BM(HIT_SPEED_CLAMP);
	}
	if (speedDiff < -BM(HIT_SPEED_CLAMP)) {
		speedDiff = -BM(HIT_SPEED_CLAMP);
	}
	x = BMD(BMD(MOVE_DATA[move].accuracy, HIT_ACC_DIV) * speedDiff, HIT_SPEED_NORM);
	if ((defender->entity.anim.animId == 0x21) || (defender->entity.anim.animId == 0x22)) {
		x = x * BM(HIT_MOVING_PCT) / 100;
	}

	result = MOVE_DATA[move].accuracy - x;
	if (result < 0) {
		result = 0;
	}

	if (result > 100) {
		result = 100;
	}

	return result;
}

void BTL_retargetAfterHit(DigimonEntity *digimon, FighterData *fighter, AttackObject attack)
{
	if (digimon == (DigimonEntity *)ENTITY_TABLE[1]) {
		if (((TARGET_ATTACKER_CHANCE[digimon->stats.current.chargeMode] - PARTNER_PARA.discipline) <= randomLimit(100)) && (BTL_isFighterDefeated(fighter->targetId) == 0)) {
			return;
		}
		BTL_selectFighterTarget(digimon, fighter, BTL_getFighterSlot(attack.casterId), 1);
	} else {
		BTL_selectFighterTarget(digimon, fighter, BTL_getFighterSlot(attack.casterId), 1);
	}
}

int32_t BTL_calculateDamage(DigimonEntity *attacker, DigimonEntity *defender, int16_t move)
{
	int32_t diff;
	uint8_t mult[3];
	long i;
	int32_t dmg;
	int16_t off;
	int16_t def;

	for (i = 0; i < 3; i++) {
		if (DIGI_PARA(defender->entity.type).special[i] != 0xff) {
			mult[i] = TYPE_FACTORS[modSpeciesMove(attacker->entity.type, move)->special][DIGI_PARA(defender->entity.type).special[i]];
		} else {
			mult[i] = BM(NEUTRAL_MULT);
		}
	}
	off = attacker->stats.base.off;
	def = defender->stats.base.def;
	if (move == 0x2d) {
		def = def * BM(GUARD_PCT) / 100; /* MOD (battle as data) */
	}
	if ((move >= 0x3a) && (move < 0x71)) {
		dmg = BMD((off + modSpeciesMove(attacker->entity.type, move)->power) * (mult[0] + mult[1] + mult[2]), DMG_DIV);
	} else {
		diff = off - def;
		dmg = BMD((mult[0] + mult[1] + mult[2]) * modBtStatPower(modSpeciesMove(attacker->entity.type, move)->power, diff), DMG_DIV) * (randomLimit(BMS(RAND_SPAN)) + BM(RAND_MIN)) / 100;
	}
	if ((move >= 0x3a) && (move < 0x71)) {
		if ((Entity *)attacker == ENTITY_TABLE[1]) {
			if (COMBAT_DATA_PTR->player.finisherChargeup[0] >= BM(CHARGE_FROM)) {
				dmg = BMD(dmg * COMBAT_DATA_PTR->player.finisherChargeup[0], CHARGE_DIV);
			}
		} else {
			dmg = dmg * (randomLimit(BMS(ENEMY_FIN_SPAN)) + BM(ENEMY_FIN_MIN)) / 100;
		}
		dmg = dmg * (randomLimit(BMS(RAND_SPAN)) + BM(RAND_MIN)) / 100;
	}
	dmg = MOD_DIFF_DAMAGE(attacker, defender, ENTITY_TABLE[1], dmg); /* MOD (difficulty): EASY, x2 dealt, /2 taken */
	if (dmg <= BM(DMG_MIN) - 1) {
		dmg = BM(DMG_MIN);
	}
	if (dmg > DAMAGE_CAP) { /* MOD (caps): was 9999 */
		dmg = DAMAGE_CAP;
	}
	return dmg;
}

int16_t BTL_getFighterSlot(int16_t entityId)
{
	int32_t i;

	for (i = 0; ENEMY_COUNT >= i; i++) {
		if (COMBAT_DATA_PTR->player.entityIds[i] == entityId) {
			return i;
		}
	}

	return -1;
}

int32_t BTL_addBlockedAttack(FighterData *fighter, FighterData *other)
{
	int32_t i;

	if (0) {
		i = 0;
	}
	for (i = 0; i < 0x95; i++) {
		if (fighter->table1[i] == -1) {
			break;
		}
		if ((fighter->table1[i] == other->effectSlot[3]) && (fighter->table2[i] == other->activeEffectSlot)) {
			return 0;
		}
	}

	fighter->table1[i] = other->effectSlot[3];
	fighter->table2[i] = other->activeEffectSlot;

	return 1;
}

void BTL_startHitAnimation(Entity *entity, AttackObject *attack, uint8_t animId)
{
	int16_t tech;

	tech = BTL_getAttackTech(attack);
	startAnimation(entity, animId);
	createParticleFX(modSpeciesMove(ENTITY_TABLE[attack->casterId]->type, tech)->special, 1, &attack->position, entity, MOVE_DATA[tech].iframes + 0x10);
}

void BTL_tickAttackHits(void)
{
	long j;
	long i;
	int32_t dmg;
	int32_t hit;
	int32_t tech;
	uint8_t *moves;
	VECTOR loc;
	FighterData *first;
	PlayerDataSub *sub;
	AttackObject attack;
	Entity *attacker;
	int32_t handled;
	FighterData *fighter;
	Entity *entity;

	fighter = COMBAT_DATA_PTR->fighter;
	first = fighter;
	sub = &COMBAT_DATA_PTR->player.unk1[0];
	for (i = 0; i <= ENEMY_COUNT; i++, fighter++, sub++) {
		if (fighter->flags & FIGHTER_FLAG_DEAD) {
			continue;
		}
		if (popAttackObject(COMBAT_DATA_PTR->player.entityIds[i], &attack) == 0) {
			continue;
		}
		if (FLEE_TIMER >= BM(FLEE_IGNORE)) {
			continue;
		}
		entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		tech = BTL_getAttackTech(&attack);
		if (DIGI_PARA(entity->type).moves[(uint32_t)(entity->anim.animId - 0x2e)] == 0x2d) {
			if (COMBAT_DATA_PTR->player.entityIds[fighter->targetId] == attack.casterId) {
				((DigimonEntity *)entity)->stats.current.isHit = 0;
				continue;
			}
		}
		if (BTL_applyBuffMove((DigimonEntity *)entity, (int16_t)i, tech) != 0) {
			((DigimonEntity *)entity)->stats.current.isHit = 0;
			continue;
		}
		BTL_removeMoveEffect((DigimonEntity *)entity, fighter);
		fighter->flags &= ~(FIGHTER_FLAG_KNOCKED_BACK | FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_TRANSFORMING);
		attacker = ENTITY_TABLE[attack.casterId];
		hit = BTL_calculateHitChance((DigimonEntity *)attacker, (DigimonEntity *)entity, fighter, tech);
		if (entity == FINISHING_ENTITY) {
			if (i == 0) {
				BTL_removeFinisherChargeup();
			}
			if (BTL_FINISHER_AURA_ID != -1) {
				BTL_removeFinisherAura(BTL_FINISHER_AURA_ID);
			}
			NO_AI_FLAG = 0;
			FINISHING_ENTITY = NULL;
		}
		if (randomLimit(100) < hit) {
			BTL_retargetAfterHit((DigimonEntity *)entity, fighter, attack);
			dmg = BTL_calculateDamage((DigimonEntity *)attacker, (DigimonEntity *)entity, tech);
			HAS_TAKEN_DAMAGE = 1;
			if (i == 0) {
				if ((dmg * 100 / PARTNER_ENTITY.digimonEntity.stats.base.hp) >= BM(BIGHIT_PCT)) {
					COMBAT_DATA_PTR->player.unk2++;
				}
			}
			/* MOD (caps): summed in 32 bits, up to DAMAGE_CAP (was 9999) */
			fighter->hpDamageBuffer = CAPS_ADD_DAMAGE(fighter->hpDamageBuffer, dmg);
			fighter->flags |= FIGHTER_FLAG_KNOCKED_BACK;
			BTL_handleHitReaction(entity, fighter, &attack, i);
			addEntityText(entity, i, 0, dmg, 0);
			fighter->invulnerableTimer = MOVE_DATA[tech].iframes;
			entity->anim.animFlag &= 0xfe;
			BTL_applyMoveStatus((DigimonEntity *)entity, fighter, tech);
			continue;
		}
		handled = 0;
		if (!(fighter->flags & FIGHTER_FLAG_BLOCKING) && (MOVE_DATA[tech].range == 1) && (DIGI_PARA(entity->type).moves[(uint32_t)(entity->anim.animId - 0x2e)] != 0x2d)) {
			moves = ((DigimonEntity *)entity)->stats.base.moves;
			for (j = 0; j < 4; j++) {
				if (((DigimonEntity *)entity)->stats.current.currentMP < BM(BLOCK_MIN_MP)) {
					break;
				}
				if ((moves[j] != 0xff) && (DIGI_PARA(entity->type).moves[moves[j] - 0x2e] == 0x2d)) {
					if (randomLimit(100) < (((DigimonEntity *)entity)->stats.base.speed / DIGI_PARA(entity->type).level)) {
						fighter->queuedAnim = moves[j];
						fighter->targetId = BTL_getFighterSlot(attack.casterId);
						fighter->moveRange = 1;
						BTL_startQueuedMove((DigimonEntity *)entity, (DigimonEntity *)attacker, fighter);
						attacker->anim.animFlag &= 0xfe;
						handled = 1;
						((DigimonEntity *)entity)->stats.current.isHit = 0;
						break;
					}
				}
			}
		}
		if (handled != 0) {
			continue;
		}
		if (BTL_addBlockedAttack(fighter, (FighterData *)&attack) != 0) {
			createParticleFX(0, 2, &attack.position, entity, 0x11);
		}
		if ((fighter->flags & FIGHTER_FLAG_BLOCKING) && (fighter->invulnerableTimer > 0)) {
			goto blocked;
		}
		if (MOVE_DATA[tech].range == 1) {
			dmg = BTL_calculateDamage((DigimonEntity *)attacker, (DigimonEntity *)entity, tech);
			dmg = dmg * (randomLimit(BMS(BLOCK_DMG_SPAN)) + BM(BLOCK_DMG_MIN)) / 100;
			if (dmg <= 0) {
				dmg = 1;
			}
			/* MOD (caps): summed in 32 bits, up to DAMAGE_CAP (was 9999) */
			fighter->hpDamageBuffer = CAPS_ADD_DAMAGE(fighter->hpDamageBuffer, dmg);
			addEntityText(entity, i, 0, dmg, 0);
		}
		BTL_addFinisherProgress(fighter, fighter->finisherGoal * BM(BLOCK_GAIN_DEF) / 100);
		for (j = 0; ENEMY_COUNT >= j; j++) {
			if (attacker == ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[j]]) {
				BTL_addFinisherProgress(&COMBAT_DATA_PTR->fighter[j], COMBAT_DATA_PTR->fighter[j].finisherGoal * BM(BLOCK_GAIN_ATK) / 100);
				break;
			}
		}
		if (i == 0) {
			COMBAT_DATA_PTR->player.blockedCount++;
		}
		loc.vx = attack.position.vx;
		loc.vy = 0;
		loc.vz = attack.position.vz;
		entityLookAtLocation(entity, &loc);
		fighter->flags |= FIGHTER_FLAG_BLOCKING;
		startAnimation(entity, 0x25);
		entity->anim.animFlag &= 0xfe;
blocked:
		fighter->invulnerableTimer = BM(BLOCK_INVULN);
		((DigimonEntity *)entity)->stats.current.isHit = 0;
	}
}

// clang-format off
void BTL_buffStats(digimon, slot, value, stat, color, flag)
	DigimonEntity *digimon;
	int16_t slot;
	int16_t value;
	int16_t *stat;
	int16_t color;
	uint8_t flag;
// clang-format on
{
	addWithLimit(stat, value, STAT_CAP); /* MOD (caps): was 999 */
	addEntityText(&digimon->entity, slot, color, value, flag);
}

#include <dw/enemy_bars.h>

void BTL_renderEnemyHPBars(void)
{
	int32_t i;
	for (i = 1; i <= ENEMY_COUNT; i++) {
		FighterData *fighter = &COMBAT_DATA_PTR->fighter[i];
		DigimonEntity *d = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		/* HP animation remains gameplay state, even while the HUD is hidden. */
		if (d->stats.current.currentHP <= 0) fighter->hpDamageBuffer = 0;
		if (fighter->hpDamageBuffer) damageTick(fighter, &d->stats);
		renderEnemyHeadHP(d, fighter, 1);
	}
}

int32_t BTL_isMoveUsable(DigimonEntity *digimon, FighterData *fighter, int16_t slot)
{
	uint8_t move;
	int16_t tech;
	int16_t mp;

	move = digimon->stats.base.moves[slot];
	if (move == 0xff) {
		return 0;
	}

	tech = entityGetTechFromAnim(&digimon->entity, move);
	if (tech == 0x2d) {
		return 0;
	}

	if ((tech >= 0x3a) && (tech < 0x71)) {
		return 0;
	}

	if ((MOVE_DATA[tech].range == 4) && (fighter->buffsRemaining == 0)) {
		return 0;
	}

	if ((fighter->targetId == 0xff) && ((MOVE_DATA[tech].unk3 & 1) == 1)) {
		return 0;
	}

	mp = modSpeciesMove(digimon->entity.type, tech)->mpCost * BM(MP_MULT); /* MOD (battle as data) */
	if (digimon == (DigimonEntity *)ENTITY_TABLE[1]) {
		mp -= mp * BMBRAIN(PARTNER_ENTITY.digimonEntity.stats.base.brain) / 100;
		if (PARTNER_PARA.condition & 0x60) {
			mp += mp * BM(MP_COND_PCT) / 100;
		}
	}

	if (digimon->stats.current.currentMP >= mp) {
		return 1;
	}

	return 0;
}

int32_t BTL_getDistanceSquared(Entity *a, Entity *b)
{
	VECTOR d;
	VECTOR *pa;
	VECTOR *pb;

	pa = &a->posData->location;
	pb = &b->posData->location;
	d.vx = pa->vx - pb->vx;
	d.vy = 0;
	d.vz = pa->vz - pb->vz;

	return (d.vx * d.vx) + (d.vz * d.vz);
}

void BTL_setupQueuedMove(DigimonEntity *digimon, FighterData *fighter, int16_t arg2, uint8_t moveIndex)
{
	int16_t tech;
	Stats *stats;

	fighter->unk15 = 0;
	stats = &digimon->stats;
	fighter->queuedAnim = stats->base.moves[moveIndex];
	tech = entityGetTechFromAnim(&digimon->entity, fighter->queuedAnim);
	fighter->moveRange = MOVE_DATA[tech].range;
	BTL_applyChargeRequirement(digimon, fighter, tech);
	fighter->flags |= FIGHTER_FLAG_TRANSFORMING;
}

void BTL_applyChargeRequirement(DigimonEntity *digimon, FighterData *fighter, int16_t tech)
{
	switch (digimon->stats.current.chargeMode) {
	case 0:
		if (fighter->speedBuffer <= 0) {
			fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
		}
		break;
	case 1:
		if ((fighter->speedBuffer < BM(SPEED_MAX)) && (fighter->speedBuffer < modSpeciesMove(digimon->entity.type, tech)->power)) {
			fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
		}
		break;
	case 2:
		if (fighter->speedBuffer < BM(SPEED_MAX)) {
			fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
		}
		break;
	}
}

void BTL_startQueuedMove(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter)
{
	int16_t tech;

	if (MOD_BATTLE_LOADING) {
		return; /* MOD (quick battle start): the techniques' effects are still loading; asked again next tick */
	}

	if (NO_AI_FLAG != 0) {
		if (FINISHING_ENTITY != &digimon->entity) {
			return;
		}
		if (BTL_FINISHER_TIMER > 0) {
			BTL_FINISHER_TIMER--;
			entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
			return;
		}
	} else {
		tech = entityGetTechFromAnim(&digimon->entity, fighter->queuedAnim);
		if ((tech >= 0x3a) && (tech < 0x71)) {
			NO_AI_FLAG = 1;
		}
		if (NO_AI_FLAG != 0) {
			FINISHING_ENTITY = &digimon->entity;
			if (digimon == (DigimonEntity *)ENTITY_TABLE[1]) {
				BTL_initializeFinisherChargeup(tech);
			}
			entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
			startAnimation(&digimon->entity, fighter->queuedAnim);
			digimon->entity.anim.animFlag &= 0xfe;
			BTL_FINISHER_AURA_ID = BTL_addFinisherAura(&digimon->entity, BM(FIN_HOLD));
			BTL_FINISHER_TIMER = BM(FIN_HOLD);
			return;
		}
	}

	if (BTL_isMoveOnCooldown(&digimon->entity, fighter) != 0) {
		return;
	}

	if (target != NULL) {
		if (target->entity.anim.animId == 0x28) {
			return;
		}
		if (target->entity.anim.animId == 0x29) {
			return;
		}
		if (digimon != target) {
			entityLookAtLocation(&digimon->entity, &target->entity.posData->location);
		}
	}

	if (MOVE_DATA[tech].unk3 & 2) {
		FLEE_DISABLED[1] = BM(FLEE_LOCK);
	}

	startAnimation(&digimon->entity, fighter->queuedAnim);
	fighter->flags |= FIGHTER_FLAG_ATTACKING;
	if (!(fighter->flags & FIGHTER_FLAG_FLATTENED)) {
		BTL_setupMoveExecution(digimon, target, fighter);
	} else {
		fighter->flatAttackTimer = BM(T_FLAT_ATTACK);
	}
}

int32_t BTL_isMoveOnCooldown(Entity *entity, FighterData *fighter)
{
	int16_t tech;

	tech = entityGetTechFromAnim(entity, fighter->queuedAnim);
	if ((MOVE_DATA[tech].unk3 & 2) && (FLEE_DISABLED[1] > 0)) {
		return 1;
	}

	return 0;
}

void BTL_setupMoveExecution(DigimonEntity *digimon, DigimonEntity *target, FighterData *fighter)
{
	int32_t tech;
	int16_t cost;
	int16_t brain;
	int32_t i;

	BATTLE_ATTACKING_DIGIMON = digimon;
	tech = entityGetTechFromAnim(&digimon->entity, fighter->queuedAnim);
	if (fighter->moveRange != 4) {
		BATTLE_TARGETED_DIGIMON = target;
		if (target == NULL) {
			if (MOVE_DATA[tech].unk3 & 1) {
				if (digimon != (DigimonEntity *)ENTITY_TABLE[1]) {
					BATTLE_TARGETED_DIGIMON = (DigimonEntity *)ENTITY_TABLE[1];
					fighter->targetId = 0;
				} else {
					for (i = 1; ENEMY_COUNT >= i; i++) {
						if (BTL_isFighterDefeated(i) != 1) {
							fighter->targetId = i;
							BATTLE_TARGETED_DIGIMON = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
						}
					}
				}
			}
		}
	} else {
		BATTLE_TARGETED_DIGIMON = digimon;
	}

	BTL_removeMoveEffect(digimon, fighter);
	if (digimon == (DigimonEntity *)ENTITY_TABLE[1]) {
		if (!((tech >= 0x3a) && (tech < 0x71))) {
			cost = modSpeciesMove(digimon->entity.type, tech)->mpCost * BM(MP_MULT);
			if (PARTNER_PARA.condition & 0x60) {
				cost += cost * BM(MP_COND_PCT) / 100;
			}
			brain = digimon->stats.base.brain;
			cost = cost * (100 - BMBRAIN(brain)) / 100;
			fighter->mpDamageBuffer += cost;
		}
	} else {
		if (!((tech >= 0x3a) && (tech < 0x71))) {
			if (FLEE_DISABLED[0] == 0) {
				digimon->stats.current.currentMP -= modSpeciesMove(digimon->entity.type, tech)->mpCost * BM(MP_MULT);
			}
		}
		if (tech < 0x3a) {
			for (i = 0; i < 0xc; i++) {
				if (COMBAT_DATA_PTR->player.usedMoves[i] == tech) {
					break;
				}
				if (COMBAT_DATA_PTR->player.usedMoves[i] == 0xff) {
					COMBAT_DATA_PTR->player.usedMoves[i] = tech;
					break;
				}
			}
		}
	}

	digimon->stats.current.unk1 = tech + 0x100;
	for (i = 0; i < 3; i++) {
		if (fighter->queuedAnim == digimon->stats.base.moves[i]) {
			break;
		}
	}

	if (i != 4) {
		digimon->stats.current.efeSubEffect = BTL_startEFE(fighter->effectSlot[i]);
		fighter->activeEffectSlot = fighter->effectSlot[i];
	}

	if ((MOVE_DATA[tech].range == 4) && (fighter->buffsRemaining != 0)) {
		fighter->buffsRemaining--;
	}

	fighter->speedBuffer -= modSpeciesMove(digimon->entity.type, tech)->power;
	if (fighter->speedBuffer < -BM(SPEED_FLOOR)) {
		fighter->speedBuffer = -BM(SPEED_FLOOR);
	}
}

void BTL_removeMoveEffect(DigimonEntity *digimon, FighterData *fighter)
{
	if (fighter->activeEffectSlot != -1L) {
		BTL_stopEFESubEffect(fighter->activeEffectSlot, digimon->stats.current.efeSubEffect);
	}
	digimon->stats.current.efeSubEffect = -1;
	fighter->activeEffectSlot = -1;
}

/* MOD (fps): battle state that render callbacks advance, rolled back around
 * the extra render passes of a tick (dw/mod_present.h): every pass starts from
 * the values the first pass started from, and the first pass's results are
 * put back afterwards, so the battle sees exactly what one render per tick
 * would have left and every pass draws the same picture.
 * - COMBAT_DATA and the fighters' stats: the HP bars drain hpDamageBuffer
 *   into currentHP (damageTick) and apply mpDamageBuffer; the enemy bars move
 *   their sprites and count their display timer (player.unk1);
 * - the "BATTLE" start text pieces, its colour timer and BTL_BATTLE_TEXT_FINISHED
 *   (the intro waits on it);
 * - the finisher gauge pulse/full counters and BTL_FINISHER_READY;
 * - the command shout's frame. */
extern uint8_t BTL_BATTLE_START_TEXT_TIMER[4];
extern int32_t BTL_BATTLE_TEXT_FINISHED;
extern uint8_t BTL_FINISHER_FULL_FRAMES;
extern uint8_t BTL_FINISHER_PULSE_FRAME;
extern uint8_t BTL_FINISHER_BRIGHTNESS;
extern uint8_t BTL_FINISHER_SEGMENTS;
extern uint8_t BTL_FINISHER_READY;
extern int8_t BTL_START_TEXT_ACTIVE;
extern BtlCommandShout BTL_COMMAND_SHOUT;

#define BTL_MOD_ROLLBACK_MAX 16
#define BTL_MOD_ROLLBACK_WORDS 1400
static ModRollbackVar BTL_MOD_ROLLBACK_VARS[BTL_MOD_ROLLBACK_MAX];
static uint32_t BTL_MOD_ROLLBACK_PRE[BTL_MOD_ROLLBACK_WORDS];
static uint32_t BTL_MOD_ROLLBACK_POST[BTL_MOD_ROLLBACK_WORDS];
static ModRollbackSet BTL_MOD_ROLLBACK = {
	BTL_MOD_ROLLBACK_VARS, 0, BTL_MOD_ROLLBACK_PRE, BTL_MOD_ROLLBACK_POST
};

static void BTL_modRollbackAdd(void *ptr, int32_t size)
{
	ModRollbackVar *v = &BTL_MOD_ROLLBACK_VARS[BTL_MOD_ROLLBACK.count++];

	v->ptr = ptr;
	v->size = size;
}

static ModRollbackSet *BTL_modRollbackSet(void)
{
	int32_t i;
	Entity *e;

	BTL_MOD_ROLLBACK.count = 0;
	BTL_modRollbackAdd(COMBAT_DATA_PTR, sizeof(CombatData));
	BTL_modRollbackAdd(&PARTNER_ENTITY.digimonEntity.stats, sizeof(Stats));
	for (i = 1; i <= ENEMY_COUNT && i < 4; i++) {
		e = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		if (e != NULL && e != &PARTNER_ENTITY.digimonEntity.entity) {
			BTL_modRollbackAdd(&((DigimonEntity *)e)->stats, sizeof(Stats));
		}
	}
	if (BTL_START_TEXT_ACTIVE) {
		BTL_modRollbackAdd(BTL_BATTLE_START_TEXT_PIECES, sizeof(BTL_BATTLE_START_TEXT_PIECES));
	}
	BTL_modRollbackAdd(BTL_BATTLE_START_TEXT_TIMER, sizeof(BTL_BATTLE_START_TEXT_TIMER));
	BTL_modRollbackAdd(&BTL_BATTLE_TEXT_FINISHED, sizeof(BTL_BATTLE_TEXT_FINISHED));
	BTL_modRollbackAdd(&BTL_FINISHER_FULL_FRAMES, 1);
	BTL_modRollbackAdd(&BTL_FINISHER_PULSE_FRAME, 1);
	BTL_modRollbackAdd(&BTL_FINISHER_BRIGHTNESS, 1);
	BTL_modRollbackAdd(&BTL_FINISHER_SEGMENTS, 1);
	BTL_modRollbackAdd(&BTL_FINISHER_READY, 1);
	BTL_modRollbackAdd(&BTL_COMMAND_SHOUT, sizeof(BTL_COMMAND_SHOUT));
	return &BTL_MOD_ROLLBACK;
}

void BTL_battleTickFrame(void)
{
	/* MOD (fps): rendered by the shared tick / present helper (main.c);
	 * the tick started with the AI in BTL_battleMain */
	BTL_modSpeedBeforeTick(); /* MOD (battle speed) */
	modTickStart(MOD_PRESENT_BATTLE);
	POLLED_INPUT = PadRead(1);
	modSpeedInput(); /* MOD (battle speed, mod_qol.c) */
	ACTIVE_FRAMEBUFFER = GsGetActiveBuff();
	ACTIVE_ORDERING_TABLE = &GS_ORDERING_TABLE[ACTIVE_FRAMEBUFFER];
	GsSetWorkBase(GS_WORK_BASES[ACTIVE_FRAMEBUFFER]);
	GsClearOt(0, 0, ACTIVE_ORDERING_TABLE);
	tickObjects();
	modPresent(MOD_PRESENT_BATTLE, BTL_modRollbackSet());
	POLLED_INPUT_PREVIOUS = POLLED_INPUT;
}

void BTL_addFinisherProgress(FighterData *fighter, int16_t amount)
{
	int16_t tech;
	int32_t i;

	for (i = 0; ENEMY_COUNT >= i; i++) {
		if (&COMBAT_DATA_PTR->fighter[i] == fighter) {
			tech = entityGetTechFromAnim(ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]], ((DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]])->stats.base.moves[3]);
			if ((tech < 0x3a) || (tech >= 0x71)) {
				return;
			}
		}
	}

	fighter->finisherProgress += amount;
	if (fighter->finisherProgress > fighter->finisherGoal) {
		fighter->finisherProgress = fighter->finisherGoal;
	}
}

void BTL_clearConfusion(DigimonEntity *digimon, FighterData *fighter)
{
	BTL_resetFighterAction(fighter);
	BTL_selectFighterTarget(digimon, fighter, fighter->targetId, 0);
	fighter->flags &= ~FIGHTER_FLAG_CONFUSED;
	fighter->confusionTimer = 0;
	if (!(fighter->flags & (FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED)) && (fighter->flatTimer == 0)) {
		fighter->flags &= ~FIGHTER_FLAG_TRANSFORMING;
		BTL_removeStatusEffectVisual(digimon, fighter, 2);
		if (fighter->flags & FIGHTER_FLAG_POISONED) {
			BTL_addStatusEffectVisual(digimon, fighter, 1);
		}
	}
}

void BTL_clearStun(DigimonEntity *digimon, FighterData *fighter)
{
	BTL_resetFighterAction(fighter);
	digimon->entity.anim.animFlag |= 1;
	fighter->flags &= ~FIGHTER_FLAG_STUNNED;
	fighter->stunTimer = 0;
	if (fighter->flags & FIGHTER_FLAG_FLATTENED) {
		return;
	}

	if (fighter->flatTimer != 0) {
		return;
	}

	BTL_removeStatusEffectVisual(digimon, fighter, 3);
	if (fighter->flags & FIGHTER_FLAG_CONFUSED) {
		BTL_addStatusEffectVisual(digimon, fighter, 2);
	}

	if (fighter->flags & FIGHTER_FLAG_POISONED) {
		BTL_addStatusEffectVisual(digimon, fighter, 1);
	}
}

void BTL_applyFlattenScale(VECTOR *scale, int32_t t)
{
	if (t < 0x40) {
		if (t >= 0x30) {
			BTL_applyStretchScale(scale, (0x40 - t) << 4);
		} else if (t >= 0x20) {
			BTL_applySquashScale(scale, (0x30 - t) << 4);
		} else if (t >= 0x18) {
			BTL_applyStretchScale(scale, (0x20 - t) << 5);
		} else if (t >= 0x10) {
			BTL_applySquashScale(scale, (0x18 - t) << 5);
		} else if (t >= 0xc) {
			BTL_applyStretchScale(scale, (0x10 - t) << 6);
		} else if (t >= 8) {
			BTL_applySquashScale(scale, (0xc - t) << 6);
		} else if (t >= 6) {
			BTL_applyStretchScale(scale, (8 - t) << 7);
		} else if (t >= 4) {
			scale->vz = scale->vy = scale->vx = 0x1000 - ((6 - t) << 11);
		} else if (0 <= t) {
			scale->vz = scale->vy = scale->vx = (4 - t) << 10;
		}
	}
}

void BTL_addStatusEffectVisual(DigimonEntity *digimon, FighterData *fighter, uint8_t kind)
{
	if (fighter->statusFxId == -1) {
		switch (kind) {
		case 1:
			fighter->statusFxId = BTL_addPoisonEffect(&digimon->entity);
			break;
		case 2:
			fighter->statusFxId = BTL_addConfusionEffect(&digimon->entity);
			break;
		case 3:
			fighter->statusFxId = BTL_addStunEffect(&digimon->entity, fighter->stunTimer);
			break;
		}
	}
}

void BTL_applyStretchScale(VECTOR *scale, int32_t angle)
{
	scale->vy = _sin(angle) + 0x1000;
	scale->vz = scale->vx = 0x1000 - (_sin(angle) / 2);
}

void BTL_applySquashScale(VECTOR *scale, int32_t angle)
{
	scale->vy = 0x1000 - (_sin(angle) / 2);
	scale->vz = scale->vx = (_sin(angle) / 2) + 0x1000;
}

void BTL_removeStatusEffectVisual(DigimonEntity *digimon, FighterData *fighter, uint8_t kind)
{
	if (fighter->statusFxId != -1L) {
		switch (kind) {
		case 1:
			BTL_removePoisonEffect(fighter->statusFxId, &digimon->entity);
			break;
		case 2:
			BTL_removeConfusionEffect(fighter->statusFxId, &digimon->entity);
			break;
		case 3:
			BTL_removeStunEffect(fighter->statusFxId, &digimon->entity);
			break;
		}
		fighter->statusFxId = -1;
	}
}

void BTL_clearPoison(DigimonEntity *digimon, FighterData *fighter)
{
	fighter->flags &= ~FIGHTER_FLAG_POISONED;
	fighter->poisonTimer = 0;
	if (!(fighter->flags & (FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED)) && (fighter->flatTimer == 0)) {
		BTL_removeStatusEffectVisual(digimon, fighter, 1);
	}
}

void BTL_healFlatten(Entity *entity, FighterData *fighter, int32_t arg2)
{
	if (fighter->flatTimer >= 0x42) {
		fighter->flatTimer = 0x41;
	}

	if (fighter->flatTimer != 0) {
		if (!(fighter->flags & FIGHTER_FLAG_FLATTENED)) {
			fighter->flags |= FIGHTER_FLAG_FLATTENED;
			if (fighter->flatTimer >= 4) {
				entity->flatSprite = 0;
			}
			fighter->flatTimer = 0x41;
		}
	}
}

void BTL_tickStatusEffects(void)
{
	long i;
	int32_t dmg;
	CombatData *combat;
	PlayerDataSub *sub;
	Entity *entity;
	Stats *stats;
	FighterData *fighter;

	combat = COMBAT_DATA_PTR;
	sub = COMBAT_DATA_PTR->player.unk1;
	i = 0;

	for (; i <= ENEMY_COUNT; i++, sub++) {
		fighter = &combat->fighter[i];
		if (fighter->flags & FIGHTER_FLAG_DEAD) {
			continue;
		}
		entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		stats = &((DigimonEntity *)entity)->stats;
		if (fighter->flags & FIGHTER_FLAG_POISONED) {
			if (NO_AI_FLAG == 0) {
				fighter->poisonTimer--;
			}
			if (fighter->poisonTimer == 0) {
				fighter->poisonTimer = BM(POISON_FRAMES);
				dmg = stats->base.hp * (randomLimit(BMS(POISON_SPAN)) + BM(POISON_MIN)) / 100;
				dmg = MOD_DIFF_DAMAGE(NULL, entity, ENTITY_TABLE[1], dmg); /* MOD (difficulty) */
				/* MOD (caps): summed in 32 bits, up to DAMAGE_CAP (was 9999) */
				fighter->hpDamageBuffer = CAPS_ADD_DAMAGE(fighter->hpDamageBuffer, dmg);
				addEntityText(entity, i, 0xc, dmg, 0);
			}
		}
		if (fighter->flags & FIGHTER_FLAG_CONFUSED) {
			if ((NO_AI_FLAG == 0) && (fighter->confusionTimer != 0)) {
				fighter->confusionTimer--;
			}
			if ((fighter->confusionTimer == 0) && !(combat->fighter[0].flags & FIGHTER_FLAG_ATTACKING)) {
				BTL_clearConfusion((DigimonEntity *)entity, fighter);
			}
		}
		if (fighter->flags & FIGHTER_FLAG_STUNNED) {
			if (NO_AI_FLAG == 0) {
				fighter->stunTimer--;
			}
			if (fighter->stunTimer == 0) {
				BTL_clearStun((DigimonEntity *)entity, fighter);
			}
		}
		if (fighter->flatTimer <= 0) {
			continue;
		}
		if ((NO_AI_FLAG == 0) || (fighter->flatTimer < 0x42)) {
			fighter->flatTimer--;
		}
		BTL_applyFlattenScale(&entity->posData->scale, fighter->flatTimer);
		if (fighter->flags & FIGHTER_FLAG_FLATTENED) {
			switch (fighter->flatTimer) {
			case 0x40:
				if ((fighter->flags & FIGHTER_FLAG_KNOCKED_BACK) || (fighter->flags & FIGHTER_FLAG_ATTACKING)) {
					fighter->flatTimer++;
				} else {
					startAnimation(entity, 0x22);
					fighter->moveRange = -1;
					fighter->flags |= FIGHTER_FLAG_TRANSFORMING;
					stats->current.isHit = 1;
				}
				break;
			case 3:
				entity->flatSprite = -1;
				break;
			case 0:
				fighter->flags &= ~(FIGHTER_FLAG_FLATTENED | FIGHTER_FLAG_TRANSFORMING);
				stats->current.isHit = 0;
				if (fighter->flags & FIGHTER_FLAG_STUNNED) {
					BTL_addStatusEffectVisual((DigimonEntity *)entity, fighter, 3);
					fighter->moveRange = 0;
				}
				if (fighter->flags & FIGHTER_FLAG_CONFUSED) {
					BTL_addStatusEffectVisual((DigimonEntity *)entity, fighter, 2);
				}
				if (fighter->flags & FIGHTER_FLAG_POISONED) {
					BTL_addStatusEffectVisual((DigimonEntity *)entity, fighter, 1);
				}
				fighter->moveRange = 0;
				BTL_resetFighterAction(fighter);
				if (modModelScale(entity->type) != 0x1000) { /* MOD (species rules as data): a species with its own scale (Meteormon) */
					entity->posData->scale.vx = entity->posData->scale.vy = entity->posData->scale.vz = modModelScale(entity->type);
				}
				/* The squash uses the tired idle even at full HP. Once
				 * restored, choose the fighter's own healthy/tired stance. */
				if ((entity->anim.animId == 0x22) &&
				    !(fighter->flags & (FIGHTER_FLAG_ATTACKING | FIGHTER_FLAG_KNOCKED_BACK |
				                        FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_CONFUSED))) {
					startBattleIdleAnimation((DigimonEntity *)entity, stats, fighter->flags);
				}
				break;
			}
		} else {
			switch (fighter->flatTimer) {
			case 0x40:
				startAnimation(entity, 0x22);
				fighter->moveRange = -1;
				fighter->flags |= FIGHTER_FLAG_TRANSFORMING;
				stats->current.isHit = 1;
				break;
			case 3:
				entity->flatSprite = 0;
				break;
			case 0:
				fighter->flags |= FIGHTER_FLAG_FLATTENED;
				fighter->flatTimer = randomLimit(BMS(FLAT_SPAN)) + BM(FLAT_BASE);
				stats->current.isHit = 0;
				fighter->flags &= ~FIGHTER_FLAG_TRANSFORMING;
				break;
			}
		}
	}
}

void BTL_resetFighterAction(FighterData *fighter)
{
	fighter->cooldown = 0;
	fighter->senileTimer = 0;
	fighter->flags &= ~(FIGHTER_FLAG_ON_CHARGEUP | FIGHTER_FLAG_ON_COOLDOWN | FIGHTER_FLAG_SENILE);
}

void BTL_addPoisonStatusVisual(DigimonEntity *digimon, FighterData *fighter)
{
	if (!(fighter->flags & (FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED)) && (fighter->flatTimer == 0)) {
		BTL_addStatusEffectVisual(digimon, fighter, 1);
	}
}

void BTL_addConfusionStatusVisual(DigimonEntity *digimon, FighterData *fighter)
{
	if (!(fighter->flags & (FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED)) && (fighter->flatTimer == 0)) {
		if (fighter->flags & FIGHTER_FLAG_POISONED) {
			BTL_removeStatusEffectVisual(digimon, fighter, 1);
		}
		BTL_addStatusEffectVisual(digimon, fighter, 2);
	}
}

void BTL_addStunStatusVisual(DigimonEntity *digimon, FighterData *fighter)
{
	if (!(fighter->flags & FIGHTER_FLAG_FLATTENED) && (fighter->flatTimer == 0)) {
		if (fighter->flags & FIGHTER_FLAG_CONFUSED) {
			BTL_removeStatusEffectVisual(digimon, fighter, 2);
		}
		if (fighter->flags & FIGHTER_FLAG_POISONED) {
			BTL_removeStatusEffectVisual(digimon, fighter, 1);
		}
		BTL_addStatusEffectVisual(digimon, fighter, 3);
	}
}

void BTL_removeStatusEffects(DigimonEntity *digimon, FighterData *fighter)
{
	if (fighter->flags & FIGHTER_FLAG_STUNNED) {
		BTL_removeStatusEffectVisual(digimon, fighter, 3);
	}

	if (fighter->flags & FIGHTER_FLAG_CONFUSED) {
		BTL_removeStatusEffectVisual(digimon, fighter, 2);
	}

	if (fighter->flags & FIGHTER_FLAG_POISONED) {
		BTL_removeStatusEffectVisual(digimon, fighter, 1);
	}
}

void BTL_healStatusEffect(int32_t arg0)
{
	FighterData *fighter;
	uint16_t *flags;
	int16_t kind;

	fighter = COMBAT_DATA_PTR->fighter;
	flags = &fighter->flags;
	if (*flags & FIGHTER_FLAG_POISONED) {
		kind = 0;
	}

	if (*flags & FIGHTER_FLAG_CONFUSED) {
		kind = 2;
	}

	if (*flags & FIGHTER_FLAG_STUNNED) {
		kind = 1;
	}

	if (*flags & FIGHTER_FLAG_FLATTENED) {
		kind = 3;
	}

	if ((*flags & (FIGHTER_FLAG_POISONED | FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED)) && (arg0 == 1)) {
		BTL_addBuffRingsEffect(kind, ENTITY_TABLE[1]);
	}

	if (*flags & FIGHTER_FLAG_POISONED) {
		BTL_clearPoison((DigimonEntity *)ENTITY_TABLE[1], fighter);
	}

	if (*flags & FIGHTER_FLAG_CONFUSED) {
		BTL_clearConfusion((DigimonEntity *)ENTITY_TABLE[1], fighter);
	}

	if (*flags & FIGHTER_FLAG_STUNNED) {
		BTL_clearStun((DigimonEntity *)ENTITY_TABLE[1], fighter);
	}

	if (*flags & FIGHTER_FLAG_FLATTENED) {
		BTL_healFlatten(ENTITY_TABLE[1], fighter, 0);
	}
}

int32_t BTL_getUsableMoves(int16_t *out, int16_t index)
{
	DigimonEntity *digimon;
	FighterData *fighter;
	int32_t found;
	int32_t i;

	digimon = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]];
	fighter = &COMBAT_DATA_PTR->fighter[index];
	found = 0;

	for (i = 0; i < 4; ++i) {
		if (BTL_isMoveUsable(digimon, fighter, i) != 0) {
			out[i] = 1;
			found = 1;
		} else {
			out[i] = 0;
		}
	}

	return found;
}

void BTL_setFighterCooldown(DigimonEntity *digimon, FighterData *fighter)
{
	fighter->cooldown = BM(CD_IDLE);
	fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
}

int16_t BTL_getRandomUsableMove(int16_t *flags)
{
	int16_t picked[4];
	int32_t i;
	int32_t count;

	count = 0;
	for (i = 0; i < 4; i++) {
		if (flags[i] == 1) {
			picked[count++] = i;
		}
	}

	return picked[randomLimit(count)];
}

// clang-format off
int16_t BTL_getStrongestMove(index, flags)
	int16_t index;
	int16_t *flags;
// clang-format on
{
	int16_t score[3];
	int16_t best;
	int16_t tech;
	uint8_t *moves;
	DigimonEntity *digimon;
	int32_t i;

	digimon = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]];
	moves = digimon->stats.base.moves;
	for (i = 0; i < 3; i++) {
		if (flags[i] == 1) {
			tech = entityGetTechFromAnim(&digimon->entity, moves[i]);
			score[i] = modSpeciesMove(digimon->entity.type, tech)->power;
		} else {
			score[i] = -1;
		}
	}

	BTL_getHighestScoredMove(score, flags, &best, 3);

	return best;
}

// clang-format off
int16_t BTL_getMostEffectiveMove(index, flags)
	int16_t index;
	int16_t *flags;
// clang-format on
{
	uint8_t *moves;
	DigimonEntity *digimon;
	int16_t score[3];
	Entity *target;
	int16_t best;
	int16_t tech;
	int32_t i;

	digimon = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]];
	moves = digimon->stats.base.moves;
	target = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[COMBAT_DATA_PTR->fighter[index].targetId]];
	for (i = 0; i < 3; i++) {
		if (flags[i] == 1) {
			tech = entityGetTechFromAnim(&digimon->entity, moves[i]);
			score[i] = TYPE_FACTORS[modSpeciesMove(digimon->entity.type, tech)->special][DIGI_PARA(target->type).special[0]];
		} else {
			score[i] = -1;
		}
	}

	BTL_getHighestScoredMove(score, flags, &best, 3);

	return best;
}

void BTL_handleHitReaction(Entity *entity, FighterData *fighter, AttackObject *attack, int16_t index)
{
	VECTOR d;
	VECTOR n;
	VECTOR *loc;
	VECTOR a;
	PositionData *posData;
	int16_t *rotY;
	int16_t ang;

	posData = entity->posData;
	loc = &entity->posData->location;
	rotY = &posData->rotation.vy;
	a.vx = attack->position.vx;
	a.vy = 0;
	a.vz = attack->position.vz;
	d.vx = loc->vx - a.vx;
	d.vy = 0;
	d.vz = loc->vz - a.vz;
	n.vx = -d.vx;
	n.vy = 0;
	n.vz = -d.vz;
	ang = _atan(n.vz, n.vx);
	if (fighter->flags & FIGHTER_FLAG_FLATTENED) {
		*rotY = ang;
		entity->flatSprite = 3;
		startAnimation(entity, 0x28);
		return;
	}
	if (*rotY >= 0x400 && *rotY < 0xc00) {
		if (*rotY - 0x400 <= ang && *rotY + 0x400 >= ang) {
			*rotY = ang;
			BTL_startHitAnimation(entity, attack, 0x28);
		} else {
			*rotY = _atan(d.vz, d.vx);
			BTL_startHitAnimation(entity, attack, 0x29);
		}
	} else if (!(0 > *rotY) && *rotY < 0x400) {
		if ((!(0 > ang) && ang <= *rotY + 0x400) || (ang >= *rotY + 0xc00 && ang < 0x1000)) {
			*rotY = ang;
			BTL_startHitAnimation(entity, attack, 0x28);
		} else {
			*rotY = _atan(d.vz, d.vx);
			BTL_startHitAnimation(entity, attack, 0x29);
		}
	} else {
		if ((*rotY - 0x400 <= ang && ang < 0x1000) || (!(0 > ang) && *rotY - 0xc00 >= ang)) {
			*rotY = ang;
			BTL_startHitAnimation(entity, attack, 0x28);
		} else {
			*rotY = _atan(d.vz, d.vx);
			BTL_startHitAnimation(entity, attack, 0x29);
		}
	}
}

void BTL_applyMoveStatus(DigimonEntity *digimon, FighterData *fighter, int32_t move)
{
	int32_t chance;

	if (fighter->flags & FIGHTER_FLAG_PROTECTED) {
		return;
	}

	if (MOVE_DATA[move].statusChance == 0) {
		return;
	}

	chance = MOVE_DATA[move].statusChance;
	if (randomLimit(100) < chance) {
		switch (MOVE_DATA[move].status) {
		case 1:
			if (!(fighter->flags & FIGHTER_FLAG_POISONED)) {
				fighter->flags |= FIGHTER_FLAG_POISONED;
				fighter->poisonTimer = BM(POISON_FRAMES);
				BTL_addPoisonStatusVisual(digimon, fighter);
			}
			break;
		case 2:
			if (!(fighter->flags & FIGHTER_FLAG_CONFUSED)) {
				fighter->flags |= FIGHTER_FLAG_CONFUSED;
				fighter->confusionTimer = randomLimit(BMS(CONF_SPAN)) + BM(CONF_BASE);
				BTL_addConfusionStatusVisual(digimon, fighter);
				BTL_resetFighterAction(fighter);
			}
			break;
		case 3:
			if (!(fighter->flags & FIGHTER_FLAG_STUNNED)) {
				fighter->flags |= FIGHTER_FLAG_STUNNED;
				fighter->stunTimer = randomLimit(BMS(STUN_SPAN)) + BM(STUN_BASE);
				BTL_addStunStatusVisual(digimon, fighter);
				BTL_resetFighterAction(fighter);
			}
			break;
		case 4:
			if (!(fighter->flags & FIGHTER_FLAG_FLATTENED)) {
				fighter->flatTimer = -1;
				BTL_removeStatusEffects(digimon, fighter);
				BTL_resetFighterAction(fighter);
			}
			break;
		}
		if (fighter == COMBAT_DATA_PTR->fighter) {
			COMBAT_DATA_PTR->player.statusedCount++;
		}
	}
}

// clang-format off
int16_t BTL_getCheapestMove(index, flags)
	int16_t index;
	int16_t *flags;
// clang-format on
{
	int16_t score[3];
	int16_t best;
	int16_t tech;
	uint8_t *moves;
	DigimonEntity *digimon;
	int32_t i;

	digimon = (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]];
	moves = digimon->stats.base.moves;
	for (i = 0; i < 3; i++) {
		if (flags[i] == 1) {
			tech = entityGetTechFromAnim(&digimon->entity, moves[i]);
			score[i] = modSpeciesMove(digimon->entity.type, tech)->mpCost * BM(MP_MULT);
		} else {
			score[i] = 1000;
		}
	}

	BTL_getLowestScoredMove(score, flags, &best, 3);

	return best;
}

void BTL_getHighestScoredMove(int16_t *values, int16_t *marks, int16_t *out, int16_t count)
{
	int32_t i;
	int32_t hits;
	int16_t best;

	best = values[0];
	for (i = 1; i < count; i++) {
		if (best < values[i]) {
			best = values[i];
		}
	}

	hits = 0;
	for (i = 0; i < count; i++) {
		if (best == values[i]) {
			hits++;
			*out = i;
			marks[i] = 1;
		} else {
			marks[i] = -1;
		}
	}

	if (hits >= 2) {
		*out = -1;
	}
}

void BTL_getLowestScoredMove(int16_t *values, int16_t *marks, int16_t *out, int16_t count)
{
	int32_t i;
	int32_t hits;
	int16_t best;

	best = values[0];
	for (i = 1; i < count; i++) {
		if (best > values[i]) {
			best = values[i];
		}
	}

	hits = 0;
	for (i = 0; i < count; i++) {
		if (best == values[i]) {
			hits++;
			*out = i;
			marks[i] = 1;
		} else {
			marks[i] = -1;
		}
	}

	if (hits >= 2) {
		*out = -1;
	}
}

int16_t BTL_getNpcEntityIndex(Entity *entity)
{
	int32_t i;

	for (i = 2; i < 0xa; i++) {
		if (entity == ENTITY_TABLE[i]) {
			return i - 2;
		}
	}

	return -1;
}

void BTL_getStatusAfflictedEnemies(Entity *self, int16_t *out, int16_t *count)
{
	CombatData *combat;
	int32_t i;
	Entity *entity;

	*count = 0;
	combat = COMBAT_DATA_PTR;
	for (i = 0; ENEMY_COUNT >= i; i++) {
		entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		if ((self != entity) && (BTL_isFighterDefeated(i) == 0) && ((combat->fighter[i].flags & (FIGHTER_FLAG_POISONED | FIGHTER_FLAG_CONFUSED | FIGHTER_FLAG_STUNNED | FIGHTER_FLAG_FLATTENED)) != 0)) {
			out[*count] = i;
			(*count)++;
		}
	}
}

void BTL_getWeakestEnemy(Entity *self, FighterData *fighter, int16_t *outScore, int16_t *outIndex)
{
	FighterData *enemy;
	Entity *other;
	int16_t best;
	int32_t i;

	best = 1000;
	*outIndex = 0xff;
	for (i = 0; ENEMY_COUNT >= i; i++) {
		enemy = &COMBAT_DATA_PTR->fighter[i];
		other = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		if ((other != self) && (BTL_isFighterDefeated(i) == 0)) {
			if ((fighter->targetId == 0xff) || (other != ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[fighter->targetId]])) {
				*outScore = BTL_calculateTargetScore((DigimonEntity *)self, (DigimonEntity *)other);
				if (*outScore < best) {
					best = *outScore;
					*outIndex = i;
				}
			}
		}
	}
}

int16_t BTL_calculateTargetScore(DigimonEntity *self, DigimonEntity *other)
{
	Stats *st;
	int16_t mult;

	st = &self->stats;
	mult = TYPE_FACTORS[DIGI_PARA(self->entity.type).special[0]]
			      [DIGI_PARA(other->entity.type).special[0]];
	switch (mult) {
	case 20:
		mult = BM(TSCORE_INV0);
		break;
	case 15:
		mult = BM(TSCORE_INV1);
		break;
	case 5:
		mult = BM(TSCORE_INV2);
		break;
	case 2:
		mult = BM(TSCORE_INV3);
		break;
	}

	/* MOD (caps): keep the score below the selector's 1000 sentinel even past STAT_CAP.
	 * Defense through the original cap keeps exactly the same integer divisions. */
	mult = st->current.currentHP * 100 / st->base.hp + BMD(mult * BMD(CAPS_OLD_STAT(st->base.def) * 100, TSCORE_DEF), TSCORE_DIV);
	return mult;
}

void BTL_sortScoresDescending(int32_t *values, int32_t *keys, int32_t *groups, int32_t count)
{
	int32_t i;
	int32_t j;
	int32_t bestValue;
	int32_t best;

	for (i = 0; i < count; i++) {
		bestValue = values[i];
		best = i;
		for (j = i; j < count; j++) {
			if (bestValue < values[j]) {
				best = j;
				bestValue = values[j];
			}
		}
		swapInt(&values[i], &values[best]);
		swapInt(&keys[i], &keys[best]);
	}

	BTL_calculateScoreRanks(values, groups, count);
}

void BTL_sortScoresAscending(int32_t *values, int32_t *keys, int32_t *groups, int32_t count)
{
	int32_t i;
	int32_t j;
	int32_t best;
	int32_t bestValue;

	for (i = 0; i < count; i++) {
		bestValue = values[i];
		best = i;
		for (j = i; j < count; j++) {
			if (bestValue > values[j]) {
				best = j;
				bestValue = values[j];
			}
		}
		swapInt(&values[i], &values[best]);
		swapInt(&keys[i], &keys[best]);
	}

	BTL_calculateScoreRanks(values, groups, count);
}

void BTL_selectPartnerMove(DigimonEntity *digimon, FighterData *fighter, int16_t index)
{
	int32_t j;
	int32_t pick;
	int32_t total;
	int32_t values[3];
	int32_t keys[3];
	int32_t groups[3];
	int16_t flags[4];
	int16_t weights[4];
	int32_t count;
	int16_t tech;
	Stats *stats;
	int32_t i;

	if (BTL_getUsableMoves(flags, index) == 0) {
		BTL_setFighterCooldown(digimon, fighter);
		return;
	}

	for (i = 0; i < 3; i++) {
		weights[i] = 0;
		keys[i] = i;
	}

	stats = &digimon->stats;
	switch (stats->current.chargeMode) {
	case 0:
		for (i = 0; i < 3; i++) {
			if (flags[i] == 0) {
				values[i] = -1;
			} else {
				tech = entityGetTechFromAnim(&digimon->entity, stats->base.moves[i]);
				values[i] = modSpeciesMove(digimon->entity.type, tech)->power;
			}
		}
		BTL_sortScoresDescending(values, keys, groups, 3);
		for (i = 0; i < 3; i++) {
			if (stats->base.moves[keys[i]] == 0xff) {
				weights[keys[i]] += BM(AI_EMPTY_WEIGHT);
			} else if (flags[keys[i]] != 0) {
				weights[keys[i]] = BM(PRIO_POWER0 + groups[i]);
			}
		}
		break;
	case 1:
		for (i = 0; i < 3; i++) {
			if (stats->base.moves[i] == 0xff) {
				weights[i] += BM(AI_EMPTY_WEIGHT);
			}
			if (flags[i] != 0) {
				weights[i] += BM(AI_MODE1_WEIGHT);
			}
		}
		break;
	case 2:
		for (i = 0; i < 3; i++) {
			if (flags[i] == 0) {
				values[i] = 10000;
			} else {
				tech = entityGetTechFromAnim(&digimon->entity, stats->base.moves[i]);
				values[i] = modSpeciesMove(digimon->entity.type, tech)->mpCost * BM(MP_MULT);
			}
		}
		BTL_sortScoresAscending(values, keys, groups, 3);
		for (i = 0; i < 3; i++) {
			if (stats->base.moves[keys[i]] == 0xff) {
				weights[keys[i]] += BM(AI_EMPTY_WEIGHT);
			} else if (flags[keys[i]] != 0) {
				weights[keys[i]] = BM(PRIO_MP0 + groups[i]);
			}
		}
		break;
	}

	for (i = 0; i < 3; i++) {
		if (stats->base.moves[i] == 0xff) {
			continue;
		}
		if (flags[i] == 0) {
			weights[i] = 0;
			continue;
		}
		tech = entityGetTechFromAnim(&digimon->entity, stats->base.moves[i]);
		if (MOVE_DATA[tech].range == 4) {
			weights[i] += fighter->buffPrioTimer + BTL_calculateElementBonus(modSpeciesMove(digimon->entity.type, tech)->special, DIGI_PARA(digimon->entity.type).special[0]);
			continue;
		}
		weights[i] += BTL_calculateElementBonus(modSpeciesMove(digimon->entity.type, tech)->special, DIGI_PARA(ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[fighter->targetId]]->type).special[0]);
		if (MOVE_DATA[tech].range != 3) {
			continue;
		}
		count = BTL_countLivingEnemies();
		switch (digimon->stats.current.chargeMode) {
		case 0:
			if ((stats->base.brain >= BM(AI_GROUP_BRAIN)) && (count >= 2)) {
				weights[i] += count * BM(AI_GROUP_BONUS0);
			}
			break;
		case 1:
			if ((stats->base.brain >= BM(AI_GROUP_BRAIN)) && (count >= 2)) {
				weights[i] += count * BM(AI_GROUP_BONUS1);
			}
			break;
		case 2:
			if ((stats->base.brain >= BM(AI_GROUP_BRAIN)) && (count >= 2)) {
				weights[i] += count * BM(AI_GROUP_BONUS2);
			} else if ((stats->current.currentHP * 100 / stats->base.hp) < BM(AI_LOWHP_PCT)) {
				for (j = 0; j < 3; j++) {
					if (flags[i] == 0) {
						values[i] = -1;
					} else {
						tech = entityGetTechFromAnim(&digimon->entity, stats->base.moves[i]);
						values[i] = MOVE_DATA[tech].distance;
					}
				}
				BTL_sortScoresDescending(values, keys, groups, 3);
				for (i = 0; i < 3; i++) {
					if (stats->base.moves[i] == 0xff) {
						continue;
					}
					if (flags[i] == 0) {
						continue;
					}
					weights[keys[i]] = BM(PRIO_WIDE0 + groups[i]);
				}
			}
			break;
		}
	}

	total = 0;
	for (i = 0; i < 3; i++) {
		total += weights[i];
	}

	pick = randomLimit(total);
	total = 0;
	for (i = 0; i < 3; i++) {
		if (weights[i] != 0) {
			total += weights[i];
			if (pick < total) {
				break;
			}
		}
	}

	if (digimon->stats.base.moves[i] != 0xff) {
		BTL_setupQueuedMove(digimon, fighter, index, i);
	} else {
		fighter->cooldown = BM(CD_IDLE);
		fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
	}
}

int16_t BTL_calculateElementBonus(int16_t arg0, int16_t arg1)
{
	switch (TYPE_FACTORS[arg0][arg1]) {
	case 20:
		return BM(ELEM0);
	case 15:
		return BM(ELEM1);
	case 10:
		return BM(ELEM2);
	case 5:
		return BM(ELEM3);
	case 2:
		return BM(ELEM4);
	}
	return 0;
}

int32_t BTL_countLivingEnemies(void)
{
	int32_t count;
	int32_t i;

	count = 0;
	for (i = 1; ENEMY_COUNT >= i; i++) {
		if (BTL_isFighterDefeated(i) == 0) {
			count++;
		}
	}

	return count;
}

void BTL_calculateScoreRanks(int32_t *values, int32_t *groups, int32_t count)
{
	int32_t i;
	int32_t group;

	group = 0;
	groups[0] = group;
	for (i = 1; i < count; i++) {
		if (values[i] == (values + i)[-1]) {
			groups[i] = group;
		} else {
			groups[i] = ++group;
		}
	}
}

void BTL_selectConfusedMove(DigimonEntity *digimon, FighterData *fighter, long index)
{
	TargetChoice choice;

	if (randomLimit(10) < BM(CONF_NOTARGET)) {
		fighter->targetId = 0xff;
	} else {
		BTL_getRemainingEnemies(&digimon->entity, choice.enemies, &choice.count);
		fighter->targetId = choice.enemies[randomLimit(choice.count)];
	}

	if (BTL_getUsableMoves(choice.flags, index) == 0) {
		BTL_setFighterCooldown(digimon, fighter);
	} else {
		BTL_setupQueuedMove(digimon, fighter, index, BTL_getRandomUsableMove(choice.flags));
	}
}

void BTL_getRemainingEnemies(Entity *self, int16_t *out, int16_t *count)
{
	CombatData *combat;
	int32_t i;
	Entity *entity;

	*count = 0;
	combat = COMBAT_DATA_PTR;
	for (i = 0; i <= ENEMY_COUNT; i++) {
		entity = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
		if ((self != entity) && (BTL_isFighterDefeated(i) == 0)) {
			out[*count] = i;
			(*count)++;
		}
	}
}

int32_t BTL_selectMoveByPower(int32_t arg0, int16_t *flags)
{
	int16_t result;

	result = BTL_getStrongestMove(arg0, flags);
	if (result != -1) {
		return result;
	}

	result = BTL_getMostEffectiveMove(arg0, flags);
	if (result != -1) {
		return result;
	}

	return BTL_getRandomUsableMove(flags);
}

int32_t BTL_selectMoveByMpCost(int32_t arg0, int16_t *flags)
{
	int16_t result;

	result = BTL_getCheapestMove(arg0, flags);
	if (result != -1) {
		return result;
	}

	result = BTL_getMostEffectiveMove(arg0, flags);
	if (result != -1) {
		return result;
	}

	return BTL_getRandomUsableMove(flags);
}

void BTL_selectFighterTarget(DigimonEntity *digimon, FighterData *fighter, uint8_t target, int32_t arg3)
{
	int16_t enemies[4];
	int16_t flags[4];
	int16_t eff;
	int16_t score;
	int16_t choice;
	int16_t brain;
	int16_t count;
	int32_t i;

	if (((arg3 != 0) && (digimon == (DigimonEntity *)ENTITY_TABLE[1])) || (digimon != (DigimonEntity *)ENTITY_TABLE[1])) {
		if (target != 0xff) {
			if (digimon != (DigimonEntity *)ENTITY_TABLE[1]) {
				if ((NPC_ENTITIES[BTL_getNpcEntityIndex(&digimon->entity)].unk1 == 1) || (randomLimit(100) >= BM(ENEMY_AIM))) {
					fighter->targetId = 0;
					return;
				}
			}
			if (BTL_isFighterDefeated(target) != 0) {
				return;
			}
			fighter->flags |= FIGHTER_FLAG_10;
			fighter->targetId = target;
		} else {
			fighter->targetId = 0;
		}
		return;
	}

	if (COMBAT_DATA_PTR->player.currentCommand[0] == 7) {
		if ((fighter->targetId != 0xff) && (BTL_isFighterDefeated(fighter->targetId) == 0)) {
			return;
		}
	}

	for (i = 0; ENEMY_COUNT >= i; i++) {
		enemies[i] = -1;
	}

	brain = digimon->stats.base.brain;
	if (brain >= BM(SMART_BRAIN)) {
		BTL_getStatusAfflictedEnemies(&digimon->entity, enemies, &count);
		if (count >= 2) {
			fighter->targetId = enemies[randomLimit(count)];
			return;
		}
		if (count == 1) {
			fighter->targetId = enemies[0];
			return;
		}
		BTL_getWeakestEnemy(&digimon->entity, fighter, &score, &choice);
		if ((fighter->targetId == 0xff) || (BTL_isFighterDefeated(fighter->targetId) != 0)) {
			fighter->targetId = choice;
			return;
		}
		eff = BTL_calculateTargetScore(digimon, (DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[fighter->targetId]]);
		if (choice == 0xff) {
			return;
		}
		if ((eff - score) < BM(SWITCH_GAP)) {
			return;
		}
		fighter->targetId = choice;
		return;
	}

	if (brain >= BM(WEAKEST_BRAIN)) {
		if ((fighter->targetId != 0xff) && (BTL_isFighterDefeated(fighter->targetId) == 0)) {
			return;
		}
		BTL_getWeakestEnemy(&digimon->entity, fighter, &score, &choice);
		fighter->targetId = choice;
		return;
	}

	for (i = 0; ENEMY_COUNT >= i; i++) {
		flags[i] = 1;
	}

	fighter->targetId = BTL_getNearestEnemy(&digimon->entity, flags);
}

int16_t BTL_getNearestEnemy(Entity *self, int16_t *flags)
{
	int32_t i;
	uint32_t dist;
	uint32_t bestDist;
	Entity *other;
	int16_t best;

	best = 0xff;
	bestDist = -1;
	for (i = 0; ENEMY_COUNT >= i; i++) {
		if (flags[i] == 1) {
			other = ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[i]];
			if ((self != other) && (BTL_isFighterDefeated(i) == 0)) {
				dist = BTL_getDistanceSquared(self, other);
				if (dist < bestDist) {
					bestDist = dist;
					best = i;
				}
			}
		}
	}

	return best;
}

uint8_t BTL_isFighterDefeated(uint8_t index)
{
	if ((((DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[index]])->stats.current.currentHP - COMBAT_DATA_PTR->fighter[index].hpDamageBuffer) <= 0) {
		return 1;
	}

	return 0;
}

void BTL_selectEnemyMove(DigimonEntity *digimon, FighterData *fighter, int16_t index)
{
	int32_t j;
	int32_t pick;
	int32_t total;
	int16_t flags[4];
	int16_t weights[4];
	int16_t tech;
	int32_t i;

	if (BTL_getUsableMoves(flags, index) == 0) {
		digimon->stats.current.chargeMode = 2;
		BTL_setFighterCooldown(digimon, fighter);
		return;
	}

	for (i = 0; i < 4; i++) {
		weights[i] = digimon->stats.base.movesPrio[i];
	}

	for (i = 0; i < 4; i++) {
		if (digimon->stats.base.moves[i] == 0xff) {
			continue;
		}
		if (flags[i] == 0) {
			weights[i] = 0;
			continue;
		}
		tech = entityGetTechFromAnim(&digimon->entity, digimon->stats.base.moves[i]);
		for (j = 1; ENEMY_COUNT >= j; j++) {
			if (j != index) {
				if ((((DigimonEntity *)ENTITY_TABLE[COMBAT_DATA_PTR->player.entityIds[j]])->stats.current.currentHP - COMBAT_DATA_PTR->fighter[j].hpDamageBuffer) > 0) {
					break;
				}
			}
		}
		if (j != (ENEMY_COUNT + 1)) {
			if (NPC_ENTITIES[BTL_getNpcEntityIndex(&digimon->entity)].unk1 == 0) {
				if (MOVE_DATA[tech].range == 3) {
					weights[i] += BM(AI_ENEMY_GROUP_WEIGHT);
				}
			}
		}
		if (MOVE_DATA[tech].range == 4) {
			weights[i] += fighter->buffPrioTimer;
		}
	}

	total = 0;
	for (i = 0; i < 4; i++) {
		total += weights[i];
	}

	pick = randomLimit(total);
	total = 0;
	for (i = 0; i < 4; i++) {
		if (weights[i] != 0) {
			total += weights[i];
			if (pick < total) {
				break;
			}
		}
	}

	if (digimon->stats.base.moves[i] != 0xff) {
		BTL_setupQueuedMove(digimon, fighter, index, (uint8_t)i);
	} else {
		fighter->cooldown = BM(CD_IDLE);
		fighter->flags |= FIGHTER_FLAG_ON_CHARGEUP;
	}
}

void BTL_setCommandIconUV(DigimonEntity *digimon, POLY_FT4 *prim, uint8_t index)
{
	int16_t c;
	int16_t eff;
	int16_t n;

	if ((index >= 8U) && (index < 0xcU)) {
		eff = modSpeciesMove(digimon->entity.type, entityGetTechFromAnim(&digimon->entity, digimon->stats.base.moves[index - 8]))->special;
		setUVWH(prim, BTL_SPECIAL_ICON_UVS[eff][0], BTL_SPECIAL_ICON_UVS[eff][1], 0x10, 0xf);
	} else {
		setUVWH(prim, BTL_COMMAND_ICON_UVS[index - 1][0], BTL_COMMAND_ICON_UVS[index - 1][1], 0x10, 0xf);
	}
}
