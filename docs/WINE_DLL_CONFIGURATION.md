# Wine/Proton Configuration Guide

## DLL Resolution Challenge

When running GeneralsX from a directory different from where the DLLs are located (e.g., build directory, Wine prefix, different assets directory), Windows/Wine cannot load required DLLs:

```
0024:err:module:import_dll Library binkw32.dll not found
0024:err:module:import_dll Library mss32.dll not found
```

**Why SetDllDirectory alone doesn't work:**
- DLLs must be resolved BEFORE `main()` is called
- SetDllDirectory() is called INSIDE main(), too late
- Windows/Wine fails to load the executable if DLLs aren't found during process initialization

## Solutions

### ✅ Solution 1: DLLs in Executable Directory (RECOMMENDED - Local Windows)

The simplest and most reliable solution - keep DLLs in the same directory as the executable.

**Setup:**
```bash
# Copy all DLLs to executable directory
cp "$ASSET_PATH"/*.dll "$EXE_DIR/"
```

**Result:**
- ✅ Windows finds DLLs immediately on startup
- ✅ SetDllDirectory still adds asset directory as backup search path
- ✅ No environment variables needed
- ✅ Works from any working directory

### Problem 2: WINEDLLPATH Environment Variable (Recommended for Wine)

Set the `WINEDLLPATH` environment variable to include the asset directory:

**macOS/Linux:**
```bash
export WINEDLLPATH="$HOME/.GeneralsX/GeneralsMD/Data:$WINEPREFIX/drive_c/windows/system32"
wine "$HOME/.GeneralsX/GeneralsMD/GeneralsXZH.exe" -win
```

**Or in a script:**
```bash
#!/bin/bash
ASSET_PATH="$HOME/.GeneralsX/GeneralsMD/Data"
WINEPREFIX="${WINEPREFIX:-$HOME/.wine}"
export WINEDLLPATH="$ASSET_PATH:$WINEPREFIX/drive_c/windows/system32"
wine "$HOME/.GeneralsX/GeneralsMD/GeneralsXZH.exe" -win
```

### Option 2: INI Configuration (GeneralsXZH.ini)

Edit `~/.GeneralsXZH.ini` and add/modify the `[Advanced]` section:

```ini
[Advanced]
DebugMode = 0
LogLevel = 2
AssetPath = /path/to/assets
DLLSearchPath = /path/to/dlls
WinePrefix = /path/to/wineprefix
```

**Example for macOS:**
```ini
[Advanced]
DLLSearchPath = /Users/yourname/.GeneralsX/GeneralsMD/Data
WinePrefix = /Users/yourname/.wine
```

**Example for Linux:**
```ini
[Advanced]
DLLSearchPath = /home/yourname/.GeneralsX/GeneralsMD/Data
WinePrefix = /home/yourname/.wine
```

### Option 3: Symlink Assets to Executable Directory

**macOS/Linux:**
```bash
cd "$HOME/.GeneralsX/GeneralsMD"
ln -s Data/* .
```

This copies all asset files (including DLLs) to the executable directory.

### Option 4: Copy DLLs to Wine System Directory

**macOS/Linux:**
```bash
cp "$HOME/.GeneralsX/GeneralsMD/Data"/*.dll "$WINEPREFIX/drive_c/windows/system32/"
```

## Configuration Keys

The following keys are now available in `[Advanced]` section of the INI file:

| Key | Purpose | Example |
|-----|---------|---------|
| `AssetPath` | Override default asset directory | `/home/user/.GeneralsX/GeneralsMD/Data` |
| `DLLSearchPath` | Additional DLL search path for Wine | `/home/user/.GeneralsX/GeneralsMD/Data` |
| `WinePrefix` | Explicit Wine prefix path | `/home/user/.wine` |
| `DebugMode` | Enable debug logging (0/1) | `1` |
| `LogLevel` | Debug log verbosity (0-3) | `2` |

## Recommended Approach

For best compatibility with Wine, use a launcher script that:

1. Sets `WINEDLLPATH` environment variable
2. Sets `WINEPREFIX` if using non-standard Wine location
3. Calls the executable with `-win -noshellmap` flags

**Example launcher (`run_zh.sh`):**
```bash
#!/bin/bash
set -e

ASSET_PATH="$HOME/.GeneralsX/GeneralsMD/Data"
EXEC_PATH="$HOME/.GeneralsX/GeneralsMD/GeneralsXZH.exe"
WINEPREFIX="${WINEPREFIX:-$HOME/.wine}"

export WINEDLLPATH="$ASSET_PATH:$WINEPREFIX/drive_c/windows/system32"
export WINEPREFIX

echo "Asset Path: $ASSET_PATH"
echo "Wine Prefix: $WINEPREFIX"
echo "DLL Search Path: $WINEDLLPATH"
echo ""

wine "$EXEC_PATH" -win -noshellmap "$@"
```

Make it executable:
```bash
chmod +x run_zh.sh
./run_zh.sh
```

## Troubleshooting

### Check DLL Resolution
```bash
# List required DLLs
wine "$HOME/.GeneralsX/GeneralsMD/GeneralsXZH.exe" --help 2>&1 | grep "Library.*not found"

# Check Wine path
echo $WINEDLLPATH
echo $WINEPREFIX
```

### Debug Output
Set `DebugMode = 1` in INI to get detailed logging:
```ini
[Advanced]
DebugMode = 1
LogLevel = 3
```

Log output goes to `$HOME/ConfigurationManager.debug.log`

### Wine Prefix Info
```bash
# List contents of Wine system32
ls -la "$WINEPREFIX/drive_c/windows/system32/" | grep -E "\.dll$"

# Check if DLLs exist
ls -la "$HOME/.GeneralsX/GeneralsMD/Data"/*.dll
```

## Next Steps

- Integration with SearchPathManager for unified path resolution
- Automatic WINEDLLPATH generation in launcher
- macOS/Linux native application bundles with embedded assets
