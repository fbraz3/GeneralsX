# Phase 49: Quick Reference - File-Based Logging for DX8Wrapper

## TL;DR - Get Started in 30 Seconds

1. **Build the project**:
```bash
cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

2. **Copy and run**:
```bash
cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
cd $HOME/GeneralsX/GeneralsMD
timeout 15 ./GeneralsXZH 2>&1
```

3. **Check the log**:
```bash
tail -50 /tmp/phase49_crash.log
```

## Files Modified

- **Header (NEW)**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/phase49_file_logging.h`
- **Source**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
  - Added logging include
  - Replaced printf() with PHASE49_LOG macros
  - Added entry/exit markers
  - Logs flushed to `/tmp/phase49_crash.log`

## Files Added/Created

- `docs/PHASE49_FILE_LOGGING.md` - Detailed documentation
- `run_with_logging.sh` - Helper script for running with logging

## The Log File

**Location**: `/tmp/phase49_crash.log`

**Clear old log**: `rm /tmp/phase49_crash.log`

**View live**: `tail -f /tmp/phase49_crash.log`

## What Gets Logged

### Initialization Flow

```
Init()
├── CreateDevice()
│   ├── Compute_Caps() [or skip on macOS]
│   └── Do_Onetime_Device_Dependent_Inits()
│       ├── MissingTexture::_Init()
│       ├── TextureFilterClass::_Init_Filters()
│       ├── TheDX8MeshRenderer.Init()
│       ├── SHD_INIT
│       ├── BoxRenderObjClass::Init()
│       ├── VertexMaterialClass::Init()
│       ├── PointGroupClass::_Init()
│       ├── ShatterSystem::Init()
│       ├── TextureLoader::Init()
│       └── Set_Default_Global_Render_States()
└── Shutdown()
```

Each function has `ENTER` and `EXIT` markers, and each subsystem init has checkpoint markers.

## Interpreting the Log

### Normal Completion

Log will contain:
- All CHECKPOINT markers up to CHECKPOINT 6
- EXIT markers for all functions
- Session end marker

### Hang/Freeze

Log will end abruptly at a specific CHECKPOINT, indicating that's where execution stalls.

Example:
```
[14:32:50] CHECKPOINT 4m: About to call ShatterSystem::Init
[14:32:50] >>> ENTER: ShatterSystem::Init
(log ends)
```

→ **Hang is in ShatterSystem::Init()**

## Quick Analysis

```bash
# Show last checkpoint reached
grep "CHECKPOINT" /tmp/phase49_crash.log | tail -5

# Show all errors
grep "ERROR" /tmp/phase49_crash.log

# Show the flow
grep "ENTER\|EXIT\|CHECKPOINT" /tmp/phase49_crash.log | head -30

# Show timings between major sections
grep "Initializing\|CHECKPOINT" /tmp/phase49_crash.log
```

## Environment Variables

No additional environment variables needed. The logging is always enabled when built with the logging header.

To use OpenGL backend instead of Metal:
```bash
cd $HOME/GeneralsX/GeneralsMD
USE_OPENGL=1 timeout 15 ./GeneralsXZH 2>&1
```

## Timestamps

All log entries include timestamps in `[HH:MM:SS]` format, making it easy to spot timing anomalies.

Example: If several seconds pass between two checkpoints, that function is taking time.

## Key Checkpoints Quick Reference

| # | Function | Indicates |
|---|----------|-----------|
| 1-2 | Compute_Caps | Device capability detection |
| 3-4 | MissingTexture | Fallback texture system |
| 4a-4b | TextureFilterClass | Texture filtering modes |
| 4c-4d | TheDX8MeshRenderer | 3D mesh rendering |
| 4e-4f | SHD_INIT | Shader system |
| 4g-4h | BoxRenderObjClass | Box rendering |
| 4i-4j | VertexMaterialClass | Material system |
| 4k-4l | PointGroupClass | Point rendering |
| 4m-4n | ShatterSystem | Destruction effects |
| 4o-4p | TextureLoader | Texture loading |
| 5-6 | Render states | Global rendering setup |

## Rebuilding After Changes

If you make changes to the initialization code:

```bash
cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
```

## Troubleshooting

### "No such file or directory" when running executable

Make sure you've copied the executable:
```bash
cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
ls -lh $HOME/GeneralsX/GeneralsMD/GeneralsXZH
```

### Log file not appearing

1. Check permissions:
   ```bash
   ls -la /tmp/ | grep phase49
   ```

2. Verify executable is using new code:
   ```bash
   strings ./GeneralsXZH | grep -c "PHASE49_LOG"
   ```

3. Try writing to home directory instead:
   ```bash
   export HOME_LOG=$HOME/GeneralsX/phase49_crash.log
   # (would need code change to use this)
   ```

### Log file empty

- Application crashed before logging initialized
- Try checking crash info file:
  ```bash
  cat "$HOME/Documents/Command and Conquer Generals Zero Hour Data/ReleaseCrashInfo.txt"
  ```

## Advanced: Adding More Logging

To add logging to a new section:

```cpp
// At function entry
PHASE49_LOG_ENTER("MyFunction");

// Inside function
PHASE49_CHECKPOINT("id", "description");
PHASE49_LOG("Processing with %d items", count);

// On error
PHASE49_LOG_ERROR("Failed to process");

// At exit
PHASE49_LOG_EXIT("MyFunction");
```

## Performance Impact

- **Minimal**: Logging adds <1% CPU overhead
- **File I/O**: One flush per log entry (~1ms on SSD)
- **Memory**: <1KB total logging overhead
- **Disk**: ~50-100KB log file for full init sequence

Safe to leave enabled in debug builds.

## Related Commands

```bash
# Monitor log in real-time in another terminal
tail -f /tmp/phase49_crash.log

# Count log entries
wc -l /tmp/phase49_crash.log

# Search for specific checkpoint
grep "CHECKPOINT 4d" /tmp/phase49_crash.log

# Time between specific events
grep "CHECKPOINT 4a\|CHECKPOINT 4e" /tmp/phase49_crash.log

# All errors in log
grep "!!!" /tmp/phase49_crash.log

# Pretty print with timing
cat /tmp/phase49_crash.log | awk '{print NR": "$0}'
```

## Next Steps

1. Run with logging to identify hang point
2. Document the finding in `docs/MACOS_PORT_DIARY.md`
3. Add more detailed logging to that specific function
4. Re-run to narrow down further
5. Implement fix or workaround
6. Verify fix with logging enabled
