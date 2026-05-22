# ENTITIES/REFERENCES.md

## Summary

Reference repositories — fighter19, jmarshall, thesuperhackers, and other external resources.

---

## Primary References

### fighter19-dxvk-port

**Type:** Platform reference (DXVK + SDL3 on Linux)

**Purpose:**
- Primary graphics/platform reference
- DXVK + SDL3 integration patterns
- Platform-specific implementation details

**Relevance:**
- Graphics integration
- SDL3 platform layer
- DXVK wrapper patterns

**Key Features:**
- Complete DXVK implementation
- SDL3 integration
- Platform-specific fixes

---

### jmarshall-win64-modern

**Type:** Audio reference (OpenAL implementation, Generals-only)

**Purpose:**
- Audio reference implementation
- OpenAL architecture patterns
- Miles→OpenAL migration guide

**Relevance:**
- Audio stack architecture
- OpenAL integration
- Miles Sound System replacement

**Key Features:**
- OpenAL implementation
- Audio abstraction
- Platform-specific audio handling

---

### thesuperhackers-main

**Type:** Upstream baseline (original Generals code)

**Purpose:**
- Upstream baseline for regression checks
- Original game code reference
- Feature completeness verification

**Relevance:**
- Regression testing
- Feature comparison
- Baseline behavior

**Key Features:**
- Original Generals code
- Zero Hour expansion
- Complete feature set

---

## Reference Usage

### Platform Development

**fighter19-dxvk-port:**
- Graphics integration patterns
- SDL3 platform layer
- DXVK wrapper examples

### Audio Development

**jmarshall-win64-modern:**
- OpenAL architecture
- Audio abstraction patterns
- Miles→OpenAL migration

### Regression Testing

**thesuperhackers-main:**
- Baseline feature set
- Expected behavior
- Regression checks

---

## Reference Maintenance

### When to Update

- **fighter19-dxvk-port:** Platform-specific fixes, SDL3 updates
- **jmarshall-win64-modern:** OpenAL improvements, audio fixes
- **thesuperhackers-main:** Upstream changes, new features

### When to Add New References

- New platform technology
- Significant feature implementation
- Major architecture change
- Critical bug fix pattern

---

## See Also

- [ENTITIES/PLATFORM.md](./PLATFORM.md) — Platform technologies
- [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md) — Platform migrations
- [CONCEPTS/RAG.md](../CONCEPTS/RAG.md) — Platform isolation

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, reference documentation
