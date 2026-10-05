#pragma once
// Timer state lives in RTC memory: survives deep sleep, cleared on reset.
// Times are makeTime(RTC) seconds. Pure logic, tested on the host by tools/sim.
#include <stdint.h>
#include "ui.h"

#define MAX_TIMERS 8

struct Timer {
  uint32_t end;
  uint32_t duration;
  bool fired;
};

extern Timer timers[MAX_TIMERS]; // newest first
extern uint8_t timerCount;

void timerAdd(uint32_t now, uint32_t duration);
void timerRemove(uint8_t i);
uint8_t timersFire(uint32_t now); // marks expired timers fired, returns how many just fired
uint32_t timerNextEnd();          // earliest end among unfired timers, 0 = none
uint8_t timerViews(uint32_t now, TimerView *out, uint8_t max);
