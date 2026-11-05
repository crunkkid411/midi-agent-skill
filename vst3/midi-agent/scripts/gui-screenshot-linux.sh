#!/usr/bin/env bash
# Open the plug-in editor without a DAW and write a screenshot.
#   VST3_SDK_DIR=~/vst3sdk vst3/midi-agent/scripts/gui-screenshot-linux.sh out.png
# Needs: the Linux build (scripts/build-linux.sh), Xvfb, xdotool, imagemagick, gcc, libx11-dev.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
SDK="${VST3_SDK_DIR:?set VST3_SDK_DIR}"
BUILD="${BUILD_DIR:-$ROOT/build/midi-agent}"
CONFIG="${CONFIG:-Release}"
SDK_BUILD="${SDK_BUILD_DIR:-$ROOT/build/vst3sdk-hosting}"
OUT="${1:-$ROOT/build/midi-agent-editor.png}"
DISPLAY_NO="${DISPLAY_NO:-:99}"

# 1. the SDK's editorhost sample (its own build tree, hosting examples on)
if [ ! -x "$SDK_BUILD/bin/$CONFIG/editorhost" ]; then
    cmake -S "$SDK" -B "$SDK_BUILD" -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG" \
          -DSMTG_ENABLE_VST3_PLUGIN_EXAMPLES=OFF -DSMTG_ENABLE_VST3_HOSTING_EXAMPLES=ON \
          -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_CREATE_PLUGIN_LINK=OFF
    cmake --build "$SDK_BUILD" --target editorhost -j"$(nproc)"
fi

# 2. a virtual display, and a client that interns _XEMBED_INFO (editorhost needs it)
Xvfb "$DISPLAY_NO" -screen 0 1400x900x24 >/dev/null 2>&1 &
XVFB=$!
sleep 1
export DISPLAY="$DISPLAY_NO"
TMP="$(mktemp -d)"
cat > "$TMP/xatom.c" <<'C'
#include <X11/Xlib.h>
#include <unistd.h>
int main(void){Display*d=XOpenDisplay(0);if(!d)return 1;XInternAtom(d,"_XEMBED_INFO",0);XFlush(d);sleep(600);return 0;}
C
gcc -o "$TMP/xatom" "$TMP/xatom.c" -lX11
"$TMP/xatom" &
XATOM=$!
sleep 1

# 3. open the editor with the fixture, wait for it to draw, screenshot
export MIDIAGENT_AUTOLOAD="${MIDIAGENT_AUTOLOAD:-$ROOT/vst3/midi-agent/fixtures/scenecore-8bar.composition.json}"
export MIDIAGENT_SKILL_ROOT="${MIDIAGENT_SKILL_ROOT:-$ROOT}"
"$SDK_BUILD/bin/$CONFIG/editorhost" "$BUILD/VST3/$CONFIG/MIDI Agent.vst3" &
HOST=$!
sleep 8
# example interactions: select segment B, select the Bass track, add a note
# xdotool mousemove 930 114 click 1; xdotool mousemove 90 290 click 1; xdotool mousemove 700 420 click 1
import -window root "$OUT"
kill $HOST $XATOM $XVFB 2>/dev/null || true
rm -rf "$TMP"
echo "wrote $OUT"
