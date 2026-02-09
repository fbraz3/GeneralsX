# Next Steps - Phase 0 Setup Complete

**Date**: 2026-02-07
**Status**: Documentation structure created, ready to begin Phase 0 analysis

## What Was Done

### 1. ✅ New Instructions File
- Created comprehensive `.github/instructions/generalsx.instructions.md`
- Defined 4-phase Linux port strategy:
  - **Phase 0**: Deep analysis & planning (CURRENT)
  - **Phase 1**: Linux graphics via DXVK
  - **Phase 2**: Linux audio via OpenAL
  - **Phase 3**: Video playback investigation (spike)
  - **Phase 4**: Polish & hardening

### 2. ✅ Documentation Structure
```
docs/
├── DEV_BLOG/
│   └── 2026-02-DIARY.md               # Development diary (update before commits)
├── WORKDIR/
│   ├── phases/
│   │   └── PHASE00_ANALYSIS_PLANNING.md  # Phase 0 progress tracking
│   ├── planning/                      # Planning & strategic documents
│   ├── reports/                       # Session reports
│   ├── support/                       # Technical discoveries
│   ├── audit/                         # Audit records
│   └── lessons/                       # Lessons learned
└── ETC/
    └── COMMAND_LINE_PARAMETERS.md     # Game launch parameters reference
```

### 3. ✅ VS Code Tasks
Added Linux/macOS build tasks to `.vscode/tasks.json`:
- `Configure (Linux MinGW i686)` - Configure MinGW build
- `Build GeneralsXZH (Linux MinGW i686)` - Build Zero Hour for Linux
- `Build GeneralsX (Linux MinGW i686)` - Build Generals for Linux
- `Pipeline: Configure + Build ZH (MinGW)` - Full pipeline
- `Validate: Check MinGW Prerequisites` - Check toolchain
- `Docs: Update Dev Blog` - Quick access to diary

### 4. ✅ Backup Original
- Original instructions saved to `.github/instructions/generalsx.instructions.md.backup`

## What You Need to Do Next

### Immediate (Before Any Coding)

1. **Install Docker on macOS**:
   ```bash
   brew install --cask docker
   # Or download from https://www.docker.com/products/docker-desktop
   ```

2. **Verify Docker installation**:
   - Run VS Code Task: `Validate: Check Docker`
   - Or manually: `docker run hello-world`

3. **Pull Ubuntu base image**:
   ```bash
   docker pull ubuntu:22.04
   ```

4. **Configure Linux preset** (if not already in CMakePresets.json):
   ```bash
   # Check if preset exists
   grep -A 10 "linux64-deploy" CMakePresets.json
   
   # If missing, copy from references/fighter19-dxvk-port/CMakePresets.json
   ```

5. **Set up reference repository remotes** (for easy diffing):
   ```bash
   cd references/fighter19-dxvk-port
   git remote add upstream https://github.com/Fighter19/CnC_Generals_Zero_Hour.git
   
   cd ../jmarshall-win64-modern
   git remote add upstream https://github.com/jmarshall2323/CnC_Generals_Zero_Hour.git
   
   cd ../thesuperhackers-main
   git remote add upstream https://github.com/TheSuperHackers/GeneralsGameCode.git
   ```

### Phase 0: Begin Analysis

Start documenting the following (create files in `docs/WORKDIR/phases/` or `docs/WORKDIR/support/`):

1. **Current Renderer Architecture** (`support/phase0-renderer-architecture.md`):
   - Where does DX8 device initialization happen?
   - How is device management handled?
   - What are the present/swap chain mechanisms?

2. **fighter19 DXVK Analysis** (`support/phase0-fighter19-analysis.md`):
   - How does DXVK wrapper integrate?
   - What files were changed?
   - What applies directly to Zero Hour?
   - What needs adaptation?

3. **jmarshall OpenAL Analysis** (`support/phase0-jmarshall-analysis.md`):
   - How is Miles→OpenAL mapped?
   - What's the audio pipeline?
   - What's Generals-specific vs Zero Hour-applicable?

4. **Platform Abstraction Design** (`support/phase0-platform-abstraction.md`):
   - Win32→POSIX mapping strategy
   - Filesystem path handling
   - #ifdef isolation strategy

5. **Build System Strategy** (`support/phase0-build-system.md`):
   - Docker build configuration
   - Linux native ELF compilation workflow
   - VM testing strategy

6. **Risk Assessment** (`support/phase0-risk-assessment.md`):
   - Determinism concerns
   - Replay compatibility
   - Debugging strategy
   - Performance implications

### Tools to Use for Analysis

1. **DeepWiki** (via AI):
   - Query `Fighter19/CnC_Generals_Zero_Hour` for DXVK patterns
   - Query `jmarshall2323/CnC_Generals_Zero_Hour` for OpenAL patterns
   - Query `TheSuperHackers/GeneralsGameCode` for baseline behavior

2. **File Comparison**:
   ```bash
   # Compare structure changes
   diff -r GeneralsMD/Code/GameEngineDevice/ references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/
   
   # Compare CMake changes
   diff CMakePresets.json references/fighter19-dxvk-port/CMakePresets.json
   ```

3. **Grep Search**:
   ```bash
   # Find DX8 device creation
   grep -r "IDirect3D8" --include="*.cpp" --include="*.h"
   
   # Find DXVK integration points in fighter19
   grep -r "DXVK\|SDL3" references/fighter19-dxvk-port/
   ```

## Docker Build Commands (Keep Mac Clean!)

Once Phase 0 is done and you're ready to build:

### Linux Native Builds (ELF)
```bash
# Build Linux native executable in Docker
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && apt install -y build-essential cmake ninja-build git && 
  cmake --preset linux64-deploy && 
  cmake --build build/linux64-deploy --target z_generals
"

# Result: build/linux64-deploy/GeneralsMD/GeneralsXZH (ELF binary)
```

### Windows Builds via MinGW (in Docker - no brew install!)
```bash
# Build Windows .exe via MinGW in Docker
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && apt install -y build-essential cmake ninja-build git mingw-w64 && 
  cmake --preset mingw-w64-i686 && 
  cmake --build build/mingw-w64-i686 --target z_generals
"

# Result: build/mingw-w64-i686/GeneralsMD/GeneralsXZH.exe (Windows PE)
# Runs on: Windows natively, or Linux via Wine+DXVK
```

### Why This Rocks
- ✅ **No toolchain pollution** on your Mac
- ✅ **Reproducible builds** (same container = same result)
- ✅ **Easy CI/CD** (GitHub Actions can use same commands)
- ✅ **Other devs get identical environment**

## Important Reminders

1. **Update Dev Blog**: Before every commit, update `docs/DEV_BLOG/2026-02-DIARY.md`
2. **Windows Compatibility**: All changes must preserve VC6 and Win32 builds
3. **Research First**: Phase 0 is analysis-heavy, code-light
4. **Document Everything**: Future you needs to understand what present you is thinking

## Phase 0 Completion Checklist

Phase 0 is complete when:
- [ ] All 6 analysis documents created in `docs/WORKDIR/support/`
- [ ] Docker workflow configured and tested (native Linux ELF builds working)
- [ ] Windows build validation pipeline working (VC6 preset)
- [ ] Phase 1 implementation plan written with step-by-step checklist
- [ ] All Phase 0 acceptance criteria marked as done in `docs/WORKDIR/phases/PHASE00_ANALYSIS_PLANNING.md`

---

**Remember**: Don't rush Phase 0. A solid understanding now prevents costly refactoring later.
