# ISSUE-004: Generals Challenge Mode Crashes with Segfault

**Status**: RESOLVED  
**Session Discovered**: 53 (2026-02-21)  
**Session Resolved**: 54 (2026-02-22)  
**Severity**: High  
**Component**: Platform / Video Playback  
**Reproducibility**: Was High (now fixed)  

## Symptom

Opening Generals Challenge mode from the main menu causes an immediate segfault (SIGSEGV). The game crashes to the terminal with no user-visible error message.

## Investigation Summary

**Root Cause**: Bink video playback was stubbed/non-functional on Linux, causing the engine to crash when attempting to play the Bink intro video for Generals Challenge mode.

**Solution**: Video playback implementation (Phase 3) resolved this by providing a proper video player backend that handles Bink files gracefully on Linux.

**Resolution Details**: When video playback was properly implemented, the crash at Bink initialization was eliminated. The Challenge mode now successfully loads the intro video without segfaulting.

## Code Audit Results

Not required - issue is resolved at the platform layer via proper video playback implementation.

## Next Steps

No further action required - issue is closed and resolved.

## Workaround

None required - issue is fully resolved.

## Impact

**Status**: RESOLVED ✓

- **Gameplay**: ✓ Generals Challenge mode now fully accessible
- **Stability**: ✓ No longer crashes on entry
- **Determinism**: N/A
- **Release blocker**: ✗ No longer a blocker

## Reference

- Session 53 discovery (2026-02-21)
- Phase 3 plan: Bink video replacement spike (`docs/WORKDIR/phases/`)
