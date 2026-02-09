# Phase 1: Graphics - Linux DXVK Port - NEXT STEPS

**Status**: CMAKE architecture FIXED, Windows compatibility layer IN PROGRESS  
**Date**: 2026-02-09  
**Target**: GeneralsXZH Linux 64-bit with full compilation + graphics rendering

---

## ✅ COMPLETED (Session 13)

### CMAKE Architecture Fixes
1. ✅ **Fixed dx8.cmake include condition** - Now always included (not just 32-bit Windows)
2. ✅ **Conditional DXVK/DX8 selection** - SAGE_USE_DX8 flag properly routes to DXVK (Linux) or min-dx8-sdk (Windows)
3. ✅ **d3d8lib target ordering** - Defined early in Core, configured late in CompatLib
4. ✅ **d3d8lib symmetry** - Now properly configured for BOTH Windows and Linux (critical fix)
5. ✅ **Removed d3dx8 platform-specific includes** - math library is pure math, not graphics-specific

### Windows Compatibility Layer (Started)
1. ✅ **windows.h shim** - Created to redirect to windows_compat.h
2. ✅ **_int64 typedefs** - Added to types_compat.h for MSVC compatibility

---

## 🔄 IN PROGRESS (Next Session Priority Order)

### Priority 1: Fix Broken Builds (CRITICAL)
**Goal**: Get Linux 64-bit Docker build past precompiled header phase

**Tasks**:
- [ ] **Fix windows.h include propagation**
  - Current: `#include <windows.h>` fails on Linux (system header)
  - Solution: Add CompatLib/Include to include search FIRST (-I prefix)
  - File: `GeneralsMD/Code/CompatLib/CMakeLists.txt` - ensure CompatLib includes are primary
  - Expected Result: Compiler finds our `windows.h` shim before system one

- [ ] **Resolve _int64 typedef conflicts in profile library**
  - Current Error: `conflicting declaration 'typedef uint64_t _int64'` in profile_funclevel.h
  - Root Cause: profile_*.h has conflicting _int64 definitions (profile_highlevel.h vs profile_funclevel.h)
  - Solution: Single definition in types_compat.h, platform-guard others
  - Files: 
    - Core/Libraries/Source/profile/profile_*.h - Add `#ifndef _int64` guards
    - Verify typedef consistency
  - Expected Result: profile precompiled header builds without conflicts

- [ ] **Handle unterminated #ifdef _WIN32**
  - Current: dx8fvf.h has malformed ifdef blocks on Linux
  - Solution: Wrap file with proper include guards or platform detection
  - File: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8fvf.h
  - Expected Result: Platform guards evaluated correctly

### Priority 2: Full Docker Rebuild Validation
**Goal**: Complete Linux 64-bit compilation, generate native ELF binary

**Tasks**:
- [ ] **Full rebuild after Windows header fixes**
  ```bash
  rm -rf build/linux64-deploy
  ./scripts/docker-build-linux-zh.sh linux64-deploy 2>&1 | tee logs/phase1_build_clean.log
  ```
  - Monitor for: New compilation errors (will appear after headers fixed)
  - Success indicator: Compilation reaches linking phase (not just preprocessing)

- [ ] **Track Windows.h related errors**
  - Document each missing header (HFONT, HBITMAP, HDC, etc.)
  - Create compat stubs as-needed
  - Reference: fighter19 CompatLib for patterns

- [ ] **Monitor GDI type errors**
  - HDC, HBITMAP, HFONT, HBRUSH, HPEN - all GDI (Windows drawing)
  - Pattern: Either stub them or keep them Windows-only behind `#ifdef _WIN32`
  - Files: Likely in WWVegas/WW3D2 (graphics library)

### Priority 3: Platform Isolation Verification
**Goal**: Ensure all Windows-specific code is behind proper guards or compat layer

**Tasks**:
- [ ] **Audit includes for direct Windows dependencies**
  ```bash
  grep -r "^#include <windows.h>" GeneralsMD/ Core/ --include="*.cpp" --include="*.h"
  grep -r "^#include <d3d8.h>" GeneralsMD/ Core/ --include="*.cpp" --include="*.h"
  ```
  - Expected: All should be via compat layer or platform-guarded

- [ ] **Verify no #ifdef _WIN32 in application logic**
  - Should be in: Platform layers, GDI code, registry code
  - Should NOT be in: Game logic, gameplay, entity systems
  - Files to check: GenerlsMD/Code/GameEngine/ (should be clean)

---

## 🎯 NEXT PHASE AFTER BUILD SUCCESS (Priority 4+)

### Phase 1 Final: Smoke Testing (When Linux binary builds)
- [ ] Launch GeneralsXZH in Docker container: `./GeneralsXZH -win -noshellmap`
- [ ] Verify graphics rendering (main menu visible)
- [ ] Load Skirmish map, verify gameplay runs without crashes
- [ ] Check that audio is silent (expected - OpenAL integration pending Phase 2)

### Windows Build Validation
- [ ] Rebuild Windows VC6 preset to ensure no regressions
  ```bash
  cmake --preset vc6
  cmake --build build/vc6 --target z_generals
  ```
- [ ] Verify VC6 retail compatibility (replays still work)

### Phase Completion: Acceptance Criteria
Per `generalsx.instructions.md` Phase 1 completion requires:
1. ✅ GeneralsXZH builds with Linux preset (linux64-deploy) ← Working toward this
2. ✅ Native Linux ELF binary launches and reaches main menu ← Blocked on windows header fixes
3. ✅ Basic gameplay (skirmish map) runs without crashes ← Follows from #2
4. ✅ Windows builds (VC6/Win32) still compile and run ← To be validated after linux64 succeeds
5. ✅ Platform-specific code properly isolated ← In progress, need audit

---

## 📊 Risk Mitigation

### Risk: Circular Include Dependencies
- **Mitigation**: Verify no `windows.h` includes CompatLib includes that include `windows.h`
- Check: `windows_compat.h` should NOT include `windows.h` again
- **Action**: Review GeneralsMD/Code/CompatLib/Include/windows_compat.h before rebuild

### Risk: Windows Builds Break
- **Mitigation**: Don't modify Windows-specific code
- The conditional SAGE_USE_DX8 routes cleanly: Windows → min-dx8-sdk, Linux → DXVK
- **Action**: Test Windows builds (VC6) after Linux is stable

### Risk: Precompiled Headers Cause Subtle Errors
- **Mitigation**: Rebuild PCH after every compat header change
- CMAKE regenerates PCH = slower builds, but catches issues faster
- **Action**: Use `cmake --build build/linux64-deploy --target cmake_pch.hxx.gch` if stuck

---

## 📝 Development Commands

### Quick Rebuild (Linux Docker)
```bash
cd /path/to/generals-linux
./scripts/docker-build-linux-zh.sh linux64-deploy 2>&1 | tee logs/phase1_build.log
```

### Check Specific Error
```bash
# Find all windows.h includes
grep -r "#include <windows.h>" GeneralsMD/ --include="*.h" --include="*.cpp"

# Check d3d8.h accessibility
grep -r "#include <d3d8.h>" GeneralsMD/ --include="*.h" --include="*.cpp"
```

### CMake Clean Reconfigure
```bash
rm -rf build/linux64-deploy
cmake --preset linux64-deploy
```

---

## 🔗 Cross-Reference

- **Phase 1 Acceptance Criteria**: [generalsx.instructions.md](/Users/felipebraz/PhpstormProjects/pessoal/generals-linux/.github/instructions/generalsx.instructions.md#phase-1-linux-graphics---dxvk-integration)
- **Build System**: [CMakeLists.txt](/Users/felipebraz/PhpstormProjects/pessoal/generals-linux/CMakeLists.txt)
- **DXVK Config**: [cmake/dx8.cmake](/Users/felipebraz/PhpstormProjects/pessoal/generals-linux/cmake/dx8.cmake)
- **CompatLib Config**: [GeneralsMD/Code/CompatLib/CMakeLists.txt](/Users/felipebraz/PhpstormProjects/pessoal/generals-linux/GeneralsMD/Code/CompatLib/CMakeLists.txt)
- **Windows Compat Headers**: [GeneralsMD/Code/CompatLib/Include/](/Users/felipebraz/PhpstormProjects/pessoal/generals-linux/GeneralsMD/Code/CompatLib/Include/)

---

**Last Updated**: 2026-02-09 Session 13  
**Last Action**: Fixed critical d3d8lib configuration asymmetry  
**Next Reviewer**: Yourself in Session 14
