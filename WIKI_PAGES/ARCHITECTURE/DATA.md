# ARCHITECTURE/DATA.md

## Summary

Data handling — serialization, memory management, and platform-specific considerations.

---

## Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                     Data Flow                                │
├─────────────────────────────────────────────────────────────┤
│  Game State ←→ Core Engine ←→ Platform Layer ←→ Platform    │
│      ↓              ↓              ↓              ↓            │
│  Serialization  Asset Loading   File I/O       Memory       │
│  (INI, SCN)     (Textures)      (SDL3)         (Platform)    │
└─────────────────────────────────────────────────────────────┘
```

---

## Serialization

### INI Files

**Purpose:** Configuration storage

**Format:**
```
[Section]
Key=Value
AnotherKey=AnotherValue
```

**Platform Considerations:**
- Case-insensitive lookup on Linux
- Cross-platform path handling
- UTF-8 encoding

**Example:**
```cpp
// Read configuration
config.ParseINI("config.ini");
int fpsLimit = config.GetInt("Game", "FPSLimit", 60);

// Write configuration
config.SetInt("Game", "FPSLimit", fpsLimit);
config.WriteINI("config.ini");
```

---

### Replay Files (.SCN)

**Purpose:** Game state snapshots

**Format:** Binary with map field percent encoding

**Platform Considerations:**
- CRC validation
- Special character handling
- Absolute and directory-based paths

**Example:**
```cpp
// Load replay
replay.Load("replay.scn");

// Verify integrity
if (!replay.VerifyCRC()) {
    std::cerr << "CRC mismatch, replay may be corrupted" << std::endl;
}
```

---

### Asset Files

**Purpose:** Textures, sounds, videos

**Format:** Various (PNG, TGA, WAV, MP3, MP4)

**Platform Considerations:**
- Case-insensitive lookup on Linux
- Cross-platform path handling
- Memory-efficient loading

**Example:**
```cpp
// Load texture
SDL_Surface* surface = SDL_LoadBMP("texture.bmp");

// Load sound
SDL_AudioStream* audio = SDL_LoadWAV("sound.wav", &spec, &buffer, nullptr);
```

---

## Memory Management

### Automatic (STL/RAII)

**Preferred approach:**
```cpp
std::unique_ptr<Texture> texture = CreateTexture();
// Automatically deleted when goes out of scope
```

### Manual (Legacy Code)

**Rules for VC6 legacy code:**
```cpp
// Always delete/delete[]
Texture* texture = new Texture();
delete texture;

// Use STLPort for VC6 builds
```

---

## Platform-Specific Considerations

### Linux

| Issue | Solution |
|-------|----------|
| Case-sensitive file access | Case-insensitive VFS lookup |
| Path separators | Always use `/` in code |
| Symbolic links | Follow symlinks in asset loading |

### macOS

| Issue | Solution |
|-------|----------|
| Case-insensitive file system | No special handling needed |
| Path separators | Always use `/` in code |
| Symbolic links | Follow symlinks in asset loading |

---

## Memory Layout

### Game State

```
Game State
├── Campaign State
├── Player State
├── Unit State
└── Map State
```

### Asset Cache

```
Asset Cache
├── Textures
├── Sounds
├── Videos
└── Sprites
```

---

## Data Integrity

### CRC Validation

**Purpose:** Verify replay integrity

**Implementation:**
```cpp
bool Replay::VerifyCRC() {
    uint32_t calculatedCRC = CalculateCRC(m_data);
    uint32_t expectedCRC = m_expectedCRC;
    return calculatedCRC == expectedCRC;
}
```

### Map Field Encoding

**Purpose:** Handle special characters in map names

**Implementation:**
- Percent encoding for special characters
- Cross-platform path resolution
- Case-insensitive matching

---

## Platform-Specific Memory Issues

### Linux

**Issue:** Memory fragmentation from mixed allocations

**Solution:**
- Use memory pools for frequent allocations
- Prefer STL containers over raw pointers
- Regular memory profiling

### macOS

**Issue:** Memory pressure warnings

**Solution:**
- Monitor memory usage
- Use memory profiling tools
- Optimize asset loading

---

## Verification Checklist

### Serialization

- [ ] INI files read/write correctly
- [ ] Replay files load without corruption
- [ ] Asset files load on both platforms
- [ ] Case-insensitive lookup on Linux

### Memory Management

- [ ] No memory leaks detected
- [ ] Automatic memory cleanup on exit
- [ ] Manual cleanup in legacy code
- [ ] No double-free errors

### Platform Compatibility

- [ ] Linux: Case-sensitive lookup works
- [ ] macOS: Path handling works
- [ ] Cross-platform: Same binary on both
- [ ] Path separators: Always `/`

---

## See Also

- [CONCEPTS/RETAIL.md](../CONCEPTS/RETAIL.md) — Retail compatibility
- [CONCEPTS/DETERMINISM.md](../CONCEPTS/DETERMINISM.md) — Determinism
- [TOPICS/DEBUGGING.md](../TOPICS/DEBUGGING.md) — Debugging techniques

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, architecture documentation
