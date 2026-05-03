# 2026-05 Lessons

## 2026-05-03 - macOS configure fails when VCPKG_ROOT is unset

- Symptom: CMake fails during preset configure with "Could not find toolchain file: /scripts/buildsystems/vcpkg.cmake".
- Root cause: `default-vcpkg` preset uses `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`; when `VCPKG_ROOT` is empty, CMake resolves to an invalid absolute path.
- Fix applied: Add `resolve_vcpkg_root()` in macOS build scripts to validate environment value, auto-detect common locations (including Homebrew), and fail with actionable guidance.
- Prevention: Keep environment checks before `cmake --preset` in user-facing platform scripts.
