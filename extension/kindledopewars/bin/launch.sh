#!/bin/sh
EXT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
APP="$EXT_DIR/bin/kindledopewars"
SAVE_DIR="$EXT_DIR/data"
LOG_FILE="$SAVE_DIR/kindledopewars.log"

mkdir -p "$SAVE_DIR"
export KINDLEDOPEWARS_SAVE="$SAVE_DIR/save.dat"
export HOME="$SAVE_DIR"
export GTK2_RC_FILES="$EXT_DIR/gtkrc"

# Keep the normal Kindle workflow: launch directly from KUAL, no WebView, no server.
# eips calls are best-effort; older/newer firmware may ignore some arguments.
eips -c >/dev/null 2>&1 || true
lipc-set-prop com.lab126.powerd preventScreenSaver 1 >/dev/null 2>&1 || true

cd "$EXT_DIR" || exit 1
"$APP" >"$LOG_FILE" 2>&1
STATUS=$?

lipc-set-prop com.lab126.powerd preventScreenSaver 0 >/dev/null 2>&1 || true
eips -c >/dev/null 2>&1 || true
exit $STATUS
