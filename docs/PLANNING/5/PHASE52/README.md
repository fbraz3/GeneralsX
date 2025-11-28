# Phase 52: INI Data Store Loading

**Phase**: 52
**Title**: INI File System and Data Store Initialization
**Duration**: 3-4 days
**Status**: COMPLETE
**Dependencies**: Phase 51 complete (GameClient working)
**Completed**: November 28, 2025

---

## Overview

Phase 52 handles the loading of all INI-based data stores. These subsystems parse game configuration files from the `Data\INI\` directory and populate in-memory data structures.

### Status: VERIFIED COMPLETE

All INI data stores initialize successfully. Verified via runtime logs on November 28, 2025.

---

## INI Data Stores Sequence

After TheGameClient, these subsystems load INI files:

| # | Subsystem | INI Path(s) | Priority |
|---|-----------|-------------|----------|
| 18 | TheRankInfoStore | `Data\INI\Rank` | Medium |
| 19 | ThePlayerTemplateStore | `Data\INI\Default\PlayerTemplate`, `Data\INI\PlayerTemplate` | High |
| 20 | TheParticleSystemManager | Factory method | High |
| 21 | TheFXListStore | `Data\INI\Default\FXList`, `Data\INI\FXList` | Medium |
| 22 | TheWeaponStore | `Data\INI\Weapon` | High |
| 23 | TheObjectCreationListStore | `Data\INI\Default\ObjectCreationList`, `Data\INI\ObjectCreationList` | Medium |
| 24 | TheLocomotorStore | `Data\INI\Locomotor` | Medium |
| 25 | TheSpecialPowerStore | `Data\INI\Default\SpecialPower`, `Data\INI\SpecialPower` | Medium |
| 26 | TheDamageFXStore | `Data\INI\DamageFX` | Low |
| 27 | TheArmorStore | `Data\INI\Armor` | Medium |
| 28 | TheBuildAssistant | No INI | Low |

---

## Potential Issues

### Path Separator Conversion

Windows uses `\` but macOS/Linux use `/`. The INI loader needs:

```cpp
// Convert paths
AsciiString fixPath(const char* path) {
    AsciiString result(path);
    result.replace('\\', '/');
    return result;
}
```

### Missing INI Files

Game assets may not include all INI files. Need graceful handling:

```cpp
if (!TheFileSystem->doesFileExist(iniPath)) {
    DEBUG_LOG(("INI file not found: %s (non-fatal)", iniPath));
    return; // Skip instead of crash
}
```

### INI Parser Hardening

Phase 22-23 added protections for:

- Unexpected `End` tokens
- Missing mandatory fields
- Malformed blocks

---

## Tasks

### Task 52.1: Path Separator Fixes

Ensure all INI paths use correct separators for current platform.

### Task 52.2: Missing File Handling

Add graceful skip for missing INI files (game may work with defaults).

### Task 52.3: Verify Asset Location

Confirm game assets are in:

- `$HOME/GeneralsX/GeneralsMD/Data/INI/`
- Or loaded from `.big` archives

### Task 52.4: INI Parser Testing

Test each store with actual game INI files:

- Rank.ini
- PlayerTemplate.ini
- Weapon.ini
- etc.

---

## Success Criteria

- [x] All path separators converted correctly
- [x] Missing INI files handled gracefully
- [x] All data stores load without crash
- [x] Game progresses to TheThingFactory

---

## Verification Evidence

From `logs/runTerminal.log` (November 28, 2025):

```log
GameEngine::init() - TheRankInfoStore initialized successfully
GameEngine::init() - ThePlayerTemplateStore initialized successfully
GameEngine::init() - TheParticleSystemManager initialized successfully
GameEngine::init() - TheFXListStore initialized successfully
GameEngine::init() - TheWeaponStore initialized successfully
GameEngine::init() - TheObjectCreationListStore initialized successfully
GameEngine::init() - TheLocomotorStore initialized successfully
GameEngine::init() - TheSpecialPowerStore initialized successfully
GameEngine::init() - TheDamageFXStore initialized successfully
GameEngine::init() - TheArmorStore initialized successfully
GameEngine::init() - TheBuildAssistant initialized successfully
```

---

**Created**: November 2025
**Completed**: November 28, 2025
