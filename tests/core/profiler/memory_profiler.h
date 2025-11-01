#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace phase47 {

class MemoryProfiler {
public:
    static MemoryProfiler& Instance() {
        static MemoryProfiler instance;
        return instance;
    }

    // Track an allocation
    void TrackAllocation(void* ptr, size_t size, const char* tag);

    // Untrack an allocation
    void UntrackAllocation(void* ptr);

    // Get allocation info
    struct AllocationInfo {
        void* ptr;
        size_t size;
        std::string tag;
    };

    // Get total allocated memory
    size_t GetTotalAllocated() const;

    // Get allocation count
    uint32_t GetAllocationCount() const;

    // Get VRAM usage estimate
    size_t GetVRAMUsage() const;

    // Get peak memory usage
    size_t GetPeakMemory() const { return peak_memory_; }

    // Print memory report
    void PrintReport() const;

    // Generate report to file
    void GenerateReport(const std::string& filename) const;

    // Get allocations by tag
    std::vector<AllocationInfo> GetAllocationsByTag(const char* tag) const;

    // Clear all tracking
    void Clear();

    // Memory statistics
    struct MemoryStats {
        size_t total_allocated;
        size_t peak_memory;
        uint32_t allocation_count;
        size_t gpu_memory;
        double fragmentation;
    };

    MemoryStats GetStats() const;

private:
    MemoryProfiler() = default;

    struct Allocation {
        void* ptr;
        size_t size;
        std::string tag;
        uint64_t timestamp;
    };

    std::vector<Allocation> allocations_;
    size_t total_allocated_ = 0;
    size_t peak_memory_ = 0;
    uint64_t allocation_sequence_ = 0;

    // Helper to estimate GPU memory usage
    size_t EstimateGPUMemory() const;
};

// Convenience macro for memory tracking
#define MEMORY_TRACK_ALLOC(ptr, size, tag) \
    phase47::MemoryProfiler::Instance().TrackAllocation(ptr, size, tag)

#define MEMORY_TRACK_FREE(ptr) \
    phase47::MemoryProfiler::Instance().UntrackAllocation(ptr)

}  // namespace phase47
