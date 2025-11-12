#include "BatchOptimizationApplier.h"
#include "PerformanceOptimizer.h"
#include "OptimizationRollback.h"
#include "SafeOptimizationValidator.h"
#include <algorithm>
#include <numeric>
#include <ctime>
#include <iostream>

namespace GeneralsX::Optimization {

// Concrete implementation of BatchOptimizationApplier
class BatchOptimizationApplierImpl : public BatchOptimizationApplier {
private:
    std::vector<OptimizationApplicationResult> batch_results;
    OptimizationApplicationResult last_result;
    int applied_optimization_count;
    double total_frame_time_gain;
    std::vector<double> application_times;
    int failed_count;
    
public:
    BatchOptimizationApplierImpl() 
        : applied_optimization_count(0), total_frame_time_gain(0.0), failed_count(0) {}
    
    virtual ~BatchOptimizationApplierImpl() = default;
    
    virtual bool ApplyBatch(const OptimizationBatch& batch) override {
        batch_results.clear();
        auto batch_start_time = std::time(nullptr);
        
        // Pre-apply validation for all optimizations in batch
        int failed_pre_validations = 0;
        for (const auto& opt : batch.optimizations) {
            if (!CanApplyOptimization(opt)) {
                failed_pre_validations++;
            }
        }
        
        // If batch is atomic and any failed validation, fail entire batch
        if (batch.atomic && failed_pre_validations > 0) {
            last_result.success = false;
            last_result.error_message = "Pre-validation failed for " + std::to_string(failed_pre_validations) + " optimizations";
            failed_count++;
            return false;
        }
        
        // Apply each optimization in batch
        bool any_failure = false;
        for (const auto& opt : batch.optimizations) {
            if (!ApplySingleOptimization(opt)) {
                any_failure = true;
                batch_results.back().success = false;
                
                if (batch.atomic) {
                    // Rollback all previously applied optimizations
                    for (int i = batch_results.size() - 2; i >= 0; --i) {
                        if (batch_results[i].success) {
                            // Mark for rollback (implementation would call RollbackManager)
                        }
                    }
                    failed_count++;
                    return false;
                }
            } else {
                batch_results.back().success = true;
                applied_optimization_count++;
                total_frame_time_gain += batch_results.back().measured_gain_ms;
            }
        }
        
        auto batch_end_time = std::time(nullptr);
        last_result.success = !any_failure || !batch.atomic;
        return !any_failure || !batch.atomic;
    }
    
    virtual bool ApplySingleOptimization(const Optimization& opt) override {
        OptimizationApplicationResult result;
        result.optimization_name = opt.name;
        result.applied_at = std::time(nullptr);
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Simulate optimization application
        // In real implementation, this would call actual optimization functions
        bool apply_success = true;
        
        if (opt.target == "draw_calls") {
            // Mesh batching simulation
            result.measured_gain_ms = opt.estimated_gain_ms * (0.85 + (rand() % 30) / 100.0);  // 0.85 - 1.15 of estimate
        } else if (opt.target == "memory") {
            // Memory compaction simulation
            result.measured_gain_ms = opt.estimated_gain_ms * (0.80 + (rand() % 40) / 100.0);  // 0.80 - 1.20 of estimate
        } else if (opt.target == "shader_complexity") {
            // Shader optimization simulation
            result.measured_gain_ms = opt.estimated_gain_ms * (0.75 + (rand() % 50) / 100.0);  // 0.75 - 1.25 of estimate
        } else {
            // Default optimization
            result.measured_gain_ms = opt.estimated_gain_ms * (0.70 + (rand() % 60) / 100.0);  // 0.70 - 1.30 of estimate
        }
        
        // Ensure minimum measurable gain
        if (result.measured_gain_ms < 0.01) {
            result.measured_gain_ms = 0.01;
        }
        
        result.error_margin = result.measured_gain_ms * 0.15;  // 15% error margin
        result.success = apply_success;
        result.error_message = apply_success ? "" : "Optimization application failed";
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        application_times.push_back(duration.count());
        
        batch_results.push_back(result);
        last_result = result;
        
        return apply_success;
    }
    
    virtual const OptimizationApplicationResult& GetLastResult() const override {
        return last_result;
    }
    
    virtual const std::vector<OptimizationApplicationResult>& GetBatchResults() const override {
        return batch_results;
    }
    
    virtual bool CanApplyOptimization(const Optimization& opt) const override {
        // Check if optimization is valid
        if (opt.name.empty() || opt.target.empty()) {
            return false;
        }
        
        // Check confidence threshold (must be >= 70%)
        if (opt.confidence_level < 0.70) {
            return false;
        }
        
        // Check if already applied
        if (opt.applied) {
            return false;
        }
        
        return true;
    }
    
    virtual OptimizationBatchStatus GetBatchStatus(const OptimizationBatch& batch) const override {
        return batch.status;
    }
    
    virtual int GetAppliedOptimizationCount() const override {
        return applied_optimization_count;
    }
    
    virtual double GetTotalFrameTimeGain() const override {
        return total_frame_time_gain;
    }
    
    virtual double GetAverageApplicationTime() const override {
        if (application_times.empty()) {
            return 0.0;
        }
        
        double sum = std::accumulate(application_times.begin(), application_times.end(), 0.0);
        return sum / application_times.size();
    }
    
    virtual double GetFailureRate() const override {
        if (applied_optimization_count + failed_count == 0) {
            return 0.0;
        }
        
        return static_cast<double>(failed_count) / (applied_optimization_count + failed_count);
    }
    
    virtual void ResetStatistics() override {
        batch_results.clear();
        applied_optimization_count = 0;
        total_frame_time_gain = 0.0;
        application_times.clear();
        failed_count = 0;
    }
};

// Static instance for singleton pattern
static std::unique_ptr<BatchOptimizationApplier> g_batch_applier_instance;

BatchOptimizationApplier* GetBatchOptimizationApplierInstance() {
    if (!g_batch_applier_instance) {
        g_batch_applier_instance = std::make_unique<BatchOptimizationApplierImpl>();
    }
    return g_batch_applier_instance.get();
}

} // namespace GeneralsX::Optimization
