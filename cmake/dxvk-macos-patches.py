#!/usr/bin/env python3
"""
Deprecated legacy helper for macOS DXVK patching.

This script is intentionally a no-op.

Current source-of-truth model:
- macOS DXVK fixes must live in the fork history.
- The build consumes a pinned fork commit from cmake/dx8.cmake.
- No local PATCH_COMMAND patching is performed in the active workflow.

Kept only for backward compatibility and historical references.
"""

import sys


def main() -> int:
    print("dxvk-macos-patches.py is deprecated and now a no-op.")
    print("DXVK macOS fixes must be committed in the fork consumed by cmake/dx8.cmake.")
    if len(sys.argv) > 1:
        print(f"Ignored path argument: {sys.argv[1]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
