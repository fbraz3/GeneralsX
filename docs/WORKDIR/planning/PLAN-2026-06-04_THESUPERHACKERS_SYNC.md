# PLAN-2026-06-04 — TheSuperHackers Upstream Sync

**Branch**: `thesuperhackers-sync-06-04-2026`
**Source**: `thesuperhackers/main` @ `7c78a5e17`
**Last local sync ancestor**: `20f42549c`
**Commits being merged**: 5 (very small sync; previous syncs caught most of upstream)
**Divergence**: 1813 commits ours / 5 commits theirs

---

## 1 — Scope: What Is Being Brought In

| PR | Commit | Subject | Risk |
|---|---|---|---|
| #2710 | `20f42549c` | fix(memory): Fix various memory leaks (2) | Low — defensive guards + RAII placement |
| #2747 | `df2224bf1` | bugfix(object): Avoid crash in Object::onDestroy with dangling contain module (Troop Crawler) | Low — null check + cached pointer |
| #2718 | `b0becc407` + `a7fad3bb2` | refactor + fix MilesAudioManager (threading + cleanup) | **HIGH — touches shared `AudioManager` base class API** |
| #2758 | `7c78a5e17` | refactor(metaevent): Split `MetaEventTranslator::translateGameMessage()` | **HIGH — bundles #2577 modifier-release-order fix, replaces members** |

---

## 2 — File Surface

### Real conflicts (`UU`)
- `Generals/Code/GameEngine/Source/GameClient/MessageStream/MetaEvent.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/MetaEvent.cpp`

### Auto-merged but LOGICALLY BROKEN (must be hand-repaired)
- `Generals/Code/GameEngine/Include/GameClient/MetaEvent.h`
- `GeneralsMD/Code/GameEngine/Include/GameClient/MetaEvent.h`
  - Auto-merge unioned NEW declarations (`onKeyEvent`, `KeyDownInfo`, etc.) but DID NOT remove the OLD members (`m_lastKeyDown`, `m_lastModState`). Header is internally inconsistent. The constructor in the cpp still initializes the old members. Must replace member section with upstream's `KeyDownInfo` block.

### Auto-merged with downstream impact on GeneralsX-specific code
- `Core/GameEngine/Include/Common/GameAudio.h` — base-class virtual signatures changed:
  - `nextMusicTrack()` / `prevMusicTrack()` now return `AsciiString` (was `void`)
  - `getMusicTrackName()` was REMOVED from the abstract base
  - **Impact**: `Core/GameEngineDevice/Source/OpenALAudioDevice/OpenALAudioManager.{h,cpp}` overrides must be updated to match new signatures, otherwise the class becomes abstract and won't link.

### Auto-merged, accepting as-is (verified clean)
- `Core/GameEngineDevice/Include/MilesAudioDevice/MilesAudioManager.h`
- `Core/GameEngineDevice/Source/MilesAudioDevice/MilesAudioManager.cpp`
- `Core/GameEngineDevice/Source/W3DDevice/.../W3DModelDraw.cpp`
- `Core/Libraries/Source/WWVegas/WW3D2/hcanim.cpp`
- `Core/Libraries/Source/WWVegas/WW3D2/seglinerenderer.cpp`
- `Dependencies/Utility/CMakeLists.txt`
- `Dependencies/Utility/Utility/interlocked_adapter.h` (new file)
- `Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp`
- `{Generals,GeneralsMD}/Code/GameEngine/Include/GameLogic/Object.h`
- `{Generals,GeneralsMD}/Code/GameEngine/Source/Common/GameEngine.cpp`
- `{Generals,GeneralsMD}/Code/GameEngine/Source/Common/RTS/Player.cpp`
- `{Generals,GeneralsMD}/Code/GameEngine/Source/Common/System/SaveGame/GameStateMap.cpp`
- `{Generals,GeneralsMD}/Code/GameEngine/Source/GameClient/MessageStream/CommandXlat.cpp`
- `{Generals,GeneralsMD}/Code/GameEngine/Source/GameLogic/Map/SidesList.cpp`
- `{Generals,GeneralsMD}/Code/GameEngine/Source/GameLogic/Object/Object.cpp`
- `{Generals,GeneralsMD}/Code/GameEngine/Source/GameLogic/Object/Update/BattlePlanUpdate.cpp`
- `{Generals,GeneralsMD}/Code/Libraries/Source/WWVegas/WW3D2/hrawanim.cpp`
- `{Generals,GeneralsMD}/Code/Tools/WorldBuilder/src/ScriptDialog.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameClient/Drawable.cpp`

These are bugfix patches with small targeted changes. Spot-check during build.

---

## 3 — CI/CD Protection Audit

Per Critical Merge Instructions:
- This sync brought in **zero** modifications to `.github/workflows/`, `.github/ISSUE_TEMPLATE/`, or `.github/copilot-instructions.md` (the upstream commits don't touch those paths).
- Confirmed by inspecting the 5 commits' file lists. Nothing to reject.

---

## 4 — Conflict Resolution Decisions

### 4.1 `MetaEvent.cpp` (both branches) — UU conflict

**Conflict region**: keyboard handling block inside `MetaEventTranslator::translateGameMessage`.

**HEAD**: ~100 lines of inline keyboard logic using `m_lastKeyDown` / `m_lastModState`.

**Upstream**: One line `onKeyEvent(msg, disp);`, with the logic extracted into new methods (`onKeyEvent`, `onKeyModStateRemoved`, `onKeyPressed`) plus the modifier-release-order bug fix from #2577. The new methods are already present at the bottom of the merged file (auto-merge added them).

**Decision**: Take the upstream side (`onKeyEvent(msg, disp);`). Rationale:
- The new methods are the SAME logic, just refactored + bug-fixed.
- The bug fix in #2577 (ignore modifier-release order) is a real gameplay improvement.
- Keeping HEAD would dead-code the new methods AND lose the fix.
- The conflict is purely about WHICH code path runs; there is no cross-platform code to preserve here.

### 4.2 `MetaEvent.h` (both branches) — broken auto-merge

**Problem**: Auto-merge unioned new method declarations + new `KeyDownInfo` struct, but kept the old `m_lastKeyDown` / `m_lastModState` member fields. The result references members the upstream code no longer uses, AND still has the old members.

**Decision**: Replace the entire `MetaEventTranslator` member section (between `private:` and `enum { NUM_MOUSE_BUTTONS … }`) with the upstream version — i.e. delete `m_lastKeyDown` / `m_lastModState`, insert the full `KeyDownInfo` struct and `KeyDownInfo m_keyDownInfos[KEY_COUNT];`. Rationale:
- The cpp body (auto-merged + the upstream `onKeyEvent` call) only uses `m_keyDownInfos`, never the old members. Keeping the old members is dead memory and a maintenance trap.
- The new struct is purely internal to the translator — no public API or cross-platform code is affected.

### 4.3 `MetaEvent.cpp` constructor — broken auto-merge

**Problem**: Constructor still initializes `m_lastKeyDown(MK_NONE)` and `m_lastModState(0)`, which won't exist after fixing the header (4.2).

**Decision**: Remove those two member initializers. `m_keyDownInfos[]` self-initializes via `KeyDownInfo()` → `m_modStateBits(0)`. Loop over mouse buttons stays.

### 4.4 `GameAudio.h` base-class API change → OpenAL override fix

**Problem**: Upstream changed `AudioManager::nextMusicTrack()` and `prevMusicTrack()` to return `AsciiString` (was `void`) and removed `getMusicTrackName()`. Our `OpenALAudioManager` still uses the old signatures, which will:
- Fail to override (silent), making the class abstract and breaking link
- Leave `getMusicTrackName` as a non-override helper (acceptable)

**Decision**:
- Update `Core/GameEngineDevice/Include/OpenALAudioDevice/OpenALAudioManager.h` and `OpenALAudioManager.cpp` to return `AsciiString` from `nextMusicTrack`/`prevMusicTrack`, returning the new track name (mirror the Miles upstream impl).
- Keep `getMusicTrackName()` as a class-only helper (no `override` since base removed it). The OpenAL impl uses it internally for tracking.
- Update the dead stub `GeneralsMD/Code/GameEngineDevice/Source/OpenALAudioManager.cpp` for source-tree cleanliness (file is never compiled in current configs but the signatures should match for future maintenance).

Rationale: The Miles upstream change makes the call site simpler (`AsciiString name = TheAudio->nextMusicTrack();`). CommandXlat.cpp already uses the new pattern (auto-merged from upstream). Our OpenAL backend must match so the cross-platform stack stays linkable.

### 4.5 Audio API change risk re-check

Verified callers via grep:
- `CommandXlat.cpp` (both branches) uses `AsciiString trackName = TheAudio->nextMusicTrack();` — matches new API.
- No remaining callers of `AudioManager::getMusicTrackName()` — safe to drop from base.

### 4.6 No INI parser changes in this sync

The macOS INI numeric-parsing risk noted in the merge instructions does **not** apply here. None of the 5 commits touch `INI.cpp` or `INIWebpageURL.cpp`.

### 4.7 No file moves into `Core/` in this sync

The "many files moved from Generals/GeneralsMD into Core/" warning from the instructions does **not** apply to this 5-commit sync. The Core/ unification mostly happened in earlier upstream syncs (already in our tree). Our last sync ancestor `20f42549c` is recent.

---

## 5 — Risk-Mitigation Plan per Subsystem

### Audio (HIGH RISK)
- The Miles refactor + base-class API change is the most invasive piece.
- Mitigation:
  - Update OpenAL signatures (4.4).
  - Configure + build Linux and macOS to catch link-time errors.
  - Smoke-test main menu load (audio init path runs on startup).
  - Do NOT touch Miles cpp/h other than what auto-merged — Miles is reference-only on Linux/macOS, but the code still compiles in some configs.

### Input / Keyboard (MEDIUM RISK)
- MetaEvent refactor bundles a real behavioral fix. The fix only matters when multiple modifier keys are pressed and released out of order, so smoke-testing main menu navigation is enough to verify no regression.

### Memory leak patches (LOW RISK)
- Small, targeted, defensive. Build verification is sufficient.

### CI / build system (NO CHANGE)
- Our `.github/` is unchanged.
- `Dependencies/Utility/CMakeLists.txt` adds a new header; verified the new header (`interlocked_adapter.h`) exists.

---

## 6 — Execution Steps (order)

1. Fix `Generals/Code/GameEngine/Include/GameClient/MetaEvent.h` (replace member block with upstream's).
2. Fix `GeneralsMD/Code/GameEngine/Include/GameClient/MetaEvent.h` (same).
3. Resolve `Generals/Code/GameEngine/Source/GameClient/MessageStream/MetaEvent.cpp` UU + drop dead constructor initializers.
4. Resolve `GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/MetaEvent.cpp` UU + drop dead constructor initializers.
5. Update `Core/GameEngineDevice/Include/OpenALAudioDevice/OpenALAudioManager.h` — new signatures.
6. Update `Core/GameEngineDevice/Source/OpenALAudioDevice/OpenALAudioManager.cpp` — return track name.
7. Update `GeneralsMD/Code/GameEngineDevice/Source/OpenALAudioManager.cpp` stub for parity (low priority — for tree cleanliness only).
8. Search for any other stale `getMusicTrackName` usage from the base class — none expected based on audit.
9. Verify zero conflict markers anywhere.
10. Configure macOS (`macos-vulkan`).
11. Build macOS `z_generals` and `z_generalsxzh` (or whatever targets succeed).
12. If macOS builds clean, attempt Linux (`linux64-deploy`) via Docker.
13. Runtime smoke for `GeneralsXZH` and `GeneralsX` (enter main loop, exit cleanly).
14. Remove generated artifacts (caches, build directories left in working tree).
15. Commit with attribution to upstream PRs.
16. Push branch to origin.

---

## 7 — Open Questions / Deferred Items

- None identified. All five upstream commits are well-scoped.
