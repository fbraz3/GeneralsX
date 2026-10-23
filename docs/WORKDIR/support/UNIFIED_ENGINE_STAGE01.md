# Unified Engine Stage 01: Consolidation Infrastructure

Date: 2026-10-23
Scope: shared FPU declarations, three canonical Zero Hour sources, and traceable
source consolidation.

The user selected Zero Hour as the engine foundation during this stage.
Shared platform declarations may stay in Core; remaining game implementation
consolidation should retain Zero Hour paths and extract Generals compatibility
behavior explicitly.

## Design

The first canonical header is Core/GameEngine/Include/GameLogic/FPUControl.h.
Existing include spelling, setFPMode implementations and call sites remain
unchanged. Both game builds resolve the declarations through their existing
Core include directories. This changes source ownership, not FPU behavior.

Generals now compiles the canonical Zero Hour W3DAssetManagerExposed.cpp,
W3DDynamicLight.cpp and W3DInGameUI.cpp, replacing its duplicate copies. These
three implementations were verified byte-for-byte equal after substituting
only the opening license game title. They remain separately compiled against
each game's headers; this does not yet introduce a single compiled engine.

`cmake/unified-engine-provenance.json` records both original source paths,
the canonical destination, the source baseline and an upstream ancestor.
Extend this map with every later consolidation. The upstream ancestor is a
recorded integration reference; it is not automatically advanced by the checker.

The checker validates canonical files and rejects reintroduced old copies.
Its optional range report handles both original paths and their canonical
destination, including upstream deletions and renames. Unmapped changed paths
are listed separately and still need normal upstream review. It neither applies
patches nor certifies that incoming changes have been semantically integrated.

## Commands

```bash
python3 scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py
python3 -m unittest discover -s scripts/qa -p test_unified_engine_provenance.py
python3 scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py \
  --upstream-range 78d94fb33^..78d94fb33 --fail-on-mapped-change
```

The historical range is a real upstream refactoring that touched both original
FPUControl headers and both copies of the three W3D sources. All four canonical
destinations are reported from changes in both game trees. Exit 2 means these
mapped changes require explicit review;
it is not a failing structural map. This detection exercise does not complete
the later full upstream merge rehearsal required by PLAN-023.

CI checks the map and Git fixture tests with full history available. A VS Code
task, `[Engine] Check consolidation provenance`, runs the structural check.

## Local validation environment

Reused the existing Distrobox container `ubuntu-dev` (official Ubuntu 24.04).
No development packages were installed on the host. The following packages
were added inside the container:

```bash
apt-get update
apt-get install -y --no-install-recommends \
  libfreetype-dev libfontconfig1-dev libcurl4-openssl-dev libglm-dev libpng-dev \
  libasound2-dev libpulse-dev libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev \
  libxkbcommon-dev libwayland-dev libdecor-0-dev libudev-dev libdrm-dev libgbm-dev \
  libegl1-mesa-dev libfribidi-dev libthai-dev libprotobuf-dev protobuf-compiler libssl-dev
```

Ubuntu 24.04 has no libgli-dev candidate. GLI was checked out at immutable commit
779b99ac6656e4d30c3b24e96e0136a59649a869 from g-truc/gli under
`~/.local/share/toolchains/generalsx-deps/gli`, configured with
`-DGLI_TEST_ENABLE=OFF` and installed to the sibling `install` directory.
This is a local system-dependency validation configuration, not the project's
pinned vcpkg release environment; no dependency lock files were changed.

Configuration, inside the container from the repository root:

```bash
cmake --preset linux64-deploy -B build/linux64-unified-baseline \
  -DCMAKE_TOOLCHAIN_FILE= \
  -DCMAKE_PREFIX_PATH="$HOME/.local/share/toolchains/generalsx-deps/install" \
  -DFETCHCONTENT_SOURCE_DIR_GAMENETWORKINGSOCKETS="$HOME/.local/share/toolchains/generalsx-deps/GameNetworkingSockets" \
  -DRTS_BUILD_CORE_TOOLS=OFF -DRTS_BUILD_GENERALS_TOOLS=OFF \
  -DRTS_BUILD_ZEROHOUR_TOOLS=OFF -DSAGE_USE_NGMP=ON
ionice -c 3 nice -n 19 cmake --build build/linux64-unified-baseline \
  --target g_generals z_generals --parallel 6
```

Use the user's actual home path for CMAKE_PREFIX_PATH when executing through
`podman exec`, where the container's HOME may refer to root. MiniAudio, FFmpeg,
deterministic GameMath and release replay settings remain enabled. Logs reside
in `logs/unified-stage1-*`.

The initial NGMP=OFF baseline compiled the Zero Hour libraries but failed to
link its executable: existing menu code references online services even with
NGMP disabled. The validation configuration was changed to the standard ON
variant before any engine source move.

The default GameNetworkingSockets fetch recursively downloads an unused WebRTC
submodule despite USE_STEAMWEBRTC=OFF. That configure attempt was stopped by
verified process ID. A non-recursive checkout of the same v1.6.0 dependency
(commit 2cb93a06350bb065db53abdb0d87cf297e0bfd34) was placed under the toolchain
directory, and the existing project `gns_ice_thinker_loop.patch` was applied
there. FETCHCONTENT_SOURCE_DIR_GAMENETWORKINGSOCKETS selects that local source.
No fetched dependency sources under build/_deps were edited.

## Retail data

A separately delegated copy task prepared both installed Steam game datasets
without changing Steam or the repository. Runtime assets are under
`~/.local/share/generalsx/game-data/Generals` and `ZeroHour`; use these as
CNC_GENERALS_PATH and CNC_GENERALS_ZH_PATH respectively. The external
asset-copy-manifest.txt records 137 Generals files (1,645,678,431 bytes) and
162 Zero Hour files (2,532,448,169 bytes), all verified against the source by
SHA-256. Assets are not Git inputs or distributable engine artifacts.

## Validation results

- Linux baseline builds of g_generals and z_generals completed successfully with
  NGMP enabled before applying source consolidation.
- Both targets rebuilt successfully after consolidation (30 incremental steps).
- Baseline and consolidated executables for both games initialized with the
  prepared Steam assets in headless mode and exited with status 0. Isolated
  XDG_DATA_HOME directories kept smoke-test state separate from user data.
- The smoke command used `-headless -replay __unified_smoke_no_matches_*.rep`:
  an intentionally unmatched pattern exercises initialization and shutdown,
  not a real replay or a gameplay session.
- All six compile contexts for the three canonical sources retain identical
  compiler arguments except source and output paths. Each game's headers and
  compile definitions remain unchanged.
- FPU header preprocessor output before and after relocation is identical for
  both game include contexts.
- The structural provenance check passes with four entries. Seven isolated Git
  fixture tests pass, including changes from both original paths, incoming
  renames, invalid maps and the review-required CLI exit code.
- Historical upstream range 78d94fb33^..78d94fb33 reports all four canonical
  destinations and both game paths for each, returning the expected status 2.

Both baseline and consolidated headless runs warn about unavailable Font Awesome
fonts. No visual UI assessment was performed. macOS, campaigns, skirmish,
save/load, actual replay CRCs and online behavior remain unvalidated. The overall
single-executable and upstream merge rehearsal gates are not completed.
