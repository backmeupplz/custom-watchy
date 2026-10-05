#include "ui.h"
#include <stdio.h>
#include "fonts/FontTime.h"
#include "fonts/FontPick.h"
#include "fonts/FontStat.h"
#include "fonts/FontLabel.h"
#include "fonts/FontSmall.h"

#define INK 0x0000   // GxEPD_BLACK
#define PAPER 0xFFFF // GxEPD_WHITE
#define DEG "`"      // fontgen renders '`' as a degree sign

static const char *const DAYS[] = {"", "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
static const char *const MONTHS[] = {"", "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                     "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

enum Align { LEFT, CENTER, RIGHT };

static int16_t textWidth(const GFXfont *f, const char *s, int8_t track) {
  int16_t w = 0;
  for (const char *p = s; *p; p++)
    if ((uint8_t)*p >= f->first && (uint8_t)*p <= f->last)
      w += f->glyph[*p - f->first].xAdvance + track;
  return w ? w - track : 0;
}

// Draws s with baseline y; track = extra pixels between letters.
static int16_t text(Adafruit_GFX &d, int16_t x, int16_t y, const char *s, const GFXfont *f,
                    Align a = LEFT, int8_t track = 0, uint16_t color = INK) {
  int16_t w = textWidth(f, s, track);
  if (a == CENTER) x -= w / 2;
  else if (a == RIGHT) x -= w;
  d.setFont(f);
  d.setTextColor(color);
  for (; *s; s++) {
    if ((uint8_t)*s < f->first || (uint8_t)*s > f->last) continue;
    d.setCursor(x, y);
    d.write(*s);
    x += f->glyph[*s - f->first].xAdvance + track;
  }
  return w;
}

static void dottedLine(Adafruit_GFX &d, int16_t y) {
  for (int16_t x = 4; x < 196; x += 3) d.drawPixel(x, y, INK);
}

static void thickLine(Adafruit_GFX &d, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t t, uint16_t c) {
  for (uint8_t i = 0; i < t; i++) {
    d.drawLine(x0 + i, y0, x1 + i, y1, c);
  }
}

// "12,345"
static void thousands(char *buf, uint32_t n) {
  if (n >= 1000) sprintf(buf, "%lu,%03lu", (unsigned long)(n / 1000), (unsigned long)(n % 1000));
  else sprintf(buf, "%lu", (unsigned long)n);
}

// Compact duration: "45s", "12m", "1h05". Minutes round up, so "1m" means under a minute left.
static void shortDur(char *buf, uint32_t s) {
  if (s < 60) sprintf(buf, "%lus", (unsigned long)s);
  else if (s < 3600) sprintf(buf, "%lum", (unsigned long)((s + 59) / 60));
  else sprintf(buf, "%luh%02lu", (unsigned long)((s + 59) / 3600), (unsigned long)((s + 59) / 60 % 60));
}

// "1:05:09" / "5:09"
static void clockDur(char *buf, uint32_t s) {
  if (s >= 3600) sprintf(buf, "%lu:%02lu:%02lu", (unsigned long)(s / 3600), (unsigned long)(s / 60 % 60), (unsigned long)(s % 60));
  else sprintf(buf, "%lu:%02lu", (unsigned long)(s / 60), (unsigned long)(s % 60));
}

/* ---------------- weather icons, drawn from primitives so they scale ---------------- */

static void cloud(Adafruit_GFX &d, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) {
  d.fillRoundRect(x, y + h / 2, w, h - h / 2, (h - h / 2) / 2, c);
  d.fillCircle(x + w * 3 / 10, y + h * 6 / 10, h * 3 / 10, c);
  d.fillCircle(x + w * 6 / 10, y + h * 45 / 100, h * 45 / 100, c);
}

static void outlinedCloud(Adafruit_GFX &d, int16_t x, int16_t y, int16_t w, int16_t h) {
  cloud(d, x - 2, y - 2, w + 4, h + 4, PAPER);
  cloud(d, x, y, w, h, INK);
}

static void sun(Adafruit_GFX &d, int16_t cx, int16_t cy, int16_t r, uint8_t t) {
  static const int8_t dir[8][2] = {{10, 0}, {7, 7}, {0, 10}, {-7, 7}, {-10, 0}, {-7, -7}, {0, -10}, {7, -7}};
  d.fillCircle(cx, cy, r, INK);
  int16_t a = r + 2 + t, b = r + 2 + t + r * 2 / 3;
  for (auto &v : dir) {
    int16_t x0 = cx + v[0] * a / 10, y0 = cy + v[1] * a / 10;
    int16_t x1 = cx + v[0] * b / 10, y1 = cy + v[1] * b / 10;
    for (uint8_t i = 0; i < t; i++) {
      d.drawLine(x0 + i, y0, x1 + i, y1, INK);
      d.drawLine(x0, y0 + i, x1, y1 + i, INK);
    }
  }
}

static void moon(Adafruit_GFX &d, int16_t cx, int16_t cy, int16_t r) {
  d.fillCircle(cx, cy, r, INK);
  d.fillCircle(cx + r * 6 / 10, cy - r * 4 / 10, r * 8 / 10, PAPER);
}

static void sky(Adafruit_GFX &d, int16_t cx, int16_t cy, int16_t r, bool day, uint8_t t) {
  if (day) sun(d, cx, cy, r, t);
  else moon(d, cx, cy, r + r / 2);
}

// Icon centered on (cx, cy) inside an s x s box.
static void weatherIcon(Adafruit_GFX &d, int16_t cx, int16_t cy, int16_t s, uint8_t code, bool day) {
  int16_t x = cx - s / 2, y = cy - s / 2;
  uint8_t t = s >= 24 ? 2 : 1;
  if (code == 0) { // clear
    sky(d, cx, cy, s / 5, day, t);
  } else if (code <= 2) { // mainly clear / partly cloudy
    sky(d, x + s * 4 / 10, y + s * 4 / 10, s / 6, day, t);
    outlinedCloud(d, x + s / 4, y + s * 45 / 100, s * 3 / 4, s / 2);
  } else if (code == 3) { // overcast
    cloud(d, x, y + s / 5, s, s * 6 / 10, INK);
  } else if (code == 45 || code == 48) { // fog
    for (uint8_t i = 0; i < 4; i++) {
      int16_t inset = (i % 2) * s / 6;
      d.fillRoundRect(x + inset, y + s / 6 + i * s / 5, s - s / 6, t + 1, 1, INK);
    }
  } else {
    cloud(d, x, y + s / 10, s, s * 55 / 100, INK);
    int16_t top = y + s * 72 / 100, bot = y + s;
    if (code >= 95) { // thunderstorm: bolt
      int16_t bx = cx;
      d.fillTriangle(bx + s / 8, top - 2, bx - s / 8, top + (bot - top) / 2 + 1, bx + 1, top + (bot - top) / 2 + 1, INK);
      d.fillTriangle(bx - 1, top + (bot - top) / 2 - 1, bx + s / 8, top + (bot - top) / 2 - 1, bx - s / 8, bot + 2, INK);
    } else if ((code >= 71 && code <= 77) || code == 85 || code == 86) { // snow
      for (int8_t i = -1; i <= 1; i++) {
        int16_t fx = cx + i * s / 3, fy = top + (bot - top) / 2 + (i == 0 ? 1 : -1) * s / 12;
        int16_t r = s / 10 + 1;
        d.drawLine(fx - r, fy, fx + r, fy, INK);
        d.drawLine(fx, fy - r, fx, fy + r, INK);
        d.drawLine(fx - r * 7 / 10, fy - r * 7 / 10, fx + r * 7 / 10, fy + r * 7 / 10, INK);
        d.drawLine(fx - r * 7 / 10, fy + r * 7 / 10, fx + r * 7 / 10, fy - r * 7 / 10, INK);
      }
    } else if (code >= 51 && code <= 57) { // drizzle: dots
      for (int8_t i = -1; i <= 1; i++) {
        d.fillCircle(cx + i * s / 3, top + (i == 0 ? (bot - top) * 2 / 3 : (bot - top) / 3), t, INK);
      }
    } else { // rain / showers
      for (int8_t i = -1; i <= 1; i++) {
        int16_t rx = cx + i * s / 3;
        thickLine(d, rx + s / 10, top, rx - s / 10, bot, t, INK);
      }
    }
  }
}

static const char *condition(uint8_t code) {
  if (code == 0) return "CLEAR";
  if (code == 1) return "MOSTLY CLEAR";
  if (code == 2) return "PARTLY CLOUDY";
  if (code == 3) return "CLOUDY";
  if (code == 45 || code == 48) return "FOG";
  if (code <= 57) return "DRIZZLE";
  if (code <= 67) return "RAIN";
  if (code <= 77) return "SNOW";
  if (code <= 82) return "SHOWERS";
  if (code <= 86) return "SNOW";
  return "STORM";
}

static void hourglass(Adafruit_GFX &d, int16_t x, int16_t y, uint16_t c) { // 7x9 at top-left x,y
  d.drawFastHLine(x, y, 7, c);
  d.drawFastHLine(x, y + 8, 7, c);
  d.fillTriangle(x + 1, y + 1, x + 5, y + 1, x + 3, y + 4, c);
  d.fillTriangle(x + 3, y + 4, x + 1, y + 7, x + 5, y + 7, c);
}

static void check(Adafruit_GFX &d, int16_t x, int16_t y, uint16_t c) { // ~8x7 at top-left x,y
  thickLine(d, x, y + 3, x + 2, y + 6, 2, c);
  thickLine(d, x + 2, y + 6, x + 7, y, 2, c);
}

/* ---------------- watch face ---------------- */

void drawFace(Adafruit_GFX &d, const FaceData &f) {
  char buf[24];
  d.fillScreen(PAPER);
  // Without timer pills, spread their row's space over the sections above (gaps grow downwards)
  int16_t s = f.timerCount ? 0 : 1;

  // Date header
  text(d, 4, 15, DAYS[f.wday], &FontLabel, LEFT, 1);
  sprintf(buf, "%s  %u", MONTHS[f.month], f.day);
  text(d, 196, 15, buf, &FontLabel, RIGHT, 1);

  // Time
  sprintf(buf, "%02u:%02u", f.hour, f.minute);
  text(d, 100, 74 + 4 * s, buf, &FontTime, CENTER, -1);

  // Current weather (left) and steps (right)
  const Weather &w = f.weather;
  if (w.valid) {
    weatherIcon(d, 19, 98 + 9 * s, 30, w.code, w.day);
    sprintf(buf, "%d" DEG, w.temp);
    text(d, 40, 104 + 9 * s, buf, &FontStat);
    text(d, 41, 117 + 9 * s, condition(w.code), &FontSmall, LEFT, 1);
  } else {
    text(d, 4, 104 + 9 * s, "--" DEG, &FontStat);
    text(d, 5, 117 + 9 * s, "NO WEATHER", &FontSmall, LEFT, 1);
  }
  thousands(buf, f.steps);
  text(d, 196, 104 + 9 * s, buf, &FontStat, RIGHT);
  text(d, 195, 117 + 9 * s, "STEPS", &FontSmall, RIGHT, 1);

  dottedLine(d, 125 + 12 * s);

  // Hourly forecast
  if (w.valid) {
    for (uint8_t i = 0; i < HOURLY_COUNT; i++) {
      const HourWeather &h = w.hours[i];
      int16_t cx = 17 + i * 33;
      sprintf(buf, "%02u", h.hour);
      text(d, cx, 140 + 14 * s, buf, &FontSmall, CENTER);
      weatherIcon(d, cx, 153 + 16 * s, 18, h.code, h.day);
      sprintf(buf, "%d" DEG, h.temp);
      text(d, cx + 2, 176 + 18 * s, buf, &FontSmall, CENTER);
    }
  } else {
    text(d, 100, 156 + 16 * s, f.wifi ? "WEATHER UNAVAILABLE" : "NO WIFI: MENU > SETUP WIFI", &FontSmall, CENTER, 1);
  }

  // Most recent timers
  for (uint8_t i = 0; i < f.timerCount && i < FACE_TIMERS; i++) {
    const TimerView &t = f.timers[i];
    int16_t x = 4 + i * 65;
    bool done = t.remaining == 0;
    shortDur(buf, done ? t.duration : t.remaining);
    if (done) {
      d.drawRoundRect(x, 182, 62, 18, 9, INK);
      check(d, x + 9, 187, INK);
    } else {
      d.fillRoundRect(x, 182, 62, 18, 9, INK);
      hourglass(d, x + 9, 186, PAPER);
    }
    text(d, x + 37, 195, buf, &FontLabel, CENTER, 0, done ? INK : PAPER);
  }
}

/* ---------------- timer app ---------------- */

// Labels next to the physical buttons: back top-left, menu bottom-left, up/down on the right.
static void buttonHints(Adafruit_GFX &d, const char *menu, const char *back) {
  text(d, 4, 12, back, &FontSmall, LEFT, 1);
  text(d, 4, 196, menu, &FontSmall, LEFT, 1);
  d.fillTriangle(190, 10, 196, 10, 193, 5, INK);
  d.fillTriangle(190, 189, 196, 189, 193, 194, INK);
}

void drawTimerList(Adafruit_GFX &d, const TimerView *timers, uint8_t count, uint8_t sel) {
  char buf[24];
  d.fillScreen(PAPER);
  buttonHints(d, sel == 0 ? "NEW" : "CANCEL", "EXIT");
  text(d, 100, 13, "TIMERS", &FontLabel, CENTER, 2);

  const uint8_t rows = 6, rowH = 26, top = 22;
  uint8_t first = sel >= rows ? sel - rows + 1 : 0;
  for (uint8_t r = 0; r < rows && first + r <= count; r++) {
    uint8_t i = first + r;
    int16_t y = top + r * rowH;
    bool selected = i == sel;
    uint16_t fg = selected ? PAPER : INK;
    if (selected) d.fillRoundRect(2, y, 196, rowH - 2, 6, INK);
    if (i == 0) {
      text(d, 12, y + 17, "+  NEW TIMER", &FontLabel, LEFT, 1, fg);
      continue;
    }
    const TimerView &t = timers[i - 1];
    if (t.remaining) {
      hourglass(d, 12, y + 7, fg);
      clockDur(buf, t.remaining);
      text(d, 26, y + 18, buf, &FontStat, LEFT, 0, fg);
    } else {
      check(d, 12, y + 8, fg);
      text(d, 26, y + 17, "DONE", &FontLabel, LEFT, 1, fg);
    }
    shortDur(buf, t.duration);
    text(d, 186, y + 17, buf, &FontLabel, RIGHT, 0, fg);
  }
}

void drawTimerPicker(Adafruit_GFX &d, const uint8_t hms[3], uint8_t field) {
  static const char *const LABELS[] = {"HRS", "MIN", "SEC"};
  char buf[4];
  d.fillScreen(PAPER);
  buttonHints(d, field == 2 ? "START" : "NEXT", field == 0 ? "CANCEL" : "BACK");
  text(d, 100, 52, "NEW TIMER", &FontLabel, CENTER, 2);

  for (uint8_t i = 0; i < 3; i++) {
    int16_t cx = 38 + i * 62;
    sprintf(buf, "%02u", hms[i]);
    text(d, cx, 116, buf, &FontPick, CENTER);
    text(d, cx, 146, LABELS[i], &FontSmall, CENTER, 1);
    if (i == field) d.fillRoundRect(cx - 22, 124, 44, 4, 2, INK);
    if (i < 2) text(d, cx + 31, 112, ":", &FontPick, CENTER);
  }
}
