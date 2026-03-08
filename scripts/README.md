# Scripts Directory

This folder is organized by function for easier maintenance.

## Directory Structure

### `build/` - Build & Deployment Scripts per Platform
- `build/linux/` - Linux/Docker build, deploy, run, bundle, configure
- `build/macos/` - macOS build, deploy, run, bundle
- `build/windows/` - Windows modern toolchain (pending scripts)

### `env/` - Environment Setup
- `env/docker/` - Docker image management and setup  
- `env/cache/` - Compiler cache setup (ccache, sccache)

### `tooling/` - Code Analysis & Utilities
- `tooling/clang-tidy/` - Custom clang-tidy plugin and runner
  - `plugin/` - clang-tidy plugin source
  - `run.py` - clang-tidy wrapper/runner
- `tooling/cpp/maintenance/` - C++ code refactoring and maintenance utilities

### `qa/` - Quality Assurance & Testing
- `qa/smoke/` - Smoke tests and basic validation

### `legacy/` - Deprecated & Compatibility
- `legacy/compat/` - Old scripts and backward-compatibility shims

### Deprecated: `cpp/` and `clang-tidy-plugin/`
These directories contain backward-compatibility wrappers. See each README for migration info.

## Quick Reference

### Linux Builds
```bash
scripts/build/linux/docker-configure-linux.sh
scripts/build/linux/docker-build-linux-zh.sh
scripts/build/linux/deploy-linux-zh.sh
scripts/build/linux/run-linux-zh.sh
```

### macOS Builds
```bash
scripts/build/macos/build-macos-zh.sh
scripts/build/macos/deploy-macos-zh.sh
scripts/build/macos/run-macos-zh.sh
```

### Code Analysis
```bash
scripts/tooling/clang-tidy/run.py
```

### Cache Setup
```bash
scripts/env/cache/setup_ccache.sh
```

For more details, see the README in each subdirectory.
