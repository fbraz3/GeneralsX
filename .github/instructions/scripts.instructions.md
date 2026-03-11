---
applyTo: 'scripts/**'
---

# Scripts Organization Instructions

The `scripts/` folder is organized by function into distinct categories to improve maintainability and discoverability.

## Structure Overview

All scripts should be placed in the appropriate subdirectory based on their function:

```
scripts/
├── build/          # Build and deployment per platform
├── env/            # Environment setup (Docker, cache, etc.)
├── tooling/        # Code analysis and utilities
├── qa/             # Quality assurance and testing
└── legacy/         # Deprecated and backward-compatible shims
```

## Adding New Scripts

### 1. Determine Script Category

- **`build/`** - Build configuration, compilation, deployment, or running the game
  - `build/linux/` - Linux docker-based build workflow
  - `build/macos/` - macOS native build workflow
  - `build/windows/` - Windows modern toolchain (pending)
  - **Examples**: `docker-build-linux-zh.sh`, `build-macos-zh.sh`, `deploy-*.sh`, `run-*.sh`

- **`env/`** - Environment setup and configuration
  - `env/docker/` - Docker image management and setup
  - `env/cache/` - Compiler cache setup (ccache, sccache)
  - **Examples**: `docker-build-images.sh`, `setup_ccache.sh`, `docker-install.sh`

- **`tooling/`** - Code analysis, maintenance, and refactoring utilities
  - `tooling/clang-tidy/` - Custom clang-tidy plugin and runner
  - `tooling/cpp/maintenance/` - C++ code refactoring and fixes
  - **Examples**: `fix_*.py`, `run-clang-tidy.py`, `*_refactor_*.py`, `monitor-*.py`

- **`qa/`** - Quality assurance, testing, and validation
  - `qa/smoke/` - Smoke tests and basic validation
  - **Examples**: `docker-smoke-test-zh.sh`, `run-bundled-game.sh`

- **`legacy/`** - Deprecated or old scripts
  - `legacy/compat/` - Backward-compatibility shims and old implementations
  - **Examples**: `docker-build.sh` (legacy), `apply-patch-13-manual.sh`, deprecated wrappers

### 2. Naming Conventions

Scripts should follow consistent naming to indicate their scope and function:

| Pattern | Meaning | Example |
|---------|---------|---------|
| `*-linux-*` | Linux-specific | `docker-build-linux-zh.sh` |
| `*-macos-*` | macOS-specific | `build-macos-zh.sh` |
| `*-mingw-*` | MinGW/Windows cross | `docker-build-mingw-zh.sh` |
| `docker-*` | Docker-related | `docker-build-images.sh` |
| `setup-*` | Configuration/setup | `setup_ccache.sh` |
| `test-*` / `*-test` | Testing/validation | `docker-smoke-test-zh.sh` |
| `*-zh` | Zero Hour (expansion) | `docker-build-linux-zh.sh` |
| `*-generals` | Base game Generals | `docker-build-linux-generals.sh` |
| `fix_*.py` | Code fixes/refactoring | `fix_matrix_conversions.py` |

### 3. Shell Script Standards

#### Shebang & Error Handling
```bash
#!/usr/bin/env bash
set -e  # Exit on first error
set -u  # Exit on undefined variable
```

#### Documentation
```bash
#!/usr/bin/env bash
# Brief description of what the script does (one line)
#
# Usage:
#   ./script-name.sh [ARGS]
#
# Environment:
#   VERBOSE=1         Enable verbose output
#   LOG_DIR=logs/     Custom log directory
```

#### Logging & Output
```bash
# Use stderr for errors
echo "ERROR: Something failed" >&2
exit 1

# Use stdout for status
echo "Building..."
echo "Successfully built"

# Log to file if needed
mkdir -p "${LOG_DIR:-logs}"
LOGFILE="${LOG_DIR}/build_$(date +%s).log"
exec > >(tee -a "$LOGFILE")
exec 2>&1
```

### 4. Python Script Standards

#### Shebang & Imports
```python
#!/usr/bin/env python3
# Comprehensive docstring

import argparse
import sys
from pathlib import Path
```

#### Main Entry
```python
def main() -> int:
    """Main entry point."""
    # implementation
    return 0

if __name__ == "__main__":
    sys.exit(main())
```

### 5. Create Backward-Compatibility Wrapper (if moving)

When moving a script to a new location, **always create a wrapper at the old path** to maintain compatibility during transition:

**Old path**: `scripts/old_script.sh`
```bash
#!/usr/bin/env bash
# Deprecated wrapper: Use scripts/new/location/old_script.sh
# GeneralsX @build 08/03/2026 Reorganization migration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/new/location/old_script.sh" "$@"
```

This allows:
- Existing references and tasks to keep working
- Gradual migration of documentation and task definitions
- Clear deprecation path to users

### 6. Update Documentation

When adding or moving scripts:

1. **Update `scripts/README.md`**
   - Add script description in the appropriate category
   - Include any specific requirements or environment variables

2. **Update `.vscode/tasks.json`** (if user-facing)
   - Create VS Code task for easy access
   - Reference new script path in `command` field

3. **Update `.github/instructions/scripts.instructions.md`** (this file)
   - Document new category if applicable
   - Add naming convention pattern if new pattern introduced

4. **Add header comment** to the script itself
   - Describe purpose, usage, and environment variables
   - Follow standards above

## Organization Rules

### DO

- ✅ Place scripts in the most specific category subfolder
- ✅ Use platform/game suffixes for clarity (`-linux`, `-macos`, `-zh`, `-generals`)
- ✅ Create wrappers when moving scripts to maintain compatibility
- ✅ Document scripts with clear headers and usage instructions
- ✅ Log verbosely to help with debugging
- ✅ Exit with meaningful error codes
- ✅ Use relative paths and handle working directory changes carefully

### DON'T

- ❌ Leave scripts in `scripts/` root; always organize into subdirectories
- ❌ Create new top-level categories without discussion
- ❌ Use inconsistent naming (avoid `script.sh`, `_script_name`, `scr1pT.SH`)
- ❌ Forget backward-compatibility wrappers when moving
- ❌ Mix concerns (one script = one clear purpose)
- ❌ Create empty files, stub scripts, or commented-out code
- ❌ Hardcode absolute paths; use relative paths and environment variables

## Current Script Inventory

### Fully Organized (March 2026)
- **build/linux/** - 7 scripts (configure, build, deploy, run, bundle, smoke test)
- **build/macos/** - 4 scripts (build, deploy, run, bundle)
- **env/docker/** - 2 scripts (image building, installation)
- **env/cache/** - 4 scripts (ccache, sccache setup/test)
- **tooling/clang-tidy/** - 1 runner (run.py) + plugin source
- **tooling/cpp/maintenance/** - 13 utilities (fixes, refactoring, monitoring)
- **qa/smoke/** - 2 scripts (smoke test, bundled validation)
- **legacy/compat/** - 4 deprecated scripts
- **Deprecated (wrappers)** - `cpp/`, `clang-tidy-plugin/`, `run-clang-tidy.py`

### Pending (Future)
- **build/windows/** - Modern Windows toolchain (VS2022 + SDL3 + DXVK + OpenAL)

## When to Create a New Subdirectory

**Only** create a new subdirectory if:

1. You have **3+ related scripts** that don't fit any existing category
2. The new category is **general-purpose** (not one-off)
3. You've **discussed with team** to avoid duplicating existing organization

**Example**: If adding `build/windows/`, that makes sense (parallel to build/linux and build/macos).

**Counter-example**: Single new script for a one-time task → maybe `legacy/compat/` instead.

## For More Information

- **Main Overview**: [scripts/README.md](../../scripts/README.md)
- **Build System**: [CMakePresets.json](../../CMakePresets.json)
- **General Conventions**: [.github/instructions/generalsx.instructions.md](.github/instructions/generalsx.instructions.md)
