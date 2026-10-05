#!/bin/sh
# Flash the last `pio run` build of Watchy V3. The S3 is on USB only while awake, so this
# waits for the port and starts esptool instantly (pio upload is too slow to catch it).
# Press a button on the watch after starting. If it won't catch: hold Back+Up >4s, release Back first.
set -e
cd "$(dirname "$0")/.."
PORT=${1:-/dev/ttyACM0}
PY=$(head -1 "$(command -v pio)" | sed 's/^#!//' | cut -d' ' -f1)
B=.pio/build/watchy-v3
echo "waiting for $PORT..."
while [ ! -e "$PORT" ]; do sleep 0.02; done
"$PY" ~/.platformio/packages/tool-esptoolpy/esptool.py --chip esp32s3 --port "$PORT" --baud 921600 \
  --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect \
  0x0 $B/bootloader.bin 0x8000 $B/partitions.bin \
  0xe000 ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin 0x10000 $B/firmware.bin
