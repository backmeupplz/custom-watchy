// Host check for src/buttons.h, run by tools/sim/run.sh
#include <assert.h>
#include <stdio.h>
#include "../../src/buttons.h"

int main() {
  Presses p;
  assert(p.feed(NONE, 0) == NONE);
  assert(p.feed(DOWN, 10) == DOWN);   // press counts once...
  assert(p.feed(DOWN, 200) == NONE);  // ...not again while held through a refresh
  assert(p.feed(NONE, 250) == NONE);
  assert(p.feed(DOWN, 300) == DOWN);  // next press counts

  assert(p.feed(DOWN, 799) == NONE);
  assert(p.feed(DOWN, 800) == DOWN);  // held 500ms: auto-repeat
  assert(p.feed(DOWN, 1100) == DOWN);

  assert(p.feed(MENU, 1200) == MENU); // switching buttons counts as a new press
  assert(p.feed(MENU, 5000) == NONE); // Menu/Back never repeat
  puts("buttons ok");
}
