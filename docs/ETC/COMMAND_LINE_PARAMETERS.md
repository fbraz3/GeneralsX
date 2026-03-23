# Command Line Parameters

Common command line parameters for `GeneralsX` (Generals) and `GeneralsXZH` (Zero Hour).

## Window & Display

| Parameter | Description | Example |
|-----------|-------------|---------|
| `-win` | Forces windowed mode | `./GeneralsXZH -win` |
| `-fullscreen` | Forces fullscreen mode | `./GeneralsXZH -fullscreen` |
| `-xres <width>` | Sets horizontal resolution | `./GeneralsXZH -xres 1920` |
| `-yres <height>` | Sets vertical resolution | `./GeneralsXZH -yres 1080` |

## Development & Testing

| Parameter | Description | Example |
|-----------|-------------|---------|
| `-noshellmap` | Disables the shell map (skip intro) | `./GeneralsXZH -noshellmap` |
| `-quickstart` | Quick launch (skip movies + shell) | `./GeneralsXZH -quickstart` |
| `-debug` | Enable debug mode | `./GeneralsXZH -debug` |
| `-logToCon` | Enables legacy debug-log console routing (`DEBUG_LOG`). **Debug builds only** (`ALLOW_DEBUG_UTILS` / `RTS_BUILD_OPTION_DEBUG=ON`); ignored in release builds. | `./GeneralsXZH -logToCon` |

## Mods & Content

| Parameter | Description | Example |
|-----------|-------------|---------|
| `-mod <path>` | Loads a mod from directory or .big file | `./GeneralsXZH -mod /path/to/mod.big` |

## Replay & Multiplayer

| Parameter | Description | Example |
|-----------|-------------|---------|
| `-replay <file>` | Play a replay file | `./GeneralsXZH -replay match.rep` |
| `-jobs <count>` | Number of parallel replay jobs | `./GeneralsXZH -jobs 4 -replay *.rep` |
| `-headless` | Run without graphics (replay testing) | `./GeneralsXZH -headless -replay *.rep` |

## Common Combinations

### Quick Testing
```bash
./GeneralsXZH -win -noshellmap
```
Launch in windowed mode, skip intro.

### Replay Compatibility Testing
```bash
./GeneralsXZH -jobs 4 -headless -replay subfolder/*.rep
```
Test multiple replays in parallel without graphics (requires optimized VC6 build with `RTS_BUILD_OPTION_DEBUG=OFF`).

### High Resolution Testing
```bash
./GeneralsXZH -win -xres 2560 -yres 1440
```
Test in windowed mode at 1440p resolution.

## Platform-Specific Notes

### Windows
- Parameters can use `/` or `-` prefix (both work)
- Paths can use backslashes

### Linux
- Must use `-` prefix
- Paths must use forward slashes
- Some parameters may not work until Linux port is complete
- `-logToCon` sets a debug flag, but many Linux diagnostics still require explicit `fprintf(stderr, ...)` instrumentation because `OutputDebugString` paths are stubbed/non-visible on this platform.
- **`-logToCon` is only available in debug builds** (`RTS_BUILD_OPTION_DEBUG=ON` / `ALLOW_DEBUG_UTILS` defined). It is unrecognized and has no effect in release builds.

## Logging Diagnostics Recipe

Use this when investigating runtime behavior (example: skirmish startup flow):

```bash
cd ~/GeneralsX/GeneralsMD
./run.sh -win -logToCon 2>&1 | grep -v "D3DRS_PATCHSEGMENTS" | tee ~/Projects/GeneralsX/logs/manual_run.log
```

Then filter the generated log for targeted markers:

```bash
grep -n "SKIRMISH_DIAG\|ScoreScreen\|SkirmishGameOptionsMenu" ~/Projects/GeneralsX/logs/manual_run.log
```

## Source Code Reference

Command line parsing is implemented in:
- `Core/GameEngine/Source/gameclient.cpp` - Client-side parameters
- `GeneralsMD/Code/Main/WinMain.cpp` - Entry point and initial parsing
