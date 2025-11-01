#include "../core/test_macros.h"
#include "../core/test_utils.h"
#include "../core/profiler/cpu_profiler.h"
#include "../core/profiler/memory_profiler.h"
#include <iostream>

namespace phase47 {
namespace integration_tests {

// Complete render loop integration tests
TEST(RenderLoopIntegration, BasicRenderCycle) {
    CPU_PROFILE_SCOPE("test::BasicRenderCycle");
    
    // Create graphics resources
    void* texture = graphics_utils::CreateTestTexture(512, 512, "RGBA8");
    void* buffer = graphics_utils::CreateTestBuffer(2048);
    void* shader = graphics_utils::CreateTestShader(R"(void main(){})");
    void* render_pass = graphics_utils::CreateTestRenderPass();
    
    ASSERT_NOT_NULL(texture);
    ASSERT_NOT_NULL(buffer);
    ASSERT_NOT_NULL(shader);
    ASSERT_NOT_NULL(render_pass);
    
    // Cleanup
    graphics_utils::DestroyTestTexture(texture);
    graphics_utils::DestroyTestBuffer(buffer);
    graphics_utils::DestroyTestShader(shader);
    graphics_utils::DestroyTestRenderPass(render_pass);
}

TEST(RenderLoopIntegration, MultiFrameRenderCycle) {
    CPU_PROFILE_SCOPE("test::MultiFrameRenderCycle");
    
    void* render_pass = graphics_utils::CreateTestRenderPass();
    ASSERT_NOT_NULL(render_pass);
    
    // Simulate 60 frames of rendering
    for (int frame = 0; frame < 60; ++frame) {
        void* texture = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
        ASSERT_NOT_NULL(texture);
        graphics_utils::DestroyTestTexture(texture);
    }
    
    graphics_utils::DestroyTestRenderPass(render_pass);
}

TEST(RenderLoopIntegration, RenderWithGameLogic) {
    CPU_PROFILE_SCOPE("test::RenderWithGameLogic");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create game objects
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
    ASSERT_NOT_NULL(unit);
    
    // Render multiple frames
    void* render_pass = graphics_utils::CreateTestRenderPass();
    ASSERT_NOT_NULL(render_pass);
    
    for (int frame = 0; frame < 30; ++frame) {
        game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 1);
        game_utils::RenderGameWorld(world);
    }
    
    graphics_utils::DestroyTestRenderPass(render_pass);
    game_utils::DestroyTestGameWorld(world);
}

// Game loop integration tests
TEST(GameLoopIntegration, CompleteGameLoopCycle) {
    CPU_PROFILE_SCOPE("test::CompleteGameLoopCycle");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create various game objects
    for (int i = 0; i < 5; ++i) {
        Vector3 pos = {static_cast<float>(i * 20), 0.0f, 0.0f};
        Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        ASSERT_NOT_NULL(unit);
    }
    
    Building* building = game_utils::CreateTestBuilding(world, {50.0f, 0.0f, 0.0f});
    ASSERT_NOT_NULL(building);
    
    // Simulate complete game loop
    for (int frame = 0; frame < 100; ++frame) {
        game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 1);
        game_utils::RenderGameWorld(world);
    }
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameLoopIntegration, MultiObjectGameplay) {
    CPU_PROFILE_SCOPE("test::MultiObjectGameplay");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create multiple units in formation
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            Vector3 pos = {
                static_cast<float>(col * 15),
                0.0f,
                static_cast<float>(row * 15)
            };
            Unit* unit = game_utils::CreateTestUnit(world, pos, 7.0f);
            ASSERT_NOT_NULL(unit);
        }
    }
    
    // Simulate gameplay
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 120);
    
    game_utils::DestroyTestGameWorld(world);
}

// Complete workflow tests
TEST(CompleteWorkflow, FullGameSession) {
    CPU_PROFILE_SCOPE("test::FullGameSession");
    
    // Initialize world
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create graphics resources
    void* texture = graphics_utils::CreateTestTexture(1024, 768, "RGBA8");
    void* buffer = graphics_utils::CreateTestBuffer(4096);
    void* shader = graphics_utils::CreateTestShader(R"(void main(){})");
    
    ASSERT_NOT_NULL(texture);
    ASSERT_NOT_NULL(buffer);
    ASSERT_NOT_NULL(shader);
    
    // Create game entities
    for (int i = 0; i < 10; ++i) {
        Vector3 pos = {static_cast<float>(i * 10), 0.0f, 0.0f};
        Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        ASSERT_NOT_NULL(unit);
    }
    
    // Run game loop
    for (int frame = 0; frame < 180; ++frame) {  // 3 seconds at 60 FPS
        game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 1);
        game_utils::RenderGameWorld(world);
    }
    
    // Cleanup
    graphics_utils::DestroyTestTexture(texture);
    graphics_utils::DestroyTestBuffer(buffer);
    graphics_utils::DestroyTestShader(shader);
    game_utils::DestroyTestGameWorld(world);
}

TEST(CompleteWorkflow, ScaledGameplay) {
    CPU_PROFILE_SCOPE("test::ScaledGameplay");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create many objects to test scaling
    for (int i = 0; i < 50; ++i) {
        float angle = (i / 50.0f) * 6.28f;  // 2π
        float distance = 30.0f + (i % 10) * 5;
        Vector3 pos = {
            distance * std::cos(angle),
            0.0f,
            distance * std::sin(angle)
        };
        Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        ASSERT_NOT_NULL(unit);
    }
    
    // Simulate 5 seconds of gameplay
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 300);
    
    game_utils::DestroyTestGameWorld(world);
}

// Memory integration tests
TEST(MemoryIntegration, AllocationTracking) {
    CPU_PROFILE_SCOPE("test::AllocationTracking");
    
    MemoryProfiler::Instance().Clear();
    
    // Allocate test resources
    void* texture = graphics_utils::CreateTestTexture(512, 512, "RGBA8");
    MEMORY_TRACK_ALLOC(texture, 512 * 512 * 4, "texture");
    
    void* buffer = graphics_utils::CreateTestBuffer(2048);
    MEMORY_TRACK_ALLOC(buffer, 2048, "buffer");
    
    // Check tracking
    size_t total = MemoryProfiler::Instance().GetTotalAllocated();
    ASSERT_GT(total, 0);
    
    // Cleanup
    MEMORY_TRACK_FREE(texture);
    MEMORY_TRACK_FREE(buffer);
    graphics_utils::DestroyTestTexture(texture);
    graphics_utils::DestroyTestBuffer(buffer);
}

TEST(MemoryIntegration, NoMemoryLeaks) {
    CPU_PROFILE_SCOPE("test::NoMemoryLeaks");
    
    size_t before = MemoryProfiler::Instance().GetTotalAllocated();
    
    {
        GameWorld* world = game_utils::CreateTestGameWorld();
        
        for (int i = 0; i < 20; ++i) {
            Vector3 pos = {static_cast<float>(i * 5), 0.0f, 0.0f};
            Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        }
        
        game_utils::DestroyTestGameWorld(world);
    }
    
    size_t after = MemoryProfiler::Instance().GetTotalAllocated();
    
    // Should not have significant memory growth (allow 10% tolerance)
    ASSERT_LT(after - before, before * 0.1f);
}

}  // namespace integration_tests
}  // namespace phase47
