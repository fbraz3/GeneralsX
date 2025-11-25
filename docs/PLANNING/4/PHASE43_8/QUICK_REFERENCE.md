# Phase 43.8 - Quick Reference Map

## Stubs Status Dashboard

### ✅ FIXED (Phase 43.7)

```text
SDL2GameEngine::createLocalFileSystem()
  └─ FIXED: NEW StdLocalFileSystem
  
SDL2GameEngine::createArchiveFileSystem()
  └─ FIXED: NEW StdBIGFileSystem
```

### 🔴 CRITICAL PENDING (Must fix for game to run)

```text
SDL2GameEngine::createLocalFileSystem()          [FIXED]
SDL2GameEngine::createArchiveFileSystem()        [FIXED]
```

### 🟠 HIGH PRIORITY (Phase 44.x)

```text
SDL2GameEngine::createNetworkInterface()
  └─ Impact: Multiplayer unavailable
  └─ Solution: Create StdNetworkInterface (POSIX sockets)
  └─ Effort: HIGH
  
SDL2GameEngine::createParticleSystemManager()
  └─ Impact: No particle effects
  └─ Solution: NEW ParticleSystemManager (already exists)
  └─ Effort: MEDIUM
  
SDL2GameEngine::createAudioManager()
  └─ Impact: No sound
  └─ Solution: OpenALAudioManager (reference: jmarshall repo)
  └─ Effort: HIGH
  └─ Phase Target: 33.x
```

### 🟡 MEDIUM PRIORITY (Phase 43.x)

```text
W3DGameClient::getMouseHandler()
  └─ Impact: Mouse input degraded
  └─ Solution: NEW SDL2MouseHandler
  └─ Effort: MEDIUM
  
GetTextureFromFileAsSurface()
GetTextureFromFile()
  └─ Impact: Textures not loaded
  └─ Solution: Use Metal/Vulkan backend
  └─ Effort: MEDIUM
  
GetFunctionRegistry()
GetGameWindowDrawFunction()
GetWindowLayoutInitFunction()
  └─ Impact: GUI incomplete
  └─ Solution: Return actual function pointers
  └─ Effort: LOW
```

### 🟢 APPROVED COMPATIBILITY STUBS (No changes)

```text
Windows API Compatibility Layer
  └─ MessageBox() - no-op on non-Windows
  └─ LoadLibraryA() - nullptr on non-Windows
  └─ GetProcAddress() - nullptr on non-Windows
  └─ FreeLibrary() - no-op on non-Windows
  └─ Status: ✅ Correct as-is
```

---

## File Locations

| Stub | File | Line(s) | Status |
|------|------|---------|--------|
| LocalFileSystem | SDL2GameEngine.cpp | 125 | ✅ FIXED |
| ArchiveFileSystem | SDL2GameEngine.cpp | 131 | ✅ FIXED |
| NetworkInterface | SDL2GameEngine.cpp | 153 | 🟡 TODO |
| ParticleSystemManager | SDL2GameEngine.cpp | 173 | 🟡 TODO |
| AudioManager | SDL2GameEngine.cpp | 180 | 🟡 TODO |
| MouseHandler | W3DGameClient.h | 129/117 | 🟡 TODO |
| TextureFromFile | phase41_missing_implementations.cpp | 64,80 | 🟡 TODO |
| FunctionRegistry | phase41_missing_implementations.cpp | 185-195 | 🟡 TODO |

---

## Implementation Order (Recommended)

1. **Phase 43.7** ✅ LocalFileSystem + ArchiveFileSystem (BLOCKING)
2. **Phase 43.x** - ParticleSystemManager (visual features)
3. **Phase 43.x** - MouseHandler (input handling)
4. **Phase 43.x** - Function registry (GUI system)
5. **Phase 33.x** - AudioManager (sound system)
6. **Phase 44.x** - NetworkInterface (multiplayer)

---

## Key Insights

### Why These Stubs Exist

1. **Factory Pattern Placeholders** - Created factory methods but implementations pending
2. **Phase Dependencies** - Some features blocked by incomplete subsystems
3. **Cross-Platform Abstraction** - Needed compatibility layers before implementations

### Why They Must Be Fixed

1. **Initialization Crashes** - nullptr dereference in subsystem chains
2. **Game Unplayable** - Critical systems depend on working factories
3. **Platform Stability** - Cross-platform support requires real implementations

### How They're Being Fixed

1. **Use Existing Classes** - StdLocalFileSystem, StdBIGFileSystem already exist
2. **Reference Implementations** - Check reference repos for working patterns
3. **Real Solutions** - No empty stubs, every factory returns working instances

---

## References

- **Complete Audit**: See [STUB_AUDIT.md](STUB_AUDIT.md)
- **Project Architecture**: `.github/copilot-instructions.md`
- **Build Guidelines**: `.github/instructions/project.instructions.md`
- **Reference Repos**: `references/jmarshall-win64-modern/`, `references/fighter19-dxvk-port/`

---

**Phase**: 43.8  
**Created**: 2025-11-25  
**Last Updated**: 2025-11-25
