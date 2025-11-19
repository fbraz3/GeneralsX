# Configuration File Examples

This directory contains example configuration files for GeneralsX and GeneralsXZH.

## File Locations by Platform

### Windows
```
%APPDATA%/GeneralsX/GeneralsX.ini       (for Generals base game)
%APPDATA%/GeneralsX/GeneralsXZH.ini     (for Zero Hour expansion)
```

### macOS
```
~/.config/GeneralsX.ini       (for Generals base game)
~/.config/GeneralsXZH.ini     (for Zero Hour expansion)
```

### Linux
```
~/.config/GeneralsX.ini       (for Generals base game)
~/.config/GeneralsXZH.ini     (for Zero Hour expansion)
```

## Cross-Platform Path Resolution

The game uses `SDL_GetPrefPath()` to automatically resolve the correct configuration directory:

```cpp
// Returns correct path for current platform:
// Windows: %APPDATA%/GeneralsX/
// macOS:   ~/.config/
// Linux:   ~/.config/

std::string config_dir = SDL_GetPrefPath("GeneralsX", "");
std::string config_file = config_dir + "GeneralsX.ini";
```

## Registry to INI Mapping

These INI files replace the Windows Registry entries that were previously stored at:

### For Generals (base game)
```
HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\EA Games\Generals
HKEY_CURRENT_USER\SOFTWARE\Electronic Arts\EA Games\Generals
```

### For Zero Hour (expansion)
```
HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\EA Games\Command and Conquer Generals Zero Hour
HKEY_CURRENT_USER\SOFTWARE\Electronic Arts\EA Games\Command and Conquer Generals Zero Hour
```

## Configuration Structure

Each INI file contains:

- **[GeneralsX Configuration]** or **[GeneralsXZH Configuration]**: Main game settings
  - Language: Localization setting
  - SKU: Game version identifier
  - Version: Game version number
  - MapPackVersion: Latest map pack version

- **[Graphics]**: Graphics configuration
  - Resolution settings
  - Windowed mode toggle
  - Backend selection (UseMetalBackend on macOS)

- **[Audio]**: Audio settings
  - Volume levels for music and sound effects

- **[Network]**: Network settings
  - Connection type
  - Bandwidth settings

- **[Player]**: Player profile settings
  - Player name
  - Default side/faction
  - Difficulty level

- **[Display]**: Display quality settings
  - Detail levels for effects and shadows

- **[Advanced]**: Advanced settings (Zero Hour only)
  - Debug mode
  - Logging level
  - Asset and map paths

## Reading Configuration

The game uses the INI parser to read these values:

```cpp
// Example: Reading a string value
AsciiString language;
if (GetStringFromRegistry("", "Language", language))
{
    // Use language value
}

// Example: Reading an integer value
UnsignedInt version;
if (GetUnsignedIntFromRegistry("", "Version", version))
{
    // Use version value
}
```

## Modifying Configuration

Configuration values are written back to the INI file when settings are changed in-game:

```cpp
// Example: Writing a string value
setStringInRegistry(config_path, "", "Language", "french");

// Example: Writing an integer value
setUnsignedIntInRegistry(config_path, "", "Version", new_version);
```

## Platform-Specific Considerations

### Windows
- Uses `%APPDATA%` environment variable
- Resolves to `C:\Users\[Username]\AppData\Roaming` by default
- Directory created automatically on first run

### macOS
- Uses `~/.config` directory
- Creates directory if it doesn't exist
- Follows XDG Base Directory Specification

### Linux
- Uses `~/.config` directory (XDG_CONFIG_HOME)
- Falls back to `~/.config` if XDG_CONFIG_HOME not set
- Follows XDG Base Directory Specification

## Implementation Notes

1. **SDL_GetPrefPath()**: Automatically handles platform-specific paths
2. **std::filesystem**: Used for directory creation and file operations
3. **INI Parser**: Existing game INI parser handles reading/writing
4. **Multi-Instance**: ClientInstance.cpp uses SDL2 mutex for single-instance lock
5. **Path Separators**: std::filesystem handles `/` vs `\` automatically
