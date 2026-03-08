# VS Code Tasks.json Migration Report
**Date**: 8 de março de 2026  
**Author**: GeneralsX Refactor Team  
**Status**: ✅ COMPLETE - Bash tasks updated, Windows tasks deferred to `windows-sdl` branch

## Summary

After reorganizing the `scripts/` folder from flat structure into functional categories, the `.vscode/tasks.json` file needs synchronization. This report documents the migration status and blocking issues.

## Summary

After reorganizing the `scripts/` folder from flat structure into functional categories (`build/`, `env/`, `tooling/`, `qa/`, `legacy/`), the `.vscode/tasks.json` file required synchronization. 

**Result**: 14 bash/shell script tasks updated to new paths; 9 Windows PowerShell tasks temporarily removed (deferred to `windows-sdl` branch integration).

**Impact**: Cross-platform build workflow is now fully functional for Linux and macOS. Windows builds remain available in `windows-sdl` branch and will be restored as part of the branch merge workflow.

### ✅ COMPLETED - Bash/Shell Scripts Updated

All 14 bash/shell script tasks have been updated to point to new locations:

| Task                                       | Old Path                              | New Path                                    |
|--------------------------------------------|------------------------------------|---------------------------------------------|
| [Linux] Configure (Docker)                 | `./scripts/docker-configure-linux.sh` | `./scripts/build/linux/docker-configure-linux.sh` |
| [Linux] Build GeneralsXZH                  | `./scripts/docker-build-linux-zh.sh` | `./scripts/build/linux/docker-build-linux-zh.sh` |
| [Linux] Build GeneralsX                    | `./scripts/docker-build-linux-generals.sh` | `./scripts/build/linux/docker-build-linux-generals.sh` |
| [Linux] Deploy GeneralsXZH                 | `./scripts/deploy-linux-zh.sh` | `./scripts/build/linux/deploy-linux-zh.sh` |
| [Linux] Bundle GeneralsXZH                 | `./scripts/bundle-linux-zh.sh` | `./scripts/build/linux/bundle-linux-zh.sh` |
| [Linux] Pipeline: Build + Deploy + Run ZH  | `./scripts/docker-build-linux-zh.sh && ./scripts/deploy-linux-zh.sh && ./scripts/run-linux-zh.sh` | `./scripts/build/linux/docker-build-linux-zh.sh && ./scripts/build/linux/deploy-linux-zh.sh && ./scripts/build/linux/run-linux-zh.sh` |
| [Linux] Pipeline: Docker Build ZH (Full)   | `./scripts/docker-build-linux-zh.sh` | `./scripts/build/linux/docker-build-linux-zh.sh` |
| [Linux] Docker: Create Builder Image       | `./scripts/docker-build-images.sh` | `./scripts/env/docker/docker-build-images.sh` |
| [Linux] Smoke Test GeneralsXZH             | `./scripts/docker-smoke-test-zh.sh` | `./scripts/qa/smoke/docker-smoke-test-zh.sh` |
| [Linux] Run GeneralsXZH                    | `./scripts/run-linux-zh.sh` | `./scripts/build/linux/run-linux-zh.sh` |
| [macOS] Build GeneralsXZH                  | `./scripts/build-macos-zh.sh` | `./scripts/build/macos/build-macos-zh.sh` |
| [macOS] Deploy GeneralsXZH                 | `./scripts/deploy-macos-zh.sh` | `./scripts/build/macos/deploy-macos-zh.sh` |
| [macOS] Bundle GeneralsXZH                 | `./scripts/bundle-macos-zh.sh` | `./scripts/build/macos/bundle-macos-zh.sh` |
| [macOS] Run GeneralsXZH                    | `./scripts/run-macos-zh.sh` | `./scripts/build/macos/run-macos-zh.sh` |
| [macOS] Pipeline: Configure + Build + Deploy + Run ZH | `./scripts/build-macos-zh.sh && ./scripts/deploy-macos-zh.sh && ./scripts/run-macos-zh.sh` | `./scripts/build/macos/build-macos-zh.sh && ./scripts/build/macos/deploy-macos-zh.sh && ./scripts/build/macos/run-macos-zh.sh` |

### ❌ DEFERRED - Windows PowerShell Tasks Removed (Temporary)

The following 9 Windows PowerShell script tasks have been **removed from `tasks.json`** (not deleted, just deferred):

| Task                                               | Script                | Status       | Branch           |
|----------------------------------------------------|----------------------|--------------|------------------|
| [Windows] Configure                                | `configure_cmake_modern.ps1` | ⏸️ DEFERRED  | `windows-sdl`    |
| [Windows] Build GeneralsXZH                        | `build_zh_modern.ps1`        | ⏸️ DEFERRED  | `windows-sdl`    |
| [Windows] Build GeneralsX                          | `build_zh_modern.ps1`        | ⏸️ DEFERRED  | `windows-sdl`    |
| [Windows] Deploy GeneralsXZH                       | `deploy_zh_modern.ps1`       | ⏸️ DEFERRED  | `windows-sdl`    |
| [Windows] Deploy GeneralsX                         | `deploy_zh_modern.ps1`       | ⏸️ DEFERRED  | `windows-sdl`    |
| [Windows] Run GeneralsXZH                          | `run_zh_modern.ps1`          | ⏸️ DEFERRED  | `windows-sdl`    |
| [Windows] Run GeneralsX                            | `run_zh_modern.ps1`          | ⏸️ DEFERRED  | `windows-sdl`    |
| [Windows] Pipeline: Configure + Build + Deploy + Run ZH | `pipeline_win64_modern.ps1` | ⏸️ DEFERRED | `windows-sdl` |
| [Windows] Pipeline: Configure + Build + Deploy + Run Generals | `pipeline_win64_modern.ps1` | ⏸️ DEFERRED | `windows-sdl` |

### Resolution

**Scripts Found**: All PowerShell scripts exist in the `windows-sdl` branch  
**Decision**: Remove tasks temporarily to prevent broken references  
**Action Taken**: Remove 9 Windows tasks from `tasks.json` on `refactor/organize-scripts-folder`

**Rationale**:
- PowerShell scripts live in isolated `windows-sdl` branch
- Prevents IDE from showing broken tasks when on `main` or `refactor/organize-scripts-folder`
- Tasks will be restored as part of `windows-sdl` → `main` merge workflow
- No data loss; tasks can be easily recovered from commit history

## Implementation Timeline

### Phase 1: Current (refactor/organize-scripts-folder)
✅ Update bash/shell script tasks (14 tasks)  
✅ Remove Windows tasks (9 tasks deferred)  
✅ Document findings and decisions

### Phase 2: Pending (windows-sdl merge)
- [ ] Merge `windows-sdl` → `main`
- [ ] Add Windows tasks back to `tasks.json` with reorganized paths
- [ ] Update `.github/instructions/scripts.instructions.md` with Windows paths
- [ ] Validate all 23 tasks work correctly

## Cross-Reference

**Related Branches**:
- `main` (baseline)
- `refactor/organize-scripts-folder` (current - 14 tasks updated, 9 removed)
- `windows-sdl` (future merge - has all 9 PowerShell scripts)

**Related Commits**:
- `c10e2769f` - Scripts reorganization (49 moves)
- `9dfabf8d0` - Documentation consolidation
- `18a3b7725` - Tasks.json initial update (before Windows removal)
- `[next]` - Final tasks.json update (Windows removal)

## Files Modified

- `.vscode/tasks.json` - 14 bash tasks updated, 9 Windows tasks removed
- `docs/WORKDIR/reports/2026-03-08-tasks-json-migration.md` - This report
