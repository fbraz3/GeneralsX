#include "RegressionDetector.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <cmath>

namespace GeneralsX::Optimization {

RegressionDetector::RegressionDetector()
    : regression_threshold_percentage_(5.0),
      critical_threshold_percentage_(10.0),
      sample_history_(100),
      anomaly_detection_enabled_(true),
      initialized_(false) {
}

RegressionDetector::~RegressionDetector() {
    if (initialized_) {
        Shutdown();
    }
}

bool RegressionDetector::Initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    std::cout << "RegressionDetector initialized" << std::endl;
    return true;
}

bool RegressionDetector::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    current_regressions_.clear();
    historical_metrics_.clear();
    initialized_ = false;
    return true;
}

bool RegressionDetector::UpdateBaseline(const PerformanceMetrics& metrics) {
    if (!initialized_) {
        return false;
    }
    
    baseline_metrics_ = metrics;
    historical_metrics_.push_back(metrics);
    
    return true;
}

bool RegressionDetector::CheckForRegressions(const PerformanceMetrics& current_metrics) {
    if (!initialized_) {
        return false;
    }
    
    current_regressions_.clear();
    historical_metrics_.push_back(current_metrics);
    
    if (historical_metrics_.size() > static_cast<size_t>(sample_history_)) {
        historical_metrics_.erase(historical_metrics_.begin());
    }
    
    // Check for regressions in average frame time
    double baseline_avg = baseline_metrics_.average_frame_time_ms;
    double current_avg = current_metrics.average_frame_time_ms;
    double degradation = current_avg - baseline_avg;
    double degradation_percentage = (degradation / baseline_avg) * 100.0;
    
    if (degradation_percentage > regression_threshold_percentage_) {
        RegressionAlert alert;
        alert.marker_name = "overall_frame_time";
        alert.baseline_time_ms = baseline_avg;
        alert.current_time_ms = current_avg;
        alert.degradation_ms = degradation;
        alert.degradation_percentage = degradation_percentage;
        alert.frames_detected = current_metrics.frame_count;
        alert.critical = degradation_percentage > critical_threshold_percentage_;
        
        current_regressions_.push_back(alert);
    }
    
    // Check for regressions in 99th percentile
    double baseline_p99 = baseline_metrics_.percentile_99_frame_time_ms;
    double current_p99 = current_metrics.percentile_99_frame_time_ms;
    double p99_degradation = current_p99 - baseline_p99;
    double p99_degradation_percentage = (p99_degradation / baseline_p99) * 100.0;
    
    if (p99_degradation_percentage > critical_threshold_percentage_) {
        RegressionAlert alert;
        alert.marker_name = "p99_frame_time";
        alert.baseline_time_ms = baseline_p99;
        alert.current_time_ms = current_p99;
        alert.degradation_ms = p99_degradation;
        alert.degradation_percentage = p99_degradation_percentage;
        alert.frames_detected = current_metrics.frame_count;
        alert.critical = true;
        
        current_regressions_.push_back(alert);
    }
    
    return true;
}

bool RegressionDetector::MonitorBottleneck(const std::string& marker_name, double current_time_ms) {
    if (!initialized_) {
        return false;
    }
    
    // Could track individual bottlenecks here
    return true;
}

std::vector<RegressionAlert> RegressionDetector::GetRegressions() const {
    return current_regressions_;
}

bool RegressionDetector::HasRegression(const std::string& marker_name) const {
    for (const auto& regression : current_regressions_) {
        if (regression.marker_name == marker_name) {
            return true;
        }
    }
    return false;
}

int RegressionDetector::GetRegressionCount() const {
    return current_regressions_.size();
}

int RegressionDetector::GetCriticalRegressionCount() const {
    int count = 0;
    for (const auto& regression : current_regressions_) {
        if (regression.critical) {
            count++;
        }
    }
    return count;
}

void RegressionDetector::SetRegressionThreshold(double percentage) {
    regression_threshold_percentage_ = percentage;
}

void RegressionDetector::SetCriticalThreshold(double percentage) {
    critical_threshold_percentage_ = percentage;
}

void RegressionDetector::SetSampleHistory(int count) {
    sample_history_ = count;
}

void RegressionDetector::SetAnomalyDetectionEnabled(bool enabled) {
    anomaly_detection_enabled_ = enabled;
}

bool RegressionDetector::DetectAnomalies(const std::vector<double>& historical_data,
                                         double current_value) {
    if (historical_data.size() < 2) {
        return false;
    }
    
    double mean = 0.0, stddev = 0.0;
    if (!CalculateStats(historical_data, mean, stddev)) {
        return false;
    }
    
    // Check if current value is more than 2 standard deviations from mean
    double deviation = std::abs(current_value - mean);
    return deviation > (2.0 * stddev);
}

bool RegressionDetector::CalculateStats(const std::vector<double>& data,
                                        double& mean, double& stddev) {
    if (data.empty()) {
        return false;
    }
    
    // Calculate mean
    mean = 0.0;
    for (double value : data) {
        mean += value;
    }
    mean /= data.size();
    
    // Calculate standard deviation
    stddev = 0.0;
    for (double value : data) {
        double diff = value - mean;
        stddev += diff * diff;
    }
    stddev /= data.size();
    stddev = std::sqrt(stddev);
    
    return true;
}

}  // namespace GeneralsX::Optimization
