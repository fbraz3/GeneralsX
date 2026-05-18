# Windows64 Task 08 - CI and Documentation

## Scope

Execute CI and contributor documentation deliverables to close the Windows64 workstream.

## Files

- Windows CI workflow files
- `docs/BUILD/WINDOWS.md`
- Relevant planning, support, and report documents

## Deliverable

- A reproducible CI build and canonical Windows64 build documentation.

## Implementation Reading (LLM)

- `.github/workflows/ci.yml`
- `.github/workflows/replay-tests.yml`
- `docs/BUILD/LINUX.md`
- `docs/BUILD/MACOS.md`
- `docs/BUILD/WINDOWS.md`
- `docs/WORKDIR/planning/PLAN-WINDOWS64_TASK_07_BUNDLE_AND_RUNTIME_VALIDATION.md`

## Execution Outputs

- Windows64 MinGW CI workflow stages configure/build/smoke.
- Canonical Windows setup/build/troubleshooting docs published and validated.

## Checklist

- Add a Windows64 MinGW pipeline.
- Publish setup and troubleshooting guidance.
- Keep Linux and macOS regression steps documented as mandatory gates.

## Done Criteria

- CI lane is green for Windows64 on target branch.
- Fresh environment can follow docs and produce a working build.