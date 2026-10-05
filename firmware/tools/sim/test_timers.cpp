// Host check for src/timers.cpp, run by tools/sim/run.sh
#include <assert.h>
#include <stdio.h>
#include "../../src/timers.h"

int main() {
  timerAdd(1000, 60);  // ends 1060
  timerAdd(1010, 300); // ends 1310, newest
  timerAdd(1020, 30);  // ends 1050, newest
  assert(timerCount == 3 && timers[0].duration == 30 && timers[2].duration == 60);
  assert(timerNextEnd() == 1050);

  assert(timersFire(1049) == 0);
  assert(timersFire(1055) == 1);
  assert(timersFire(1056) == 0); // fires once
  assert(timerNextEnd() == 1060);
  assert(timersFire(2000) == 2);
  assert(timerNextEnd() == 0);

  TimerView v[3];
  timerAdd(2000, 90);
  assert(timerViews(2010, v, 3) == 3);
  assert(v[0].remaining == 80 && v[0].duration == 90);
  assert(v[1].remaining == 0 && v[1].duration == 30);

  timerRemove(0);
  assert(timerCount == 3 && timers[0].duration == 30);
  for (int i = 0; i < 20; i++) timerAdd(3000, i + 1);
  assert(timerCount == MAX_TIMERS && timers[0].duration == 20);
  puts("timers ok");
}
