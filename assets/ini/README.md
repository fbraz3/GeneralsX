# GeneralsX Configuration Files (INI Format)

This directory contains example INI configuration files for GeneralsX and GeneralsXZH.

## Platform Paths

Configuration files are stored in platform-specific user directories:

- **Windows**: `%APPDATA%\GeneralsX\GeneralsX.ini` or `GeneralsXZH.ini`
- **macOS**: `~/.config/GeneralsX.ini` or `GeneralsXZH.ini`
- **Linux**: `~/.config/GeneralsX.ini` or `GeneralsXZH.ini`

## Automatic Directory Creation

The game automatically creates the configuration directory on first run using `SDL_GetPrefPath("GeneralsX", "")`.

## INI File Structure

### Generals Settings Section

Standard configuration keys for base game:

```
[Generals Settings]
Language = english
SKU = GeneralsX
Version = 65540
MapPackVersion = 65536
InstallPath = /path/to/installation
Proxy = 
```

### GeneralsXZH Settings Section

Configuration for Zero Hour expansion with additional keys:

```
[GeneralsXZH Settings]
Language = english
SKU = GeneralsZH
Version = 65540
MapPackVersion = 65536
InstallPath = /path/to/installation
Proxy = 
ERGC = GP205480888522112040
```

## Graphics Section

Platform-specific graphics configuration:

```
[Graphics]
Width = 1024
Height = 768
Windowed = 0
ColorDepth = 32
UseMetalBackend = 1     # macOS only
```

## Audio Section

Audio subsystem configuration:

```
[Audio]
Enabled = 1
MusicVolume = 100
SoundVolume = 100
```

## Network Section

Networking configuration:

```
[Network]
ConnectionType = LAN
Bandwidth = 100000
```

## Player Section

Player preferences:

```
[Player]
Name = Player
Side = USA
Difficulty = Hard
GeneralIndex = 0        # Zero Hour only
```

## Advanced Section

Debug and advanced options:

```
[Advanced]
EnableDebug = 0
LogLevel = 0
AssetPath =             # Optional asset override
MapPath =               # Optional map override
```

## Registry Migration (Windows)

When upgrading from registry-based configuration to INI:

1. Game checks for existing INI file
2. If not found, game uses Windows Registry (HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER)
3. On first write, game migrates values to INI
4. Subsequent reads/writes use only INI file

**Note**: With Phase 39.5, registry access is completely replaced by INI files on all platforms.

## Porting Registry Values

Registry keys from Windows are mapped to INI sections:

| Registry Location | INI Mapping |
|---|---|
| `HKEY_LOCAL_MACHINE\...\Generals` | `[Generals Settings]` |
| `HKEY_LOCAL_MACHINE\...\Generals Zero Hour` | `[GeneralsXZH Settings]` |
| `HKEY_CURRENT_USER\...\Generals` | `[Generals Settings]` (read-only priority) |

## Cross-Platform Benefits

- Single configuration format across all platforms
- No platform-specific registry dependencies
- Easy to backup and transfer configurations
- Human-readable INI format for manual editing
- Version control friendly

## Example Usage

### Reading Configuration

```cpp
AsciiString language;
if (GetStringFromRegistry("Language", language)) {
    // Use language from INI file
}
```

### Writing Configuration

```cpp
SetStringInRegistry("Language", "portuguese-br");
```

All configuration changes are automatically persisted to INI files in `~/.config/` or `%APPDATA%`.
