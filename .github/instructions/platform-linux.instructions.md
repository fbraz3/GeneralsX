---
applyTo: 'scripts/build/linux/**,scripts/env/docker/**,GeneralsMD/Code/CompatLib/**'
---

## Linux Build Environment

**Native**:
```bash
sudo apt install build-essential cmake ninja-build git
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals
```

**Docker (from any host)**:
```bash
./scripts/build/linux/docker-configure-linux.sh linux64-deploy
./scripts/build/linux/docker-build-linux-zh.sh linux64-deploy
```

## Run & Test

```bash
./scripts/build/linux/deploy-linux-zh.sh
./scripts/build/linux/run-linux-zh.sh -win
./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy

# GDB backtrace
mkdir -p logs && gdb -batch -ex "run -win" -ex "bt full" -ex "thread apply all bt" \
  ./build/linux64-deploy/GeneralsMD/GeneralsXZH 2>&1 | tee logs/gdb.log
```

## Linux-Specific Notes

- **Case-sensitive filesystem**: Include paths must match exact case. Use `scripts/tooling/cpp/maintenance/fixIncludesCase.sh`.
- **DXVK requires Vulkan**: `vulkan-tools`, `mesa-vulkan-drivers`, or proprietary GPU drivers.
- **SDL3**: fetched via CMake FetchContent — no system package needed.
- **DXVK source policy**: fixes go in `references/fbraz3-dxvk`, never in `build/_deps/...`.
- **CompatLib**: `GeneralsMD/Code/CompatLib/` provides Win32 API compatibility shims (`windows_compat.h`).
- **No native POSIX calls**: use SDL3 abstractions for timers, threads, file I/O. No raw `pthread_*`, `open()`.
- **`-logToCon`**: only available in debug builds (`RTS_BUILD_OPTION_DEBUG=ON`).
- **Diagnostics**: prefer `fprintf(stderr, ...)` probes; capture stderr and grep targeted markers.

```bash
# Recommended debug run
cd ~/GeneralsX/GeneralsMD
./run.sh -win -logToCon 2>&1 | grep -v "D3DRS_PATCHSEGMENTS" | tee ~/GeneralsX/logs/manual_run.log
```
