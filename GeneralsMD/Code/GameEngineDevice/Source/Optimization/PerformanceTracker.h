#pragma once

#include <vector>
#include <string>
#include <map>
#include <ctime>

namespace GeneralsX::Optimization {

struct OptimizationRecord {
    std::string optimization_name;
    time_t applied_time;
    double frame_time_before_ms;
    double frame_time_after_ms;
    double actual_gain_ms;
    double gain_percentage;
    bool is_active;
};

class PerformanceTracker {
public:
    PerformanceTracker();
    ~PerformanceTracker();
    
    // Tracking
    bool Initialize();
    bool Shutdown();
    
    // Recording operations
    bool RecordOptimization(const Optimization& opt, const ValidationResult& result);
    bool RecordFrameTime(double frame_time_ms);
    bool RecordBottleneck(const Bottleneck& bottleneck);
    
    // Results
    std::vector<OptimizationRecord> GetOptimizationHistory() const;
    OptimizationRecord GetOptimization(const std::string& opt_name) const;
    
    // Statistics
    double GetTotalGainSinceLaunch() const;
    int GetAppliedOptimizationCount() const;
    int GetActiveOptimizationCount() const;
    std::vector<double> GetFrameTimeHistory() const;
    
    // Analysis
    double GetAverageFrameTime() const;
    double GetCurrentFrameTime() const;
    bool HasOptimizationBeenApplied(const std::string& opt_name) const;
    
    // Configuration
    void SetHistoryLimit(int max_records);
    void SetAutoCleanup(bool enabled);

private:
    bool SaveToDatabase();
    bool LoadFromDatabase();
    
    std::vector<OptimizationRecord> optimization_history_;
    std::vector<double> frame_time_history_;
    std::map<std::string, OptimizationRecord> optimization_map_;
    
    int history_limit_;
    bool auto_cleanup_enabled_;
    double total_gain_ms_;
    
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
