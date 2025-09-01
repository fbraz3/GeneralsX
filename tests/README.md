# Test Suite Index
## Command & Conquer: Generals - Test Directory

This directory contains all automated tests for the project.

## 📋 Quick Reference

## 📚 Documentation Integration

This test directory is part of a comprehensive testing strategy:

**General Testing:** [../TESTING_COMPLETE.md](../TESTING.md) - Covers all project testing including platform-specific procedures, build configurations, and CI/CD workflows

**OpenGL Testing:** [../OPENGL_TESTING.md](../OPENGL_TESTING.md) - Detailed OpenGL testing procedures, results, troubleshooting, and development workflow

**Implementation:** [../OPENGL_COMPLETE.md](../OPENGL_SUMMARY.md) - Complete OpenGL implementation guide and architecture

## 📁 Directory Structure

```
tests/
├── README.md               # This index file
└── opengl/                 # OpenGL-specific tests
    ├── run_opengl_tests.sh # Main test runner script
    ├── test_*.cpp          # Individual test files
    └── ... (test artifacts)
```

## 🚀 Quick Start

### Run All OpenGL Tests
```bash
# From project root
./tests/opengl/run_opengl_tests.sh
```

### Run Individual Tests
```bash
cd tests/opengl
./run_opengl_tests.sh <test_name>
```

## 📊 Current Test Status

| Test Suite | Status | Location |
|------------|--------|----------|
| OpenGL Basic | ✅ PASS | `opengl/test_opengl_*.cpp` |
| W3D Integration | ✅ PASS | `opengl/test_w3d_*.cpp` |
| Graphics Pipeline | ✅ PASS | `opengl/test_graphics.cpp` |
| Compilation | ✅ PASS | `opengl/test_*_compile.cpp` |

## 📚 Documentation

- **[TESTING_COMPLETE.md](../TESTING.md)** - Comprehensive testing guide for all platforms and configurations
- **[OPENGL_TESTING.md](../OPENGL_TESTING.md)** - Detailed OpenGL testing procedures and results

---

**Note**: This directory structure supports automated testing for the OpenGL migration and cross-platform porting efforts. All tests should pass before code integration.
