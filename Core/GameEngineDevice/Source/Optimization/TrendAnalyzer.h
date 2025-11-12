#pragma once

#include <vector>
#include <string>
#include <map>

namespace GeneralsX::Optimization {

struct TrendData {
    std::string marker_name;
    std::vector<double> time_values;
    std::vector<double> frame_numbers;
    double trend_slope;
    double trend_r_squared;
    std::string trend_direction;  // "improving", "degrading", "stable"
};

class TrendAnalyzer {
public:
    TrendAnalyzer();
    ~TrendAnalyzer();
    
    // Engine
    bool Initialize();
    bool Shutdown();
    
    // Analysis operations
    bool AnalyzeTrends(const std::vector<double>& frame_times);
    bool TrackMarkerTrend(const std::string& marker_name,
                          const std::vector<double>& time_values);
    
    // Results
    TrendData GetTrend(const std::string& marker_name) const;
    std::vector<TrendData> GetAllTrends() const;
    std::string GetTrendReport() const;
    
    // Analysis
    double GetTrendSlope(const std::string& marker_name) const;
    std::string GetTrendDirection(const std::string& marker_name) const;
    bool IsTrendImproving(const std::string& marker_name) const;
    bool IsTrendDegrading(const std::string& marker_name) const;
    
    // Prediction
    double PredictFutureValue(const std::string& marker_name,
                              int frames_ahead);
    std::vector<double> PredictFrameTimeSeries(int steps_ahead);
    
    // Configuration
    void SetTrendWindowSize(int frame_count);
    void SetMinimumDataPoints(int count);
    void SetPredictionHorizon(int frames);

private:
    bool CalculateLinearRegression(const std::vector<double>& x,
                                   const std::vector<double>& y,
                                   double& slope,
                                   double& intercept,
                                   double& r_squared);
    
    std::string DetermineTrendDirection(double slope);
    
    std::map<std::string, TrendData> trend_data_;
    std::vector<double> overall_frame_times_;
    
    int trend_window_size_;
    int minimum_data_points_;
    int prediction_horizon_;
    
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
