# GeneralsX: Instructions for AI Coding Agents

## What I Am
GeneralsX is a cross-platform port of Command & Conquer: Generals Zero Hour for **Linux and macOS**, porting legacy Windows DirectX 8 + Miles Sound code to a modern stack (SDL3 + DXVK + OpenAL + 64-bit). This is a **massive C++ game engine** (~500k LOC) preserving retail gameplay while modernizing the platform layer.

## Key Entry Points
- `GeneralsMD/Code/Main/WinMain.cpp`
- `Generals/Code/Main/WinMain.cpp`
- `Core/GameEngineDevice/Source/`

## Platform Focus
- **Active**: Linux (`linux64-deploy`), macOS (`macos-vulkan`)
- **Future/Exploratory**: Windows (MinGW path, issue #29)
- **Legacy**: VC6 + DirectX 8 + Miles (reference only)

## Architecture
| Layer   | Technology          | Replaces                     |
|---------|---------------------|------------------------------|
| Graphics| DXVK                | DirectX 8 (d3d8.dll)         |
| Windowing| SDL3              | Win32 API                    |
| Audio   | OpenAL (MiniAudio WIP)| Miles Sound System           |
| Video   | FFmpeg              | Bink Video (intro/videos)    |
| Platform| SDL3 + libc         | Win32 POSIX calls            |

**CRITICAL**: Platform code must be isolated to `Core/GameEngineDevice/` and `Core/Libraries/Source/Platform/`. No native Win32/Cocoa/X11 calls in game logic.

## Golden Rules
1. **Single codebase** – Linux and macOS build from same source
2. **SDL3 everywhere** – No native platform calls in game code
3. **DXVK everywhere** – DX8 → Vulkan translation on all platforms
4. **OpenAL default (MiniAudio WIP)** – Cross-platform audio stack. Audio fixes must prioritize OpenAL but be backported to MiniAudio.
5. **64-bit native** – x86_64 only (32-bit via VC6 upstream)
6. **Retail compatibility** – Original replays and mods must work
7. **Determinism** – Rendering/audio changes must not affect gameplay logic
8. **No band-aids** – Fix underlying issues, not symptoms
9. **Update worklog** – Update `docs/WORKLOG/YYYY-MM-DIARY.md` before committing (see [.github/instructions/docs.instructions.md](.github/instructions/docs.instructions.md) for details)
10. **Reference repos** – Study patterns, don't copy-paste
11. **Backport to Generals** – Bugfixes and improvements must be backported to the Generals base game.
12. **Deterministic Math** – The native math functions were replaced by deterministic multi-platform WWMath equivalents to maintain determinism. This pattern must be preserved. When syncing upstream, replace these functions:
    - `sqrt` / `sqrtf` -> `WWMath::SqrtOrigin` / `WWMath::SqrtfOrigin` or `WWMath::Sqrt`
    - `sin` / `sinf` -> `WWMath::SinTrig` or `WWMath::Sin`
    - `cos` / `cosf` -> `WWMath::CosTrig` or `WWMath::Cos`
    - `tan` / `tanf` -> `WWMath::TanTrig`
    - `acos` / `acosf` -> `WWMath::ACosTrig` or `WWMath::Acos`
    - `asin` / `asinf` -> `WWMath::ASinTrig` or `WWMath::Asin`
    - `atan` / `atanf` -> `WWMath::Atan`
    - `atan2` / `atan2f` -> `WWMath::Atan2Origin` or `WWMath::Atan2`
    - `ceil` / `ceilf` -> `WWMath::Ceil`
    - `floor` / `floorf` -> `WWMath::Floor`
    - `pow` -> `WWMath::PowOrigin`

## Reference Repositories
- **fighter19-dxvk-port** – Primary graphics/platform reference (DXVK + SDL3 on Linux)
- **jmarshall-win64-modern** – Audio reference (OpenAL implementation, Generals-only)
- **thesuperhackers-main** – Upstream baseline for regression checks

## Build Commands

### Linux (Docker-based)
Docker is the recommended build method on Linux hosts to ensure all toolchain requirements are met.

*   **Configure Build**:
    ```bash
    ./scripts/build/linux/docker-configure-linux.sh linux64-deploy
    ```
*   **Build Zero Hour**:
    ```bash
    ./scripts/build/linux/docker-build-linux-zh.sh linux64-deploy
    ```
*   **Build Generals (Base Game)**:
    ```bash
    ./scripts/build/linux/docker-build-linux-generals.sh linux64-deploy
    ```
*   **Flatpak Bundle Packaging**:
    ```bash
    ./scripts/build/linux/build-linux-flatpak.sh linux64-deploy Generals    # Base game
    ./scripts/build/linux/build-linux-flatpak.sh linux64-deploy GeneralsMD  # Zero Hour
    ```
*   **Optional MinGW Windows Cross-build**:
    ```bash
    ./scripts/build/linux/docker-build-mingw-zh.sh mingw-w64-i686
    ```

### Native Linux
*   **Configure & Build via CMake**:
    ```bash
    cmake --preset linux64-deploy
    cmake --build build/linux64-deploy --target z_generals
    ```
*   **Deploy**:
    ```bash
    ./scripts/build/linux/deploy-linux.sh     # Generals base game
    ./scripts/build/linux/deploy-linux-zh.sh  # Generals Zero Hour
    ```

### Native macOS
*   **Configure Build**:
    ```bash
    cmake --preset macos-vulkan
    ```
*   **Build via Scripts**:
    ```bash
    ./scripts/build/macos/build-macos-generals.sh  # Generals base game
    ./scripts/build/macos/build-macos-zh.sh        # Generals Zero Hour
    ```
    *(Alternatively, build via CMake: `cmake --build build/macos-vulkan --target z_generals`)*
*   **Deploy**:
    ```bash
    ./scripts/build/macos/deploy-macos-generals.sh  # Generals base game
    ./scripts/build/macos/deploy-macos-zh.sh        # Generals Zero Hour
    ```
*   **App Bundle Packaging**:
    ```bash
    ./scripts/build/macos/bundle-macos-generals.sh  # Generals Mac app bundle
    ./scripts/build/macos/bundle-macos-zh.sh        # Generals Zero Hour Mac app bundle
    ```

## Target Priority
1. **GeneralsXZH** (Zero Hour) – Primary target, most feature-complete
2. **GeneralsX** (Base game) – Stable and functional. Bugfixes and improvements must be backported.

## Backport Rules
**Backport to Generals when:**
- Change is platform/backend code (SDL3, DXVK, OpenAL/MiniAudio) or a general bugfix/improvement
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
1. Monthly diary in `docs/WORKLOG/YYYY-MM-DIARY.md` (YYYY=year, MM=month only, e.g., `2026-05-DIARY.md`)
2. Active work notes in `docs/WORKDIR/` (phases/planning/reports/support/audit/lessons)
3. Step-by-step tutorials in `docs/HOWTO/` (user-facing guides for common tasks)
4. Never drop working docs directly under `docs/` root

## GitHub CLI Examples

> [!IMPORTANT]
> When running `gh` commands within the agent sandbox environment, if `GITHUB_TOKEN=github_pat_antigravitydummytoken` is present, it will override local credentials and cause `HTTP 401: Bad credentials`. Unset the dummy token by prepending `env -u GITHUB_TOKEN -u GH_TOKEN` to your `gh` commands.
> 
> Example: `env -u GITHUB_TOKEN -u GH_TOKEN gh pr create ...`

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
- `references/`: old-refs/thesuperhackers-main, fbraz3-dxvk (active); old-refs/ (historical).
- `docs/WORKDIR/`: current work docs.
- `docs/HOWTO/`: user-facing step-by-step tutorials (SagePatch config, etc.)
- `logs/`: build/run/debug logs.

## Instruction Context Loading

The `.github/instructions/` files are scoped VS Code hints — they load only when the file path matches.

You MUST load the files below to your context when the file/dir you are working on matches the applyTo column pattern.

The `**` at applyTo means all files, you MUST load it everytime.

| Instruction File | applyTo | Purpose |
|---|---|---|
| [.github/instructions/git-commit.instructions.md](.github/instructions/git-commit.instructions.md) | `**` | Commit/PR message standards |
| [.github/instructions/cpp-conventions.instructions.md](.github/instructions/cpp-conventions.instructions.md) | `**/*.{cpp,h,hpp,c}` | Code style, annotations, platform isolation |
| [.github/instructions/build.instructions.md](.github/instructions/build.instructions.md) | `cmake/**,CMakeLists.txt,CMakePresets.json` | Build presets, DXVK source of truth |
| [.github/instructions/platform-linux.instructions.md](.github/instructions/platform-linux.instructions.md) | `scripts/build/linux/**` | Linux build notes |
| [.github/instructions/platform-macos.instructions.md](.github/instructions/platform-macos.instructions.md) | `scripts/build/macos/**,references/fbraz3-dxvk/**` | macOS/DXVK build notes |
| [.github/instructions/docs.instructions.md](.github/instructions/docs.instructions.md) | `**/*.md` | Documentation structure and workflow |
| [.github/instructions/scripts.instructions.md](.github/instructions/scripts.instructions.md) | `scripts/**` | Script organization and naming |

Update this table when instruction files are added, removed, or renamed.
