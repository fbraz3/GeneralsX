# PLAN-023 Unified Engine with Runtime Game Selection

Date: 2026-10-23
Status: Architecture agreed; source comparison completed; implementation pending

## Product contract

The user selected one executable with startup selection of Generals or Zero
Hour. Each game retains its original rules. A launcher that delegates to two
engine executables does not meet this goal.

Both games must retain campaigns, skirmish, LAN, mods, saves and replay
compatibility. Online work targets NGMP. Selecting a game must happen before
mounting its assets or constructing engine subsystems. Switching games during
a running session is outside the initial scope.

## Verified starting point

At main commit afb09f144:

- Generals/Code and GeneralsMD/Code contain 1,014 and 1,148 tracked C/C++
  source/header files respectively.
- 1,006 relative paths occur in both trees; 27 are byte-identical and 979
  differ. Of those 979, 536 differ only in the opening license game title;
  443 have further differences. These are file counts, not semantic difference
  measurements, and include development tools.
- There are 8 Generals-only and 142 Zero Hour-only paths.
- gi_always and zi_always define RTS_GENERALS and RTS_ZEROHOUR respectively.
- corei_gameengine_private is an INTERFACE target with INTERFACE sources;
  shared source location does not imply a single compiled engine library.
- Game-specific include paths precede Core headers. Header and class layout
  differences must be examined before moving translation units into a shared
  compiled target.

See [the source comparison](../audit/UNIFIED_ENGINE_SOURCE_COMPARISON_2026-10-23.md)
for reproducible counts, concrete save/layout/rules differences and initial
deduplication candidates. In particular, identical NameKeyGenerator.cpp files
consume different class layouts, and Player save version 8 has different fields
in the two games. Share canonical sources before sharing compiled objects.

## Target architecture

One application initializes an immutable game profile, then starts one engine.
The profile selects asset roots, original game/version identity, user-data
paths and an explicit rules policy. Selection may be exposed through a startup
chooser and a command-line option; final option spelling is not yet fixed.

Platform devices, rendering, audio, video, filesystem and engine infrastructure
have one implementation. Genuine gameplay differences live in named rules
policies or game-specific modules. A base-game profile must not silently acquire
Zero Hour behavior merely because Zero Hour code is newer.

Keep original serialized numeric identifiers and formats. Choose serializers
and replay behavior from the selected profile, rather than renumbering shared
enums or assuming that matching names imply compatible layouts. NGMP/LAN must
reject incompatible game profiles before a match begins.

## Migration gates

1. **Baseline and inventory.** Record reproducible builds and representative
   campaign, skirmish, save/load and replay checks for both existing engines.
   Inventory active source lists, game macros, include shadowing, global
   singletons, serializers and variant-specific modules. Distinguish cosmetic
   differences from behavior and data-layout differences.
2. **Shared platform implementation.** Consolidate CompatLib and SDL3/device
   implementations in the platform layer, in small validated changes. Retain
   both executable targets as migration checks. Do not share objects across
   incompatible game headers.
3. **Profile foundation.** Introduce game identity selected before subsystem
   initialization, initially through the existing entry points. Route asset
   discovery and user-data selection through it. Keep profile choice fixed
   throughout the process lifetime.
4. **Shared engine contracts.** Reconcile duplicate headers and definitions
   subsystem by subsystem. Replace game compile-time branches with explicit
   profile policies where required. Audit class layouts and global ownership
   before linking formerly separate code into one target.
5. **Rules and serialization.** Preserve intentional base/expansion behavior
   behind explicit policies, including modules, commands, save versions,
   replay input and network compatibility. Compare deterministic simulation
   results against each game's baseline, including Linux/macOS cross-play.
6. **One executable.** Link the reconciled engine once, add startup selection,
   adapt packaging and smoke/replay automation. Remove redundant executable
   targets and source copies only after both profiles pass the gates.

## Validation and completion

For each migrated subsystem build both games on Linux; validate macOS through
the existing supported workflow. Run relevant runtime checks for each profile.
Retain deterministic math, FPU guards and disabled FMA contraction. Replay
checks use RTS_BUILD_OPTION_DEBUG=OFF. Use deep CRC diagnostics for divergences.

Completion requires one executable running both profiles, independent asset
and user-data selection, preserved original rules, working campaigns/skirmish/
LAN and supported mods, save/replay compatibility, and no duplicated general
backend implementations. A successful compile alone is insufficient.

## Current limitations

No baseline builds or runtime checks were performed during this architecture
survey. Serena C++ symbol navigation failed during initialization in the
preceding survey; repair it before relying on semantic refactoring tools.
Migration gates above remain uncompleted.
