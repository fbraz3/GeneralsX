# Echelon

**A unified game engine with an integrated launcher.**

Echelon brings Command & Conquer: Generals and Zero Hour into one executable
with game selection at startup and the original rules of each game. It builds
on the Zero Hour engine from [GeneralsX](https://github.com/fbraz3/GeneralsX),
retaining a traceable path for upstream updates.

The project is in development. The single executable and integrated launcher
are the target architecture; the current build still produces separate
Generals and Zero Hour executables.

## Current progress

- Zero Hour is the canonical foundation; Generals will use an explicit
  compatibility profile for its original rules, assets, saves and replays.
- Initial consolidation shares FPU declarations and three Zero Hour device
  implementations between the existing builds.
- A source provenance map and checker track both original game paths so
  future upstream changes remain visible after consolidation.
- Both Linux executables have built and passed headless initialization and
  shutdown checks before and after this initial consolidation. Gameplay,
  save/replay compatibility and macOS validation remain open migration gates.

See the [unified engine plan](docs/WORKDIR/planning/PLAN-023_UNIFIED_ENGINE.md)
and [validated Stage 01 work](docs/WORKDIR/support/UNIFIED_ENGINE_STAGE01.md)
for scope, evidence and remaining work.

## Engine foundation

The inherited platform stack uses SDL3 for windowing and input, DXVK for
DirectX 8 to Vulkan translation, MiniAudio for audio, and FFmpeg for video.
Linux and macOS are the active platform targets. Online work targets
GeneralsOnline (NGMP).

Deterministic simulation and original game compatibility are required during
consolidation. The launcher will select a game profile before mounting assets
or initializing the engine.

Original game data is required and is not distributed with Echelon.

## Building during migration

Existing target names and source paths are retained while the engine is
consolidated:

```bash
# Linux (x86_64)
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target g_generals z_generals

# macOS (Apple Silicon)
cmake --preset macos-vulkan
cmake --build build/macos-vulkan --target g_generals z_generals
```

For prerequisites and platform setup, consult the upstream GeneralsX
[Linux guide](https://github.com/fbraz3/GeneralsX/wiki/Building-on-Linux) or
[macOS guide](https://github.com/fbraz3/GeneralsX/wiki/Building-on-macOS).
These guides and upstream release packages use GeneralsX branding.
Echelon does not yet have its own release packages.

## Lineage and development

Echelon is an independent continuation of GeneralsX. Its name covers both the
engine and the integrated launcher. The earlier Echelon prototype is retained
separately as a source of ideas; its code has not been imported into this fork.

Read [UPSTREAM.md](UPSTREAM.md) for fork identity and update policy, and
[CONTRIBUTING.md](CONTRIBUTING.md) for development guidance.

Credits include Westwood Studios and Electronic Arts for the original games,
[TheSuperHackers](https://github.com/TheSuperHackers/GeneralsGameCode) for
game-code modernization and preservation,
[fbraz3 and GeneralsX contributors](https://github.com/fbraz3/GeneralsX) for the
cross-platform port, and [Fighter19](https://github.com/Fighter19) and
[feliwir](https://github.com/feliwir) for earlier platform reference work.
Existing licenses and source attributions are preserved.

See [LICENSE.md](LICENSE.md). Electronic Arts has not endorsed and does not
support this product. Command & Conquer is a trademark of Electronic Arts Inc.
