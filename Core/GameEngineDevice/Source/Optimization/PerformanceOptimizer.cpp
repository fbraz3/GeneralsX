#include "PerformanceOptimizer.h"
#include "ProfilerDataAggregator.h"
#include "BottleneckAnalyzer.h"
#include "OptimizationRecommender.h"
#include "OptimizationValidator.h"
#include "PerformanceTracker.h"
#include "ReportGenerator.h"
#include "RegressionDetector.h"
#include "TrendAnalyzer.h"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace GeneralsX::Optimization {

// Singleton instance
static PerformanceOptimizer* g_instance = nullptr;

PerformanceOptimizer& PerformanceOptimizer::GetInstance() {
    if (!g_instance) {
        g_instance = new PerformanceOptimizer();
    }
    return *g_instance;
}

PerformanceOptimizer::PerformanceOptimizer()
    : confidence_threshold_(0.7),
      max_bottlenecks_(10),
      analysis_interval_(300),
      frames_since_analysis_(0),
      max_frame_samples_(300),
      initialized_(false) {
}

PerformanceOptimizer::~PerformanceOptimizer() {
    if (initialized_) {
        Shutdown();
    }
}

bool PerformanceOptimizer::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // Initialize components
    data_aggregator_ = std::make_unique<ProfilerDataAggregator>();
    bottleneck_analyzer_ = std::make_unique<BottleneckAnalyzer>();
    recommendation_engine_ = std::make_unique<OptimizationRecommender>();
    validator_ = std::make_unique<OptimizationValidator>();
    tracker_ = std::make_unique<PerformanceTracker>();
    report_generator_ = std::make_unique<ReportGenerator>();
    regression_detector_ = std::make_unique<RegressionDetector>();
    trend_analyzer_ = std::make_unique<TrendAnalyzer>();
    
    // Initialize each component
    if (!data_aggregator_->Initialize() ||
        !bottleneck_analyzer_->Initialize() ||
        !recommendation_engine_->Initialize() ||
        !validator_->Initialize() ||
        !tracker_->Initialize() ||
        !report_generator_->Initialize() ||
        !regression_detector_->Initialize() ||
        !trend_analyzer_->Initialize()) {
        std::cerr << "PerformanceOptimizer: Failed to initialize components" << std::endl;
        return false;
    }
    
    // Initialize session
    current_session_.start_time = time(nullptr);
    current_session_.frame_count = 0;
    current_session_.total_gain_ms = 0.0;
    
    initialized_ = true;
    std::cout << "PerformanceOptimizer initialized" << std::endl;
    return true;
}

bool PerformanceOptimizer::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    // Shutdown components in reverse order
    if (trend_analyzer_) trend_analyzer_->Shutdown();
    if (regression_detector_) regression_detector_->Shutdown();
    if (report_generator_) report_generator_->Shutdown();
    if (tracker_) tracker_->Shutdown();
    if (validator_) validator_->Shutdown();
    if (recommendation_engine_) recommendation_engine_->Shutdown();
    if (bottleneck_analyzer_) bottleneck_analyzer_->Shutdown();
    if (data_aggregator_) data_aggregator_->Shutdown();
    
    initialized_ = false;
    std::cout << "PerformanceOptimizer shutdown" << std::endl;
    return true;
}

void PerformanceOptimizer::BeginFrame() {
    if (!initialized_) return;
    
    current_session_.frame_count++;
    frames_since_analysis_++;
}

void PerformanceOptimizer::EndFrame(double frame_time_ms) {
    if (!initialized_) return;
    
    // Record frame time
    frame_times_.push_back(frame_time_ms);
    if (frame_times_.size() > static_cast<size_t>(max_frame_samples_)) {
        frame_times_.erase(frame_times_.begin());
    }
    
    current_session_.current_frame_time_ms = frame_time_ms;
    tracker_->RecordFrameTime(frame_time_ms);
    
    // Check for regressions every frame
    current_metrics_.frame_count++;
}

bool PerformanceOptimizer::CollectProfilerData(int sample_count) {
    if (!initialized_ || !data_aggregator_) return false;
    
    data_aggregator_->SetSampleCount(sample_count);
    if (!data_aggregator_->CollectSamples(sample_count)) {
        std::cerr << "PerformanceOptimizer: Failed to collect profiler data" << std::endl;
        return false;
    }
    
    if (!data_aggregator_->AggregateData()) {
        std::cerr << "PerformanceOptimizer: Failed to aggregate profiler data" << std::endl;
        return false;
    }
    
    return true;
}

bool PerformanceOptimizer::AnalyzeBottlenecks() {
    if (!initialized_ || !data_aggregator_ || !bottleneck_analyzer_) return false;
    
    auto profile = data_aggregator_->GetAggregatedProfile();
    double avg_frame_time = current_metrics_.average_frame_time_ms;
    
    if (!bottleneck_analyzer_->AnalyzeBottlenecks(profile.samples, avg_frame_time)) {
        std::cerr << "PerformanceOptimizer: Failed to analyze bottlenecks" << std::endl;
        return false;
    }
    
    detected_bottlenecks_ = bottleneck_analyzer_->GetBottlenecks();
    return true;
}

std::vector<Optimization> PerformanceOptimizer::GetRecommendations() {
    if (!initialized_ || !recommendation_engine_) return {};
    
    return recommendation_engine_->GetRecommendations();
}

bool PerformanceOptimizer::ApplyOptimization(const Optimization& optimization) {
    if (!initialized_) return false;
    
    // Store the optimization
    Optimization opt = optimization;
    opt.applied_time = time(nullptr);
    applied_optimizations_[optimization.name] = opt;
    
    current_session_.applied_optimizations.push_back(opt);
    
    return true;
}

bool PerformanceOptimizer::ValidateOptimization(const Optimization& opt, double& actual_gain) {
    if (!initialized_ || !validator_) return false;
    
    if (!validator_->BeginValidation(opt)) {
        return false;
    }
    
    actual_gain = validator_->GetActualGain();
    return true;
}

bool PerformanceOptimizer::DetectRegressions() {
    if (!initialized_ || !regression_detector_) return false;
    
    // Update baseline on first run
    static bool first_run = true;
    if (first_run) {
        regression_detector_->UpdateBaseline(current_metrics_);
        first_run = false;
        return true;
    }
    
    return regression_detector_->CheckForRegressions(current_metrics_);
}

std::string PerformanceOptimizer::GenerateReport() {
    if (!initialized_ || !report_generator_) return "PerformanceOptimizer not initialized";
    
    return report_generator_->GenerateSummaryReport(current_session_);
}

std::string PerformanceOptimizer::GenerateTrendAnalysis() {
    if (!initialized_ || !trend_analyzer_) return "PerformanceOptimizer not initialized";
    
    trend_analyzer_->AnalyzeTrends(frame_times_);
    return trend_analyzer_->GetTrendReport();
}

PerformanceMetrics PerformanceOptimizer::GetCurrentMetrics() const {
    return current_metrics_;
}

OptimizationSession PerformanceOptimizer::GetCurrentSession() const {
    return current_session_;
}

std::vector<Bottleneck> PerformanceOptimizer::GetTopBottlenecks(int count) const {
    if (!bottleneck_analyzer_) return {};
    
    return bottleneck_analyzer_->GetTopBottlenecks(count);
}

bool PerformanceOptimizer::IsOptimizationApplied(const std::string& opt_name) const {
    return applied_optimizations_.find(opt_name) != applied_optimizations_.end();
}

void PerformanceOptimizer::SetMinimumConfidenceThreshold(double threshold) {
    confidence_threshold_ = threshold;
    if (recommendation_engine_) {
        recommendation_engine_->SetMinimumConfidence(threshold);
    }
}

void PerformanceOptimizer::SetMaxBottlenecksToAnalyze(int count) {
    max_bottlenecks_ = count;
    if (bottleneck_analyzer_) {
        bottleneck_analyzer_->SetMaxBottlenecks(count);
    }
}

void PerformanceOptimizer::SetAnalysisInterval(int frame_count) {
    analysis_interval_ = frame_count;
}

}  // namespace GeneralsX::Optimization
