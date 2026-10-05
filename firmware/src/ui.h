#pragma once
// Pure drawing: no hardware access, so tools/sim can render it to PNG on the host.
#include <Adafruit_GFX.h>

#define HOURLY_COUNT 6 // next 12h in 2h steps
#define FACE_TIMERS 3

struct HourWeather {
  uint8_t hour;
  int8_t temp;
  uint8_t code; // WMO weather code
  bool day;
};

struct Weather {
  bool valid;
  int8_t temp;
  uint8_t code;
  bool day;
  HourWeather hours[HOURLY_COUNT];
};

struct TimerView {
  uint32_t remaining; // seconds, 0 = done
  uint32_t duration;
};

struct FaceData {
  uint8_t hour, minute, wday, day, month; // wday: 1 = Sunday (TimeLib)
  uint32_t steps;
  Weather weather;
  TimerView timers[FACE_TIMERS]; // most recent first
  uint8_t timerCount;
};

void drawFace(Adafruit_GFX &d, const FaceData &f);
// sel 0 = "new timer" row, 1.. = timers[sel-1]
void drawTimerList(Adafruit_GFX &d, const TimerView *timers, uint8_t count, uint8_t sel);
// field 0..2 = h/m/s being edited
void drawTimerPicker(Adafruit_GFX &d, const uint8_t hms[3], uint8_t field);
