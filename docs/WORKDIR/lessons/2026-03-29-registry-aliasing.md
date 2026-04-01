Registry INI: product-path aliasing
=================================

Date: 2026-03-29

Summary
-------
Added product-path aliasing to `RegistryIni` to improve compatibility with
mods and upstream tools that write/read different product registry paths.

What changed
------------
- `Core/Libraries/Source/WWVegas/WWLib/registryini.cpp` now normalizes common
  product name variants (e.g. "Zero Hour", "Generals Zero Hour",
  "Command & Conquer Generals Zero Hour", "cnc_generals_zh") into a
  canonical section name: `command and conquer generals zero hour`.
- The aliasing handles both single-segment names and multi-segment names
  by joining consecutive path segments and matching known aliases.

Why
---
Many community mods and installers write registry keys using slightly
different product names or separators (ampersand vs "and", underscores,
abbreviations). Normalizing these variants to a single canonical name
reduces false negatives when reading/writing the emulated registry.ini and
increases compatibility without requiring a full Windows registry export.

Notes and next steps
--------------------
- This is intentionally conservative: only a curated set of aliases are
  mapped. If additional variants are observed, add them to the alias table.
- Future work: provide a small unit test or integration test that writes
  keys under multiple variant paths and verifies reads resolve to the
  same canonical section.

Author: GitHubCopilot
