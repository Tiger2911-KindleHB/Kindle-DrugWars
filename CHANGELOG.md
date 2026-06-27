# Changelog

## 0.1.14 - Aggressive Layout Debug Controls

- Added direct `drug row height` debugging separate from chart text size so spacing between buyable drug rows can be compressed or expanded independently.
- Changed non-font debug controls to move in 5-point increments instead of tiny 1-point adjustments.
- Expanded layout controls to allow large negative and positive values for aggressive Kindle screen tuning.
- Added region-height controls for header, stats/location, event/news, chart box, and bottom controls so each main UI section can be resized directly.
- Changed the drug chart from an expanding leftover-space region to a fixed-height, scrollable chart box controlled by `chart box height`, preventing it from consuming most of the screen by default.
- Reduced default chart-row spacing and chart-box height so the market table starts from a more compact layout.
- Added `Export UI` in Settings, writing the current debug values and effective calculated sizes to `/mnt/us/extensions/kindledopewars/data/ui-debug.txt`.
- Auto-exports UI debug values after every Settings adjustment so the latest tuning is always available even before tapping Export UI.

## 0.1.13 - Layout Spacing Debug Controls

- Added persistent spacing controls in Settings for `section gap`, `chart gap`, and `bottom gap` so the Kindle UI can be tuned beyond text size alone.
- Reduced default vertical dead space between major regions by lowering root padding, separator padding, and fixed region heights.
- Made drug-chart row height depend on both chart font size and the new `chart gap` setting instead of using a large fixed extra row height.
- Reduced the default chart row height calculation so more of the item list and bottom controls fit on-screen.
- Made bottom buy/sell/travel/bank button spacing and height use the new `bottom gap` setting.
- Updated Settings helper text to explain the difference between text sizing and spacing controls.

## 0.1.12 - Remove Mockup Region Colors

- Removed colored backgrounds from the main market UI.
- Kept the five debug font-size regions: header, stats/location, event/news, chart, and bottom controls.
- Updated Settings helper text to clarify that colored mockup blocks are layout markers only, not visual styling for the Kindle app.
- Left the tappable news area, street-tip confirmation, and day-scaling tip price behavior intact.

## 0.1.11 - Colored Region UI / Tip Confirmation

- Rebuilt the main market screen to match the supplied colored-region layout: orange header, green stats/location, yellow event text, purple drug chart, and blue buy/sell/travel/bank controls.
- Added five Settings font scalers for the colored regions: `header`, `stats`, `event`, `chart`, and `bottom`.
- Restored New/Settings/Exit to the orange header row with larger fixed tap targets instead of tiny corner controls.
- Fixed GTK button label font application so button text uses the requested debug size instead of inheriting the oversized default GTK font.
- Replaced drug item buttons with tappable chart cells so the chart text scales cleanly and does not get inflated by button padding/default fonts.
- Added a Street Tip confirmation popup with Yes/No buttons when the yellow event area is tapped.
- Changed street-tip pricing from a flat `$250` to `$250 + $50 per elapsed day`, so information becomes more expensive as the run progresses.
- Persisted the new `stats` and `event` font sizes in the save file.

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
