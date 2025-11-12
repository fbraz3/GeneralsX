#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>

namespace GeneralsX::Optimization {

struct ProfilerSample {
    std::string marker_name;
    double cpu_time_ms;
    double gpu_time_ms;
    int call_count;
    double memory_mb;
};

struct AggregatedProfile {
    std::vector<ProfilerSample> samples;
    int total_frames_sampled;
    double aggregation_time_ms;
};

class ProfilerDataAggregator {
public:
    ProfilerDataAggregator();
    ~ProfilerDataAggregator();
    
    // Data collection
    bool Initialize();
    bool Shutdown();
    
    // Sampling and aggregation
    bool CollectSamples(int frame_count = 300);
    bool AggregateData();
    
    // Analysis getters
    AggregatedProfile GetAggregatedProfile() const;
    std::vector<ProfilerSample> GetSamplesForMarker(const std::string& marker_name) const;
    
    // Statistics
    double GetAverageCPUTime(const std::string& marker_name) const;
    double GetAverageGPUTime(const std::string& marker_name) const;
    double GetAverageMemory(const std::string& marker_name) const;
    int GetTotalCallCount(const std::string& marker_name) const;
    
    // Configuration
    void SetSampleCount(int count);
    void SetMinimumTimeThreshold(double ms);
    
    // Statistics
    int GetMarkerCount() const;
    int GetTotalSamplesCollected() const;

private:
    bool QueryProfilerData();
    bool ProcessSamples();
    
    AggregatedProfile aggregated_data_;
    std::map<std::string, std::vector<ProfilerSample>> marker_samples_;
    
    int sample_count_;
    double min_time_threshold_ms_;
    int total_samples_collected_;
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
