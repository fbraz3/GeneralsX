# Phase 47, Stage 1: Test Infrastructure Implementation

## Objective
Create comprehensive test runner, test utilities, and test harness for all upcoming unit, integration, and performance tests.

## Architecture

### Test Runner System
```cpp
class TestRunner {
    // Registration
    void RegisterTest(const std::string& name, TestFunction fn, TestCategory category);
    
    // Execution
    int RunAllTests();
    int RunCategory(TestCategory category);
    int RunTest(const std::string& name);
    
    // Reporting
    TestResult GetResult(const std::string& test_name);
    void PrintSummary();
    void GenerateReport(const std::string& filename);
};
```

### Test Macros
```cpp
// Basic assertions
TEST(SuiteName, TestName) {
    ASSERT_TRUE(condition);
    ASSERT_FALSE(condition);
    ASSERT_EQ(expected, actual);
    ASSERT_NE(value1, value2);
    ASSERT_LT(value1, value2);
    ASSERT_LE(value1, value2);
    ASSERT_GT(value1, value2);
    ASSERT_GE(value1, value2);
    ASSERT_NULL(ptr);
    ASSERT_NOT_NULL(ptr);
    ASSERT_STREQ(str1, str2);
}

// Performance assertions
PERF_TEST(SuiteName, TestName) {
    ASSERT_PERFORMANCE(block, max_time_ms);
    ASSERT_MEMORY_USAGE(block, max_bytes);
}
```

### Test Utility Library
```cpp
class TestUtils {
    // Graphics helpers
    static VulkanDevice* CreateTestDevice();
    static VkRenderPass CreateTestRenderPass(VulkanDevice* device);
    static VkShaderModule CreateTestShader(VulkanDevice* device, const char* code);
    static Texture* CreateTestTexture(VulkanDevice* device, int width, int height);
    static Buffer* CreateTestBuffer(VulkanDevice* device, size_t size);
    
    // Game logic helpers
    static GameWorld* CreateTestGameWorld();
    static GameObject* CreateTestObject(GameWorld* world, ObjectType type);
    static Unit* CreateTestUnit(GameWorld* world, const Vector3& position);
    static Building* CreateTestBuilding(GameWorld* world, const Vector3& position);
    
    // Performance helpers
    static void MeasureTime(const std::string& name, std::function<void()> fn);
    static void MeasureMemory(const std::string& name, std::function<void()> fn);
    static double GetAverageFrameTime(int frame_count);
};
```

### Test Categories
- Graphics (textures, buffers, shaders, rendering)
- GameLogic (objects, world, input, camera)
- Integration (render loop, game loop)
- Performance (frame timing, memory, stress)

## Implementation Files

### tests/core/test_macros.h
- Define `TEST()` macro for test registration
- Define assertion macros (`ASSERT_EQ`, `ASSERT_TRUE`, etc.)
- Define performance test macros (`PERF_TEST`)
- Track test results and statistics

### tests/core/test_runner.h/cpp
- TestRunner class implementation
- Test registration and execution
- Result aggregation and reporting
- Command-line test selection

### tests/core/test_utils.h/cpp
- Vulkan device/resource creation helpers
- Game world/object creation helpers
- Performance measurement utilities
- Memory tracking utilities

### tests/core/profiler/cpu_profiler.h/cpp
- Scope-based CPU timing
- Aggregate statistics collection
- Report generation

### tests/core/profiler/gpu_profiler.h/cpp
- Vulkan query pool management
- GPU timing queries
- Report generation

### tests/core/profiler/memory_profiler.h/cpp
- Allocation tracking
- Memory statistics
- Report generation

## CMakeLists.txt Updates

```cmake
# Create test executable
add_executable(phase47_tests
    tests/core/test_runner.cpp
    tests/core/test_utils.cpp
    tests/core/profiler/cpu_profiler.cpp
    tests/core/profiler/gpu_profiler.cpp
    tests/core/profiler/memory_profiler.cpp
    # Stage 2 graphics tests
    tests/graphics/test_texture_loading.cpp
    # Stage 3 game logic tests
    tests/game_logic/test_gameobject_lifecycle.cpp
    # ... more tests added in later stages
)

target_link_libraries(phase47_tests
    ww3d2
    wwlib
    core_engine
    vulkan
)

# Add test run command
add_custom_target(run-tests
    COMMAND phase47_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

## Execution Plan

1. Create `tests/core/` directory structure
2. Implement test macros and basic assertions
3. Implement TestRunner class with registration and execution
4. Implement test utilities for graphics and game logic
5. Implement profiler classes (CPU, GPU, Memory)
6. Create build targets and run commands
7. Execute and validate infrastructure

## Success Criteria

- ✅ Test runner can register and execute tests
- ✅ All assertion macros work correctly
- ✅ Test utilities can create test resources
- ✅ CPU profiler captures timing data
- ✅ GPU profiler records Vulkan query results
- ✅ Memory profiler tracks allocations
- ✅ Reports generate successfully
- ✅ Build system includes test targets

## Expected Output

```
$ ./phase47_tests
═══════════════════════════════════════════════
GeneralsX Phase 47 Test Suite
═══════════════════════════════════════════════

Running Tests...

[Graphics] test_texture_creation ............ PASS (2.3ms)
[Graphics] test_buffer_upload .............. PASS (1.1ms)
[GameLogic] test_gameobject_creation ....... PASS (0.8ms)

═══════════════════════════════════════════════
Results: 3/3 passed, 0 failed
Total Time: 4.2ms
═══════════════════════════════════════════════
```

## Next Steps (Stage 2)
- Implement graphics unit tests for texture, buffer, shader systems
- Focus on graphics pipeline validation
- Measure and report graphics performance
