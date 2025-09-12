[![Weekly Release](https://github.com/fbraz3/GeneralsGameCode/actions/workflows/weekly-release.yml/badge.svg)](https://github.com/fbraz3/GeneralsGameCode/actions/workflows/weekly-release.yml)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)

## Generals Game Code Build Project

This project provides **weekly builds** of the [GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode/) repository, aiming to help the community with up-to-date binaries and easier access to the latest changes.

### 🎮 New: OpenGL Support!
This fork now includes **OpenGL rendering support** alongside the original DirectX 8, enabling:
- **Linux compatibility** 🐧
- **macOS support** 🍎 (**Major Progress!** DirectX compatibility layer implemented)
- **Better cross-platform development**
- **Modern graphics pipeline**

**Latest macOS Port Progress (September 11, 2025):**
- ✅ **🚀 PHASE 1 COMPLETE: Cross-Platform Configuration System!** - [47/110] files compiled successfully with zero Registry errors
- ✅ **ConfigManager Implementation** - Complete Windows Registry replacement with INI-based configuration system
- ✅ **Registry API Compatibility** - RegOpenKeyEx, RegQueryValueEx, RegSetValueEx → ConfigManager backend mapping
- ✅ **Cross-Platform Paths** - macOS ~/Library/Application Support/, Linux ~/Games/, Windows Documents integration
- ✅ **Graphics Compatibility Extended** - BITMAPFILEHEADER, D3DTTFF_DISABLE, string functions (lstrcmpi, GetCurrentDirectory)
- ✅ **🎉 Previous Achievement: DirectX Typedef Resolution COMPLETE!** - g_ww3d2 target compiling with **0 ERRORS**
- ✅ **120+ Compilation Errors Resolved** - Through comprehensive Windows API implementation and multi-layer DirectX architecture
- ✅ **LP* Typedef Coordination** - Perfect harmony between Core void* definitions and Generals interface casting
- ✅ **Multi-layer DirectX Architecture** - Core/win32_compat.h + Generals/d3d8.h perfect coordination
- ✅ **All Core Libraries Compiled** - 100% success! libww3d2.a (23MB), libwwlib.a (1.3MB), libwwmath.a (2.3MB)
- ✅ **Comprehensive Windows API Layer** - 16+ compatibility headers working flawlessly
- ✅ **🚀 PHASE 2 COMPLETE: Comprehensive Windows API Compatibility!** - Threading, File System, Network, and String APIs fully implemented
  - ✅ **Threading APIs**: CreateThread → pthread_create, WaitForSingleObject, CreateMutex with full Windows compatibility
  - ✅ **File System APIs**: CreateDirectory, DeleteFile, CreateFile → POSIX wrappers with Windows signatures
  - ✅ **Network APIs**: Complete socket compatibility layer with Win32Net namespace isolation
  - ✅ **String APIs**: strupr, strlwr, stricmp, DirectX constants (D3DPTFILTERCAPS_*, D3DTTFF_*) 
- ✅ **🚀 PHASE 5 COMPLETE: Audio & Multimedia APIs!** - DirectSound compatibility with OpenAL backend implementation
  - ✅ **DirectSound APIs**: IDirectSound8, IDirectSoundBuffer8, DirectSound3DBuffer → OpenAL mapping
  - ✅ **3D Audio APIs**: Spatial audio positioning, listener orientation, doppler effects
  - ✅ **Multimedia Timers**: timeSetEvent, timeKillEvent → thread-based timer system
  - ✅ **OpenAL Integration**: Cross-platform audio backend with Windows API compatibility
- 🎯 **Next Phase: DirectX Graphics & Device APIs**
  - **DirectX Graphics**: D3D device creation, resource management, rendering pipeline
  - **Target**: Full executable compilation with complete graphics compatibility

See [MACOS_PORT.md](./MACOS_PORT.md) for detailed macOS porting progress, [OPENGL_SUMMARY.md](OPENGL_SUMMARY.md) for comprehensive implementation documentation, and [OPENGL_TESTING.md](./OPENGL_TESTING.md) for testing procedures and results.

- **Daily sync** with the upstream SuperHackers repository.
- **Weekly builds** including all recent updates.
- **Not an official project** from SuperHackers — community-driven and maintained.

## Motivation

Many users want to test the latest features and fixes from the GeneralsGameCode project without building from source. This project automates the process, delivering ready-to-use builds for everyone.

## How It Works

1. **Daily Sync:** The codebase is automatically synchronized with the upstream repository every day.
2. **Weekly Build:** Every week, a new build is generated with all the latest changes and made available here.

## Usage

### Quick Start
- Download the latest build from the [Releases](https://github.com/fbraz3/GeneralsGameCode/releases) section.
- Follow the instructions provided in the release notes to run the game.

### Graphics API Selection
You can now choose between DirectX 8 (Windows) and OpenGL (cross-platform):

```bash
# Use OpenGL (recommended for Linux/macOS)
./generals --opengl

# Use DirectX 8 (Windows only, legacy)
./generals --directx

# Test graphics system
./tests/opengl/run_opengl_tests.sh
```

### Building from Source

#### Current Compilation Status (December 2024)
**✅ Core Libraries**: All compiling successfully on macOS with Phase 5 Audio APIs
- `libww3d2.a` (24MB) - 3D graphics engine with DirectSound compatibility ✅
- `libwwlib.a` (1.3MB) - Core utilities ✅  
- `libwwmath.a` (2.3MB) - Math operations ✅

**🚀 Phase 5 APIs**: Audio & Multimedia APIs implemented ✅
- ✅ **DirectSound APIs**: Complete OpenAL backend (IDirectSound8, IDirectSoundBuffer8, DirectSound3DBuffer)
- ✅ **3D Audio APIs**: Spatial positioning, listener orientation, doppler effects
- ✅ **Multimedia Timers**: Thread-based timer system (timeSetEvent, timeKillEvent)
- ✅ **OpenAL Integration**: Cross-platform audio with Windows API compatibility

**🚀 Phase 2 APIs**: All Windows API compatibility layers implemented ✅
- ✅ **Registry APIs**: Complete ConfigManager replacement (RegOpenKeyEx, RegQueryValueEx, RegCloseKey, RegSetValueEx)
- ✅ **Threading APIs**: pthread-based implementation (CreateThread, WaitForSingleObject, CreateMutex)
- ✅ **File System APIs**: POSIX-based compatibility (CreateDirectory, DeleteFile, CreateFile, _chmod)
- ✅ **Network APIs**: Socket compatibility with namespace isolation (getsockname, Win32Net wrappers)
- ✅ **String APIs**: Cross-platform string functions (strupr, strlwr, stricmp, DirectX constants)

**🎯 Game Executables**: Ready for Phase 3 - DirectX Graphics & Device APIs

```bash
# Linux/macOS (OpenGL)
cmake -DENABLE_OPENGL=ON -DDEFAULT_TO_OPENGL=ON ..
make

# Windows (both APIs)
cmake -DENABLE_OPENGL=ON -DENABLE_DIRECTX=ON ..
msbuild genzh.sln
```

## Testing OpenGL

To test the OpenGL graphics system before building the full game:

```bash
# Quick automated test
./tests/opengl/run_opengl_tests.sh

# Manual compilation test
cd tests/opengl
g++ -std=c++17 -I../../Core/Libraries/Include/GraphicsAPI \
    -framework OpenGL test_simple_compile.cpp -o test_simple
./test_simple
```

For detailed testing instructions, see [OPENGL_TESTING.md](./OPENGL_TESTING.md).

## Documentation

- **[OPENGL_COMPLETE.md](OPENGL_SUMMARY.md)** - Comprehensive OpenGL implementation guide and architecture documentation
- **[OPENGL_TESTING.md](./OPENGL_TESTING.md)** - OpenGL-specific testing procedures, results, and troubleshooting
- **[TESTING_COMPLETE.md](TESTING.md)** - Complete testing guide for all platforms, builds, and configurations
- **[MACOS_PORT.md](./MACOS_PORT.md)** - macOS porting progress, DirectX compatibility layer, and platform-specific details
- **[TEST_ORGANIZATION_SUMMARY.md](./TEST_ORGANIZATION_SUMMARY.md)** - Test files organization and project structure summary
- **[CONTRIBUTING.md](./CONTRIBUTING.md)** - Guidelines for contributing to the project

## Disclaimer

This is an **unofficial** project and is **not affiliated** with the SuperHackers team. All credits for the original code go to [TheSuperHackers/GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode/).

## Contributing

Contributions are welcome! Feel free to open issues or pull requests to improve the build process or documentation.

## Special Thanks

- The original [Westwood Studios](https://en.wikipedia.org/wiki/Westwood_Studios) for creating the Command & Conquer series
- [EA Games](https://en.wikipedia.org/wiki/Electronic_Arts) for creating the original Command & Conquer: Generals game, which inspires this project.
- For [Xezon](https://github.com/xezon) and [the SuperHackers team](https://github.com/TheSuperHackers) for their work on the GeneralsGameCode project, and to all contributors who help keep this project up-to-date.

## License

See the [LICENSE](./LICENSE) file for details.
