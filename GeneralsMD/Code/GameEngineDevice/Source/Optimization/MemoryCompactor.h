#pragma once

#include <vector>
#include <memory>
#include <string>
#include <ctime>

namespace GeneralsX::Optimization {

// Memory compaction result
struct CompactionResult {
    std::string heap_name;
    size_t memory_before_mb;
    size_t memory_after_mb;
    size_t freed_memory_mb;
    int fragmentation_before;        // 0-100 percentage
    int fragmentation_after;         // 0-100 percentage
    double compaction_time_ms;
    bool success;
    std::string error_message;
};

// Memory compaction statistics
struct CompactionStatistics {
    int total_compactions;
    size_t total_memory_freed_mb;
    double average_fragmentation_reduction;  // Percentage
    double average_compaction_time_ms;
    int successful_compactions;
    int failed_compactions;
    double total_frame_time_gain_ms;
};

// Memory region for tracking allocation patterns
struct MemoryRegion {
    std::string region_name;
    size_t total_size_mb;
    size_t used_size_mb;
    size_t free_size_mb;
    int fragmentation_percentage;
    int allocation_count;
    int free_chunk_count;
};

// Interface for memory compaction optimization
class MemoryCompactor {
public:
    virtual ~MemoryCompactor() = default;
    
    // Analyze memory fragmentation
    virtual std::vector<MemoryRegion> AnalyzeMemoryFragmentation() = 0;
    
    // Compact a specific memory region
    virtual CompactionResult CompactRegion(const std::string& region_name) = 0;
    
    // Compact all memory regions
    virtual std::vector<CompactionResult> CompactAllRegions() = 0;
    
    // Get current memory statistics
    virtual const CompactionStatistics& GetStatistics() const = 0;
    
    // Get memory regions info
    virtual const std::vector<MemoryRegion>& GetMemoryRegions() const = 0;
    
    // Estimate memory that could be freed
    virtual size_t EstimateFreeableMemory() const = 0;
    
    // Set fragmentation threshold for triggering compaction
    virtual void SetFragmentationThreshold(int percentage) = 0;
    
    // Get current overall fragmentation
    virtual double GetOverallFragmentation() const = 0;
    
    // Enable/disable automatic compaction
    virtual void SetAutoCompactionEnabled(bool enabled) = 0;
    
    // Get interval for automatic compaction (in frames)
    virtual void SetAutoCompactionInterval(int frames) = 0;
    
    // Perform targeted compaction for specific allocation type
    virtual CompactionResult CompactAllocationsByType(const std::string& type_name) = 0;
    
    // Get frame time improvement from last compaction
    virtual double GetLastCompactionFrameTimeGain() const = 0;
    
    // Reset compaction history
    virtual void ResetStatistics() = 0;
    
    // Check if compaction would be beneficial
    virtual bool ShouldCompact() const = 0;
};

} // namespace GeneralsX::Optimization
