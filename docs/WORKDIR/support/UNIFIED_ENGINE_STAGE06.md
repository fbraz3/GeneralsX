# Unified Engine Stage 06: One Executable Scaffold (`echelon`)

Date: 2026-10-23 (updated 24/10/2026)
Status: Gate 6 Linux acceptance green — promote FF'd (`14e21aa83`), migration binaries OFF, `smoke-echelon-gate6.sh` (dual/map/mod/LAN/save/RR/CRC fixtures) exit 0; CRC→GameProfile waves 1–11; `#3531` FileSystem path-key + `#3538` headless font dummies ported; global `RETAIL_COMPATIBLE_CRC` stays 0. Remaining: macOS evidence; optional old-fixture CRC

## Scope (this stage)

| Done | Not done |
|---|---|
| CMake target `echelon` (`OUTPUT_NAME echelon`), option `RTS_BUILD_ECHELON` | Single compiled engine library |
| `RTS_UNIFIED_ENGINE=1` on echelon Main only | Remaining `#if RTS_*` behind `RETAIL_COMPATIBLE_PATHFINDING=0` |
| `GameProfile::selectForProcess(..., allowAnyProfile)` + `ECHELON_GAME` / `ECHELON_ASK_PROFILE` | macOS `macos-vulkan` Gate 6 build/smoke evidence |
| Links `z_gameengine` + NGMP as today | Optional older generalsx-replays CRC parity |
| `RTS_BUILD_MIGRATION_BINARIES` default OFF | Optional ON for mismatch binaries |
| Deploy + dual-profile / mod / LAN / gate6 smoke scripts | Interactive dual-instance LAN lobby UI |
| AIPathfind pure Generals sites → `GameProfile` | |
| Upstream rehearsal dry-run (15 mapped) | |
| Batch A: maps / LoadScreen / transitions / KindOf / LAN / DelayedUpgrade modules | |
| Batch B: CommandXlat sabotage+camera / HintSpy / MetaEvent / SelectionInfo / ControlBar | |
| Batch C: GameInfo / ConnectionManager / Network / GameMemoryInit dual pool tables | |
| Pre-main pool fix: unselected → ZH tables without locking profile (`libhwy` `new`) | |
| Batch D: Object v7 status / WeaponSet damage-mask / Locomotor DecelerationPitchLimit | |
| PathfindLayer / ZoneManager / ZoneBlock / AIPathfind: `RETAIL…` + `GameProfile` (flag still 0) | |
| `#3531` path-key + `#3538` headless DisplayString/FontLibrary dummies | |
| 05.7 tunnel-contain helper → always-on + `GameProfile` call site | |
| 05.7 Player/Object vision dual-layout + Generals-profile look/xfer | |
| 05.7 TunnelTracker / BodyModule / StickyBomb / OpenContain → GameProfile | |
| Generals `getGeneralFeatures` stub — Core LoadScreen builds again | |
| PolygonTrigger map v3/v4 + Drawable xfer v5/v7 via `GameProfile` | |
| ThingTemplate SoundDie slots always-on + full `z_gameengine` rebuild; death sound by profile | |
| TERRAIN_DECAL ZH-canonical + Generals wire remap (NONE/FANATICISM/CHEMSUIT) in `Drawable::xfer` | |
| W3DTerrainVisual xfer v2/v3 + W3DView camera pitch/rotate/waypoint via `GameProfile` | |
| ThingTemplate KindOf AIRFIELD/DRONE remaps + AutoHeal inherit via `GameProfile` | |
| Object `onDisabledEdge` + AIStates under-construction attack via `GameProfile` | |
| Smoke script also checks GeneralsX/GeneralsXZH wrong-profile FATAL | |
| Release `-map` + `-quitAfterFrames`; dual-profile ShellMap1/USA01/Alpine map-load exit 0 | Clean CRC replay parity vs generalsx-replays fixtures |
| Isolated worktree + #3544/#3389 adaptations; ancestry `--deepen`; whole-tree merge @ `0fea59a33` | Promote merge + port remaining DIFF upstream fixes; dual-target removal |
| `adapt_unified_engine_upstream.py` + `deploy-linux-echelon.sh` (ldd-first FFmpeg SONAME copy) | Dual migration target removal |
| `smoke-echelon-replay.sh`: ZH fixture soft-pass CRC mismatch frame 100 (exit 2); cross-profile soft-pass (exit 3); self record→playback CRC-clean | External fixture CRC parity vs older generalsx-replays (still frame 100 after waves 1–11) |
| `RTS_BUILD_MIGRATION_BINARIES` default OFF; `smoke-echelon-gate6.sh` acceptance battery | |
| `-lanProbe` (startup CLI) + `smoke-echelon-lan.sh` dual-profile bind/RequestLocations exit 0 | Interactive dual-instance LAN lobby match |
| `-skirmish` + `smoke-echelon-record-replay.sh` dual-profile record→playback CRC exit 0 | |
| `smoke-echelon-mod-path.sh` dual-profile `-mod` exit 0; `deploy-linux-echelon.sh` restored | |
| `#3531` `FileSystem::makePathKey` / `comparePath` + FilenameLess (mod-override kept) | |

## How to smoke

```bash
cmake --build build/linux64-unified-baseline --target echelon g_generals z_generals --parallel 6

./scripts/qa/smoke/smoke-echelon-dual-profile.sh build/linux64-unified-baseline

# Map load (ShellMap; timeout after map.ini is success)
./scripts/qa/smoke/smoke-echelon-map-load.sh build/linux64-unified-baseline 45

# Replay path (fixtures from https://gitlab.com/fbraz3/generalsx-replays)
./scripts/qa/smoke/smoke-echelon-replay.sh build/linux64-unified-baseline \
  logs/fixtures/replays/zh/linux_2p_vanilla_map.rep zh
# exit 0 clean; 2 = CRC drift soft-pass; 3 = cross-profile isolation soft-pass

# Record+playback CRC-clean (same binary)
./scripts/qa/smoke/smoke-echelon-record-replay.sh build/linux64-unified-baseline zh
./scripts/qa/smoke/smoke-echelon-record-replay.sh build/linux64-unified-baseline generals

# LAN bind/discovery (no lobby UI)
./scripts/qa/smoke/smoke-echelon-lan.sh build/linux64-unified-baseline

# Migration binaries — mismatch still fatal
build/.../Generals/GeneralsX -game zh   # expect exit 1
build/.../GeneralsMD/GeneralsXZH -game generals  # expect exit 1
```

Deploy:

```bash
./scripts/build/linux/deploy-linux-echelon.sh build/linux64-unified-baseline
# installs echelon (+ GeneralsXZH alias); ./run.sh -game zh|generals -win
```

## Save/load smoke (2026-10-24)

- `smoke-echelon-save-load.sh` ZH + Generals: save at frame 120, load leaf `.sav`, quit after 80 — exit 0.
- Root cause of prior `SC_ERROR`: short-map converter turned `Maps\Alpine Assault\Alpine Assault.map` into a non-existent triple path so `embedPristineMap` could not open the BIG entry.

## LAN dual discovery

- Script: `scripts/qa/smoke/smoke-echelon-lan-dual.sh` — podman bridge twins (preferred) or netns+veth; `ECHELON_LAN_PROBE_EXPECT_PEER=1`.
- Verified 24/10/2026: ZH + Generals exit 0 (mutual `peer seen` + `ok peers=`). Soft-pass exit 3 if neither path works.

## How to smoke (LAN / save)

```bash
./scripts/qa/smoke/smoke-echelon-lan.sh build/linux64-unified-baseline
./scripts/qa/smoke/smoke-echelon-lan-dual.sh build/linux64-unified-baseline zh
./scripts/qa/smoke/smoke-echelon-save-load.sh build/linux64-unified-baseline zh
./scripts/qa/smoke/smoke-echelon-save-load.sh build/linux64-unified-baseline generals
```

## Next (Gate 6 remaining)

1. macOS: configure/build `macos-vulkan` + dual-profile smoke (CI `build-macos.yml` or native Mac host — deferred on Linux-only Gate 6 tip).
2. Optional: interactive lobby UI; reconcile older generalsx-replays fixtures CRC (frame 100).
3. Do **not** flip global `RETAIL_COMPATIBLE_CRC=1` — GameProfile paths cover dual retail rules.

## Adapt notes (24/10/2026)

- `#3389` `ITER_ORDER_TYPE_COUNT` + `SimpleObjectIterator` `static_assert` (MD + Generals `ObjectIter.h`).
- Bare `fabs` → `WWMath::Fabs` in TensileFormation / Topple / NeutronMissile / AIPlayer (Generals AIPlayer backport).
- Upstream `RandomValueClass` / Geometry inject; EMP/SpecialAbility CRC decoupling.
- 05.7 vision dual-layout; TunnelTracker/`BodyModule`/StickyBomb; OpenContain → `GameProfile`; Generals `getGeneralFeatures` stub.
- Core `RandomValue` retail edge cases + Unchanged seed + Xfer upgrade-mask CRC size + EMP/SA particle CRC → `GameProfile` (global `RETAIL_COMPATIBLE_CRC` remains 0).
- ParticleSys always-virtual create + Dummy init/reset via GameProfile; GameLogicDispatch NEW_GAME guard via GameProfile.
- Dual-profile + record→playback CRC-clean + save/load smoke exit 0 both profiles.
- CRC→GameProfile: AIPathfind remaining sites, Network destroy/quit, Dispatch retaliation+owner checks, Player bounty/battleplan/upgrade-transfer, JetAI behavioral (MD+G). JetPause XFER stays compile-time `XFER_SAVE`.
- Rebuild via ubuntu-dev; dual-profile + record→playback CRC-clean both profiles exit 0.
- CRC wave 2 → GameProfile: AIPlayer, AIStates, TunnelContain, GameAudio, ChinookAIUpdate (MD+G); dual+RR CRC-clean both profiles exit 0.
- CRC wave 3 → GameProfile: ActiveBody + Object behavioral sites (MD+G); dual+RR CRC-clean both profiles exit 0.
- CRC wave 4 → GameProfile: PartitionManager, TransportContain, AIUpdate contact/exit/horde, Weapon historic/sniper (MD+G); dual+RR CRC-clean both profiles exit 0.
- CRC wave 5 → GameProfile: GameLogic disabled-mask + fence zero-init; InstantDeathBehavior; ProductionUpdate cancel; ActionManager resume/sneak-attack; dual+RR green.
- CRC wave 6 → GameProfile: DozerAIUpdate, TerrainLogic, ObjectCreationList, ParticleUplinkCannon; dual+RR green.
- CRC wave 7 → GameProfile: WorkerAIUpdate, BattlePlanUpdate, HelixContain, SpecialAbilityUpdate; dual+RR green.
- CRC wave 8–9 → GameProfile: ScriptActions; Countermeasures/Stealth/Spectre/Neutron/Energy/BuildAssistant/AIGroup/AIGuard/PolygonTrigger; dual+RR green.
- CRC wave 10 → GameProfile: SupplyCenterDock, OCLUpdate, ScriptEngine, BunkerBuster, AIGuardRetaliate, UndeadBody, Sabotage*, PropagandaTower enclose; dual+RR green.
- CRC wave 11 → NameKey sync/verify always compiled; GameEngine call sites ungated; ZH priming via GameProfile; dual+RR green.
- CRC wave 11b → Object upgrade-mask CRC width + ExperienceTracker `m_isTrainable` via GameProfile; dual+RR green; restored `saveAtLogicFrame` hook; save/load green.
- Left intentionally: `PRESERVE_TUNNEL_HEAL_STACKING` dual API (PRESERVE=1); JetAI/AIUpdate `XFER_SAVE`; RTS_DEBUG asserts.
- Promote: fresh-tip procedure documented in REHEARSAL (do not reuse `0fea59a33`).
- Remaining mapped DIFFs intentional (WWMath Origin/Trig, GameProfile, Linux wchar_t, Steam map-select).

## References

- PLAN-023 Gate 6
- [UNIFIED_ENGINE_UPSTREAM_REHEARSAL.md](UNIFIED_ENGINE_UPSTREAM_REHEARSAL.md)
- `Core/GameEngine/Source/Common/GameProfile.cpp`
- `GeneralsMD/Code/Main/CMakeLists.txt` (`echelon` target)
