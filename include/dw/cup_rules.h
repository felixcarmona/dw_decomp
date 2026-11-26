#ifndef DW_CUP_RULES_H
#define DW_CUP_RULES_H

#include <dw/types.h>
#include <dw/file.h>
#include <dw/file_queue.h>
#include <dw/species.h>

int32_t isFileReadQueueBusy(void);

/*
 * MOD (cup rules): a cup's list of who may enter (entry 3 of its section 10 record) may open with CUP_RULE_MARK, a kind and
 * a value; the ids after them are entered as well, whatever the rule says. Without the mark the list is the original's: the
 * ids it names, and the species of DIGI.DAT that stand in for them.
 *
 * What a rule asks of a species is in its cup tag: one u32 per id in DIGI.DAT, at the sectors the header names at
 * SPECIES_HEADER_CUP_TAGS (sector in the low 24 bits, how many sectors above them; tools/species.py cup_tag). The registration
 * desk (DGET) and the arena (STD) read it; nothing of it is resident.
 */
#define CUP_RULE_MARK		253
enum {
	CUP_RULE_EXACT,     /* only the ids of the list: no species stands in for another */
	CUP_RULE_LEVEL,     /* value: the level (3 Rookie .. 6 Mega) */
	CUP_RULE_ATTRIBUTE, /* value: 1 Data, 2 Vaccine, 3 Virus */
	CUP_RULE_SPECIALTY, /* value: the first specialty + 1 */
	CUP_RULE_FIELD,     /* value: the Field's bit */
};

#define SPECIES_HEADER_CUP_TAGS	0x4f8
#define CUP_TAGS_PER_SECTOR	512
#define CUP_TAG_FIELDS(tag)	((tag) & 0x3ff)
#define CUP_TAG_ATTRIBUTE(tag)	((tag) >> 10 & 3)
#define CUP_TAG_SPECIALTY(tag)	((tag) >> 12 & 7)
#define CUP_TAG_LEVEL(tag)	((tag) >> 15 & 7)
#define CUP_TAG_RIVAL		(1 << 18) /* a species of DIGI.DAT the arena can draw: raised by players, Rookie or higher */

/* does a species with this tag and level meet the rule? (the level is passed apart: the original species' may be swapped) */
static inline int32_t cupRuleHolds(int32_t kind, int32_t value, uint32_t tag, int32_t level)
{
	switch (kind) {
	case CUP_RULE_LEVEL:
		return level == value;
	case CUP_RULE_ATTRIBUTE:
		return CUP_TAG_ATTRIBUTE(tag) == value;
	case CUP_RULE_SPECIALTY:
		return CUP_TAG_SPECIALTY(tag) == value;
	case CUP_RULE_FIELD:
		return (CUP_TAG_FIELDS(tag) >> value & 1) != 0;
	}
	return 0;
}

#endif
