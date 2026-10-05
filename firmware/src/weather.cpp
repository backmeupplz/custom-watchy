#include "weather.h"
#ifdef ESP32
#include <esp_attr.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#else
#define RTC_DATA_ATTR
#endif

RTC_DATA_ATTR Forecast forecast;

#ifdef ESP32
bool weatherFetch(const char *lat, const char *lon, int32_t &utcOffset) {
  char url[320];
  snprintf(url, sizeof url,
           "http://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s"
           "&current=temperature_2m,weather_code,is_day&hourly=temperature_2m,weather_code,is_day"
           "&forecast_hours=%d&timezone=auto&timeformat=unixtime",
           lat, lon, FORECAST_HOURS);
  HTTPClient http;
  http.setConnectTimeout(3000);
  http.begin(url);
  bool ok = http.GET() == 200;
  if (ok) {
    JSONVar r = JSON.parse(http.getString());
    JSONVar hourly = r["hourly"];
    ok = JSON.typeof(hourly) == "object" && hourly["time"].length() == FORECAST_HOURS;
    if (ok) {
      utcOffset = (int)r["utc_offset_seconds"];
      forecast.fetched = (uint32_t)(double)r["current"]["time"] + utcOffset;
      forecast.start = (uint32_t)(double)hourly["time"][0] + utcOffset;
      forecast.curTemp = lround((double)r["current"]["temperature_2m"]);
      forecast.curCode = (int)r["current"]["weather_code"];
      forecast.curDay = (int)r["current"]["is_day"];
      for (int i = 0; i < FORECAST_HOURS; i++) {
        forecast.temp[i] = lround((double)hourly["temperature_2m"][i]);
        forecast.code[i] = (int)hourly["weather_code"][i];
        forecast.day[i] = (int)hourly["is_day"][i];
      }
    }
  }
  http.end();
  return ok;
}
#endif

Weather weatherView(uint32_t now) {
  Weather w = {};
  if (!forecast.fetched || now < forecast.start) return w;
  uint32_t k = (now - forecast.start) / 3600; // slot containing now
  if (k + 2 * HOURLY_COUNT >= FORECAST_HOURS) return w;
  bool fresh = now - forecast.fetched < 3600;
  w.temp = fresh ? forecast.curTemp : forecast.temp[k];
  w.code = fresh ? forecast.curCode : forecast.code[k];
  w.day = fresh ? forecast.curDay : forecast.day[k];
  for (uint8_t i = 0; i < HOURLY_COUNT; i++) {
    uint32_t j = k + 2 * (i + 1);
    w.hours[i] = {(uint8_t)((forecast.start + j * 3600) % 86400 / 3600), forecast.temp[j], forecast.code[j], forecast.day[j]};
  }
  w.valid = true;
  return w;
}
