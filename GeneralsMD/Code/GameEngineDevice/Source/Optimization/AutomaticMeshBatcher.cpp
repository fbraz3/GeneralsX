#include "AutomaticMeshBatcher.h"
#include <algorithm>
#include <numeric>
#include <ctime>
#include <iostream>

namespace GeneralsX::Optimization {

// Concrete implementation of AutomaticMeshBatcher
class AutomaticMeshBatcherImpl : public AutomaticMeshBatcher {
private:
    std::vector<MeshBatch> batches;
    BatchingStatistics statistics;
    int minimum_batch_size;
    int maximum_batch_size;
    int applied_batch_count;
    
public:
    AutomaticMeshBatcherImpl() 
        : minimum_batch_size(2), maximum_batch_size(512), applied_batch_count(0) {
        statistics = BatchingStatistics();
    }
    
    virtual ~AutomaticMeshBatcherImpl() = default;
    
    virtual int AnalyzeAndBatchMeshes() override {
        batches.clear();
        statistics = BatchingStatistics();
        
        // Simulate analyzing meshes and creating batches
        // In real implementation, this would scan actual mesh data
        
        // Create batches based on simulated mesh data
        int mesh_count = 50 + (rand() % 100);  // 50-150 meshes
        int batch_size = minimum_batch_size;
        
        statistics.total_meshes_batched = 0;
        int original_draw_calls_total = 0;
        
        for (int i = 0; i < mesh_count; i += batch_size) {
            MeshBatch batch;
            batch.batch_name = "MeshBatch_" + std::to_string(batches.size());
            batch.mesh_count = std::min(batch_size, mesh_count - i);
            batch.original_draw_calls = batch.mesh_count;  // 1 draw call per mesh before
            batch.optimized_draw_calls = 1;  // 1 draw call after batching
            batch.memory_overhead_mb = 0.5 + (batch.mesh_count * 0.05);  // ~0.05 MB per mesh
            batch.frame_time_gain_ms = (batch.mesh_count - 1) * 0.5;  // ~0.5ms per batched mesh
            batch.is_valid = true;
            batch.created_at = std::time(nullptr);
            
            batches.push_back(batch);
            
            statistics.total_meshes_batched += batch.mesh_count;
            statistics.total_batches_created++;
            original_draw_calls_total += batch.original_draw_calls;
        }
        
        // Calculate statistics
        statistics.total_draw_calls_before = original_draw_calls_total;
        statistics.total_draw_calls_after = statistics.total_batches_created;
        statistics.total_frame_time_gain_ms = 0.0;
        
        for (const auto& batch : batches) {
            statistics.total_frame_time_gain_ms += batch.frame_time_gain_ms;
            statistics.memory_overhead_total_mb += batch.memory_overhead_mb;
        }
        
        if (!batches.empty()) {
            statistics.average_batch_size = static_cast<double>(statistics.total_meshes_batched) / batches.size();
        }
        
        return batches.size();
    }
    
    virtual const std::vector<MeshBatch>& GetBatches() const override {
        return batches;
    }
    
    virtual bool ApplyBatch(const MeshBatch& batch) override {
        // Find batch in our list and mark as applied
        for (auto& b : batches) {
            if (b.batch_name == batch.batch_name) {
                b.is_valid = true;
                applied_batch_count++;
                statistics.total_frame_time_gain_ms += batch.frame_time_gain_ms;
                return true;
            }
        }
        
        return false;
    }
    
    virtual int ApplyAllBatches() override {
        int applied = 0;
        
        for (auto& batch : batches) {
            if (ApplyBatch(batch)) {
                applied++;
            }
        }
        
        return applied;
    }
    
    virtual bool RevertBatch(const MeshBatch& batch) override {
        // Find batch and revert
        for (auto& b : batches) {
            if (b.batch_name == batch.batch_name) {
                b.is_valid = false;
                applied_batch_count = std::max(0, applied_batch_count - 1);
                statistics.total_frame_time_gain_ms -= batch.frame_time_gain_ms;
                return true;
            }
        }
        
        return false;
    }
    
    virtual int EstimateMeshReduction() const override {
        // Estimate how many draw calls could be saved
        if (batches.empty()) {
            return 0;
        }
        
        int reduction = statistics.total_meshes_batched - statistics.total_batches_created;
        return reduction;
    }
    
    virtual int EstimateDrawCallReduction() const override {
        // Estimate draw call reduction if all batches applied
        int reduction = statistics.total_draw_calls_before - statistics.total_draw_calls_after;
        return reduction;
    }
    
    virtual double EstimateFrameTimeGain() const override {
        return statistics.total_frame_time_gain_ms;
    }
    
    virtual bool IsMeshBatchable(const std::string& mesh_name) const override {
        // Check if mesh is suitable for batching
        // In real implementation, would check mesh properties
        
        // Most meshes are batchable unless very large
        return !mesh_name.empty();
    }
    
    virtual void SetMinimumBatchSize(int size) override {
        minimum_batch_size = size > 0 ? size : 1;
    }
    
    virtual void SetMaximumBatchSize(int size) override {
        maximum_batch_size = size > minimum_batch_size ? size : minimum_batch_size + 1;
    }
    
    virtual const BatchingStatistics& GetStatistics() const override {
        return statistics;
    }
    
    virtual int GetAppliedBatchCount() const override {
        return applied_batch_count;
    }
    
    virtual void ResetAllBatches() override {
        for (auto& batch : batches) {
            batch.is_valid = false;
        }
        
        applied_batch_count = 0;
        statistics.total_frame_time_gain_ms = 0.0;
    }
    
    virtual void OptimizeForTargetDrawCalls(int target_draw_calls) override {
        // Adjust batching strategy to meet target draw call count
        if (target_draw_calls <= 0 || batches.empty()) {
            return;
        }
        
        // Calculate how aggressively to batch
        int current_draw_calls = statistics.total_draw_calls_before;
        if (current_draw_calls <= target_draw_calls) {
            // Already meets target
            return;
        }
        
        // Increase batch sizes to reduce draw calls
        double reduction_needed = static_cast<double>(current_draw_calls - target_draw_calls) 
                                / current_draw_calls;
        
        for (auto& batch : batches) {
            int additional_meshes = static_cast<int>(batch.mesh_count * reduction_needed);
            batch.mesh_count = std::min(batch.mesh_count + additional_meshes, maximum_batch_size);
            batch.frame_time_gain_ms = (batch.mesh_count - 1) * 0.5;
        }
        
        // Recalculate statistics
        statistics.total_draw_calls_after = std::max(1, 
            statistics.total_draw_calls_before - static_cast<int>(reduction_needed * current_draw_calls));
    }
};

// Static instance for singleton pattern
static std::unique_ptr<AutomaticMeshBatcher> g_mesh_batcher_instance;

AutomaticMeshBatcher* GetAutomaticMeshBatcherInstance() {
    if (!g_mesh_batcher_instance) {
        g_mesh_batcher_instance = std::make_unique<AutomaticMeshBatcherImpl>();
    }
    return g_mesh_batcher_instance.get();
}

} // namespace GeneralsX::Optimization
