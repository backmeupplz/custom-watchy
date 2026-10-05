// Host check for weatherView() in src/weather.cpp, run by tools/sim/run.sh
#include <assert.h>
#include <stdio.h>
#include "../../src/weather.h"

int main() {
  assert(!weatherView(1000).valid); // never fetched
  uint32_t start = 20000 * 86400 + 14 * 3600; // 14:00 local
  forecast = {start + 37 * 60, start, 10, 3, true};
  for (int i = 0; i < FORECAST_HOURS; i++) forecast.temp[i] = i, forecast.code[i] = i;

  Weather w = weatherView(start + 37 * 60); // 14:37, fresh
  assert(w.valid && w.temp == 10 && w.code == 3);
  assert(w.hours[0].hour == 16 && w.hours[0].temp == 2);
  assert(w.hours[5].hour == 2 && w.hours[5].temp == 12); // wraps midnight

  w = weatherView(start + 3 * 3600 + 5); // 17:00, stale: current from hourly slot
  assert(w.valid && w.temp == 3 && w.hours[0].hour == 19 && w.hours[0].temp == 5);

  assert(weatherView(start + 12 * 3600 + 59 * 60).valid);
  assert(!weatherView(start + 13 * 3600).valid); // forecast ran out
  puts("weather ok");
}
