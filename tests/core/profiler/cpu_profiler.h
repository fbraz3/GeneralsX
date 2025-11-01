#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>
#include <iostream>

namespace phase47 {

class CPUProfiler {
public:
    static CPUProfiler& Instance() {
        static CPUProfiler instance;
        return instance;
    }

    // Begin a named scope
    void BeginScope(const std::string& scope_name);

    // End the current scope
    void EndScope(const std::string& scope_name);

    // Get timing for a specific scope
    struct ScopeStats {
        double total_time_ms;
        double min_time_ms;
        double max_time_ms;
        double avg_time_ms;
        uint32_t call_count;
    };

    ScopeStats GetScopeStats(const std::string& scope_name) const;

    // Print profiling report
    void PrintReport() const;

    // Generate report to file
    void GenerateReport(const std::string& filename) const;

    // Clear all data
    void Clear();

    // RAII scope guard
    class ScopeGuard {
    public:
        ScopeGuard(CPUProfiler& profiler, const std::string& scope_name)
            : profiler_(profiler), scope_name_(scope_name) {
            profiler_.BeginScope(scope_name);
        }

        ~ScopeGuard() {
            profiler_.EndScope(scope_name_);
        }

    private:
        CPUProfiler& profiler_;
        std::string scope_name_;
    };

private:
    CPUProfiler() = default;

    struct ScopeData {
        std::string name;
        std::chrono::high_resolution_clock::time_point start_time;
        double total_time_ms = 0.0;
        double min_time_ms = std::numeric_limits<double>::max();
        double max_time_ms = 0.0;
        uint32_t call_count = 0;
    };

    std::map<std::string, ScopeData> scopes_;
    std::vector<std::pair<std::string, std::chrono::high_resolution_clock::time_point>> scope_stack_;
};

// Convenience macro for scope-based profiling
#define CPU_PROFILE_SCOPE(name) \
    phase47::CPUProfiler::ScopeGuard guard(phase47::CPUProfiler::Instance(), name)

#define CPU_PROFILE_BEGIN(name) \
    phase47::CPUProfiler::Instance().BeginScope(name)

#define CPU_PROFILE_END(name) \
    phase47::CPUProfiler::Instance().EndScope(name)

}  // namespace phase47
