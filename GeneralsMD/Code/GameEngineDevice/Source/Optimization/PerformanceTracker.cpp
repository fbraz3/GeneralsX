#include "PerformanceTracker.h"
#include <algorithm>
#include <iostream>
#include <numeric>

namespace GeneralsX::Optimization {

PerformanceTracker::PerformanceTracker()
    : history_limit_(1000),
      auto_cleanup_enabled_(true),
      total_gain_ms_(0.0),
      initialized_(false) {
}

PerformanceTracker::~PerformanceTracker() {
    if (initialized_) {
        Shutdown();
    }
}

bool PerformanceTracker::Initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    std::cout << "PerformanceTracker initialized" << std::endl;
    return true;
}

bool PerformanceTracker::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    optimization_history_.clear();
    frame_time_history_.clear();
    optimization_map_.clear();
    initialized_ = false;
    return true;
}

bool PerformanceTracker::RecordOptimization(const Optimization& opt,
                                           const ValidationResult& result) {
    if (!initialized_) {
        return false;
    }
    
    OptimizationRecord record;
    record.optimization_name = opt.name;
    record.applied_time = time(nullptr);
    record.frame_time_before_ms = result.before_time_ms;
    record.frame_time_after_ms = result.after_time_ms;
    record.actual_gain_ms = result.actual_gain_ms;
    record.gain_percentage = result.gain_percentage;
    record.is_active = true;
    
    optimization_history_.push_back(record);
    optimization_map_[opt.name] = record;
    
    total_gain_ms_ += result.actual_gain_ms;
    
    // Apply cleanup if needed
    if (auto_cleanup_enabled_ && optimization_history_.size() > static_cast<size_t>(history_limit_)) {
        optimization_history_.erase(optimization_history_.begin());
    }
    
    return true;
}

bool PerformanceTracker::RecordFrameTime(double frame_time_ms) {
    if (!initialized_) {
        return false;
    }
    
    frame_time_history_.push_back(frame_time_ms);
    
    if (auto_cleanup_enabled_ && frame_time_history_.size() > static_cast<size_t>(history_limit_)) {
        frame_time_history_.erase(frame_time_history_.begin());
    }
    
    return true;
}

bool PerformanceTracker::RecordBottleneck(const Bottleneck& bottleneck) {
    if (!initialized_) {
        return false;
    }
    
    // Bottleneck recording for future analysis
    return true;
}

std::vector<OptimizationRecord> PerformanceTracker::GetOptimizationHistory() const {
    return optimization_history_;
}

OptimizationRecord PerformanceTracker::GetOptimization(const std::string& opt_name) const {
    auto it = optimization_map_.find(opt_name);
    if (it != optimization_map_.end()) {
        return it->second;
    }
    return OptimizationRecord();
}

double PerformanceTracker::GetTotalGainSinceLaunch() const {
    return total_gain_ms_;
}

int PerformanceTracker::GetAppliedOptimizationCount() const {
    return optimization_history_.size();
}

int PerformanceTracker::GetActiveOptimizationCount() const {
    int count = 0;
    for (const auto& record : optimization_history_) {
        if (record.is_active) {
            count++;
        }
    }
    return count;
}

std::vector<double> PerformanceTracker::GetFrameTimeHistory() const {
    return frame_time_history_;
}

double PerformanceTracker::GetAverageFrameTime() const {
    if (frame_time_history_.empty()) return 0.0;
    
    double sum = 0.0;
    for (double time : frame_time_history_) {
        sum += time;
    }
    return sum / frame_time_history_.size();
}

double PerformanceTracker::GetCurrentFrameTime() const {
    if (frame_time_history_.empty()) return 0.0;
    return frame_time_history_.back();
}

bool PerformanceTracker::HasOptimizationBeenApplied(const std::string& opt_name) const {
    return optimization_map_.find(opt_name) != optimization_map_.end();
}

void PerformanceTracker::SetHistoryLimit(int max_records) {
    history_limit_ = max_records;
}

void PerformanceTracker::SetAutoCleanup(bool enabled) {
    auto_cleanup_enabled_ = enabled;
}

bool PerformanceTracker::SaveToDatabase() {
    // Mock database save
    return true;
}

bool PerformanceTracker::LoadFromDatabase() {
    // Mock database load
    return true;
}

}  // namespace GeneralsX::Optimization
