/* Technique layout A: old saves store animation slots, not technique identities.
 * Keep the library intact. Replace a removed selection only with an already-known,
 * compatible normal, preferring its element then the closest power (slot breaks ties).
 * All migration code/table and the save builder stay in MENU. Title-only
 * OPTIONS code shares EVL with the picker to retain both heap and MENU limits. */
static void migrateCoherentSlots(int32_t type, uint8_t *equipped, const uint32_t *learned)
{
	static const struct { uint16_t type; uint8_t moves[16]; } old[] = {
		{111, {37, 35, 33, 36, 34, 32, 38, 98, 255, 255, 255, 255, 255, 255, 255, 255}},
		{185, {40, 43, 41, 42, 37, 34, 35, 75, 255, 255, 255, 255, 255, 255, 255, 255}},
		{353, {43, 41, 42, 40, 19, 21, 23, 95, 255, 255, 255, 255, 255, 255, 255, 255}},
		{189, {46, 40, 41, 42, 43, 44, 39, 34, 35, 37, 75, 255, 255, 255, 255, 255}},
		{308, {46, 44, 41, 42, 43, 40, 39, 34, 35, 37, 100, 255, 255, 255, 255, 255}},
		{327, {46, 40, 41, 42, 43, 44, 39, 34, 35, 37, 81, 255, 255, 255, 255, 255}},
		{330, {46, 40, 45, 41, 42, 43, 34, 35, 37, 32, 81, 255, 255, 255, 255, 255}},
		{93, {12, 16, 19, 21, 18, 11, 20, 95, 255, 255, 255, 255, 255, 255, 255, 255}},
		{214, {36, 39, 33, 38, 32, 37, 35, 34, 27, 29, 25, 28, 31, 73, 255, 255}},
		{257, {46, 47, 40, 45, 44, 43, 41, 42, 33, 39, 36, 38, 32, 73, 255, 255}},
		{307, {56, 52, 51, 50, 54, 49, 53, 55, 20, 47, 21, 23, 19, 18, 62, 16}},
	};
	uint8_t removed[3];
	DigimonPara *para;
	int32_t row, i, j, k, tech, prior, best, score, distance;

	for (row = 0; row < sizeof(old) / sizeof(old[0]); row++) {
		if (old[row].type == type) break;
	}
	if (row == sizeof(old) / sizeof(old[0])) return;
	para = &DIGI_PARA(type);
	for (i = 0; i < 3; i++) {
		removed[i] = 0xff;
		if (equipped[i] == 0xff) continue;
		prior = equipped[i] >= 0x2e && equipped[i] < 0x3e ? old[row].moves[equipped[i] - 0x2e] : 0xff;
		/* v0.0.7 used Battle in these slots; the unmarked audit build used Ice.
		 * A full library cannot distinguish them. Never guess a saved identity. */
		if (type == 307 && equipped[i] >= 0x2e && equipped[i] < 0x3e &&
		    ((0xbd00u >> (equipped[i] - 0x2e)) & 1u)) prior = 0xff;
		equipped[i] = 0xff;
		if (prior >= 58) continue;
		for (j = 0; j < 16; j++) {
			tech = para->moves[j];
			if (tech != prior || (learned && !((learned[tech / 32] >> (tech % 32)) & 1))) continue;
			for (k = 0; k < 3 && para->special[k] != MOVE_DATA[tech].special; k++) {}
			if (k < 3) { equipped[i] = 0x2e + j; break; }
		}
		if (equipped[i] == 0xff) removed[i] = prior;
	}
	if (learned == NULL) return; /* Registered VS fighters have no library: clear removed selections. */
	for (i = 0; i < 3; i++) {
		prior = removed[i];
		if (prior == 0xff) continue;
		best = -1;
		score = 0x7fffffff;
		for (j = 0; j < 16; j++) {
			tech = para->moves[j];
			if (tech >= 58 || !((learned[tech / 32] >> (tech % 32)) & 1)) continue;
			for (k = 0; k < 3 && para->special[k] != MOVE_DATA[tech].special; k++) {}
			if (k == 3) continue;
			for (k = 0; k < 3 && equipped[k] != 0x2e + j; k++) {}
			if (k < 3) continue;
			distance = MOVE_DATA[tech].power - MOVE_DATA[prior].power;
			if (distance < 0) distance = -distance;
			if (MOVE_DATA[tech].special != MOVE_DATA[prior].special) distance += 0x10000;
			if (distance < score) { score = distance; best = j; }
		}
		equipped[i] = best < 0 ? 0xff : 0x2e + best;
	}
}

/* 0: unversioned slots; 1: layout A; -1: a layout this reader cannot interpret. */
static int32_t coherentSlotLayout(uint8_t *e)
{
	int32_t at = sizeof(SaveExtHead);
	int32_t tag, len;
	for (;;) {
		tag = e[at] | (e[at + 1] << 8);
		len = e[at + 2] | (e[at + 3] << 8);
		if (tag == SAVE_TAG_END) return 0;
		if (tag == SAVE_TAG_TECHNIQUE_LAYOUT) return len == 1 && e[at + 4] == 1 ? 1 : -1;
		at += 4 + len;
	}
}


/* Registered equipment is also animation slots. Migrate it before stamping a save
 * or handing a legacy card's entries directly to VS (which bypasses Continue). */
static void migrateCoherentRegistered(RegisteredDigimon *p, int32_t n)
{
	int32_t i;
	for (i = 0; i < n; i++, p++) migrateCoherentSlots(registeredSpecies(p), p->moves, NULL);
}
