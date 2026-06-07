# TheSuperHackers Upstream Sync — Conflict Resolution Plan

**Date:** 2026-06-05
**Branch:** `thesuperhackers-sync-06-05-2026`
**Sync base:** `ed7e96f8f` (unify(shroud) — common ancestor of `main` and `thesuperhackers/main`)
**Upstream tip:** `f334383ec` (bugfix(contain))
**Commits imported:** 6 (ahead of our `main`)

## 1. Scope of the Sync

Upstream commits pulled in this sync:

| SHA | Type | Title | Touched areas |
|---|---|---|---|
| `f334383ec` | bugfix | Prevent riders added to destroyed container (#2746) | `Generals{,MD}/.../Object/Contain/OpenContain.cpp`, `TransportContain.cpp` |
| `7c78a5e17` | refactor | Split `MetaEventTranslator::translateGameMessage()` (#2758) | `Generals{,MD}/.../MessageStream/MetaEvent.{h,cpp}` |
| `a7fad3bb2` | fix | Prevent multithread crash in `MilesAudioManager` (#2718) | `Core/.../MilesAudioDevice/MilesAudioManager.{h,cpp}`, `Dependencies/Utility/{CMakeLists.txt,Utility/interlocked_adapter.h}` |
| `b0becc407` | refactor | Clean up and simplify `MilesAudioManager` (#2718) | same + `Core/GameEngine/Include/Common/GameAudio.h` + `Generals{,MD}/.../MessageStream/CommandXlat.cpp` |
| `df2224bf1` | bugfix | Avoid crash with dangling contain in `Object::onDestroy` (#2747) | `Generals{,MD}/.../GameLogic/Object.{h,cpp}` |
| `20f42549c` | fix(memory) | Fix various memory leaks (2) (#2710) | `Generals{,MD}/.../GameEngine.cpp`, `Player.cpp`, `BattlePlanUpdate.cpp`, `SidesList.cpp`, `Drawable.cpp`, `W3DModelDraw.cpp`, `GameLogicDispatch.cpp`, `GameStateMap.cpp` + `Generals{,MD}/.../Libraries/Source/WWVegas/WW3D2/{hcanim,hrawanim,seglinerenderer}.cpp` + `Generals/.../Tools/WorldBuilder/src/ScriptDialog.cpp` |

## 2. Subsystem Risk Assessment

| Subsystem | Risk | Notes |
|---|---|---|
| **Input / MetaEvent** | HIGH | GeneralsX has a pre-existing local divergence (mods-only-changed detection + `m_lastKeyDown`/`m_lastModState` members). Upstream refactor (PR #2758) splits `translateGameMessage` into helper functions and uses a new `m_keyDownInfos[KEY_COUNT]` bit-array per key. **Conflict expected.** |
| **MilesAudioManager** | LOW | Source files are listed in `Core/GameEngineDevice/CMakeLists.txt` as a future-only/exploratory comment block — not compiled by any GeneralsX preset. New `interlocked_adapter.h` is small and inert. |
| **Object / Contain** | LOW | Upstream changes are pure additions (defensive null checks, mod-skip). No conflict expected. |
| **Memory leak fixes** | LOW | Additive in upstream. Conflict unlikely. GeneralsX code paths untouched. |
| **CI / workflows / docs** | NONE | None of the upstream commits touch `.github/`, `docs/`, `cmake/`, `scripts/`. We retain all GeneralsX CI infrastructure intact. |
| **Platform abstraction (SDL3/DXVK/OpenAL/FFmpeg)** | NONE | None of the upstream commits touch `Core/GameEngineDevice/` platform paths used by GeneralsX at runtime, or `Core/Libraries/Source/Platform/`. |

## 3. Conflict Resolution Strategy

### 3.1 `MetaEvent.cpp` (Generals/ and GeneralsMD/)

**Conflict source:** PR #2758 refactored `translateGameMessage` into four new helper functions (`onMouseEvent`, `onKeyEvent`, `onKeyModStateRemoved`, `onKeyPressed`, plus `getActionKeyType`/`getKeyModState` statics), and replaced the `m_lastKeyDown`/`m_lastModState` members with a `KeyDownInfo m_keyDownInfos[KEY_COUNT]` bit-array.

**GeneralsX divergence:** A pre-existing local change had ALREADY simplified the input pipeline (removing the order-of-modifier-release branch that PR #2757 introduced) and added a "mods-only-changed" detection using `m_lastKeyDown` and `m_lastModState`. This GeneralsX behavior must be preserved.

**Resolution:** Adopt the structural decomposition of the refactor, but with GeneralsX semantics inside the helper. Concretely:

- Keep the new top-level dispatch: `onMouseEvent(msg)` and `onKeyEvent(msg, disp)`.
- **Inside `onKeyEvent`**, use GeneralsX's logic:
  - Compute `newModState` from `systemKeyState` (CTRL/SHIFT/ALT).
  - Loop `TheMetaMap` and check BOTH:
    1. The GeneralsX "mods-only-changed" branch (`map->m_key == MK_NONE && newModState != m_lastModState` and the UP/DOWN transitions).
    2. The "normal" key transition branch (`map->m_key == key && map->m_modState == newModState`).
  - Fast-forward replay hack stays inline (GeneralsX keeps it here, upstream also keeps it in `onKeyPressed`).
  - Update `m_lastKeyDown` on `MSG_RAW_KEY_DOWN`.
  - Update `m_lastModState` at the end.
- **Do not** add `m_keyDownInfos`/`KeyDownInfo`, **do not** add `onKeyModStateRemoved`, `onKeyPressed`, `getActionKeyType`, `getKeyModState`. GeneralsX's mods-only-changed is the chosen behavior; introducing upstream's order-independent release would be a behavior change beyond the sync's scope.
- Keep GeneralsX's `MetaEventTranslator()` constructor that initializes `m_lastKeyDown(MK_NONE)` and `m_lastModState(0)`.
- Preserve the `DUMP_ALL_KEYS_TO_LOG` block (GeneralsMD only).

### 3.2 `MetaEvent.h` (Generals/ and GeneralsMD/)

The auto-merge produced an inconsistent state: it kept GeneralsX's `m_lastKeyDown`/`m_lastModState` members AND added upstream's helper function declarations, but it did NOT add the `KeyDownInfo` struct or `m_keyDownInfos[KEY_COUNT]` member (because they were on the same line as our existing members and git kept ours).

**Resolution:**

- Keep `m_lastKeyDown` and `m_lastModState` (GeneralsX semantics).
- Keep `onMouseEvent` and `onKeyEvent` declarations (upstream structure).
- **Remove** `onKeyModStateRemoved`, `onKeyPressed`, `getActionKeyType`, `getKeyModState` (unused — we won't define them in the `.cpp`).
- Do NOT add `KeyDownInfo` struct or `m_keyDownInfos`.

### 3.3 `MilesAudioManager.{h,cpp}` and `interlocked_adapter.h`

- Auto-merge result is acceptable. The `MilesAudioManager` is not compiled by any current GeneralsX preset (verified — the source path appears only in a comment block of `Core/GameEngineDevice/CMakeLists.txt`).
- `Dependencies/Utility/CMakeLists.txt` and `Dependencies/Utility/Utility/interlocked_adapter.h` are additive and safe.
- Will rebuild and re-run smoke to confirm no surprise side-effects.

### 3.4 All Other Files (auto-merged)

- Pure additive upstream changes. Trust the auto-merge.
- Sanity-check: re-run the configure flow on macOS (then Linux) and confirm the tree compiles.

## 4. Special Constraints

- **Do not** modify `.github/workflows/`, `.github/ISSUE_TEMPLATE/`, `.github/copilot-instructions.md`, or any CI configuration. None of the upstream commits touch these; the instruction is preventative.
- **Do not** introduce `m_keyDownInfos` / `KeyDownInfo` because it is semantically wrong for GeneralsX. Doing so would silently change input behavior beyond the sync's intent.
- **Do not** revert GeneralsX's mods-only-changed detection. It is the established GeneralsX input behavior.

## 5. Validation Plan

1. **macOS first** (primary validation target per the project's platform focus):
   - `cmake --preset macos-vulkan` — verify configure succeeds with the merged tree.
   - `cmake --build build/macos-vulkan --target z_generals` — verify build.
2. **Linux** (secondary, after macOS passes):
   - `cmake --preset linux64-deploy` (or `linux64-testing` if dependencies are limited).
   - `cmake --build build/linux64-deploy --target z_generals` — verify build.
3. **Smoke test** if binaries produce: run with `-win -logToCon` and confirm clean entry into the main loop. Use `scripts/qa/smoke/docker-smoke-test-zh.sh` if Docker is available, or `run.sh -win` on macOS/Linux host.

## 6. Steps to Execute

1. Resolve `MetaEvent.h` (Generals/ and GeneralsMD/) — remove unused upstream helper decls.
2. Resolve `MetaEvent.cpp` (Generals/ and GeneralsMD/) — adopt refactor with GeneralsX semantics.
3. Verify no `<<<<<<<` markers remain.
4. Run macOS configure + build.
5. Run Linux configure + build.
6. Commit, push, and provide final report.
