#include "OptimizationMonitor.h"
#include <algorithm>
#include <numeric>
#include <ctime>
#include <iostream>
#include <cmath>
#include <sstream>

namespace GeneralsX::Optimization {

// Concrete implementation of OptimizationMonitor
class OptimizationMonitorImpl : public OptimizationMonitor {
private:
    std::string current_optimization;
    ImpactMeasurement current_measurement;
    std::map<std::string, std::vector<ImpactMeasurement>> impact_history;
    std::vector<SideEffect> detected_side_effects;
    MonitoringStatistics statistics;
    int measurement_duration;
    std::vector<double> frame_times;
    std::vector<double> gpu_times;
    std::vector<int> draw_call_counts;
    
public:
    OptimizationMonitorImpl() : measurement_duration(60) {
        statistics = MonitoringStatistics();
    }
    
    virtual ~OptimizationMonitorImpl() = default;
    
    virtual void BeginMonitoring(const std::string& optimization_name) override {
        current_optimization = optimization_name;
        current_measurement = ImpactMeasurement();
        current_measurement.optimization_name = optimization_name;
        current_measurement.measured_at = std::time(nullptr);
        
        // Clear frame data
        frame_times.clear();
        gpu_times.clear();
        draw_call_counts.clear();
    }
    
    virtual ImpactMeasurement EndMonitoring() override {
        // Calculate final measurements
        if (!frame_times.empty()) {
            double avg_before = frame_times[0];
            double avg_after = 0.0;
            
            if (frame_times.size() > 1) {
                avg_after = std::accumulate(frame_times.begin() + 1, frame_times.end(), 0.0) 
                           / (frame_times.size() - 1);
            }
            
            current_measurement.frame_time_delta_ms = avg_before - avg_after;
        }
        
        if (!gpu_times.empty()) {
            double avg_before = gpu_times[0];
            double avg_after = std::accumulate(gpu_times.begin(), gpu_times.end(), 0.0) 
                              / gpu_times.size();
            current_measurement.gpu_time_delta_ms = avg_before - avg_after;
        }
        
        if (!draw_call_counts.empty()) {
            int before = draw_call_counts.front();
            int after = draw_call_counts.back();
            current_measurement.draw_call_delta = before - after;
        }
        
        statistics.total_optimizations_monitored++;
        statistics.total_measurements++;
        
        if (impact_history.find(current_optimization) == impact_history.end()) {
            impact_history[current_optimization] = std::vector<ImpactMeasurement>();
        }
        impact_history[current_optimization].push_back(current_measurement);
        
        // Update average metrics
        statistics.average_frame_time_delta_ms = 
            (statistics.average_frame_time_delta_ms * (statistics.total_measurements - 1) + current_measurement.frame_time_delta_ms)
            / statistics.total_measurements;
        
        return current_measurement;
    }
    
    virtual const ImpactMeasurement& GetCurrentMeasurement() const override {
        return current_measurement;
    }
    
    virtual void RecordFrameMetrics(double frame_time_ms, double gpu_time_ms, int draw_calls,
                                    double cpu_util, double gpu_util) override {
        frame_times.push_back(frame_time_ms);
        gpu_times.push_back(gpu_time_ms);
        draw_call_counts.push_back(draw_calls);
        
        current_measurement.cpu_utilization_delta = cpu_util;
        current_measurement.gpu_utilization_delta = gpu_util;
    }
    
    virtual std::vector<SideEffect> DetectSideEffects() override {
        std::vector<SideEffect> effects;
        
        // Check for memory increases
        if (current_measurement.memory_delta_mb > 50.0) {
            SideEffect effect;
            effect.detected_issue = "High memory increase detected";
            effect.affected_system = "Memory";
            effect.severity_level = std::min(1.0, current_measurement.memory_delta_mb / 200.0);
            effect.requires_attention = effect.severity_level > 0.7;
            effect.recommended_action = "Consider memory compaction";
            effects.push_back(effect);
        }
        
        // Check for GPU utilization spikes
        if (current_measurement.gpu_utilization_delta > 20.0) {
            SideEffect effect;
            effect.detected_issue = "GPU utilization spike detected";
            effect.affected_system = "GPU";
            effect.severity_level = std::min(1.0, current_measurement.gpu_utilization_delta / 100.0);
            effect.requires_attention = effect.severity_level > 0.8;
            effect.recommended_action = "Check shader complexity";
            effects.push_back(effect);
        }
        
        // Check for regression in frame time
        if (current_measurement.frame_time_delta_ms < -2.0) {  // Negative = slowdown
            SideEffect effect;
            effect.detected_issue = "Performance regression detected";
            effect.affected_system = "FrameTime";
            effect.severity_level = std::min(1.0, -current_measurement.frame_time_delta_ms / 10.0);
            effect.requires_attention = true;
            effect.recommended_action = "Automatic rollback recommended";
            effects.push_back(effect);
            statistics.automatic_rollbacks_triggered++;
        }
        
        detected_side_effects.insert(detected_side_effects.end(), effects.begin(), effects.end());
        statistics.detected_side_effects += effects.size();
        
        return effects;
    }
    
    virtual const std::vector<SideEffect>& GetAllSideEffects() const override {
        return detected_side_effects;
    }
    
    virtual const std::vector<ImpactMeasurement>& GetImpactHistory(const std::string& optimization_name) const override {
        static std::vector<ImpactMeasurement> empty;
        
        auto it = impact_history.find(optimization_name);
        if (it != impact_history.end()) {
            return it->second;
        }
        
        return empty;
    }
    
    virtual const MonitoringStatistics& GetStatistics() const override {
        return statistics;
    }
    
    virtual double AnalyzeImpactTrend(const std::string& optimization_name) override {
        auto it = impact_history.find(optimization_name);
        if (it == impact_history.end() || it->second.empty()) {
            return 0.0;
        }
        
        const auto& measurements = it->second;
        
        // Calculate trend (linear regression)
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
        int n = measurements.size();
        
        for (int i = 0; i < n; ++i) {
            sum_x += i;
            sum_y += measurements[i].frame_time_delta_ms;
            sum_xy += i * measurements[i].frame_time_delta_ms;
            sum_x2 += i * i;
        }
        
        double denominator = n * sum_x2 - sum_x * sum_x;
        if (std::abs(denominator) < 1e-6) {
            return 0.0;
        }
        
        double slope = (n * sum_xy - sum_x * sum_y) / denominator;
        return slope;  // Negative = getting better, positive = getting worse
    }
    
    virtual void SetMeasurementDuration(int frames) override {
        measurement_duration = frames > 0 ? frames : 60;
    }
    
    virtual bool CheckSideEffectThreshold(double threshold) const override {
        for (const auto& effect : detected_side_effects) {
            if (effect.severity_level > threshold) {
                return true;
            }
        }
        
        return false;
    }
    
    virtual ImpactMeasurement GetAverageImpact() const override {
        ImpactMeasurement avg;
        
        if (impact_history.empty()) {
            return avg;
        }
        
        int total_measurements = 0;
        double total_frame_gain = 0.0;
        double total_memory_delta = 0.0;
        double total_gpu_delta = 0.0;
        int total_draw_call_delta = 0;
        
        for (const auto& pair : impact_history) {
            for (const auto& measurement : pair.second) {
                total_frame_gain += measurement.frame_time_delta_ms;
                total_memory_delta += measurement.memory_delta_mb;
                total_gpu_delta += measurement.gpu_time_delta_ms;
                total_draw_call_delta += measurement.draw_call_delta;
                total_measurements++;
            }
        }
        
        if (total_measurements > 0) {
            avg.frame_time_delta_ms = total_frame_gain / total_measurements;
            avg.memory_delta_mb = total_memory_delta / total_measurements;
            avg.gpu_time_delta_ms = total_gpu_delta / total_measurements;
            avg.draw_call_delta = total_draw_call_delta / total_measurements;
        }
        
        return avg;
    }
    
    virtual double PredictFutureImpact(int frames_ahead) const override {
        if (frame_times.size() < 2) {
            return 0.0;
        }
        
        // Simple linear extrapolation
        double first_frame = frame_times[0];
        double last_frame = frame_times.back();
        int elapsed_frames = frame_times.size() - 1;
        
        if (elapsed_frames == 0) {
            return last_frame;
        }
        
        double frame_rate = (last_frame - first_frame) / elapsed_frames;
        return last_frame + (frame_rate * frames_ahead);
    }
    
    virtual bool IsOptimizationCausingRegression(const std::string& optimization_name) const override {
        auto it = impact_history.find(optimization_name);
        if (it == impact_history.end() || it->second.empty()) {
            return false;
        }
        
        const auto& measurements = it->second;
        
        // Check if recent measurements show degradation
        double avg_gain = 0.0;
        for (const auto& m : measurements) {
            avg_gain += m.frame_time_delta_ms;
        }
        avg_gain /= measurements.size();
        
        return avg_gain < -1.0;  // Negative = slowdown
    }
    
    virtual std::vector<std::string> GetRegressionInducingOptimizations() const override {
        std::vector<std::string> regressions;
        
        for (const auto& pair : impact_history) {
            if (IsOptimizationCausingRegression(pair.first)) {
                regressions.push_back(pair.first);
            }
        }
        
        return regressions;
    }
    
    virtual void ResetMonitoringData() override {
        current_measurement = ImpactMeasurement();
        impact_history.clear();
        detected_side_effects.clear();
        statistics = MonitoringStatistics();
        frame_times.clear();
        gpu_times.clear();
        draw_call_counts.clear();
    }
    
    virtual std::string GenerateMonitoringReport() const override {
        std::stringstream ss;
        ss << "=== Optimization Monitoring Report ===\n";
        ss << "Total Optimizations Monitored: " << statistics.total_optimizations_monitored << "\n";
        ss << "Total Measurements: " << statistics.total_measurements << "\n";
        ss << "Average Frame Time Delta: " << statistics.average_frame_time_delta_ms << "ms\n";
        ss << "Average Memory Delta: " << statistics.average_memory_delta_mb << "MB\n";
        ss << "Average CPU Utilization Improvement: " << statistics.average_cpu_utilization_improvement << "%\n";
        ss << "Average GPU Utilization Improvement: " << statistics.average_gpu_utilization_improvement << "%\n";
        ss << "Detected Side Effects: " << statistics.detected_side_effects << "\n";
        ss << "Automatic Rollbacks Triggered: " << statistics.automatic_rollbacks_triggered << "\n";
        
        return ss.str();
    }
};

// Static instance for singleton pattern
static std::unique_ptr<OptimizationMonitor> g_monitor_instance;

OptimizationMonitor* GetOptimizationMonitorInstance() {
    if (!g_monitor_instance) {
        g_monitor_instance = std::make_unique<OptimizationMonitorImpl>();
    }
    return g_monitor_instance.get();
}

} // namespace GeneralsX::Optimization
