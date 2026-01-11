# PHASE05: Stability, Performance, and Gameplay Validation

## Objective
Harden the SDL2 runtime path until it meets the primary-goal acceptance criteria.

## Checklist
- [ ] Fix remaining platform edge cases (focus, fullscreen, input capture).
- [ ] Ensure frame pacing and minimized behavior do not cause CPU spikes.
- [ ] Validate audio focus behavior and recovery.
- [ ] Run the full acceptance test matrix:
  - [ ] Assets load
  - [ ] Main menu navigation
  - [ ] Skirmish vs AI (start → play → victory/defeat)
  - [ ] Campaign mission (start → play → completion)
  - [ ] Generals Challenge (start → play → completion)
- [ ] Produce a session report summarizing remaining gaps and next actions.

## Exit Criteria
- All primary-goal acceptance criteria pass on macOS/Linux, and Windows remains functional.
