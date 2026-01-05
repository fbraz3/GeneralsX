#pragma once

#include <vector>
#include <string>

namespace GeneralsX::Optimization {

struct RegressionAlert {
    std::string marker_name;
    double baseline_time_ms;
    double current_time_ms;
    double degradation_ms;
    double degradation_percentage;
    int frames_detected;
    bool critical;
};

class RegressionDetector {
public:
    RegressionDetector();
    ~RegressionDetector();
    
    // Engine
    bool Initialize();
    bool Shutdown();
    
    // Detection operations
    bool UpdateBaseline(const PerformanceMetrics& metrics);
    bool CheckForRegressions(const PerformanceMetrics& current_metrics);
    bool MonitorBottleneck(const std::string& marker_name, double current_time_ms);
    
    // Results
    std::vector<RegressionAlert> GetRegressions() const;
    bool HasRegression(const std::string& marker_name) const;
    int GetRegressionCount() const;
    int GetCriticalRegressionCount() const;
    
    // Configuration
    void SetRegressionThreshold(double percentage);
    void SetCriticalThreshold(double percentage);
    void SetSampleHistory(int count);
    void SetAnomalyDetectionEnabled(bool enabled);

private:
    bool DetectAnomalies(const std::vector<double>& historical_data,
                         double current_value);
    bool CalculateStats(const std::vector<double>& data,
                        double& mean, double& stddev);
    
    PerformanceMetrics baseline_metrics_;
    std::vector<RegressionAlert> current_regressions_;
    std::vector<PerformanceMetrics> historical_metrics_;
    
    double regression_threshold_percentage_;
    double critical_threshold_percentage_;
    int sample_history_;
    bool anomaly_detection_enabled_;
    
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
