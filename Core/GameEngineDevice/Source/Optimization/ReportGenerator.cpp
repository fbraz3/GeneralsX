#include "ReportGenerator.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

namespace GeneralsX::Optimization {

ReportGenerator::ReportGenerator()
    : report_format_(ReportFormat::TEXT),
      include_timestamps_(true),
      include_detailed_metrics_(true),
      verbose_mode_(false),
      initialized_(false) {
}

ReportGenerator::~ReportGenerator() {
    if (initialized_) {
        Shutdown();
    }
}

bool ReportGenerator::Initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    std::cout << "ReportGenerator initialized" << std::endl;
    return true;
}

bool ReportGenerator::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    initialized_ = false;
    return true;
}

std::string ReportGenerator::GeneratePerformanceReport(const PerformanceMetrics& metrics) {
    std::stringstream ss;
    
    ss << "Performance Report\n";
    ss << "==================\n\n";
    
    if (include_timestamps_) {
        time_t now = time(nullptr);
        ss << "Generated: " << ctime(&now);
    }
    
    ss << "Frame Time Statistics:\n";
    ss << "  Average: " << std::fixed << std::setprecision(2) << metrics.average_frame_time_ms << " ms\n";
    ss << "  Min: " << metrics.min_frame_time_ms << " ms\n";
    ss << "  Max: " << metrics.max_frame_time_ms << " ms\n";
    ss << "  99th Percentile: " << metrics.percentile_99_frame_time_ms << " ms\n";
    ss << "  Total Frames: " << metrics.frame_count << "\n\n";
    
    return ss.str();
}

std::string ReportGenerator::GenerateBottleneckReport(const std::vector<Bottleneck>& bottlenecks) {
    std::stringstream ss;
    
    ss << "Bottleneck Analysis Report\n";
    ss << "==========================\n\n";
    
    ss << "Detected Bottlenecks: " << bottlenecks.size() << "\n\n";
    
    for (size_t i = 0; i < bottlenecks.size(); ++i) {
        ss << (i + 1) << ". " << bottlenecks[i].marker_name << "\n";
        ss << "   Time: " << std::fixed << std::setprecision(2) 
           << bottlenecks[i].time_ms << " ms ("
           << bottlenecks[i].percentage_of_frame << "%)\n";
        ss << "   Call Count: " << bottlenecks[i].call_count << "\n\n";
    }
    
    return ss.str();
}

std::string ReportGenerator::GenerateRecommendationReport(const std::vector<Optimization>& recommendations) {
    std::stringstream ss;
    
    ss << "Optimization Recommendations\n";
    ss << "============================\n\n";
    
    ss << "Total Recommendations: " << recommendations.size() << "\n\n";
    
    for (size_t i = 0; i < recommendations.size(); ++i) {
        ss << (i + 1) << ". " << recommendations[i].name << "\n";
        ss << "   Description: " << recommendations[i].description << "\n";
        ss << "   Estimated Gain: " << std::fixed << std::setprecision(2)
           << recommendations[i].estimated_gain_ms << " ms\n";
        ss << "   Confidence: " << (recommendations[i].confidence_level * 100) << "%\n";
        ss << "   Priority: " << recommendations[i].priority_rank << "\n\n";
    }
    
    return ss.str();
}

std::string ReportGenerator::GenerateOptimizationHistoryReport(const std::vector<OptimizationRecord>& history) {
    std::stringstream ss;
    
    ss << "Optimization History\n";
    ss << "====================\n\n";
    
    ss << "Total Applied: " << history.size() << "\n\n";
    
    for (size_t i = 0; i < history.size(); ++i) {
        ss << (i + 1) << ". " << history[i].optimization_name << "\n";
        ss << "   Before: " << std::fixed << std::setprecision(2) 
           << history[i].frame_time_before_ms << " ms\n";
        ss << "   After: " << history[i].frame_time_after_ms << " ms\n";
        ss << "   Gain: " << history[i].actual_gain_ms << " ms ("
           << history[i].gain_percentage << "%)\n\n";
    }
    
    return ss.str();
}

std::string ReportGenerator::GenerateSummaryReport(const OptimizationSession& session) {
    std::stringstream ss;
    
    ss << "Optimization Session Summary\n";
    ss << "=============================\n\n";
    
    if (include_timestamps_) {
        ss << "Start Time: " << ctime(&session.start_time);
    }
    
    ss << "Frame Count: " << session.frame_count << "\n";
    ss << "Current Frame Time: " << std::fixed << std::setprecision(2) 
       << session.current_frame_time_ms << " ms\n";
    ss << "Baseline Frame Time: " << session.baseline_frame_time_ms << " ms\n";
    ss << "Total Gain: " << session.total_gain_ms << " ms\n";
    ss << "Applied Optimizations: " << session.applied_optimizations.size() << "\n\n";
    
    return ss.str();
}

void ReportGenerator::SetReportFormat(ReportFormat format) {
    report_format_ = format;
}

void ReportGenerator::SetIncludeTimestamps(bool include) {
    include_timestamps_ = include;
}

void ReportGenerator::SetIncludeDetailedMetrics(bool include) {
    include_detailed_metrics_ = include;
}

void ReportGenerator::SetVerboseMode(bool verbose) {
    verbose_mode_ = verbose;
}

bool ReportGenerator::ExportReport(const std::string& filename, const std::string& report_content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for export: " << filename << std::endl;
        return false;
    }
    
    file << report_content;
    file.close();
    
    std::cout << "Report exported to: " << filename << std::endl;
    return true;
}

std::string ReportGenerator::GetLastReport() const {
    return last_report_;
}

std::string ReportGenerator::FormatPerformanceSection(const PerformanceMetrics& metrics) {
    return GeneratePerformanceReport(metrics);
}

std::string ReportGenerator::FormatBottleneckSection(const std::vector<Bottleneck>& bottlenecks) {
    return GenerateBottleneckReport(bottlenecks);
}

std::string ReportGenerator::FormatRecommendationSection(const std::vector<Optimization>& recommendations) {
    return GenerateRecommendationReport(recommendations);
}

std::string ReportGenerator::FormatHistorySection(const std::vector<OptimizationRecord>& history) {
    return GenerateOptimizationHistoryReport(history);
}

std::string ReportGenerator::FormatAsJSON(const std::string& data) {
    return "{\"data\": \"" + data + "\"}";
}

std::string ReportGenerator::FormatAsHTML(const std::string& data) {
    return "<html><body>" + data + "</body></html>";
}

std::string ReportGenerator::FormatAsCSV(const std::vector<std::string>& rows) {
    std::stringstream ss;
    for (const auto& row : rows) {
        ss << row << "\n";
    }
    return ss.str();
}

}  // namespace GeneralsX::Optimization
