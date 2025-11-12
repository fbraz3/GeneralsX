#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <ctime>

namespace GeneralsX::Optimization {

// Forward declarations
class ProfilerDataAggregator;
class BottleneckAnalyzer;
class OptimizationRecommender;
class OptimizationValidator;
class PerformanceTracker;
class ReportGenerator;
class RegressionDetector;
class TrendAnalyzer;

// Data structures for optimization pipeline
struct Bottleneck {
    std::string marker_name;
    double time_ms;
    double percentage_of_frame;
    int call_count;
    int frame_count;
    
    Bottleneck() : time_ms(0.0), percentage_of_frame(0.0), call_count(0), frame_count(0) {}
};

struct Optimization {
    std::string name;
    std::string description;
    Bottleneck target;
    double estimated_gain_ms;
    double confidence_level;  // 0.0 - 1.0
    int priority_rank;
    bool applied;
    double actual_gain_ms;
    time_t applied_time;
    
    Optimization() : estimated_gain_ms(0.0), confidence_level(0.0), priority_rank(0),
                     applied(false), actual_gain_ms(0.0), applied_time(0) {}
};

struct OptimizationSession {
    time_t start_time;
    std::vector<Optimization> applied_optimizations;
    double total_gain_ms;
    double current_frame_time_ms;
    double baseline_frame_time_ms;
    int frame_count;
    
    OptimizationSession() : start_time(0), total_gain_ms(0.0), current_frame_time_ms(0.0),
                           baseline_frame_time_ms(0.0), frame_count(0) {}
};

struct PerformanceMetrics {
    double average_frame_time_ms;
    double min_frame_time_ms;
    double max_frame_time_ms;
    double percentile_99_frame_time_ms;
    int frame_count;
    std::vector<double> frame_times;
    
    PerformanceMetrics() : average_frame_time_ms(0.0), min_frame_time_ms(0.0),
                          max_frame_time_ms(0.0), percentile_99_frame_time_ms(0.0),
                          frame_count(0) {}
};

// Main Performance Optimizer class
class PerformanceOptimizer {
public:
    static PerformanceOptimizer& GetInstance();
    
    // Lifecycle
    bool Initialize();
    bool Shutdown();
    
    // Frame operations
    void BeginFrame();
    void EndFrame(double frame_time_ms);
    
    // Analysis operations
    bool CollectProfilerData(int sample_count = 300);
    bool AnalyzeBottlenecks();
    std::vector<Optimization> GetRecommendations();
    bool ApplyOptimization(const Optimization& optimization);
    bool ValidateOptimization(const Optimization& opt, double& actual_gain);
    
    // Tracking and reporting
    bool DetectRegressions();
    std::string GenerateReport();
    std::string GenerateTrendAnalysis();
    
    // Getters
    PerformanceMetrics GetCurrentMetrics() const;
    OptimizationSession GetCurrentSession() const;
    std::vector<Bottleneck> GetTopBottlenecks(int count = 10) const;
    bool IsOptimizationApplied(const std::string& opt_name) const;
    
    // Configuration
    void SetMinimumConfidenceThreshold(double threshold);
    void SetMaxBottlenecksToAnalyze(int count);
    void SetAnalysisInterval(int frame_count);

private:
    PerformanceOptimizer();
    ~PerformanceOptimizer();
    
    // Prevent copying
    PerformanceOptimizer(const PerformanceOptimizer&) = delete;
    PerformanceOptimizer& operator=(const PerformanceOptimizer&) = delete;
    
    // Component managers
    std::unique_ptr<ProfilerDataAggregator> data_aggregator_;
    std::unique_ptr<BottleneckAnalyzer> bottleneck_analyzer_;
    std::unique_ptr<OptimizationRecommender> recommendation_engine_;
    std::unique_ptr<OptimizationValidator> validator_;
    std::unique_ptr<PerformanceTracker> tracker_;
    std::unique_ptr<ReportGenerator> report_generator_;
    std::unique_ptr<RegressionDetector> regression_detector_;
    std::unique_ptr<TrendAnalyzer> trend_analyzer_;
    
    // Internal state
    OptimizationSession current_session_;
    PerformanceMetrics current_metrics_;
    std::vector<Bottleneck> detected_bottlenecks_;
    std::vector<Optimization> generated_recommendations_;
    std::map<std::string, Optimization> applied_optimizations_;
    
    // Configuration
    double confidence_threshold_;
    int max_bottlenecks_;
    int analysis_interval_;
    int frames_since_analysis_;
    
    // Frame timing
    std::vector<double> frame_times_;
    int max_frame_samples_;
    
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
