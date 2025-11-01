#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace phase47 {

// Forward declare Vulkan types
struct VkQueryPool_T;
using VkQueryPool = VkQueryPool_T*;

class GPUProfiler {
public:
    static GPUProfiler& Instance() {
        static GPUProfiler instance;
        return instance;
    }

    // Initialize GPU profiling (must be called after device creation)
    bool Initialize(void* vulkan_device, void* command_pool);

    // Begin a named GPU query
    void BeginQuery(const std::string& query_name);

    // End the current GPU query
    void EndQuery(const std::string& query_name);

    // Get timing for a specific query
    struct QueryStats {
        double total_time_ms;
        double min_time_ms;
        double max_time_ms;
        double avg_time_ms;
        uint32_t sample_count;
    };

    QueryStats GetQueryStats(const std::string& query_name) const;

    // Collect query results (call this after rendering)
    void CollectResults(void* command_buffer);

    // Print profiling report
    void PrintReport() const;

    // Generate report to file
    void GenerateReport(const std::string& filename) const;

    // Clear all data
    void Clear();

    // Check if GPU profiling is available
    bool IsAvailable() const { return available_; }

private:
    GPUProfiler() = default;

    struct QueryData {
        std::string name;
        uint32_t query_index;
        double total_time_ms = 0.0;
        double min_time_ms = std::numeric_limits<double>::max();
        double max_time_ms = 0.0;
        uint32_t sample_count = 0;
    };

    void* vulkan_device_ = nullptr;
    void* command_pool_ = nullptr;
    std::vector<VkQueryPool> query_pools_;
    std::map<std::string, QueryData> queries_;
    uint32_t current_query_index_ = 0;
    bool available_ = false;

    const uint32_t QUERIES_PER_POOL = 1024;
};

// Convenience macros
#define GPU_PROFILE_BEGIN(name) \
    phase47::GPUProfiler::Instance().BeginQuery(name)

#define GPU_PROFILE_END(name) \
    phase47::GPUProfiler::Instance().EndQuery(name)

}  // namespace phase47
