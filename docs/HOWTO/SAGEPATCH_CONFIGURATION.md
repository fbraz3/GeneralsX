# SagePatch Configuration

SagePatch is a quality-of-life (QoL) enhancement layer for GeneralsX. It provides features like F11 screenshots, cursor lock, brightness controls, window snap, and camera/scroll overrides.

## How It Works

On first launch, the engine automatically creates a `SagePatch.ini` file in your user data directory with default QoL values. This file is loaded after the game's built-in `GameData.ini`, so values here override the defaults.

### User Data Directories

| Platform | Path |
|---|---|
| macOS | `~/Library/Application Support/GeneralsX/GeneralsZH/` |
| Linux | `~/.local/share/GeneralsX/GeneralsZH/` |

## Configuration Options

Edit `SagePatch.ini` with any text editor. The file uses the standard Generals INI format.

### Camera Settings

```ini
GameData
  ; Maximum camera height (vanilla: 310, default: 350)
  MaxCameraHeight = 350.0

  ; Minimum camera height (vanilla: 120, default: 100)
  MinCameraHeight = 100.0

  ; Set to "No" to allow scrolling past max camera height (default: No)
  EnforceMaxCameraHeight = No
End
```

### Scroll Speed

```ini
GameData
  ; Keyboard scroll speed factor (vanilla: 0.5, default: 1.0)
  ; Higher values = faster scrolling
  KeyboardScrollSpeedFactor = 1.0
End
```

### Terrain Draw Distance

```ini
GameData
  ; Terrain draw distance scale (default: 1.05)
  ; Values > 1.0 draw more terrain at max zoom out, fixing pop-in
  ; Values < 1.0 draw less terrain (improves performance on low-end systems)
  TerrainDrawDistanceScale = 1.05
End
```

## Applying Changes

After editing `SagePatch.ini`:

1. Save the file
2. Restart GeneralsX

Changes take effect on the next game launch.

## Disabling SagePatch

To disable SagePatch entirely, set the environment variable before launching:

```bash
# Linux
export SAGE_PATCH_DISABLED=1
./run.sh

# macOS
export SAGE_PATCH_DISABLED=1
./GeneralsXZH.app/Contents/MacOS/run.sh
```

## Troubleshooting

### Settings not taking effect

1. Verify the file exists in the correct user data directory
2. Check that values are inside a `GameData ... End` block
3. Restart the game after making changes

### File was overwritten

The engine only creates `SagePatch.ini` if it does not exist. If you delete the file, it will be recreated with defaults on the next launch.

### Camera feels wrong

- `MaxCameraHeight` controls how far out you can zoom
- `MinCameraHeight` controls how close you can zoom in
- `EnforceMaxCameraHeight = No` allows scrolling past the max (recommended)

## Technical Details

SagePatch settings are parsed by the engine's `GlobalData` INI system. The `SagePatch.ini` file is loaded after the game's built-in `GameData.ini`, so all values here override the original defaults.

The engine auto-creates this file in the user data directory using `fopen()` during `GlobalData` initialization. No external scripts or manual copying is required.
