#include "test_utils.h"
#include <iostream>
#include <chrono>
#include <numeric>

namespace phase47 {

namespace perf_utils {

PerfTimer::PerfTimer() : elapsed_ms_(0.0) {}

void PerfTimer::Start() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

void PerfTimer::Stop() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time_
    ).count();
    elapsed_ms_ = duration_us / 1000.0;
}

double PerfTimer::GetElapsedMs() const {
    return elapsed_ms_;
}

void PerfTimer::Reset() {
    elapsed_ms_ = 0.0;
}

PerfMetrics MeasureFunction(std::function<void()> fn, int iterations) {
    PerfMetrics metrics;
    metrics.min_time_ms = std::numeric_limits<double>::max();
    metrics.max_time_ms = 0.0;
    metrics.total_time_ms = 0.0;
    metrics.sample_count = iterations;

    std::vector<double> times;

    for (int i = 0; i < iterations; ++i) {
        PerfTimer timer;
        timer.Start();
        fn();
        timer.Stop();

        double elapsed = timer.GetElapsedMs();
        times.push_back(elapsed);
        metrics.total_time_ms += elapsed;
        metrics.min_time_ms = std::min(metrics.min_time_ms, elapsed);
        metrics.max_time_ms = std::max(metrics.max_time_ms, elapsed);
    }

    metrics.avg_time_ms = metrics.total_time_ms / iterations;

    return metrics;
}

PerfMetrics MeasureFrameTiming(std::function<void()> frame_fn, int frame_count) {
    return MeasureFunction(frame_fn, frame_count);
}

double MeasureMemoryUsage(std::function<void()> fn) {
    double initial_memory = GetCurrentMemoryUsage();
    fn();
    double final_memory = GetCurrentMemoryUsage();
    return final_memory - initial_memory;
}

double GetCurrentMemoryUsage() {
    // In a real implementation, this would query the system for memory usage
    // For now, return a placeholder
    return 0.0;
}

}  // namespace perf_utils

namespace graphics_utils {

void* CreateTestTexture(uint32_t width, uint32_t height, const std::string& format) {
    // Placeholder implementation
    return new uint8_t[width * height * 4];
}

void DestroyTestTexture(void* texture) {
    delete[] static_cast<uint8_t*>(texture);
}

void* CreateTestBuffer(uint32_t size, const void* data) {
    uint8_t* buffer = new uint8_t[size];
    if (data) {
        std::memcpy(buffer, data, size);
    }
    return buffer;
}

void DestroyTestBuffer(void* buffer) {
    delete[] static_cast<uint8_t*>(buffer);
}

void* CreateTestShader(const char* code) {
    // Placeholder implementation
    return new int(0);  // Dummy shader handle
}

void DestroyTestShader(void* shader) {
    delete static_cast<int*>(shader);
}

void* CreateTestRenderPass() {
    // Placeholder implementation
    return new int(0);  // Dummy render pass handle
}

void DestroyTestRenderPass(void* render_pass) {
    delete static_cast<int*>(render_pass);
}

}  // namespace graphics_utils

namespace game_utils {

GameWorld* CreateTestGameWorld() {
    // Placeholder - would need actual GameWorld class
    return nullptr;
}

void DestroyTestGameWorld(GameWorld* world) {
    // Placeholder
}

GameObject* CreateTestObject(GameWorld* world, int object_type, const Vector3& position) {
    // Placeholder
    return nullptr;
}

Unit* CreateTestUnit(GameWorld* world, const Vector3& position, float speed) {
    // Placeholder
    return nullptr;
}

Building* CreateTestBuilding(GameWorld* world, const Vector3& position) {
    // Placeholder
    return nullptr;
}

Effect* CreateTestEffect(GameWorld* world, const Vector3& position, float lifetime) {
    // Placeholder
    return nullptr;
}

void UpdateGameWorld(GameWorld* world, float delta_time, int frame_count) {
    // Placeholder
}

void RenderGameWorld(GameWorld* world) {
    // Placeholder
}

}  // namespace game_utils

}  // namespace phase47
