#include "ProfilerDataAggregator.h"
#include <algorithm>
#include <iostream>
#include <numeric>

namespace GeneralsX::Optimization {

ProfilerDataAggregator::ProfilerDataAggregator()
    : sample_count_(300),
      min_time_threshold_ms_(0.1),
      total_samples_collected_(0),
      initialized_(false) {
}

ProfilerDataAggregator::~ProfilerDataAggregator() {
    if (initialized_) {
        Shutdown();
    }
}

bool ProfilerDataAggregator::Initialize() {
    if (initialized_) {
        return true;
    }
    
    aggregated_data_.total_frames_sampled = 0;
    aggregated_data_.aggregation_time_ms = 0.0;
    
    initialized_ = true;
    std::cout << "ProfilerDataAggregator initialized" << std::endl;
    return true;
}

bool ProfilerDataAggregator::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    marker_samples_.clear();
    aggregated_data_.samples.clear();
    initialized_ = false;
    return true;
}

bool ProfilerDataAggregator::CollectSamples(int frame_count) {
    if (!initialized_) {
        return false;
    }
    
    // This would normally query the profiler from Phase 39
    // For now, we create mock data for demonstration
    return QueryProfilerData();
}

bool ProfilerDataAggregator::QueryProfilerData() {
    // Mock profiler data collection
    // In production, this would query Phase 39 Profiler
    
    // Create sample markers
    std::vector<std::string> markers = {
        "Render",
        "Physics",
        "AI",
        "Animation",
        "Terrain",
        "Particles",
        "UI",
        "Input",
        "Audio",
        "Memory"
    };
    
    // Generate mock samples for each marker
    for (const auto& marker : markers) {
        ProfilerSample sample;
        sample.marker_name = marker;
        sample.cpu_time_ms = 0.5 + (rand() % 30) / 10.0;  // 0.5-3.5ms
        sample.gpu_time_ms = 0.3 + (rand() % 20) / 10.0;  // 0.3-2.3ms
        sample.call_count = 10 + (rand() % 90);
        sample.memory_mb = 1.0 + (rand() % 50);
        
        aggregated_data_.samples.push_back(sample);
        total_samples_collected_++;
    }
    
    aggregated_data_.total_frames_sampled = sample_count_;
    return ProcessSamples();
}

bool ProfilerDataAggregator::ProcessSamples() {
    // Organize samples by marker name
    for (const auto& sample : aggregated_data_.samples) {
        marker_samples_[sample.marker_name].push_back(sample);
    }
    
    std::cout << "ProfilerDataAggregator: Collected " << aggregated_data_.samples.size()
              << " marker types, " << total_samples_collected_ << " total samples" << std::endl;
    return true;
}

bool ProfilerDataAggregator::AggregateData() {
    if (!initialized_) {
        return false;
    }
    
    // Filter out samples below threshold
    std::vector<ProfilerSample> filtered_samples;
    for (const auto& sample : aggregated_data_.samples) {
        if (sample.cpu_time_ms >= min_time_threshold_ms_) {
            filtered_samples.push_back(sample);
        }
    }
    
    aggregated_data_.samples = filtered_samples;
    return true;
}

AggregatedProfile ProfilerDataAggregator::GetAggregatedProfile() const {
    return aggregated_data_;
}

std::vector<ProfilerSample> ProfilerDataAggregator::GetSamplesForMarker(const std::string& marker_name) const {
    auto it = marker_samples_.find(marker_name);
    if (it != marker_samples_.end()) {
        return it->second;
    }
    return {};
}

double ProfilerDataAggregator::GetAverageCPUTime(const std::string& marker_name) const {
    auto samples = GetSamplesForMarker(marker_name);
    if (samples.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& sample : samples) {
        sum += sample.cpu_time_ms;
    }
    return sum / samples.size();
}

double ProfilerDataAggregator::GetAverageGPUTime(const std::string& marker_name) const {
    auto samples = GetSamplesForMarker(marker_name);
    if (samples.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& sample : samples) {
        sum += sample.gpu_time_ms;
    }
    return sum / samples.size();
}

double ProfilerDataAggregator::GetAverageMemory(const std::string& marker_name) const {
    auto samples = GetSamplesForMarker(marker_name);
    if (samples.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& sample : samples) {
        sum += sample.memory_mb;
    }
    return sum / samples.size();
}

int ProfilerDataAggregator::GetTotalCallCount(const std::string& marker_name) const {
    auto samples = GetSamplesForMarker(marker_name);
    if (samples.empty()) return 0;
    
    int total = 0;
    for (const auto& sample : samples) {
        total += sample.call_count;
    }
    return total;
}

void ProfilerDataAggregator::SetSampleCount(int count) {
    sample_count_ = count;
}

void ProfilerDataAggregator::SetMinimumTimeThreshold(double ms) {
    min_time_threshold_ms_ = ms;
}

int ProfilerDataAggregator::GetMarkerCount() const {
    return marker_samples_.size();
}

int ProfilerDataAggregator::GetTotalSamplesCollected() const {
    return total_samples_collected_;
}

}  // namespace GeneralsX::Optimization
