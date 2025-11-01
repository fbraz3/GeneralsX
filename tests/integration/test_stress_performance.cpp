#include "../core/test_macros.h"
#include "../core/test_utils.h"
#include "../core/profiler/cpu_profiler.h"
#include "../core/profiler/memory_profiler.h"
#include <cmath>
#include <iostream>

namespace phase47 {
namespace integration_tests {

// Performance scaling tests
TEST(PerformanceScaling, LargeObjectCount) {
    CPU_PROFILE_SCOPE("test::LargeObjectCount");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create 100 units
    for (int i = 0; i < 100; ++i) {
        float x = (i % 10) * 10.0f;
        float z = (i / 10) * 10.0f;
        Vector3 pos = {x, 0.0f, z};
        Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        ASSERT_NOT_NULL(unit);
    }
    
    // Simulate frames
    for (int frame = 0; frame < 60; ++frame) {
        game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 1);
    }
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(PerformanceScaling, TextureIntensiveRendering) {
    CPU_PROFILE_SCOPE("test::TextureIntensiveRendering");
    
    // Create many textures
    for (int i = 0; i < 50; ++i) {
        void* texture = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
        ASSERT_NOT_NULL(texture);
        graphics_utils::DestroyTestTexture(texture);
    }
}

TEST(PerformanceScaling, BufferIntensiveRendering) {
    CPU_PROFILE_SCOPE("test::BufferIntensiveRendering");
    
    // Create many buffers
    for (int i = 0; i < 100; ++i) {
        void* buffer = graphics_utils::CreateTestBuffer(4096);
        ASSERT_NOT_NULL(buffer);
        graphics_utils::DestroyTestBuffer(buffer);
    }
}

// Stress tests
TEST(StressTest, ContinuousGameplay) {
    CPU_PROFILE_SCOPE("test::ContinuousGameplay");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create initial units
    for (int i = 0; i < 25; ++i) {
        Vector3 pos = {static_cast<float>(i * 5), 0.0f, 0.0f};
        Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        ASSERT_NOT_NULL(unit);
    }
    
    // Simulate 10 seconds of gameplay
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 600);
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(StressTest, MemoryStability) {
    CPU_PROFILE_SCOPE("test::MemoryStability");
    
    MemoryProfiler::Instance().Clear();
    size_t initial = MemoryProfiler::Instance().GetTotalAllocated();
    
    for (int cycle = 0; cycle < 10; ++cycle) {
        GameWorld* world = game_utils::CreateTestGameWorld();
        
        for (int i = 0; i < 10; ++i) {
            Vector3 pos = {static_cast<float>(i * 5), 0.0f, 0.0f};
            Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        }
        
        game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 60);
        game_utils::DestroyTestGameWorld(world);
    }
    
    size_t final = MemoryProfiler::Instance().GetTotalAllocated();
    
    // Memory should remain relatively stable
    ASSERT_LT(final - initial, 1000000);  // Less than 1MB growth
}

}  // namespace integration_tests
}  // namespace phase47
