# ISSUE-002: Skirmish vs CPU Ends Immediately with Victory Screen

**Status**: OPEN  
**Session Discovered**: 53 (2026-02-21)  
**Severity**: Critical  
**Component**: Gameplay  
**Reproducibility**: 100% — Start any skirmish match against a CPU opponent  

## Symptom

Starting a skirmish game against CPU loads the map and runs for a few seconds, then the "You have won" victory screen appears and the game exits to the main menu. No combat occurs.

## Investigation Summary

No investigation done yet.

### Hypotheses (in priority order)

1. **CPU player immediately destroyed / never spawned** — If the CPU player's start location or initial units are not correctly initialized on Linux, the game may detect zero enemy units and immediately trigger a win condition. Possible root cause: INI/map parsing issue, start position mismatch, or object spawning failure.

2. **Win condition evaluator miscounts teams** — `VictoryCondition` or `TeamRelationship` logic reads player/team counts. A 64-bit ABI mismatch (pointer size, struct padding) in team management could report zero enemies from the start frame.

3. **Game logic frame counter overflow or immediate timeout** — If `TheGameLogic->getFrame()` returns a large garbage value on startup (uninitialized), a time-limit victory condition could fire instantly.

4. **Missing skirmish script / AI initialization** — Skirmish maps rely on side-specific scripts. If script loading fails silently (file not found, BIG archive not mounted), the AI may not initialize and the game declares victory for the only "active" player.

## Code Audit Results

Not yet investigated.

## Next Steps (for Future Session)

1. Add `fprintf(stderr, ...)` trace inside `VictoryConditions` evaluation to log which condition fires and on which frame.
2. Check `GameLogic::checkForVictory()` / `VictorCondition::evaluate()` — confirm enemy player is alive before triggering.
3. Verify CPU player `Player` object is created: check `ThePlayerList` count during skirmish init.
4. Check if `TheScriptEngine` completes AI initialization without error.
5. Cross-reference with fighter19 reference — does it have the same symptom?

## Workaround

None.

## Impact

- **Gameplay**: Blocking — skirmish mode completely unusable
- **Stability**: Stable (no crash, clean exit)
- **Determinism**: N/A
- **Release blocker**: Yes

## Reference

- Session 53 discovery (2026-02-21)
