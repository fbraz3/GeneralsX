# 2026-05 Lessons

## 2026-05-04 - Guard texture creation at all abstraction layers in headless mode

- Symptom: Replay progressed but crashed at `D3DXCreateTexture +0` during uncompressed texture loading.
- Root cause: Texture allocation path assumed DX8 device availability; in headless transition windows the call chain could reach D3DX8 wrappers with null device/context.
- Fix applied: Added layered guards in `D3DXCreateTexture`, `DX8Wrapper::_Create_DX8_Texture`, and texture loader begin paths so allocation failures return cleanly.
- Prevention: For replay/headless stability, enforce defensive checks at API boundary, wrapper boundary, and task boundary instead of relying on a single guard.

## 2026-05-03 - Route through existing WWMath APIs before adding deterministic-only wrappers

- Symptom: Initial Trig routing to `WWMath::SinTrig`/`CosTrig`/`SqrtOrigin` failed in local branch because those symbols are not present in baseline `wwmath.h`.
- Root cause: Upstream PR assumes a wider deterministic wrapper layer that has not been ported yet.
- Fix applied: Add a transitional wrapper block in `wwmath.h` and keep routing changes small (`trig.h`, `Trig.cpp`, `BaseType.h`) so the build remains green while enabling next deterministic steps.
- Prevention: Before mechanical replacements, verify symbol availability in local WWMath and introduce compatibility wrappers first.

## 2026-05-03 - macOS configure fails when VCPKG_ROOT is unset

- Symptom: CMake fails during preset configure with "Could not find toolchain file: /scripts/buildsystems/vcpkg.cmake".
- Root cause: `default-vcpkg` preset uses `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`; when `VCPKG_ROOT` is empty, CMake resolves to an invalid absolute path.
- Fix applied: Add `resolve_vcpkg_root()` in macOS build scripts to validate environment value, auto-detect common locations (including Homebrew), and fail with actionable guidance.
- Prevention: Keep environment checks before `cmake --preset` in user-facing platform scripts.

## 2026-05-04 - Headless dummies must override hot-path virtuals

- Symptom: macOS headless replay crashed inside `ParticleSystemManager::update()` even after routing factory creation to `ParticleSystemManagerDummy`.
- Root cause: The dummy type did not override `update()`, so virtual dispatch still executed the full particle update path.
- Fix applied: Override `ParticleSystemManagerDummy::update()` as a no-op in `ParticleSys.h`.
- Prevention: When introducing dummy/headless subsystem implementations, audit and override all high-frequency virtual methods used by headless loops.
