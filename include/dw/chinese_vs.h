#ifndef DW_CHINESE_VS_H
#define DW_CHINESE_VS_H
#include <libgpu.h>
#include <dw/vs.h>
char *zhVsName(RegisteredDigimon *rec);
POLY_FT4 *zhVsText(POLY_FT4 *prim, char *text, int32_t x, int32_t y, int32_t columns, int32_t scroll, int32_t height, int32_t depth);
#endif
