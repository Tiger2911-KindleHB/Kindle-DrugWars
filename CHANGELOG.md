# Changelog
## 0.1.10 - Large Main Menu / Touch Deferral Fix

- Removed the tiny New/Settings/Exit strip from the main market screen.
- Added one large full-width `MENU / SETTINGS` button with a much larger font and tap target.
- Moved New Run and Exit access into the Settings screen so they remain reachable without tiny top-corner buttons.
- Converted Settings from a modal popup into a full-screen in-app page with large plus/minus controls and large Back/New Run/Exit buttons.
- Changed deferred touch actions from GTK idle callbacks to a short timeout callback so screen rebuilds happen after the touch event has fully finished processing.

## 0.1.9 - Touch Stability and Large Top Controls

- Replaced the tiny top-right New/Settings/Exit controls with a full-width top control strip using large tap targets.
- Added a larger top safe margin so the Kindle status overlay does not cover the control strip.
- Changed the main app window back from popup/override-redirect mode to a normal fullscreen GTK toplevel for more reliable touch handling.
- Deferred screen-changing button actions through the GTK idle loop so the app no longer destroys/rebuilds the active button while Kindle touch events are still being processed.
- Increased the bottom action-button hit target height so Buy/Sell/Travel/Bank are easier to tap.
- Trimmed the news area height slightly to recover vertical space for the larger controls.


## 0.1.8 - Top Menu Visibility Fix

- Moved New/Settings/Exit out of the overloaded stat row and onto a dedicated top control strip so they remain visible and tappable on the Kindle screen.
- Added a larger top safe margin to keep the app controls away from the Kindle status overlay area.
- Reduced the fixed menu-button font/height so the three top controls fit even when header debug text is larger.
- Lowered news and bottom-control vertical sizes slightly to recover screen space after the dedicated top control strip was added.
- Expanded debug font minimums down to 4 so oversized saved settings can be stepped down farther from Settings.

## 0.1.7 - Debug Font Controls

- Reduced the default drug chart text size again, with item rows now using the new `chart` debug size instead of fixed larger fonts.
- Added Settings debug controls for the three primary UI font groups: `header`, `chart`, and `bottom`.
- Added plus/minus controls in Settings with the requested layout: `[ - ] [header: x] [ + ]`, `[ - ] [chart: x] [ + ]`, and `[ - ] [bottom: x] [ + ]`.
- Persisted debug font sizes in the save file so Kindle-specific tuning survives app restarts.
- Routed top stat labels and New/Settings/Exit through the header size, the drug table through the chart size, and Buy/Sell/Travel/Bank through the bottom size.

## 0.1.6 - Kindle UI Scale Reduction

- Reduced the main-screen font scale so the top stat bar, New/Settings/Exit buttons, table, and bottom action buttons fit inside the visible Kindle screen.
- Lowered the height and text size of Buy/Sell/Travel/Bank controls so their labels are not clipped at the bottom edge.
- Reduced the location header and news/event area height to free vertical space for the item table and bottom controls.
- Fixed compact stat font scaling so long values shrink instead of accidentally growing larger than the requested base size.
- Added a small top spacer and reduced root padding/spacing to avoid top-row clipping while preserving usable vertical space.

## 0.1.5 - User Layout Pass

- Rebuilt the main market screen to match the supplied Kindle UI mockup: top stat bar, left cash/debt/debt-days block, centered current location block, tappable news area, item/price/held table, and bottom Buy/Sell/Travel/Bank controls.
- Centered button label alignment across the market, popup, and action buttons.
- Added dynamic font sizing for compact stat fields so longer money/stat values are less likely to overflow their assigned UI area.
- Changed Travel from a full-page view into a modal popup with location choices.
- Changed Bank / Loan Shark from a full-page view into a modal popup with deposit, withdraw, debt payment, and borrow actions.
- Added a Settings popup for basic control guidance.
- Made the news/random-event area tappable; spending $250 now reveals a scheduled future market tip.
- Added scheduled future market events so purchased street information maps to an actual upcoming shortage or market flood.
- Persisted scheduled future-event fields in the save file.

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
