#include "BottleneckAnalyzer.h"
#include "ProfilerDataAggregator.h"
#include <algorithm>
#include <iostream>
#include <numeric>

namespace GeneralsX::Optimization {

BottleneckAnalyzer::BottleneckAnalyzer()
    : minimum_threshold_percentage_(1.0),
      max_bottlenecks_(10),
      analysis_depth_(3),
      total_bottleneck_time_(0.0),
      frame_time_ms_(0.0),
      initialized_(false) {
}

BottleneckAnalyzer::~BottleneckAnalyzer() {
    if (initialized_) {
        Shutdown();
    }
}

bool BottleneckAnalyzer::Initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    std::cout << "BottleneckAnalyzer initialized" << std::endl;
    return true;
}

bool BottleneckAnalyzer::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    bottlenecks_.clear();
    ranked_bottlenecks_.clear();
    initialized_ = false;
    return true;
}

bool BottleneckAnalyzer::AnalyzeBottlenecks(const std::vector<ProfilerSample>& samples, double frame_time_ms) {
    if (!initialized_) {
        return false;
    }
    
    frame_time_ms_ = frame_time_ms;
    bottlenecks_.clear();
    
    if (!IdentifyHotspots(samples)) {
        return false;
    }
    
    if (!RankBottlenecks()) {
        return false;
    }
    
    if (!CalculateImpact()) {
        return false;
    }
    
    return true;
}

bool BottleneckAnalyzer::IdentifyHotspots(const std::vector<ProfilerSample>& samples) {
    for (const auto& sample : samples) {
        double total_time = sample.cpu_time_ms + sample.gpu_time_ms;
        double percentage = (total_time / frame_time_ms_) * 100.0;
        
        if (percentage >= minimum_threshold_percentage_) {
            Bottleneck bottleneck;
            bottleneck.marker_name = sample.marker_name;
            bottleneck.time_ms = total_time;
            bottleneck.percentage_of_frame = percentage;
            bottleneck.call_count = sample.call_count;
            bottleneck.frame_count = 1;
            
            bottlenecks_.push_back(bottleneck);
            total_bottleneck_time_ += total_time;
        }
    }
    
    return true;
}

bool BottleneckAnalyzer::RankBottlenecks() {
    // Sort bottlenecks by percentage of frame (descending)
    ranked_bottlenecks_ = bottlenecks_;
    std::sort(ranked_bottlenecks_.begin(), ranked_bottlenecks_.end(),
        [](const Bottleneck& a, const Bottleneck& b) {
            return a.percentage_of_frame > b.percentage_of_frame;
        });
    
    return true;
}

bool BottleneckAnalyzer::CalculateImpact() {
    // Already calculated during IdentifyHotspots
    return true;
}

std::vector<Bottleneck> BottleneckAnalyzer::GetBottlenecks() const {
    return bottlenecks_;
}

std::vector<Bottleneck> BottleneckAnalyzer::GetTopBottlenecks(int count) const {
    std::vector<Bottleneck> top;
    int limit = std::min(count, static_cast<int>(ranked_bottlenecks_.size()));
    
    for (int i = 0; i < limit; ++i) {
        top.push_back(ranked_bottlenecks_[i]);
    }
    
    return top;
}

Bottleneck BottleneckAnalyzer::GetBottleneck(const std::string& marker_name) const {
    for (const auto& bottleneck : bottlenecks_) {
        if (bottleneck.marker_name == marker_name) {
            return bottleneck;
        }
    }
    return Bottleneck();
}

double BottleneckAnalyzer::GetTotalBottleneckTime() const {
    return total_bottleneck_time_;
}

int BottleneckAnalyzer::GetBottleneckCount() const {
    return bottlenecks_.size();
}

void BottleneckAnalyzer::SetMinimumThreshold(double percentage) {
    minimum_threshold_percentage_ = percentage;
}

void BottleneckAnalyzer::SetMaxBottlenecks(int count) {
    max_bottlenecks_ = count;
}

void BottleneckAnalyzer::SetAnalysisDepth(int depth) {
    analysis_depth_ = depth;
}

}  // namespace GeneralsX::Optimization
