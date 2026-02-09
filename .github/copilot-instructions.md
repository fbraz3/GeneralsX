# AI Coding Agent Quickstart

- **Scope**: Two games; Zero Hour lives in `GeneralsMD/` (primary), Generals in `Generals/`; shared engine/libs under `Core/`.
- **Platform strategy**: Keep Windows VC6 + win32 builds intact. Linux work = DXVK + SDL3 (graphics/window/input) and future OpenAL audio; isolate platform code to `Core/GameEngineDevice/` and `Core/Libraries/Source/Platform/`.
- **Key entry points**: Game launchers in `GeneralsMD/Code/Main/WinMain.cpp` and `Generals/Code/Main/WinMain.cpp`. Renderer device setup in `Core/GameEngineDevice/Source/` (DX8 now; DXVK path follows fighter19 reference under `references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/`).
- **Critical convention**: Every user-facing code change needs `// TheSuperHackers @keyword author DD/MM/YYYY Description` above it. Keywords: @bugfix/@feature/@performance/@refactor/@tweak/@build.
- **Build presets**: `cmake --preset vc6` or `win32`; Linux native via Docker `./scripts/docker-configure-linux.sh linux64-deploy` then `./scripts/docker-build-linux-zh.sh linux64-deploy`; MinGW cross Windows exe via `./scripts/docker-build-mingw-zh.sh mingw-w64-i686`.
- **Testing hotspots**: Replay compatibility uses VC6 optimized builds with `RTS_BUILD_OPTION_DEBUG=OFF` and replays in `GeneralsReplays/`; run via `generalszh.exe -jobs 4 -headless -replay subfolder/*.rep`. Keep determinism—avoid logic changes when touching rendering/audio paths.
- **Platform isolation rules**: No Linux-specific code inside gameplay (GameLogic). Use compile guards and device/platform layers. Keep DX8 path working for Windows; add DXVK/OpenAL behind feature flags.
- **Reference guides**: DXVK patterns in `references/fighter19-dxvk-port/` (CMake presets, SDL3 hooks, device wrappers). OpenAL/Miles mapping ideas in `references/jmarshall-win64-modern/Code/Audio/` (Generals-only, adapt carefully for Zero Hour).
- **Docs workflow**: Monthly diary in `docs/DEV_BLOG/YYYY-MM-DIARY.md` (newest-first). Active work notes in `docs/WORKDIR/` (phases/planning/reports/support/audit/lessons). Do not drop working docs directly under `docs/` root.
- **Common pitfalls**: Manual memory (delete/delete[]; STLPort for VC6). Retail compatibility matters—debug options break replays. Watch include-case on Linux; scripts/cpp/fixIncludesCase.sh can help. Avoid big refactors mixed with gameplay fixes.
- **Where to tweak build flags**: `CMakePresets.json` for presets; `cmake/config-build.cmake` and `cmake/dx8.cmake` for renderer flags; `cmake/miles.cmake` for audio; `cmake/mingw.cmake` for cross builds.
- **Run recipes**: Linux binary smoke: `./scripts/docker-smoke-test-zh.sh linux64-deploy`. Quick Windows run (win32 preset): see tasks using `scripts/run_zh.ps1`/`scripts/run_zh.ps1 -Generals`.
- **When backporting to Generals**: Only for shared platform/back-end changes; avoid expansion-specific logic moves. Keep Zero Hour first.
