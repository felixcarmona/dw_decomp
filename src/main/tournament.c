#include <dw/clock.h>
#include <dw/entity.h>
#include <dw/partner.h>
#include <dw/pstat.h>
#include <dw/tamer.h>
#include <dw/tournament.h>
#include <dw/trigger.h>
#include <dw/types.h>
#include <dw/ui.h>

#include "common.h"

extern uint16_t ACTIVE_MAP_SCRIPT;
extern uint16_t SCRIPT_STATE_2;
extern uint8_t ACTIVE_INSTRUCTION;
extern uint8_t SCRIPT_TEXTBOX_MODE;
extern uint16_t SCRIPT_NEXT_STATE_2;
extern int16_t TOURNAMENT_WINS;

uint8_t *getScript(uint32_t scriptId);
uint8_t *getScriptSection(uint8_t *ptr, int32_t section);

void closeBox(int32_t id);
void startTournament(void);
void unlockMedal(uint16_t medalId);
void showMapHeadTextbox(int32_t, int32_t, int32_t, int32_t);
int32_t isTriggerSet(uint16_t trigger);
int32_t hasMedal(uint16_t medal);
int32_t getCardAmount(uint8_t cardId);
void unsetTrigger(uint16_t trigger);
uint8_t readPStat(int32_t id);
void callScriptSection(int32_t, int32_t, int32_t);
void updateMapLightState(void);
extern int32_t IS_SCRIPT_PAUSED;

/* MOD (medals as data): rows start unused (medal -1); tools/balance.py build writes them from content/tables/medals.json */
#define MOD_NO_MEDAL { -1, 0, 0, 0, 0, 0, 0 }
ModMedal MOD_MEDALS[MOD_MEDAL_MAX] = {
	MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL,
	MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL, MOD_NO_MEDAL,
};

void updateTournamentRegistration(void)
{
	uint8_t i;
	uint8_t value;
	int32_t flag;
	uint32_t minutes;
	DigimonId partnerType; /* MOD: was u8 */

	flag = 0;
	if (isTriggerSet(TRIGGER_TOURNAMENT_REGISTERED)) {
		minutes = minutesOfDay();
		partnerType = PARTNER_ENTITY.digimonEntity.entity.type;
		value = readPStat(PSTAT_TOURNAMENT_DIGIMON);
		if ((partnerType & 0xff) != value) { /* MOD (portraits): the pstat is a byte, a species >= 256 wraps */
			unsetTrigger(TRIGGER_TOURNAMENT_REGISTERED);
			return;
		}

		i = DAY;
		value = readPStat(PSTAT_TOURNAMENT_DAY);
		if (value & 0x80) {
			return;
		}

		if (i != value) {
			flag = 1;
		}

		if (minutes < 0x2D0) {
			return;
		}

		if (minutes >= 0x565) {
			flag = 1;
		}

		if (!IS_SCRIPT_PAUSED) {
			return;
		}

		if (tamerGetState()) {
			return;
		}

		if (partnerGetState() != 1) {
			return;
		}

		for (i = 0; i < 6; ++i) {
			if (UI_BOX_DATA[i].state != 0) {
				return;
			}
		}

		if (flag) {
			callScriptSection(0, 0x4DA, 1);
			unsetTrigger(TRIGGER_TOURNAMENT_REGISTERED);
			return;
		}

		if (minutes < 0x4B0) {
			if (!isTriggerSet(TRIGGER_38)) {
				return;
			}

			unsetTrigger(TRIGGER_38);
		} else {
			if (!isTriggerSet(TRIGGER_39)) {
				return;
			}

			unsetTrigger(TRIGGER_39);
		}

		callScriptSection(0, 0x4D9, 1);
	}

	updateMapLightState();
}


uint32_t minutesOfDay(void)
{
	return MINUTE + HOUR * 60;
}

void scriptStartTournament(void)
{
	switch (SCRIPT_STATE_2) {
	case 0:
		SCRIPT_STATE_2 = 2;
		break;
	case 1:
		break;
	case 2:
		closeBox(0);
		startTournament();
		/* fall through */
	case 3:
		ACTIVE_INSTRUCTION = 0;
		break;
	}
}

uint8_t *getCupDataJumpTable(uint8_t section, uint8_t id)
{
	uint8_t *script;
	uint8_t *sectionPtr;

	script = getScript(ACTIVE_MAP_SCRIPT);
	sectionPtr = getScriptSection(script, section);

	return getCupDataJumpTableEntry(sectionPtr, id);
}

uint8_t *getCupDataJumpTableEntry(uint8_t *scriptPtr, uint8_t id)
{
	uint8_t *script;

	script = getScript(ACTIVE_MAP_SCRIPT);
	scriptPtr = scriptPtr + id * 4 + 2;
	scriptPtr = script + *(uint16_t *)scriptPtr;

	return scriptPtr;
}

/* MOD (medals as data): the first MOD_MEDALS row of the script routine whose condition holds and whose medal is not owned
 * (the row index, -1 for none); was a chain of loops that stopped at the first condition that held, owned or not */
int32_t checkTournamentMedalConditions(void)
{
	int32_t i;

	for (i = 0; i < MOD_MEDAL_MAX; i++) {
		if (MOD_MEDALS[i].medal >= 0 && (MOD_MEDALS[i].flags & MOD_MEDAL_SCRIPT) != 0 &&
		    hasMedal(MOD_MEDALS[i].medal) == 0 && modMedalHolds(&MOD_MEDALS[i])) {
			return i;
		}
	}
	return -1;
}

void scriptCheckTournamentMedal(void)
{
	int32_t result;

	switch (SCRIPT_STATE_2) {
	case 0:
		result = checkTournamentMedalConditions();
		if (result == -1) {
			ACTIVE_INSTRUCTION = 0;
			return;
		}

		unlockMedal(MOD_MEDALS[result].medal);
		showMapHeadTextbox(MOD_MEDALS[result].text, 0xff, 0, 0x4db);
		SCRIPT_STATE_2 = 1;
		SCRIPT_NEXT_STATE_2 = 0;
		SCRIPT_TEXTBOX_MODE = 1;
		break;
	case 1:
		break;
	}
}
