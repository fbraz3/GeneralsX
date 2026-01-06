# Phase 47 — Quick Reference Guide

**Status**: Stage 1 Complete ✅, Stage 2-8 Pending  
**Current Focus**: Graphics Unit Tests (Stage 2)  
**Session**: November 1, 2025

## Quick Start

### View Phase 47 Documentation
```bash
# Overview
cat docs/WORKDIR/phases/4/PHASE47/README.md

# Planning and strategy
cat docs/WORKDIR/phases/4/PHASE47/PLANNING.md

# Stage 1 completion details
cat docs/WORKDIR/phases/4/PHASE47/STAGE1_COMPLETE.md

# Session summary
cat docs/WORKDIR/phases/4/PHASE47/SESSION_SUMMARY_NOV1.md
```

### Run Tests (When Available)
```bash
# Build test infrastructure
cmake --build build/macos-arm64 --target phase47_tests -j 4

# Run tests
./tests/run_tests.sh

# View results
cat test_results.txt
cat cpu_profile.txt
cat memory_profile.txt
```

## Current Status

### Stage 1: Test Infrastructure ✅ COMPLETE
- TestRunner with automatic registration
- 10 assertion macro types
- CPU, GPU, and Memory profilers
- Test utilities and helpers
- 1,443 lines of production code
- 14 files created

### Stage 2: Graphics Unit Tests ⏳ PENDING
- Texture loading tests
- Buffer management tests
- Shader compilation tests
- Render pass validation tests
- Draw call recording tests

### Stages 3-8: Pending
- Game logic unit tests
- Integration tests
- Profiling system full integration
- Bug fixes and stabilization
- Performance optimization
- Final validation

## Key Files

| Path | Purpose | Status |
|------|---------|--------|
| `tests/core/test_macros.h/cpp` | Assertions & macros | ✅ Complete |
| `tests/core/test_runner.h/cpp` | Test execution | ✅ Complete |
| `tests/core/test_utils.h/cpp` | Test utilities | ✅ Complete |
| `tests/core/profiler/cpu_profiler.h/cpp` | CPU timing | ✅ Complete |
| `tests/core/profiler/gpu_profiler.h/cpp` | GPU queries | ✅ Complete |
| `tests/core/profiler/memory_profiler.h/cpp` | Memory tracking | ✅ Complete |
| `tests/main.cpp` | Test entry point | ✅ Complete |
| `tests/run_tests.sh` | Execution script | ✅ Complete |
| `tests/graphics/` | Graphics tests | ⏳ Pending |
| `tests/game_logic/` | Game logic tests | ⏳ Pending |
| `tests/integration/` | Integration tests | ⏳ Pending |

## Testing API Quick Reference

### Assertions
```cpp
ASSERT_TRUE(condition);
ASSERT_FALSE(condition);
ASSERT_EQ(expected, actual);
ASSERT_NE(value1, value2);
ASSERT_LT(a, b);
ASSERT_LE(a, b);
ASSERT_GT(a, b);
ASSERT_GE(a, b);
ASSERT_NULL(ptr);
ASSERT_NOT_NULL(ptr);
ASSERT_STREQ(str1, str2);
ASSERT_FLOAT_EQ(a, b, tolerance);
```

### Test Registration
```cpp
TEST(SuiteName, TestName) {
    ASSERT_TRUE(1 + 1 == 2);
}

TEST_F(FixtureName, TestName) {
    // Fixture initialized before test
}

PERF_TEST(SuiteName, TestName) {
    // Performance test
}
```

### CPU Profiling
```cpp
CPU_PROFILE_SCOPE("FunctionName");  // RAII guard
// or
CPU_PROFILE_BEGIN("name");
// ... code ...
CPU_PROFILE_END("name");
```

### GPU Profiling
```cpp
GPU_PROFILE_BEGIN("DrawCalls");
// ... GPU commands ...
GPU_PROFILE_END("DrawCalls");
```

### Memory Tracking
```cpp
MEMORY_TRACK_ALLOC(ptr, size, "tag");
MEMORY_TRACK_FREE(ptr);
```

## Architecture Overview

```
Phase 47 Testing & Optimization

Test Infrastructure (Stage 1) ✅
├── TestRunner (automatic registration)
├── 10 Assertion macros
├── Test utilities (graphics, game logic, perf)
└── Profilers (CPU, GPU, Memory)

Graphics Tests (Stage 2) ⏳
├── Texture loading
├── Buffer management
├── Shader compilation
├── Render pass validation
└── Draw call recording

Game Logic Tests (Stage 3) ⏳
├── GameObject lifecycle
├── GameWorld queries
├── Game loop coordination
├── Input system
└── Camera system

Integration Tests (Stage 4) ⏳
├── Complete render loop
├── Game loop integration
├── Multi-object rendering
└── Gameplay workflows

Profiling (Stage 5) ⏳
├── CPU timing analysis
├── GPU utilization
└── Memory usage tracking

Bug Fixes (Stage 6) ⏳
├── Nullptr crashes
├── Memory leaks
├── GPU sync issues
└── Vulkan validation

Optimization (Stage 7) ⏳
├── Draw call batching
├── Texture atlasing
├── LOD system
└── Frustum culling

Validation (Stage 8) ⏳
├── Test execution
├── Performance metrics
├── Stress testing
└── Documentation
```

## Git History

```
98b1b77d docs(phase-47): add development diary entry for stage 1
a052a925 feat(phase-47): implement comprehensive testing and profiling infrastructure
```

## Performance Targets

### Build
- Full build: ~20-30 minutes (with ccache)
- Incremental build: ~30-60 seconds

### Testing
- Quick tests: < 30 seconds
- Full suite: 5-10 minutes
- Stress test: 30 minutes

### Runtime
- 60 FPS target
- < 16.67ms per frame
- < 512MB memory

## Integration Points

### Phase 40-44 (Graphics)
- ✅ Ready for graphics unit tests
- ✅ Test utilities for texture/buffer

### Phase 45-46 (Game Logic)
- ✅ Ready for game logic tests
- ✅ Test utilities for GameObject/GameWorld

### Phase 39+ (Vulkan)
- ✅ GPU profiler ready for VkQueryPool
- ⏳ Integration pending Phase 39

## References

- [Phase 47 README](./README.md)
- [Phase 47 Planning](./PLANNING.md)
- [Stage 1 Implementation](./STAGE1_TEST_INFRASTRUCTURE.md)
- [Stage 1 Completion](./STAGE1_COMPLETE.md)
- [Session Summary](./SESSION_SUMMARY_NOV1.md)

## Next Steps

1. **Stage 2: Graphics Unit Tests**
   - Create graphics test files
   - Implement texture tests
   - Implement buffer tests
   - Run and validate

2. **Stage 3: Game Logic Tests**
   - Create game logic test files
   - Implement GameObject tests
   - Implement GameWorld tests
   - Run and validate

3. **Stages 4-8: Complete Testing Pipeline**
   - Integration tests
   - Profiling integration
   - Bug identification
   - Performance optimization
   - Final validation

---

**Ready to proceed with Stage 2** ✅
