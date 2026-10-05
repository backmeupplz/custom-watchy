#include "timers.h"
#include <string.h>
#ifdef ESP32
#include <esp_attr.h>
#else
#define RTC_DATA_ATTR
#endif

RTC_DATA_ATTR Timer timers[MAX_TIMERS];
RTC_DATA_ATTR uint8_t timerCount;

void timerAdd(uint32_t now, uint32_t duration) {
  // ponytail: when full, the oldest timer is dropped even if still running
  memmove(&timers[1], &timers[0], sizeof(Timer) * (MAX_TIMERS - 1));
  timers[0] = {now + duration, duration, false};
  if (timerCount < MAX_TIMERS) timerCount++;
}

void timerRemove(uint8_t i) {
  if (i >= timerCount) return;
  memmove(&timers[i], &timers[i + 1], sizeof(Timer) * (timerCount - i - 1));
  timerCount--;
}

uint8_t timersFire(uint32_t now) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < timerCount; i++) {
    if (!timers[i].fired && timers[i].end <= now) {
      timers[i].fired = true;
      n++;
    }
  }
  return n;
}

uint32_t timerNextEnd() {
  uint32_t next = 0;
  for (uint8_t i = 0; i < timerCount; i++) {
    if (!timers[i].fired && (!next || timers[i].end < next)) next = timers[i].end;
  }
  return next;
}

uint8_t timerViews(uint32_t now, TimerView *out, uint8_t max) {
  uint8_t n = timerCount < max ? timerCount : max;
  for (uint8_t i = 0; i < n; i++) {
    // fired means done even if the clock jumped back (e.g. NTP sync)
    out[i] = {timers[i].fired || timers[i].end <= now ? 0 : timers[i].end - now, timers[i].duration};
  }
  return n;
}
