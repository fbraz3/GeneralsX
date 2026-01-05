#pragma once

#include <vector>
#include <string>

namespace GeneralsX::Optimization {

class BottleneckAnalyzer {
public:
    BottleneckAnalyzer();
    ~BottleneckAnalyzer();
    
    // Analysis
    bool Initialize();
    bool Shutdown();
    
    // Bottleneck identification
    bool AnalyzeBottlenecks(const std::vector<ProfilerSample>& samples, double frame_time_ms);
    
    // Results
    std::vector<Bottleneck> GetBottlenecks() const;
    std::vector<Bottleneck> GetTopBottlenecks(int count) const;
    
    // Analysis details
    Bottleneck GetBottleneck(const std::string& marker_name) const;
    double GetTotalBottleneckTime() const;
    int GetBottleneckCount() const;
    
    // Configuration
    void SetMinimumThreshold(double percentage);
    void SetMaxBottlenecks(int count);
    void SetAnalysisDepth(int depth);

private:
    bool IdentifyHotspots(const std::vector<ProfilerSample>& samples);
    bool RankBottlenecks();
    bool CalculateImpact();
    
    std::vector<Bottleneck> bottlenecks_;
    std::vector<Bottleneck> ranked_bottlenecks_;
    
    double minimum_threshold_percentage_;
    int max_bottlenecks_;
    int analysis_depth_;
    double total_bottleneck_time_;
    double frame_time_ms_;
    
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
