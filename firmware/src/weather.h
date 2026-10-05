#pragma once
// Open-Meteo forecast (free, no API key), kept in RTC memory.
#include <stdint.h>
#include "ui.h"

#define FORECAST_HOURS 25

struct Forecast {
  uint32_t fetched; // local time of fetch, 0 = never
  uint32_t start;   // local time of temp[0]
  int8_t curTemp;
  uint8_t curCode;
  bool curDay;
  int8_t temp[FORECAST_HOURS];
  uint8_t code[FORECAST_HOURS];
  bool day[FORECAST_HOURS];
};

extern Forecast forecast;

// Needs WiFi up. utcOffset gets the location's offset incl. DST, for NTP.
bool weatherFetch(const char *lat, const char *lon, int32_t &utcOffset);
// Current conditions + next 12h in 2h steps as of local time now; invalid if the forecast ran out.
Weather weatherView(uint32_t now);
