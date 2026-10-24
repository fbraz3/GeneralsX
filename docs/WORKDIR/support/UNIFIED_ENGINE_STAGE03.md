# Unified Engine Stage 03: GameProfile Foundation

Date: 2026-10-23
Scope: immutable process game identity selected before asset mount and
subsystem construction, routed through existing Generals and Zero Hour entry
points.

## Design

PLAN-023 gate 3 requires a fixed game profile chosen before mounting assets or
constructing engine subsystems. Until the single executable exists, each binary
defaults to its compiled game (`RTS_GENERALS` / `RTS_ZEROHOUR`). Provisional CLI
spelling: `-game generals|zh` (aliases: `gen`, `g`, `zerohour`, `zero-hour`,
`md`). A mismatched `-game` exits before initialization; cross-game selection
waits for the unified link.

`GameProfile` lives in Core (`Common/GameProfile.h`). It exposes short/display
names and the primary/legacy asset environment variable names. Asset discovery
in `BIGFileSystem` and the Unix fallback in `StdLocalFileSystem` read those
names from the selected profile instead of compile-time branches for the
primary root. Zero Hour still mounts base Generals BIG archives when
`loadsBaseGeneralsAssets()` is true. The ZH-only duplicate `INIZH.big` skip is
also profile-gated so Generals builds never compile that path away incorrectly
when sources are shared.

Selection is wired in Linux `SDL3Main.cpp` and Windows `WinMain.cpp` for both
games trees, after `initMemoryManager()` and before
`CommandLine::parseCommandLineForStartup()`.

`GetStringFromRegistry` / `GetUnsignedIntFromRegistry` in both game
`registry.cpp` copies use `getRegistryGamePath()` and the profile-specific
`CNC_GENERALS_` / `CNC_ZH_` environment prefix on Unix. Dedicated
`GetStringFromGeneralsRegistry` remains Generals-fixed for Zero Hour base-asset
lookups.

## Local validation

Rebuilt `g_generals` and `z_generals` in `ubuntu-dev` on
`build/linux64-unified-baseline`. Headless smokes:

| Case | Result |
|---|---|
| Generals default | exit 0, `[GAME_PROFILE] selected 'generals'` |
| Generals `-game generals` | exit 0 |
| Generals `-game zh` | exit 1 before init (expected mismatch) |
| Zero Hour default | exit 0, `[GAME_PROFILE] selected 'zh'` |
| Zero Hour `-game zh` | exit 0 |

Logs: `logs/unified-stage3-*.out`.

## Intentionally deferred

- Startup chooser UI and final CLI spelling freeze.
- Replacing remaining gameplay `#if RTS_*` branches with profile policies.
- One executable linking both profiles.
