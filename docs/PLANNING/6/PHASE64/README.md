# Phase 64: Gameplay Testing - First Playable Build

**Phase**: 64
**Title**: Complete Gameplay Loop Verification
**Duration**: 5-7 days
**Status**: NOT STARTED
**Dependencies**: Phase 62 complete (Models rendering), Phase 63 optional (Audio)

---

## Overview

Phase 64 is a **validation phase** that tests the complete gameplay loop. This is where we verify that the game is actually playable - not just rendering, but responding to input and simulating combat.

### Current State (Post-Phase 62)

- ✅ UI/Menu functional
- ✅ Terrain renders
- ✅ Units/Buildings render
- ❓ Game logic running?
- ❓ Units respond to commands?
- ❓ Combat works?

### Goal

Verify complete gameplay: Start skirmish → Build base → Train units → Fight enemy → Win/Lose

---

## Gameplay Loop Components

### 1. Game Initialization

```
Main Menu → Skirmish Setup → Map Load → Game Start
```

Verify:

- Map selection works
- Faction selection works
- AI opponent configurable
- Game loads and starts

### 2. Base Building

```
Build Dozer → Construct buildings → Unlock tech tree
```

Verify:

- Construction Dozer spawns
- Build menu appears
- Buildings can be placed
- Construction completes

### 3. Resource Gathering

```
Supply Depot → Chinook/Worker → Supplies collected
```

Verify:

- Supply source detected
- Gatherer assigned
- Resources increment
- UI shows correct count

### 4. Unit Production

```
Barracks → Train infantry → Units spawn
```

Verify:

- Production queue works
- Build time progresses
- Units appear at rally point
- Population count updates

### 5. Combat

```
Select units → Attack move → Engage enemy
```

Verify:

- Unit selection works
- Attack command issues
- Units engage enemies
- Damage applies
- Units die when health = 0

### 6. Victory/Defeat

```
Destroy enemy base → Victory screen
Lose all buildings → Defeat screen
```

Verify:

- Win condition triggers
- Loss condition triggers
- End game screen appears

---

## Testing Protocol

### Test Session 1: Basic Gameplay (2 hours)

1. Start Skirmish vs Easy AI
2. Build basic base (Command Center, Supply Depot, Barracks)
3. Train 10 infantry
4. Attack enemy base
5. Observe combat
6. Document any issues

### Test Session 2: Full Game (4 hours)

1. Play complete game to victory
2. Test all three factions (USA, China, GLA)
3. Test multiple maps
4. Document crashes, glitches, balance issues

### Test Session 3: Edge Cases (2 hours)

1. Max unit count
2. Very long game (1+ hour)
3. Rapid commands
4. Save/Load (if implemented)

---

## Common Issues and Fixes

### Issue: Units Not Responding

Possible causes:

- AI pathfinding broken
- Command queue not processing
- Animation system not updating

Debug:

```bash
grep -i "pathfind\|command\|order" logs/phase64_gameplay.log
```

### Issue: Combat Not Working

Possible causes:

- Damage calculation errors
- Weapon range incorrect
- Target acquisition broken

Debug:

```bash
grep -i "damage\|weapon\|attack" logs/phase64_gameplay.log
```

### Issue: Resources Not Collecting

Possible causes:

- Supply source not detected
- Gatherer AI broken
- Resource increment not happening

Debug:

```bash
grep -i "supply\|resource\|gather" logs/phase64_gameplay.log
```

---

## Performance Baseline

Establish performance baseline during gameplay:

| Metric | Target | Minimum |
|--------|--------|---------|
| FPS (empty map) | 60 | 45 |
| FPS (mid-game) | 45 | 30 |
| FPS (large battle) | 30 | 20 |
| Memory (start) | <512 MB | <768 MB |
| Memory (mid-game) | <1 GB | <1.5 GB |
| Load time | <30 sec | <60 sec |

### Measurement Commands

```bash
# Monitor FPS (if game outputs)
grep -i "fps\|frame" logs/phase64_gameplay.log | tail -20

# Monitor memory
top -pid $(pgrep GeneralsXZH) -l 1
```

---

## Bug Triage Categories

### Critical (Blocks Gameplay)

- Game crash
- Cannot start skirmish
- Units don't move
- Combat doesn't work

### High (Major Issues)

- AI doesn't attack
- Buildings don't complete
- Resources don't collect
- Victory condition broken

### Medium (Annoying)

- Visual glitches
- Audio issues
- UI element misaligned
- Performance drops

### Low (Polish)

- Text typos
- Minor graphical issues
- Animation glitches

---

## Documentation Requirements

After each test session, document:

1. **Session Summary**: Duration, objectives, outcomes
2. **Bugs Found**: Category, reproduction steps, screenshots
3. **Performance Data**: FPS, memory, load times
4. **Recommendations**: Priority fixes for next phase

---

## Success Criteria

- [ ] Can start skirmish game
- [ ] Can build all basic structures (USA faction)
- [ ] Can train all basic units (USA faction)
- [ ] Resources collect correctly
- [ ] Units respond to move commands
- [ ] Units respond to attack commands
- [ ] Combat deals damage
- [ ] Units can die
- [ ] Can win game (destroy enemy)
- [ ] Can lose game (be destroyed)
- [ ] No crashes during 30-minute game
- [ ] FPS stays above 30 during gameplay

---

## Blockers and Dependencies

### Must Work Before Phase 64

- Vulkan rendering (Phase 54-62)
- Input handling (SDL2 - Phase 50)
- Game logic initialization (Phase 55)

### Nice to Have

- Audio (Phase 63) - Game playable without sound
- Multiplayer - Test single-player first

---

## Next Steps After Phase 64

If Phase 64 succeeds:

- **Phase 65**: All factions playable
- **Phase 66**: Campaign mode
- **Phase 67**: Multiplayer (LAN)
- **Phase 68**: Performance optimization
- **Phase 69**: Polish and bug fixes
- **Phase 70**: Release preparation

If Phase 64 reveals major issues:

- Create targeted fix phases (64.1, 64.2, etc.)
- Address blockers before proceeding
- Re-test after fixes

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
