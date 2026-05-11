# Generals Parity Wave 4 Audit: Libraries and Build Integration

Date: 2026-05-11
Scope: `GeneralsMD/Code/Libraries/Source/**` and related build-glue deltas missing in `Generals/Code/Libraries/Source/**`.
Status: Completed (Batch 1 completed)

## Classification Legend

- Applicability:
  - `Direct`: Can be ported with minimal/no semantic adaptation.
  - `Adapted`: Ported with Generals base-specific target/path adjustments.
  - `N/A`: Not portable as-is (missing file, game-tree divergence, or annotation-only delta).
- Risk:
  - `Low`, `Medium`, `High`

## ZH -> Generals Missing Candidate Inventory

| File | Subsystem | Applicability | Risk | Decision | Notes |
|---|---|---|---|---|---|
| Code/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt | Build glue | Adapted | Low | Ported (Batch 1) | Disabled PCH for non-Windows path and added explicit `d3d8lib` link in `g_ww3d2` for stable cross-platform include/link behavior. |
| Code/Libraries/Source/WWVegas/WW3D2/assetmgr.cpp | Runtime loader glue | Adapted | Low | Ported (Batch 1) | Added `shdlib.h` include, `SHD_REG_LOADER`, and 64-bit-safe `lstrcpyn` pointer-delta calculation using `SIZE_T`. |
| Code/Libraries/Source/WWVegas/WW3D2/ww3d.cpp | Runtime render flush glue | Adapted | Low | Ported (Batch 1) | Added `win.h` + `shdlib.h` include parity and `SHD_FLUSH` call in `WW3D::Flush`. |
| Code/Libraries/Source/WWVegas/WW3D2/part_ldr.h | Emitter serialization API | Adapted | Medium | Ported (Batch 1) | Added emitter extra-info accessors, virtual read/save API, and `m_ExtraInfo` state. |
| Code/Libraries/Source/WWVegas/WW3D2/part_ldr.cpp | Emitter serialization behavior | Adapted | Medium | Ported (Batch 1) | Added extra-info load/save handling, fixed line-properties chunk ID check, persisted line-properties and extra-info in `Save_W3D`, and aligned non-Windows `lstrlen` macro usage. |
| Code/Libraries/Source/WWVegas/WW3D2/dx8fvf.h | Header annotation parity | N/A | Low | Deferred (intentional) | Delta is annotation/comment-only; include behavior is already equivalent. |
| Code/Libraries/Source/WWVegas/CMakeLists.txt | Build glue annotation parity | N/A | Low | Deferred (intentional) | Delta is annotation/date-only and target-name divergence (`g_*` vs `z_*`) without functional impact. |
| Code/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt (`linegrp.*`, `dx8rendererdebugger.*`) | Source list parity | N/A | Medium | Deferred (intentional) | Files are absent in Generals tree; cannot mirror source-list entries directly without introducing new modules. |

## Batch 1 Progress (This Session)

- Ported Wave 4 parity deltas in 5 files:
  - `Generals/Code/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`
  - `Generals/Code/Libraries/Source/WWVegas/WW3D2/assetmgr.cpp`
  - `Generals/Code/Libraries/Source/WWVegas/WW3D2/ww3d.cpp`
  - `Generals/Code/Libraries/Source/WWVegas/WW3D2/part_ldr.h`
  - `Generals/Code/Libraries/Source/WWVegas/WW3D2/part_ldr.cpp`
- Kept scope restricted to backend/build/serialization parity (no gameplay logic modifications).

## Validation Evidence

1. Static diagnostics (`get_errors`) on all edited files: no errors.
2. Platform build executed: `./scripts/build/macos/build-macos-generals.sh` -> `Build complete`.
3. Platform deploy executed: `[Platform] Deploy GeneralsX` task -> `Deploy complete`.

## Remaining Work for Wave 4

1. Wave 4 Libraries/build parity is complete for current Generals base scope.
2. Unported deltas are intentional and classified as `N/A` or annotation-only.

## Notes

- This audit intentionally excludes automatic game execution.
- Runtime smoke and replay validation remain user-driven after this batch.
