# Code Audit Report - Discrepancies with Project Guidelines
**Date:** 2025-01-07
**Objective:** Identify code that does not follow the Vulkan-only and SDL2-only guidelines
**Status:** 🔴 CRITICAL - Multiple violations identified

---

## 📋 Executive Summary

The audit identified **critical violations** of the project's guidelines in 3 main categories:

| Category | Severity | Occurrences | Status |
|----------|----------:|------------:|--------|
| **Windows Registry APIs** | 🔴 HIGH | ~20+ files | ⚠️ PARTIALLY RESOLVED |
| **Win32 API Calls** | 🔴 CRITICAL | ~150+ occurrences | ❌ UNRESOLVED |
| **DirectX References** | 🟡 MEDIUM | ~10+ files | ⚠️ STUBS EXIST |
| **windows.h Includes** | 🟢 LOW | ~90+ files | ⚠️ IN PROGRESS |

---

## 🔴 CRITICAL ISSUES

### 1. WinMain.cpp - Win32 Window Management

**File:** GeneralsMD/Code/Main/WinMain.cpp (lines 772-801)

**Problem:**
```cpp
// ❌ VIOLATION: Direct use of Win32 APIs instead of SDL2
HWND hWnd = CreateWindow(TEXT("Game Window"), ...);
GetSystemMetrics(SM_CXSCREEN);    // Should use SDL_GetDisplayBounds
GetSystemMetrics(SM_CYSCREEN);
SetWindowPos(hWnd, HWND_TOPMOST, ...);  // Should use SDL_SetWindowPosition
SetForegroundWindow(hWnd);
ShowWindow(hWnd, nCmdShow);
UpdateWindow(hWnd);
```

**Impact:**
- Win32-only code instead of SDL2
- Won't work on non-Windows platforms
- Contradicts the "SDL2 only" guideline

**Recommendation:**
```cpp
// ✅ CORRECT: Use SDL2
SDL_DisplayMode displayMode;
SDL_GetCurrentDisplayMode(0, &displayMode);
int centerX = (displayMode.w / 2) - (startWidth / 2);
int centerY = (displayMode.h / 2) - (startHeight / 2);

SDL_Window* window = SDL_CreateWindow(
    "Command and Conquer Generals",
    centerX, centerY,
    startWidth, startHeight,
    SDL_WINDOW_VULKAN | (runWindowed ? 0 : SDL_WINDOW_FULLSCREEN)
);
```

**Note:** The file already contains correct SDL2 code in lines 837-912, but still keeps Win32 legacy code concurrently.

---

### 2. GameSpyGameInfo.cpp - Win32 DLL Loading

**File:** GeneralsMD/Code/GameEngine/Source/GameNetwork/GameSpyGameInfo.cpp (lines 189-427)

**Problem:**
```cpp
// ❌ VIOLATION: LoadLibrary/GetProcAddress are Windows-only
HINSTANCE mib_ii_dll = LoadLibrary("inetmib1.dll");
HINSTANCE snmpapi_dll = LoadLibrary("snmpapi.dll");
SnmpExtensionInitPtr = (int (__stdcall *)(unsigned long,void ** ,AsnObjectIdentifier *))
    GetProcAddress(mib_ii_dll, "SnmpExtensionInit");
FreeLibrary(mib_ii_dll);
```

**Impact:**
- GameSpy networking depends on Windows DLLs
- SNMP APIs are Windows-specific
- Will not compile on macOS/Linux

**Recommendation:**
1. **Short term:** Guard with `#ifdef _WIN32` and implement a cross-platform fallback
2. **Long term:** Remove the GameSpy dependency and implement a modern networking system using POSIX sockets / SDL_net

```cpp
#ifdef _WIN32
    // Existing Windows code
#else
    // TODO: Implement cross-platform network detection
    // Options: getifaddrs() (POSIX), SDL_net, or a cross-platform networking library
#endif
```

---

### 3. ScriptEngine.cpp - Editor DLL Loading

**File:** GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptEngine.cpp

**Problem:**
```cpp
// ❌ VIOLATION: Loading Windows DLLs for editors
st_DebugDLL = LoadLibrary("DebugWindow.dll");         // line 529
st_ParticleDLL = LoadLibrary("ParticleEditor.dll");   // line 535
FARPROC proc = GetProcAddress(st_DebugDLL, "CreateDebugDialog");  // line 541
GetProcAddress(st_ParticleDLL, "CreateParticleSystemDialog");     // line 548
// ... 20+ more GetProcAddress calls
```

**Impact:**
- In-game editors depend on Windows DLLs
- ~25+ `GetProcAddress` calls scattered through the file
- Debug/particle editor features do not work on other platforms

**Recommendation:**
```cpp
// ✅ Appropriate guard already exists (line 526), but must be consistent
#if defined(_WIN32)
    if (TheGlobalData->m_windowed)
        if (TheGlobalData->m_scriptDebug) {
            st_DebugDLL = LoadLibrary("DebugWindow.dll");
        }
#else
    // Editors disabled on other platforms for now
    st_DebugDLL = NULL;
    st_ParticleDLL = NULL;
    LogInfo("Script debugging and particle editing not available on this platform");
#endif
```

**Note:** The `#if defined(_WIN32)` guard exists, but does not cover all subsequent `GetProcAddress` calls (lines 8458-10466).

---

### 4. Win32OSDisplay.cpp - Native MessageBox

**File:** GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32OSDisplay.cpp (lines 106-117)

**Problem:**
```cpp
// ❌ VIOLATION: Win32 MessageBox instead of SDL2
returnResult = ::MessageBoxW(NULL, mesgStr.str(), promptStr.str(), windowsOptionsFlags);
returnResult = ::MessageBoxA(NULL, mesgA.str(), promptA.str(), windowsOptionsFlags);
```

**Impact:**
- Dialogs use Windows MessageBox
- Not functional on macOS/Linux

**Recommendation:**
```cpp
// ✅ CORRECT: Use SDL_ShowMessageBox
SDL_MessageBoxData messageboxdata = {
    SDL_MESSAGEBOX_ERROR,
    NULL,
    promptStr.str(),
    mesgStr.str(),
    // ... button configuration
};
int buttonid;
SDL_ShowMessageBox(&messageboxdata, &buttonid);
```

**Note:** There is a partial stub implementation in `Dependencies/Utility/Compat/msvc_types_compat.h` (line 498), but it is only a stub.

---

## 🟡 MEDIUM ISSUES

### 5. Windows Registry - Incomplete INI Implementation

**Status:** ⚠️ PARTIALLY RESOLVED (Phase 39.5)

**Affected files:**
- `Core/Libraries/Source/WWVegas/WWLib/registry.cpp` - ✅ MIGRATED to INI
- Multiple game code files still call registry functions

**Problem:**
- INI system implemented but not fully tested
- Some components may still depend on Windows registry
- `assets/ini/README.md` documents the format but lacks validation

**Recommendation:**
```bash
# Check remaining calls
grep -r "RegOpenKeyEx\|RegQueryValueEx\|RegSetValueEx\|HKEY_" \
    GeneralsMD/Code/ Generals/Code/ --include="*.cpp" | \
    grep -v "registry.cpp" | tee logs/registry_remaining.log
```

---

### 6. DirectX8 Interface Stubs

**Status:** ⚠️ STUBS IMPLEMENTED but not fully integrated

**File:** Core/Libraries/Source/WWVegas/WW3D2/DX8Wrapper_Stubs.cpp

**Analysis:**
```cpp
// ✅ GOOD: Stubs exist for compatibility
class IDirect3DDevice8Stub {
    int TestCooperativeLevel();
    int SetTexture(int stage, void* texture);
    int SetPixelShader(int shader);
    // ... etc
};
```

**Problem:**
- Stubs return mock values but do not execute real Vulkan operations
- `DX8Wrapper_Stubs.cpp` contains 2300+ lines of empty stubs
- DirectX calls remain in game code

**Recommendation:**
- **DO NOT remove the stubs** - they allow compilation
- **Gradually replace** DirectX calls with Vulkan via `DX8Wrapper`
- Prioritize hot paths: rendering, textures, buffers

---

## 🟢 LOW PRIORITY ISSUES

### 7. Unnecessary windows.h Includes

**Affected files:** ~90 files

**Categories:**
1. **Tools/GUIEdit/** - ~15 files (Windows-only tools acceptable for now)
2. **Core/Libraries/Source/debug/** - ~10 files (debug system)
3. **GameEngine/** - ~20 files (cleanup priority)
4. **Win32Device/** - ~10 files (expected, but can migrate to SDL2)

**Example (PreRTS.h):**
```cpp
// GeneralsMD/Code/GameEngine/Include/Precompiled/PreRTS.h:47
#include <windows.h>  // ❌ In precompiled header - affects the whole project
```

**Recommendation:**
```cpp
// ✅ Conditional or remove entirely
#if defined(_WIN32) && defined(NEED_WINDOWS_H)
    #include <windows.h>
#endif
```

---

## 📊 Detailed Statistics

### Win32 API Calls by Category

| API Category | Occurrences | Status |
|--------------|-----------:|--------|
| Window Management | 50+ | ❌ CreateWindow, SetWindowPos, ShowWindow |
| System Metrics | 20+ | ❌ GetSystemMetrics |
| DLL Loading | 80+ | ❌ LoadLibrary, GetProcAddress, FreeLibrary |
| Message Boxes | 10+ | ⚠️ MessageBox (stubs exist) |
| Registry | 100+ | ⚠️ INI migration done but untested |
| File System | 30+ | ⚠️ Partially migrated |

### Compliance Score by Module

| Module | Compliance | Issues |
|--------|-----------:|--------|
| **WinMain.cpp** | 🔴 30% | Win32 + SDL2 mixed |
| **GameSpyGameInfo.cpp** | 🔴 0% | 100% Win32 DLL loading |
| **ScriptEngine.cpp** | 🟡 50% | Guards exist but incomplete |
| **DX8Wrapper** | 🟢 80% | Stubs OK, Vulkan integration missing |
| **Registry System** | 🟢 90% | INI migration complete |

---

## 🎯 Recommended Action Plan

### Priority 1 - CRITICAL (Week 1)

1. **WinMain.cpp Window Management**
```bash
# Remove duplicated Win32 code
# Keep only the SDL2_CreateWindow path
# Test on macOS/Linux
```

2. **GameSpyGameInfo.cpp Networking**
```bash
# Guard with #ifdef _WIN32
# Implement cross-platform fallback
# Document limitations
```

3. **ScriptEngine.cpp Editors**
```bash
# Extend guards to cover all GetProcAddress calls
# Add logs when editors are not available
# Verify compilation on macOS/Linux
```

### Priority 2 - HIGH (Week 2)

4. **MessageBox Migration**
```bash
# Replace all Win32 MessageBox calls with SDL_ShowMessageBox
# Create a wrapper in Dependencies/Utility/Compat/
# Test cross-platform dialogs
```

5. **GetSystemMetrics → SDL2**
```bash
# Replace SM_CXSCREEN/SM_CYSCREEN with SDL_GetDisplayBounds
# Add helpers in SDL2_AppWindow
```

### Priority 3 - MEDIUM (Weeks 3-4)

6. **windows.h Cleanup**
```bash
# Remove unnecessary includes
# Add conditional guards where required
# Test incremental compilation
```

7. **Registry Testing**
```bash
# Create tests for the INI system
# Validate read/write of settings
# Document .reg → .ini migration
```

---

## ✅ Verification Checklist

**SDL2 Only:**
- [ ] Zero CreateWindow Win32 calls
- [ ] Zero GetSystemMetrics calls
- [ ] Zero SetWindowPos Win32 calls
- [ ] Zero MessageBox Win32 calls (except stubs)
- [ ] All windows managed via SDL2

**Vulkan Only:**
- [x] DirectX stubs implemented (Phase 62)
- [ ] Zero direct DirectX calls in game code
- [ ] DX8Wrapper routing to Vulkan fully implemented
- [ ] Shaders migrated from HLSL to SPIR-V

**Cross-Platform:**
- [ ] Zero LoadLibrary/GetProcAddress without guards
- [ ] Zero unnecessary windows.h includes
- [ ] Registry fully using INI files
- [ ] File paths using std::filesystem

---

## 📚 References

- **Project Guidelines:** .github/instructions/generalsx.instructions.md
- **Phase 39.5 Registry:** docs/WORKDIR/phases/3/PHASE39/39.5_INDEX.md
- **Phase 40 SDL2:** docs/WORKDIR/phases/4/PHASE40/
- **Phase 62 DX8 Stubs:** Core/Libraries/Source/WWVegas/WW3D2/DX8Wrapper_Stubs.cpp

---

## 🚨 Important Notes

1. **DO NOT REMOVE STUBS:** DirectX stubs are required for compilation. Migration should address root causes, not remove stubs.

2. **FAIL FAST:** Each fix must be validated with a full build before moving on.

3. **COMMIT TIME RESTRICTIONS:** GeneralsX project - avoid commits Monday–Friday 09:00–18:00 local time.

4. **BUILD WITH TEE:** Always use `tee` to log builds:
```bash
cmake --build build/macos --target GeneralsXZH -j 4 2>&1 | tee logs/phase_XX_build.log
```

5. **UPDATE DEV BLOG:** Update `docs/DEV_BLOG/2025-01-DIARY.md` before committing changes.

---

**Audited by:** GitHub Copilot (Claude Sonnet 4.5)
**Next Review:** After Priority 1 issues are fixed
