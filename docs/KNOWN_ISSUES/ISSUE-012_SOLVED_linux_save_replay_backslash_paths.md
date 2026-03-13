# ISSUE-012: Linux Save and Replay Paths Use Backslash Literals

**Status**: RESOLVED  
**Session Discovered**: 2026-03-12  
**Severity**: High  
**Component**: Gameplay / Platform  
**Reproducibility**: High  

## Symptom

Saving appears to succeed (no immediate error), but saves do not show up in the save/load menu.

On Linux, user data inspection shows entries such as `Save\` and `Save\00000002.sav` created as literal names under the user data root rather than as files inside a `Save/` directory.

## Investigation Summary

Root cause is Windows-style path separators hardcoded in save/replay directory builders, even on non-Windows builds:
- `GameState::getSaveDirectory()` used `"Save\\"`
- `RecorderClass::getReplayDir()` used `"Replays\\"`
- `RecorderClass::getReplayArchiveDir()` used `"ArchivedReplays\\"`

On Linux, backslash is not a path separator. It is a valid character in a filename, so code created malformed entries like `Save\00000002.sav` in the user data root.

Additional clarification:
- Not finding data under `~/.config` is expected in this project.
- Linux user data path is configured as `$XDG_DATA_HOME/generals_zh/` or `$HOME/.local/share/generals_zh/` in `GlobalData.cpp`.

## Code Audit Results

Confirmed in:
- `GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp`
- `Generals/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp`
- `Generals/Code/GameEngine/Source/Common/Recorder.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp` (Linux user data path)

Implemented fix in branch `fix/issue-011-replay-menu-segfault`:
- Use `"Save/"`, `"Replays/"`, and `"ArchivedReplays/"` on non-Windows
- Keep original backslash behavior on Windows via `#ifdef _WIN32`

## Next Steps (for Future Session)

1. Runtime-verify save/load and replay list behavior after the separator fix.
2. Add a one-time migration routine for legacy malformed Linux entries (`Save\\*`, `Replays\\*`) to proper directories.
3. Add a regression test/checklist item for path separator correctness on Linux/macOS.

## Workaround

Before migration lands, malformed entries can be moved manually from user data root into proper directories:
- from `Save\000000NN.sav` to `Save/000000NN.sav`
- from `Replays\*.rep` to `Replays/*.rep`

## Impact

- **Gameplay**: Major (save/load and replay discoverability affected)
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: Yes

## Reference

- `docs/KNOWN_ISSUES/ISSUE-011_SOLVED_replay_menu_loading_screen_segfault.md`
- `GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp`
- `Generals/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp`
- `Generals/Code/GameEngine/Source/Common/Recorder.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp`
