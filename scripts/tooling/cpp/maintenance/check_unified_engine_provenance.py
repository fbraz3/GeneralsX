#!/usr/bin/env python3
"""Validate consolidated sources and report incoming changes without editing files.

Usage: python3 scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py
       ... --upstream-range BASE..HEAD [--fail-on-mapped-change]

The manifest is intentionally explicit: changes from either former game path
must be reviewed against the canonical implementation. No patches are applied.
"""

import argparse
import json
from pathlib import Path, PurePosixPath
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[4]
MANIFEST = "cmake/unified-engine-provenance.json"


def git(root, *args):
    """Run read-only git plumbing; option parsing ends before revision input."""
    return subprocess.check_output(["git", "-C", str(root), *args], stderr=subprocess.PIPE)


def revision(root, value):
    if not isinstance(value, str) or not value:
        raise ValueError("Revision must be a nonempty string")
    return git(root, "rev-parse", "--verify", "--end-of-options", value + "^{commit}").decode().strip()


def checked_path(root, value):
    if not isinstance(value, str) or not value or "\\" in value:
        raise ValueError("Paths must be nonempty repository-relative POSIX strings")
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or path.as_posix() != value or value == ".":
        raise ValueError("Invalid repository-relative path: " + value)
    try:
        (root / value).resolve().relative_to(root.resolve())
    except ValueError as exc:
        raise ValueError("Path escapes the repository: " + value) from exc
    return value


def validate(root, manifest):
    """Reject missing destinations, ambiguous mappings and revived old copies."""
    if not isinstance(manifest, dict) or manifest.get("schema_version") != 1:
        raise ValueError("Unsupported provenance schema")
    source_revision = revision(root, manifest["source_revision"])
    revision(root, manifest["upstream_revision"])
    entries = manifest["entries"]
    if not isinstance(entries, list) or not entries:
        raise ValueError("Provenance entries must be a nonempty list")
    seen = set()
    result = []
    for entry in entries:
        canonical = checked_path(root, entry["canonical_path"])
        origins = entry["original_paths"]
        if not isinstance(origins, list) or not origins:
            raise ValueError("Original paths must be a nonempty list")
        paths = [canonical] + [checked_path(root, p) for p in origins]
        if len(set(paths)) != len(paths) or seen.intersection(paths):
            raise ValueError("Ambiguous or duplicate source mapping: " + canonical)
        seen.update(paths)
        if not (root / canonical).is_file():
            raise ValueError("Missing canonical source: " + canonical)
        for origin in origins:
            if (root / origin).exists() or (root / origin).is_symlink():
                raise ValueError("Consolidated source has reappeared: " + origin)
            git(root, "cat-file", "-e", source_revision + ":" + origin)
        result.append({"canonical_path": canonical, "original_paths": origins})
    return result


def report_changes(root, entries, value):
    """Diff explicit endpoints, disabling rename inference so neither path is lost."""
    if "..." in value or value.count("..") != 1:
        raise ValueError("Use an explicit BASE..HEAD range")
    base, head = value.split("..")
    base, head = revision(root, base), revision(root, head)
    changed = set(filter(None, git(root, "diff", "--name-only", "--no-renames", "-z",
                                   base, head, "--").decode().split("\0")))
    mapped = []
    covered = set()
    for entry in entries:
        touched = changed.intersection([entry["canonical_path"]] + entry["original_paths"])
        if touched:
            covered.update(touched)
            mapped.append({"canonical_path": entry["canonical_path"],
                           "changed_paths": sorted(touched)})
    return {"base": base, "head": head, "mapped_changes": mapped,
            "unmapped_paths": sorted(changed - covered)}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--manifest", default=MANIFEST)
    parser.add_argument("--upstream-range", help="Explicit BASE..HEAD commit range; read-only")
    parser.add_argument("--fail-on-mapped-change", action="store_true",
                        help="Exit 2 if incoming changes need adaptation to consolidated sources")
    args = parser.parse_args(argv)
    if args.fail_on_mapped_change and not args.upstream_range:
        parser.error("--fail-on-mapped-change requires --upstream-range")
    try:
        root = args.root.resolve()
        manifest_path = root / checked_path(root, args.manifest)
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        entries = validate(root, manifest)
        result = {"valid": True, "consolidated_entries": len(entries)}
        if args.upstream_range:
            result.update(report_changes(root, entries, args.upstream_range))
        print(json.dumps(result, indent=2))
        return 2 if args.fail_on_mapped_change and result["mapped_changes"] else 0
    except (ValueError, KeyError, TypeError, OSError, subprocess.CalledProcessError) as exc:
        print("Provenance check failed: " + str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
