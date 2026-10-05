#ifndef WATCHY_MENU_H
#define WATCHY_MENU_H
// Menu drawing only (no hardware), so firmware/tools/sim can render it.
#include <Adafruit_GFX.h>

void drawMenu(Adafruit_GFX &d, int menuIndex);

#endif
