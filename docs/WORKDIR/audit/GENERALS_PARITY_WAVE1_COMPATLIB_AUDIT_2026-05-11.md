# Generals Parity Wave 1 Audit: CompatLib

Date: 2026-05-11
Scope: `GeneralsMD/Code/CompatLib` deltas that do not currently appear in `Generals/Code/CompatLib` annotation set.
Status: In Progress

## Classification Legend

- Applicability:
  - `Direct`: Can be ported with minimal/no semantic adaptation.
  - `Adapted`: Should be ported with adjustments for Generals base specifics.
  - `N/A`: Zero Hour-only or irrelevant for Generals base.
- Risk:
  - `Low`, `Medium`, `High`

## Candidate Delta Inventory

| File | Category | Applicability | Risk | Decision | Notes |
|---|---|---|---|---|---|
| Code/CompatLib/CMakeLists.txt | Build glue | Adapted | Medium | Pending (Batch 2) | Requires Generals-specific target naming/order to avoid collisions with existing global `d3d8lib`/Compat targets coming from current tree topology. |
| Code/CompatLib/Include/com_compat.h | Platform API compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/comip.h | Platform API compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/comip_compat.h | Platform API compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/comutil.h | Platform API compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/comutil_compat.h | Platform API compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/file_compat.h | Filesystem portability | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/gdi_compat.h | GDI compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/mbstring_compat.h | CRT compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/memory_compat.h | Runtime compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/socket_compat.h | Networking portability | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/string_compat.h | CRT compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/thread_compat.h | Threading compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/time_compat.h | Timing compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/types_compat.h | Type compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/vfw_compat.h | Video-for-Windows compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/windows.h | Windows shim wrapper | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/windows_compat.h | Windows shim definitions | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/windows_wrapper.h | Include bridge | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Include/wnd_compat.h | Window API compatibility | Direct | Low | Ported (Batch 1) | Header synced from Zero Hour to Generals base. |
| Code/CompatLib/Source/d3dx8_compat.cpp | D3DX compatibility | Adapted | Medium | Pending (Batch 2) | Needs coordinated integration with Build glue and d3dx8 target ownership to avoid duplicate definitions/order issues. |
| Code/CompatLib/Source/module_compat.cpp | Module loader compatibility | Adapted | Medium | Pending (Batch 2) | Depends on `module_compat.h` declarations now present; implementation enablement requires CompatLib source target in Generals. |
| Code/CompatLib/Source/string_compat.cpp | CRT compatibility | Adapted | Medium | Pending (Batch 2) | Implementation should be enabled together with other Source files to keep weak-symbol behavior consistent. |
| Code/CompatLib/Source/thread_compat.cpp | Threading compatibility | Adapted | Medium | Pending (Batch 2) | Must be introduced with matching threading target wiring to satisfy declarations in `thread_compat.h`. |
| Code/CompatLib/Source/time_compat.cpp | Timing compatibility | Adapted | Medium | Pending (Batch 2) | Introduce with source-target wiring to satisfy timing declarations and avoid mixed inline/extern behavior drift. |
| Code/CompatLib/Source/wnd_compat.cpp | Window API compatibility | Adapted | Medium | Pending (Batch 2) | Requires synchronized inclusion with Source target wiring and SDL3 guard expectations. |

## Step 1 + Step 2 Delta Update

- Step 1 complete for Wave 1 header surface: applicability and risk classified for all include-layer candidates.
- Step 2 first low-risk batch complete: synced missing `Code/CompatLib/Include/*.h` files from Zero Hour to Generals base.
- Structural result: include-level parity gap for `Code/CompatLib/Include` is now zero.

## Batch 2 Progress (This Session)

- Updated global UNIX include ownership from `GeneralsMD/Code/CompatLib/Include` to `Generals/Code/CompatLib/Include` in root CMake.
- Added `CompatLib/Include` to `gi_always` include interface in Generals code CMake.
- Validation completed with Build + Deploy on macOS platform tasks (no run).
- Next Batch 2 focus remains: Source/CMake target wiring for local Generals CompatLib source implementation.

## Wave 1 Decision Order

1. Build and include-chain safety first
   - `windows_compat.h`, `windows_wrapper.h`, `windows.h`, `types_compat.h`, `com_compat.h`
2. Runtime portability and filesystem
   - `file_compat.h`, `string_compat.h`, `time_compat.h`, `thread_compat.h`, `socket_compat.h`
3. Source implementations
   - `module_compat.cpp`, `thread_compat.cpp`, `time_compat.cpp`, `string_compat.cpp`, `wnd_compat.cpp`
4. Build glue finalization
   - `Code/CompatLib/CMakeLists.txt`

## Validation Gates For Each Batch

1. Configure/build on active platform preset.
2. Launch to main menu.
3. Quick skirmish load.
4. Replay smoke path (when impacted by timing/thread/path changes).

## Notes

- This file is the execution worksheet for Wave 1.
- Every row must end with an explicit decision (`Port`, `Port with adaptation`, `Do not port`) and a short rationale.
