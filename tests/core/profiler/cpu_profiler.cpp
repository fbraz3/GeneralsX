#include "cpu_profiler.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <limits>

namespace phase47 {

void CPUProfiler::BeginScope(const std::string& scope_name) {
    scope_stack_.push_back({scope_name, std::chrono::high_resolution_clock::now()});
}

void CPUProfiler::EndScope(const std::string& scope_name) {
    auto now = std::chrono::high_resolution_clock::now();

    if (scope_stack_.empty()) {
        return;
    }

    auto& last = scope_stack_.back();
    if (last.first == scope_name) {
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
            now - last.second
        ).count();

        double duration_ms = duration_us / 1000.0;

        auto& stats = scopes_[scope_name];
        stats.name = scope_name;
        stats.total_time_ms += duration_ms;
        stats.min_time_ms = std::min(stats.min_time_ms, duration_ms);
        stats.max_time_ms = std::max(stats.max_time_ms, duration_ms);
        stats.call_count++;

        scope_stack_.pop_back();
    }
}

CPUProfiler::ScopeStats CPUProfiler::GetScopeStats(const std::string& scope_name) const {
    auto it = scopes_.find(scope_name);
    if (it != scopes_.end()) {
        const auto& data = it->second;
        ScopeStats stats;
        stats.total_time_ms = data.total_time_ms;
        stats.min_time_ms = data.min_time_ms;
        stats.max_time_ms = data.max_time_ms;
        stats.call_count = data.call_count;
        stats.avg_time_ms = (data.call_count > 0) ? 
            data.total_time_ms / data.call_count : 0.0;
        return stats;
    }

    ScopeStats empty;
    empty.total_time_ms = 0.0;
    empty.min_time_ms = 0.0;
    empty.max_time_ms = 0.0;
    empty.avg_time_ms = 0.0;
    empty.call_count = 0;
    return empty;
}

void CPUProfiler::PrintReport() const {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "CPU Profiling Report\n";
    std::cout << "═══════════════════════════════════════════════\n";

    double total_time = 0.0;

    for (const auto& pair : scopes_) {
        const auto& data = pair.second;
        total_time += data.total_time_ms;

        std::cout << std::left << std::setw(30) << data.name << ": ";
        std::cout << std::fixed << std::setprecision(2) << std::setw(8) << data.total_time_ms
                  << "ms total | ";
        std::cout << std::setw(8) << (data.call_count > 0 ? data.total_time_ms / data.call_count : 0.0)
                  << "ms avg | ";
        std::cout << std::setw(6) << data.call_count << " calls\n";
    }

    std::cout << "─────────────────────────────────────────────────\n";
    std::cout << "Total Time: " << std::fixed << std::setprecision(2) << total_time << "ms\n";
    std::cout << "═══════════════════════════════════════════════\n";
}

void CPUProfiler::GenerateReport(const std::string& filename) const {
    std::ofstream report(filename);

    report << "CPU Profiling Report\n";
    report << "═══════════════════════════════════════════════\n\n";

    double total_time = 0.0;

    for (const auto& pair : scopes_) {
        const auto& data = pair.second;
        total_time += data.total_time_ms;

        report << std::left << std::setw(30) << data.name << ": ";
        report << std::fixed << std::setprecision(4)
               << "Total=" << std::setw(10) << data.total_time_ms << "ms, "
               << "Avg=" << std::setw(10) << (data.call_count > 0 ? data.total_time_ms / data.call_count : 0.0) << "ms, "
               << "Min=" << std::setw(10) << data.min_time_ms << "ms, "
               << "Max=" << std::setw(10) << data.max_time_ms << "ms, "
               << "Count=" << std::setw(6) << data.call_count << "\n";
    }

    report << "\n═══════════════════════════════════════════════\n";
    report << "Total Time: " << std::fixed << std::setprecision(2) << total_time << "ms\n";
    report.close();
}

void CPUProfiler::Clear() {
    scopes_.clear();
    scope_stack_.clear();
}

}  // namespace phase47
