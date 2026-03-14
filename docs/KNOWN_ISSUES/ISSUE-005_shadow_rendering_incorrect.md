# ISSUE-005: Building and Unit Shadows Render Incorrectly

**Status**: OPEN  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: Medium  
**Component**: Graphics  
**Reproducibility**: High  

## Symptom

Building shadows and unit shadows do not render correctly in the cross-platform port.

The issue is visible during regular gameplay, but it does not currently prevent matches from progressing.

## Investigation Summary

The defect appears isolated to the rendering path rather than gameplay logic.

Current session state (2026-03-14):
- A first diagnostics pass was completed in the projected shadow pipeline and render state path.
- Runtime-gated probes were added to inspect matrix/projection values and additive/sorted render states without changing default gameplay behavior.
- No functional renderer fix was merged yet; this issue remains OPEN while data is collected and compared.

Main audit focus remains:
- W3D shadow manager behavior in the SDL3 + DXVK path
- Shadow texture/material setup and transform flags versus legacy behavior
- Platform behavior differences between Linux and macOS

## Code Audit Results

Initial audit and instrumentation completed in these areas:
- Projected shadow transform setup path
- Projected shadow camera/projector update path
- Render queue state probes for headlight/sorted/additive meshes

Runtime diagnostics toggles now available:
- `GENERALSX_DEBUG_PROJECTED_SHADOW=1`
- `GENERALSX_DEBUG_HEADLIGHTS=1`
- `GENERALSX_DEBUG_SORTED_ADDITIVE=1`

Current conclusion:
- The work so far improves observability and confirms where to measure projection state, but does not yet identify a single confirmed root-cause fix.

## Next Steps (for Future Session)

1. Capture reproducible screenshots and synchronized runtime logs with diagnostics enabled on Linux and macOS.
2. Correlate projector matrix values, texture transform flags, and sorted/additive render states during the ShellMap and regular gameplay.
3. Compare measured behavior with fighter19 reference patterns to isolate the first concrete behavioral mismatch.
4. Implement a minimal renderer-side fix in the identified choke point and validate visual outcome plus no determinism risk.
5. Keep this issue updated with evidence (logs, screenshots, affected code path) after each validation run.

## Workaround

None.

## Impact

- **Gameplay**: Minor
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: No

## Reference

- [GITHUB_ISSUE_DRAFTS_2026-03-12.md](GITHUB_ISSUE_DRAFTS_2026-03-12.md)