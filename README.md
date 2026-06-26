# Dope Wars Kindle

Native Kindle KUAL trading game built with the same workflow used for the prior Kindle apps: C++17, GTK2, Meson, GitHub Actions, KOReader kindlehf toolchain, KindleModding SDK, and one KUAL extension zip.

## Game

This is a Kindle-native Dope Wars-style game. The core loop is classic street-market trading:

- Start with cash, debt, health, and trenchcoat capacity.
- Buy low and sell high across New York locations.
- Travel to advance the day and refresh prices.
- Debt grows daily.
- Police and random street events can hurt or help the run.
- The game has no 30/60/90-day limit.
- If cash is negative for more than seven consecutive day advances, the run ends.

## Repository Layout

```text
.github/workflows/kindlehf.yml     GitHub Actions Kindle build
src/main.cpp                       Native GTK2 game source
meson.build                        Meson build definition
scripts/package-kual.sh            Builds final KUAL zip from compiled binary
scripts/write-meson-crossfile.sh   Fallback cross-file writer
extension/kindledopewars/          KUAL extension template
CHANGELOG.md                       Change history
PROMPT.md                          Build and design prompt followed for this release
```

## Build in GitHub Actions

1. Create a new GitHub repository.
2. Upload the contents of this zip to the repository root.
3. Commit and push.
4. Open the **Actions** tab.
5. Run **Kindle PW12 kindlehf build**.
6. Download the artifact named `kindledopewars-kual`.
7. Inside it, use `kindledopewars-kual.zip`.

## Install on Kindle

1. Extract `kindledopewars-kual.zip`.
2. Copy the resulting `kindledopewars` folder to:

```text
/mnt/us/extensions/
```

3. Disconnect the Kindle.
4. Open KUAL.
5. Launch **Dope Wars Kindle**.

## Save File

The save file is stored inside the extension:

```text
/extensions/kindledopewars/data/save.dat
```

The launch log is stored at:

```text
/extensions/kindledopewars/data/kindledopewars.log
```

## Future Patch Rule

For future fixes or changes, patch zips should include only the files or folders changed by that request, plus an updated `CHANGELOG.md`.
