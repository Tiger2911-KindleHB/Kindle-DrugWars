#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 /path/to/kindledopewars-binary" >&2
  exit 2
fi

BIN="$1"
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DIST_DIR="$ROOT_DIR/dist"
PKG_DIR="$DIST_DIR/pkg"
EXT_NAME="kindledopewars"

rm -rf "$DIST_DIR"
mkdir -p "$PKG_DIR"
cp -R "$ROOT_DIR/extension/$EXT_NAME" "$PKG_DIR/$EXT_NAME"
mkdir -p "$PKG_DIR/$EXT_NAME/bin" "$PKG_DIR/$EXT_NAME/data"
cp "$BIN" "$PKG_DIR/$EXT_NAME/bin/kindledopewars"
chmod +x "$PKG_DIR/$EXT_NAME/bin/kindledopewars"
chmod +x "$PKG_DIR/$EXT_NAME/bin/launch.sh"

(
  cd "$PKG_DIR"
  zip -r "$DIST_DIR/${EXT_NAME}-kual.zip" "$EXT_NAME"
)

echo "Created $DIST_DIR/${EXT_NAME}-kual.zip"
