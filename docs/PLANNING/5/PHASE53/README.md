# Phase 53: Object and Template Systems

**Phase**: 53
**Title**: Object Factory and Template Loading
**Duration**: 4-5 days
**Status**: ⏳ PLANNED
**Dependencies**: Phase 52 complete (INI data loaded)

---

## Overview

Phase 53 initializes the core object management systems that handle game entities (units, buildings, projectiles, etc.).

---

## Subsystems Sequence

| # | Subsystem | Purpose | Complexity |
|---|-----------|---------|------------|
| 29 | TheThingFactory | Master factory for all game objects | High |
| 30 | TheActionManager | Action/command handling | Medium |
| 32 | TheTerrainTypes | Terrain type definitions | Low |
| 33 | TheTerrainRoads | Road pathfinding data | Low |
| 34 | TheScience | Tech tree/science management | Medium |
| 35 | TheScienceStore | Parsed science INI data | Medium |
| 36 | TheMultiplayerSettings | Network game settings | Medium |

---

## High-Risk Subsystems

### TheThingFactory

This is the master object factory that creates ALL game entities:

- Units
- Buildings
- Projectiles
- Effects
- etc.

It depends heavily on:

1. WeaponStore (Phase 52)
2. ObjectCreationListStore (Phase 52)
3. LocomotorStore (Phase 52)
4. ArmorStore (Phase 52)

If any of these fail, TheThingFactory will crash.

### TheActionManager

Handles all player actions/commands. Cross-platform issues may include:

- Keyboard binding differences
- Mouse input handling
- Network command serialization

---

## Tasks

### Task 53.1: TheThingFactory Initialization

Verify all dependencies loaded before ThingFactory initializes.

### Task 53.2: Object Template Parsing

Test object creation with sample templates:

- Humvee
- Ranger
- Supply Depot
- Command Center

### Task 53.3: TheActionManager Verification

Ensure action system works with SDL2 input.

### Task 53.4: Terrain Systems

Verify terrain type and road data loading.

### Task 53.5: Science/Tech Tree

Test science tree parsing and research mechanics.

---

## Debugging Commands

```bash
# Run with object creation logging
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase53_objects.log

# Filter for factory messages
grep -i "factory\|template\|thing" logs/phase53_objects.log
```

---

## Success Criteria

- [ ] TheThingFactory initializes correctly
- [ ] Sample objects can be created
- [ ] Action system responds to input
- [ ] Terrain systems load properly
- [ ] Science tree parses correctly
- [ ] Game progresses to display initialization

---

**Created**: November 2025
