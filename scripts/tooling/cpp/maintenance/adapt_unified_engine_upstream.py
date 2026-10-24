#!/usr/bin/env python3
"""Report and optionally stage upstream tip blobs for PLAN-023 mapped provenance paths.

Full `git merge upstream/main` can fail when fork ancestry diverges (unrelated
histories). This tool supports the file-level adaptation workflow:

  1. Validate provenance manifest.
  2. Diff each mapped dual-path hit against --upstream-tip (default: upstream/main).
  3. With --stage-canonical, write upstream tip content into the ZH-canonical path
     as a starting point for manual WWMath / GameProfile adaptation review.

Does not commit. Never overwrites without --stage-canonical.

Usage:
  python3 scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py
  python3 scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py \\
      --upstream-tip upstream/main --only WanderAIUpdate.cpp
  python3 scripts/tooling/cpp/maintenance/adapt_unified_engine_upstream.py --stage-canonical
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
MANIFEST = ROOT / "cmake" / "unified-engine-provenance.json"


def git(*args: str) -> bytes:
    return subprocess.check_output(["git", "-C", str(ROOT), *args], stderr=subprocess.PIPE)


def rev_parse(value: str) -> str:
    return git("rev-parse", "--verify", "--end-of-options", value + "^{commit}").decode().strip()


def show_blob(rev: str, path: str) -> bytes | None:
    try:
        return git("show", f"{rev}:{path}")
    except subprocess.CalledProcessError:
        return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--upstream-tip", default="upstream/main", help="Upstream commit-ish")
    ap.add_argument("--only", default="", help="Substring filter on canonical_path")
    ap.add_argument(
        "--stage-canonical",
        action="store_true",
        help="Write upstream tip blob into canonical_path (review before commit)",
    )
    args = ap.parse_args()

    tip = rev_parse(args.upstream_tip)
    manifest = json.loads(MANIFEST.read_text())
    if manifest.get("schema_version") != 1:
        print("ERROR: unsupported provenance schema", file=sys.stderr)
        return 2

    entries = manifest["entries"]
    reported = 0
    staged = 0
    for entry in entries:
        canonical = entry["canonical_path"]
        if args.only and args.only not in canonical:
            continue
        origins = entry["original_paths"]
        up = show_blob(tip, canonical)
        if up is None and origins:
            # Prefer GeneralsMD origin path if canonical missing at tip (pre-consolidation)
            for origin in origins:
                if "GeneralsMD/" in origin:
                    up = show_blob(tip, origin)
                    if up is not None:
                        break
            if up is None:
                up = show_blob(tip, origins[0])
        if up is None:
            print(f"MISS  {canonical} (not in {args.upstream_tip})")
            continue

        local = (ROOT / canonical).read_bytes() if (ROOT / canonical).is_file() else None
        if local is None:
            print(f"LOCAL-MISSING {canonical}")
            reported += 1
            continue
        if local == up:
            print(f"SAME  {canonical}")
            continue

        print(f"DIFF  {canonical}  local={len(local)} upstream={len(up)}")
        reported += 1
        if args.stage_canonical:
            path = ROOT / canonical
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(up)
            print(f"  staged upstream tip -> {canonical}")
            staged += 1

    print(f"Summary: tip={tip[:12]} diffs={reported} staged={staged}")
    if staged:
        print("Review staged files for WWMath Origin/Trig and GameProfile before commit.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
