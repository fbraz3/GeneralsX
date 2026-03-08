# VS Code Tasks.json Migration Report
**Date**: 8 de março de 2026  
**Author**: GeneralsX Refactor Team  
**Status**: ⚠️ PARTIAL - Bash tasks updated, PowerShell scripts missing

## Summary

After reorganizing the `scripts/` folder from flat structure into functional categories, the `.vscode/tasks.json` file needs synchronization. This report documents the migration status and blocking issues.

## Migration Status

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

### ❌ BLOCKING ISSUE - PowerShell Scripts Missing

The following 7 Windows PowerShell script tasks reference files that **no longer exist** in the codebase:

| Task                                               | Command File                      | Status       |
|----------------------------------------------------|-----------------------------------|--------------|
| [Windows] Configure                                | `configure_cmake_modern.ps1`      | ❌ MISSING   |
| [Windows] Build GeneralsXZH                        | `build_zh_modern.ps1`             | ❌ MISSING   |
| [Windows] Build GeneralsX                          | `build_zh_modern.ps1`             | ❌ MISSING   |
| [Windows] Deploy GeneralsXZH                       | `deploy_zh_modern.ps1`            | ❌ MISSING   |
| [Windows] Deploy GeneralsX                         | `deploy_zh_modern.ps1`            | ❌ MISSING   |
| [Windows] Run GeneralsXZH                          | `run_zh_modern.ps1`               | ❌ MISSING   |
| [Windows] Run GeneralsX                            | `run_zh_modern.ps1`               | ❌ MISSING   |
| [Windows] Pipeline: Configure + Build + Deploy + Run ZH  | `pipeline_win64_modern.ps1` | ❌ MISSING   |
| [Windows] Pipeline: Configure + Build + Deploy + Run Generals | `pipeline_win64_modern.ps1` | ❌ MISSING   |

### Evidence

**Git Investigation**:
- Last known PowerShell script edits: commit `b5bed80a2707f90b0e4670e3f7b9184b8a8f57c3` (28/02/2026)
- These files were in `scripts/` directory but were **NOT included** in the reorganization (commit `c10e2769f`)
- Attempted to locate them in `main` branch: **NOT FOUND**
- Conclusion: PowerShell scripts were either deleted, lost during merge, or never committed to main

**Current State**:
```bash
$ find . -maxdepth 2 -name "*.ps1"
# (no results)
```

## Recommendations

### Option A: Recover Scripts (Preferred)
1. Check historical branches for PowerShell scripts
2. Determine if deletion was intentional or accidental
3. Restore and reorganize them into `scripts/build/windows/` or `scripts/legacy/`
4. Update `tasks.json` with correct paths

### Option B: Remove Tasks (Short-term Workaround)
1. Comment out or remove all 9 Windows PowerShell tasks from `tasks.json`
2. Add a note: `// Windows (PowerShell) tasks require scripts to be restored`
3. Plan recovery for next sprint

### Option C: Create Stub Scripts
1. Create minimal stub scripts that output "NOT IMPLEMENTED"
2. Move them to `scripts/build/windows/` 
3. Task execution will fail gracefully with clear error message
4. Lower priority than options A/B

## Next Steps

**Before merging this branch:**
1. Decide on action for PowerShell scripts (A, B, or C above)
2. If Option A: Restore and reorganize
3. If Option B or C: Update tasks.json accordingly
4. Commit tasks.json update with this report as reference

**Long-term:**
- Document Windows build workflow in `docs/WORKDIR/support/`
- Consider integrating GitHub Actions for cross-platform CI/CD

## Files Modified

- `.vscode/tasks.json` - Updated 14 bash/shell script paths
- `docs/WORKDIR/reports/2026-03-08-tasks-json-migration.md` - This report

---

**Related**:
- Branch: `refactor/organize-scripts-folder`
- Commit (reorganization): `c10e2769f`
- Commit (docs): `9dfabf8d0`
