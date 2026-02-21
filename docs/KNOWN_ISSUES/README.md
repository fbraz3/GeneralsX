# Known Issues

This directory contains documentation for all known issues, bugs, and limitations discovered during development.

## File Naming Convention

Use the format: `ISSUE-XXX_slug_description.md`

Where:
- `XXX` = Zero-padded issue number (001, 002, 003, etc.)
- `slug_description` = Brief lowercase, underscore-separated description of the issue

**Examples:**
- `ISSUE-001_shell_map_unit_immortality.md`
- `ISSUE-002_audio_crackling_on_startup.md`
- `ISSUE-003_replay_desync_multiplayer.md`

## Issue Document Structure

Each issue file should contain:

```markdown
# ISSUE-XXX: [Full Title]

**Status**: [OPEN|INVESTIGATING|BLOCKED|RESOLVED|WONTFIX]
**Session Discovered**: XX (YYYY-MM-DD)
**Severity**: [Critical|High|Medium|Low]
**Component**: [Graphics|Audio|Gameplay|Platform|Build|Other]
**Reproducibility**: [100%|High|Intermittent|Rare]

## Symptom

(Clear description of observable behavior)

## Investigation Summary

(Root cause analysis, potential hypotheses, findings)

## Code Audit Results

(What was checked, what was found)

## Next Steps (for Future Session)

(Actionable investigation items)

## Workaround

(If any exists)

## Impact

- **Gameplay**: [None|Minor|Major|Blocking]
- **Stability**: [None|Intermittent Crashes|Always Crashes]
- **Determinism**: [Not Applicable|Critical|Non-Critical]
- **Release blocker**: [Yes|No]

## Reference

(Links to related code, dev diary entries, etc.)
```

## Status Definitions

- **OPEN** — Confirmed issue, awaiting investigation or fix
- **INVESTIGATING** — Currently being researched; uncertain root cause or multiple hypotheses
- **BLOCKED** — Waiting for external feedback, data access, or prerequisite fixes
- **RESOLVED** — Fixed; waiting for verification or release
- **WONTFIX** — Intentionally deferred; documented rationale for non-resolution

## Severity Levels

- **Critical** — Game-breaking, prevents progress, crashes required
- **High** — Major feature impaired, significant gameplay impact
- **Medium** — Observable but workaroundable, cosmetic or edge-case impact
- **Low** — Cosmetic only, no gameplay impact, deferred indefinitely

## Component Categories

- **Graphics** — Rendering, DXVK, DirectX, Vulkan, visual artifacts
- **Audio** — Miles Sound System, OpenAL, music, voice, sound effects
- **Gameplay** — Game logic, physics, unit behavior, campaigns, multiplayer
- **Platform** — Linux/Windows compatibility, build system, POSIX/Win32 layer
- **Build** — CMake, compilation, linker errors, toolchain setup
- **Other** — Miscellaneous

---

**Generated**: 2026-02-20  
**Phase**: 1 (Linux Graphics Port)
