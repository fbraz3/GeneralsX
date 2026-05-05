# 2026-05 Lessons

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
