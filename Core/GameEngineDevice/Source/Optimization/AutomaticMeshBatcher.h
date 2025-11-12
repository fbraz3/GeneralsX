#pragma once

#include <vector>
#include <memory>
#include <string>
#include <ctime>

namespace GeneralsX::Optimization {

// Mesh batch information
struct MeshBatch {
    std::string batch_name;
    int mesh_count;                   // Number of meshes in batch
    int original_draw_calls;          // Draw calls before batching
    int optimized_draw_calls;         // Draw calls after batching
    double memory_overhead_mb;        // Memory added by batching
    double frame_time_gain_ms;        // Measured frame time improvement
    bool is_valid;                    // Is batch valid/usable?
    std::time_t created_at;
    
    MeshBatch() : mesh_count(0), original_draw_calls(0), 
                  optimized_draw_calls(0), memory_overhead_mb(0.0),
                  frame_time_gain_ms(0.0), is_valid(false), created_at(0) {}
};

// Batching statistics
struct BatchingStatistics {
    int total_meshes_batched;
    int total_batches_created;
    int total_draw_calls_before;
    int total_draw_calls_after;
    double total_frame_time_gain_ms;
    double average_batch_size;
    double memory_overhead_total_mb;
};

// Interface for automatic mesh batching optimization
class AutomaticMeshBatcher {
public:
    virtual ~AutomaticMeshBatcher() = default;
    
    // Analyze meshes and create batches
    virtual int AnalyzeAndBatchMeshes() = 0;
    
    // Get list of created batches
    virtual const std::vector<MeshBatch>& GetBatches() const = 0;
    
    // Apply a specific mesh batch
    virtual bool ApplyBatch(const MeshBatch& batch) = 0;
    
    // Apply all batches
    virtual int ApplyAllBatches() = 0;
    
    // Revert a batch (restore original meshes)
    virtual bool RevertBatch(const MeshBatch& batch) = 0;
    
    // Get mesh count reduction estimate
    virtual int EstimateMeshReduction() const = 0;
    
    // Get draw call reduction estimate
    virtual int EstimateDrawCallReduction() const = 0;
    
    // Get frame time improvement estimate
    virtual double EstimateFrameTimeGain() const = 0;
    
    // Check if mesh is suitable for batching
    virtual bool IsMeshBatchable(const std::string& mesh_name) const = 0;
    
    // Set minimum batch size threshold
    virtual void SetMinimumBatchSize(int size) = 0;
    
    // Set maximum batch size limit
    virtual void SetMaximumBatchSize(int size) = 0;
    
    // Get batching statistics
    virtual const BatchingStatistics& GetStatistics() const = 0;
    
    // Get number of applied batches
    virtual int GetAppliedBatchCount() const = 0;
    
    // Clear all batches and revert to original
    virtual void ResetAllBatches() = 0;
    
    // Optimize batches based on target draw calls
    virtual void OptimizeForTargetDrawCalls(int target_draw_calls) = 0;
};

} // namespace GeneralsX::Optimization
