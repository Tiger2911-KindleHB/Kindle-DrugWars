#!/bin/sh
EXT_DIR="/mnt/us/extensions/kindledopewars"
APP="$EXT_DIR/bin/kindledopewars"
SAVE_DIR="$EXT_DIR/data"
LOG_FILE="$SAVE_DIR/kindledopewars.log"

mkdir -p "$SAVE_DIR"
{
  echo "=== Dope Wars Kindle launch $(date) ==="
  echo "EXT_DIR=$EXT_DIR"
  echo "APP=$APP"
  echo "PWD=$(pwd)"
  echo "USER=$(id 2>/dev/null || true)"
  echo "UNAME=$(uname -a 2>/dev/null || true)"
} >"$LOG_FILE" 2>&1

export DISPLAY=:0
export KINDLEDOPEWARS_SAVE="$SAVE_DIR/save.dat"
export HOME="$SAVE_DIR"
export GTK2_RC_FILES="$EXT_DIR/gtkrc"
export GDK_NATIVE_WINDOWS=1

if [ ! -x "$APP" ]; then
  echo "ERROR: app binary is missing or not executable: $APP" >>"$LOG_FILE" 2>&1
  ls -l "$EXT_DIR" "$EXT_DIR/bin" >>"$LOG_FILE" 2>&1 || true
  exit 127
fi

# Stop any stale copy before starting a new one from KUAL.
killall kindledopewars >>"$LOG_FILE" 2>&1 || true
sleep 1

# Keep the normal Kindle workflow: launch directly from KUAL, no WebView, no server.
eips -c >/dev/null 2>&1 || true
lipc-set-prop com.lab126.powerd preventScreenSaver 1 >/dev/null 2>&1 || true

cd "$EXT_DIR" || exit 1
"$APP" >>"$LOG_FILE" 2>&1
STATUS=$?
echo "Application exited with status $STATUS" >>"$LOG_FILE" 2>&1

lipc-set-prop com.lab126.powerd preventScreenSaver 0 >/dev/null 2>&1 || true
eips -c >/dev/null 2>&1 || true
exit $STATUS
