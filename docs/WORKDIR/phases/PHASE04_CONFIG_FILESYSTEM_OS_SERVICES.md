# PHASE04: Config + Filesystem + OS Services

## Objective
Remove remaining runtime dependence on Windows registry and Win32-only OS services in the cross-platform path.

## Checklist
- [ ] Replace runtime registry reads/writes with INI/config equivalents.
- [ ] Ensure user-data paths and config discovery are cross-platform.
- [ ] Ensure all runtime file IO goes through the existing filesystem abstractions.
- [ ] Replace URL launching with a cross-platform approach.
- [ ] Replace any remaining Win32-only dialogs needed at runtime (if any).
- [ ] Verify asset loading (.big, textures, sounds) on macOS/Linux.

## Exit Criteria
- Game loads assets and reaches main menu with no registry dependency.
