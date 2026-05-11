# 2026-05 Lessons

## 2026-05-11 - Reverse-diff parity should mirror only low-risk behavioral deltas

- Symptom: Wave 5 reverse scan found multiple Generals-only annotated files, but most differences were expansion-specific or annotation-only.
- Root cause: File-level candidate extraction highlights where to inspect, not what must be mirrored.
- Fix applied: Classified six candidates semantically and mirrored only one low-risk behavioral delta (`GameLOD.cpp` non-Windows fallback now applies defaults only when detection returns unknown/zero).
- Prevention: Keep Wave 5 workflow as `file-level discovery -> semantic classification -> minimal mirror patches` and avoid bulk mirroring in high-divergence gameplay/render files.

## 2026-05-11 - Wave 4 parity must include emitter chunk-path serialization, not only build toggles

- Symptom: Libraries/build parity initially looked complete by annotation scan, but `part_ldr` still diverged in runtime chunk handling (`W3D_CHUNK_EMITTER_LINE_PROPERTIES` and optional `W3D_CHUNK_EMITTER_EXTRA_INFO`).
- Root cause: Annotation-only inventory under-reported unannotated behavioral deltas in serializer/loader paths.
- Fix applied: Ported `part_ldr` extra-info API/state and read/save flow to Generals base, including Save pipeline wiring and chunk-ID correction.
- Prevention: For future wave audits, always follow annotation discovery with targeted file diff for serialization-heavy files (`*_ldr.*`, loaders, save paths) before marking wave complete.

## 2026-05-11 - Volumetric shadow extrusion must be clamped before mesh volume build

- Symptom: Shadow volumes can spike or stretch excessively under shallow light angles during animated updates.
- Root cause: `vectorScaleMax` reached volume construction without an explicit clamp to `MAX_EXTRUSION_LENGTH` in the Generals base path.
- Fix applied: Added pre-construction clamp in `W3DVolumetricShadow::Update()` before calling `constructVolume` / `constructVolumeVB`.
- Prevention: Keep explicit extrusion clamp in any shadow-volume parity ports, especially when touching dynamic/animated caster paths.

## 2026-05-11 - Guard ChallengeGenerals access when porting ZH menu filtering to Generals base

- Symptom: Opening Generals base Singleplayer/Skirmish crashed with `EXC_BAD_ACCESS` in `AsciiString::AsciiString` while populating faction combo entries.
- Root cause: A Wave 2 parity port added `TheChallengeGenerals->getGeneralByTemplateName(...)` to `PopulatePlayerTemplateComboBox`, but `TheChallengeGenerals` may be null in Generals base during menu setup.
- Fix applied: Added a null guard in `Generals/Code/GameEngine/Source/GameNetwork/GUIUtil.cpp` and only queried locked-general state when `TheChallengeGenerals` is available.
- Prevention: For any ZH-to-Generals parity port that references challenge/general subsystems, verify singleton lifetime differences first and guard optional systems in base-game menu paths.

## 2026-05-11 - Base API must be verified before porting name-based fallback logic

- Symptom: A Wave 2 skirmish ownership fallback compiled in Zero Hour but failed in Generals base because `Player` did not expose an ASCII name getter.
- Root cause: The port assumed the same public API existed in both trees, but the base tree only stored `m_playerName` internally.
- Fix applied: Added a minimal `Player::getPlayerName()` accessor and kept the fallback logic in `PlayerList` / `TeamFactory` unchanged.
- Prevention: Before porting any ZH lookup fallback, confirm the base tree exposes the same accessor or add the smallest possible API shim first.

## 2026-05-11 - Header-level CompatLib parity should be established before source/CMake parity in Generals base

- Symptom: Generals base had only one local CompatLib header while Zero Hour maintained a full compatibility include surface, creating an implicit cross-tree dependency and making parity work noisy.
- Root cause: Parity work started from runtime bugs without first normalizing the local compatibility header foundation in `Generals/Code/CompatLib/Include`.
- Fix applied: Synchronized missing CompatLib headers from Zero Hour into Generals base as the first low-risk batch and documented Wave 1 classification/decisions in the parity audit sheet.
- Prevention: For future Generals-vs-ZH parity work, always do include-surface alignment first, then move to source/CMake parity in small validated batches.

## 2026-05-11 - Asset-root fallback must preserve case-insensitive lookup semantics on Linux

- Symptom: On Linux, default/action/attack cursors intermittently fell back to OS pointer; intro/campaign videos also showed inconsistent playback behavior.
- Root cause: Relative file lookups that fail in cwd were retried from the asset root path, but that fallback did not apply case-insensitive traversal. Mixed-case data references (Windows-style) failed against lowercase files on disk.
- Fix applied: In `StdLocalFileSystem::fixFilenameFromWindowsPath`, added Linux-only case-insensitive traversal for asset-root fallback paths before returning not found.
- Prevention: Any new asset-root fallback path in cross-platform filesystem code must mirror the same case-resolution policy used by primary path traversal on Linux.

## 2026-05-09 - OpenAL positional voice playback must not drop multichannel aircraft samples

- Symptom: Aircraft selection/order voice lines on Linux/macOS played only the radio-static portion while the spoken line never started.
- Root cause: Some positional aircraft voice assets can be multichannel; `OpenALAudioCache` rejected those buffers outright for positional playback, so OpenAL advanced past the static intro but silently lost the spoken portion.
- Fix applied: Keep the buffer loadable and make `OpenALAudioManager::playSample3D()` fall back multichannel positional assets to direct non-spatial playback instead of returning silence.
- Prevention: For cross-platform OpenAL, treat unsupported spatialization formats as fallback-to-2D cases first; only hard-fail when the asset cannot be decoded at all.

## 2026-05-07 - In unify merges, resolve modify/delete by validating semantic deltas before deleting legacy paths

- Symptom: Upstream unify moved shared gadget implementation files into `Core/`, while local branch still had modifications in legacy `Generals` paths, producing modify/delete conflicts.
- Root cause: Structural refactor (file relocation) intersected with local functional edits on old paths.
- Fix applied: Compared conflicted legacy files against new `Core` destinations, verified functional deltas, then resolved conflicts by keeping unified `Core` ownership and deleting obsolete `Generals` files.
- Prevention: For future unify-sync cycles, always map `old path -> unified Core path` first and diff semantics before accepting delete/rename conflict resolutions.

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
