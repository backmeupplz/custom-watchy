#pragma once
// Turns raw button levels into presses: each press counts once, Up/Down auto-repeat while held.
// Pure logic, tested on the host by tools/sim.
#include <stdint.h>

#define REPEAT_MS 500 // hold Up/Down this long to auto-repeat

enum Button { NONE, MENU, BACK, UP, DOWN };

struct Presses {
  Button held = NONE; // button down right now, already counted
  uint32_t since = 0;

  Button feed(Button raw, uint32_t ms) {
    if (raw != held) {
      held = raw;
      since = ms;
      return raw;
    }
    return raw != NONE && (raw == UP || raw == DOWN) && ms - since >= REPEAT_MS ? raw : NONE;
  }
};
