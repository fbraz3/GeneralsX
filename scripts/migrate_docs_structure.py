#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Replacement:
    description: str
    pattern: re.Pattern[str]
    repl: str


def build_replacements() -> list[Replacement]:
    # Order matters: do most-specific first.
    rules: list[Replacement] = []

    def add(description: str, pattern: str, repl: str) -> None:
        rules.append(
            Replacement(
                description=description,
                pattern=re.compile(pattern),
                repl=repl,
            )
        )

    # docs/PLANNING -> docs/WORKDIR (with sub-mapping)
    add(
        "PLANNING sidequests dir",
        r"docs/PLANNING/SIDEQUESTS(?=/|\b)",
        "docs/WORKDIR/planning/sidequests",
    )
    add(
        "PLANNING reorg notes file",
        r"docs/PLANNING/PHASE_REORGANIZATION_NOTES\.md\b",
        "docs/WORKDIR/planning/PHASE_REORGANIZATION_NOTES.md",
    )
    add(
        "PLANNING planned phases dir",
        r"docs/PLANNING/PLANNED_PHASES(?=/|\b)",
        "docs/WORKDIR/phases/planned",
    )
    add(
        "PLANNING legacy docs dir",
        r"docs/PLANNING/LEGACY_DOCS(?=/|\b)",
        "docs/WORKDIR/phases/legacy",
    )
    add(
        "PLANNING numbered decade dir",
        r"docs/PLANNING/([0-9])(?=/)",
        r"docs/WORKDIR/phases/\1",
    )
    add(
        "PLANNING generic dir",
        r"docs/PLANNING(?=/|\b)",
        "docs/WORKDIR",
    )

    # docs/MISC -> docs/WORKDIR (categorized)
    add(
        "MISC dev archives dir (upper)",
        r"docs/MISC/DEV_ARCHIVES(?=/|\b)",
        "docs/WORKDIR/reports/DEV_ARCHIVES",
    )
    add(
        "MISC dev archives dir (mixed)",
        r"docs/Misc/DEV_ARCHIVES(?=/|\b)",
        "docs/WORKDIR/reports/DEV_ARCHIVES",
    )

    # Specific files from former docs/MISC
    misc_file_map = {
        "LESSONS_LEARNED.md": "docs/WORKDIR/lessons/LESSONS_LEARNED.md",
        "CRITICAL_VFS_DISCOVERY.md": "docs/WORKDIR/support/CRITICAL_VFS_DISCOVERY.md",
        "CRITICAL_FIXES.md": "docs/WORKDIR/support/CRITICAL_FIXES.md",
        "BIG_FILES_REFERENCE.md": "docs/WORKDIR/support/BIG_FILES_REFERENCE.md",
        "AUDIO_BACKEND_STATUS.md": "docs/WORKDIR/support/AUDIO_BACKEND_STATUS.md",
        "BACKEND_SELECTION_EXPLAINED.md": "docs/WORKDIR/support/BACKEND_SELECTION_EXPLAINED.md",
        "GRAPHICS_BACKENDS.md": "docs/WORKDIR/support/GRAPHICS_BACKENDS.md",
        "OPENGL_SUMMARY.md": "docs/WORKDIR/support/OPENGL_SUMMARY.md",
        "OPENGL_TESTING.md": "docs/WORKDIR/support/OPENGL_TESTING.md",
        "INI_PARSER_BLOCKER_ANALYSIS.md": "docs/WORKDIR/support/INI_PARSER_BLOCKER_ANALYSIS.md",
        "MULTITHREADING_ANALYSIS.md": "docs/WORKDIR/support/MULTITHREADING_ANALYSIS.md",
        "VULKAN_ANALYSIS.md": "docs/WORKDIR/support/VULKAN_ANALYSIS.md",
        "DX8WRAPPER_LEGACY_ANALYSIS.md": "docs/WORKDIR/support/DX8WRAPPER_LEGACY_ANALYSIS.md",
        "PROJECT_STATUS.md": "docs/WORKDIR/support/PROJECT_STATUS.md",
        "COMPREHENSIVE_VULKAN_PLAN.md": "docs/WORKDIR/planning/COMPREHENSIVE_VULKAN_PLAN.md",
        "INTEGRATION_AUDIT_PHASES_32_35.md": "docs/WORKDIR/audit/INTEGRATION_AUDIT_PHASES_32_35.md",
        "SESSION_SUMMARY_PHASE41_ANALYSIS.md": "docs/WORKDIR/reports/SESSION_SUMMARY_PHASE41_ANALYSIS.md",
        "PHASE_16_20_REVIEW_COMPLETE.md": "docs/WORKDIR/reports/PHASE_16_20_REVIEW_COMPLETE.md",
    }

    for name, target in misc_file_map.items():
        add(
            f"MISC file (upper): {name}",
            rf"docs/MISC/{re.escape(name)}\b",
            target,
        )
        add(
            f"MISC file (mixed): {name}",
            rf"docs/Misc/{re.escape(name)}\b",
            target,
        )

    return rules


def should_process(path: Path) -> bool:
    if path.is_dir():
        return False
    if any(part in {".git", "build", "Dependencies", "Core", "Generals", "GeneralsMD"} for part in path.parts):
        return False
    if path.as_posix().endswith("scripts/migrate_docs_structure.py"):
        return False
    return path.suffix.lower() in {".md", ".txt", ".py"} or path.name.endswith(".instruction.md")


def apply_replacements(text: str, replacements: list[Replacement]) -> tuple[str, int]:
    total = 0
    for rule in replacements:
        text, n = rule.pattern.subn(rule.repl, text)
        total += n
    return text, total


def main() -> int:
    parser = argparse.ArgumentParser(description="Migrate doc path references to the new docs structure.")
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="Repository root")
    parser.add_argument("--dry-run", action="store_true", help="Do not write files, only report")
    args = parser.parse_args()

    root: Path = args.root
    replacements = build_replacements()

    changed_files: list[tuple[Path, int]] = []

    for path in root.rglob("*"):
        if not should_process(path):
            continue

        try:
            original = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            original = path.read_text(encoding="utf-8", errors="ignore")

        updated, count = apply_replacements(original, replacements)
        if count == 0:
            continue

        changed_files.append((path, count))
        if not args.dry_run:
            path.write_text(updated, encoding="utf-8")

    changed_files.sort(key=lambda t: str(t[0]))

    print(f"Updated {len(changed_files)} files")
    for path, count in changed_files:
        rel = path.relative_to(root)
        print(f"- {rel} ({count} replacements)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
