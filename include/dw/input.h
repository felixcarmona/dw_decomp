#ifndef DW_INPUT_H
#define DW_INPUT_H

#include <dw/version.h>

#if VERSION_REGION_IS(NTSCJ)
#define CONFIRM_BUTTON 0x20
#define CANCEL_BUTTON 0x40
#define ALT_BUTTON 0x10
#else
/*
 * MOD (options): OPTIONS' "JAPANESE CONTROLS" (src/main/mod_options.c) gives the three the Japanese disc's buttons, as
 * the masks above: ○ confirms, × cancels, △ is the third. A button the code tests by its bit (△ opening the field
 * menu, tamer.c) is the same on both discs and stays.
 */
extern int MOD_OPT_BUTTONS;
#define CONFIRM_BUTTON (MOD_OPT_BUTTONS ? 0x20 : 0x40)
#define CANCEL_BUTTON (MOD_OPT_BUTTONS ? 0x40 : 0x10)
#define ALT_BUTTON (MOD_OPT_BUTTONS ? 0x10 : 0x20)
#endif

#endif
