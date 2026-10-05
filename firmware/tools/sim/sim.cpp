// Renders screens from the real drawing code to extras/custom/*.png: tools/sim/run.sh
#include <stdio.h>
#include "../../src/ui.h"
#include "../../../src/Menu.h"

static GFXcanvas1 c(200, 200);

static void save(const char *name) {
  char path[64];
  snprintf(path, sizeof path, "out/%s.pbm", name);
  FILE *f = fopen(path, "wb");
  fprintf(f, "P4\n200 200\n");
  // PBM: 1 = black; canvas: 1 = white (GxEPD_WHITE)
  for (int i = 0; i < 200 * 200 / 8; i++) fputc(~c.getBuffer()[i], f);
  fclose(f);
}

static void face(const char *name, const FaceData &f) {
  drawFace(c, f);
  save(name);
}

int main() {
  // hour, minute, wday (1 = Sun), day, month, steps, weather, timers, timerCount, wifi
  FaceData day = {14, 37, 2, 5, 10, 4231,
                  {true, 10, 3, true,
                   {{16, 12, 2, true}, {18, 14, 3, true}, {20, 13, 61, false}, {22, 11, 63, false}, {0, 9, 0, false}, {2, 8, 45, false}}},
                  {{252, 300}, {3900, 7200}, {0, 600}}, 3, true};
  face("face", day);
  day.timerCount = 0;
  face("face-notimers", day);

  face("face-snow", {8, 5, 4, 14, 1, 128,
                     {true, -3, 73, true,
                      {{10, -2, 71, true}, {12, -1, 3, true}, {14, 1, 2, true}, {16, 0, 85, true}, {18, -2, 0, false}, {20, -4, 0, false}}},
                     {{1500, 1800}}, 1, true});

  face("face-night", {23, 12, 6, 17, 7, 12408,
                      {true, 18, 0, false,
                       {{1, 16, 0, false}, {3, 15, 1, false}, {5, 14, 2, false}, {7, 16, 1, true}, {9, 19, 0, true}, {11, 23, 0, true}}},
                      {{0, 2700}, {0, 45}}, 2, true});

  face("face-storm", {7, 45, 3, 3, 11, 2050,
                      {true, 8, 81, true,
                       {{9, 9, 95, true}, {11, 10, 61, true}, {13, 11, 53, true}, {15, 11, 3, true}, {17, 9, 45, false}, {19, 7, 2, false}}},
                      {}, 0, true});

  face("face-nowifi", {0, 0, 5, 1, 1, 0, {false}, {}, 0, false});
  face("face-noweather", {9, 30, 5, 1, 1, 312, {false}, {}, 0, true});

  TimerView t[] = {{252, 300}, {3900, 7200}, {0, 600}, {12, 90}, {0, 45}, {1000, 1500}, {0, 60}};
  drawTimerList(c, t, 7, 0);
  save("timers");
  drawTimerList(c, t, 7, 2);
  save("timers-selected");
  drawTimerList(c, t, 7, 7);
  save("timers-scrolled");
  drawTimerList(c, t, 0, 0);
  save("timers-empty");

  uint8_t hms[] = {0, 25, 0};
  drawTimerPicker(c, hms, 1);
  save("picker");
  uint8_t hms2[] = {1, 30, 15};
  drawTimerPicker(c, hms2, 2);
  save("picker-start");

  drawMenu(c, 0);
  save("menu");
  drawMenu(c, 6);
  save("menu-refresh");
}
