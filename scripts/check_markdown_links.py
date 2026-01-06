#!/usr/bin/env python3

import argparse
import os
import re
import sys
from dataclasses import dataclass


@dataclass(frozen=True)
class MissingLink:
    source_file: str
    target_raw: str
    resolved_path: str


LINK_PATTERN = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
SCHEME_PATTERN = re.compile(r"^[a-zA-Z][a-zA-Z0-9+.-]*:")


def iter_markdown_files(root: str) -> list[str]:
    markdown_files: list[str] = []
    for dirpath, _, filenames in os.walk(root):
        for filename in filenames:
            if filename.lower().endswith(".md"):
                markdown_files.append(os.path.join(dirpath, filename))
    return markdown_files


def should_check_target(target: str) -> bool:
    if not target:
        return False

    candidate = target.split("#", 1)[0]
    candidate = candidate.split("?", 1)[0]
    candidate = candidate.strip().strip("<>")

    if target.startswith("#"):
        return False
    if SCHEME_PATTERN.match(candidate):
        return False
    if any(ch.isspace() for ch in candidate):
        return False

    # Only check targets that look like local paths. This avoids false positives
    # from things that resemble Markdown links but are not intended to be paths
    # (e.g., C/C++ template arguments like `vector<void*>`).
    if (
        candidate.startswith(".")
        or candidate.startswith("/")
        or "/" in candidate
        or candidate.lower().endswith(".md")
    ):
        return True

    return False


def resolve_target(source_file: str, target: str) -> str:
    target = target.split("#", 1)[0]
    target = target.split("?", 1)[0]
    target = target.strip().strip("<>")

    if target.startswith("/"):
        # Repo-root relative path (GitHub-style)
        return target.lstrip("/")

    base_dir = os.path.dirname(source_file)
    return os.path.normpath(os.path.join(base_dir, target))


def find_missing_links(files: list[str]) -> tuple[int, list[MissingLink]]:
    checked_count = 0
    missing: list[MissingLink] = []

    for path in sorted(set(files)):
        if not os.path.exists(path):
            continue

        try:
            with open(path, "r", encoding="utf-8", errors="ignore") as handle:
                lines = handle.readlines()
        except OSError:
            continue

        in_fence = False
        fence_marker: str | None = None
        non_code_lines: list[str] = []
        for line in lines:
            stripped = line.lstrip()
            if stripped.startswith("```") or stripped.startswith("~~~"):
                marker = stripped[:3]
                if not in_fence:
                    in_fence = True
                    fence_marker = marker
                elif fence_marker == marker:
                    in_fence = False
                    fence_marker = None
                continue
            if not in_fence:
                non_code_lines.append(line)

        text = "".join(non_code_lines)

        for match in LINK_PATTERN.finditer(text):
            raw_target = match.group(1).strip()
            if not should_check_target(raw_target):
                continue

            resolved_path = resolve_target(path, raw_target)
            if not resolved_path:
                continue

            checked_count += 1
            if not os.path.exists(resolved_path):
                missing.append(
                    MissingLink(
                        source_file=path,
                        target_raw=raw_target,
                        resolved_path=resolved_path,
                    )
                )

    return checked_count, missing


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Check local Markdown links in the repository.",
    )
    parser.add_argument(
        "--include",
        action="append",
        default=[],
        help=(
            "Additional Markdown files to include (repeatable). "
            "Defaults to README.md and CONTRIBUTING.md if present."
        ),
    )
    parser.add_argument(
        "--docs-dir",
        default="docs",
        help="Docs directory to scan (default: docs).",
    )
    parser.add_argument(
        "--max",
        type=int,
        default=200,
        help="Max missing links to print (default: 200).",
    )

    args = parser.parse_args()

    files: list[str] = []

    for default in ["README.md", "CONTRIBUTING.md"]:
        if os.path.exists(default):
            files.append(default)

    for extra in args.include:
        if os.path.exists(extra):
            files.append(extra)

    if os.path.isdir(args.docs_dir):
        files.extend(iter_markdown_files(args.docs_dir))

    checked, missing = find_missing_links(files)

    print(f"Checked {checked}")
    print(f"Missing {len(missing)}")

    for item in missing[: args.max]:
        print(f"- {item.source_file}: {item.target_raw} -> {item.resolved_path}")

    if len(missing) > args.max:
        print(f"...and {len(missing) - args.max} more")

    return 0 if not missing else 2


if __name__ == "__main__":
    raise SystemExit(main())
