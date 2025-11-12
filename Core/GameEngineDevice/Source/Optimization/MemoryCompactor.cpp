#include "MemoryCompactor.h"
#include <algorithm>
#include <numeric>
#include <ctime>
#include <iostream>

namespace GeneralsX::Optimization {

// Concrete implementation of MemoryCompactor
class MemoryCompactorImpl : public MemoryCompactor {
private:
    std::vector<MemoryRegion> memory_regions;
    CompactionStatistics statistics;
    int fragmentation_threshold;
    bool auto_compaction_enabled;
    int auto_compaction_interval;
    int current_frame_count;
    double last_compaction_frame_gain;
    
public:
    MemoryCompactorImpl() 
        : fragmentation_threshold(50), auto_compaction_enabled(false),
          auto_compaction_interval(300), current_frame_count(0),
          last_compaction_frame_gain(0.0) {
        statistics = CompactionStatistics();
        InitializeMemoryRegions();
    }
    
    virtual ~MemoryCompactorImpl() = default;
    
    void InitializeMemoryRegions() {
        // Simulate game memory regions
        MemoryRegion r1;
        r1.region_name = "GraphicsMemory";
        r1.total_size_mb = 512;
        r1.used_size_mb = 384;
        r1.free_size_mb = 128;
        r1.fragmentation_percentage = 45;
        r1.allocation_count = 1024;
        r1.free_chunk_count = 256;
        memory_regions.push_back(r1);
        
        MemoryRegion r2;
        r2.region_name = "GameObjectMemory";
        r2.total_size_mb = 256;
        r2.used_size_mb = 192;
        r2.free_size_mb = 64;
        r2.fragmentation_percentage = 35;
        r2.allocation_count = 512;
        r2.free_chunk_count = 128;
        memory_regions.push_back(r2);
        
        MemoryRegion r3;
        r3.region_name = "AIMemory";
        r3.total_size_mb = 128;
        r3.used_size_mb = 96;
        r3.free_size_mb = 32;
        r3.fragmentation_percentage = 55;
        r3.allocation_count = 256;
        r3.free_chunk_count = 64;
        memory_regions.push_back(r3);
    }
    
    virtual std::vector<MemoryRegion> AnalyzeMemoryFragmentation() override {
        // Analyze fragmentation in each region
        for (auto& region : memory_regions) {
            // Simulate analysis (in real implementation, would scan actual heap)
            region.fragmentation_percentage = 30 + (rand() % 50);  // 30-80% fragmentation
        }
        
        return memory_regions;
    }
    
    virtual CompactionResult CompactRegion(const std::string& region_name) override {
        CompactionResult result;
        result.heap_name = region_name;
        result.success = false;
        
        // Find region
        for (auto& region : memory_regions) {
            if (region.region_name == region_name) {
                result.memory_before_mb = region.total_size_mb;
                result.fragmentation_before = region.fragmentation_percentage;
                
                // Simulate compaction
                auto start_time = std::chrono::high_resolution_clock::now();
                
                // Calculate freed memory (based on fragmentation)
                double fragmentation_ratio = region.fragmentation_percentage / 100.0;
                size_t freed_memory = static_cast<size_t>(region.free_size_mb * fragmentation_ratio * 0.7);
                
                result.freed_memory_mb = freed_memory;
                result.memory_after_mb = result.memory_before_mb - freed_memory;
                
                // Reduce fragmentation after compaction
                region.fragmentation_percentage = std::max(5, 
                    region.fragmentation_percentage - (rand() % 30 + 10));  // 10-40% reduction
                result.fragmentation_after = region.fragmentation_percentage;
                
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                result.compaction_time_ms = duration.count();
                
                // Frame time improvement from freed memory
                double frame_time_gain = freed_memory * 0.5;  // ~0.5ms per 100MB freed
                
                result.success = true;
                
                statistics.total_compactions++;
                statistics.total_memory_freed_mb += freed_memory;
                statistics.average_compaction_time_ms = 
                    (statistics.average_compaction_time_ms * (statistics.total_compactions - 1) + result.compaction_time_ms) 
                    / statistics.total_compactions;
                statistics.successful_compactions++;
                statistics.total_frame_time_gain_ms += frame_time_gain;
                last_compaction_frame_gain = frame_time_gain;
                
                break;
            }
        }
        
        if (!result.success) {
            result.error_message = "Region not found: " + region_name;
            statistics.failed_compactions++;
        }
        
        return result;
    }
    
    virtual std::vector<CompactionResult> CompactAllRegions() override {
        std::vector<CompactionResult> results;
        
        for (const auto& region : memory_regions) {
            results.push_back(CompactRegion(region.region_name));
        }
        
        return results;
    }
    
    virtual const CompactionStatistics& GetStatistics() const override {
        return statistics;
    }
    
    virtual const std::vector<MemoryRegion>& GetMemoryRegions() const override {
        return memory_regions;
    }
    
    virtual size_t EstimateFreeableMemory() const override {
        size_t total = 0;
        
        for (const auto& region : memory_regions) {
            // Estimate based on fragmentation
            double fragmentation_ratio = region.fragmentation_percentage / 100.0;
            total += static_cast<size_t>(region.free_size_mb * fragmentation_ratio * 0.7);
        }
        
        return total;
    }
    
    virtual void SetFragmentationThreshold(int percentage) override {
        fragmentation_threshold = std::max(0, std::min(100, percentage));
    }
    
    virtual double GetOverallFragmentation() const override {
        if (memory_regions.empty()) {
            return 0.0;
        }
        
        double total_fragmentation = 0.0;
        for (const auto& region : memory_regions) {
            total_fragmentation += region.fragmentation_percentage;
        }
        
        return total_fragmentation / memory_regions.size();
    }
    
    virtual void SetAutoCompactionEnabled(bool enabled) override {
        auto_compaction_enabled = enabled;
    }
    
    virtual void SetAutoCompactionInterval(int frames) override {
        auto_compaction_interval = frames > 0 ? frames : 300;
    }
    
    virtual CompactionResult CompactAllocationsByType(const std::string& type_name) override {
        CompactionResult result;
        result.heap_name = type_name;
        
        // Compact specific allocation type
        // In real implementation, would target specific allocation patterns
        
        result.freed_memory_mb = 10 + (rand() % 50);  // 10-60 MB
        result.memory_before_mb = 200;
        result.memory_after_mb = result.memory_before_mb - result.freed_memory_mb;
        result.compaction_time_ms = 2.5 + (rand() % 5);  // 2.5-7.5ms
        result.fragmentation_before = 40;
        result.fragmentation_after = 20;
        result.success = true;
        
        return result;
    }
    
    virtual double GetLastCompactionFrameTimeGain() const override {
        return last_compaction_frame_gain;
    }
    
    virtual void ResetStatistics() override {
        statistics = CompactionStatistics();
        last_compaction_frame_gain = 0.0;
        current_frame_count = 0;
    }
    
    virtual bool ShouldCompact() const override {
        // Check if compaction would be beneficial
        double fragmentation = GetOverallFragmentation();
        return fragmentation >= fragmentation_threshold;
    }
};

// Static instance for singleton pattern
static std::unique_ptr<MemoryCompactor> g_memory_compactor_instance;

MemoryCompactor* GetMemoryCompactorInstance() {
    if (!g_memory_compactor_instance) {
        g_memory_compactor_instance = std::make_unique<MemoryCompactorImpl>();
    }
    return g_memory_compactor_instance.get();
}

} // namespace GeneralsX::Optimization
