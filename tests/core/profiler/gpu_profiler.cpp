#include "gpu_profiler.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace phase47 {

bool GPUProfiler::Initialize(void* vulkan_device, void* command_pool) {
    vulkan_device_ = vulkan_device;
    command_pool_ = command_pool;

    // In a real implementation, we would create Vulkan query pools here
    // For now, mark as available for testing infrastructure
    available_ = (vulkan_device != nullptr && command_pool != nullptr);

    return available_;
}

void GPUProfiler::BeginQuery(const std::string& query_name) {
    if (!available_) return;

    auto& query = queries_[query_name];
    query.name = query_name;
    query.query_index = current_query_index_++;

    if (current_query_index_ >= QUERIES_PER_POOL) {
        current_query_index_ = 0;
        // In a real implementation, allocate a new query pool
    }
}

void GPUProfiler::EndQuery(const std::string& query_name) {
    if (!available_) return;

    // In a real implementation, this would record the end of the Vulkan query
}

GPUProfiler::QueryStats GPUProfiler::GetQueryStats(const std::string& query_name) const {
    auto it = queries_.find(query_name);
    if (it != queries_.end()) {
        const auto& data = it->second;
        QueryStats stats;
        stats.total_time_ms = data.total_time_ms;
        stats.min_time_ms = data.min_time_ms;
        stats.max_time_ms = data.max_time_ms;
        stats.sample_count = data.sample_count;
        stats.avg_time_ms = (data.sample_count > 0) ? 
            data.total_time_ms / data.sample_count : 0.0;
        return stats;
    }

    QueryStats empty;
    empty.total_time_ms = 0.0;
    empty.min_time_ms = 0.0;
    empty.max_time_ms = 0.0;
    empty.avg_time_ms = 0.0;
    empty.sample_count = 0;
    return empty;
}

void GPUProfiler::CollectResults(void* command_buffer) {
    if (!available_) return;

    // In a real implementation, this would call vkGetQueryPoolResults
    // to retrieve GPU timing data
}

void GPUProfiler::PrintReport() const {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "GPU Profiling Report (Vulkan Queries)\n";
    std::cout << "═══════════════════════════════════════════════\n";

    if (!available_) {
        std::cout << "GPU profiling not available\n";
        std::cout << "═══════════════════════════════════════════════\n";
        return;
    }

    double total_time = 0.0;

    for (const auto& pair : queries_) {
        const auto& data = pair.second;
        total_time += data.total_time_ms;

        std::cout << std::left << std::setw(30) << data.name << ": ";
        std::cout << std::fixed << std::setprecision(2) << std::setw(8) << data.total_time_ms
                  << "ms total | ";
        std::cout << std::setw(8) << (data.sample_count > 0 ? data.total_time_ms / data.sample_count : 0.0)
                  << "ms avg | ";
        std::cout << std::setw(6) << data.sample_count << " samples\n";
    }

    std::cout << "─────────────────────────────────────────────────\n";
    std::cout << "Total GPU Time: " << std::fixed << std::setprecision(2) << total_time << "ms\n";
    std::cout << "═══════════════════════════════════════════════\n";
}

void GPUProfiler::GenerateReport(const std::string& filename) const {
    std::ofstream report(filename);

    report << "GPU Profiling Report (Vulkan Queries)\n";
    report << "═══════════════════════════════════════════════\n\n";

    if (!available_) {
        report << "GPU profiling not available\n";
        report.close();
        return;
    }

    double total_time = 0.0;

    for (const auto& pair : queries_) {
        const auto& data = pair.second;
        total_time += data.total_time_ms;

        report << std::left << std::setw(30) << data.name << ": ";
        report << std::fixed << std::setprecision(4)
               << "Total=" << std::setw(10) << data.total_time_ms << "ms, "
               << "Avg=" << std::setw(10) << (data.sample_count > 0 ? data.total_time_ms / data.sample_count : 0.0) << "ms, "
               << "Samples=" << std::setw(6) << data.sample_count << "\n";
    }

    report << "\n═══════════════════════════════════════════════\n";
    report << "Total GPU Time: " << std::fixed << std::setprecision(2) << total_time << "ms\n";
    report.close();
}

void GPUProfiler::Clear() {
    queries_.clear();
    current_query_index_ = 0;
}

}  // namespace phase47
