#!/usr/bin/env bash
# Build MIDI Agent on Linux (compile check, tests, validator).
#   VST3_SDK_DIR=~/vst3sdk vst3/midi-agent/scripts/build-linux.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
SDK="${VST3_SDK_DIR:-}"
BUILD="${BUILD_DIR:-$ROOT/build/midi-agent}"
CONFIG="${CONFIG:-Release}"
if [ -z "$SDK" ]; then
    echo "set VST3_SDK_DIR to the vst3sdk root (clone it with --recursive)" >&2
    exit 2
fi
cmake -S "$ROOT/vst3/midi-agent" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG" \
      -DSMTG_VST3_SDK_DIR="$SDK" -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_CREATE_PLUGIN_LINK=OFF
cmake --build "$BUILD" -j"$(nproc)"
ctest --test-dir "$BUILD" --output-on-failure
cmake --build "$BUILD" --target validator -j"$(nproc)"
"$BUILD/bin/$CONFIG/validator" "$BUILD/VST3/$CONFIG/MIDI Agent.vst3" | tail -4
