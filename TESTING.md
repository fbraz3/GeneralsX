# Testing Complete Guide
## Command & Conquer: Generals - Comprehensive Testing Documentation

**Last Updated:** September 1, 2025
**Scope:** All project testing procedures, platforms, and configurations

---

## 📋 Overview

This document provides comprehensive testing procedures for the Command & Conquer: Generals project, covering:
- General project testing (replay compatibility, builds)
- Platform-specific testing (Windows, macOS, Linux)
- Build configuration testing
- CI/CD procedures

**For OpenGL-specific testing:** See [OPENGL_TESTING.md](./OPENGL_TESTING.md)

### OpenGL Testing Integration

This document covers general project testing. For comprehensive OpenGL testing procedures, including:
- OpenGL test suite execution and results
- OpenGL-specific troubleshooting and issue resolution
- Detailed OpenGL progress reports and benchmarks
- OpenGL development workflow and best practices

**→ See the dedicated [OPENGL_TESTING.md](./OPENGL_TESTING.md) documentation.**

The OpenGL testing is tightly integrated with the general testing workflow but requires specialized procedures documented separately due to its complexity and platform-specific requirements.

---

## 🎮 Replay Testing

### Test Replays

The `GeneralsReplays/` folder contains replays and required maps tested in CI to ensure retail compatibility.

### Windows Replay Testing
Test with replays locally:

1. **Setup:**
   - Copy replays to: `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Replays/subfolder/`
   - Copy maps to: `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Maps`

2. **Run Test:**
   ```bat
   START /B /W generalszh.exe -jobs 4 -headless -replay subfolder/*.rep > replay_check.log
   echo %errorlevel%
   PAUSE
   ```

**Requirements:**
- VC6 build with optimizations
- `RTS_BUILD_OPTION_DEBUG = OFF` (for retail compatibility)

---

## 🖥️ Platform Testing

### Windows Testing

#### Build Testing
```bash
# Standard Windows build
cmake -DRTS_BUILD_GENERALS=ON -DRTS_BUILD_ZEROHOUR=ON ..
msbuild genzh.sln

# OpenGL + DirectX build
cmake -DENABLE_OPENGL=ON -DENABLE_DIRECTX=ON ..
msbuild genzh.sln
```

#### Performance Testing
```bash
# Run with performance monitoring
generalszh.exe -jobs 4 -benchmark > performance.log
```

### macOS Testing

#### Compilation Progress Testing
```bash
# Check current compilation status
cd GeneralsGameCode/build/vc6
ninja g_generals 2>&1 | grep "FAILED:" | wc -l

# View compilation progress
ninja g_generals 2>&1 | grep -E "(Building|FAILED)" | head -20

# Test specific modules
ninja g_ww3d2      # Graphics module
ninja core_wwlib   # Core libraries
```

#### DirectX Compatibility Layer Testing
```bash
# Test individual file compilation with DirectX compatibility
cd build/vc6

# Asset manager compatibility
c++ -DENABLE_OPENGL=1 -D_UNIX -I../../Core/Libraries/Source/WWVegas/WW3D2 \
    -c ../../Generals/Code/Libraries/Source/WWVegas/WW3D2/assetmgr.cpp

# DirectX FVF compatibility
c++ -DENABLE_OPENGL=1 -D_UNIX -I../../Core/Libraries/Source/WWVegas/WW3D2 \
    -c ../../Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8fvf.cpp
```

#### Cross-Platform Build Testing
```bash
# Clean build test
rm -rf build && mkdir build && cd build

# Different configurations
cmake -DENABLE_OPENGL=ON -DRTS_BUILD_GENERALS=ON ..
ninja g_generals

cmake -DENABLE_OPENGL=ON -DRTS_BUILD_ZEROHOUR=ON ..
cmake -DENABLE_OPENGL=ON -DRTS_BUILD_CORE_TOOLS=ON ..
```

### Linux Testing

#### Standard Build Testing
```bash
# Linux build with parallel processing
make g_generals -j$(nproc)

# OpenGL-specific Linux build
cmake -DENABLE_OPENGL=ON -DDEFAULT_TO_OPENGL=ON ..
make -j$(nproc)
```

#### OpenGL Graphics Testing
```bash
# OpenGL test suite (see OPENGL_TESTING.md for details)
./tests/opengl/run_opengl_tests.sh

# Integration with main build
cmake -DENABLE_OPENGL=ON -DDEFAULT_TO_OPENGL=ON ..
make -j$(nproc)
```

#### Package Dependencies Testing
```bash
# Test required packages (Ubuntu/Debian)
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev

# Test required packages (Fedora)
sudo dnf install mesa-libGL-devel mesa-libGLU-devel
```

---

## 🔧 Build Configuration Testing

### Debug Builds
```bash
# Debug build with full debugging symbols
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_LOGGING=ON ..
make -j$(nproc)
```

### Release Builds
```bash
# Optimized release build
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPTIMIZATIONS=ON ..
make -j$(nproc)
```

### Feature-Specific Builds
```bash
# Core tools only
cmake -DRTS_BUILD_CORE_TOOLS=ON ..

# Generals tools
cmake -DRTS_BUILD_GENERALS_TOOLS=ON ..

# Zero Hour tools
cmake -DRTS_BUILD_ZEROHOUR_TOOLS=ON ..

# Extras
cmake -DRTS_BUILD_CORE_EXTRAS=ON -DRTS_BUILD_GENERALS_EXTRAS=ON ..
```

---

## 🧪 Automated Testing

### Test Suite Structure

```
tests/
├── README.md               # Test suite index
└── opengl/                 # OpenGL-specific tests
    ├── run_opengl_tests.sh # Main OpenGL test runner
    ├── test_*.cpp          # Individual test files
    └── ... (test artifacts)
```

### Running Test Suites

#### All OpenGL Tests
```bash
# From project root
./tests/opengl/run_opengl_tests.sh

# Or from test directory
cd tests/opengl
./run_opengl_tests.sh
```

#### Individual Test Categories
```bash
cd tests/opengl

# Basic functionality tests
./run_opengl_tests.sh basic

# Integration tests
./run_opengl_tests.sh integration

# Compilation tests
./run_opengl_tests.sh compile
```

### Test Requirements

#### System Requirements
- **macOS**: OpenGL via Metal backend (with deprecation warnings)
- **Linux**: OpenGL via GLX
- **Windows**: OpenGL via WGL or DirectX 8

#### Build Requirements
- C++11 compatible compiler (g++, clang++, MSVC)
- OpenGL development libraries
- CMake 3.25+ (for full project builds)

#### Dependencies
- Core Graphics API library
- W3D type system (mock or real)
- Utility library for compatibility headers

---

## 📊 Testing Status Matrix

| Test Category | Windows | macOS | Linux | Status | Documentation |
|---------------|---------|-------|-------|--------|---------------|
| Replay Compatibility | ✅ PASS | 🔄 WIP | 🔄 WIP | Primary | This document |
| Basic Compilation | ✅ PASS | ✅ PASS | ✅ PASS | Complete | This document |
| OpenGL Rendering | ✅ PASS | ✅ PASS | ✅ PASS | Complete | [OPENGL_TESTING.md](./OPENGL_TESTING.md) |
| DirectX Compatibility | ✅ PASS | ✅ PASS | ➖ N/A | Complete | This document |
| W3D Integration | ✅ PASS | 🔄 WIP | 🔄 WIP | Progress | [OPENGL_TESTING.md](./OPENGL_TESTING.md) |
| Core Libraries | ✅ PASS | ✅ PASS | ✅ PASS | Complete | This document |
| Tools Building | ✅ PASS | 🔄 WIP | 🔄 WIP | Progress | This document |

**Legend:**
- ✅ PASS - Working correctly
- 🔄 WIP - Work in progress
- ❌ FAIL - Not working
- ➖ N/A - Not applicable

---

## 🛠️ Troubleshooting

### Common Issues

#### Include Path Errors
```bash
# Ensure you're running from project root
pwd  # Should show GeneralsGameCode directory

# Or use absolute paths in compilation
g++ -I$(pwd)/Core/Libraries/Include ...
```

#### OpenGL Deprecation (macOS)
```bash
# Add deprecation silence flag
g++ -DGL_SILENCE_DEPRECATION ...
```

#### Missing Dependencies
```bash
# Ensure dependencies are built first
cmake --build . --target core_utility
cmake --build . --target core_wwlib
```

### Debug Mode Compilation
```bash
# Add debug flags for detailed error information
g++ -g -DDEBUG -DVERBOSE_ERRORS -I../../Core/Libraries/Include ...
```

### Memory Testing
```bash
# Linux/macOS memory leak detection
valgrind --leak-check=full ./test_executable

# macOS specific
leaks ./test_executable
```

---

## ✅ Testing Checklists

### Before Each Release

#### General Testing
- [ ] Windows build compiles without errors
- [ ] macOS build compiles with minimal errors (track progress)
- [ ] Linux build compiles without errors
- [ ] All replay tests pass on Windows
- [ ] Core libraries link successfully
- [ ] Game startup doesn't crash immediately

#### OpenGL Testing
- [ ] All automated OpenGL tests pass
- [ ] Manual testing on all supported platforms
- [ ] Performance benchmarks within acceptable ranges
- [ ] Memory leak testing completed
- [ ] Visual regression testing completed

#### OpenGL Testing
- [ ] All automated OpenGL tests pass
- [ ] Manual testing on all supported platforms
- [ ] Performance benchmarks within acceptable ranges
- [ ] Memory leak testing completed
- [ ] Visual regression testing completed
- [ ] OpenGL context creation works on all platforms
- [ ] W3D integration tests pass

**→ For detailed OpenGL testing procedures, see [OPENGL_TESTING.md](./OPENGL_TESTING.md)**

#### Platform-Specific Testing

##### macOS Port Specific
- [ ] All DirectX interfaces compile without missing methods
- [ ] Windows API compatibility functions work correctly
- [ ] No pointer casting errors on 64-bit macOS
- [ ] CMake properly detects macOS platform and applies compatibility headers
- [ ] Core libraries (core_wwlib) build successfully
- [ ] WW3D2 graphics module compiles with <5 file failures

##### Linux Specific
- [ ] Package dependencies correctly specified
- [ ] Distribution-specific builds work (Ubuntu, Fedora, etc.)
- [ ] OpenGL context creation works with different window managers

---

## 🔄 CI/CD Integration

### Automated Testing Pipeline

#### Daily Builds
```yaml
# Example CI configuration
build-matrix:
  - platform: windows
    config: release
  - platform: macos
    config: debug
  - platform: linux
    config: release
```

#### Test Execution
- Exit codes indicate success/failure
- Colored output for human readability
- Detailed error reporting
- Cleanup of temporary files
- Performance regression detection

### Test Artifact Management
- Build logs preserved for analysis
- Test output captured and indexed
- Performance metrics tracked over time
- Failure patterns analyzed for trends

---

## 📝 Adding New Tests

### Test File Guidelines
1. Create test file in appropriate subdirectory (`tests/category/`)
2. Follow naming convention: `test_<feature>_<type>.cpp`
3. Include comprehensive error reporting
4. Update test runner scripts if adding to main suite
5. Document test purpose and requirements

### Test Categories
- **Basic**: Core functionality tests
- **Integration**: System integration tests
- **Performance**: Benchmark and performance tests
- **Regression**: Tests for specific bug fixes
- **Platform**: Platform-specific functionality tests

---

## 📚 Related Documentation

- **[OPENGL_TESTING.md](./OPENGL_TESTING.md)** - OpenGL-specific testing procedures and results
- **[OPENGL_COMPLETE.md](OPENGL_SUMMARY.md)** - Complete OpenGL implementation documentation
- **[MACOS_PORT.md](./MACOS_PORT.md)** - macOS porting progress and platform-specific details
- **[tests/README.md](./tests/README.md)** - Test suite directory index

---

**Note**: This comprehensive testing infrastructure supports the full Command & Conquer: Generals project across all platforms and build configurations. All tests should pass before integrating changes into the main codebase.
