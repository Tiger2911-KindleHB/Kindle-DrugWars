# Changelog

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
