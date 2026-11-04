#ifndef DW_MOD_QOL_H
#define DW_MOD_QOL_H

#include <dw/types.h>

/* MOD (quality of life, src/main/mod_qol.c) */

/* a plate at (x, y) with text (the letters R 2 x 1 5 S T A and spaces): the letters from index hi on in (r, g, b), the
   others white */
void modPlate(int32_t x, int32_t y, const char *text, int32_t hi, int32_t r, int32_t g, int32_t b);

/* the training block (src/trn/trn_block.h): x1 (0) or x5 (1), switched with R2 in a gym session (main.c) */
extern int8_t MOD_TRAIN_BLOCK;
/* why a block stops (MOD_TB_LEARNED is the block's own check) after a session that started at minute t0 of the day, with the gym's tiredness rule tiredAt */
enum { MOD_TB_GO, MOD_TB_HUNGRY, MOD_TB_TOILET, MOD_TB_SLEEPY, MOD_TB_TIRED, MOD_TB_UNWELL, MOD_TB_EVOLVE, MOD_TB_LIFE,
       MOD_TB_LEARNED };
int32_t modTrainStopReason(int32_t t0, int32_t tiredAt);

#endif
