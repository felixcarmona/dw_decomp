#ifndef DW_CARDS_H
#define DW_CARDS_H
#include <stdint.h>
#include <dw/ids.h>
typedef struct {
	uint16_t digimonId;
	uint8_t spriteId;
	uint8_t meritValue;
} CardData;
typedef char CardTableSectorCheck[sizeof(CardData) * NUM_CARDS == 2048 ? 1 : -1];

/* One sector, owned by the album or card shop while no battle is active. */
#include <dw/memmap.h>
#define CARD_DATA ((CardData *)OVL_BTL_BASE)
#define CARD_EXTRA_BYTES ((NUM_CARDS - 66) / 2)
#define CARD_NONE 0xffff
extern uint8_t MOD_CARD_EXTRA[CARD_EXTRA_BYTES];
extern uint16_t MOD_CARD_STOCK[6];
extern uint8_t MOD_CARD_REAL[NUM_CARDS / 8];
void modLoadCardTable(void);
int32_t modCardIsReal(int32_t id);
int32_t modLoadCardImage(int32_t id, void *buffer);

uint8_t getCardAmount(int32_t id);
int32_t setCardAmount(int32_t id, int32_t amount);
int32_t modCardMove(int32_t id, int32_t input);
int32_t modCardTop(int32_t id, int32_t top);
#endif
