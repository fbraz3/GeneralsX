# 2026-02 Lessons Learned - Phase 1 Linux Graphics Port

**Month**: February 2026  
**Sessions Covered**: 19-24  
**Focus Area**: Linux graphics port (DXVK + SDL3) + build system stabilization

---

## 🎯 Critical Lessons

### 1. CMake Asymmetry Spreads Like Wildfire 🔴

**Problem**: Initial CMake setup configured d3d8lib differently for Windows vs Linux builds.

**Impact**:
- Windows builds worked, Linux builds failed silently
- Issue took 3 sessions to diagnose
- Root cause: Platform-specific #ifdef inside CMakeLists.txt

**Lesson**: 
- ✅ **Always build BOTH target platforms** in same session (even if one fails)
- ✅ **CMake configuration must be symmetric** - platform differences belong in source code, not build config
- ✅ **Use feature flags** (`SAGE_USE_DX8`, `BUILD_WITH_DXVK`) not platform guards (#ifdef) in CMake

**Pattern**:
```cmake
# ❌ WRONG - CMake platform decision
if(WIN32)
    target_link_libraries(d3d8lib PUBLIC directx)
else()
    target_link_libraries(d3d8lib PUBLIC dxvk)
endif()

# ✅ RIGHT - Feature flag decision
if(SAGE_USE_DX8)
    target_link_libraries(d3d8lib PUBLIC directx)  # or dxvk
endif()
```

---

### 2. Legacy Code Has Hidden Platform Assumptions 🏚️

**Problem**: Dozens of `#include <windows.h>` scattered in old code, assumed to always be available.

**Impact**:
- Linux builds failed with 100+ "windows.h not found" errors
- Each fixed manually, one by one
- Spread across multiple build attempts

**Lesson**:
- ✅ **Create platform abstraction layer FIRST** before fixing code
- ✅ **Use header shim pattern** (`windows_compat.h`) to intercept system includes
- ✅ **Centralize platform guarding** rather than fixing each occurrence individually

**Anti-pattern** (saw this, don't do it):
```cpp
// ❌ SCATTERED across 20+ files
#ifdef _WIN32
    #include <windows.h>
    #define MY_CONSTANT 100
#else
    #define MY_CONSTANT 200
#endif
```

**Pattern** (do this):
```cpp
// ✅ Core/Libraries/Include/compat/windows_compat.h
#ifndef _WINDOWS_COMPAT_H_
    #include <my_platform_definitions>
#endif

// Then in source files
#include "compat/windows_compat.h"  // Works everywhere
```

---

### 3. Precompiled Headers Hide Real Errors 🙈

**Problem**: PCH (precompiled headers) compilation succeeded, but real source files had hundreds of errors.

**Impact**:
- Spent 1 session debugging "PCH won't compile" when real issue was source dependencies
- CMake didn't report errors until after PCH phase
- False sense of progress

**Lesson**:
- ✅ **Disable PCH early in porting** (`PCH_ENABLED=OFF` in CMake)
- ✅ **Compile real source first**, add PCH optimization later
- ✅ **PCH errors are red herrings** in cross-platform ports - fix underlying includes

**Workflow**:
```bash
# Phase 1: Disable PCH (find all real errors)
cmake --preset linux64-deploy -DPCH_ENABLED=OFF

# Phase 2: Enable PCH after core build works
cmake --preset linux64-deploy -DPCH_ENABLED=ON
```

---

### 4. Reference Code Differs More Than Expected 📖

**Problem**: fighter19's DXVK solution looked straightforward, but details varied from our codebase.

**Impact**:
- Copy-paste didn't work (type mismatches, function signatures)
- Required re-architecting 3 times to find the pattern
- Sessions 16-18 spent on this alone

**Lesson**:
- ✅ **Study reference code ARCHITECTURE, not implementation**
- ✅ **Adapt patterns to your codebase** rather than copy-paste
- ✅ **Reference code is proof-of-concept**, not production template

**Process**:
```
Reference → Understand Pattern → Adapt to OUR Code → Test → Refine
No:    Reference → Copy-Paste → Compile Error → Give Up
```

---

### 5. Windows Compatibility Testing is Early Validation 🔄

**Problem**: Made changes that broke Windows VC6 build, didn't test until late.

**Impact**:
- Had to revert changes
- Regression took 2 hours to diagnose
- Could have been caught immediately if tested both platforms

**Lesson**:
- ✅ **Test Windows build DAILY** even when focusing on Linux
- ✅ **Use CI/CD or scripting** to validate both platforms quickly
- ✅ **"Linux works, Windows broken" is still a failure**

**Pattern**:
```bash
# After EACH significant change
./scripts/docker-build-mingw-zh.sh mingw-w64-i686    # Windows cross-compile
./scripts/docker-build-linux-zh.sh linux64-deploy    # Linux native build
# Both must succeed
```

---

## 🎓 Anti-Patterns to Avoid

### 1. ❌ Lazy Stubs for Platform-Specific Code

**Seen**: Empty `{}` implementations for Linux-only functions, planning to "do it later"

**Problem**: Breaks determinism, hides actual requirements, technical debt spirals

**Alternative**: 
- Document stub with `TODO:` or `FIXME:` comment
- Assert/log if called unexpectedly
- Make it visible, not silent

### 2. ❌ Mixing Platform Logic into Game Logic

**Seen**: `#ifdef __linux__` inside gameplay calculations

**Problem**: Breaks gameplay determinism, makes replays fragile, hard to test

**Alternative**: 
- Isolate platform code to `Core/GameEngineDevice/` and `Core/Libraries/Source/Platform/`
- Inject platform-specific behavior via callbacks/interfaces
- Keep game logic pure and testable

### 3. ❌ "Works on My Mac" Testing

**Seen**: Testing only on macOS Docker, not in actual target environments

**Problem**: Doesn't catch Linux-specific issues, terminal assumptions, library paths

**Alternative**:
- Test in Linux VM sometimes (not just Docker)
- Test Windows build at least weekly
- Use GitHub Actions for continuous validation

---

## 🏆 Patterns That Worked Well

### 1. ✅ Docker-Based Development

**Why**: Isolated build environment, reproducible results, no Mac pollution

**How**: Used `./scripts/docker-build-linux-zh.sh` consistently

**Result**: Reliable builds, easy to validate in clean environment

### 2. ✅ Handoff Documentation

**Why**: SESSION handoff document captures context for next session

**How**: Each session end = detailed handoff (blockers, achievements, next steps)

**Result**: Rapid context recovery, no "what were we doing?" moment

### 3. ✅ Minimal Diffs Approach

**Why**: Small, focused changes easier to validate and debug

**How**: Fix ONE category of issues per session (e.g., "Windows compatibility layer" or "D3D type definitions")

**Result**: Fast debugging, clear cause-effect, easier to revert if needed

---

## 📈 Process Improvements Discovered

### 1. Build Validation Script Pattern

**Current**: Manual `docker-build-linux-zh.sh` commands  
**Improved**: Created consolidated `validate-both-platforms.sh`

```bash
#!/bin/bash
# Validate Windows + Linux builds before commit
docker-build-mingw-zh.sh mingw-w64-i686 || exit 1
docker-build-linux-zh.sh linux64-deploy || exit 1
echo "✓ Both platforms validated"
```

### 2. Session Checklist

**Current**: Free-form session work  
**Improved**: Use standardized checklist in handoff

```markdown
## Session Checklist
- [ ] Windows VC6 build still works
- [ ] Linux docker build compiles to [X%]
- [ ] No Windows regression
- [ ] Documented blockers clearly
- [ ] Next session has clear "first task"
```

### 3. Architecture Decision Log (ADL)

**Current**: Decisions scattered in handoffs  
**Improved**: Consolidate in `planning/ARCHITECTURE_DECISIONS.md`

```markdown
| Date | Decision | Rationale | Trade-offs |
|------|----------|-----------|-----------|
| 2026-02-09 | DXVK wrapper pattern | Minimal game code changes | Abstraction layer overhead |
```

---

## 🔮 Recommendations for Future Phases

### Phase 2 (Audio) - What We Learned

- **Use OpenAL abstraction layer** pattern from start (don't repeat graphics layer mistakes)
- **Miles → OpenAL mapping** simpler than DirectX → Vulkan, validate early
- **Consider audio determinism** requirement (replays need matching audio events)

### Phase 3+ (Video) - Considerations

- **Reference solution too complex** - investigate simpler video skipping approach
- **Balance features vs complexity** - full video support may not be worth it

### General Process Improvement

- **Establish "Platform Compatibility Review" gate** - before merging any code, check both platforms build
- **Monthly lessons consolidation** - don't let knowledge scatter
- **Reference code audit sprint** - map all 3 reference repos to patterns once, document once

---

## Session-Specific Insights

### Session 19-20: Foundation Shakes ⚠️
- Hidden complexity of DirectX8 type availability
- SDL3 headers dependency chain very deep
- Learned: **Architecture decisions early pay dividends**

### Session 21-22: Hard Yards 💪
- Network/FTP code surprisingly platform-portable
- Small fixes compound into visible progress
- Learned: **Persistence on small issues unlocks momentum**

### Session 23-24: Breakthrough Approach 🚀
- Build progression tracking visible (112→200+ files)
- Handoff documentation clarity improves velocity
- Learned: **Transparency into progress boosts morale**

---

## Metrics & Velocity

| Metric | Value | Trend |
|--------|-------|-------|
| Build completion % | 112→200+ files (per session) | ⬆️ Accelerating |
| Error categories resolved | 8+ (per phase) | ⬆️ Growing |
| Session handoff clarity | Sessions 19-24 | ⬆️ Improving |
| Windows regression count | 1 (Session 23, fixed) | ➡️ Stable |

---

## Conclusion

**Phase 1 Linux Graphics Port** has surfaced critical patterns for cross-platform game development:

1. **Platform symmetry matters** - build BOTH platforms, always
2. **Architecture abstractions save time** - witness our CMake+compat layer pattern
3. **Reference code is guide, not gospel** - adapt, don't copy-paste
4. **Testing both platforms is non-negotiable** - Windows break = Phase failure
5. **Handoff documentation multiplies velocity** - next session starts ahead

**For Phase 2 (Audio)**: Apply these lessons immediately. **No lazy stubs. No platform logic in game code. Test both platforms.**

---

**Written**: 2026-02-11  
**Based on**: Sessions 19-24 handoffs and reports  
**Next review**: 2026-03-15 (after Phase 2 completion)
