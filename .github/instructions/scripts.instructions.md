---
applyTo: 'scripts/**'
---

# Scripts Organization Instructions

Organize `scripts/` by function so files stay easy to find and maintain.

## Structure Overview

Put each script in the right subdirectory:

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

- **`build/`**: build, deploy, run, package.
  - `build/linux/`: Linux docker workflow.
  - `build/macos/`: macOS native workflow.
  - `build/windows/`: future Windows toolchain.
- **`env/`**: environment setup.
  - `env/docker/`: Docker images and setup.
  - `env/cache/`: ccache/sccache setup.
- **`tooling/`**: analysis, maintenance, refactoring.
  - `tooling/clang-tidy/`, `tooling/cpp/maintenance/`.
- **`qa/`**: tests and validation.
  - `qa/smoke/`.
- **`legacy/`**: deprecated scripts and compatibility shims.

### 2. Naming Conventions

Use names that show scope and function:

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

When moving a script, create wrapper at old path so existing references keep working:

**Old path**: `scripts/old_script.sh`
```bash
#!/usr/bin/env bash
# Deprecated wrapper: Use scripts/new/location/old_script.sh
# GeneralsX @build 08/03/2026 Reorganization migration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/new/location/old_script.sh" "$@"
```

This keeps old references working and gives a clean deprecation path.

### 6. Update Documentation

When adding or moving scripts:

1. Update `scripts/README.md`.
2. Update `.vscode/tasks.json` if user-facing.
3. Update this file when category or naming changes.
4. Add header comment to script itself.

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
- **build/linux/** - 7 scripts.
- **build/macos/** - 4 scripts.
- **env/docker/** - 2 scripts.
- **env/cache/** - 4 scripts.
- **tooling/clang-tidy/** - 1 runner + plugin.
- **tooling/cpp/maintenance/** - 13 utilities.
- **qa/smoke/** - 2 scripts.
- **legacy/compat/** - 4 deprecated scripts.
- **Deprecated wrappers** - `cpp/`, `clang-tidy-plugin/`, `run-clang-tidy.py`.

### Pending (Future)
- **build/windows/** - modern Windows toolchain.

## When to Create a New Subdirectory

Create one only if you have 3+ related scripts, category is general-purpose, and team has agreed.

## For More Information

- **Main Overview**: [scripts/README.md](../../scripts/README.md)
- **Build System**: [CMakePresets.json](../../CMakePresets.json)
- **General Conventions**: [.github/instructions/generalsx.instructions.md](.github/instructions/generalsx.instructions.md)
