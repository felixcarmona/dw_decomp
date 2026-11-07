#ifndef DW_SAVE_TAGS_H
#define DW_SAVE_TAGS_H

/*
 * MOD (save v4, docs/mods/save-format.md): the extension of a save file is a list of records
 *
 *     u16 tag, u16 length, length bytes        (little endian, no padding)
 *
 * ended by a record of tag 0 and length 0. A build reads the records it knows and skips the others, so new data never
 * breaks an old reader and a changed size (more species, more flags) never breaks a new one: a known record is copied
 * as far as both sides have room, and whatever it does not cover keeps the value the v1 part of the file or the
 * defaults gave it.
 *
 * The rules that keep every save readable by every later build:
 *  - A tag number is never reused or given another meaning; a record whose elements change gets a new tag.
 *  - A record may only grow at its end.
 *  - Bit 15 of the tag marks a record the file cannot be read without (the story state of the engine tier 2): a build
 *    that does not know it refuses the file ("Data Error") instead of dropping what it holds.
 *
 * tools/mcd.py mirrors this table to print a save as JSON.
 */
#define SAVE_TAG_END			0x0000
#define SAVE_TAG_SCREENS		0x0001 /* i16 current screen, i16 previous screen (authoritative; the v1 bytes are the low byte) */
#define SAVE_TAG_INVENTORY		0x0002 /* i16[n] item ids of the inventory slots, -1 = empty */
#define SAVE_TAG_PARTNER_TYPE		0x0003 /* i32 species of the partner */
#define SAVE_TAG_WREGS			0x0004 /* u16[n] the script registers w0.. */
#define SAVE_TAG_REGISTERED		0x0005 /* i16[n] species of the registered Digimon of the VS mode (kept for reading tools) */
#define SAVE_TAG_POOP_MAPS		0x0006 /* i16[n] screens of the poop in the world, -1 = none (authoritative since screens beyond 255; the v1 byte is 0xFF for a screen >= 255) */
#define SAVE_TAG_SPECIES_RAISED		0x0007 /* bit array: species the tamer has raised */
#define SAVE_TAG_SPECIES_RECRUITED	0x0008 /* bit array: species the tamer has recruited */
/* OPTIONS byte 13: bit 0 original enemy bars, bit 1 brief-after-hit; missing = 0. */
#define SAVE_TAG_OPTIONS		0x0009 /* u8[n] the title menu's OPTIONS: [0] button layout (0 US, 1 Japanese) */
#define SAVE_TAG_VANILLA_SWAP		0x000A /* i16 species, i16 vanilla species: NEW DIGIMON off turned the partner from the first into the second (main_menu.c modVanillaSwap) */
#define SAVE_TAG_GROUND_PICKED		0x000B /* {i16 screen, u8 tileX, u8 tileY}[64]: the ground spots picked up today (retired; a zero-length record writes an empty legacy history) */
#define SAVE_TAG_GROUND_PICKUP_BITS	0x000C /* bit array: stable ground pickup IDs collected today; registry content/ground-items.json */
#define SAVE_TAG_REQUIRED		0x8000
/* engine tier 2 */
#define SAVE_TAG_STORY_FLAGS		0x8001 /* bit array: story flag n = trigger 800 + n */
#define SAVE_TAG_STORY_VARS		0x8002 /* i32[n] story variables */

#endif
