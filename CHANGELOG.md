# Changelog

## 0.1.4 - KUAL Launch / Home Screen Takeover Fix

- Changed KUAL `menu.json` to call the launch script by absolute path: `/mnt/us/extensions/kindledopewars/bin/launch.sh`.
- Hardened the launch script with `DISPLAY=:0`, absolute extension paths, stale-process cleanup, and a real launch log at `/mnt/us/extensions/kindledopewars/data/kindledopewars.log`.
- Changed the GTK window from an ordinary toplevel to a popup/override-redirect full-screen window so Kindle Home is less likely to sit on top of the app.
- Forced the app window to position `0,0`, use the detected Kindle screen size, stay above other windows, and present itself after drawing.
- Added static runtime link arguments for `libstdc++` and `libgcc` when supported by the kindlehf compiler to reduce missing-runtime-library failures on device.

## 0.1.3 - KUAL Discovery Fix

- Added the required `<menus>` block to `extension/kindledopewars/config.xml`.
- Pointed KUAL explicitly at `menu.json` with `<menu type="json" dynamic="true">menu.json</menu>`.
- Fixes the extension compiling successfully but not appearing in the KUAL extension list.

## 0.1.2 - GitHub Actions Dependency Fix

- Replaced Ubuntu package `libnettle-dev` with `nettle-dev` in the GitHub Actions host dependency install step.
- Fixes first-build failure on Ubuntu 24.04 / `ubuntu-latest` where `libnettle-dev` has no installation candidate.

## 0.1.1 - Preflight Fixes

- Hardened GitHub Actions workflow against lost executable bits when files are uploaded from Windows or through the GitHub web UI.
- Switched workflow script calls to `bash ./scripts/...` so first compile does not depend on Git file mode preservation.
- Changed the GTK window title to Kindle Awesome WM metadata format so the native app is less likely to launch behind the home screen.

## 0.1.0 - First GitHub Release

- Added native C++17 / GTK2 Kindle KUAL application.
- Added Meson build file.
- Added GitHub Actions kindlehf cross-build workflow using the proven KOReader koxtoolchain + KindleModding SDK pattern.
- Added KUAL extension package structure with `menu.json`, `config.xml`, launch script, persistent save folder, and GTK font defaults.
- Added first playable Dope Wars-style trading loop:
  - Unlimited day count; no 30/60/90-day ending.
  - Game over when cash stays negative for more than seven consecutive day advances.
  - New York travel locations.
  - Buy/sell market with classic contraband inventory list and volatile prices.
  - Cash, bank, debt, health, guns, trenchcoat capacity, and inventory tracking.
  - Daily loan-shark interest.
  - Random market spikes/crashes and street events.
  - Police encounter screen with fight/run choices.
  - Street offers for gun and trenchcoat capacity.
  - Save/resume support inside the KUAL extension data folder.
