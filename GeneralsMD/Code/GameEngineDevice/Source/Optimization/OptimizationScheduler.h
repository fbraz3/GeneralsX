#pragma once

#include <vector>
#include <memory>
#include <string>
#include <queue>

namespace GeneralsX::Optimization {

// Forward declarations
struct Optimization;
struct OptimizationBatch;

// Scheduling priority levels
enum class SchedulingPriority {
    CRITICAL,    // Must apply immediately
    HIGH,        // Apply in next few frames
    NORMAL,      // Apply when convenient
    LOW,         // Apply if time permits
    DEFERRED     // Apply only if no other options
};

// Scheduler task information
struct ScheduledTask {
    Optimization optimization;
    SchedulingPriority priority;
    int frame_to_apply;          // Frame number when to apply
    bool is_batch;               // Is this a batch task?
    int dependency_count;        // Number of unsatisfied dependencies
};

// Scheduling statistics
struct SchedulingStatistics {
    int total_scheduled_optimizations;
    int applied_optimizations;
    int pending_optimizations;
    int failed_optimizations;
    double average_wait_time_frames;
    int batches_created;
    double average_batch_size;
};

// Interface for optimization scheduling
class OptimizationScheduler {
public:
    virtual ~OptimizationScheduler() = default;
    
    // Schedule an optimization for future application
    virtual void ScheduleOptimization(const Optimization& opt, SchedulingPriority priority = SchedulingPriority::NORMAL) = 0;
    
    // Schedule a batch of optimizations
    virtual void ScheduleBatch(const OptimizationBatch& batch, SchedulingPriority priority = SchedulingPriority::NORMAL) = 0;
    
    // Get next optimization to apply
    virtual Optimization GetNextOptimization() = 0;
    
    // Get next batch to apply
    virtual OptimizationBatch GetNextBatch() = 0;
    
    // Remove a scheduled optimization
    virtual bool UnscheduleOptimization(const std::string& optimization_name) = 0;
    
    // Reprioritize a scheduled optimization
    virtual bool ReprioritizeOptimization(const std::string& optimization_name, SchedulingPriority new_priority) = 0;
    
    // Advance scheduler by one frame (check if any optimizations should be applied)
    virtual void AdvanceFrame() = 0;
    
    // Get current scheduled tasks
    virtual const std::vector<ScheduledTask>& GetScheduledTasks() const = 0;
    
    // Get pending optimizations count
    virtual int GetPendingOptimizationCount() const = 0;
    
    // Get scheduling statistics
    virtual const SchedulingStatistics& GetStatistics() const = 0;
    
    // Set frame rate assumption (for scheduling calculations)
    virtual void SetTargetFrameRate(double fps) = 0;
    
    // Add dependency between optimizations
    virtual void AddDependency(const std::string& dependent_opt, const std::string& dependency_opt) = 0;
    
    // Clear all scheduled optimizations
    virtual void ClearSchedule() = 0;
    
    // Check if optimization is already scheduled
    virtual bool IsScheduled(const std::string& optimization_name) const = 0;
    
    // Get approximate wait time until optimization is applied
    virtual int GetApproximateWaitFrames(const std::string& optimization_name) const = 0;
    
    // Pause scheduling
    virtual void Pause() = 0;
    
    // Resume scheduling
    virtual void Resume() = 0;
    
    // Check if scheduler is paused
    virtual bool IsPaused() const = 0;
    
    // Reset statistics
    virtual void ResetStatistics() = 0;
};

} // namespace GeneralsX::Optimization
