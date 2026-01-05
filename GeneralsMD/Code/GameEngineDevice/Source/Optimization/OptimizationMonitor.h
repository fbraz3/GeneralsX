#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <ctime>

namespace GeneralsX::Optimization {

// Real-time impact measurement
struct ImpactMeasurement {
    std::string optimization_name;
    double frame_time_delta_ms;      // Change in frame time
    double memory_delta_mb;          // Change in memory usage
    double gpu_time_delta_ms;        // Change in GPU time
    int draw_call_delta;             // Change in draw call count
    double cpu_utilization_delta;    // Change in CPU utilization
    double gpu_utilization_delta;    // Change in GPU utilization
    std::time_t measured_at;
};

// Side effect detection
struct SideEffect {
    std::string detected_issue;
    std::string affected_system;
    double severity_level;           // 0.0 - 1.0
    bool requires_attention;
    std::string recommended_action;
};

// Optimization monitoring statistics
struct MonitoringStatistics {
    int total_optimizations_monitored;
    int total_measurements;
    double average_frame_time_delta_ms;
    double average_memory_delta_mb;
    double average_cpu_utilization_improvement;
    double average_gpu_utilization_improvement;
    int detected_side_effects;
    int automatic_rollbacks_triggered;
};

// Interface for real-time optimization impact monitoring
class OptimizationMonitor {
public:
    virtual ~OptimizationMonitor() = default;
    
    // Begin monitoring impact of an optimization
    virtual void BeginMonitoring(const std::string& optimization_name) = 0;
    
    // End monitoring and get impact measurement
    virtual ImpactMeasurement EndMonitoring() = 0;
    
    // Get current impact measurement (ongoing monitoring)
    virtual const ImpactMeasurement& GetCurrentMeasurement() const = 0;
    
    // Collect impact data for frame
    virtual void RecordFrameMetrics(double frame_time_ms, double gpu_time_ms, int draw_calls, 
                                    double cpu_util, double gpu_util) = 0;
    
    // Check for side effects
    virtual std::vector<SideEffect> DetectSideEffects() = 0;
    
    // Get all detected side effects
    virtual const std::vector<SideEffect>& GetAllSideEffects() const = 0;
    
    // Get impact history for an optimization
    virtual const std::vector<ImpactMeasurement>& GetImpactHistory(const std::string& optimization_name) const = 0;
    
    // Get monitoring statistics
    virtual const MonitoringStatistics& GetStatistics() const = 0;
    
    // Analyze trend of optimization impact over time
    virtual double AnalyzeImpactTrend(const std::string& optimization_name) = 0;
    
    // Set measurement duration (number of frames to measure)
    virtual void SetMeasurementDuration(int frames) = 0;
    
    // Alert if side effect threshold crossed
    virtual bool CheckSideEffectThreshold(double threshold) const = 0;
    
    // Get average impact across all optimizations
    virtual ImpactMeasurement GetAverageImpact() const = 0;
    
    // Predict impact for next N frames
    virtual double PredictFutureImpact(int frames_ahead) const = 0;
    
    // Check if optimization is causing regression
    virtual bool IsOptimizationCausingRegression(const std::string& optimization_name) const = 0;
    
    // Get list of optimizations causing regressions
    virtual std::vector<std::string> GetRegressionInducingOptimizations() const = 0;
    
    // Clear monitoring data
    virtual void ResetMonitoringData() = 0;
    
    // Export monitoring report to string
    virtual std::string GenerateMonitoringReport() const = 0;
};

} // namespace GeneralsX::Optimization
