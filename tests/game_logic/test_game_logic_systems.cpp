#include "../core/test_macros.h"
#include "../core/test_utils.h"
#include "../core/profiler/cpu_profiler.h"
#include <iostream>

namespace phase47 {
namespace game_logic_tests {

// GameObject tests
TEST(GameObjectLifecycle, ObjectCreation) {
    CPU_PROFILE_SCOPE("test::ObjectCreation");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create a test unit
    Vector3 pos = {10.0f, 0.0f, 20.0f};
    GameObject* obj = game_utils::CreateTestObject(world, 0, pos);
    ASSERT_NOT_NULL(obj);
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameObjectLifecycle, UnitCreation) {
    CPU_PROFILE_SCOPE("test::UnitCreation");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    Vector3 pos = {5.0f, 0.0f, 15.0f};
    Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
    ASSERT_NOT_NULL(unit);
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameObjectLifecycle, BuildingCreation) {
    CPU_PROFILE_SCOPE("test::BuildingCreation");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Building* building = game_utils::CreateTestBuilding(world, pos);
    ASSERT_NOT_NULL(building);
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameObjectLifecycle, EffectCreation) {
    CPU_PROFILE_SCOPE("test::EffectCreation");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    Vector3 pos = {25.0f, 5.0f, 30.0f};
    Effect* effect = game_utils::CreateTestEffect(world, pos, 2.0f);
    ASSERT_NOT_NULL(effect);
    
    game_utils::DestroyTestGameWorld(world);
}

// GameWorld tests
TEST(GameWorld, WorldCreation) {
    CPU_PROFILE_SCOPE("test::WorldCreation");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameWorld, MultipleObjectCreation) {
    CPU_PROFILE_SCOPE("test::MultipleObjectCreation");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create multiple objects
    for (int i = 0; i < 10; ++i) {
        Vector3 pos = {static_cast<float>(i * 5), 0.0f, 0.0f};
        Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        ASSERT_NOT_NULL(unit);
    }
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameWorld, SpatialQueries) {
    CPU_PROFILE_SCOPE("test::SpatialQueries");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create objects in a grid
    for (int x = 0; x < 5; ++x) {
        for (int z = 0; z < 5; ++z) {
            Vector3 pos = {static_cast<float>(x * 10), 0.0f, static_cast<float>(z * 10)};
            Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
            ASSERT_NOT_NULL(unit);
        }
    }
    
    game_utils::DestroyTestGameWorld(world);
}

// Game loop tests
TEST(GameLoop, FrameUpdate) {
    CPU_PROFILE_SCOPE("test::FrameUpdate");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Simulate 60 frames
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 60);
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameLoop, FrameRendering) {
    CPU_PROFILE_SCOPE("test::FrameRendering");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create some objects and render
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
    ASSERT_NOT_NULL(unit);
    
    game_utils::RenderGameWorld(world);
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(GameLoop, MultiFrameSimulation) {
    CPU_PROFILE_SCOPE("test::MultiFrameSimulation");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create multiple objects
    for (int i = 0; i < 5; ++i) {
        Vector3 pos = {static_cast<float>(i * 10), 0.0f, 0.0f};
        Unit* unit = game_utils::CreateTestUnit(world, pos, 5.0f);
        ASSERT_NOT_NULL(unit);
    }
    
    // Simulate multiple frames
    for (int frame = 0; frame < 120; ++frame) {
        game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 1);
        game_utils::RenderGameWorld(world);
    }
    
    game_utils::DestroyTestGameWorld(world);
}

// Unit behavior tests
TEST(UnitBehavior, UnitMovement) {
    CPU_PROFILE_SCOPE("test::UnitMovement");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Unit* unit = game_utils::CreateTestUnit(world, pos, 10.0f);
    ASSERT_NOT_NULL(unit);
    
    // Simulate unit movement
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 60);
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(UnitBehavior, UnitTargeting) {
    CPU_PROFILE_SCOPE("test::UnitTargeting");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    Vector3 pos1 = {0.0f, 0.0f, 0.0f};
    Vector3 pos2 = {50.0f, 0.0f, 0.0f};
    
    Unit* unit1 = game_utils::CreateTestUnit(world, pos1, 5.0f);
    Unit* unit2 = game_utils::CreateTestUnit(world, pos2, 5.0f);
    
    ASSERT_NOT_NULL(unit1);
    ASSERT_NOT_NULL(unit2);
    
    game_utils::DestroyTestGameWorld(world);
}

// Building behavior tests
TEST(BuildingBehavior, BuildingConstruction) {
    CPU_PROFILE_SCOPE("test::BuildingConstruction");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Building* building = game_utils::CreateTestBuilding(world, pos);
    ASSERT_NOT_NULL(building);
    
    // Simulate construction
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 300);  // 5 seconds at 60 FPS
    
    game_utils::DestroyTestGameWorld(world);
}

// Effect tests
TEST(EffectSystem, EffectLifetime) {
    CPU_PROFILE_SCOPE("test::EffectLifetime");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    Vector3 pos = {10.0f, 5.0f, 20.0f};
    Effect* effect = game_utils::CreateTestEffect(world, pos, 1.0f);
    ASSERT_NOT_NULL(effect);
    
    // Simulate effect lifetime
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 90);  // 1.5 seconds (longer than lifetime)
    
    game_utils::DestroyTestGameWorld(world);
}

TEST(EffectSystem, MultipleEffects) {
    CPU_PROFILE_SCOPE("test::MultipleEffects");
    
    GameWorld* world = game_utils::CreateTestGameWorld();
    ASSERT_NOT_NULL(world);
    
    // Create multiple effects
    for (int i = 0; i < 10; ++i) {
        Vector3 pos = {static_cast<float>(i * 5), static_cast<float>(i), 0.0f};
        Effect* effect = game_utils::CreateTestEffect(world, pos, 2.0f);
        ASSERT_NOT_NULL(effect);
    }
    
    // Simulate effects
    game_utils::UpdateGameWorld(world, 1.0f / 60.0f, 180);
    
    game_utils::DestroyTestGameWorld(world);
}

}  // namespace game_logic_tests
}  // namespace phase47
