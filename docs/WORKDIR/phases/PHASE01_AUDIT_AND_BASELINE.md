# PHASE01: Audit and Baseline (Win32 → SDL2)

## Objective
Build a precise inventory of Win32-only dependencies in the shipping runtime path (GeneralsXZH) and define the exact replacement targets using SDL2.

## Scope
- Shipping runtime only (GeneralsXZH / core engine runtime code paths).
- Exclude tooling where it does not block the game running.
- Exclude reference submodules from the audit results.

## Deliverables
- A curated list of Win32 APIs used by runtime, grouped by subsystem.
- A list of existing abstraction points to hook SDL2 into (entry point, device layer, filesystem, logging, config).
- A minimal “SDL2 device layer” directory skeleton plan (names, files, ownership boundaries).

## Checklist
- [ ] Identify all runtime entry points and platform selection logic (Windows vs non-Windows).
- [ ] Enumerate Win32 window/message pump usage and where it feeds into engine.
- [ ] Enumerate input dependencies (mouse, keyboard, IME) in runtime.
- [ ] Enumerate timing/sleep usage in runtime.
- [ ] Enumerate display/fullscreen management usage in runtime.
- [ ] Enumerate registry usage in runtime (and decide replacements).
- [ ] Enumerate OS-integration usage (URL launch, clipboard, dialogs) in runtime.
- [ ] Document the “single-instance” mechanism and whether it needs SDL2-compatible replacement.
- [ ] Produce an initial integration diagram: Entry → Engine → Device → OS bridge.

## Exit Criteria
- The audit list is complete enough that PHASE02 can be executed without additional “surprise” Win32 APIs.
