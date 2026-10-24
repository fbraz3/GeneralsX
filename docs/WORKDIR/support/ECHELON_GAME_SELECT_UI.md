# Echelon native GameSelect UI

Date: 2026-10-24

## Decisions (locked)

- Monolithic `echelon` process; profile locked once before memory pools / BIG mount.
- **GUI launch** (no automation flags): always show native GameSelect, then continue into the selected game’s original Main Menu after normal engine init.
- **Automation** (`-headless`, `-replay`, `-map`, `-skirmish`, `-lanProbe`, … or `ECHELON_SKIP_GAME_SELECT=1`): skip UI; `selectForProcess` as before.
- Pre-init SDL chooser (dark shell + gold accents) — no relaunch; same session handoff to retail Shell/MainMenu after assets mount.

## Files

- `GeneralsMD/Code/Main/GameSelectUI.{h,cpp}`
- Hook: `SDL3Main.cpp` under `RTS_UNIFIED_ENGINE`
- Helpers: `GameProfile::isAutomationLaunch`, `peekPreferredKind`, `selectKind`
