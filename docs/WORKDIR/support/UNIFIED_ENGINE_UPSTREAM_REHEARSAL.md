# Unified Engine Upstream Integration Rehearsal (Gate 6)

Date: 2026-10-23
Status: ancestry repaired; isolated whole-tree merge of `upstream/main` completed on `echelon/upstream-rehearsal-gate6` @ `0fea59a33` (165 conflicts, GeneralsX-preferring resolution); actionable mapped DIFF ports landed on primary plan branch (RandomValueClass/Geometry/`WWMath::Fabs`/`#3389`); remaining DIFF vs tip intentional WWMath/GameProfile/Linux

## Procedure (PLAN-023)

1. Record incoming range; preserve downstream HEAD (do not merge onto the working branch).
2. Run the provenance checker over the range.
3. Review mapped dual-path hits; adapt into canonical destinations (not ours/theirs).
4. Build both profiles / `echelon`; validate affected scenarios.
5. Record integrated commit, adaptations, and validation.

Isolated worktree: `/home/cheviiot/Project/Games/GeneralsX-wt-upstream-rehearsal`
(`echelon/upstream-rehearsal-gate6` @ `fadcebedd` + adaptation commits).

## Recorded range

| Field | Value |
|---|---|
| Downstream HEAD (at rehearsal) | `fadcebedd` (Echelon branding) |
| Provenance `upstream_revision` (base) | `0af9db488ece8301f4e04fbb6af22115096d4131` |
| Upstream `main` tip (head) | `b7f0e31cb964ba572c0547fe44a83053f3da875e` |
| Range | `0af9db48..b7f0e31c` |
| Checker log | `logs/upstream-rehearsal-provenance.json` |

Command:

```bash
# Shallow fetch makes merge-base empty ("unrelated histories"). Deepen first:
./scripts/tooling/git/fetch-upstream-main.sh
# or: git fetch upstream main --deepen=5000

python3 scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py \
  --upstream-range 0af9db488ece8301f4e04fbb6af22115096d4131..b7f0e31cb964ba572c0547fe44a83053f3da875e
```

## Checker summary

- Manifest valid: **170** consolidated entries.
- **Mapped changes: 15** (both Generals + GeneralsMD former paths touched; must land on ZH canonical).
- **Unmapped paths: 578** (expected for this wide range: Core-only edits, tools, cmake, tests, and still-separate game trees).

### Mapped canonicals (adaptation required)

1. `GeneralsMD/.../Handicap.cpp`
2. `GeneralsMD/.../Geometry.cpp`
3. `GeneralsMD/.../PopupReplay.cpp`
4. `GeneralsMD/.../ReplayMenu.cpp`
5. `GeneralsMD/.../SimpleObjectIterator.cpp`
6. `GeneralsMD/.../POWTruckAIUpdate.cpp`
7. `GeneralsMD/.../WanderAIUpdate.cpp`
8. `GeneralsMD/.../CleanupHazardUpdate.cpp`
9. `GeneralsMD/.../SupplyWarehouseDockUpdate.cpp`
10. `GeneralsMD/.../DynamicShroudClearingRangeUpdate.cpp`
11. `GeneralsMD/.../FloatUpdate.cpp`
12. `GeneralsMD/.../NeutronMissileSlowDeathUpdate.cpp`
13. `GeneralsMD/.../NeutronMissileUpdate.cpp`
14. `GeneralsMD/.../TensileFormationUpdate.cpp`
15. `GeneralsMD/.../ToppleUpdate.cpp`

Each mapped hit lists both `Generals/...` and `GeneralsMD/...` in `changed_paths`.

### Sample adaptation note (`Geometry.cpp`)

Upstream commits in range (both trees):

- `b9fb894bf` — route game-engine math through WWMath (#2670)
- `e572d872f` — MinGW BSTR redefinition fix (#3550)

Stat: 18 insertions / 18 deletions on each former path. On merge, apply once to
`GeneralsMD/Code/GameEngine/Source/Common/System/Geometry.cpp` (canonical), confirm
Generals CMake still compiles that path, and do not revive
`Generals/Code/GameEngine/Source/Common/System/Geometry.cpp`.

## Findings for Gate 6

- Provenance tooling correctly flags many-to-one updates; `--fail-on-mapped-change`
  would exit 2 on this range (15 mapped).
- A full rehearsal merge remains open: isolated worktree, dual-profile / `echelon`
  builds, and targeted replay/smoke after adapting the 15 files.
- Wide unmapped Core noise does not mean Core is unmanaged; many Core files were
  never dual-origin pairs in the map. Review Core diffs separately when they
  affect shared runtime.

## Adaptation progress (2026-10-24)

Re-fetched `upstream/main` and compared the 15 mapped canonicals:

| Canonical | vs upstream tip | Notes |
|---|---|---|
| `Handicap.cpp` | adapted | Applied #3389 unsized arrays + `static_assert` on ZH-canonical |
| `WanderAIUpdate.cpp` | adapted | Upstream #3544 via `GameProfile` (Generals NE bias / ZH ±50 about position) |
| `SimpleObjectIterator.cpp` | identical | |
| Remaining 12 | GeneralsX-ahead | Keep `WWMath::*Origin`/`*Trig`, Linux path separators, div-by-zero guards; do **not** revert to upstream `fabs`/`Sqrt`/`Pow` |

### Ancestry repair (2026-10-24)

Root cause of “unrelated histories”: `upstream/main` was a **shallow** fetch
(~50 commits, orphan tip). After `git fetch upstream main --deepen=5000`:

| Check | Result |
|---|---|
| `git rev-list --count upstream/main` | ~2028 |
| `git merge-base HEAD upstream/main` | `0af9db488` (provenance base) |
| `0af9db488` ancestor of `upstream/main` | yes |

Whole-tree merge is no longer blocked by missing ancestry. Helper:
`scripts/tooling/git/fetch-upstream-main.sh`.

### Isolated merge rehearsal (2026-10-24)

| Field | Value |
|---|---|
| Worktree | `/home/cheviiot/Project/Games/GeneralsX-wt-upstream-rehearsal` |
| Branch | `echelon/upstream-rehearsal-gate6` |
| Merge commit | `0fea59a33` |
| Range merged | `0af9db488..b7f0e31cb` (58 commits on `upstream/main`) |
| Conflicts | **165** (log: `logs/upstream-merge-conflicts-20261024.txt`) |

Resolution policy (GeneralsX-preferring, not blind ours/theirs):

1. `GameEngineDevice` / W3D device → **ours** (platform isolation)
2. `cmake/` / CMakeLists → **ours** (GeneralsX presets / gamemath)
3. `Generals/Code/**` dual-tree side → **ours** (canonical adaptations land on ZH)
4. WWVegas / WWMath / Lib → **ours** (determinism)
5. Remaining Core + `GeneralsMD` conflicts → **ours** (preserve WWMath / GameProfile);
   re-check with `adapt_unified_engine_upstream.py` and manually port upstream-only
   logic into canonical paths when DIFF is intentional GeneralsX divergence

Post-merge: DIFF list in `logs/upstream-adapt-after-merge-20261024.txt`. CompatLib
MISS entries are expected (upstream layout differs). Primary working tree smokes
are unchanged (merge not promoted to `echelon/unified-engine-plan` yet).

Campaign/skirmish map-load smokes (USA01, Alpine Assault, ShellMap1) exit 0 on
the primary working tree with `-map` + `-quitAfterFrames`.

## File-level sync helper

```bash
python3 scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py
python3 scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py --only WanderAIUpdate
# Review-only staging of upstream tip into canonical paths:
# python3 scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py --stage-canonical --only Handicap.cpp
```

Reports SAME/DIFF/MISS vs `upstream/main` for every provenance canonical. Does not
auto-merge WWMath or GameProfile adaptations.

## Promote onto `echelon/unified-engine-plan` (do not reuse `0fea59a33`)

The rehearsal commit `0fea59a33` was merged onto the tip at rehearsal time
(`fadcebedd`). The plan branch has since advanced with GameProfile / CRC / dual-layout
work. Blindly merging or resetting to `0fea59a33` would drop those commits.

Promote procedure (fresh isolated worktree from **current** plan tip):

```bash
# 1) Deepen upstream if needed
./scripts/tooling/git/fetch-upstream-main.sh

# 2) Fresh worktree from current plan tip (not 0fea59a33)
WT=/home/cheviiot/Project/Games/GeneralsX-wt-upstream-promote
git worktree add -b echelon/upstream-promote-$(date +%Y%m%d) "$WT" echelon/unified-engine-plan

# 3) Merge current upstream/main tip; resolve with GeneralsX-preferring policy
#    (same as Isolated merge rehearsal section above)
cd "$WT"
git merge upstream/main   # expect conflicts; do not -Xours/-Xtheirs blindly

# 4) Re-run adapt + provenance; dual-profile + record-replay smokes
python3 scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py
./scripts/qa/smoke/smoke-echelon-dual-profile.sh build/linux64-unified-baseline
./scripts/qa/smoke/smoke-echelon-record-replay.sh build/linux64-unified-baseline zh
./scripts/qa/smoke/smoke-echelon-record-replay.sh build/linux64-unified-baseline generals

# 5) Only after green smokes: fast-forward or merge promote branch into plan tip
```

Gate 6 acceptance still requires dual migration target removal after promote +
fixture CRC parity review.


## Fresh-tip promote (2026-10-24)

| Field | Value |
|---|---|
| Worktree | `/home/cheviiot/Project/Games/GeneralsX-wt-upstream-promote` |
| Branch | `echelon/upstream-promote-20261024` @ `14e21aa83` |
| Tip snapshot | `de2d0331f` (Gate 6 GameProfile dual-path tip) |
| Merge commit | `d6a0388f1` (then restore/fix commits) |
| Upstream tip | `f02fcd381` (`upstream/main`) |
| Conflicts | **186** (log: `logs/upstream-promote-conflicts-20261024.txt`) |
| Resolution | GeneralsX-preferring ours; Core/Generals/GeneralsMD/cmake restored to tip after auto-merge API skew; WWMath upstream-name aliases kept |
| Smoke | dual-profile + record→playback ZH/Generals exit 0 after promote restore |

Do not reuse `0fea59a33`. Follow-up: selective Core upstream ports (filesystem case-fold #3531, headless font dummies #3538, etc.) without breaking WWMath pairing; merge promote into `echelon/unified-engine-plan` when ready; then `RTS_BUILD_MIGRATION_BINARIES=OFF`.

## References

- PLAN-023 Integration procedure
- `cmake/unified-engine-provenance.json`
- `scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py`
- `scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py`
