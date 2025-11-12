#include "OptimizationScheduler.h"
#include "PerformanceOptimizer.h"
#include <algorithm>
#include <queue>
#include <ctime>
#include <iostream>

namespace GeneralsX::Optimization {

// Concrete implementation of OptimizationScheduler
class OptimizationSchedulerImpl : public OptimizationScheduler {
private:
    std::vector<ScheduledTask> scheduled_tasks;
    SchedulingStatistics statistics;
    double target_frame_rate;
    bool paused;
    int current_frame;
    
public:
    OptimizationSchedulerImpl() 
        : target_frame_rate(60.0), paused(false), current_frame(0) {
        statistics = SchedulingStatistics();
    }
    
    virtual ~OptimizationSchedulerImpl() = default;
    
    virtual void ScheduleOptimization(const Optimization& opt, SchedulingPriority priority) override {
        if (paused) {
            return;
        }
        
        ScheduledTask task;
        task.optimization = opt;
        task.priority = priority;
        
        // Calculate frame to apply based on priority
        int frames_to_wait = 0;
        switch (priority) {
            case SchedulingPriority::CRITICAL:
                frames_to_wait = 1;
                break;
            case SchedulingPriority::HIGH:
                frames_to_wait = 10 + (rand() % 10);
                break;
            case SchedulingPriority::NORMAL:
                frames_to_wait = 30 + (rand() % 30);
                break;
            case SchedulingPriority::LOW:
                frames_to_wait = 60 + (rand() % 60);
                break;
            case SchedulingPriority::DEFERRED:
                frames_to_wait = 120 + (rand() % 120);
                break;
        }
        
        task.frame_to_apply = current_frame + frames_to_wait;
        task.is_batch = false;
        task.dependency_count = 0;
        
        scheduled_tasks.push_back(task);
        statistics.total_scheduled_optimizations++;
    }
    
    virtual void ScheduleBatch(const OptimizationBatch& batch, SchedulingPriority priority) override {
        // Schedule each optimization in batch
        for (const auto& opt : batch.optimizations) {
            ScheduleOptimization(opt, priority);
        }
        
        statistics.batches_created++;
    }
    
    virtual Optimization GetNextOptimization() override {
        Optimization result;
        
        // Find next optimization to apply (by frame number and priority)
        ScheduledTask* next_task = nullptr;
        int best_priority = -1;
        
        for (auto& task : scheduled_tasks) {
            if (task.frame_to_apply <= current_frame && task.dependency_count == 0) {
                int priority_value = static_cast<int>(task.priority);
                if (priority_value > best_priority) {
                    best_priority = priority_value;
                    next_task = &task;
                }
            }
        }
        
        if (next_task) {
            result = next_task->optimization;
            
            // Remove from scheduled list
            auto it = std::find_if(scheduled_tasks.begin(), scheduled_tasks.end(),
                [&](const ScheduledTask& t) { return t.optimization.name == result.name; });
            
            if (it != scheduled_tasks.end()) {
                scheduled_tasks.erase(it);
                statistics.applied_optimizations++;
            }
        }
        
        return result;
    }
    
    virtual OptimizationBatch GetNextBatch() override {
        OptimizationBatch batch;
        batch.atomic = true;
        batch.priority = 0;
        
        // Collect several scheduled optimizations into a batch
        int batch_size = 3 + (rand() % 3);  // 3-5 optimizations per batch
        int collected = 0;
        
        auto it = scheduled_tasks.begin();
        while (it != scheduled_tasks.end() && collected < batch_size) {
            if (it->frame_to_apply <= current_frame && it->dependency_count == 0) {
                batch.optimizations.push_back(it->optimization);
                it = scheduled_tasks.erase(it);
                collected++;
                statistics.applied_optimizations++;
            } else {
                ++it;
            }
        }
        
        if (!batch.optimizations.empty()) {
            statistics.average_batch_size = 
                (statistics.average_batch_size * (statistics.batches_created - 1) + batch.optimizations.size()) 
                / statistics.batches_created;
        }
        
        return batch;
    }
    
    virtual bool UnscheduleOptimization(const std::string& optimization_name) override {
        auto it = std::find_if(scheduled_tasks.begin(), scheduled_tasks.end(),
            [&](const ScheduledTask& t) { return t.optimization.name == optimization_name; });
        
        if (it != scheduled_tasks.end()) {
            scheduled_tasks.erase(it);
            return true;
        }
        
        return false;
    }
    
    virtual bool ReprioritizeOptimization(const std::string& optimization_name, SchedulingPriority new_priority) override {
        for (auto& task : scheduled_tasks) {
            if (task.optimization.name == optimization_name) {
                task.priority = new_priority;
                return true;
            }
        }
        
        return false;
    }
    
    virtual void AdvanceFrame() override {
        if (!paused) {
            current_frame++;
            
            // Update average wait time
            if (!scheduled_tasks.empty()) {
                double total_wait = 0.0;
                for (const auto& task : scheduled_tasks) {
                    total_wait += std::max(0, task.frame_to_apply - current_frame);
                }
                statistics.average_wait_time_frames = total_wait / scheduled_tasks.size();
            }
        }
    }
    
    virtual const std::vector<ScheduledTask>& GetScheduledTasks() const override {
        return scheduled_tasks;
    }
    
    virtual int GetPendingOptimizationCount() const override {
        return scheduled_tasks.size();
    }
    
    virtual const SchedulingStatistics& GetStatistics() const override {
        return statistics;
    }
    
    virtual void SetTargetFrameRate(double fps) override {
        target_frame_rate = fps > 0.0 ? fps : 60.0;
    }
    
    virtual void AddDependency(const std::string& dependent_opt, const std::string& dependency_opt) override {
        // Mark dependent_opt as depending on dependency_opt
        for (auto& task : scheduled_tasks) {
            if (task.optimization.name == dependent_opt) {
                task.dependency_count++;
            }
        }
    }
    
    virtual void ClearSchedule() override {
        scheduled_tasks.clear();
        statistics = SchedulingStatistics();
        current_frame = 0;
    }
    
    virtual bool IsScheduled(const std::string& optimization_name) const override {
        for (const auto& task : scheduled_tasks) {
            if (task.optimization.name == optimization_name) {
                return true;
            }
        }
        
        return false;
    }
    
    virtual int GetApproximateWaitFrames(const std::string& optimization_name) const override {
        for (const auto& task : scheduled_tasks) {
            if (task.optimization.name == optimization_name) {
                return std::max(0, task.frame_to_apply - current_frame);
            }
        }
        
        return -1;  // Not scheduled
    }
    
    virtual void Pause() override {
        paused = true;
    }
    
    virtual void Resume() override {
        paused = false;
    }
    
    virtual bool IsPaused() const override {
        return paused;
    }
    
    virtual void ResetStatistics() override {
        statistics = SchedulingStatistics();
        current_frame = 0;
    }
};

// Static instance for singleton pattern
static std::unique_ptr<OptimizationScheduler> g_scheduler_instance;

OptimizationScheduler* GetOptimizationSchedulerInstance() {
    if (!g_scheduler_instance) {
        g_scheduler_instance = std::make_unique<OptimizationSchedulerImpl>();
    }
    return g_scheduler_instance.get();
}

} // namespace GeneralsX::Optimization
