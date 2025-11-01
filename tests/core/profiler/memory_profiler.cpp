#include "memory_profiler.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <limits>

namespace phase47 {

void MemoryProfiler::TrackAllocation(void* ptr, size_t size, const char* tag) {
    if (ptr == nullptr) return;

    Allocation alloc;
    alloc.ptr = ptr;
    alloc.size = size;
    alloc.tag = tag ? std::string(tag) : "unknown";
    alloc.timestamp = allocation_sequence_++;

    allocations_.push_back(alloc);
    total_allocated_ += size;

    if (total_allocated_ > peak_memory_) {
        peak_memory_ = total_allocated_;
    }
}

void MemoryProfiler::UntrackAllocation(void* ptr) {
    if (ptr == nullptr) return;

    auto it = std::find_if(allocations_.begin(), allocations_.end(),
        [ptr](const Allocation& a) { return a.ptr == ptr; });

    if (it != allocations_.end()) {
        total_allocated_ -= it->size;
        allocations_.erase(it);
    }
}

size_t MemoryProfiler::GetTotalAllocated() const {
    return total_allocated_;
}

uint32_t MemoryProfiler::GetAllocationCount() const {
    return static_cast<uint32_t>(allocations_.size());
}

size_t MemoryProfiler::GetVRAMUsage() const {
    return EstimateGPUMemory();
}

size_t MemoryProfiler::EstimateGPUMemory() const {
    // In a real implementation, this would query GPU memory from the graphics backend
    // For now, estimate based on allocations tagged as GPU memory
    size_t gpu_memory = 0;
    for (const auto& alloc : allocations_) {
        if (alloc.tag.find("gpu") != std::string::npos ||
            alloc.tag.find("texture") != std::string::npos ||
            alloc.tag.find("buffer") != std::string::npos) {
            gpu_memory += alloc.size;
        }
    }
    return gpu_memory;
}

void MemoryProfiler::PrintReport() const {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "Memory Profiling Report\n";
    std::cout << "═══════════════════════════════════════════════\n";

    std::map<std::string, size_t> tag_totals;
    for (const auto& alloc : allocations_) {
        tag_totals[alloc.tag] += alloc.size;
    }

    std::cout << "Total Allocated: " << std::fixed << std::setprecision(2)
              << total_allocated_ / (1024.0 * 1024.0) << " MB\n";
    std::cout << "Peak Memory:     " << peak_memory_ / (1024.0 * 1024.0) << " MB\n";
    std::cout << "Allocation Count: " << allocations_.size() << "\n";
    std::cout << "GPU Memory:      " << GetVRAMUsage() / (1024.0 * 1024.0) << " MB\n";

    std::cout << "\nBreakdown by Tag:\n";
    for (const auto& pair : tag_totals) {
        std::cout << "  " << std::left << std::setw(20) << pair.first
                  << ": " << std::fixed << std::setprecision(2)
                  << pair.second / (1024.0 * 1024.0) << " MB\n";
    }

    std::cout << "═══════════════════════════════════════════════\n";
}

void MemoryProfiler::GenerateReport(const std::string& filename) const {
    std::ofstream report(filename);

    report << "Memory Profiling Report\n";
    report << "═══════════════════════════════════════════════\n\n";

    std::map<std::string, size_t> tag_totals;
    for (const auto& alloc : allocations_) {
        tag_totals[alloc.tag] += alloc.size;
    }

    report << "Total Allocated: " << std::fixed << std::setprecision(2)
           << total_allocated_ / (1024.0 * 1024.0) << " MB\n";
    report << "Peak Memory:     " << peak_memory_ / (1024.0 * 1024.0) << " MB\n";
    report << "Allocation Count: " << allocations_.size() << "\n";
    report << "GPU Memory:      " << GetVRAMUsage() / (1024.0 * 1024.0) << " MB\n";

    report << "\nBreakdown by Tag:\n";
    for (const auto& pair : tag_totals) {
        report << "  " << std::left << std::setw(30) << pair.first
               << ": " << std::fixed << std::setprecision(2)
               << pair.second / (1024.0 * 1024.0) << " MB\n";
    }

    report << "\nIndividual Allocations:\n";
    for (const auto& alloc : allocations_) {
        report << "  " << std::hex << alloc.ptr << std::dec
               << " - " << alloc.size << " bytes (" << alloc.tag << ")\n";
    }

    report.close();
}

std::vector<MemoryProfiler::AllocationInfo> MemoryProfiler::GetAllocationsByTag(const char* tag) const {
    std::vector<AllocationInfo> result;
    std::string tag_str = tag ? std::string(tag) : "";

    for (const auto& alloc : allocations_) {
        if (alloc.tag == tag_str) {
            result.push_back({alloc.ptr, alloc.size, alloc.tag});
        }
    }

    return result;
}

void MemoryProfiler::Clear() {
    allocations_.clear();
    total_allocated_ = 0;
    peak_memory_ = 0;
    allocation_sequence_ = 0;
}

MemoryProfiler::MemoryStats MemoryProfiler::GetStats() const {
    MemoryStats stats;
    stats.total_allocated = total_allocated_;
    stats.peak_memory = peak_memory_;
    stats.allocation_count = static_cast<uint32_t>(allocations_.size());
    stats.gpu_memory = GetVRAMUsage();
    
    // Calculate fragmentation estimate
    if (allocations_.size() > 0) {
        size_t min_size = allocations_[0].size;
        size_t max_size = allocations_[0].size;
        for (const auto& alloc : allocations_) {
            min_size = std::min(min_size, alloc.size);
            max_size = std::max(max_size, alloc.size);
        }
        stats.fragmentation = static_cast<double>(max_size) / (min_size + 1);
    } else {
        stats.fragmentation = 0.0;
    }

    return stats;
}

}  // namespace phase47
