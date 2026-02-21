# ISSUE-004: Generals Challenge Mode Crashes with Segfault

**Status**: OPEN  
**Session Discovered**: 53 (2026-02-21)  
**Severity**: High  
**Component**: Platform / Video Playback  
**Reproducibility**: High (observed on first attempt)  

## Symptom

Opening Generals Challenge mode from the main menu causes an immediate segfault (SIGSEGV). The game crashes to the terminal with no user-visible error message.

## Investigation Summary

Not yet investigated in depth. The most likely cause based on code knowledge:

### Primary Hypothesis: Bink Video Playback Crash

Generals Challenge mode plays a Bink (`.bik`) intro video when first opened. The Bink SDK (`bink.cmake`, `Bink/`) is a Windows-only native library. On Linux:

- `BinkVideo` or equivalent is either stubbed or linked against a non-functional Linux Bink build.
- Attempting to open/play a `.bik` file may dereference a null function pointer or null `BINK*` handle.
- This is consistent with the user's intuition ("acho que é do blink video").

### Secondary Hypotheses

1. **Null function pointer in Bink wrapper** — If `BinkOpen()` or `BinkDoFrame()` are stubbed as `nullptr` on Linux and the caller does not check for null, a direct call crash occurs.
2. **Missing `.bik` file** — If the video file is not present in the deploy directory, the engine may crash instead of failing gracefully.
3. **Generals Challenge-specific INI/script failure** — Some data unique to this mode causes a bad memory access during initialization unrelated to video.

## Code Audit Results

Not yet investigated. Relevant areas to check:

- `GeneralsMD/Code/GameEngine/Source/` — video/Bink initialization
- `cmake/bink.cmake` — Bink library linking on Linux
- `Core/GameEngine/Include/` — `BinkVideoPlayer` or similar interface

## Next Steps (for Future Session)

1. Run under GDB (`Debug GeneralsXZH (Linux GDB, Backtrace)` task) and capture full backtrace.
2. Identify exact crash site from backtrace — confirm if it is inside Bink code path.
3. If Bink: implement a null-safe stub that logs a warning and returns immediately instead of crashing.
4. Phase 3 covers proper video playback replacement (FFmpeg or skip-video stub).

## Workaround

Avoid opening Generals Challenge mode until Phase 3 video playback is addressed.

## Impact

- **Gameplay**: Major (entire Generals Challenge campaign mode inaccessible)
- **Stability**: Always crashes on entry
- **Determinism**: N/A
- **Release blocker**: Yes (crash is unacceptable; graceful skip acceptable as interim)

## Reference

- Session 53 discovery (2026-02-21)
- Phase 3 plan: Bink video replacement spike (`docs/WORKDIR/phases/`)
