# Windows64 Task 07 - Bundle and Runtime Validation

## Scope

Create the runtime bundle layout and validation steps for Windows64.

## Files

- `scripts/build/windows/*`
- Windows bundle staging paths
- Smoke-test support scripts

## Deliverable

- A repeatable Windows64 deploy/run/bundle flow with smoke validation.

## Checklist

- Define the exact runtime DLL set.
- Add build, deploy, run, and bundle scripts.
- Add a smoke command with `-win -noshellmap`.