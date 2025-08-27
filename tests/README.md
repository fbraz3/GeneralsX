# Test Suite for Command & Conquer: Generals OpenGL Migration

This directory contains all tests for the OpenGL graphics system migration.

## Directory Structure

```
tests/
├── README.md               # This file
└── opengl/                 # OpenGL-specific tests
    ├── run_opengl_tests.sh # Main test runner script
    ├── test_opengl_minimal.cpp     # Basic functionality test
    ├── test_opengl_context.cpp     # Context creation test
    ├── test_w3d_integration.cpp    # W3D integration test
    └── ... (other test files)
```

## Running Tests

### All OpenGL Tests
```bash
# From project root
./tests/opengl/run_opengl_tests.sh

# Or from test directory
cd tests/opengl
./run_opengl_tests.sh
```

### Individual Tests
```bash
# From project root
cd tests/opengl

# Basic functionality
g++ -I../../Core/Libraries/Include -I../../Core/Libraries/Include/GraphicsAPI \
    -I../../Dependencies/Utility test_opengl_minimal.cpp \
    ../../Core/Libraries/Source/GraphicsAPI/OpenGLRenderer.cpp \
    ../../Core/Libraries/Source/GraphicsAPI/GraphicsRenderer.cpp \
    -o test_opengl_minimal -DENABLE_OPENGL -std=c++11 -framework OpenGL
./test_opengl_minimal

# W3D Integration
g++ -I../../Core/Libraries/Include -I../../Core/Libraries/Include/GraphicsAPI \
    -I../../Dependencies/Utility test_w3d_integration.cpp \
    ../../Core/Libraries/Source/GraphicsAPI/OpenGLRendererW3D.cpp \
    ../../Core/Libraries/Source/GraphicsAPI/OpenGLRenderer.cpp \
    ../../Core/Libraries/Source/GraphicsAPI/GraphicsRenderer.cpp \
    -o test_w3d_integration -DENABLE_OPENGL -DENABLE_W3D_INTEGRATION \
    -std=c++11 -framework OpenGL
./test_w3d_integration
```

## Test Files Description

### Core OpenGL Tests

- **`test_opengl_minimal.cpp`**: Basic OpenGL functionality and type system
- **`test_opengl_context.cpp`**: OpenGL context creation and management
- **`test_opengl_compile.cpp`**: Compilation verification test

### Integration Tests

- **`test_w3d_integration.cpp`**: W3D type system integration with OpenGL
- **`test_full_integration.cpp`**: Complete system integration test
- **`test_advanced_compile.cpp`**: Advanced compilation scenarios

### Graphics Tests

- **`test_graphics.cpp`**: General graphics system tests
- **`test_simple_compile.cpp`**: Simple compilation verification

## Test Requirements

### System Requirements
- **macOS**: OpenGL via Metal backend (with deprecation warnings)
- **Linux**: OpenGL via GLX
- **Windows**: OpenGL via WGL

### Build Requirements
- C++11 compatible compiler (g++, clang++)
- OpenGL development libraries
- CMake 3.25+ (for full project builds)

### Dependencies
- Core Graphics API library
- W3D type system (mock or real)
- Utility library for compatibility headers

## Test Status

| Test Category | Status | Notes |
|---------------|--------|-------|
| Basic Functionality | ✅ PASS | Core OpenGL operations working |
| Context Creation | ✅ PASS | Platform-specific context creation |
| Compilation | ✅ PASS | All modules compile correctly |
| W3D Integration | ✅ PASS | Mock W3D types integrated |

## Troubleshooting

### Common Issues

1. **Include Path Errors**: Ensure you're running from project root or using relative paths correctly
2. **OpenGL Deprecation (macOS)**: Add `-DGL_SILENCE_DEPRECATION` to suppress warnings
3. **Missing Dependencies**: Ensure `core_utility` and other dependencies are built

### Debug Mode
Add `-g -DDEBUG` to compilation flags for debug builds:
```bash
g++ -g -DDEBUG -I../../Core/Libraries/Include ... (rest of flags)
```

## Adding New Tests

1. Create test file in appropriate subdirectory (`tests/opengl/`)
2. Follow naming convention: `test_<feature>_<type>.cpp`
3. Update `run_opengl_tests.sh` if adding to main test suite
4. Document test purpose and requirements in this README

## Integration with CI/CD

The test suite is designed to be CI/CD friendly:
- Exit codes indicate success/failure
- Colored output for human readability
- Detailed error reporting
- Cleanup of temporary files

---

**Note**: This test infrastructure supports the OpenGL migration project for Command & Conquer: Generals. All tests should pass before integrating changes into the main game engine.
