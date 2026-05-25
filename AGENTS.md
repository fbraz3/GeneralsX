# GeneralsX: Instructions for AI Coding Agents

Mandatory: always read and follow applicable `.github/instructions/*.instructions.md` files based on each file's `applyTo` pattern before making changes.
# GeneralsX: Instructions for AI Coding Agents

Mandatory: always read and follow applicable `.github/instructions/*.instructions.md` files based on each file's `applyTo` pattern before making changes.

## What I Am
GeneralsX is Linux/macOS port of Command & Conquer: Generals Zero Hour. Legacy DirectX 8 + Miles code move to SDL3 + DXVK + OpenAL + 64-bit while retail gameplay stay intact.

## Must-Load Context
Before work, read:
- `.github/instructions/generalsx.instructions.md`
- `.github/instructions/git-commit.instructions.md`
- `.github/instructions/docs.instructions.md`
- `.github/instructions/scripts.instructions.md`
- `docs/DEV_BLOG/YYYY-MM-DIARY.md`

## Key Entry Points
- `GeneralsMD/Code/Main/WinMain.cpp`
- `Generals/Code/Main/WinMain.cpp`
- `Core/GameEngineDevice/Source/`

## Platform Focus
- **Active**: Linux (`linux64-deploy`), macOS (`macos-vulkan`)
- **Future/Exploratory**: Windows (MinGW path, issue #29)
- **Legacy**: VC6 + DirectX 8 + Miles

## Architecture
| Layer   | Technology          | Replaces                     |
|---------|---------------------|------------------------------|
| Graphics| DXVK                | DirectX 8 (d3d8.dll)         |
| Windowing| SDL3              | Win32 API                    |
| Audio   | OpenAL              | Miles Sound System           |
| Video   | FFmpeg              | Bink Video (intro/videos)    |
| Platform| SDL3 + libc         | Win32 POSIX calls            |
**CRITICAL**: Platform code must be isolated to `Core/GameEngineDevice/` and `Core/Libraries/Source/Platform/`; no native Win32/Cocoa/X11 in game logic.
| Layer   | Technology          | Replaces                     |
|---------|---------------------|------------------------------|
| Graphics| DXVK                | DirectX 8 (d3d8.dll)         |
| Windowing| SDL3              | Win32 API                    |
| Audio   | OpenAL              | Miles Sound System           |
| Video   | FFmpeg              | Bink Video (intro/videos)    |
| Platform| SDL3 + libc         | Win32 POSIX calls            |

**CRITICAL**: Platform code must be isolated to `Core/GameEngineDevice/` and `Core/Libraries/Source/Platform/`. No native Win32/Cocoa/X11 calls in game logic.

## Golden Rules
1. Single codebase for Linux/macOS.
2. SDL3, DXVK, OpenAL everywhere.
3. 64-bit native only.
4. Preserve retail compatibility and determinism.
5. No band-aids; fix root cause.
6. Update dev blog before commit.
7. Use reference repos as patterns, not copy-paste.

## Reference Repositories
- **fighter19-dxvk-port**: DXVK + SDL3 + Linux platform patterns.
- **jmarshall-win64-modern**: OpenAL and 64-bit audio reference.
- **thesuperhackers-main**: Upstream regression baseline.

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
1. **GeneralsXZH** first.
2. **GeneralsX** only when change is shared and low risk.

## Backport Rules
- Backport platform/backend code and shared Core libs.
- Skip Zero Hour-only gameplay or risky changes.

**Do NOT backport:**
- Zero Hour-specific gameplay/logic
- Expansion-specific features
- High-risk changes to Zero Hour

## DXVK Source of Truth (macOS)
- Branch: `generalsx-macos-v2.6`.
- Local mode: `-DSAGE_DXVK_USE_LOCAL_FORK=ON`.
- Never edit `build/_deps/...`; fix fork source first.

## Common Pitfalls
- Linux case sensitivity: include paths must match exact case.
- DXVK needs Vulkan drivers.
- `-logToCon` works only in debug builds.
- SDL3 comes from FetchContent.
- Use `RTS_BUILD_OPTION_DEBUG=OFF` for replay tests.
- Linux traces may still need `fprintf(stderr, ...)`.

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

## Build Config Touchpoints
- `CMakePresets.json`
- `cmake/config-build.cmake`, `cmake/dx8.cmake`
- `cmake/miles.cmake`, `cmake/openal.cmake`
- `cmake/mingw.cmake`

## Branching & Sync
### TheSuperHackers upstream sync
```bash
git remote add thesuperhackers git@github.com:TheSuperHackers/GeneralsGameCode.git
git fetch thesuperhackers
git merge thesuperhackers/main
```

**Conflict resolution**:
- Platform code: keep ours.
- Game logic: keep theirs.
- Build system: merge carefully, test both.

## Code Conventions
- Annotate user-facing changes with `// GeneralsX @keyword author DD/MM/YYYY Description`.
- Use `@bugfix`, `@feature`, `@performance`, `@refactor`, `@tweak`, or `@build`.
- Add upstream PR attribution when relevant.
- English only. No empty stubs, empty catch blocks, or commented-out code.

## GitHub PR/Issue Formatting
- Use `--body-file`, not `--body`.
- Avoid literal `\n`; use real newlines.

## VS Code Tasks
- Prefer task-first build/test/debug.
- Keep logs in `logs/`.
- Primary labels: `[Linux]`, `[macOS]`, `[Linux] Pipeline: Build + Deploy + Run ZH`.

## Docs Workflow
1. Monthly diary in `docs/DEV_BLOG/YYYY-MM-DIARY.md`.
2. Active notes in `docs/WORKDIR/`.
3. Never drop working docs in `docs/` root.

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
- `GeneralsMD/`: Zero Hour.
- `Generals/`: base game.
- `Core/`: shared libraries.
- `references/`: fighter19, jmarshall, thesuperhackers-main.
- `docs/WORKDIR/`: current work docs.
- `logs/`: build/run/debug logs.

## Instruction Context Loading
Check `.github/instructions/*.instructions.md` and follow any file whose `applyTo` matches target path.

| Instruction File Path | applyTo | When to Use |
|---|---|---|
| `.github/instructions/generalsx.instructions.md` | `**` | Global platform rules |
| `.github/instructions/git-commit.instructions.md` | `**` | Commit/PR standards |
| `.github/instructions/docs.instructions.md` | `**/*.md` | Markdown docs |
| `.github/instructions/scripts.instructions.md` | `scripts/**` | Scripts under scripts/ |

Update table when instruction files change.

| Instruction File Path | applyTo | When to Use |
|---|---|---|
| `.github/instructions/generalsx.instructions.md` | `**` | Global project architecture and platform rules |
| `.github/instructions/git-commit.instructions.md` | `**` | Commit/PR naming and message standards |
| `.github/instructions/docs.instructions.md` | `**/*.md` | Any markdown documentation creation/update |
| `.github/instructions/scripts.instructions.md` | `scripts/**` | Any script under scripts/ tree |

LLM maintenance disclaimer: update this table immediately whenever instruction files are added, removed, renamed, or when any `applyTo` pattern changes.
