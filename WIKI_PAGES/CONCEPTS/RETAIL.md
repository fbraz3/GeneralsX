# CONCEPTS/RETAIL.md

## Summary

Retail Compatibility — preserving original gameplay, replays, and mods.

---

## Purpose

Ensure the port maintains compatibility with original retail features and community content.

---

## Key Principles

### Principle 1: Game Logic Unchanged

**Rule:** Game logic must remain identical to original retail build.

**Why:**
- Original replays must work
- Community mods must function
- Custom game files must be compatible

### Principle 2: Determinism

**Rule:** Rendering/audio changes must not affect gameplay logic.

**Why:**
- Replays must produce identical results
- AI behavior must be predictable
- Competitive integrity must be maintained

---

## Retail Compatibility Checklist

### Replay System

- [x] Absolute and directory-based `-replay` paths
- [x] CRC validation and mismatch diagnostics
- [x] Map field percent encoding/decoding
- [x] Special character handling in map names

### Map Cache

- [x] POSIX-compliant path handling
- [x] Case-insensitive lookup on Linux
- [x] Cross-platform compatibility

### Asset Loading

- [x] Cursor loading with fallback
- [x] Video playback with case-insensitive lookup
- [x] Sprite loading across platforms
- [x] Sound loading with platform abstraction

### Configuration

- [x] INI file reading/writing
- [x] Platform-specific path handling
- [x] Cross-platform compatibility

---

## Determinism Rules

### FPS Caps

**Purpose:** Ensure consistent frame timing across platforms.

**Implementation:**
- Shared FPS cap code in both games
- Platform-specific frame pacing
- Deterministic frame limits

**Fix History:**
- 2026-05-11: FPS uncapped fix (shared code)
- 2026-05-11: Frame counter instability fix

### Math Consistency

**Purpose:** Ensure floating-point consistency across platforms.

**Implementation:**
- `-ffp-contract=off` for non-MSVC builds
- Shared `Sqrt(double)` gateway in `trig.h`
- Deterministic math integration in `cmake/gamemath.cmake`

**Phases:**
- Phase 1-4: Core geometry and trig functions
- Phase 5+: Full math library coverage

---

## Platform Issues & Fixes

### FPS Uncapped (ISSUE #132)

**Problem:** FPS limit not enforced on Linux/macOS

**Fix:**
1. FramePacer API guard for Windows
2. FPS limit validation
3. FPS init logic fix
4. Build flag: `-ffp-contract=off`

**Status:** Fixed (2026-05-11)

### Case-Sensitive Assets (ISSUE #128)

**Problem:** Linux case-sensitive file access broke asset loading

**Fix:** Case-insensitive traversal in VFS

**Status:** Fixed (2026-05-11)

### macOS Transparency (ISSUE #131)

**Problem:** Render target switching caused transparency issues

**Fix:**
1. Pillarbox render-target switching through DX8Wrapper
2. Post-blit `Invalidate_Cached_Render_States()`
3. Pillarbox presentation fix

**Status:** Fixed (2026-05-10)

### Headless Texture Path

**Problem:** Texture loading failed in headless replay mode

**Fix:**
1. Null guards in `D3DXCreateTexture`
2. Safe `DX8Wrapper::_Create_DX8_Texture`
3. Graceful failure in texture load tasks

**Status:** Fixed (2026-05-04)

---

## Testing Protocol

### Replay Testing

**Requirements:**
- Test on both Linux and macOS
- Use retail replays
- Verify pass/fail reports
- Check CRC validation

**CI Integration:**
- Linux + macOS replay tests
- Per-file pass/fail details
- Explicit CI failure on replay failures

### Retail Replay Checklist

- [ ] Original replays load correctly
- [ ] Gameplay matches original
- [ ] No unexpected behavior changes
- [ ] CRC validation works
- [ ] Map names parse correctly

---

## See Also

- [CONCEPTS/DETERMINISM.md](./DETERMINISM.md) — FPS caps, math
- [CONCEPTS/RAG.md](./RAG.md) — Platform isolation
- [ENTITIES/GAME.md](../ENTITIES/GAME.md) — Game entities
- [ARCHITECTURE/DATA.md](../ARCHITECTURE/DATA.md) — Data handling

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, retail compatibility docs
