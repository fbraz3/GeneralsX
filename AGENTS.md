# GeneralsX: Instructions for AI Coding Agents

## What I Am
GeneralsX is a cross-platform port of Command & Conquer: Generals Zero Hour for **Linux and macOS**, porting legacy Windows DirectX 8 + Miles Sound code to a modern stack (SDL3 + DXVK + OpenAL + 64-bit). This is a **massive C++ game engine** (~500k LOC) preserving retail gameplay while modernizing the platform layer.

## Must-Load Context
Before starting work, read:
- `.github/copilot-instructions.md` – quick reference
- `.github/instructions/generalsx.instructions.md` – full architecture
- `.github/instructions/git-commit.instructions.md` – commit standards
- `.github/instructions/docs.instructions.md` – documentation workflow
- `docs/DEV_BLOG/YYYY-MM-DIARY.md` – current development notes

## Platform Focus
- **Active**: Linux (`linux64-deploy`), macOS (`macos-vulkan`)
- **Future/Exploratory**: Windows (MinGW path, issue #29)
- **Legacy**: VC6 + DirectX 8 + Miles (reference only)

## Architecture
| Layer   | Technology          | Replaces                     |
|---------|---------------------|------------------------------|
| Graphics| DXVK                | DirectX 8 (d3d8.dll)         |
| Windowing| SDL3              | Win32 API                    |
| Audio   | OpenAL              | Miles Sound System           |
| Video   | FFmpeg              | Bink Video (intro/videos)    |
| Platform| SDL3 + libc         | Win32 POSIX calls            |

**CRITICAL**: Platform code must be isolated to `Core/GameEngineDevice/` and `Core/Libraries/Source/Platform/`. No native Win32/Cocoa/X11 calls in game logic.

## Golden Rules
1. **Single codebase** – Linux and macOS build from same source
2. **SDL3 everywhere** – No native platform calls in game code
3. **DXVK everywhere** – DX8 → Vulkan translation on all platforms
4. **OpenAL everywhere** – Cross-platform audio stack
5. **64-bit native** – x86_64 only (32-bit via VC6 upstream)
6. **Retail compatibility** – Original replays and mods must work
7. **Determinism** – Rendering/audio changes must not affect gameplay logic
8. **No band-aids** – Fix underlying issues, not symptoms
9. **Update dev blog** – `docs/DEV_BLOG/YYYY-MM-DIARY.md` before committing
10. **Reference repos** – Study patterns, don't copy-paste

## Reference Repositories
- **fighter19-dxvk-port** – Primary graphics/platform reference (DXVK + SDL3 on Linux)
- **jmarshall-win64-modern** – Audio reference (OpenAL implementation, Generals-only)
- **thesuperhackers-main** – Upstream baseline for regression checks

## Build Commands

### Docker (recommended on Linux host)
```bash
# Linux build
./scripts/build/linux/docker-configure-linux.sh linux64-deploy
./scripts/build/linux/docker-build-linux-zh.sh linux64-deploy

# Optional: Windows via MinGW cross-build
./scripts/build/linux/docker-build-mingw-zh.sh mingw-w64-i686
```

### Native Linux
```bash
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals
```

### Native macOS
```bash
cmake --preset macos-vulkan
cmake --build build/macos-vulkan --target z_generals
```

## Target Priority
1. **GeneralsXZH** (Zero Hour) – Primary target, most feature-complete
2. **GeneralsX** (Base game) – Backport only when changes are clearly shared

## Backport Rules
**Backport to Generals when:**
- Change is platform/backend code (SDL3, DXVK, OpenAL)
- Change is in shared Core libraries
- Change is low-risk and clearly applicable

**Do NOT backport:**
- Zero Hour-specific gameplay/logic
- Expansion-specific features
- High-risk changes to Zero Hour

## DXVK Source of Truth (macOS)
- Default: GitHub fork branch `generalsx-macos-v2.6` (auto-update enabled)
- Local mode: `-DSAGE_DXVK_USE_LOCAL_FORK=ON`
- **Rule**: Never edit files in `build/_deps/...` directly. Always commit fixes in fork repo first.

## Common Pitfalls
- **Linux case sensitivity**: Include paths must match exact case. Use `scripts/tooling/cpp/fixIncludesCase.sh`.
- **DXVK needs Vulkan**: Install `vulkan-tools`, `mesa-vulkan-drivers` or GPU drivers.
- **-logToCon only in debug**: Available only with `RTS_BUILD_OPTION_DEBUG=ON`.
- **SDL3 from source**: Fetched via CMake FetchContent. No system package needed.
- **Manual memory**: Always delete/delete[]. Use STLPort for VC6 legacy builds.
- **Debug options break replays**: Use `RTS_BUILD_OPTION_DEBUG=OFF` for replay tests.

## Testing & Validation
### Smoke test
```bash
./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy
```

### Replay testing
```bash
cd ~/GeneralsX/GeneralsMD
./run.sh -win -logToCon 2>&1 | grep -v "D3DRS_PATCHSEGMENTS" | tee ~/GeneralsX/logs/manual_run.log
```

### Debug GDB
```bash
mkdir -p logs && gdb -batch -ex "run -win" -ex "bt full" -ex "thread apply all bt" \
  ./build/linux64-deploy/GeneralsMD/GeneralsXZH 2>&1 | tee logs/gdb.log
```

## Important Commands
```bash
# Linux deployment
./scripts/build/linux/deploy-linux-zh.sh
./scripts/build/linux/run-linux-zh.sh -win

# macOS workflow
./scripts/build/macos/build-macos-zh.sh
./scripts/build/macos/deploy-macos-zh.sh
./scripts/build/macos/run-macos-zh.sh -win

# VS Code tasks recommended
# Linux: [Linux] Configure (Docker), [Linux] Build GeneralsXZH, [Linux] Run GeneralsXZH
# macOS: [macOS] Configure, [macOS] Build GeneralsXZH, [macOS] Run GeneralsXZH
```

## Branching & Sync
### TheSuperHackers upstream sync
```bash
git remote add thesuperhackers git@github.com:TheSuperHackers/GeneralsGameCode.git
git fetch thesuperhackers
git merge thesuperhackers/main
```

**Conflict resolution**:
- Platform code (`Core/GameEngineDevice/`): keep ours
- Game logic (`GeneralsMD/Code/GameEngine/`): keep theirs
- Build system: merge carefully, test both versions

## Code Conventions
- **Annotate changes**: `// GeneralsX @keyword author DD/MM/YYYY Description`
- **Keywords**: `@bugfix` / `@feature` / `@performance` / `@refactor` / `@tweak` / `@build`
- **Attribution**: Add upstream PR references with author and GitHub URL
- **English only**: All code, comments, documentation
- **No lazy code**: No empty stubs, empty catch blocks, or commented-out code

## GitHub PR/Issue Formatting
- Use `--body-file` with real Markdown file instead of `--body`
- Avoid literal `\n` sequences; prefer actual newlines in multi-line strings

## VS Code Tasks
- Prefer task-first execution for build/test/debug
- Logs captured to `logs/` directory
- Primary labels: `[Linux]`, `[macOS]`, `[Linux] Pipeline: Build + Deploy + Run ZH`

## Docs Workflow
1. Monthly diary in `docs/DEV_BLOG/YYYY-MM-DIARY.md` (YYYY=year, MM=month only, e.g., `2026-05-DIARY.md`)
2. Active work notes in `docs/WORKDIR/` (phases/planning/reports/support/audit/lessons)
3. Never drop working docs directly under `docs/` root

## GitHub CLI Examples
**Create issues:**
```bash
gh issue create \
  --title "Brief, actionable title" \
  --body "## Context\n...\n## Goal\n...\n## Acceptance Criteria\n..." \
  --label bug --label Linux
```

**Create PRs (use temp file for body):**
```bash
cat > /tmp/pr-body.md << 'EOF'
## Description
Fixes #123

## Changes
- Platform isolation
EOF
gh pr create --title "Description" --body-file /tmp/pr-body.md
```

**Verify PR body (check for literal \n):**
```bash
body=$(gh pr view <number> --json body --jq .body)
printf "%s" "$body" | rg '\\n' && echo "HAS_LITERAL_BACKSLASH_N=YES" || echo "HAS_LITERAL_BACKSLASH_N=NO"
```

## Build Presets Reference
- **linux64-deploy** – GCC/Clang x86_64, Release (PRIMARY LINUX)
- **linux64-testing** – Debug variant
- **macos-vulkan** – macOS ARM64, RelWithDebInfo (PRIMARY MACOS)
- **mingw-w64-i686** – MinGW cross-compile (exploratory)
- **vc6** – Visual Studio 6, 32-bit (legacy)
- **win32** – MSVC 2022, experimental

## Directories
- `GeneralsMD/` – Zero Hour (PRIMARY TARGET)
- `Generals/` – Base game
- `Core/` – Shared libraries (`GameEngine/`, `GameEngineDevice/`, `Libraries/`)
- `references/` – fighter19, jmarshall, thesuperhackers-main
- `docs/WORKDIR/` – current work documentation
- `logs/` – build/run/debug logs (gitignored)
