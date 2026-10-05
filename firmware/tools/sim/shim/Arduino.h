// Minimal Arduino shim so Adafruit_GFX + src/ui.cpp build on the host (tools/sim).
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <math.h>
#define radians(d) ((d) * M_PI / 180.0)
#include "Print.h"
#define PROGMEM
typedef bool boolean;
class __FlashStringHelper;
class String : public std::string {
public:
  using std::string::string;
};
