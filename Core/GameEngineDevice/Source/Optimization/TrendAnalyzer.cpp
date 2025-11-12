#include "TrendAnalyzer.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <cmath>
#include <sstream>

namespace GeneralsX::Optimization {

TrendAnalyzer::TrendAnalyzer()
    : trend_window_size_(100),
      minimum_data_points_(10),
      prediction_horizon_(10),
      initialized_(false) {
}

TrendAnalyzer::~TrendAnalyzer() {
    if (initialized_) {
        Shutdown();
    }
}

bool TrendAnalyzer::Initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    std::cout << "TrendAnalyzer initialized" << std::endl;
    return true;
}

bool TrendAnalyzer::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    trend_data_.clear();
    overall_frame_times_.clear();
    initialized_ = false;
    return true;
}

bool TrendAnalyzer::AnalyzeTrends(const std::vector<double>& frame_times) {
    if (!initialized_) {
        return false;
    }
    
    if (frame_times.size() < static_cast<size_t>(minimum_data_points_)) {
        return false;
    }
    
    overall_frame_times_ = frame_times;
    
    // Calculate overall trend
    std::vector<double> x_values;
    for (size_t i = 0; i < frame_times.size(); ++i) {
        x_values.push_back(static_cast<double>(i));
    }
    
    double slope = 0.0, intercept = 0.0, r_squared = 0.0;
    if (CalculateLinearRegression(x_values, frame_times, slope, intercept, r_squared)) {
        TrendData trend;
        trend.marker_name = "overall_frame_time";
        trend.time_values = frame_times;
        trend.frame_numbers = x_values;
        trend.trend_slope = slope;
        trend.trend_r_squared = r_squared;
        trend.trend_direction = DetermineTrendDirection(slope);
        
        trend_data_["overall_frame_time"] = trend;
    }
    
    return true;
}

bool TrendAnalyzer::TrackMarkerTrend(const std::string& marker_name,
                                     const std::vector<double>& time_values) {
    if (!initialized_) {
        return false;
    }
    
    if (time_values.size() < static_cast<size_t>(minimum_data_points_)) {
        return false;
    }
    
    std::vector<double> x_values;
    for (size_t i = 0; i < time_values.size(); ++i) {
        x_values.push_back(static_cast<double>(i));
    }
    
    double slope = 0.0, intercept = 0.0, r_squared = 0.0;
    if (CalculateLinearRegression(x_values, time_values, slope, intercept, r_squared)) {
        TrendData trend;
        trend.marker_name = marker_name;
        trend.time_values = time_values;
        trend.frame_numbers = x_values;
        trend.trend_slope = slope;
        trend.trend_r_squared = r_squared;
        trend.trend_direction = DetermineTrendDirection(slope);
        
        trend_data_[marker_name] = trend;
        return true;
    }
    
    return false;
}

TrendData TrendAnalyzer::GetTrend(const std::string& marker_name) const {
    auto it = trend_data_.find(marker_name);
    if (it != trend_data_.end()) {
        return it->second;
    }
    return TrendData();
}

std::vector<TrendData> TrendAnalyzer::GetAllTrends() const {
    std::vector<TrendData> trends;
    for (const auto& pair : trend_data_) {
        trends.push_back(pair.second);
    }
    return trends;
}

std::string TrendAnalyzer::GetTrendReport() const {
    std::stringstream ss;
    ss << "Trend Analysis Report\n";
    ss << "=====================\n\n";
    
    for (const auto& pair : trend_data_) {
        const TrendData& trend = pair.second;
        ss << "Marker: " << trend.marker_name << "\n";
        ss << "  Trend Direction: " << trend.trend_direction << "\n";
        ss << "  Slope: " << std::fixed << std::setprecision(6) << trend.trend_slope << "\n";
        ss << "  R-squared: " << trend.trend_r_squared << "\n\n";
    }
    
    return ss.str();
}

double TrendAnalyzer::GetTrendSlope(const std::string& marker_name) const {
    auto it = trend_data_.find(marker_name);
    if (it != trend_data_.end()) {
        return it->second.trend_slope;
    }
    return 0.0;
}

std::string TrendAnalyzer::GetTrendDirection(const std::string& marker_name) const {
    auto it = trend_data_.find(marker_name);
    if (it != trend_data_.end()) {
        return it->second.trend_direction;
    }
    return "unknown";
}

bool TrendAnalyzer::IsTrendImproving(const std::string& marker_name) const {
    auto it = trend_data_.find(marker_name);
    if (it != trend_data_.end()) {
        return it->second.trend_slope < 0.0;  // Negative slope means improving
    }
    return false;
}

bool TrendAnalyzer::IsTrendDegrading(const std::string& marker_name) const {
    auto it = trend_data_.find(marker_name);
    if (it != trend_data_.end()) {
        return it->second.trend_slope > 0.0;  // Positive slope means degrading
    }
    return false;
}

double TrendAnalyzer::PredictFutureValue(const std::string& marker_name, int frames_ahead) {
    auto it = trend_data_.find(marker_name);
    if (it == trend_data_.end()) {
        return 0.0;
    }
    
    const TrendData& trend = it->second;
    if (trend.time_values.empty()) {
        return 0.0;
    }
    
    // Linear prediction: y = slope * x + intercept
    double x = static_cast<double>(trend.frame_numbers.size() + frames_ahead);
    double last_value = trend.time_values.back();
    
    // Simple linear extrapolation
    return last_value + (trend.trend_slope * frames_ahead);
}

std::vector<double> TrendAnalyzer::PredictFrameTimeSeries(int steps_ahead) {
    std::vector<double> predictions;
    
    auto it = trend_data_.find("overall_frame_time");
    if (it == trend_data_.end()) {
        return predictions;
    }
    
    const TrendData& trend = it->second;
    double last_value = trend.time_values.back();
    
    for (int i = 1; i <= steps_ahead; ++i) {
        double predicted = last_value + (trend.trend_slope * i);
        predictions.push_back(predicted);
    }
    
    return predictions;
}

void TrendAnalyzer::SetTrendWindowSize(int frame_count) {
    trend_window_size_ = frame_count;
}

void TrendAnalyzer::SetMinimumDataPoints(int count) {
    minimum_data_points_ = count;
}

void TrendAnalyzer::SetPredictionHorizon(int frames) {
    prediction_horizon_ = frames;
}

bool TrendAnalyzer::CalculateLinearRegression(const std::vector<double>& x,
                                             const std::vector<double>& y,
                                             double& slope,
                                             double& intercept,
                                             double& r_squared) {
    if (x.size() != y.size() || x.empty()) {
        return false;
    }
    
    int n = x.size();
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;
    
    for (int i = 0; i < n; ++i) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
        sum_y2 += y[i] * y[i];
    }
    
    double denominator = n * sum_x2 - sum_x * sum_x;
    if (denominator == 0.0) {
        return false;
    }
    
    slope = (n * sum_xy - sum_x * sum_y) / denominator;
    double mean_x = sum_x / n;
    double mean_y = sum_y / n;
    intercept = mean_y - slope * mean_x;
    
    // Calculate R-squared
    double ss_tot = 0.0, ss_res = 0.0;
    for (int i = 0; i < n; ++i) {
        double y_pred = slope * x[i] + intercept;
        ss_tot += (y[i] - mean_y) * (y[i] - mean_y);
        ss_res += (y[i] - y_pred) * (y[i] - y_pred);
    }
    
    r_squared = 1.0 - (ss_res / ss_tot);
    
    return true;
}

std::string TrendAnalyzer::DetermineTrendDirection(double slope) {
    if (slope < -0.01) {
        return "improving";
    } else if (slope > 0.01) {
        return "degrading";
    } else {
        return "stable";
    }
}

}  // namespace GeneralsX::Optimization
