# CONCEPTS/DETERMINISM.md

## Summary

Determinism — FPS caps, math consistency, and predictable behavior.

---

## Purpose

Ensure predictable, reproducible behavior across all platforms.

---

## Key Principles

### Principle 1: FPS Cap Consistency

**Rule:** FPS cap code must be shared between both games.

**Why:**
- Ensures consistent frame timing
- Prevents platform-specific timing variations
- Maintains replay compatibility

### Principle 2: Math Consistency

**Rule:** Floating-point operations must be deterministic.

**Why:**
- Prevents platform-specific calculation differences
- Ensures replay determinism
- Maintains competitive integrity

---

## FPS Caps Implementation

### Purpose

Ensure consistent frame timing across platforms.

### Implementation Details

**FramePacer API Guard:**
```cpp
#ifdef _WIN32
  timeBeginPeriod(1);
  timeEndPeriod(1);
#endif
```

**FPS Limit Validation:**
```cpp
if (m_framesPerSecondLimit <= 0) {
  // Early-exit for invalid FPS targets
  return;
}
```

**FPS Init Logic Fix:**
Changed from:
```cpp
if (m_useFpsLimit && m_framesPerSecondLimit <= 0)
```
To:
```cpp
if (m_framesPerSecondLimit <= 0)
```

**Result:** Always ensures valid BaseFps (30) default.

### Build Flag

**Flag:** `-ffp-contract=off`

**Purpose:** Ensure floating-point consistency

**Scope:** Non-MSVC builds

**Integration:** `cmake/gamemath.cmake`

---

## Math Consistency

### Phase 1-4: Baseline

**Changes:**
- Added `-ffp-contract=off` for non-MSVC builds
- Introduced shared `Sqrt(double)` gateway in `trig.h`
- Replaced core geometry `sqrt` calls in `BaseType.h`
- Routed `Trig.cpp` through WWMath wrapper functions
- Added `cmake/gamemath.cmake` as Phase 4 scaffold

### Future Phases

| Phase | Focus | Status |
|-------|-------|--------|
| Phase 5 | Full math library | Pending |
| Phase 6 | External dependencies | Pending |
| Phase 7 | Verification | Pending |

---

## Common Pitfalls

### FPS Uncapped

**Symptoms:**
- Game runs faster on high-refresh monitors
- Replays don't match
- AI behavior inconsistent

**Solution:**
- Verify FPS cap is enabled
- Check for platform-specific overrides
- Review frame pacing code

### Math Inconsistency

**Symptoms:**
- Different results on different platforms
- Replays don't match
- AI behavior varies

**Solution:**
- Ensure `-ffp-contract=off` is set
- Review math gateway functions
- Check for platform-specific math overrides

---

## Testing

### Replay Determinism

- [ ] Same replay produces identical results
- [ ] No platform-specific behavior differences
- [ ] AI behavior is consistent

### FPS Verification

- [ ] FPS cap enforced on all platforms
- [ ] Frame pacing consistent
- [ ] No unexpected frame drops

---

## See Also

- [CONCEPTS/RETAIL.md](./RETAIL.md) — Retail compatibility
- [CONCEPTS/RAG.md](./RAG.md) — Platform isolation
- [ENTITIES/PLATFORM.md](../ENTITIES/PLATFORM.md) — Platform technologies
- [TOPICS/BUILD.md](../TOPICS/BUILD.md) — Build configuration

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, determinism documentation
