# Echelon native GameSelect UI

Date: 2026-10-24

## Decisions (locked)

- Monolithic `echelon` process; profile locked once before memory pools / BIG mount
  (required: Generals vs ZH memory pool tables diverge).
- **GUI launch** (no automation flags): always show native GameSelect, then continue
  into the selected game’s **original** Main Menu after normal engine init.
- **Automation** (`-headless`, `-replay`, `-map`, `-skirmish`, `-lanProbe`, … or
  `ECHELON_SKIP_GAME_SELECT=1`): skip UI; `selectForProcess` as before.
- Pre-init SDL chooser (dark shell + gold accents + product icons) — no process relaunch.
- Seamless handoff: chooser window size/position/title are reused when recreating the
  `SDL_WINDOW_VULKAN` window for DXVK (renderer cannot share a Vulkan window).

## Why not Shell `.wnd` for the chooser

`GameWindow` / `MainMenu.wnd` need FontLibrary, ImageCollection and mounted BIG/INI.
Those require `initMemoryManager()` which already needs a locked `GameProfile`.
Therefore the chooser is pre-pool SDL; the post-init menu is the real retail Main Menu.

## Files

- `GeneralsMD/Code/Main/GameSelectUI.{h,cpp}`
- Hook: `SDL3Main.cpp` under `RTS_UNIFIED_ENGINE`
- Helpers: `GameProfile::isAutomationLaunch`, `peekPreferredKind`, `selectKind`
