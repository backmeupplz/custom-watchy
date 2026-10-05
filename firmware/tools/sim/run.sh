#!/bin/sh
# Host tests + screenshots of the real drawing code (src/ui.cpp, ../src/Menu.cpp) into extras/custom/.
# Needs `pio run` once (for Adafruit GFX), g++ and ImageMagick.
set -e
cd "$(dirname "$0")"
GFX="../../.pio/libdeps/watchy-v3/Adafruit GFX Library"
SHOTS=../../../extras/custom
cxx() { g++ -std=gnu++17 -O1 -DARDUINO=200 -DARDUINO_WATCHY_V20 -Ishim -I"$GFX" "$@"; }
mkdir -p out "$SHOTS"
cxx -o out/test_timers test_timers.cpp ../../src/timers.cpp && ./out/test_timers
cxx -o out/test_weather test_weather.cpp ../../src/weather.cpp && ./out/test_weather
cxx -o out/sim sim.cpp ../../src/ui.cpp ../../../src/Menu.cpp "$GFX/Adafruit_GFX.cpp"
rm -f out/*.pbm "$SHOTS"/*.png
./out/sim
for f in out/*.pbm; do n=$(basename "$f" .pbm); magick "$f" -scale 300% "$SHOTS/$n.png"; done
magick montage "$SHOTS"/*.png -tile 6x -geometry +8+8 -background '#888' out/all.png
echo "wrote extras/custom/*.png (overview: tools/sim/out/all.png)"
