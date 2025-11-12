#pragma once

#include <string>
#include <vector>

namespace GeneralsX::Optimization {

enum class ReportFormat {
    TEXT,
    JSON,
    CSV,
    HTML
};

class ReportGenerator {
public:
    ReportGenerator();
    ~ReportGenerator();
    
    // Engine
    bool Initialize();
    bool Shutdown();
    
    // Report generation
    std::string GeneratePerformanceReport(const PerformanceMetrics& metrics);
    std::string GenerateBottleneckReport(const std::vector<Bottleneck>& bottlenecks);
    std::string GenerateRecommendationReport(const std::vector<Optimization>& recommendations);
    std::string GenerateOptimizationHistoryReport(const std::vector<OptimizationRecord>& history);
    std::string GenerateSummaryReport(const OptimizationSession& session);
    
    // Format control
    void SetReportFormat(ReportFormat format);
    void SetIncludeTimestamps(bool include);
    void SetIncludeDetailedMetrics(bool include);
    void SetVerboseMode(bool verbose);
    
    // Export
    bool ExportReport(const std::string& filename, const std::string& report_content);
    std::string GetLastReport() const;

private:
    std::string FormatPerformanceSection(const PerformanceMetrics& metrics);
    std::string FormatBottleneckSection(const std::vector<Bottleneck>& bottlenecks);
    std::string FormatRecommendationSection(const std::vector<Optimization>& recommendations);
    std::string FormatHistorySection(const std::vector<OptimizationRecord>& history);
    
    std::string FormatAsJSON(const std::string& data);
    std::string FormatAsHTML(const std::string& data);
    std::string FormatAsCSV(const std::vector<std::string>& rows);
    
    ReportFormat report_format_;
    bool include_timestamps_;
    bool include_detailed_metrics_;
    bool verbose_mode_;
    std::string last_report_;
    
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
