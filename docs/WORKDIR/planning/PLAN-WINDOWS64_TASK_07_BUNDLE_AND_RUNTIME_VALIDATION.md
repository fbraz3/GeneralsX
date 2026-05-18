# Windows64 Task 07 - Bundle and Runtime Validation

## Scope

Execute runtime bundle layout and validation for Windows64.

## Files

- `scripts/build/windows/*`
- Windows bundle staging paths
- Smoke-test support scripts

## Deliverable

- A repeatable Windows64 deploy/run/bundle flow with smoke validation.

## Implementation Reading (LLM)

- `scripts/build/linux/`
- `scripts/build/macos/`
- `cmake/dx8.cmake`
- `resources/CMakeLists.txt`
- `docs/BUILD/LINUX.md`
- `docs/BUILD/MACOS.md`

## Execution Outputs

- Windows build/deploy/run/bundle scripts implemented in `scripts/build/windows/`.
- Runtime artifact layout captured in scripts and validated by smoke launch.

## Checklist

- Define the exact runtime DLL set.
- Add build, deploy, run, and bundle scripts.
- Add a smoke command with `-win -noshellmap`.

## Done Criteria

- Single-command Windows64 bundle flow produces runnable output.
- Smoke run succeeds from bundled/runtime layout.