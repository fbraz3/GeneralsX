# Unified Engine Source Comparison

Date: 2026-10-23
Source baseline: afb09f144 (engine sources unchanged at 9db5cf30a)
Scope: tracked C/C++ files in Generals/Code and GeneralsMD/Code, including tools.

## Conclusion

Substantial source deduplication is possible before changing game rules.
However, identical translation-unit text is not proof that its compiled objects
can be shared. Game-specific headers, preprocessor definitions, initialization
order and serializers are part of the engine contract.

Use one canonical source tree first, while temporarily compiling it separately
for each existing game target. Reconcile type contracts and runtime policies
before replacing these builds with one compiled engine and one executable.
This intermediate step is not the final user-visible goal.

## Exact comparison results

The comparison preserves every byte except, in the second category, the first
occurrence of `Command & Conquer Generals(tm)` being replaced with
`Command & Conquer Generals Zero Hour(tm)` in the opening license banner.
No comments, whitespace, literals or preprocessor branches are otherwise removed.

| Area | Paired files | Byte-identical | License title only | Further differences |
|---|---:|---:|---:|---:|
| CompatLib/Include | 27 | 27 | 0 | 0 |
| GameEngine/Include | 314 | 0 | 192 | 122 |
| GameEngine/Source | 314 | 0 | 141 | 173 |
| GameEngineDevice/Include | 27 | 0 | 15 | 12 |
| GameEngineDevice/Source | 25 | 0 | 4 | 21 |
| Libraries/Source | 66 | 0 | 15 | 51 |
| Main | 5 | 0 | 1 | 4 |
| Tools/GUIEdit | 35 | 0 | 26 | 9 |
| Tools/WorldBuilder | 193 | 0 | 142 | 51 |
| **Total** | **1,006** | **27** | **536** | **443** |

There are 8 Generals-only and 142 Zero Hour-only paths, for 1,156 unique
relative paths. Excluding Tools leaves 778 paired files: 27 byte-identical,
368 license-title-only and 383 with further differences. These counts describe
tracked files, not active Linux compilation units or semantic differences.
The 443 remainder can still include cosmetic changes.

Reproduction from repository root (Python standard library only):

```python
from pathlib import Path
from collections import Counter
import subprocess

files = subprocess.check_output(['git', 'ls-files'], text=True).splitlines()
def inventory(prefix):
    return {p[len(prefix):]: Path(p) for p in files
            if p.startswith(prefix) and p.endswith(('.cpp', '.h', '.hpp', '.c'))}
a = inventory('Generals/Code/')
b = inventory('GeneralsMD/Code/')
counts = Counter()
for p in a.keys() & b.keys():
    x, y = a[p].read_bytes(), b[p].read_bytes()
    if x == y:
        counts['identical'] += 1
    elif x.replace(b'Command & Conquer Generals(tm)',
                   b'Command & Conquer Generals Zero Hour(tm)', 1) == y:
        counts['license_title_only'] += 1
    else:
        counts['further_differences'] += 1
print(dict(counts))
print('Generals only:', len(a.keys() - b.keys()))
print('Zero Hour only:', len(b.keys() - a.keys()))
```

## Verified migration constraints

Paths below are relative to the repository root. Line references apply to this
source baseline.

### Shared sources are compiled in different type contexts

`Core/GameEngine/CMakeLists.txt:1182` declares INTERFACE targets and propagates
GAMEENGINE_SRC via target_sources. Core/GameEngineDevice does the same.
`Generals/Code/CMakeLists.txt` and its GeneralsMD counterpart define
RTS_GENERALS=1 and RTS_ZEROHOUR=1 respectively, with game headers ordered before
Core headers. Merely linking g_gameengine and z_gameengine into one executable
does not reconcile their same-named classes and global engine state.

A textual preprocessor inventory finds game macro references in 50 Core files
(157 directive lines), 25 Generals files (41) and 25 GeneralsMD files (43).
This includes tools and legacy code and excludes non-directive references;
it is a discovery aid, not the complete runtime-policy migration workload.

### Identical .cpp files can have incompatible class layouts

`GameEngine/Source/Common/NameKeyGenerator.cpp` differs only in its license title.
But `GameEngine/Include/Common/NameKeyGenerator.h` defines SOCKET_COUNT as 6473
for Generals (line 122) and 45007 for Zero Hour (line 120). The value sizes the
member array m_sockets. A shared compiled object built against one header is
not interchangeable with callers built against the other.

The same source also guards syncNameKeyID with RTS_ZEROHOUR. GameEngine::init
checks expected name-key IDs 1586 for Generals and 2265 for Zero Hour in the
retail-compatible initialization paths. Preserve insertion order and existing
profile-specific CRC behavior when consolidating this subsystem.

### Equal save version numbers do not identify equal formats

`GameEngine/Source/Common/RTS/Player.cpp` uses currentVersion=8 in both trees
(Generals line 3877; Zero Hour line 4051). Generals serializes m_visionSpiedBy
and m_visionSpiedMask after m_attackedBy (lines 4220 and 4223); Zero Hour proceeds
from m_attackedBy to m_cashBountyPercent without those fields.

Player.h also differs in methods, return types and members, including the
Zero Hour m_baseSide field. Object.h adds Zero Hour sighting and helper state.
The unified loader needs game identity in addition to each object's version;
do not treat Player version 8 as a universal cross-game format.

`Object::xfer` selects currentVersion=7 for retail-compatible Generals saves,
8 for its alternative path, and 9 for Zero Hour. The latter serializes extra
sighting state under version >= 9. Keep these original contracts rather than
silently rewriting all saves to the expansion format.

### Rules differ inside existing modules, not just new expansion modules

`GameEngine/Source/GameLogic/Object/WeaponSet.cpp`, getVictimAntiMask, treats
KINDOF_MINE before missile/projectile cases in Generals. Zero Hour handles
missiles/projectiles first and then treats KINDOF_MINE or KINDOF_DEMOTRAP as
mine/ground targets. Its attack methods additionally accept a specific weapon
slot, and garrison range handling checks isEnclosingContainerFor.

These are policy candidates requiring baseline behavior checks. Loading the
Generals INI files into unmodified Zero Hour logic does not preserve every
Generals rule.

### Registrations must retain modules present only in the base game

Generals ModuleFactory.cpp registers DelayedWeaponSetUpgradeUpdate (line 354)
and DelayedUpgrade (435). Their implementations and headers exist only in its
tree and are listed in its GameEngine CMake source list.

Zero Hour adds CountermeasuresBehavior, GrantScienceUpgrade, HelixContain,
Spectre-related modules and others. Start with a union of implementations;
make registration and behavior explicit per profile where required. Do not
assume that the expansion tree is a strict superset of the base game.

### Enum insertion requires a compatibility audit

`GameEngine/Include/Common/SpecialPowerType.h` inserts SPECIAL_HELIX_NAPALM_BOMB
before SPECIAL_HACKER_DISABLE_BUILDING in Zero Hour (lines 74-75), and inserts
SPECIAL_BOOBY_TRAP later. Subsequent implicit numeric values therefore differ.
Trace serialization, CRC and command uses before selecting a common internal
representation; this inspection establishes numeric drift, not that every
enum value is directly stored in a retail packet or save.

### Replay behavior and game identity need a profile

Both Recorder.cpp implementations use the GENREP signature. Headers contain
version strings, version number, exeCRC and iniCRC. A common file signature is
not sufficient to select the ruleset.

Zero Hour updateRecord excludes GAME_SINGLE_PLAYER at line 459; Generals does
not have this exclusion. Preserve the existing distinction until a deliberate
behavior change is separately validated. Maintain profile-specific game/version
identity even when both games share one executable release version.

### Asset selection already branches inside Core

`Core/GameEngine/Source/Common/System/BIGFileSystem.cpp:345` selects the primary
environment variable through RTS_ZEROHOUR. Its init path additionally loads
base Generals assets for Zero Hour at line 533. StdLocalFileSystem.cpp:265 also
selects its environment fallback through the game macro.

The future runtime profile must select both the primary asset root and the
ordered base/expansion asset layers before filesystem initialization. Preserve
loose-file asset-root propagation as well as BIG archive discovery.

### NGMP integration is asymmetric

The GeneralsMD tree contains GeneralsOnline headers and implementations and
appends its online source files when SAGE_USE_NGMP is enabled. Generals has no
matching online implementation tree; its Recorder.cpp uses an additional
__has_include guard around the replay-upload integration.

The unified engine should share the actual online service implementation and
enforce profile compatibility. This source survey does not establish working
online multiplayer for the base game today. Legacy GameSpy parity is excluded.

## First consolidation candidates

| Candidate | Verified property | Remaining gate |
|---|---|---|
| FPUControl.h | License-title-only difference; declaration and ScopedFPUGuard definition match | Keep setFPMode implementation and all call boundaries intact |
| 27 CompatLib headers | Byte-identical | Audit include ordering; canonical platform location; keep legacy build guards |
| W3DAssetManagerExposed.cpp, W3DDynamicLight.cpp, W3DInGameUI.cpp, W3DGameLogic.cpp | License-title-only difference | Initially compile canonical source separately against each game's headers |
| NameKeyGenerator.cpp | License-title-only difference | Share source only until header layout and game-dependent init are reconciled |
| SDL3GameEngine.cpp | Mostly shared structure, but actual differences remain | Reconcile audio includes, allocation and fallback behavior first |

MiniAudio and FFmpeg backend implementations already reside under Core;
deduplication should target remaining initialization and device glue rather than
create another parallel backend layer.

## What this survey does not prove

No code was moved, no object layouts were measured with a compiler, no complete
CMake dependency graph was configured, and no build, game, save or replay was
run. File counts include unbuilt and legacy files. Further-difference files
were sampled in high-risk subsystems, not exhaustively reviewed line by line.
Serena semantic navigation was unavailable in the preceding survey; this pass
used exact file comparison and targeted source inspection.
