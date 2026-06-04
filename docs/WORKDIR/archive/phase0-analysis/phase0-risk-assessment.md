# Phase 0: Risk Assessment & Mitigation

**Created**: 2026-02-07
**Status**: In Progress

## Objective

Identify potential risks for the Linux port and establish mitigation strategies before implementation begins.

## Critical Risks

### 1. Gameplay Determinism Break
**Impact**: HIGH
**Probability**: MEDIUM

**Risk**: Changes to rendering or audio layers accidentally affect gameplay logic, breaking replay compatibility and deterministic simulation.

**Symptoms**:
- Replays desynchronize
- Multiplayer goes out of sync
- Same inputs produce different results

**Mitigation**:
- ✅ Use abstraction layers (no core logic changes)
- ✅ Keep Windows DX8/Miles paths intact for verification
- ✅ Test VC6 builds with replay suite after every change
- ✅ Document determinism boundaries (what affects it, what doesn't)

**Validation**:
```bash
# After any change, run replay tests on VC6 build
cmake --preset vc6
cmake --build build/vc6 --target z_generals
generalszh.exe -jobs 4 -headless -replay GeneralsReplays/**/*.rep
```

**Status**: Mitigated by design (abstraction-only approach)

---

### 2. Windows Build Regression
**Impact**: HIGH
**Probability**: MEDIUM

**Risk**: Linux-specific changes break Windows builds (VC6 or Win32 presets).

**Symptoms**:
- Windows build fails to compile
- Windows binary crashes or has visual/audio issues
- Retail compatibility lost

**Mitigation**:
- ✅ Test Windows builds BEFORE every commit
- ✅ Use `#ifdef` guards for all platform code
- ✅ CI/CD validates both platforms
- ✅ Never remove Windows code paths

**Validation**:
```bash
# Before every commit
cmake --preset vc6 && cmake --build build/vc6
cmake --preset win32 && cmake --build build/win32
```

**Status**: Mitigated by process (mandatory testing)

---

### 3. Zero Hour Feature Gaps
**Impact**: HIGH
**Probability**: MEDIUM

**Risk**: jmarshall's reference only covers Generals base game. Zero Hour has additional features that may need special handling.

**Symptoms**:
- Expansion features missing/broken
- Zero Hour-specific audio/rendering fails
- Mod compatibility issues

**Mitigation**:
- ✅ Focus on GeneralsXZH (Zero Hour) as primary target
- ✅ Cross-reference fighter19 (has Zero Hour coverage)
- ✅ Test with Zero Hour content explicitly
- ⚠️ Document Generals-only assumptions from jmarshall
- ⚠️ Identify Zero Hour expansion features early

**Validation**:
- Test with Zero Hour campaigns
- Test with expansion-specific units/buildings
- Test with GLA/China factions (expansion features)

**Status**: Partially mitigated (fighter19 covers Zero Hour, jmarshall doesn't)

---

### 4. DXVK/Vulkan Driver Issues
**Impact**: MEDIUM
**Probability**: MEDIUM

**Risk**: DXVK translation layer has bugs or performance issues. Vulkan driver compatibility varies across Linux distributions.

**Symptoms**:
- Rendering artifacts or crashes
- Performance degradation vs native DX8
- Driver-specific issues (NVIDIA vs AMD vs Intel)

**Mitigation**:
- ✅ Use proven DXVK version (from fighter19)
- ✅ Test on multiple GPU vendors (NVIDIA, AMD, Intel)
- ✅ Document minimum driver versions
- ⚠️ Provide fallback documentation (Wine/Proton)
- ⚠️ Consider Mesa vs proprietary driver differences

**Validation**:
- Test on VirtualBox (Intel GPU emulation)
- Test on real hardware (NVIDIA/AMD if available)
- Monitor DXVK logs for warnings/errors

**Status**: Mitigated by using proven DXVK integration

---

### 5. Audio Synchronization Issues
**Impact**: MEDIUM
**Probability**: MEDIUM

**Risk**: OpenAL backend has timing differences from Miles Sound System, causing audio desync or pops/clicks.

**Symptoms**:
- Audio-visual desync
- Crackling or popping sounds
- Music timing off
- Voice lines cut off

**Mitigation**:
- ✅ Use jmarshall's proven OpenAL implementation as base
- ✅ Maintain Miles→OpenAL API compatibility
- ✅ Test audio quality extensively
- ⚠️ Implement fallback (disable audio if broken)
- ⚠️ Document known audio quality differences

**Validation**:
- Compare audio timing with Windows build
- Test 3D positional audio
- Test music transitions
- Test battle audio (many simultaneous sounds)

**Status**: Mitigated by proven reference implementation

---

### 6. Cross-Compilation Complexity
**Impact**: LOW
**Probability**: HIGH

**Risk**: Building from macOS for Linux/Windows introduces toolchain complexity and debugging difficulty.

**Symptoms**:
- Build failures on macOS
- "Works on my machine" syndrome
- Difficult to debug cross-compiled binaries

**Mitigation**:
- ✅ Use Docker for reproducible builds
- ✅ Document exact build environment
- ✅ Test in VMs (Linux/Windows) for validation
- ✅ Use logging extensively (capture build/run logs)

**Validation**:
- Build in Docker
- Test on real target platform
- Compare logs between macOS build and VM test

**Status**: Mitigated by Docker workflow

---

### 7. SDL3 Input/Windowing Issues
**Impact**: MEDIUM
**Probability**: LOW

**Risk**: SDL3 windowing/input handling differs from Win32, causing input lag, window management issues, or display bugs.

**Symptoms**:
- Input lag or unresponsive controls
- Fullscreen/windowed mode issues
- Multi-monitor problems
- Resolution switching problems

**Mitigation**:
- ✅ Use fighter19's proven SDL3 integration
- ✅ Test windowed and fullscreen modes
- ✅ Test resolution changes
- ⚠️ Document SDL3 version requirements

**Validation**:
- Test window creation/destruction
- Test input responsiveness
- Test resolution switching
- Test alt-tab behavior

**Status**: Mitigated by proven reference implementation

---

### 8. File Path Case Sensitivity
**Impact**: LOW
**Probability**: HIGH

**Risk**: Windows is case-insensitive, Linux is case-sensitive. Assets referenced with wrong case will fail to load.

**Symptoms**:
- "File not found" errors on Linux
- Works on Windows, fails on Linux
- Mod compatibility broken

**Mitigation**:
- ✅ Implement case-insensitive file lookup on Linux
- ✅ Test with known case-sensitive issues
- ✅ Use VFS (virtual file system) abstraction
- ⚠️ Document case sensitivity requirements for modders

**Validation**:
- Test with .big file archives
- Test with loose file mods
- Test with various asset types

**Status**: Known issue, mitigation available (case-insensitive lookup)

---

## Medium Risks

### 9. Replay Compatibility Testing Overhead
**Impact**: MEDIUM
**Probability**: HIGH

**Risk**: Replay testing requires Windows VM, slowing development cycle.

**Mitigation**:
- Use CI/CD for automated replay testing
- Keep replay suite small but representative
- Only test replays on VC6 build (not every platform)

**Status**: Process overhead accepted

---

### 10. Debugging Difficulty
**Impact**: MEDIUM
**Probability**: MEDIUM

**Risk**: Debugging native Linux builds from macOS is harder than local debugging.

**Mitigation**:
- Use extensive logging (capture to files)
- Use remote debugging (gdb over SSH)
- Use VM for interactive debugging
- Keep debug symbols in builds

**Status**: Accepted complexity

---

### 11. Performance Regression
**Impact**: MEDIUM
**Probability**: LOW

**Risk**: DXVK/OpenAL introduce performance overhead vs native DX8/Miles.

**Mitigation**:
- Profile early and often
- Compare benchmarks (Linux vs Windows)
- Optimize hot paths if needed
- Document acceptable performance delta

**Status**: Monitor during implementation

---

### 12. Deployment Complexity
**Impact**: MEDIUM
**Probability**: MEDIUM

**Risk**: Packaging for Linux (AppImage, Flatpak, etc.) adds distribution complexity.

**Symptoms**:
- Dependency hell
- Library version mismatches
- Installation issues on different distros

**Mitigation**:
- Use AppImage for portable distribution
- Bundle dependencies (DXVK, OpenAL, SDL3)
- Test on multiple distros (Ubuntu, Fedora, Arch)
- Document system requirements clearly

**Status**: Deferred to post-Phase 3

---

## Low Risks

### 13. Mod Compatibility
**Impact**: MEDIUM
**Probability**: LOW

**Risk**: Mods designed for Windows may not work on Linux port.

**Mitigation**:
- Maintain .big file format compatibility
- Test with popular mods
- Document modding guidelines

**Status**: Test after Phase 1 complete

---

### 14. Multiplayer/Network Compatibility
**Impact**: HIGH (if applicable)
**Probability**: UNKNOWN

**Risk**: Network code may have platform-specific assumptions.

**Note**: Multiplayer is OUT OF SCOPE for initial port. Address in future phase.

**Status**: Deferred

---

## Risk Matrix

| Risk | Impact | Probability | Status |
|------|--------|-------------|--------|
| Gameplay Determinism | HIGH | MEDIUM | Mitigated by design |
| Windows Regression | HIGH | MEDIUM | Mitigated by process |
| Zero Hour Gaps | HIGH | MEDIUM | Partially mitigated |
| DXVK/Vulkan Issues | MEDIUM | MEDIUM | Mitigated (proven) |
| Audio Sync | MEDIUM | MEDIUM | Mitigated (proven) |
| Cross-compile | LOW | HIGH | Mitigated (Docker) |
| SDL3 Input | MEDIUM | LOW | Mitigated (proven) |
| Case Sensitivity | LOW | HIGH | Known, mitigatable |

## Overall Risk Level

**MODERATE** - Most critical risks mitigated by:
- Abstraction-only approach (no core changes)
- Proven reference implementations
- Docker build workflow
- Mandatory testing process

## Notes

- Risk assessment will be updated during Phase 0 investigation
- New risks may be identified as we analyze references
- Mitigation strategies will be refined based on findings

## Next Steps

1. Update risk list during Phase 0 investigation
2. Refine mitigation strategies
3. Get approval before Phase 1 starts
4. Revisit after each phase completes
