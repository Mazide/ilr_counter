#!/usr/bin/env bash
set -euo pipefail
task_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
task_cli="${ARDUINO_CLI:-arduino-cli}"
if ! command -v "$task_cli" >/dev/null 2>&1; then
  task_cli=/opt/arduino-ide/resources/app/lib/backend/resources/arduino-cli
fi
task_build="$(mktemp -d /tmp/ilr_countdown_c3.XXXXXX)"
mkdir -p "$task_root/firmware"
printf '#define WM_STRINGS_FILE "%s"\n' "$task_root/localization/wm_strings_ru.h" > "$task_build/wm_language.h"
"$task_cli" compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc \
  --build-property "compiler.cpp.extra_flags=-include $task_build/wm_language.h" \
  --build-path "$task_build/build" --output-dir "$task_build/output" "$task_root" \
  > "$task_root/firmware/build.log" 2>&1
cp "$task_build/output/ilr_countdown_c3.ino.merged.bin" "$task_root/firmware/ilr_countdown_c3.merged.bin"
cp "$task_build/output/ilr_countdown_c3.ino.bin" "$task_root/firmware/ilr_countdown_c3.app.bin"
cp "$task_build/output/ilr_countdown_c3.ino.partitions.bin" "$task_root/firmware/ilr_countdown_c3.partitions.bin"
cat "$task_root/firmware/build.log"
