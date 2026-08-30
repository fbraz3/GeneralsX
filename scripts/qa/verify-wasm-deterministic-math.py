#!/usr/bin/env python3
"""Verify that a configured CMake build directory has deterministic,
cross-platform-safe floating point math enabled.

Used by the WebAssembly CI build gate (.github/workflows/build-wasm.yml) to
fail loudly rather than silently building with platform-native (non
deterministic) math or FMA-contracted float ops. See AGENTS.md's
"Cross-Platform Determinism" section: SAGE_USE_DETERMINISTIC_MATH and
-ffp-contract=off are both required for cross-platform replay/CRC parity
with the native Linux/macOS builds.

Usage:
    ./scripts/qa/verify-wasm-deterministic-math.py <build_dir>
"""

import json
import sys
from pathlib import Path


def check_cmake_cache(build_dir: Path) -> None:
    cache_path = build_dir / "CMakeCache.txt"
    cache_text = cache_path.read_text()
    if "SAGE_USE_DETERMINISTIC_MATH:BOOL=ON" not in cache_text:
        print(f"ERROR: SAGE_USE_DETERMINISTIC_MATH is not ON in {cache_path}", file=sys.stderr)
        sys.exit(1)
    print("OK: SAGE_USE_DETERMINISTIC_MATH=ON")


def check_fp_contract(build_dir: Path) -> None:
    compile_commands_path = build_dir / "compile_commands.json"
    entries = json.loads(compile_commands_path.read_text())
    zh_entries = [e for e in entries if "/GeneralsMD/" in e["file"]]
    if not zh_entries:
        print(f"ERROR: no GeneralsMD compile commands found in {compile_commands_path}", file=sys.stderr)
        sys.exit(1)

    missing = [e["file"] for e in zh_entries if "-ffp-contract=off" not in e["command"]]
    if missing:
        print(f"ERROR: -ffp-contract=off missing from {len(missing)} compile command(s):", file=sys.stderr)
        for f in missing[:5]:
            print(f"  {f}", file=sys.stderr)
        sys.exit(1)
    print(f"OK: -ffp-contract=off present in all {len(zh_entries)} GeneralsMD compile commands")


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <build_dir>", file=sys.stderr)
        return 2

    build_dir = Path(sys.argv[1])
    check_cmake_cache(build_dir)
    check_fp_contract(build_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
