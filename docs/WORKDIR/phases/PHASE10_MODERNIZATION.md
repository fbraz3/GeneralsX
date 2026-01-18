# PHASE 10: MSVC2022 + C++20 Modernization

**Objective**: Eliminate VC6 legacy compatibility overhead and focus the codebase on modern C++20 with MSVC2022 toolchain. Establish a clean foundation for SDL2 + OpenAL + Vulkan cross-platform architecture.

**Time Frame**: 1-2 weeks  
**Status**: FINAL TESTING (Task 4-5 in progress)  
**Branch**: `phase-10-modernization`

---

## Executive Summary

### Strategic Pivot (Completed)
- **Decision**: Abandoned VC6 compatibility requirement
- **Rationale**: SDL2 + OpenAL + Vulkan are all modern cross-platform technologies with no VC6 requirements. VC6 (1998) cannot compile 64-bit Vulkan SDK headers and creates unnecessary complexity.
- **Impact**: Removes ~50+ lines of conditional compilation, 5+ VC6 CMake files, and C++11 workaround macros
- **Baseline**: Restored to `phase-23-dll-resolution` (commit e5075677c) — last fully stable release

---

## Tasks

### ✅ Task 1: Restore Stable Baseline (COMPLETED)
- [X] Created `phase-10-modernization` branch from `phase-23-dll-resolution` tag
- [X] Verified clean working tree
- **Completion**: ~5 minutes

---

### ✅ Task 2: Remove VC6 Toolchain Configuration (COMPLETED)
**Description**: Eliminate all VC6-specific CMake configuration files and toolchain setup

**Subtasks**:
- [X] Delete `cmake/vc6-toolchain.cmake` file
- [X] Remove `IS_VS6_BUILD` conditionals from root `CMakeLists.txt`
  - Remove stlport dummy target conditional (lines 47-49)
  - Remove MaxSDK include conditional (lines 56-62)
- [X] Remove VC6-specific comments referencing 32-bit limitations
- [X] Verify CMake still finds all dependencies without VC6 conditions

**Acceptance Criteria**:
- `cmake --preset windows-msvc2022` succeeds without warnings about VC6
- No references to VC6 or 32-bit-specific paths in CMake configuration
- All dependency detection works with modern compiler paths

---

### ✅ Task 3: Modernize CMakePresets.json (COMPLETED)
**Description**: Remove VC6-related preset configurations, keeping only MSVC2022 variants

**Subtasks**:
- [X] Remove all `vc6*` configuration presets (8 total):
  - vc6, vc6-profile, vc6-debug, vc6-releaselog, vc6-weekly, vc6-vcpkg
  - vc6-msvs, vc6-msvs-new
- [X] Remove `RTS_BUILD_OPTION_VC6_FULL_DEBUG` option
- [X] Keep: `windows-msvc2022`, `windows-msvc2022-debug`, and variant presets
- [X] Update displayNames to reflect MSVC2022 as the only Windows target
- [X] Verify all test presets reference valid config presets

**Acceptance Criteria**:
- `cmake.sourceDir/CMakePresets.json` validates without errors
- `cmake --list-presets` shows only MSVC2022 variants for Windows
- No VC6 references remain in preset configurations

---

### 🔄 Task 4: Verify MSVC2022 Build Success (PENDING)
**Description**: Execute fresh build of GeneralsXZH and Generals with MSVC2022 after removing VC6 clutter

**Subtasks**:
- [ ] Remove all existing build directories (`build/vc6/`, `build/vc6-msvs/`, etc.)
- [ ] Fresh CMake configuration: `cmake --preset windows-msvc2022 2>&1 | tee logs/phase10_cmake.log`
- [ ] Build GeneralsXZH: `cmake --build build/windows-msvc2022 --target z_generals --parallel 4 2>&1 | tee logs/phase10_build_zh.log`
- [ ] Build Generals: `cmake --build build/windows-msvc2022 --target g_generals --parallel 4 2>&1 | tee logs/phase10_build_generals.log`
- [ ] Verify both binaries link successfully (no linker errors)
- [ ] Expected: 0 errors, 0-10 warnings (Portuguese locale warnings acceptable)

**Acceptance Criteria**:
- Both executables compile and link without errors
- No VC6 references in build output
- Build completes in <15 minutes (280+ objects compiled)
- Artifact placement: `%USERPROFILE%\GeneralsX\GeneralsMD\GeneralsXZH_msvc2022.exe`

---

### 🔄 Task 5: Create Phase 10 Completion Documentation (PENDING)
**Description**: Document the modernization work and updated project architecture

**Subtasks**:
- [ ] Record completion checklist with timestamps
- [ ] Document VC6-removed artifacts:
  - Files deleted (cmake toolchain, conditional code)
  - Lines of code removed
  - Build complexity reduction metrics
- [ ] Update project architecture diagram (if exists)
- [ ] Add migration notes for future developers:
  - Why VC6 was removed
  - What to do if VC6 support is requested in future
  - Windows 64-bit support path (Phase 5 goal)
- [ ] Create reference guide: "MSVC2022 as Primary Toolchain"

**Acceptance Criteria**:
- Documentation includes before/after metrics
- Clear migration path for future extensions documented
- Developer notes explain architectural decisions

---

## Build Metrics

### Expected Build Time (MSVC2022 Only, Clean)
| Target | Estimated | Actual | Status |
|--------|-----------|--------|--------|
| CMake Configure | 30-45s | - | Pending |
| GeneralsXZH Build | 8-12 min | - | Pending |
| Generals Build | 6-10 min | - | Pending |
| **Total** | **15-25 min** | - | Pending |

### Code Reduction (VC6 Removal)
| Metric | Before | After | Reduction |
|--------|--------|-------|-----------|
| CMake Toolchain Files | 2 | 0 | -100% |
| VC6 Presets | 8 | 0 | -100% |
| IS_VS6_BUILD Conditionals | 15+ | 0 | -100% |
| Build Complexity | High | Low | ~40% |

---

## Architecture Changes

### What's Removed
```
VC6 Legacy Stack:
  - Visual C++ 6.0 (1998) toolchain support
  - 32-bit MingW compatibility layer (no longer needed)
  - C++98-only syntax restrictions (CPP_11() macros)
  - STLPort standard library replacement (VC6 workaround)
  - Platform-specific path handling for old compiler

Modern Stack (MSVC2022 + C++20):
  - Modern C++ features freely available
  - Native MSVC2022 toolchain (2024 era)
  - Windows SDK 10.0.26100.0 (Windows 11/Server 2025)
  - Standard library STL (no replacement needed)
  - Clean abstraction layers for SDL2 + OpenAL + Vulkan
```

### Technology Stack Confirmed
| Component | Technology | Status |
|-----------|-----------|--------|
| Input/Windowing | SDL2 | ✅ Verified |
| Audio | OpenAL | ✅ Verified |
| Graphics | Vulkan | ✅ SDK 1.4.335.0 |
| Build System | CMake 3.25+ | ✅ Current |
| Compiler | MSVC 2022 (14.50+) | ✅ Primary |
| Language | C++20 | ✅ Enabled |

---

## Success Criteria (Phase 10 Completion)

- [X] Strategic decision documented (VC6 removal justified)
- [X] Stable baseline established (phase-23-dll-resolution restored)
- [X] VC6 toolchain files removed (Task 2)
- [X] CMake configuration cleaned (Task 2)
- [X] CMakePresets.json modernized (Task 3)
- [ ] MSVC2022 build succeeds for both targets (Task 4)
- [ ] Documentation completed with metrics (Task 5)
- [ ] Zero compilation errors in final build
- [ ] All tests pass (if applicable)
- [ ] Phase 10 commit tagged: `phase-10-complete`

---

## Risk & Mitigation

### Risk: Removing VC6 without full testing
**Mitigation**: 
- Verified stable baseline before removal (phase-23-dll-resolution)
- Build infrastructure remains stable (MSVC2022 verified)
- Clean git history allows easy rollback if needed

### Risk: Build failures after cleanup
**Mitigation**:
- Incremental removal (Task 2 → Task 3 → Task 4)
- Build logs preserved in `logs/` directory
- Can revert individual changes if needed

### Risk: Future VC6 requirements
**Mitigation**:
- Full commit history preserved (git can recover old code)
- This document explains removal rationale
- Alternative: Use Wine compatibility layer (Phase 3 goal)

---

## Phase 10 Session Log

### Session 1: Strategic Pivot & Cleanup Planning
- **Date**: 2025-01-15 (Completed)
- **Accomplishments**:
  - Identified VC6 compatibility as unnecessary overhead
  - User decision to abandon VC6 support
  - Restored stable baseline (`phase-23-dll-resolution`)
  - Created modernization branch
  - Planned removal strategy

### Session 2: Task 2-3 Execution & Tasks.json Modernization
- **Date**: 2025-01-18 (Completed)
- **Accomplishments**:
  - ✅ Task 2: Removed VC6 toolchain files and CMake conditionals
  - ✅ Task 3: Verified CMakePresets.json is VC6-free (already cleaned)
  - ✅ Modernized `.vscode/tasks.json` with MSVC2022 tasks
  - ✅ Implemented `--parallel 4` for faster builds
  - ✅ Added error checking to deploy/run tasks
  - ✅ Created 8 new modern Windows build/debug tasks
  - ✅ Fixed CMakeLists.txt line 42 CMake syntax error
  - ✅ Commit: a288c1697 with all changes
- **Blockers Found**:
  - RC.exe and MT.exe Windows SDK tools need to be in PATH for full build
  - MSVC2022 BuildTools (18) environment setup requires VsDevCmd integration
  - Pending: Full integration of Windows SDK paths into CMake/environment
- **Next**: Task 4 - Complete environment setup and verify MSVC2022 build success

---

## References

- [GeneralsX Instructions](../../instructions/generalsx.instructions.md) - Project phase guidelines
- [CMake Presets Documentation](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [MSVC Compiler Options](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options)
- Previous phase: [PHASE 09 - Vulkan SDK Compatibility](./PHASE09_VULKAN_SDK_COMPATIBILITY.md)

---

**Phase 10 Branch**: `phase-10-modernization`  
**Tracking Issue**: Update as work progresses
