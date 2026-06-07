# Headless Replay Testing Reference

Practical reference for running and debugging GeneralsXZH replay files in headless
(no-display, no-window) mode on macOS and Linux.

---

## Overview

Headless replay mode runs the game simulation end-to-end without opening a window or
initializing a display. The binary reads the replay file, simulates all game frames,
and exits with code `0` on success or non-zero on failure (CRC mismatch, map not found,
crash, timeout, etc.).

This is the primary method for replay regression testing across platforms.

---

## Basic Usage

```bash
cd /path/to/game/dir          # e.g. ~/GeneralsX/GeneralsZH on macOS
./GeneralsXZH -headless -replay "/absolute/path/to/replay.rep"
```

> **Always use an absolute path** for `-replay`. Relative paths may resolve
> incorrectly depending on the working directory.

---

## Command-Line Parameters

| Parameter | Description |
|-----------|-------------|
| `-headless` | Run without display (no window, no GPU required) |
| `-replay <path>` | Path to the `.rep` replay file to simulate |
| `-win` | Run windowed (use for manual inspection, incompatible with headless) |
| `-noshellmap` | Skip the main menu shell map (faster startup, not needed in headless) |
| `-logToCon` | Route `DEBUG_LOG` output to console (**debug builds only**; no effect in release) |

---

## Replay File Locations

### macOS

```
~/Library/Application Support/GeneralsX/GeneralsZH/Replays/
```

### Linux

```
~/.local/share/GeneralsX/GeneralsZH/Replays/
# or, if using a custom install path:
<INSTALL_DIR>/Replays/
```

### Custom / User Maps

Custom maps must be installed in the user maps directory **before** running a replay
that uses them. The game searches for them by path first, then falls back to CRC+size
scan across the map cache.

**macOS**:
```
~/Library/Application Support/GeneralsX/GeneralsZH/Maps/<MapFolder>/<MapName>.map
```

**Linux**:
```
~/.local/share/GeneralsX/GeneralsZH/Maps/<MapFolder>/<MapName>.map
```

---

## Single Replay Test

```bash
cd ~/GeneralsX/GeneralsZH
./GeneralsXZH -headless -replay "$HOME/Library/Application Support/GeneralsX/GeneralsZH/Replays/my_replay.rep"
echo "Exit code: $?"
```

---

## Batch Test Loop

Run all replays matching a pattern and report results:

```bash
REPLAY_DIR="$HOME/Library/Application Support/GeneralsX/GeneralsZH/Replays"
GAME_DIR="$HOME/GeneralsX/GeneralsZH"
PASS=0; FAIL=0

for rep in "$REPLAY_DIR"/*.rep; do
    echo "=== $(basename "$rep") ==="
    timeout 180 "$GAME_DIR/GeneralsXZH" -headless -replay "$rep" 2>&1 \
        | grep -E "\[GeneralsX\]|Simulating|Elapsed|CRC|exit|MISMATCH|Error" \
        | tail -10
    code=$?
    if [ $code -eq 0 ]; then
        echo "PASS (exit 0)"
        PASS=$((PASS + 1))
    else
        echo "FAIL (exit $code)"
        FAIL=$((FAIL + 1))
    fi
    echo ""
done

echo "Results: $PASS passed, $FAIL failed"
```

> Adjust `REPLAY_DIR` and `GAME_DIR` for your platform and install path.

**Linux variant** (adjust paths):
```bash
REPLAY_DIR="$HOME/.local/share/GeneralsX/GeneralsZH/Replays"
GAME_DIR="$HOME/GeneralsX/GeneralsZH"
```

---

## Interpreting Output

### Success

```
Simulating Replay "/path/to/replay.rep"
Elapsed Time: 00:04 Game Time: 12:15/12:15
INFO: GameMain() returned with code 0
Exiting with code 0
```

### CRC Fallback (map resolved by CRC/size, not by path)

```
[GeneralsX] Replay map resolved via CRC fallback: CRC=0x43396DD8 size=162490 -> '/path/to/map.map'
```

This is **non-fatal** — the replay will still succeed if the map file is found. It
indicates the `M=` path in the replay header did not match directly; see the known
tech-debt issue in `REPLAY_MAPCACHE_TECH_DEBT.md`.

### CRC Mismatch (determinism failure)

```
[GeneralsX] REPLAY_CRC_MISMATCH frame=1200 inGame=0x1A2B3C4D replay=0xDEADBEEF
[GeneralsX] This replay is incompatible with the current map/game-code state.
```

Exit code will be non-zero. This means the simulation diverged from what was recorded.
Common causes: different game version, different map file, non-deterministic code path.

### Map Not Found

```
Error: Could not load map ...
```

Exit code non-zero. Ensure the map file exists at the expected path and the map cache
has been refreshed (launch the game once in normal mode to rebuild it).

### Timeout

If `timeout` kills the process before it finishes, exit code will be `124`. Increase the
timeout for long replays (12+ minute games can take 4-5 minutes to simulate headlessly).

---

## Debugging Tips

### Capture Full Log

```bash
./GeneralsXZH -headless -replay "$REP" 2>&1 | tee /tmp/replay_debug.log
```

### Filter Noise (suppress D3D spam)

```bash
./GeneralsXZH -headless -replay "$REP" 2>&1 \
    | grep -v "D3DRS_PATCHSEGMENTS" \
    | tee /tmp/replay_debug.log
```

### Check Map Cache

The map cache file is at:
- **macOS**: `~/Library/Application Support/GeneralsX/GeneralsZH/Maps/MapCache.ini`
- **Linux**: `~/.local/share/GeneralsX/GeneralsZH/Maps/MapCache.ini`

If it is missing or stale, launch the game once in normal mode to regenerate it. The
headless mode does not rebuild the cache.

### Debug Builds Only

Add `-logToCon` to route `DEBUG_LOG` macros to stderr. This flag has **no effect in
release builds**. If diagnostics are missing, add explicit `fprintf(stderr, ...)` probes
to the source and rebuild.

### GDB Backtrace (Linux)

```bash
mkdir -p logs
gdb -batch \
    -ex "run -headless -replay /path/to/replay.rep" \
    -ex "bt full" \
    -ex "thread apply all bt" \
    ./GeneralsXZH 2>&1 | tee logs/gdb_replay.log
```

### lldb Backtrace (macOS)

```bash
lldb ./GeneralsXZH -- -headless -replay "/path/to/replay.rep"
# Inside lldb:
# (lldb) run
# (lldb) bt
```

---

## Platform Notes

### macOS

- No `DISPLAY` variable needed; headless mode skips SDL window creation.
- Vulkan/MoltenVK must be available even in headless mode (DXVK initializes the device).
- Replay files recorded on macOS may use lowercase paths in `M=` header field.

### Linux

- Headless mode can run inside Docker with no display.
- Ensure Vulkan drivers are installed (`mesa-vulkan-drivers` or proprietary).
- Docker smoke test script: `./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy`
- Game and replay dirs inside Docker may differ — check the smoke test script for path mapping.

### Cross-Platform Replay Compatibility

Replays recorded on one platform can be played on another **as long as**:
- The same game version is used
- The same map file (identical CRC + size) is present
- No platform-specific non-determinism was introduced

If a replay recorded on Windows fails on Linux/macOS, check for floating-point or
path-related divergence first.

---

## Known Issues / Tech Debt

See [REPLAY_MAPCACHE_TECH_DEBT.md](REPLAY_MAPCACHE_TECH_DEBT.md) for tracked issues:

- **Custom map CRC fallback**: replays with custom maps resolve via CRC fallback instead
  of direct path match; works but slower and fragile.
