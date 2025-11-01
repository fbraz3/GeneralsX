#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

namespace phase47 {

// Forward declarations
class GameWorld;
class GameObject;
class Unit;
class Building;
class Effect;
struct Vector3;
struct Quaternion;
struct Matrix4x4;

// Graphics test utilities
namespace graphics_utils {

// Texture creation helpers
void* CreateTestTexture(uint32_t width, uint32_t height, const std::string& format);
void DestroyTestTexture(void* texture);

// Buffer creation helpers
void* CreateTestBuffer(uint32_t size, const void* data = nullptr);
void DestroyTestBuffer(void* buffer);

// Shader helpers
void* CreateTestShader(const char* code);
void DestroyTestShader(void* shader);

// Render pass helpers
void* CreateTestRenderPass();
void DestroyTestRenderPass(void* render_pass);

}  // namespace graphics_utils

// Game logic test utilities
namespace game_utils {

// GameWorld creation
GameWorld* CreateTestGameWorld();
void DestroyTestGameWorld(GameWorld* world);

// GameObject creation
GameObject* CreateTestObject(GameWorld* world, int object_type, const Vector3& position);

// Unit creation
Unit* CreateTestUnit(GameWorld* world, const Vector3& position, float speed = 5.0f);

// Building creation
Building* CreateTestBuilding(GameWorld* world, const Vector3& position);

// Effect creation
Effect* CreateTestEffect(GameWorld* world, const Vector3& position, float lifetime);

// Utility functions
void UpdateGameWorld(GameWorld* world, float delta_time, int frame_count);
void RenderGameWorld(GameWorld* world);

}  // namespace game_utils

// Performance measurement utilities
namespace perf_utils {

struct PerfMetrics {
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
    double total_time_ms;
    int sample_count;
};

class PerfTimer {
public:
    PerfTimer();
    void Start();
    void Stop();
    double GetElapsedMs() const;
    void Reset();

private:
    std::chrono::high_resolution_clock::time_point start_time_;
    double elapsed_ms_;
};

PerfMetrics MeasureFunction(
    std::function<void()> fn,
    int iterations = 100
);

PerfMetrics MeasureFrameTiming(
    std::function<void()> frame_fn,
    int frame_count = 1000
);

double MeasureMemoryUsage(std::function<void()> fn);
double GetCurrentMemoryUsage();

}  // namespace perf_utils

// Memory utilities
class MemoryTracker {
public:
    static MemoryTracker& Instance() {
        static MemoryTracker instance;
        return instance;
    }

    void TrackAllocation(void* ptr, size_t size, const char* tag);
    void UntrackAllocation(void* ptr);
    size_t GetTotalAllocated() const;
    size_t GetAllocationCount() const;
    void PrintReport() const;
    void Clear();

private:
    struct Allocation {
        void* ptr;
        size_t size;
        std::string tag;
    };

    std::vector<Allocation> allocations_;
};

}  // namespace phase47
