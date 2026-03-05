# 2026-03 Lessons Learned — Phase 6 Windows Modern Stack (win64-modern)

**Month**: March 2026
**Sessions Covered**: 57
**Focus Area**: Phase 6 win64-modern audit — compat layer correctness, CMake guards, build regressions

---

## Lesson 1 (Session 57): Header Guard Duplication Pattern — Partial #ifndef Wrapping Bug

**Context**: Phase 6 (win64-modern) patched `gdi_compat.h` to add an `#ifndef _WIN32` /
`#endif // !_WIN32` wrapper around GDI stubs that are Linux-only. The patch correctly
inserted the opening guard at the top and the closing `#endif` in the middle, but the
**original un-guarded content below the `#endif` was never deleted**. Result: the full
block of types and functions was **duplicated** — once inside the guard (correct), and
once outside of any guard (broken).

**Bug Signature**:
```
[File] = #ifndef _WIN32
            ... full block ...
         #endif // !_WIN32
                                <- should end here
            ... exact same block again, no guards ...
        (EOF)
```

**Impact on Linux builds**: `static inline` functions defined twice across TUs → latent
ODR violations. Typedef duplication (`HFONT`, `HDC`, `TEXTMETRIC`) → hard compile errors
in any TU that includes the header twice via different paths.

**Impact on Windows builds**: None immediately (the shim `windows.h` is a no-op on Windows
so `gdi_compat.h` was not reached). But the file's state was misleading and dangerous.

**Root Cause**: The Phase 6 diff added the `#ifndef _WIN32` / `#endif` wrapper but the
author did not delete the old identical block that already existed below it.

**Fix**: Truncate the file to the line containing `#endif // !_WIN32` (line 225). Delete
everything after it. The first block is the canonical version; the second is the orphan.

**Prevention Checklist**:
- When wrapping existing code with `#ifndef _WIN32` guards, always **delete the original
  content** that was left outside the guard.
- After applying a wrapper patch, `grep -c 'typedef.*HFONT' gdi_compat.h` must return 1.
- Do a before/after `wc -l` sanity check: if the guarded file is longer than the original,
  something is wrong.

---

## Lesson 2 (Session 57): PCH Must Not Be Silently Removed

**Context**: The Phase 6 diff removed the `target_precompile_headers(z_gameengine …)` call
entirely without a comment or replacement. The PCH compiled `PreRTS.h` + `CppMacros.h`
once per library, saving many seconds per build. Without it, every `.cpp` in `z_gameengine`
compiles `PreRTS.h` independently.

**Impact**:
- **Compile time regression**: 2–4× slower full rebuilds for `z_gameengine` (≈600 source files).
- **No correctness breakage**: functionally identical, just slower.
- **Hidden cost**: regressions of this type are not visible in error logs — only in wall time.

**Why It Was Removed** (inferred): The Phase 6 change
`target_include_directories(z_gameengine PUBLIC ../CompatLib/Include)` was correctly moved
inside `if(UNIX)`. The PCH was removed because `PreRTS.h` includes `windows_compat.h` on
Linux and the real SDK `<windows.h>` on Windows; it was feared the PCH would behave
differently per platform. **That fear was unfounded** — `PreRTS.h` already has correct
`#ifdef _WIN32` guards for all platform-specific includes.

**Fix**: Restore `target_precompile_headers(z_gameengine PRIVATE …)` unchanged.

**Rule**: **Never remove PCH without a documented reason**. If the PCH causes a build break on
a new platform, it means the headers it includes have a bug — fix the headers, not the PCH.

---

## Lesson 3 (Session 57): CMake Options Are NOT Preprocessor Definitions

**Context**: `windows_compat.h` used the guard:
```cpp
#if !defined(_WIN32) || (defined(_WIN32) && !defined(SAGE_USE_DX8))
```
to conditionally define `_MEMORYSTATUS_DEFINED` / `_IUNKNOWN_DEFINED`, intending:
- Linux → always apply guards
- Windows Modern (win64-modern, `SAGE_USE_DX8=OFF`) → apply guards
- Windows Legacy (vc6/win32, `SAGE_USE_DX8=ON`) → do NOT apply

**The Bug**: `SAGE_USE_DX8` is a **CMake cache variable** — it is never translated to
`-DSAGE_USE_DX8` in the compiler invocation. At C++ preprocessor time, `SAGE_USE_DX8`
is always undefined. So `!defined(SAGE_USE_DX8)` is always `true`, and the full
condition simplifies to `!_WIN32 OR _WIN32` = unconditionally `true`. The intended
Windows Legacy exclusion never happened.

**Secondary observation**: `windows_compat.h` is included only inside `#ifndef _WIN32`
inside the shim `windows.h`. On every Windows build (all presets) the shim is a no-op,
so `windows_compat.h` is **never included on Windows at all**. The `_WIN32` branches
of any logic inside `windows_compat.h` are structurally unreachable.

**Fix**: Remove the `#if` condition entirely. Define the guards unconditionally in
`windows_compat.h` with a clear comment explaining that this file is Linux-only:
```cpp
#define _MEMORYSTATUS_DEFINED   // Tell DXVK: skip incomplete MEMORYSTATUS
#define _IUNKNOWN_DEFINED       // Tell DXVK: skip incomplete IUnknown
```

**Golden Rule**: **CMake `option()` variables are NOT `-D` compile definitions.**
To make a CMake flag visible to C++ code, explicitly call:
```cmake
if(SAGE_USE_DX8)
    target_compile_definitions(my_target PRIVATE SAGE_USE_DX8)
endif()
```
Without this, `#ifdef SAGE_USE_DX8` is always false.

---

## Lesson 4 (Session 57): Audit Both Directions When Doing `#ifndef _WIN32` Cleanup

**Context**: The Phase 6 work correctly made many compat headers Linux-only by adding
`#ifndef _WIN32` … `#endif` wrappers. This is the right architectural move — the shim
layer exists for Linux only; Windows uses the real SDK.

**However**: each wrapper edit must be audited to ensure:

1. **Nothing legitimately cross-platform was accidentally gated inside the guard.**
   - Constants like `DEFAULT_CHARSET`, `ANSI_CHARSET`, `ETO_OPAQUE` were already consumed by
     game code on Windows too (the real `wingdi.h` defines them, so no problem — but verify).
2. **The file has no duplicated content after the `#endif`** (Lesson 1 above).
3. **The wrapper doesn't create a false sense that Nothing changed for Linux.**
   - A file that compiles fine on Linux after wrapping might still have latent issues if
     the wrapped types are different from what DXVK's `windows_base.h` provides.

**Recommended post-patch audit command**:
```bash
# Count typedef/define occurrences to detect duplication
for sym in HFONT HDC HBITMAP TEXTMETRIC DEFAULT_CHARSET ETO_OPAQUE; do
    count=$(grep -c "^\(typedef\|#define\).*$sym" file.h)
    echo "$sym: $count"  # expect 1 for each
done
```

---

**Written**: 2026-03-01, Session 57
**Based on**: Phase 6 win64-modern code audit

---

## Lesson 5 (Session 58): INTERFACE Library Sources Must Use Absolute Paths in Generator Expressions

**Context**: `Core/WW3D2/CMakeLists.txt` creates `corei_ww3d2` as an INTERFACE library.
When enabling `FramGrab.cpp` for Windows-only builds, a generator expression was used:
```cmake
$<$<BOOL:${WIN32}>:FramGrab.cpp>   # WRONG — relative path
```
Because `corei_ww3d2` is INTERFACE, its `INTERFACE_SOURCES` propagate to all consumers.
CMake 4.x cannot convert generator-expression paths to absolute; relative paths are
evaluated at the consumer's source directory. `GeneralsMD/WW3D2` doesn't have `FramGrab.cpp`.

**Fix**: Use `${CMAKE_CURRENT_SOURCE_DIR}` inside the generator expression:
```cmake
$<$<BOOL:${WIN32}>:${CMAKE_CURRENT_SOURCE_DIR}/FramGrab.cpp>   # CORRECT
```

**Rule**: **All file paths inside generator expressions in INTERFACE `target_sources` calls
MUST be absolute** (`${CMAKE_CURRENT_SOURCE_DIR}/file.cpp`). Relative paths in INTERFACE
sources cause "Cannot find source file" errors in consumer CMake/build directories.

---

## Lesson 6 (Session 58): vcpkg Rejects Empty STATIC Libraries; INTERFACE Chains Produce Phantom .lib

**Context**: `z_wwdownload STATIC` had no direct sources — it only linked `corei_wwdownload
INTERFACE` (which lists sources via `INTERFACE_SOURCES`). On Windows (vcpkg toolchain),
`_add_library` wrapper in `vcpkg.cmake` requires STATIC targets to have at least one source.
First workaround attempt: convert to `INTERFACE`. This caused `corei_wwdownload.lib` to
appear in the final link command (CMake serializes the INTERFACE chain as library names).

**Two-part rule**:
1. Never create an empty `add_library(X STATIC)` — vcpkg on Windows rejects it.
2. Never use INTERFACE → INTERFACE chain for targets that will be linked into executables
   via transitive dependencies — CMake generates phantom `.lib` names in the link command.

**Fix**: List the actual source files explicitly in `target_sources(z_wwdownload PRIVATE ...)`.
Use `${CMAKE_SOURCE_DIR}` (workspace root, not `${PROJECT_SOURCE_DIR}` which may point to
a subproject root) for cross-directory source paths.

---

## Lesson 7 (Session 58): PROJECT_SOURCE_DIR Is the Nearest project() Call, Not the Workspace Root

**Context**: When fixing `z_wwdownload CMakeLists.txt` in `GeneralsMD/...`, tried to reference
`${PROJECT_SOURCE_DIR}/Core/...`. CMake expanded this to `GeneralsX/GeneralsMD/Core/...`
(wrong — path doesn't exist) instead of `GeneralsX/Core/...`.

**Explanation**: `PROJECT_SOURCE_DIR` is set to the directory containing the most recently
executed `project()` call in cmake scope. `GeneralsMD/CMakeLists.txt` defines
`project(GeneralsXZH ...)`, so `PROJECT_SOURCE_DIR = <workspace>/GeneralsMD`.

**Rule**: Use `CMAKE_SOURCE_DIR` (always the top-level CMakeLists.txt directory) when
constructing paths that must point to the workspace root. Use `PROJECT_SOURCE_DIR` only
when you need the subproject's own root.

---

## Lesson 8 (Session 58): FramGrab.cpp VFW Guard Pattern — Stub Body for Out-of-Scope Features

**Context**: `FramGrab.cpp` implements AVI video recording via VFW (Video for Windows).
VFW header `vfw.h` has known issues in Windows SDK 10.0.26100.0 and VFW is classified
"Phase 3 out-of-scope" for Phase 1 (graphics port). The linker still needs FramGrab symbols
because `ww3d.cpp` instantiates `FrameGrabClass`.

**Pattern used**: Guard the full VFW implementation with:
```cpp
#if defined(ENABLE_FRAME_GRABBER) && !defined(DISABLE_VFW)
    ... full AVIFileInit/AVIStreamWrite implementation ...
#else
    // Stub: symbols exist for linker; AVI recording is disabled
    FrameGrabClass::FrameGrabClass(...) { /* zero-init members */ }
    FrameGrabClass::~FrameGrabClass() {}
    void FrameGrabClass::CleanupAVI() {}
    void FrameGrabClass::GrabAVI(void*) {}
#endif
```
Non-VFW methods (`Grab`, `ConvertFrame`, `ConvertGrab`, `GrabRawFrame`) compile unconditionally.

**Rule**: When a feature is "out of scope" for the current phase but its symbols are needed
by the linker, provide minimal stub implementations guarded by a feature flag instead of
commenting out the file. This avoids linker errors while making it trivial to re-enable
the feature later.

---

## Lesson 9 (Session 58): SDL3 Is Cross-Platform — Remove #ifndef _WIN32 Guards from SDL3 Files

**Context**: `SDL3Mouse.cpp` and `SDL3Keyboard.cpp` had `#ifndef _WIN32` wrapping the
entire file body. SDL3 is explicitly cross-platform (Windows + Linux + macOS). These guards
were left from an earlier Linux-only porting attempt and incorrectly excluded all SDL3
Mouse/Keyboard code from Windows builds, causing LNK2019 for all `SDL3Mouse::*` and
`SDL3Keyboard::*` symbols.

**Fix**: Remove the outer `#ifndef _WIN32` / `#endif // !_WIN32` guards. Inner guards for
Linux-only dependencies (e.g., `SDL3_image`) remain correct.

**Rule**: SDL3 is the unified platform layer. SDL3 abstractions (window, input, events) are
available on all target platforms. Never wrap SDL3 code in `#ifndef _WIN32`.

---

## Lesson 10 (Session 58): SDL_main.h Required for WinMain Entry Point on Windows

**Context**: `SDL3Main.cpp` defines `int main(int argc, char* argv[])`. The win64-modern
build uses `/subsystem:windows` which requires `WinMain` (not `main`). SDL3 provides a
`WinMain` implementation compiled into `SDL3.dll` (via `SDL_sysmain_runapp.c.obj`) that
calls `SDL_main()`. For this to work, `SDL3/SDL_main.h` must be included BEFORE `main()`
is defined — it `#define main SDL_main`, redirecting our `main()` → `SDL_main()`, and
SDL3.dll's `WinMain` calls `SDL_main()`.

**Fix**:
```cpp
#ifdef _WIN32
#include <SDL3/SDL_main.h>  // #define main SDL_main; SDL3.dll provides WinMain
#endif
#include <SDL3/SDL.h>
```

**Rule**: Any SDL3 application on Windows with `/subsystem:windows` MUST include
`SDL3/SDL_main.h` before defining `main()`. Without it: LNK2019 `WinMain unresolved external`.

---

## Lesson 11 (Session 58): WWDownload POSIX Compat — MSVC Missing Functions

**Context**: `FTP.cpp` and `Download.cpp` (WWDownload module) use several POSIX functions
not available in MSVC CRT:

| Symbol | Available on Linux | Available on MSVC CRT | Fix |
|---|---|---|---|
| `strlcpy` | Yes | **No** | Inline polyfill in `wwdownload_compat.h` |
| `strlcat` | Yes | **No** | Inline polyfill in `wwdownload_compat.h` |
| `socklen_t` | Yes (`sys/socket.h`) | Only in `ws2tcpip.h` | Switch to winsock2 + include `ws2tcpip.h` |
| `ARRAY_SIZE` | Defined in wwlib | Not propagated to wwdownload | `#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))` in compat |
| `_S_IWRITE/_S_IREAD` | Defined as POSIX alias | In `<sys/stat.h>` | Add `#include <sys/stat.h>` in Windows block |
| `timeGetTime` | Unavailable | In `<timeapi.h>` | `#include <timeapi.h>` explicitly (not just `mmsystem.h`) |
| `time()` | `<time.h>` | `<time.h>` missing from Windows block | Add `#include <time.h>` |

**Key insight for `timeGetTime`**: With `WIN32_LEAN_AND_MEAN` and `winsock2.h` already
included (pulling in most of `windows.h`), `mmsystem.h` may not be reached in some
configurations. `<timeapi.h>` is the canonical direct header for `timeGetTime` in modern
Windows SDK and should be included explicitly.

**Created**: `Core/Libraries/Source/WWVegas/WWDownload/wwdownload_compat.h` — centralized
Windows compat shims for the WWDownload module (strlcpy, strlcat, ARRAY_SIZE, ws2tcpip.h).

---

## Lesson 12 (Session 58): winsock.h → winsock2.h Migration; Include Order Matters

**Context**: `ftp.h` included `<winsock.h>` (WinSock v1). The project features list shows
`UseWinsock2` enabled, linking `ws2_32.lib`. winsock.h v1 doesn't define `socklen_t` and
has older APIs. Using winsock.h with a project that links ws2_32.lib can cause subtle issues.

**Fix**: Changed `ftp.h` to `#include <winsock2.h>`.

**Critical ordering rule**: `winsock2.h` MUST be included BEFORE `windows.h`. When `winsock2.h`
is included first, it defines `_WINSOCKAPI_` which prevents `winsock.h` from being included
by `windows.h` automatically. If `windows.h` comes first (without winsock2 applied), it may
include `winsock.h` (v1), and then including `winsock2.h` afterwards causes redefinition errors.

**Since `ftp.h` is included before the platform block** in both `FTP.cpp` and `Download.cpp`,
winsock2.h is always the first socket header — the ordering is correct.

---

**Written**: 2026-03-02, Session 58
**Based on**: win64-modern LNK2019 fix session — `generalszh.exe` links successfully (10.4 MB)

---

## Lesson N+1 (Session 77): Never call find_package after FetchContent has created the same target

**Context**: `windows-sdl` branch modified `GeneralsMD/Code/CompatLib/CMakeLists.txt` to load GLM via
FetchContent (`cmake/glm.cmake`) and then immediately call `find_package(glm CONFIG QUIET)`. On macOS
with Homebrew GLM installed at `/usr/local/share/glm/`, vcpkg's wrapper tried to create `glm::glm` AND
`glm::glm-header-only` — but `glm::glm` already existed from FetchContent.

**Rule**: Once `cmake/glm.cmake` creates a target via FetchContent, do NOT call `find_package` for the
same library in downstream CMakeLists. The FetchContent target is authoritative.

**Written**: 2026-03-04, Session 77

---

## Lesson N+2 (Session 77): Never rely on Homebrew path detection for arch-specific libs on Apple Silicon

**Context**: `cmake/openal.cmake` detected openal-soft at `/usr/local/opt/openal-soft` (Intel Homebrew
at x86_64). The library was x86_64-only and failed to link against native arm64 binary:
`ld: symbol(s) not found for architecture arm64`.

**Fix**: Use FetchContent for ALL platforms (Linux, macOS, Windows). FetchContent compiles openal-soft
for the target architecture — always correct on arm64, never victim to arch mismatch from a mismatched
Homebrew installation.

**Written**: 2026-03-04, Session 77
**Based on**: macOS arm64 linker failure on windows-sdl branch — binary confirmed arm64 after fix.
