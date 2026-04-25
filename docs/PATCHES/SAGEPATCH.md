# SagePatch — Casual QoL Extras for GeneralsX (macOS)

SagePatch is an optional, drop-in patch that adds quality-of-life features to
GeneralsX without modifying the game source. Inspired by the casual subset of
GenTool, it ships as a separate dylib that is loaded via `DYLD_INSERT_LIBRARIES`
plus a small `GameData` INI override that the engine picks up at startup.

## Features

| Feature | Trigger | Notes |
|---|---|---|
| Screenshot | `F11` | PNG saved to `~/Pictures/GeneralsX/`. Captures the actual game window via CoreGraphics. |
| Cursor lock toggle | `Scroll Lock` | Confines the mouse to the game window. Useful in windowed mode + multi-monitor setups. |
| Brightness up | `Ctrl + Page Up` | +8 step on the gamma curve, range −128…+128. |
| Brightness down | `Ctrl + Page Down` | −8 step. |
| Camera zoom range | (passive) | `MaxCameraHeight=800`, `MinCameraHeight=60`, `EnforceMaxCameraHeight=No`. |
| Keyboard scroll speed | (passive) | `KeyboardScrollSpeedFactor=1.0` (vanilla 0.5). |

Hot-key collisions: SagePatch eats the events it handles, so they do not
also reach the game.

## How to enable

Build:

```bash
cmake --preset macos-vulkan -DRTS_BUILD_OPTION_SAGE_PATCH=ON
cmake --build build/macos-vulkan --target z_generals -j$(sysctl -n hw.logicalcpu)
```

(The `macos-vulkan` preset already turns this `ON` by default — disable with
`-DRTS_BUILD_OPTION_SAGE_PATCH=OFF` if you want a vanilla build.)

Deploy:

```bash
./scripts/build/macos/deploy-macos-zh.sh
```

The deploy script:
1. Copies `libsage_patch.dylib` next to `GeneralsXZH`
2. Drops `Override.ini` into `Data/INI/Default/GameData/SagePatch.ini`
3. Configures the wrapper `run.sh` to set `DYLD_INSERT_LIBRARIES`

Run as usual:

```bash
~/GeneralsX/GeneralsZH/run.sh -win
```

Take a screenshot mid-game with **F11**.

## Disabling at runtime

Without rebuilding:

```bash
SAGE_PATCH_DISABLED=1 ~/GeneralsX/GeneralsZH/run.sh -win
```

This skips the `DYLD_INSERT_LIBRARIES` step. The INI override remains active —
delete `Data/INI/Default/GameData/SagePatch.ini` to revert camera/scroll values.

## Architecture

```
Game process (GeneralsXZH)
    │
    ├── DYLD_INSERT_LIBRARIES → libsage_patch.dylib
    │       │
    │       └── __DATA,__interpose table replaces SDL_PollEvent
    │              │
    │              └── on F11 / Scroll Lock / Ctrl+PgUp/Dn → SagePatch handlers
    │                      │
    │                      └── CoreGraphics / SDL3 / CGSetDisplayTransferByFormula
    │
    └── Engine reads Data/INI/Default/GameData/SagePatch.ini → overrides camera/scroll
```

No D3D8 proxy, no Vulkan layer, no engine source modifications. The whole patch
is two files: a dylib and an INI.

## Why this approach

The original GenTool had to be a `d3d8.dll` proxy because Windows games of that
era exposed no other plugin surface. On modern macOS we have:

- **`__interpose`** — ld + dyld replace symbol resolution at load time. We see
  `SDL_PollEvent` exactly as the game calls it.
- **CoreGraphics window capture** — the macOS compositor already has the
  composited pixels; we ask for them. No need to read a Vulkan back-buffer.
- **Engine INI overrides** — GeneralsX's INI loader merges files in
  `Data/INI/Default/<Subsystem>/`. Parameter tweaks need zero code.

This keeps SagePatch ~400 lines instead of the ~3000 lines a full COM proxy
would require.

## Limits / what's *not* in scope

By design, SagePatch sticks to **casual** QoL:

- No replay tools (frame stepping, fog-of-war replay, controls bar)
- No competitive features (Money Display, Player Table, Random Balance)
- No anti-cheat (MDS, Game File Validator, version validation)
- No external server integrations (CNC Online, GameRanger, Upload Mode, ticker,
  ranked maps, ladder, GenTool updater)

Engine-side bug fixes (scud bug, tunnel bug, building bug, multiplayer crash)
also live outside this patch — they require modifications inside the game's
source rather than a side-loaded dylib.

## Files

```
Patches/SagePatch/
    CMakeLists.txt
    include/SagePatch/Hooks.h
    include/SagePatch/Features.h
    include/SagePatch/Logger.h
    src/Init.cpp                    # constructor / destructor
    src/interposers.cpp             # __DATA,__interpose table
    src/features/KeyHandler.cpp     # dispatches hot-keys
    src/features/Screenshot.cpp     # F11 → PNG
    src/features/CursorLock.cpp     # Scroll Lock toggle
    src/features/Brightness.cpp     # Ctrl+PgUp / Ctrl+PgDn
    src/util/Logger.cpp             # placeholder
    src/util/Config.cpp             # placeholder
    resources/Override.ini          # engine-side INI override
docs/PATCHES/SAGEPATCH.md           # this file
```
