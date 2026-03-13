# ISSUE-013: Linux Compat `FindFirstFile` Ignores Pattern and Directory

**Status**: OPEN  
**Session Discovered**: 2026-03-12  
**Severity**: High  
**Component**: Platform  
**Reproducibility**: High  

## Symptom

On non-Windows builds, code paths that rely on `FindFirstFile` wildcard filtering can behave incorrectly.

Typical outcomes include:
- false positives when checking whether specific wildcard matches exist
- iteration over unrelated entries from the current working directory
- feature behavior that appears inconsistent with requested file masks

## Investigation Summary

The Linux compatibility implementation of `FindFirstFile`/`FindNextFile` in `file_compat.h` currently:
- always opens `"."` via `opendir(".")`
- stores the `pattern` but does not apply wildcard matching
- returns first directory entry regardless of requested mask

This diverges from Windows semantics where `FindFirstFile("path/pattern", ...)` enumerates files matching the provided path+mask.

Impact is broader than one menu flow because this API is reused across game and tools code.

## Code Audit Results

Confirmed in:
- `GeneralsMD/Code/CompatLib/Include/file_compat.h`
  - `FindFirstFile(const char* pattern, WIN32_FIND_DATA* findData)` opens `opendir(".")`
  - `pattern` is copied into handle state but not used for filtering
  - `FindNextFile` also returns raw `readdir` entries without wildcard/path filtering

One runtime-adjacent consumer affected by this mismatch:
- `GeneralsMD/Code/GameEngine/Source/GameClient/System/Image.cpp`
  - checks mapped image INIs via `FindFirstFile(userDataPath, ...)`

## Next Steps (for Future Session)

1. Implement proper parsing of `pattern` into directory part + filename wildcard.
2. Use `opendir(parsedDirectory)` instead of always `opendir(".")`.
3. Apply wildcard matching (`*`, `?`) to `d_name` before returning entries.
4. Preserve Win32-like behavior for invalid patterns and handle lifecycle.
5. Validate with call sites that depend on wildcard existence checks.

## Workaround

No robust workaround in code. Feature-level behavior may only be recoverable by avoiding paths that depend on wildcard filtering through the compat API.

## Impact

- **Gameplay**: Major (can affect runtime file discovery behaviors)
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: Yes

## Reference

- `GeneralsMD/Code/CompatLib/Include/file_compat.h`
- `GeneralsMD/Code/GameEngine/Source/GameClient/System/Image.cpp`
- `docs/KNOWN_ISSUES/ISSUE-012_SOLVED_linux_save_replay_backslash_paths.md`
