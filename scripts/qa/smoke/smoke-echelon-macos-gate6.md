# macOS Gate 6 echelon evidence (PLAN-023)

Linux host cannot produce Mach-O / MoltenVK smoke locally. Use a Mac or CI.

## Native Mac

```bash
cmake --preset macos-vulkan -DRTS_BUILD_MIGRATION_BINARIES=ON
cmake --build build/macos-vulkan --target echelon -j"$(sysctl -n hw.ncpu)"
./scripts/build/macos/deploy-macos-echelon.sh build/macos-vulkan
./scripts/qa/smoke/smoke-echelon-dual-profile.sh build/macos-vulkan
./scripts/qa/smoke/smoke-echelon-mod-path.sh build/macos-vulkan
./scripts/qa/smoke/smoke-echelon-record-replay.sh build/macos-vulkan zh
./scripts/qa/smoke/smoke-echelon-record-replay.sh build/macos-vulkan generals
```

Optional full battery (same scripts as Linux; set game-data ENV as on Linux):

```bash
./scripts/qa/smoke/smoke-echelon-gate6.sh build/macos-vulkan
```

## CI (GitHub Actions)

1. Push `echelon/unified-engine-plan` (or open a PR).
2. Dispatch **Build macOS** (`macos-vulkan`) — configure uses
   `-DRTS_BUILD_MIGRATION_BINARIES=ON` and the ZH matrix leg also builds `echelon`.
3. Optionally dispatch **GeneralsX CI** for macOS replay tests against the ZH zip.

Record run URL + SHA in `docs/WORKLOG/YYYY-MM-DIARY.md` when green.
