# 2026-02 Lessons Learned - Phase 1 Linux Graphics Port

**Month**: February 2026  
**Sessions Covered**: 19-24, 28-30, 33, 34, 35  
**Focus Area**: Linux graphics port (DXVK + SDL3) + build system stabilization + compilation blockers + compat headers

---

## 🎯 Critical Lessons

### 1 (Session 35). Time/Clock Compatibility Layer Pattern — Pre-Stub Approach 🕐

**Problem**: W3DDisplay.cpp needs `QueryPerformanceCounter`/`QueryPerformanceFrequency` (Windows kernel32 APIs). GCC/Clang don't have these; must stub before code includes them.

**Attempted Approach (❌ WRONG)**:
- Define stubs inline in W3DDisplay.cpp
- Result: Conflicts with DXVK headers that also define them (ephemeral patch issue)
- Cause: Stubs must be defined BEFORE any conflicting includes

**Correct Approach (✅ RIGHT)**:
- Create compat header (`time_compat.h`) with pre-guards
- Define QueryPerformanceCounter/Frequency BEFORE anything includes Windows.h
- Use `#ifdef _WIN32` to expose Windows externs, `#else` for POSIX implementations

**Implementation**:
```cpp
// In GeneralsMD/Code/CompatLib/Include/time_compat.h
#ifdef _WIN32
extern "C" {
    BOOL __stdcall QueryPerformanceCounter(void* lpPerformanceCount);
    BOOL __stdcall QueryPerformanceFrequency(void* lpFrequency);
}
#else
// Linux: Implement using POSIX clock_gettime
inline BOOL QueryPerformanceCounter(void* lpPerformanceCount) {
    if (!lpPerformanceCount) return 0;
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    // Convert to 100-nanosecond intervals for Windows API compatibility
    int64_t* pCount = (int64_t*)lpPerformanceCount;
    *pCount = ts.tv_sec * 10000000LL + ts.tv_nsec / 100;
    return 1;
}
// Similar for QueryPerformanceFrequency...
#endif

// Then in source files that need it:
#include "compat/time_compat.h"  // Works everywhere
QueryPerformanceCounter(&tmp);   // Uses Windows API on Windows, clock_gettime on Linux
```

**Why This Works**:
- ✅ Defined in ONE place (no ephemeral patch issues)
- ✅ Pre-guard pattern prevents redefinition conflicts
- ✅ Inline implementations work for Linux stubs (no lib dependencies)
- ✅ Windows code uses native kernel32 implementations unchanged

**Reference**: Fighter19's approach + Session 33 Network.cpp pattern verification.

---

### 2 (Session 34-35). MSVC Compiler Builtins Are Not Portable — Use Standard Macros Instead 💻

**Problem**: MSVC has compiler intrinsics `__max`, `__min`, `__int64` not available in GCC/Clang.

**Anti-Pattern (❌ WRONG)**:
```cpp
Int buffer_size = __max(DEFAULT_SIZE, requested_size);  // MSVC only
Int64 timestamp = __int64 value;  // Platform-specific
```

**Correct Pattern (✅ RIGHT)**:
```cpp
// Use defined macros/typedefs from BaseTypeCore.h (already existing!)
Int buffer_size = MAX(DEFAULT_SIZE, requested_size);  // Portable
Int64 timestamp = value;  // Already typedef'd to int64_t on Linux
```

**Discovery**: The game codebase ALREADY has these macros/typedefs defined:
- `BaseTypeCore.h` defines `MAX(a,b)` macro
- `BaseTypeCore.h` defines `Int64` typedef = `int64_t`
- Sessions 34-35 just needed to find and use them

**Cleanup Audit**:
```bash
# After major compilation session, grep for remaining MSVC builtins:
grep -r "__max|__min|__int64" GeneralsMD/ Generals/ Core/ \
  | grep -v "// TheSuperHackers @" | head -20  # Find unchanged instances
```

---

### 3 (Session 35). Large Mono-Platform Functions Need Wrapper Wrapping, Not Line-By-Line Stubbing 📦

**Problem**: Screenshot function (CreateBMPFile, takeScreenShot) uses 80+ lines with 7+ Windows APIs (CreateFile, WriteFile, LocalAlloc, bitmap types, etc.).

**Anti-Pattern (❌ WRONG)**:
```cpp
// Stub each line individually
#ifdef _WIN32
    hf = CreateFile(...);
#else
    // Create a dummy handle?
#endif

#ifdef _WIN32
    WriteFile(hf, ...);
#else
    // Write to... what?
#endif
// Nightmare: 80 lines, 8+ guards, incomprehensible
```

**Correct Pattern (✅ RIGHT - Wrapper Wrapping)**:
```cpp
// Wrap ENTIRE function
#ifdef _WIN32
static void CreateBMPFile(LPTSTR pszFile, char *image, Int width, Int height) {
    // Full Windows impl (80 lines, all Windows-specific)
    HANDLE hf = CreateFile(...);
    WriteFile(...);
    LocalFree(...);
    CloseHandle(hf);
}
#else
// Linux stub - defer to Phase 3
static void CreateBMPFile(const char* pszFile, char *image, Int width, Int height) {
    // TODO (Phase 3): Implement SDL3-based screenshot capture
}
#endif
```

**When to Use Wrapper Wrapping**:
1. **Dependency Density**: >50% of function uses Windows APIs
2. **Type Complexity**: Uses 5+ Windows-specific types/constants
3. **Non-critical Feature**: Screenshot export, file save dialogs, admin ops (don't affect core gameplay determinism)

**When NOT to Use**:
- Low-dependency code (1-2 API calls) → use inline stubs
- Gameplay logic (must work on both platforms) → refactor into platform layer
- Performance-critical paths → profile before wrapping

---

### 1 (Session 19-30). CMake Asymmetry Spreads Like Wildfire 🔴

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

### 6. DXVK Pre-Guards Are Ineffective 🛡️

**Problem**: Attempted to prevent DXVK type conflicts by pre-defining guard macros in windows_compat.h.

**Impact**:
- Pre-guards failed completely (DXVK doesn't check them before defining types)
- Build stuck at [321/1254] for entire session debugging
- Manual ephemeral patches required in DXVK headers directly
- Patches lost on every clean build (`rm -rf build/`)

**Lesson**:
- ✅ **DXVK wrapper headers don't check custom guards** - must patch source directly
- ✅ **Ephemeral patches are dangerous** - lost on CMake reconfigure
- ✅ **Automate patching via CMake** - use `ExternalProject_Add` with `PATCH_COMMAND`
- ✅ **Guard pattern location matters** - add guards IN the header being included, not before

**Pattern**:
```cpp
// ❌ DOESN'T WORK - DXVK ignores this
// GeneralsMD/Code/CompatLib/Include/windows_compat.h
#ifndef _WIN32
    #define _MEMORYSTATUS_DEFINED 1    // Trying to prevent DXVK from defining
    #define _IUNKNOWN_DEFINED 1
#endif
#include <windows_base.h>              // DXVK still defines them anyway!

// ✅ WORKS - Patch DXVK header directly
// build/_deps/dxvk-src/include/dxvk/windows_base.h
#ifndef _MEMORYSTATUS_DEFINED
typedef struct MEMORYSTATUS {
    DWORD  dwLength;
    SIZE_T dwTotalPhys;
} MEMORYSTATUS;
#define _MEMORYSTATUS_DEFINED 1
#endif
```

**Future Fix**: Automate with CMake patch command (avoid manual edits).

---

### 7. Incremental Builds Mask True Complexity 📊

**Problem**: Build target count varied wildly (1254→936→128) between builds, making progress tracking confusing.

**Impact**:
- Hard to measure actual progress ("Did we fix things or just recompile less?")
- Ninja only rebuilds changed files + dependents
- CompatLib change = entire Core + Game rebuild (100s of targets)
- Clean build reveals true scale (1254 targets for full GeneralsXZH)

**Lesson**:
- ✅ **Target count is NOT progress metric** - use percentage of total instead
- ✅ **Understand Ninja dependency tracking** - file timestamp changes trigger cascading rebuilds
- ✅ **Clean builds reveal reality** - incremental hides technical debt
- ✅ **Track cumulative fixes, not current target count**

**Breakdown** (Clean Build Target Distribution):
```
Total: 1254 targets (GeneralsXZH linux64-deploy)
- SDL3 dependencies: ~600 targets
- GameSpy networking: ~50 targets  
- CompatLib: ~20 targets
- Core engine: ~100 targets
- Game logic: ~400 targets
- Tools: ~84 targets
```

**Why Counts Vary**:
- Change `windows_compat.h` → recompile Core + Game (~500 targets)
- Change `LocalFile.cpp` only → recompile dependents (~10 targets)
- Clean build → ALL 1254 targets

---

### 8. GameSpy Code Is Safe to Stub (Legacy Services) 🪦

**Problem**: GameSpy networking code (lobby, SNMP, matchmaking) has Windows-specific APIs that don't port easily.

**Impact**:
- StagingRoomGameInfo.cpp blocker (44 SNMP errors)
- Concerns about breaking multiplayer functionality
- Temptation to invest weeks porting complex networking stack

**Lesson**:
- ✅ **GameSpy online services shut down in 2013** - servers don't exist anymore
- ✅ **LAN multiplayer doesn't need GameSpy lobby** - direct IP connection works
- ✅ **Stubbing online features is safe** - preserves offline/LAN gameplay
- ✅ **fighter19/jmarshall both removed GameSpy** - proven approach

**What's Safe to Stub**:
- Online lobby/matchmaking (servers dead)
- GameSpy SNMP monitoring (diagnostic tool)
- NAT traversal helpers (needed for online only)
- Account authentication (no servers)

**What Must Work**:
- LAN discovery (local network)
- Direct IP connection
- Replay system
- Single-player campaign

**Pattern** (Reference from fighter19):
```cpp
// GameSpy lobby functions
#ifdef _WIN32
    // Full Windows GameSpy implementation
#else
    // Linux: Stub returns failure, LAN multiplayer unaffected
    bool ConnectToGameSpyLobby() { return false; }
#endif
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

### Session 28-30: Systematic Blocker Elimination 🔨
- DXVK conflicts require direct patching (pre-guards insufficient)
- 64-bit pointer cast pattern established (20+ occurrences fixed)
- Platform isolation working (`#ifdef _WIN32` guards compile cleanly)
- "100 targets/day" question answered: **viable when clean, blockers more valuable than raw speed**
- Progress: [321/1254] → [~800/1254] (25.6% → 63.8%)
- Learned: **Methodical blocker resolution beats rushing toward arbitrary metrics**

---

## Metrics & Velocity

| Metric | Value | Trend |
|--------|-------|-------|
| Build completion % | 112→200+ files (S19-24), 321→800/1254 (S28-30) | ⬆️ Accelerating |
| Error categories resolved | 8+ (per phase), 11 blockers (S28-30) | ⬆️ Growing |
| Session handoff clarity | Sessions 19-24, 28-30 | ⬆️ Improving |
| Windows regression count | 1 (Session 23, fixed) | ➡️ Stable |
| Phase 1 compilation progress | 63.8% (800/1254 targets) | ⬆️ Strong |
| Pointer cast fixes | 20+ cumulative (S28-30) | ⬆️ Pattern established |

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

**Written**: 2026-02-11 (Sessions 19-24), Updated: 2026-02-13 (Sessions 28-30)  
**Based on**: Sessions 19-24, 28-30 handoffs and reports  
**Next review**: 2026-03-15 (after Phase 2 completion)
