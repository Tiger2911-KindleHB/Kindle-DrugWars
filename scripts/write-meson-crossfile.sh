#!/usr/bin/env bash
set -euo pipefail

TC_DIR="${1:-$HOME/x-tools/arm-kindlehf-linux-gnueabihf}"
CROSS_FILE="$TC_DIR/meson-crosscompile.txt"
SYSROOT="$TC_DIR/arm-kindlehf-linux-gnueabihf/sysroot"

if [[ ! -d "$TC_DIR" ]]; then
  echo "Toolchain directory not found: $TC_DIR" >&2
  exit 1
fi

if [[ ! -d "$SYSROOT" ]]; then
  SYSROOT="$TC_DIR/sysroot"
fi

cat > "$CROSS_FILE" <<EOF
[binaries]
c = '$TC_DIR/bin/arm-kindlehf-linux-gnueabihf-gcc'
cpp = '$TC_DIR/bin/arm-kindlehf-linux-gnueabihf-g++'
ar = '$TC_DIR/bin/arm-kindlehf-linux-gnueabihf-ar'
strip = '$TC_DIR/bin/arm-kindlehf-linux-gnueabihf-strip'
pkgconfig = '$TC_DIR/bin/arm-kindlehf-linux-gnueabihf-pkg-config'

[host_machine]
system = 'linux'
cpu_family = 'arm'
cpu = 'armv7'
endian = 'little'

[properties]
sys_root = '$SYSROOT'
pkg_config_libdir = [
  '$SYSROOT/usr/lib/pkgconfig',
  '$SYSROOT/usr/share/pkgconfig',
  '$SYSROOT/lib/pkgconfig'
]
EOF

echo "$CROSS_FILE"
