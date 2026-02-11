# Docker Build Images

This directory contains Dockerfiles for GeneralsX development environments.

## Images

### `Dockerfile.linux`
**Image**: `generalsx/linux-builder:latest`  
**Purpose**: Linux native ELF builds (GeneralsX, GeneralsXZH)  
**Base**: Ubuntu 22.04 (linux/amd64)  
**Size**: ~90MB (vcpkg is mounted as volume, not included)

**Includes**:
- GCC, Clang, Ninja, Git
- CMake 3.25.0
- Python3 + pip
- pkg-config, curl, zip, unzip

**Note**: vcpkg is mounted from `~/.generalsx/vcpkg` at runtime (auto-initialized on first run)

**Build**:
```bash
docker build -t generalsx/linux-builder:latest -f Dockerfile.linux .
# Or use: ./scripts/docker-build-images.sh linux
```

### `Dockerfile.mingw`
**Image**: `generalsx/mingw-builder:latest`  
**Purpose**: Windows .exe cross-compilation (MinGW-w64)  
**Base**: Ubuntu 22.04 (linux/amd64)  
**Size**: ~660MB

**Includes**:
- MinGW-w64 (i686 + x86_64 targets)
- CMake 3.25.0
- Ninja, Git
- Wine64 (for testing Windows .exe)

**Note**: vcpkg NOT needed for MinGW builds (uses system libraries)

**Build**:
```bash
docker build -t generalsx/mingw-builder:latest -f Dockerfile.mingw .
# Or use: ./scripts/docker-build-images.sh mingw
```

## UFirst-Time Setup

vcpkg is stored locally at `~/.generalsx/vcpkg` and mounted into containers:

```bash
# Option 1: Automatic (recommended)
# Build scripts auto-initialize vcpkg on first run
./scripts/docker-build-linux-zh.sh  # Will run docker-vcpkg-init.sh if needed

# Option 2: Manual (if you want to set up ahead of time)
./scripts/docker-vcpkg-init.sh  # One-time, ~2-5 minutes
```

**What happens**:
- vcpkg is cloned to `~/.generalsx/vcpkg` (full clone for baseline commits)
- Bootstrap script runs (compiles vcpkg binary)
- Subsequent builds mount this directory as volume at `/opt/vcpkg` in container

**Benefits**:
- ✅ Image stays small (~90MB vs ~328MB)
- ✅ vcpkg shared across all builds (no duplication)
- ✅ Easy to update: `cd ~/.generalsx/vcpkg && git pull && ./bootstrap-vcpkg.sh`
- ✅ vcpkg Location

Default location: `~/.generalsx/vcpkg`

To change:
```bash
# Edit scripts to use different path
export VCPKG_DIR="/custom/path/to/vcpkg"
```

### Updating vcpkg

```bash
cd ~/.generalsx/vcpkg
git pull
./bootstrap-vcpkg.sh -disableMetrics
```

Or re-run init script:
```bash
rm -rf ~/.generalsx/vcpkg
./scripts/docker-vcpkg-init.sh
```

### Persists vcpkg package cache between builds

### Automated (Recommended)
Build scripts automatically check for images and vcpkg, build/initialize
### Automated (Recommended)
Build scripts automatically check for images and build if missing:
```bash
./scripts/docker-build-linux-zh.sh  # Auto-uses generalsx/linux-builder
./scripts/docker-build-mingw-zh.sh  # Auto-uses generalsx/mingw-builder
```

### Manual
Build images explicitly:
```bash in Dockerfiles:
```dockerfile
# Current (3.25.0)
RUN curl -sL https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0-linux-x86_64.tar.gz | tar -xz ...

# New version (e.g., 3.28.0)
RUN curl -sL https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-linux-x86_64.tar.gz | tar -xz ...
```

Then rebuild:
```bash
./scripts/docker-build-images.sh all
```dockerfile
# Current (3.25.0)
RUN curl -sL https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0-linux-x86_64.tar.gz | tar -xz ...

# New version (e.g., 3.28.0)
RUN curl -sL https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-linux-x86_64.tar.gz | tar -xz ...
```

Then rebuild:
```bash
./scripts/docker-build-images.sh all
```

### Updating vcpkg
vcpkg is cloned from GitHub at build time (always latest). To update:
```bash
# Just rebuild the image
./scripts/docker-build-images.sh linux

# Or manually update in running container
docker run --rm -it generalsx/linux-builder:latest bash
cd /opt/vcpkg
git pull
./bootstrap-vcpkg.sh -disableMetrics
```

## Troubleshooting

### Image Not Found
If build scripts complain about missing image:
```bash
./scripts/docker-build-images.sh all
```

### Image Taking Too Much Space
```bash
# Remove old images
docker rmi generalsx/linux-builder:latest
docker rmi generalsx/mingw-builder:latest

# Rebuild
./scvcpkg Not Found
If build scripts complain about vcpkg:
```bash
./scripts/docker-vcpkg-init.sh
```

Or check if it exists:
```bash
ls -la ~/.generalsx/vcpkg
```

### vcpkg Baseline Errors
If you see git baseline errors, vcpkg might be corrupted:
```bash
# Re-initialize
rm -rf ~/.generalsx/vcpkg
./scripts/docker-vcpkg-init.sh
```

### ripts/docker-build-images.sh all
```

### Full Cleanup
```bash
# WARNING: Removes ALL unused Docker data
docker system prune --all --volumes

# Rebuild GeneralsX images
./scripts/docker-build-images.sh all
```

## VS Code Tasks

Tasks available in VS Code (Cmd+Shift+P → "Tasks: Run Task"):
- **Docker: Build Images (All)** - Build both images
- **Docker: Build Linux Builder Image** - Build Linux only
- **Docker: Build MinGW Builder Image** - Build MinGW only

## References

- Documentation: `docs/WORKDIR/support/DOCKER_WORKFLOW.md`
- Build scripts: `scripts/docker-*.sh`
- Tasks: `.vscode/tasks.json`

---

**Note**: Docker layer caching means rebuilds are fast—only changed layers are rebuilt!
