# Phase 40: Game Logic & Gameplay Systems (FASE D - LONGO PRAZO)

**Status**: Ready (after Phase 39)  
**Estimated Time**: 5-7 days  
**Priority**: 🎯 **CRITICAL** - Core gameplay functionality

## Overview

Implement core gameplay systems: map loading, unit/building rendering, AI state machines, pathfinding, collision detection, and game input handling. This phase transforms the game from menu-only to a playable RTS experience.

## Current Status

✅ **What's Working**:
- Game initialization and shutdown
- Menu system (Phase 39)
- Audio events (Phase 38)
- Asset loading (Phase 37)
- Metal rendering pipeline

❌ **What's Missing**:
- Gameplay map loading
- Unit/building rendering
- Camera control and movement
- Player input (unit selection, command issuance)
- AI opponent behavior
- Pathfinding and unit movement
- Collision detection
- Resource management
- Win/loss conditions

## Tasks

### Phase 40.1: Map Loading & Scene Rendering (1.5 days)

**Objective**: Load game map and render terrain/buildings/units

**Checklist**:

- [ ] Load .map file format
  - [ ] Parse map header (width, height, terrain type)
  - [ ] Load terrain mesh from map data
  - [ ] Load static objects (buildings, trees, rocks)
  - [ ] Load initial unit placements

- [ ] Render terrain system
  - [ ] Terrain mesh with textures
  - [ ] Texture blending at terrain transitions
  - [ ] Dynamic LOD for far terrain
  - [ ] No rendering artifacts or holes

- [ ] Render static scene objects
  - [ ] Building models with textures
  - [ ] Environmental props (trees, rocks, etc.)
  - [ ] Destructible object states
  - [ ] Correct depth-sorting

- [ ] Verify rendering performance
  - [ ] Frame rate stable (60+ FPS) on typical map
  - [ ] No excessive draw calls
  - [ ] Appropriate LOD transitions

**Files to Modify**:
- `Map.cpp` - Map file loading
- `TerrainRender.cpp` - Terrain rendering
- `SceneRender.cpp` - Scene object rendering
- Metal shaders - Terrain/object-specific shaders

**Commands**:

```bash
# Build with map loading
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Load skirmish map
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 30 ./GeneralsXZH 2>&1 | tee /tmp/phase40_map_load.log

# Verify terrain rendering
grep -i "map\|terrain\|load" /tmp/phase40_map_load.log | head -30
```

### Phase 40.2: Unit & Player Object Systems (1.5 days)

**Objective**: Render and manage in-game units and buildings

**Checklist**:

- [ ] Unit object system
  - [ ] Load unit models (soldier, tank, aircraft)
  - [ ] Render units with animations
  - [ ] Unit health/damage visualization
  - [ ] Team color/faction differentiation

- [ ] Building system
  - [ ] Load building models (barracks, factory, power plant)
  - [ ] Render building construction
  - [ ] Building health visualization
  - [ ] Destruction and rebuild states

- [ ] Selection visualization
  - [ ] Selected unit highlight
  - [ ] Selection box or outline
  - [ ] Multiple unit selection indicators
  - [ ] UI panel showing selected unit info

- [ ] Particle effects
  - [ ] Muzzle flashes on fire
  - [ ] Explosions and impact effects
  - [ ] Smoke from damaged units
  - [ ] Water splashes (if applicable)

**Files to Modify**:
- `Object.cpp` - Base object class
- `Unit.cpp` - Unit-specific behavior
- `Building.cpp` - Building-specific behavior
- `ObjectRender.cpp` - Rendering logic
- Particle effect system

**Commands**:

```bash
# Build with unit rendering
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Load map with units
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 60 ./GeneralsXZH 2>&1 | tee /tmp/phase40_units.log

# Check unit rendering
grep -i "unit\|object\|render" /tmp/phase40_units.log | head -30
```

### Phase 40.3: Player Input & Unit Control (1.5 days)

**Objective**: Implement user interaction for unit control

**Checklist**:

- [ ] Camera control
  - [ ] Mouse drag to pan camera
  - [ ] Mouse wheel to zoom
  - [ ] Keyboard arrows to move camera
  - [ ] Boundary checking (can't pan past map edges)

- [ ] Unit selection
  - [ ] Left-click selects unit
  - [ ] Shift-click adds to selection
  - [ ] Drag box for area selection
  - [ ] Click empty space to deselect

- [ ] Unit commands
  - [ ] Right-click move command
  - [ ] Attack command on enemy
  - [ ] Guard mode (follow unit)
  - [ ] Ability shortcuts (number keys)

- [ ] UI feedback
  - [ ] Cursor changes (move, attack, invalid)
  - [ ] Command audio feedback (Phase 38)
  - [ ] Selection sounds
  - [ ] Command acknowledgement sounds

**Files to Modify**:
- `Camera.cpp` - Camera control
- `InputSystem.cpp` - Input interpretation
- `CommandSystem.cpp` - Command issuance
- `GameLogic.cpp` - Command execution

**Commands**:

```bash
# Build with input system
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Interactive input test
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 60 ./GeneralsXZH 2>&1 | tee /tmp/phase40_input.log

# Verify input handling
grep -i "input\|command\|select" /tmp/phase40_input.log | head -20
```

### Phase 40.4: AI & Pathfinding (2 days)

**Objective**: Implement basic AI opponent and unit pathfinding

**Checklist**:

- [ ] Pathfinding system
  - [ ] A* algorithm implementation
  - [ ] Waypoint generation
  - [ ] Collision avoidance
  - [ ] Formation movement

- [ ] Unit AI behavior
  - [ ] Idle state (standing around)
  - [ ] Moving state (follow waypoints)
  - [ ] Combat state (attack target)
  - [ ] State transitions

- [ ] Opponent AI (basic)
  - [ ] Build units periodically
  - [ ] Expand to new bases
  - [ ] Defend against attacks
  - [ ] Simple target acquisition

- [ ] Win/Loss conditions
  - [ ] Eliminate all opponent units = WIN
  - [ ] Lose all structures = LOSS
  - [ ] Victory screen with stats
  - [ ] Return to menu option

**Files to Modify**:
- `Pathfind.cpp` - Pathfinding algorithm
- `AIPlayer.cpp` - AI decision making
- `GameLogic.cpp` - Victory/defeat logic
- `UnitAI.cpp` - Unit behavior state machine

**Commands**:

```bash
# Build with AI system
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Run gameplay session
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 120 ./GeneralsXZH 2>&1 | tee /tmp/phase40_ai.log

# Verify AI activity
grep -i "ai\|pathfind\|move\|attack" /tmp/phase40_ai.log | head -30
```

## Success Criteria

- ✅ Map loads and renders terrain without crashes
- ✅ 10+ units visible on map (both player and opponent)
- ✅ Unit selection works (click to select, shows highlight)
- ✅ Camera can be controlled (pan, zoom, move)
- ✅ Right-click move command works (units follow path)
- ✅ Simple combat between units (targeting and firing)
- ✅ Game can end (player victory or defeat)
- ✅ Frame rate stable (30-60 FPS) during gameplay
- ✅ No crashes for 5+ minute gameplay session

## Known Challenges

1. **Pathfinding Performance**
   - A* can be slow with large maps
   - Solution: Use waypoint graphs instead of full A*
   - Cache common paths

2. **AI Decision Making**
   - Original game has complex AI (influence maps, goal planning)
   - Solution: Start with simple reactive AI
   - Expand as time permits

3. **Formation Movement**
   - Units should move in groups, not individually
   - Solution: Implement loose formation system
   - Units follow leader with spacing

4. **Resource Management**
   - Tracking energy, credits, unit caps
   - Solution: Simple resource tracking system
   - No complex economy simulation initially

## Debugging Strategy

1. **Enable gameplay logging**:
   - Unit state transitions
   - Pathfinding calculations
   - AI decisions
   - Command execution

2. **Use in-game debug visualization**:
   - Show pathfinding waypoints
   - Display unit AI state
   - Show selected objects
   - Visualize collision detection

3. **Test incrementally**:
   - Start with single unit controllable
   - Add pathfinding
   - Add opponent AI
   - Test combat

## Files to Monitor

| File | Purpose | Priority |
|------|---------|----------|
| `Map.cpp` | Map loading | HIGH |
| `Unit.cpp` | Unit behavior | HIGH |
| `Camera.cpp` | Camera control | HIGH |
| `Pathfind.cpp` | Unit pathfinding | HIGH |
| `AIPlayer.cpp` | Opponent AI | MEDIUM |
| `GameLogic.cpp` | Game state | MEDIUM |

## References

- Original game source code - Game logic patterns
- RTS game architecture tutorials
- A* pathfinding algorithm references
- Formation movement systems

## Dependencies

✅ **Complete**:
- Phase 30: Metal Backend
- Phase 31: Texture System
- Phase 37: Asset Loading
- Phase 38: Audio System
- Phase 39: UI/Menu System

⏳ **Blocking Phase XX**:
- Phase 40 must complete for multiplayer (Phase XX) to function

## Next Phase

**Phase XX**: Multiplayer & Networking (requires stable single-player gameplay)

---

**Last Updated**: October 27, 2025  
**Status**: Ready after Phase 39 completion
