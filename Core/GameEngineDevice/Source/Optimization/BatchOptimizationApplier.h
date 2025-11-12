#pragma once

#include <vector>
#include <memory>
#include <string>
#include <ctime>

namespace GeneralsX::Optimization {

// Forward declarations
struct Optimization;

// Batch status enumeration
enum class OptimizationBatchStatus {
    PENDING,      // Waiting to be applied
    APPLYING,     // Currently being applied
    APPLIED,      // Successfully applied
    FAILED,       // Application failed
    ROLLED_BACK   // Rolled back after failure
};

// Represents an atomic batch of optimizations
struct OptimizationBatch {
    // Core data
    std::vector<Optimization> optimizations;
    bool atomic;                            // All succeed or all rollback
    int priority;                           // Higher priority = applied first
    OptimizationBatchStatus status;         // Current batch status
    std::time_t created_at;                 // When batch was created
    std::time_t applied_at;                 // When batch was applied (0 if not)
    
    // Metrics
    double measured_total_gain_ms;          // Actual measured frame time improvement
    double error_margin_ms;                 // Validation error margin
    bool validation_passed;                 // Did post-apply validation pass?
    
    // Diagnostic info
    std::string failure_reason;             // Why did application fail (if failed)
    std::vector<std::string> warnings;      // Non-critical issues during application
    
    OptimizationBatch() 
        : atomic(true), priority(0), status(OptimizationBatchStatus::PENDING),
          created_at(std::time(nullptr)), applied_at(0), measured_total_gain_ms(0.0),
          error_margin_ms(0.0), validation_passed(false) {}
};

// Result of applying a single optimization
struct OptimizationApplicationResult {
    std::string optimization_name;
    bool success;
    double measured_gain_ms;
    double error_margin;
    std::string error_message;
    std::time_t applied_at;
};

// Interface for batch optimization application
class BatchOptimizationApplier {
public:
    virtual ~BatchOptimizationApplier() = default;
    
    // Apply a batch of optimizations atomically
    // Returns success status; use GetLastResult() for details
    virtual bool ApplyBatch(const OptimizationBatch& batch) = 0;
    
    // Apply a single optimization
    virtual bool ApplySingleOptimization(const Optimization& opt) = 0;
    
    // Get result of last operation
    virtual const OptimizationApplicationResult& GetLastResult() const = 0;
    
    // Get all results from last batch
    virtual const std::vector<OptimizationApplicationResult>& GetBatchResults() const = 0;
    
    // Check if optimization is applicable (pre-apply check)
    virtual bool CanApplyOptimization(const Optimization& opt) const = 0;
    
    // Get batch status
    virtual OptimizationBatchStatus GetBatchStatus(const OptimizationBatch& batch) const = 0;
    
    // Get number of applied optimizations in session
    virtual int GetAppliedOptimizationCount() const = 0;
    
    // Get total frame time gain from all applied optimizations
    virtual double GetTotalFrameTimeGain() const = 0;
    
    // Get average application time per optimization
    virtual double GetAverageApplicationTime() const = 0;
    
    // Get application failure rate (0.0 - 1.0)
    virtual double GetFailureRate() const = 0;
    
    // Clear application history
    virtual void ResetStatistics() = 0;
};

} // namespace GeneralsX::Optimization
