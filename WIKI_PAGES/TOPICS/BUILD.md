# TOPICS/BUILD.md

## Summary

Build instructions — creating and testing GeneralsX on Linux and macOS.

---

## Prerequisites

### Linux

- [Docker](https://www.docker.com/) — Recommended for CI compatibility
- [Git](https://git-scm.com/) — Version control
- Build tools: `cmake`, `ninja`, `make`

### macOS

- [Homebrew](https://brew.sh/) — Package manager
- [Git](https://git-scm.com/) — Version control
- [vcpkg](https://vcpkg.io/) — C++ library manager (for SDL3, DXVK)

---

## Step-by-Step Guide

### Step 1: Clone Repository

```bash
git clone https://github.com/yourusername/GeneralsX.git
cd GeneralsX
```

### Step 2: Configure Build

#### Docker (Linux, Recommended)

```bash
# Linux build
./scripts/build/linux/docker-configure-linux.sh linux64-deploy

# Optional: Windows via MinGW cross-build
./scripts/build/linux/docker-build-mingw-zh.sh mingw-w64-i686
```

#### Native Linux

```bash
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals
```

#### macOS

```bash
./scripts/build/macos/build-macos-zh.sh
```

### Step 3: Verify Build

```bash
# Check binary exists
ls -lh build/linux64-deploy/GeneralsMD/GeneralsXZH

# Run smoke test
./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy
```

---

## Platform-Specific Notes

| Platform | Command | Notes |
|----------|---------|-------|
| Linux | `./scripts/build/linux/deploy-linux-zh.sh` | Deployment script |
| macOS | `./scripts/build/macos/deploy-macos-zh.sh` | Deployment script |

---

## Troubleshooting

### "SDL3 not found" error

**Cause:** Missing or incomplete vcpkg installation

**Solution:**
```bash
# macOS
VCPKG_ROOT=$(resolve_vcpkg_root)
export VCPKG_ROOT
```

### "vcpkg not found" error

**Cause:** VCPKG_ROOT not set or vcpkg not installed

**Solution:**
```bash
# Install vcpkg
https://vcpkg.io/docs/getting-started.html

# Set VCPKG_ROOT
export VCPKG_ROOT=/path/to/vcpkg
```

---

## See Also

- [CONCEPTS/PORTING.md](../CONCEPTS/PORTING.md) — Platform technologies
- [TOPICS/TESTING.md](./TESTING.md) — Testing protocol

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, build documentation
