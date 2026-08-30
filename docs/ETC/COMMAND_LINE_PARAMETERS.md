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
| `-webrtc` | Enables the native macOS/Linux WebRTC UDP transport. Native UDP remains the default. Requires a build configured with `-DSAGE_USE_NATIVE_WEBRTC=ON`. | `./GeneralsXZH -webrtc -webrtc-room LAN1` |
| `-webrtc-signaling <url>` | Sets the Cloudflare-compatible signaling origin. | `-webrtc-signaling https://signaling.generalsx.org` |
| `-webrtc-room <code>` | Joins a 4-10 character uppercase alphanumeric room. | `-webrtc-room R7K2QX` |
| `-webrtc-name <name>` | Sets the signaling roster name (1-24 safe characters). | `-webrtc-name "Native Player"` |
| `-webrtc-capacity <2-8>` | Sets room capacity when creating a room. | `-webrtc-capacity 4` |
| `-webrtc-no-turn` | Disables the `/turn-credentials` request and uses direct ICE only. | `./GeneralsXZH -webrtc -webrtc-no-turn` |

### Native WebRTC environment options

The equivalent environment variables are `GENERALSX_WEBRTC=1`,
`GENERALSX_WEBRTC_SIGNALING_URL`, `GENERALSX_WEBRTC_ROOM`,
`GENERALSX_WEBRTC_PLAYER_NAME`, `GENERALSX_WEBRTC_CAPACITY`, and
`GENERALSX_WEBRTC_DISABLE_TURN=1`.

By default, the client retrieves short-lived TURN credentials from the
signaling origin's `/turn-credentials` endpoint. Long-lived Cloudflare TURN
keys remain Worker secrets and are never accepted by the game. Operators may
provide a short-lived RTC `iceServers` array through
`GENERALSX_WEBRTC_ICE_SERVERS_JSON`; do not place credentials in command-line
arguments, configuration files, or source control.

The room assigns each connection a stable slot and the synthetic address
`10.0.0.(slot+1)`. The game continues to use its normal LAN and lockstep
protocols above the WebRTC-backed UDP abstraction.

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
cd ~/GeneralsX/GeneralsZH
./run.sh -win -logToCon 2>&1 | grep -v "D3DRS_PATCHSEGMENTS" | tee ~/Projects/GeneralsX/logs/manual_run.log
```

Legacy fallback during migration:

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
- `Core/GameEngine/Source/Common/CommandLine.cpp` - Shared parameters
- `GeneralsMD/Code/Main/WinMain.cpp` - Entry point and initial parsing
