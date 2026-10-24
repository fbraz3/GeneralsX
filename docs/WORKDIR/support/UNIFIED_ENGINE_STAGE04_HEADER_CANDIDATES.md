# Unified Engine Stage 04: Header Candidates (License-Only)

Date: 2026-10-23
Scope: inventory of safe Generals ↔ Zero Hour header pairs for later Core
promotion. No Generals headers were deleted in this pass.

## Pair scan (no Tools/, no GeneralsOnline)

Paired paths under `Generals/Code` and `GeneralsMD/Code`, excluding `Tools/`
and any path containing `GeneralsOnline`.

License-only means byte-identical after normalizing the banner product title
`Command & Conquer Generals(tm)` ↔ `Command & Conquer Generals Zero Hour(tm)`.

| Bucket | Count |
|---|---|
| License-only headers | **221** |
| Content-diff headers | 154 |
| Content-diff sources (`.cpp`/`.c`/`.inl`) | 228 |
| Further-diff headers+sources (blocker pool) | **382** |
| License-only sources remaining | 5 |

## Why Include-tree merge is blocked

Adding `GeneralsMD/Code/GameEngine/Include` to Generals (`g_gameengine`) include
paths pulls Zero Hour NGMP headers (`GameNetwork/GeneralsOnline/...`). The
Generals link then fails on unresolved NGMP symbols. Relative one-line redirect
wrappers from Generals Include → GeneralsMD are also fragile (depth-dependent
paths, and any wide Include retarget reintroduces NGMP).

Keep separate Generals and Zero Hour `GameEngine/Include` trees until headers
are promoted deliberately into `Core/` (or otherwise reconciled without exposing
the full ZH Include tree to Generals).

## GameEngineDevice / Libraries: no deletes this pass

**15** license-only headers remain under `GameEngineDevice/Include`, and **13**
under `Libraries/Source/WWVegas/WW3D2`. Generals already compiles some matching
ZH `.cpp` units (Stage 01 provenance), but:

- Generals `g_gameenginedevice` only publishes its own `Include/`
- Those W3D headers are **not** present under `Core/GameEngineDevice/Include`
- Generals does **not** search `GeneralsMD/Code/GameEngineDevice/Include`

Deleting the Generals copies would break the Generals build unless each header
is first promoted to Core (or an equivalent narrow include path is added). That
promotion is out of scope for this inventory pass.

## Top 40 safest `GameEngine/Include` candidates

Selection: license-only under `GameEngine/Include`, verified body match after
license-title substitution. Prefer pure enum/flag/constant / non-layout headers
first (24 such), then fill to 40 with the smallest remaining license-only
headers (still layout-bearing class stubs, but body-identical to ZH).

Size is the Generals file size in bytes. Paths are relative to
`Generals/Code/` (mirror under `GeneralsMD/Code/`).

| # | Size | Kind | Path |
|---|---:|---|---|
| 1 | 1248 | enum/const | `GameEngine/Include/GameLogic/WeaponBonusConditionFlags.h` |
| 2 | 1315 | enum/const | `GameEngine/Include/GameLogic/WeaponSetFlags.h` |
| 3 | 1343 | enum/const | `GameEngine/Include/GameLogic/WeaponStatus.h` |
| 4 | 1390 | enum/const | `GameEngine/Include/Common/SpecialPowerMaskType.h` |
| 5 | 1402 | enum/const | `GameEngine/Include/GameClient/FontDesc.h` |
| 6 | 1443 | enum/const | `GameEngine/Include/Common/encrypt.h` |
| 7 | 1499 | enum/const | `GameEngine/Include/Common/PerfMetrics.h` |
| 8 | 1568 | enum/const | `GameEngine/Include/Common/QuotedPrintable.h` |
| 9 | 1643 | enum/const | `GameEngine/Include/Common/Terrain.h` |
| 10 | 1771 | enum/const | `GameEngine/Include/GameClient/GameInfoWindow.h` |
| 11 | 1774 | enum/const | `GameEngine/Include/Common/INIException.h` |
| 12 | 1825 | enum/const | `GameEngine/Include/GameNetwork/GameSpyGP.h` |
| 13 | 1856 | enum/const | `GameEngine/Include/Common/Registry.h` |
| 14 | 1878 | enum/const | `GameEngine/Include/GameLogic/Powers.h` |
| 15 | 2030 | enum/const | `GameEngine/Include/GameLogic/ObjectScriptStatusBits.h` |
| 16 | 2307 | enum/const | `GameEngine/Include/Common/OSDisplay.h` |
| 17 | 2460 | enum/const | `GameEngine/Include/GameNetwork/GUIUtil.h` |
| 18 | 2476 | enum/const | `GameEngine/Include/Common/ThingSort.h` |
| 19 | 2642 | enum/const | `GameEngine/Include/GameNetwork/GameSpyChat.h` |
| 20 | 2793 | enum/const | `GameEngine/Include/Common/StackDump.h` |
| 21 | 4379 | enum/const | `GameEngine/Include/Common/DisabledTypes.h` |
| 22 | 6377 | enum/const | `GameEngine/Include/Common/BitFlagsIO.h` |
| 23 | 8776 | enum/const | `GameEngine/Include/GameClient/KeyDefs.h` |
| 24 | 14008 | enum/const | `GameEngine/Include/Common/KindOf.h` |
| 25 | 1204 | layout-stub | `GameEngine/Include/Common/BorderColors.h` |
| 26 | 1282 | layout-stub | `GameEngine/Include/Common/ScopedMutex.h` |
| 27 | 1955 | layout-stub | `GameEngine/Include/GameLogic/Module/DestroyDie.h` |
| 28 | 2055 | layout-stub | `GameEngine/Include/GameLogic/Module/WanderAIUpdate.h` |
| 29 | 2145 | layout-stub | `GameEngine/Include/GameLogic/Module/ObjectHelper.h` |
| 30 | 2160 | layout-stub | `GameEngine/Include/Common/GameSpyMiscPreferences.h` |
| 31 | 2162 | layout-stub | `GameEngine/Include/GameLogic/Module/PreorderCreate.h` |
| 32 | 2192 | layout-stub | `GameEngine/Include/GameLogic/Module/SupplyWarehouseCreate.h` |
| 33 | 2234 | layout-stub | `GameEngine/Include/GameClient/DrawableInfo.h` |
| 34 | 2234 | layout-stub | `GameEngine/Include/GameLogic/Module/HealCrateCollide.h` |
| 35 | 2245 | layout-stub | `GameEngine/Include/GameLogic/Module/ImmortalBody.h` |
| 36 | 2245 | layout-stub | `GameEngine/Include/GameLogic/Module/ShroudCrateCollide.h` |
| 37 | 2250 | layout-stub | `GameEngine/Include/GameLogic/Module/SpecialPowerCreate.h` |
| 38 | 2253 | layout-stub | `GameEngine/Include/GameLogic/Module/PrisonDockUpdate.h` |
| 39 | 2269 | layout-stub | `GameEngine/Include/GameLogic/Module/DestroyModule.h` |
| 40 | 2269 | layout-stub | `GameEngine/Include/GameLogic/Module/SupplyCenterCreate.h` |

Total license-only under `GameEngine/Include`: **195** (24 enum/const-ish above;
171 still have class/layout surface but license-identical bodies).

## Suggested next leverage

Do not collapse Include trees. Prefer Core promotion of the enum/const set
above, then module stubs whose `.cpp` units are already Stage 04 ZH-canonical.
For the **382** further-diff pairs, next high-value reconciliation is Player /
Object xfer and related serialization layout — not blanket header redirects.
