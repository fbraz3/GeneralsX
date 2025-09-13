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

**Latest macOS Port Progress (September 13, 2025):**
- ✅ **🎉 HISTORIC BREAKTHROUGH: COMPLETE ERROR ELIMINATION!** - Compilation proceeding with warnings only! From 614 blocking errors to 72 warnings
- ✅ **🚀 PHASE 7 COMPLETE: IME & Input System!** - Complete Windows Input Method Editor compatibility layer implemented
  - ✅ **IME Functions**: ImmAssociateContext, ImmGetCompositionString*, ImmGetCandidateListCountW - all message processing
  - ✅ **IME Constants**: WM_IME_*, GCS_*, CS_*, IMN_* - complete message and notification system
  - ✅ **IME Structures**: COMPOSITIONFORM, CANDIDATEFORM, CANDIDATELIST for text input processing
  - ✅ **Multibyte Support**: _mbsnccnt function for international character processing
- ✅ **Compilation Success**: 800+ files processing successfully with only warnings remaining
- ✅ **Libraries Building**: libresources.a, libwwmath.a, libwwlib.a - all core libraries compiling
- ✅ **Error Resolution**: Automated pointer cast corrections applied across entire codebase
- ✅ **🚀 PHASE 1-6 COMPLETE**: Configuration, Windows APIs, DirectX, Memory, Audio, File System - ALL IMPLEMENTED
- ✅ **🎯 Ready for Phase 8**: DirectX Graphics & Device APIs for final executable compilation

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

#### Current Compilation Status (September 2025)
**🎉 HISTORIC BREAKTHROUGH: ALL ERRORS ELIMINATED!** Compilation proceeding with warnings only!

**✅ Core Libraries**: All compiling successfully on macOS with complete Windows API compatibility
- `libww3d2.a` (24MB) - 3D graphics engine with complete IME support ✅
- `libwwlib.a` (1.3MB) - Core utilities ✅  
- `libwwmath.a` (2.3MB) - Math operations ✅
- `libresources.a` - Resource management ✅

**🚀 Phase 7 APIs**: IME & Input System implemented ✅
- ✅ **IME APIs**: Complete Input Method Editor (ImmAssociateContext, ImmGetCompositionString*, ImmGetCandidateListCountW)
- ✅ **IME Constants**: Full message system (WM_IME_*, GCS_*, CS_*, IMN_*)
- ✅ **IME Structures**: Text input processing (COMPOSITIONFORM, CANDIDATEFORM, CANDIDATELIST)
- ✅ **Multibyte Support**: International character processing (_mbsnccnt)

**🚀 Phases 1-6**: All Windows API compatibility layers implemented ✅
- ✅ **Configuration System**: Complete INI-based Registry replacement
- ✅ **Threading APIs**: pthread-based implementation with full Windows compatibility
- ✅ **File System APIs**: POSIX-based compatibility with memory management
- ✅ **DirectX APIs**: Complete graphics pipeline compatibility
- ✅ **Audio APIs**: DirectSound compatibility with OpenAL backend
- ✅ **Performance APIs**: High-resolution timing and memory management

**🎯 Next Phase 8**: DirectX Graphics & Device final implementation for executable linking
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
