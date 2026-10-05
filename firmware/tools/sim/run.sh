#!/bin/sh
# Host preview of src/ui.cpp. Needs `pio run` once (for Adafruit GFX), g++ and ImageMagick.
set -e
cd "$(dirname "$0")"
GFX="../../.pio/libdeps/watchy/Adafruit GFX Library"
mkdir -p out
g++ -std=gnu++17 -O1 -DARDUINO=200 -Ishim -I"$GFX" -o out/sim sim.cpp ../../src/ui.cpp "$GFX/Adafruit_GFX.cpp"
./out/sim
g++ -std=gnu++17 -DARDUINO=200 -Ishim -I"$GFX" -o out/test_timers test_timers.cpp ../../src/timers.cpp
./out/test_timers
g++ -std=gnu++17 -DARDUINO=200 -Ishim -I"$GFX" -o out/test_weather test_weather.cpp ../../src/weather.cpp
./out/test_weather
for f in out/*.pbm; do magick "$f" -scale 300% "${f%.pbm}.png"; rm "$f"; done
magick montage out/*.png -tile 4x -geometry +8+8 -background '#888' out/all.png
echo "wrote tools/sim/out/*.png"
