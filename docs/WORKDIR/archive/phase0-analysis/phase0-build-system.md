# Phase 0: Docker Build System Strategy

**Created**: 2026-02-07
**Status**: In Progress

## Objective

Configure Docker-based build workflow for:
- Native Linux ELF compilation (primary target)
- MinGW cross-compilation (Windows .exe from macOS/Linux)
- CI/CD pipeline integration
- Local development testing

## Why Docker?

**Problem**: Developing on macOS, targeting Linux + Windows
**Solution**: Docker provides isolated Linux build environment

**Benefits**:
- No local toolchain pollution (keep Mac clean)
- Reproducible builds (same environment every time)
- Easy CI/CD integration (GitHub Actions, etc.)
- Multi-platform support (Linux native + MinGW cross-compile)

## Build Targets

### Target 1: Native Linux ELF (Primary)
**Goal**: Compile GeneralsXZH for Linux x86_64
**Output**: Native ELF binary that runs on Linux
**Technologies**: GCC/Clang + DXVK + OpenAL + SDL3

**Docker Command**:
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && 
  apt install -y build-essential cmake ninja-build git \
    libsdl3-dev libvulkan-dev libopenal-dev && 
  cmake --preset linux64-deploy && 
  cmake --build build/linux64-deploy --target z_generals
"
```

**Result**: `build/linux64-deploy/GeneralsMD/GeneralsXZH` (ELF 64-bit)

### Target 2: Windows EXE via MinGW (Cross-compile)
**Goal**: Compile Windows .exe from macOS/Linux
**Output**: Windows PE executable
**Technologies**: MinGW-w64 cross-compiler

**Docker Command**:
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && 
  apt install -y build-essential cmake ninja-build git mingw-w64 && 
  cmake --preset mingw-w64-i686 && 
  cmake --build build/mingw-w64-i686 --target z_generals
"
```

**Result**: `build/mingw-w64-i686/GeneralsMD/GeneralsXZH.exe` (PE32)

## CMake Presets Configuration

### Linux64 Deploy Preset
**Status**: To be added to `CMakePresets.json`

```json
{
  "name": "linux64-deploy",
  "displayName": "Linux 64-bit (Deploy)",
  "description": "Native Linux build with DXVK/OpenAL/SDL3",
  "generator": "Ninja",
  "binaryDir": "${sourceDir}/build/linux64-deploy",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Release",
    "CMAKE_C_COMPILER": "gcc",
    "CMAKE_CXX_COMPILER": "g++",
    "BUILD_WITH_DXVK": "ON",
    "USE_OPENAL": "ON",
    "USE_SDL3": "ON"
  }
}
```

### MinGW Cross-Compile Preset
**Status**: Check if exists in fighter19 reference

```json
{
  "name": "mingw-w64-i686",
  "displayName": "MinGW-w64 i686 (Windows 32-bit)",
  "description": "Cross-compile Windows .exe",
  "generator": "Ninja",
  "binaryDir": "${sourceDir}/build/mingw-w64-i686",
  "toolchainFile": "${sourceDir}/cmake/toolchains/mingw-w64-i686.cmake"
}
```

## Docker Images

### Base Image: ubuntu:22.04
**Why**: Stable, well-supported, has all needed packages

**Installed Packages**:
- `build-essential`: GCC, G++, make, etc.
- `cmake`: Build system
- `ninja-build`: Fast build tool
- `git`: Version control (for submodules)
- `mingw-w64`: Windows cross-compiler
- `libsdl3-dev`: SDL3 development headers
- `libvulkan-dev`: Vulkan development headers
- `libopenal-dev`: OpenAL development headers

### Optimization: Custom Dockerfile
**Future**: Create `Dockerfile` with pre-installed tools

```dockerfile
FROM ubuntu:22.04

RUN apt update && apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    mingw-w64 \
    libsdl3-dev \
    libvulkan-dev \
    libopenal-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
```

**Build**: `docker build -t generalsx-builder .`
**Use**: `docker run --rm -v "$PWD:/work" generalsx-builder cmake --preset linux64-deploy`

## VS Code Tasks Integration

### Task: Build Linux (Docker)
```json
{
  "label": "Build GeneralsXZH (Linux Docker)",
  "type": "shell",
  "command": "mkdir -p logs && docker run --rm -v \"$PWD:/work\" -w /work ubuntu:22.04 bash -c \"apt update && apt install -y build-essential cmake ninja-build git && cmake --preset linux64-deploy && cmake --build build/linux64-deploy --target z_generals\" 2>&1 | tee logs/phase1_build_zh_docker.log",
  "group": "build"
}
```

**Already configured in `.vscode/tasks.json`** ✅

### Task: Build Windows (MinGW)
```json
{
  "label": "Build Windows (MinGW Docker)",
  "type": "shell",
  "command": "mkdir -p logs && docker run --rm -v \"$PWD:/work\" -w /work ubuntu:22.04 bash -c \"apt update && apt install -y build-essential cmake ninja-build git mingw-w64 && cmake --preset mingw-w64-i686 && cmake --build build/mingw-w64-i686 --target z_generals\" 2>&1 | tee logs/phase1_build_windows_mingw_docker.log",
  "group": "build"
}
```

**Already configured in `.vscode/tasks.json`** ✅

## Testing Workflow

### Local Testing (Docker)
```bash
# Build Linux native
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && apt install -y build-essential cmake ninja-build git && 
  cmake --preset linux64-deploy && 
  cmake --build build/linux64-deploy --target z_generals
"

# Test in Linux container (requires X11 forwarding or VNC)
docker run --rm -v "$PWD:/work" -w /work \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  ubuntu:22.04 \
  /work/build/linux64-deploy/GeneralsMD/GeneralsXZH -win
```

### VM Testing
**Better approach for graphics testing**:
- Build in Docker
- Copy binary to Linux VM
- Test with native Vulkan/OpenGL

### CI/CD (GitHub Actions)
```yaml
# .github/workflows/linux-build.yml
name: Linux Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-22.04
    steps:
      - uses: actions/checkout@v3
      - name: Build Linux
        run: |
          cmake --preset linux64-deploy
          cmake --build build/linux64-deploy --target z_generals
```

## Windows Build Validation

**Critical**: Must validate Windows builds still work

### Option 1: GitHub Actions Windows Runner
```yaml
jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup MSVC
        uses: microsoft/setup-msbuild@v1
      - name: Build VC6
        run: |
          cmake --preset vc6
          cmake --build build/vc6 --target z_generals
```

### Option 2: Windows VM (Local)
- Keep a Windows 10/11 VM for testing
- Use VS Code Remote SSH to build
- Test replay compatibility

## Logging Strategy

**Convention**: All build logs to `logs/` directory

```bash
logs/
  phase1_configure_docker.log
  phase1_build_zh_docker.log
  phase1_build_generals_docker.log
  phase1_build_windows_mingw_docker.log
  phase1_run_linux.log
```

## Prerequisites Validation

### Check Docker Installation
```bash
docker --version
docker run --rm hello-world
```

**VS Code Task**: "Validate: Check Docker Prerequisites" ✅

### Check CMake Presets
```bash
cmake --list-presets
# Should show: linux64-deploy, mingw-w64-i686
```

## Notes

- Docker approach keeps macOS development clean
- Native Linux ELF is primary target (NOT Wine/MinGW runtime)
- MinGW is for cross-compiling Windows .exe only
- VM testing preferred for graphics validation
- CI/CD validates multi-platform builds

## Next Steps

1. Verify Docker installed and working
2. Check if `linux64-deploy` preset exists in CMakePresets.json
3. Test Docker build workflow
4. Configure VM for testing (if using VirtualBox/Parallels)
5. Document any issues encountered

## Dockerfile Template & VS Code Task (no-run configuration)

Below is a recommended `Dockerfile` to build the native Linux toolchain. Add this to `resources/docker/` or project root as `Dockerfile.builder`.

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build git wget ca-certificates \
    libsdl3-dev libvulkan-dev libopenal-dev pkg-config gnupg && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /work

CMD ["/bin/bash"]
```

Suggested `Docker` build command (do not run here):
```bash
docker build -t generalsx-builder -f Dockerfile.builder .
```

VS Code task (add to `.vscode/tasks.json` or adapt existing):

```json
{
  "label": "Docker: Configure Linux Build",
  "type": "shell",
  "command": "docker run --rm -v \"$PWD:/work\" -w /work generalsx-builder bash -c \"cmake --preset linux64-deploy\"",
  "group": "build",
  "problemMatcher": []
}
```

And build task:

```json
{
  "label": "Docker: Build GeneralsXZH",
  "type": "shell",
  "command": "docker run --rm -v \"$PWD:/work\" -w /work generalsx-builder bash -c \"cmake --build build/linux64-deploy --target z_generals\"",
  "group": "build",
  "problemMatcher": []
}
```

Note: These tasks only configure/refer to the Docker image and presets — do not execute them now. They can be enabled locally once Docker image is built.
