# Unified Engine Stage 04: Shared Engine Contracts (License-Identical Sources and NameKey Profile)

Date: 2026-10-23
Scope: consolidate ~138 license-identical Generals `GameEngine` sources onto Zero Hour
canonical paths, and reconcile `NameKeyGenerator` layout differences through
`GameProfile` instead of game-macro compile branches.

## Design

Gate 4 of PLAN-023 asks for shared engine contracts: reconcile duplicate
definitions subsystem by subsystem and replace game compile-time branches with
explicit profile policies where required. This stage stays on the low-risk
slice of that gate.

Generals `CMakeLists.txt` now compiles the matching Zero Hour
`GeneralsMD/Code/GameEngine/Source/...` translation units for sources that were
identical after substituting only the opening license game title. The Generals
copies are deleted. Each game still compiles those units separately against its
own headers; this does not introduce a single compiled engine library.

`NameKeyGenerator` was previously deferred (Stage 02) because the headers
disagreed on hash table size: Generals used a fixed `SOCKET_COUNT = 6473`,
Zero Hour used `45007`, and only Zero Hour declared `syncNameKeyID()`. The
implementation sources were already license-identical. Both game
`NameKeyGenerator.h` copies now use a dynamic `Bucket**` table sized at init
from `GameProfile::getNameKeySocketCount()` (6473 Generals / 45007 ZH).
`syncNameKeyID()` remains available under `RETAIL_COMPATIBLE_CRC` but no-ops
unless the selected profile is Zero Hour. Generals compiles the canonical
Zero Hour `NameKeyGenerator.cpp`.

`AudioSettings` money-transaction default volume was the same compile-time
pattern (`1.0f` Generals / `0.0f` Zero Hour). It now reads
`GameProfile::get().isGenerals()` instead of `#if RTS_*`.

## Anti-pattern: do not share game Include trees yet

Briefly adding `GeneralsMD/Code/GameEngine/Include` to `g_gameengine` include
paths made Generals pick up Zero Hour NGMP headers
(`GameNetwork/GeneralsOnline/...`). The Generals link then failed on unresolved
NGMP symbols (`NGMP_OnlineServicesManager::getInstance`, `TheNGMPGame`, and
related calls from Core objects that include those headers). That include
retarget is **not** a consolidation solution. Keep separate Generals and Zero
Hour `GameEngine/Include` trees until headers are reconciled deliberately.
CompatLib Include sharing (Stage 02) remains fine; game headers do not.

`NameKeyGenerator.h` therefore stays duplicated under each game Include tree,
with matching layout. A provenance entry that claimed Generals would fall
through to the Zero Hour header was removed for that reason.

## Provenance

`cmake/unified-engine-provenance.json` gains 138 Stage 04 GameEngine Source
entries (one per deleted Generals copy). Total map size is **170** entries
(Stage 01–02 baseline 32 + 138). Baseline revisions are unchanged
(`source_revision` afb09f144, `upstream_revision` 0af9db488). Policy text for
these sources records Zero Hour as canonical, license-title-only identity, and
separate compilation against each game's headers.

## Commands

```bash
python3 scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py
python3 -m unittest discover -s scripts/qa -p test_unified_engine_provenance.py
```

## Local validation

Work continued on the Stage 01 `ubuntu-dev` Ubuntu 24.04 tree
`build/linux64-unified-baseline`.

Evidence already on disk:

| Check | Result | Log |
|---|---|---|
| Generals build after source retarget | linked `Generals/GeneralsX` | `logs/unified-stage4-build.log` |
| Both targets after NameKey profile sizing | linked `g_generals` and `z_generals` | `logs/unified-stage4-namekey-build.log` |
| Headless Generals smoke | exit 0, `[GAME_PROFILE] selected 'generals'` | `logs/unified-stage4-generals-smoke.out` |
| Headless Zero Hour smoke | exit 0, `[GAME_PROFILE] selected 'zh'` | `logs/unified-stage4-zh-smoke.out` |
| Anti-pattern: GeneralsMD GameEngine Include on `g_gameengine` | Generals link fails (NGMP undefs) | `logs/unified-stage4b-validate.log` |
| Rebuild after Include revert + NameKey/AudioSettings touch | linked both executables | `logs/unified-stage4-rebuild2.log` |
| Post-rebuild headless smokes | both exit 0 with `[GAME_PROFILE]` | `logs/unified-stage4-generals-smoke.out`, `logs/unified-stage4-zh-smoke.out` |
| NameKey expected-ID profile wiring | both linked; smokes exit 0 | `logs/unified-stage4-namekeyid-*.out` |
| Provenance structural check | 170 entries valid | — |

## Intentionally deferred

- Sharing remaining divergent `GameEngine` sources or a single compiled engine
  object library.
- Collapsing Generals/Zero Hour `GameEngine/Include` trees (blocked by NGMP and
  other unreconciled headers; see anti-pattern above).
- Replacing the remaining gameplay `#if RTS_*` branches with profile policies.
- Rules/serialization policies and the single executable (later PLAN-023 gates).
- **Header inventory (explore pass):** ~221 license-only / identical headers
  remain (no Tools/; no GeneralsOnline). ~382 further-diff no-Tools pairs
  (headers+sources) still block Include-tree merge; do not add ZH
  `GameEngine/Include` to Generals. Candidate Core promotions listed in
  `UNIFIED_ENGINE_STAGE04_HEADER_CANDIDATES.md`. Next leverage: Player/Object
  xfer reconciliation rather than header redirects or GED Include deletes
  (Generals still resolves those via its own `Include/`, not Core/ZH).
