#!/usr/bin/env python3
"""Exercise source provenance checks against real, isolated Git repositories.

Run: python3 -m unittest discover -s scripts/qa -p test_unified_engine_provenance.py
Scratch repositories stay under build/, never in the system temporary directory.
"""

import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py"
SPEC = importlib.util.spec_from_file_location("provenance", SCRIPT)
provenance = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(provenance)


class ProvenanceTests(unittest.TestCase):
    def setUp(self):
        scratch = ROOT / "build/unified-provenance-tests"
        scratch.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=scratch)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.run_git("init", "-q")
        self.origins = ["Generals/Code/Guard.h", "GeneralsMD/Code/Guard.h"]
        for name in self.origins:
            self.write(name, "void guard();\n")
        self.commit("original sources")
        self.base = self.run_git("rev-parse", "HEAD")
        self.canonical = "Core/Guard.h"
        self.write(self.canonical, "void guard();\n")
        for name in self.origins:
            (self.root / name).unlink()
        self.manifest = {"schema_version": 1, "source_revision": self.base,
                         "upstream_revision": self.base,
                         "entries": [{"canonical_path": self.canonical,
                                      "original_paths": self.origins}]}

    def run_git(self, *args):
        return subprocess.check_output(["git", "-C", str(self.root), *args],
                                       stderr=subprocess.PIPE, text=True).strip()

    def write(self, name, content):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def commit(self, message):
        self.run_git("add", ".")
        self.run_git("-c", "user.name=GeneralsX tests", "-c",
                     "user.email=tests@example.invalid", "commit", "-qm", message)

    def test_valid_consolidation(self):
        self.assertEqual(provenance.validate(self.root, self.manifest),
                         self.manifest["entries"])

    def test_revived_original_and_missing_destination_are_rejected(self):
        for origin in self.origins:
            with self.subTest(origin=origin):
                self.write(origin, "void guard();\n")
                with self.assertRaisesRegex(ValueError, "reappeared"):
                    provenance.validate(self.root, self.manifest)
                (self.root / origin).unlink()
        (self.root / self.canonical).unlink()
        with self.assertRaisesRegex(ValueError, "Missing canonical"):
            provenance.validate(self.root, self.manifest)

    def test_ambiguous_mapping_and_invalid_paths_are_rejected(self):
        self.manifest["entries"].append(dict(self.manifest["entries"][0]))
        with self.assertRaisesRegex(ValueError, "duplicate"):
            provenance.validate(self.root, self.manifest)
        for path in ["../outside", "/absolute", "Core/../other", "Core\\Guard.h"]:
            with self.subTest(path=path), self.assertRaises(ValueError):
                provenance.checked_path(self.root, path)

    def test_changes_from_each_original_path_are_reported(self):
        # Commit incoming changes to the old tree, then restore the consolidated
        # worktree without altering the committed endpoints inspected by the tool.
        for origin in self.origins:
            self.write(origin, "void changedGuard();\n")
        self.write("Other.h", "void unrelated();\n")
        (self.root / self.canonical).unlink()
        self.commit("upstream changes to both originals")
        self.write(self.canonical, "void guard();\n")
        for origin in self.origins:
            (self.root / origin).unlink()
        entries = provenance.validate(self.root, self.manifest)
        before = self.run_git("status", "--porcelain")
        report = provenance.report_changes(self.root, entries, self.base + "..HEAD")
        self.assertEqual(self.run_git("status", "--porcelain"), before)
        self.assertEqual(report["mapped_changes"], [{"canonical_path": self.canonical,
                                                    "changed_paths": self.origins}])
        self.assertIn("Other.h", report["unmapped_paths"])

    def test_cli_exit_codes_for_adaptation_and_invalid_state(self):
        for origin in self.origins:
            self.write(origin, "void changedGuard();\n")
        (self.root / self.canonical).unlink()
        self.commit("incoming changes")
        self.write(self.canonical, "void guard();\n")
        for origin in self.origins:
            (self.root / origin).unlink()
        self.write("manifest.json", json.dumps(self.manifest))
        command = ["python3", str(SCRIPT), "--root", str(self.root),
                   "--manifest", "manifest.json", "--upstream-range", self.base + "..HEAD",
                   "--fail-on-mapped-change"]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stderr)
        self.assertEqual(len(json.loads(result.stdout)["mapped_changes"]), 1)
        self.write(self.origins[0], "revived copy\n")
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn("reappeared", result.stderr)

    def test_incoming_rename_still_reports_the_old_source_path(self):
        for origin in self.origins:
            self.write(origin, "void guard();\n")
        (self.root / self.canonical).unlink()
        self.run_git("mv", self.origins[0], "RenamedGuard.h")
        self.commit("rename upstream source")
        (self.root / self.origins[1]).unlink()
        self.write(self.canonical, "void guard();\n")
        entries = provenance.validate(self.root, self.manifest)
        report = provenance.report_changes(self.root, entries, self.base + "..HEAD")
        self.assertEqual(report["mapped_changes"][0]["changed_paths"], [self.origins[0]])
        self.assertIn("RenamedGuard.h", report["unmapped_paths"])

    def test_invalid_range_is_rejected(self):
        entries = provenance.validate(self.root, self.manifest)
        for value in ["HEAD", "HEAD...HEAD", "..HEAD", "--help..HEAD"]:
            with self.subTest(value=value), self.assertRaises((ValueError, subprocess.CalledProcessError)):
                provenance.report_changes(self.root, entries, value)


if __name__ == "__main__":
    unittest.main()
