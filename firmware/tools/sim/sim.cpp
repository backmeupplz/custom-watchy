// Renders every screen in src/ui.cpp to PNG: tools/sim/run.sh
#include <stdio.h>
#include "../../src/ui.h"

static void save(GFXcanvas1 &c, const char *name) {
  char path[64];
  snprintf(path, sizeof path, "out/%s.pbm", name);
  FILE *f = fopen(path, "wb");
  fprintf(f, "P4\n200 200\n");
  // PBM: 1 = black; canvas: 1 = white (GxEPD_WHITE)
  for (int i = 0; i < 200 * 200 / 8; i++) fputc(~c.getBuffer()[i], f);
  fclose(f);
}

int main() {
  GFXcanvas1 c(200, 200);
  FaceData f = {14, 37, 2, 5, 10, 4231,
                {true, 10, 3, true,
                 {{16, 12, 2, true}, {18, 14, 3, true}, {20, 13, 61, false}, {22, 11, 63, false}, {0, 9, 0, false}, {2, 8, 45, false}}},
                {{252, 300}, {3900, 7200}, {0, 600}}, 3};
  drawFace(c, f);
  save(c, "face");
  f.timerCount = 0;
  drawFace(c, f);
  save(c, "face-notimers");

  FaceData g = {9, 58, 4, 30, 9, 128, {true, -3, 0, true,
                {{11, -1, 1, true}, {13, 1, 71, true}, {15, 2, 95, true}, {17, 0, 51, true}, {19, -2, 85, false}, {21, -4, 0, false}}},
                {{40, 60}}, 1};
  drawFace(c, g);
  save(c, "face-cold");

  FaceData h = {0, 0, 4, 1, 1, 0, {false}, {}, 0};
  drawFace(c, h);
  save(c, "face-noweather");
  h.wifi = true;
  drawFace(c, h);
  save(c, "face-noweather-wifi");

  TimerView t[] = {{252, 300}, {3900, 7200}, {0, 600}, {12, 90}, {0, 45}, {1000, 1500}, {0, 60}};
  drawTimerList(c, t, 7, 0);
  save(c, "timers");
  drawTimerList(c, t, 7, 2);
  save(c, "timers-sel");
  drawTimerList(c, t, 7, 7);
  save(c, "timers-scroll");

  uint8_t hms[] = {0, 5, 30};
  drawTimerPicker(c, hms, 1);
  save(c, "picker");
}
